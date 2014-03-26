/**************************************************************************
**  This file is part of Fossil VCS plugin for Qt Creator
**
**  Copyright (c) 2013 - 2014, Artur Shepilko, <qtc-fossil@nomadbyte.com>.
**
**  Based on Bazaar VCS plugin for Qt Creator by Hugues Delorme.
**
**  Permission is hereby granted, free of charge, to any person obtaining a copy
**  of this software and associated documentation files (the "Software"), to deal
**  in the Software without restriction, including without limitation the rights
**  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
**  copies of the Software, and to permit persons to whom the Software is
**  furnished to do so, subject to the following conditions:
**
**  The above copyright notice and this permission notice shall be included in
**  all copies or substantial portions of the Software.
**
**  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
**  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
**  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
**  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
**  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
**  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
**  THE SOFTWARE.
**************************************************************************/

#include "fossilplugin.h"
#include "constants.h"
#include "fossilclient.h"
#include "fossilcontrol.h"
#include "optionspage.h"
#include "fossilcommitwidget.h"
#include "fossileditor.h"
#include "pullorpushdialog.h"
#include "configuredialog.h"
#include "commiteditor.h"
#include "clonewizard.h"

#include "ui_revertdialog.h"

#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/id.h>
#include <coreplugin/vcsmanager.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/icore.h>
#include <coreplugin/documentmanager.h>
#include <coreplugin/editormanager/editormanager.h>

#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/project.h>

#include <locator/commandlocator.h>

#include <utils/parameteraction.h>
#include <utils/qtcassert.h>

#include <vcsbase/basevcseditorfactory.h>
#include <vcsbase/basevcssubmiteditorfactory.h>
#include <vcsbase/vcsbasesubmiteditor.h>
#include <vcsbase/vcsbaseeditor.h>
#include <vcsbase/vcsbaseoutputwindow.h>

#include <QtPlugin>
#include <QAction>
#include <QMenu>
#include <QDir>
#include <QDialog>
#include <QMessageBox>
#include <QFileDialog>

#include <QDebug>


using namespace Fossil::Internal;
using namespace Fossil;

static const VcsBase::VcsBaseEditorParameters editorParameters[] = {
    {   VcsBase::LogOutput,
        Constants::FILELOG_ID,
        Constants::FILELOG_DISPLAY_NAME,
        Constants::FILELOG,
        Constants::LOGAPP},

    {    VcsBase::AnnotateOutput,
         Constants::ANNOTATELOG_ID,
         Constants::ANNOTATELOG_DISPLAY_NAME,
         Constants::ANNOTATELOG,
         Constants::ANNOTATEAPP},

    {   VcsBase::DiffOutput,
        Constants::DIFFLOG_ID,
        Constants::DIFFLOG_DISPLAY_NAME,
        Constants::DIFFLOG,
        Constants::DIFFAPP}
};

static const VcsBase::VcsBaseSubmitEditorParameters submitEditorParameters = {
    Constants::COMMITMIMETYPE,
    Constants::COMMIT_ID,
    Constants::COMMIT_DISPLAY_NAME,
    Constants::COMMIT_ID,
    VcsBase::VcsBaseSubmitEditorParameters::DiffFiles
};


FossilPlugin *FossilPlugin::m_instance = 0;

FossilPlugin::FossilPlugin()
    : m_optionsPage(0),
      m_client(0),
      m_commandLocator(0),
      m_addAction(0),
      m_deleteAction(0),
      m_menuAction(0),
      m_submitActionTriggered(false)
{
    m_instance = this;
}

FossilPlugin::~FossilPlugin()
{
    if (m_client) {
        delete m_client;
        m_client = 0;
    }

    m_instance = 0;
}

bool FossilPlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    Q_UNUSED(arguments);
    Q_UNUSED(errorMessage);

    typedef VcsBase::VcsEditorFactory<FossilEditor> FossilEditorFactory;

    m_client = new FossilClient(&m_settings);
    initializeVcs(new FossilControl(m_client));

    m_optionsPage = new OptionsPage();
    addAutoReleasedObject(m_optionsPage);
    m_settings.readSettings(Core::ICore::settings());

    connect(m_client, SIGNAL(changed(QVariant)), versionControl(), SLOT(changed(QVariant)));

    static const char *describeSlot = SLOT(view(QString,QString));
    const int editorCount = sizeof(editorParameters) / sizeof(VcsBase::VcsBaseEditorParameters);
    for (int i = 0; i < editorCount; i++)
        addAutoReleasedObject(new FossilEditorFactory(editorParameters + i, m_client, describeSlot));

    addAutoReleasedObject(new VcsBase::VcsSubmitEditorFactory<CommitEditor>(&submitEditorParameters));

    addAutoReleasedObject(new CloneWizard);

    const QString prefix = QLatin1String("fossil");
    m_commandLocator = new Locator::CommandLocator(Core::Id("Fossil"), prefix, prefix);
    addAutoReleasedObject(m_commandLocator);

    createMenu();

    createSubmitEditorActions();

    return true;
}

FossilPlugin *FossilPlugin::instance()
{
    return m_instance;
}

FossilClient *FossilPlugin::client() const
{
    return m_client;
}

const FossilSettings &FossilPlugin::settings() const
{
    return m_settings;
}

void FossilPlugin::setSettings(const FossilSettings &settings)
{
    if (settings != m_settings) {
        const FossilSettings previousSettings(m_settings);

        m_settings = settings;

        static_cast<FossilControl *>(versionControl())->emitConfigurationChanged();
    }
}

void FossilPlugin::createMenu()
{
    Core::Context context(Core::Constants::C_GLOBAL);

    // Create menu item for Fossil
    m_fossilContainer = Core::ActionManager::createMenu(Core::Id("Fossil.FossilMenu"));
    QMenu *menu = m_fossilContainer->menu();
    menu->setTitle(tr("&Fossil"));

    createFileActions(context);
    m_fossilContainer->addSeparator(context);
    createDirectoryActions(context);
    m_fossilContainer->addSeparator(context);
    createRepositoryActions(context);
    m_fossilContainer->addSeparator(context);

    // Request the Tools menu and add the Fossil menu to it
    Core::ActionContainer *toolsMenu = Core::ActionManager::actionContainer(Core::Id(Core::Constants::M_TOOLS));
    toolsMenu->addMenu(m_fossilContainer);
    m_menuAction = m_fossilContainer->menu()->menuAction();
}

void FossilPlugin::createFileActions(const Core::Context &context)
{
    Core::Command *command;

    m_annotateFile = new Utils::ParameterAction(tr("Annotate Current File"), tr("Annotate \"%1\""), Utils::ParameterAction::EnabledWithParameter, this);
    command = Core::ActionManager::registerAction(m_annotateFile, Core::Id(Constants::ANNOTATE), context);
    command->setAttribute(Core::Command::CA_UpdateText);
    connect(m_annotateFile, SIGNAL(triggered()), this, SLOT(annotateCurrentFile()));
    m_fossilContainer->addAction(command);
    m_commandLocator->appendCommand(command);

    m_diffFile = new Utils::ParameterAction(tr("Diff Current File"), tr("Diff \"%1\""), Utils::ParameterAction::EnabledWithParameter, this);
    command = Core::ActionManager::registerAction(m_diffFile, Core::Id(Constants::DIFF), context);
    command->setAttribute(Core::Command::CA_UpdateText);
    command->setDefaultKeySequence(QKeySequence(Core::UseMacShortcuts ? tr("Meta+I,Meta+D") : tr("ALT+I,Alt+D")));
    connect(m_diffFile, SIGNAL(triggered()), this, SLOT(diffCurrentFile()));
    m_fossilContainer->addAction(command);
    m_commandLocator->appendCommand(command);

    m_logFile = new Utils::ParameterAction(tr("Timeline Current File"), tr("Timeline \"%1\""), Utils::ParameterAction::EnabledWithParameter, this);
    command = Core::ActionManager::registerAction(m_logFile, Core::Id(Constants::LOG), context);
    command->setAttribute(Core::Command::CA_UpdateText);
    command->setDefaultKeySequence(QKeySequence(Core::UseMacShortcuts ? tr("Meta+I,Meta+L") : tr("ALT+I,Alt+L")));
    connect(m_logFile, SIGNAL(triggered()), this, SLOT(logCurrentFile()));
    m_fossilContainer->addAction(command);
    m_commandLocator->appendCommand(command);

    m_statusFile = new Utils::ParameterAction(tr("Status Current File"), tr("Status \"%1\""), Utils::ParameterAction::EnabledWithParameter, this);
    command = Core::ActionManager::registerAction(m_statusFile, Core::Id(Constants::STATUS), context);
    command->setAttribute(Core::Command::CA_UpdateText);
    command->setDefaultKeySequence(QKeySequence(Core::UseMacShortcuts ? tr("Meta+I,Meta+S") : tr("ALT+I,Alt+S")));
    connect(m_statusFile, SIGNAL(triggered()), this, SLOT(statusCurrentFile()));
    m_fossilContainer->addAction(command);
    m_commandLocator->appendCommand(command);

    m_fossilContainer->addSeparator(context);

    m_addAction = new Utils::ParameterAction(tr("Add"), tr("Add \"%1\""), Utils::ParameterAction::EnabledWithParameter, this);
    command = Core::ActionManager::registerAction(m_addAction, Core::Id(Constants::ADD), context);
    command->setAttribute(Core::Command::CA_UpdateText);
    connect(m_addAction, SIGNAL(triggered()), this, SLOT(addCurrentFile()));
    m_fossilContainer->addAction(command);
    m_commandLocator->appendCommand(command);

    m_deleteAction = new Utils::ParameterAction(tr("Delete..."), tr("Delete \"%1\"..."), Utils::ParameterAction::EnabledWithParameter, this);
    command = Core::ActionManager::registerAction(m_deleteAction, Core::Id(Constants::DELETE), context);
    command->setAttribute(Core::Command::CA_UpdateText);
    connect(m_deleteAction, SIGNAL(triggered()), this, SLOT(promptToDeleteCurrentFile()));
    m_fossilContainer->addAction(command);
    m_commandLocator->appendCommand(command);

    m_revertFile = new Utils::ParameterAction(tr("Revert Current File..."), tr("Revert \"%1\"..."), Utils::ParameterAction::EnabledWithParameter, this);
    command = Core::ActionManager::registerAction(m_revertFile, Core::Id(Constants::REVERT), context);
    command->setAttribute(Core::Command::CA_UpdateText);
    connect(m_revertFile, SIGNAL(triggered()), this, SLOT(revertCurrentFile()));
    m_fossilContainer->addAction(command);
    m_commandLocator->appendCommand(command);
}

void FossilPlugin::addCurrentFile()
{
    const VcsBase::VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasFile(), return);
    m_client->synchronousAdd(state.currentFileTopLevel(), state.relativeCurrentFile());
}

void FossilPlugin::annotateCurrentFile()
{
    const VcsBase::VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasFile(), return);
    m_client->annotate(state.currentFileTopLevel(), state.relativeCurrentFile());
}

void FossilPlugin::diffCurrentFile()
{
    const VcsBase::VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasFile(), return);
    m_client->diff(state.currentFileTopLevel(), QStringList(state.relativeCurrentFile()));
}

void FossilPlugin::logCurrentFile()
{
    const VcsBase::VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasFile(), return);
    QStringList extraOptions;
    extraOptions << QLatin1String("-n") << QString(QLatin1String("%1"))
                                          .arg(settings().intValue(FossilSettings::logCountKey));

    // annotate only supported for current revision, so disable context menu
    m_client->log(state.currentFileTopLevel(), QStringList(state.relativeCurrentFile()),
                  extraOptions, false);
}

void FossilPlugin::revertCurrentFile()
{
    const VcsBase::VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasFile(), return);

    QDialog dialog;
    Ui::RevertDialog revertUi;
    revertUi.setupUi(&dialog);
    if (dialog.exec() != QDialog::Accepted)
        return;
    m_client->revertFile(state.currentFileTopLevel(),
                         state.relativeCurrentFile(),
                         revertUi.revisionLineEdit->text());
}

void FossilPlugin::statusCurrentFile()
{
    const VcsBase::VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasFile(), return);
    m_client->status(state.currentFileTopLevel(), state.relativeCurrentFile());
}

void FossilPlugin::createDirectoryActions(const Core::Context &context)
{
    QAction *action;
    Core::Command *command;

    action = new QAction(tr("Diff"), this);
    m_repositoryActionList.append(action);
    command = Core::ActionManager::registerAction(action, Core::Id(Constants::DIFFMULTI), context);
    connect(action, SIGNAL(triggered()), this, SLOT(diffRepository()));
    m_fossilContainer->addAction(command);
    m_commandLocator->appendCommand(command);

    action = new QAction(tr("Timeline"), this);
    m_repositoryActionList.append(action);
    command = Core::ActionManager::registerAction(action, Core::Id(Constants::LOGMULTI), context);
    command->setDefaultKeySequence(QKeySequence(Core::UseMacShortcuts ? tr("Meta+I,Meta+T") : tr("ALT+I,Alt+T")));
    connect(action, SIGNAL(triggered()), this, SLOT(logRepository()));
    m_fossilContainer->addAction(command);
    m_commandLocator->appendCommand(command);

    action = new QAction(tr("Revert..."), this);
    m_repositoryActionList.append(action);
    command = Core::ActionManager::registerAction(action, Core::Id(Constants::REVERTMULTI), context);
    connect(action, SIGNAL(triggered()), this, SLOT(revertAll()));
    m_fossilContainer->addAction(command);
    m_commandLocator->appendCommand(command);

    action = new QAction(tr("Status"), this);
    m_repositoryActionList.append(action);
    command = Core::ActionManager::registerAction(action, Core::Id(Constants::STATUSMULTI), context);
    connect(action, SIGNAL(triggered()), this, SLOT(statusMulti()));
    m_fossilContainer->addAction(command);
    m_commandLocator->appendCommand(command);
}


void FossilPlugin::diffRepository()
{
    const VcsBase::VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasTopLevel(), return);
    m_client->diff(state.topLevel());
}

void FossilPlugin::logRepository()
{
    const VcsBase::VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasTopLevel(), return);
    QStringList extraOptions;
    FossilClient::SupportedFeatures features = m_client->supportedFeatures();
    extraOptions << QLatin1String("-n") << QString(QLatin1String("%1"))
                                                .arg(settings().intValue(FossilSettings::logCountKey));
    if (features.testFlag(FossilClient::TimelineWidthFeature))
        extraOptions << QLatin1String("-W") << QString(QLatin1String("%1"))
                                                    .arg(settings().intValue(FossilSettings::timelineWidthKey));

    m_client->logRepository(state.topLevel(), extraOptions);
}

void FossilPlugin::revertAll()
{
    const VcsBase::VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasTopLevel(), return);

    QDialog dialog;
    Ui::RevertDialog revertUi;
    revertUi.setupUi(&dialog);
    if (dialog.exec() != QDialog::Accepted)
        return;
    m_client->revertAll(state.topLevel(), revertUi.revisionLineEdit->text());
}

void FossilPlugin::statusMulti()
{
    const VcsBase::VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasTopLevel(), return);
    m_client->status(state.topLevel());
}

void FossilPlugin::createRepositoryActions(const Core::Context &context)
{
    QAction *action = 0;
    Core::Command *command = 0;

    action = new QAction(tr("Pull..."), this);
    m_repositoryActionList.append(action);
    command = Core::ActionManager::registerAction(action, Core::Id(Constants::PULL), context);
    connect(action, SIGNAL(triggered()), this, SLOT(pull()));
    m_fossilContainer->addAction(command);
    m_commandLocator->appendCommand(command);

    action = new QAction(tr("Push..."), this);
    m_repositoryActionList.append(action);
    command = Core::ActionManager::registerAction(action, Core::Id(Constants::PUSH), context);
    connect(action, SIGNAL(triggered()), this, SLOT(push()));
    m_fossilContainer->addAction(command);
    m_commandLocator->appendCommand(command);

    action = new QAction(tr("Update..."), this);
    m_repositoryActionList.append(action);
    command = Core::ActionManager::registerAction(action, Core::Id(Constants::UPDATE), context);
    command->setDefaultKeySequence(QKeySequence(Core::UseMacShortcuts ? tr("Meta+I,Meta+U") : tr("ALT+I,Alt+U")));
    connect(action, SIGNAL(triggered()), this, SLOT(update()));
    m_fossilContainer->addAction(command);
    m_commandLocator->appendCommand(command);

    action = new QAction(tr("Commit..."), this);
    m_repositoryActionList.append(action);
    command = Core::ActionManager::registerAction(action, Core::Id(Constants::COMMIT), context);
    command->setDefaultKeySequence(QKeySequence(Core::UseMacShortcuts ? tr("Meta+I,Meta+C") : tr("ALT+I,Alt+C")));
    connect(action, SIGNAL(triggered()), this, SLOT(commit()));
    m_fossilContainer->addAction(command);
    m_commandLocator->appendCommand(command);

    action = new QAction(tr("Settings ..."), this);
    m_repositoryActionList.append(action);
    command = Core::ActionManager::registerAction(action, Core::Id(Constants::CONFIGURE_REPOSITORY), context);
    connect(action, SIGNAL(triggered()), this, SLOT(configureRepository()));
    m_fossilContainer->addAction(command);
    m_commandLocator->appendCommand(command);

    QAction *createRepositoryAction = new QAction(tr("Create Repository..."), this);
    command = Core::ActionManager::registerAction(createRepositoryAction, Core::Id(Constants::CREATE_REPOSITORY), context);
    connect(createRepositoryAction, SIGNAL(triggered()), this, SLOT(createRepository()));
    m_fossilContainer->addAction(command);
}

void FossilPlugin::pull()
{
    const VcsBase::VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasTopLevel(), return);

    PullOrPushDialog dialog(PullOrPushDialog::PullMode);
    dialog.setLocalBaseDirectory(m_client->settings()->stringValue(FossilSettings::defaultRepoPathKey));
    dialog.setDefaultRemoteLocation(m_client->synchronousGetRepositoryURL(state.topLevel()));
    if (dialog.exec() != QDialog::Accepted)
        return;

    QString remoteLocation(dialog.remoteLocation());
    if (remoteLocation.isEmpty())
        remoteLocation = m_client->synchronousGetRepositoryURL(state.topLevel());

    if (remoteLocation.isEmpty()) {
        VcsBase::VcsBaseOutputWindow *outputWindow = VcsBase::VcsBaseOutputWindow::instance();

        outputWindow->appendError(tr("Remote repository is not defined."));
        return;
    }

    QStringList extraOptions;
    if (!dialog.isRememberOptionEnabled())
        extraOptions << QLatin1String("--once");
    if (dialog.isPrivateOptionEnabled())
        extraOptions << QLatin1String("--private");
    m_client->synchronousPull(state.topLevel(), remoteLocation, extraOptions);
}

void FossilPlugin::push()
{
    const VcsBase::VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasTopLevel(), return);

    PullOrPushDialog dialog(PullOrPushDialog::PushMode);
    dialog.setLocalBaseDirectory(m_client->settings()->stringValue(FossilSettings::defaultRepoPathKey));
    dialog.setDefaultRemoteLocation(m_client->synchronousGetRepositoryURL(state.topLevel()));
    if (dialog.exec() != QDialog::Accepted)
        return;

    QString remoteLocation(dialog.remoteLocation());
    if (remoteLocation.isEmpty())
        remoteLocation = m_client->synchronousGetRepositoryURL(state.topLevel());

    if (remoteLocation.isEmpty()) {
        VcsBase::VcsBaseOutputWindow *outputWindow = VcsBase::VcsBaseOutputWindow::instance();

        outputWindow->appendError(tr("Remote repository is not defined."));
        return;
    }

    QStringList extraOptions;
    if (!dialog.isRememberOptionEnabled())
        extraOptions << QLatin1String("--once");
    if (dialog.isPrivateOptionEnabled())
        extraOptions << QLatin1String("--private");
    m_client->synchronousPush(state.topLevel(), remoteLocation, extraOptions);
}

void FossilPlugin::update()
{
    const VcsBase::VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasTopLevel(), return);

    QDialog dialog;
    Ui::RevertDialog revertUi;
    revertUi.setupUi(&dialog);
    dialog.setWindowTitle(tr("Update"));
    if (dialog.exec() != QDialog::Accepted)
        return;
    m_client->update(state.topLevel(), revertUi.revisionLineEdit->text());
}

void FossilPlugin::configureRepository()
{
    const VcsBase::VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasTopLevel(), return);

    ConfigureDialog dialog;

    // retrieve current settings from the repository
    RepositorySettings currentSettings = m_client->synchronousSettingsQuery(state.topLevel());

    dialog.setSettings(currentSettings);
    if (dialog.exec() != QDialog::Accepted)
        return;
    const RepositorySettings newSettings = dialog.settings();

    m_client->synchronousConfigureRepository(state.topLevel(), newSettings, currentSettings);
}

void FossilPlugin::createSubmitEditorActions()
{
    Core::Context context(Constants::COMMIT_ID);
    Core::Command *command;

    m_editorCommit = new QAction(VcsBase::VcsBaseSubmitEditor::submitIcon(), tr("Commit"), this);
    command = Core::ActionManager::registerAction(m_editorCommit, Core::Id(Constants::COMMIT), context);
    command->setAttribute(Core::Command::CA_UpdateText);
    connect(m_editorCommit, SIGNAL(triggered()), this, SLOT(commitFromEditor()));

    m_editorDiff = new QAction(VcsBase::VcsBaseSubmitEditor::diffIcon(), tr("Diff &Selected Files"), this);
    command = Core::ActionManager::registerAction(m_editorDiff, Core::Id(Constants::DIFFEDITOR), context);

    m_editorUndo = new QAction(tr("&Undo"), this);
    command = Core::ActionManager::registerAction(m_editorUndo, Core::Id(Core::Constants::UNDO), context);

    m_editorRedo = new QAction(tr("&Redo"), this);
    command = Core::ActionManager::registerAction(m_editorRedo, Core::Id(Core::Constants::REDO), context);
}

void FossilPlugin::commit()
{
    if (raiseSubmitEditor())
        return;

    const VcsBase::VcsBasePluginState state = currentState();
    QTC_ASSERT(state.hasTopLevel(), return);

    m_submitRepository = state.topLevel();

    connect(m_client, SIGNAL(parsedStatus(QList<VcsBase::VcsBaseClient::StatusItem>)),
            this, SLOT(showCommitWidget(QList<VcsBase::VcsBaseClient::StatusItem>)));

    QStringList extraOptions;
    m_client->emitParsedStatus(m_submitRepository, extraOptions);
}

void FossilPlugin::showCommitWidget(const QList<VcsBase::VcsBaseClient::StatusItem> &status)
{
    VcsBase::VcsBaseOutputWindow *outputWindow = VcsBase::VcsBaseOutputWindow::instance();
    //Once we receive our data release the connection so it can be reused elsewhere
    disconnect(m_client, SIGNAL(parsedStatus(QList<VcsBase::VcsBaseClient::StatusItem>)),
               this, SLOT(showCommitWidget(QList<VcsBase::VcsBaseClient::StatusItem>)));

    if (status.isEmpty()) {
        outputWindow->appendError(tr("There are no changes to commit."));
        return;
    }

    // Start new temp file for commit message
    Utils::TempFileSaver saver;
    // Keep the file alive, else it removes self and forgets its name
    saver.setAutoRemove(false);
    if (!saver.finalize()) {
        VcsBase::VcsBaseOutputWindow::instance()->appendError(saver.errorString());
        return;
    }

    Core::IEditor *editor = Core::EditorManager::openEditor(saver.fileName(), Constants::COMMIT_ID);
    if (!editor) {
        outputWindow->appendError(tr("Unable to create an editor for the commit."));
        return;
    }

    CommitEditor *commitEditor = qobject_cast<CommitEditor *>(editor);

    if (!commitEditor) {
        outputWindow->appendError(tr("Unable to create a commit editor."));
        return;
    }
    setSubmitEditor(commitEditor);

    const QString msg = tr("Commit changes for \"%1\".").
            arg(QDir::toNativeSeparators(m_submitRepository));
    commitEditor->document()->setDisplayName(msg);

    const RevisionInfo currentRevision = m_client->synchronousRevisionQuery(m_submitRepository);
    const BranchInfo currentBranch = m_client->synchronousBranchQuery(m_submitRepository);
    const QString currentUser = m_client->synchronousUserDefaultQuery(m_submitRepository);
    QStringList tags = m_client->synchronousTagQuery(m_submitRepository, currentRevision.id);
    // Fossil includes branch name in tag list -- remove.
    tags.removeAll(currentBranch.name());
    commitEditor->setFields(m_submitRepository, currentBranch, tags, currentUser, status);

    commitEditor->registerActions(m_editorUndo, m_editorRedo, m_editorCommit, m_editorDiff);
    connect(commitEditor, SIGNAL(diffSelectedFiles(QStringList)),
            this, SLOT(diffFromEditorSelected(QStringList)));
    commitEditor->setCheckScriptWorkingDirectory(m_submitRepository);
}

void FossilPlugin::diffFromEditorSelected(const QStringList &files)
{
    m_client->diff(m_submitRepository, files);
}

static inline bool ask(QWidget *parent, const QString &title, const QString &question, bool defaultValue = true)

{
    const QMessageBox::StandardButton defaultButton = defaultValue ? QMessageBox::Yes : QMessageBox::No;
    return QMessageBox::question(parent, title, question, QMessageBox::Yes|QMessageBox::No, defaultButton) == QMessageBox::Yes;
}

void FossilPlugin::createRepository()
{
    // re-implemented from void VcsBasePlugin::createRepository()

    // Find current starting directory
    QString directory;
    if (const ProjectExplorer::Project *currentProject = ProjectExplorer::ProjectExplorerPlugin::currentProject())
        directory = QFileInfo(currentProject->document()->filePath()).absolutePath();
    // Prompt for a directory that is not under version control yet
    QWidget *mw = Core::ICore::mainWindow();
    do {
        directory = QFileDialog::getExistingDirectory(mw, tr("Choose Checkout Directory"), directory);
        if (directory.isEmpty())
            return;
        const Core::IVersionControl *managingControl = Core::VcsManager::findVersionControlForDirectory(directory);
        if (managingControl == 0)
            break;
        const QString question = tr("The directory '%1' is already managed by a version control system (%2)."
                                    " Would you like to specify another directory?").arg(directory, managingControl->displayName());

        if (!ask(mw, tr("Repository already under version control"), question))
            return;
    } while (true);
    // Create
    const bool rc = static_cast<FossilControl *>(versionControl())->vcsCreateRepository(directory);
    const QString nativeDir = QDir::toNativeSeparators(directory);
    if (rc) {
        QMessageBox::information(mw, tr("Repository Created"),
                                 tr("A version control repository has been created in %1.").
                                 arg(nativeDir));
    } else {
        QMessageBox::warning(mw, tr("Repository Creation Failed"),
                                 tr("A version control repository could not be created in %1.").
                                 arg(nativeDir));
    }
}

void FossilPlugin::commitFromEditor()
{
    // Close the submit editor
    m_submitActionTriggered = true;
    Core::EditorManager::closeEditor();
}

bool FossilPlugin::submitEditorAboutToClose()
{
    CommitEditor *commitEditor = qobject_cast<CommitEditor *>(submitEditor());
    QTC_ASSERT(commitEditor, return true);
    Core::IDocument *editorDocument = commitEditor->document();
    QTC_ASSERT(editorDocument, return true);

    bool promptOnSubmit = false;
    const VcsBase::VcsBaseSubmitEditor::PromptSubmitResult response =
            commitEditor->promptSubmit(tr("Close Commit Editor"), tr("Do you want to commit the changes?"),
                                       tr("Message check failed. Do you want to proceed?"),
                                       &promptOnSubmit, !m_submitActionTriggered);
    m_submitActionTriggered = false;

    switch (response) {
    case VcsBase::VcsBaseSubmitEditor::SubmitCanceled:
        return false;
    case VcsBase::VcsBaseSubmitEditor::SubmitDiscarded:
        return true;
    default:
        break;
    }

    QStringList files = commitEditor->checkedFiles();
    if (!files.empty()) {
        //save the commit message
        if (!Core::DocumentManager::saveDocument(editorDocument))
            return false;

        //rewrite entries of the form 'file => newfile' to 'newfile' because
        //this would mess the commit command
        for (QStringList::iterator iFile = files.begin(); iFile != files.end(); ++iFile) {
            const QStringList parts = iFile->split(QLatin1String(" => "), QString::SkipEmptyParts);
            if (!parts.isEmpty())
                *iFile = parts.last();
        }

        FossilCommitWidget *commitWidget = commitEditor->commitWidget();
        QStringList extraOptions;
        // Author -- override the repository-default user
        if (!commitWidget->committer().isEmpty())
            extraOptions << QLatin1String("--user") << commitWidget->committer();
        // Branch
        QString branch = commitWidget->newBranch();
        if (!branch.isEmpty()) {
            // @TODO: make enquote utility function
            QString enquotedBranch = branch;
            if (branch.contains(QRegExp(QLatin1String("\\s"))))
                enquotedBranch = QString(QLatin1String("\"%1\"")).arg(branch);
            extraOptions << QLatin1String("--branch") << enquotedBranch;
        }
        // Tags
        foreach (QString tag, commitWidget->tags()) {
            extraOptions << QLatin1String("--tag") << tag;
        }

        // Whether local commit or not
        if (commitWidget->isPrivateOptionEnabled())
            extraOptions += QLatin1String("--private");
        m_client->commit(m_submitRepository, files, editorDocument->filePath(), extraOptions);
    }
    return true;
}


void FossilPlugin::updateActions(VcsBase::VcsBasePlugin::ActionState as)
{
    if (!enableMenuAction(as, m_menuAction)) {
        m_commandLocator->setEnabled(false);
        return;
    }
    const QString filename = currentState().currentFileName();
    const bool repoEnabled = currentState().hasTopLevel();
    m_commandLocator->setEnabled(repoEnabled);

    m_annotateFile->setParameter(filename);
    m_diffFile->setParameter(filename);
    m_logFile->setParameter(filename);
    m_addAction->setParameter(filename);
    m_deleteAction->setParameter(filename);
    m_revertFile->setParameter(filename);
    m_statusFile->setParameter(filename);

    foreach (QAction *repoAction, m_repositoryActionList)
        repoAction->setEnabled(repoEnabled);
}

#ifdef WITH_TESTS
#include <QTest>

void Fossil::Internal::FossilPlugin::testDiffFileResolving_data()
{
    QTest::addColumn<QByteArray>("header");
    QTest::addColumn<QByteArray>("fileName");

    QTest::newRow("New") << QByteArray(
            "ADDED   src/plugins/fossil/fossilclient.cpp\n"
            "Index: src/plugins/fossil/fossilclient.cpp\n"
            "==================================================================\n"
            "--- src/plugins/fossil/fossilclient.cpp\n"
            "+++ src/plugins/fossil/fossilclient.cpp\n"
            "@@ -0,0 +1,295 @@\n"
        )
        << QByteArray("src/plugins/fossil/fossilclient.cpp");
    QTest::newRow("Deleted") << QByteArray(
            "DELETED src/plugins/fossil/fossilclient.cpp\n"
            "Index: src/plugins/fossil/fossilclient.cpp\n"
            "==================================================================\n"
            "--- src/plugins/fossil/fossilclient.cpp\n"
            "+++ src/plugins/fossil/fossilclient.cpp\n"
            "@@ -1,266 +0,0 @@\n"
        )
        << QByteArray("src/plugins/fossil/fossilclient.cpp");
    QTest::newRow("Modified") << QByteArray(
            "Index: src/plugins/fossil/fossilclient.cpp\n"
            "==================================================================\n"
            "--- src/plugins/fossil/fossilclient.cpp\n"
            "+++ src/plugins/fossil/fossilclient.cpp\n"
            "@@ -112,22 +112,37 @@\n"
        )
        << QByteArray("src/plugins/fossil/fossilclient.cpp");
}

void Fossil::Internal::FossilPlugin::testDiffFileResolving()
{
    FossilEditor editor(editorParameters + 2, 0);
    editor.testDiffFileResolving();
}

void Fossil::Internal::FossilPlugin::testLogResolving()
{
    QByteArray data(
        "=== 2014-03-08 ===\n"
        "22:14:02 [ac6d1129b8] Change scaling algorithm. (user: ninja tags: ninja-fixes-5.1)\n"
        "   EDITED src/core/scaler.cpp\n"
        "20:23:51 [56d6917c3b] *BRANCH* Add width option (conditional). (user: ninja tags: ninja-fixes-5.1)\n"
        "   EDITED src/core/scaler.cpp\n"
        "   EDITED src/core/scaler.h\n"
    );
    FossilEditor editor(editorParameters, 0);
    editor.testLogResolving(data, "ac6d1129b8", "56d6917c3b");
}
#endif

Q_EXPORT_PLUGIN(FossilPlugin)
