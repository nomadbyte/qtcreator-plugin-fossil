/**************************************************************************
**  This file is part of Fossil VCS plugin for Qt Creator
**
**  Copyright (c) 2013 - 2018, Artur Shepilko, <qtc-fossil@nomadbyte.com>.
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

#include "fossilclient.h"
#include "fossileditor.h"
#include "constants.h"

#include <coreplugin/id.h>

#include <vcsbase/vcsbaseplugin.h>
#include <vcsbase/vcsbaseeditor.h>
#include <vcsbase/vcsbaseeditorconfig.h>
#include <vcsbase/vcsoutputwindow.h>
#include <vcsbase/vcscommand.h>

#include <utils/algorithm.h>
#include <utils/fileutils.h>
#include <utils/hostosinfo.h>
#include <utils/qtcassert.h>

#include <QSyntaxHighlighter>

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QMap>
#include <QRegularExpression>

namespace Fossil {
namespace Internal {

// Parameter widget controlling whitespace diff mode, associated with a parameter
class FossilDiffConfig : public VcsBase::VcsBaseEditorConfig
{
    Q_OBJECT

public:
    FossilDiffConfig(FossilClient *client, QToolBar *toolBar) :
        VcsBase::VcsBaseEditorConfig(toolBar)
    {
        QTC_ASSERT(client, return);

        VcsBase::VcsBaseClientSettings &settings = client->settings();
        FossilClient::SupportedFeatures features = client->supportedFeatures();

        if (features.testFlag(FossilClient::DiffIgnoreWhiteSpaceFeature)) {
            mapSetting(addToggleButton("-w", tr("Ignore All Whitespace")),
                       settings.boolPointer(FossilSettings::diffIgnoreAllWhiteSpaceKey));
            mapSetting(addToggleButton("--strip-trailing-cr", tr("Strip Trailing CR")),
                       settings.boolPointer(FossilSettings::diffStripTrailingCRKey));
        }
    }
};

// Parameter widget controlling annotate/blame mode
class FossilAnnotateConfig : public VcsBase::VcsBaseEditorConfig
{
    Q_OBJECT

public:
    FossilAnnotateConfig(FossilClient *client, QToolBar *toolBar) :
        VcsBase::VcsBaseEditorConfig(toolBar)
    {
        QTC_ASSERT(client, return);

        VcsBase::VcsBaseClientSettings &settings = client->settings();
        FossilClient::SupportedFeatures features = client->supportedFeatures();

        if (features.testFlag(FossilClient::AnnotateBlameFeature)) {
            mapSetting(addToggleButton("|BLAME|", tr("Show Committers")),
                       settings.boolPointer(FossilSettings::annotateShowCommittersKey));
        }

        // Force listVersions setting to false by default.
        // This way the annotated line number would not get offset by the version list.
        settings.setValue(FossilSettings::annotateListVersionsKey, false);

        mapSetting(addToggleButton("--log", tr("List Versions")),
                   settings.boolPointer(FossilSettings::annotateListVersionsKey));
    }
};

class FossilLogCurrentFileConfig : public VcsBase::VcsBaseEditorConfig
{
    Q_OBJECT

public:
    FossilLogCurrentFileConfig(FossilClient *client, QToolBar *toolBar) :
        VcsBase::VcsBaseEditorConfig(toolBar)
    {
        QTC_ASSERT(client, return);
    }

};

class FossilLogConfig : public VcsBase::VcsBaseEditorConfig
{
    Q_OBJECT

public:
    FossilLogConfig(FossilClient *client, QToolBar *toolBar) :
        VcsBase::VcsBaseEditorConfig(toolBar),
        m_client(client)
    {
        QTC_ASSERT(client, return);

        addLineageComboBox();
        addVerboseToggleButton();
        addItemTypeComboBox();
    }

    void addLineageComboBox()
    {
        VcsBase::VcsBaseClientSettings &settings = m_client->settings();

        // ancestors/descendants filter
        // This is a positional argument not an option.
        // Normally it takes the checkin/branch/tag as an additional parameter
        // (trunk by default)
        // So we kludge this by coding it as a meta-option (pipe-separated),
        // then parse it out in arguments.
        // All-choice is a blank argument with no additional parameters
        QList<ComboBoxItem> lineageFilterChoices;
        lineageFilterChoices << ComboBoxItem(tr("Ancestors"), "ancestors")
                        << ComboBoxItem(tr("Descendants"), "descendants")
                        << ComboBoxItem(tr("Unfiltered"), "");
        mapSetting(addComboBox(QStringList("|LINEAGE|%1|current"), lineageFilterChoices),
                   settings.stringPointer(FossilSettings::timelineLineageFilterKey));
    }

    void addVerboseToggleButton()
    {
        VcsBase::VcsBaseClientSettings &settings = m_client->settings();

        // show files
        mapSetting(addToggleButton("-showfiles", tr("Verbose"),
                                   tr("Show files changed in each revision")),
                   settings.boolPointer(FossilSettings::timelineVerboseKey));
    }

    void addItemTypeComboBox()
    {
        VcsBase::VcsBaseClientSettings &settings = m_client->settings();

        // option: -t <val>
        const QList<ComboBoxItem> itemTypeChoices = {
            ComboBoxItem(tr("All Items"), "all"),
            ComboBoxItem(tr("File Commits"), "ci"),
            ComboBoxItem(tr("Technical Notes"), "e"),
            ComboBoxItem(tr("Tags"), "g"),
            ComboBoxItem(tr("Tickets"), "t"),
            ComboBoxItem(tr("Wiki Commits"), "w")
        };

        // here we setup the ComboBox to map to the "-t <val>", which will produce
        // the enquoted option-values (e.g "-t all").
        // Fossil expects separate arguments for option and value ( i.e. "-t" "all")
        // so we need to handle the splitting explicitly in arguments().
        mapSetting(addComboBox(QStringList("-t %1"), itemTypeChoices),
                   settings.stringPointer(FossilSettings::timelineItemTypeKey));
    }

    QStringList arguments() const final
    {
        QStringList args;

        // split "-t val" => "-t" "val"
        foreach (const QString &arg, VcsBaseEditorConfig::arguments()) {
            if (arg.startsWith("-t")) {
                args << arg.split(' ');

            } else if (arg.startsWith('|')){
                // meta-option: "|OPT|val|extra1|..."
                QStringList params = arg.split('|');
                QString option = params[1];
                for (int i = 2; i < params.size(); ++i) {
                    if (option == "LINEAGE" && params[i].isEmpty()) {
                        // empty lineage filter == Unfiltered
                        break;
                    }
                    args << params[i];
                }

            } else {
                args << arg;
            }
        }

        return args;
    }

private:
    FossilClient *m_client;
};

unsigned FossilClient::makeVersionNumber(int major, int minor, int patch)
{
    return (QString().setNum(major).toUInt(0,16) << 16) +
           (QString().setNum(minor).toUInt(0,16) << 8) +
           (QString().setNum(patch).toUInt(0,16));
}

static inline QString versionPart(unsigned part)
{
    return QString::number(part & 0xff, 16);
}

QString FossilClient::makeVersionString(unsigned version)
{
    return QString::fromLatin1("%1.%2.%3")
                    .arg(versionPart(version >> 16))
                    .arg(versionPart(version >> 8))
                    .arg(versionPart(version));
}

FossilClient::FossilClient() : VcsBase::VcsBaseClient(new FossilSettings)
{
    setDiffConfigCreator([this](QToolBar *toolBar) {
        return new FossilDiffConfig(this, toolBar);
    });
}

unsigned int FossilClient::synchronousBinaryVersion() const
{
    if (settings().binaryPath().isEmpty())
        return 0;

    QStringList args("version");

    const Utils::SynchronousProcessResponse response = vcsFullySynchronousExec(QString(), args);
    if (response.result != Utils::SynchronousProcessResponse::Finished)
        return 0;

    QString output = response.stdOut();
    output = output.trimmed();

    // fossil version:
    // "This is fossil version 1.27 [ccdefa355b] 2013-09-30 11:47:18 UTC"
    QRegularExpression versionPattern("(\\d+)\\.(\\d+)");
    QTC_ASSERT(versionPattern.isValid(), return 0);
    QRegularExpressionMatch versionMatch = versionPattern.match(output);
    QTC_ASSERT(versionMatch.hasMatch(), return 0);
    const int major = versionMatch.captured(1).toInt();
    const int minor = versionMatch.captured(2).toInt();
    const int patch = 0;
    return makeVersionNumber(major,minor,patch);
}

QList<BranchInfo> FossilClient::branchListFromOutput(const QString &output, const BranchInfo::BranchFlags defaultFlags)
{
    // Branch list format:
    // "  branch-name"
    // "* current-branch"
    return Utils::transform(output.split('\n', QString::SkipEmptyParts), [=](const QString& l) {
        const QString &name = l.mid(2);
        QTC_ASSERT(!name.isEmpty(), return BranchInfo());
        const BranchInfo::BranchFlags flags = (l.startsWith("* ") ? defaultFlags | BranchInfo::Current : defaultFlags);
        return BranchInfo(name, flags);
    });
}

BranchInfo FossilClient::synchronousCurrentBranch(const QString &workingDirectory)
{
    if (workingDirectory.isEmpty())
        return BranchInfo();

    // First try to get the current branch from the list of open branches
    const Utils::SynchronousProcessResponse response = vcsFullySynchronousExec(workingDirectory, {"branch", "list"});
    if (response.result != Utils::SynchronousProcessResponse::Finished)
        return BranchInfo();

    const QString output = sanitizeFossilOutput(response.stdOut());
    BranchInfo currentBranch = Utils::findOrDefault(branchListFromOutput(output), [](const BranchInfo &b) {
        return b.isCurrent();
    });

    if (!currentBranch.isCurrent()) {
        // If not available from open branches, request it from the list of closed branches.
        const Utils::SynchronousProcessResponse response = vcsFullySynchronousExec(workingDirectory, {"branch", "list", "--closed"});
        if (response.result != Utils::SynchronousProcessResponse::Finished)
            return BranchInfo();

        const QString output = sanitizeFossilOutput(response.stdOut());
        currentBranch = Utils::findOrDefault(branchListFromOutput(output, BranchInfo::Closed), [](const BranchInfo &b) {
            return b.isCurrent();
        });
    }

    return currentBranch;
}

QList<BranchInfo> FossilClient::synchronousBranchQuery(const QString &workingDirectory)
{
    // Return a list of all branches, including the closed ones.
    // Sort the list by branch name.

    if (workingDirectory.isEmpty())
        return QList<BranchInfo>();

    // First get list of open branches
    Utils::SynchronousProcessResponse response = vcsFullySynchronousExec(workingDirectory, {"branch", "list"});
    if (response.result != Utils::SynchronousProcessResponse::Finished)
        return QList<BranchInfo>();

    QString output = sanitizeFossilOutput(response.stdOut());
    QList<BranchInfo> branches = branchListFromOutput(output);

    // Append a list of closed branches.
    response = vcsFullySynchronousExec(workingDirectory, {"branch", "list", "--closed"});
    if (response.result != Utils::SynchronousProcessResponse::Finished)
        return QList<BranchInfo>();

    output = sanitizeFossilOutput(response.stdOut());
    branches.append(branchListFromOutput(output, BranchInfo::Closed));

    std::sort(branches.begin(), branches.end(),
          [](const BranchInfo &a, const BranchInfo &b) { return a.name() < b.name(); });
    return branches;
}

RevisionInfo FossilClient::synchronousRevisionQuery(const QString &workingDirectory, const QString &id)
{
    // Query details of the given revision/check-out id,
    // if none specified, provide information about current revision
    if (workingDirectory.isEmpty())
        return RevisionInfo();

    QStringList args("info");
    if (!id.isEmpty())
        args << id;

    const Utils::SynchronousProcessResponse response = vcsFullySynchronousExec(workingDirectory, args);
    if (response.result != Utils::SynchronousProcessResponse::Finished)
        return RevisionInfo();

    const QString output = sanitizeFossilOutput(response.stdOut());

    QString revisionId;
    QString parentId;

    const QRegularExpression idRx("([0-9a-f]{5,40})");
    QTC_ASSERT(idRx.isValid(), return RevisionInfo());

    for (const QString &l : output.split('\n', QString::SkipEmptyParts)) {
        if (l.startsWith("checkout: ", Qt::CaseInsensitive)
            || l.startsWith("uuid: ", Qt::CaseInsensitive)) {
            const QRegularExpressionMatch idMatch = idRx.match(l);
            QTC_ASSERT(idMatch.hasMatch(), return RevisionInfo());
            revisionId = idMatch.captured(1);

        } else if (l.startsWith("parent: ", Qt::CaseInsensitive)){
            const QRegularExpressionMatch idMatch = idRx.match(l);
            if (idMatch.hasMatch())
                parentId = idMatch.captured(1);
        }
    }

    // make sure id at least partially matches the retrieved revisionId
    QTC_ASSERT(revisionId.startsWith(id, Qt::CaseInsensitive), return RevisionInfo());

    if (parentId.isEmpty())
        parentId = revisionId;  // root

    return RevisionInfo(revisionId, parentId);
}

QStringList FossilClient::synchronousTagQuery(const QString &workingDirectory, const QString &id)
{
    // Return a list of tags for the given revision.
    // If no revision specified, all defined tags are listed.
    // Tag list includes branch names.

    if (workingDirectory.isEmpty())
        return QStringList();

    QStringList args({"tag", "list"});

    if (!id.isEmpty())
        args << id;

    const Utils::SynchronousProcessResponse response = vcsFullySynchronousExec(workingDirectory, args);
    if (response.result != Utils::SynchronousProcessResponse::Finished)
        return QStringList();

    const QString output = sanitizeFossilOutput(response.stdOut());

    return output.split('\n', QString::SkipEmptyParts);
}

RepositorySettings FossilClient::synchronousSettingsQuery(const QString &workingDirectory)
{
    if (workingDirectory.isEmpty())
        return RepositorySettings();

    RepositorySettings repoSettings;

    repoSettings.user = synchronousUserDefaultQuery(workingDirectory);
    if (repoSettings.user.isEmpty())
        repoSettings.user = settings().stringValue(FossilSettings::userNameKey);

    const QStringList args("settings");

    const Utils::SynchronousProcessResponse response = vcsFullySynchronousExec(workingDirectory, args);
    if (response.result != Utils::SynchronousProcessResponse::Finished)
        return RepositorySettings();

    const QString output = sanitizeFossilOutput(response.stdOut());

    for (const QString &line : output.split('\n', QString::SkipEmptyParts)) {
        // parse settings line:
        // <property> <(local|global)> <value>
        // Fossil properties are case-insensitive; force them to lower-case.
        // Values may be in mixed-case; force lower-case for fixed values.
        const QStringList fields = line.split(' ', QString::SkipEmptyParts);

        const QString property = fields.at(0).toLower();
        const QString value = (fields.size() >= 3 ? fields.at(2) : QString());
        const QString lcValue = value.toLower();

        if (property == "autosync") {
            if (lcValue == "on"
                || lcValue == "1")
                repoSettings.autosync = RepositorySettings::AutosyncOn;
            else if (lcValue == "off"
                     || lcValue == "0")
                repoSettings.autosync = RepositorySettings::AutosyncOff;
            else if (lcValue == "pullonly"
                     || lcValue == "2")
                repoSettings.autosync = RepositorySettings::AutosyncPullOnly;
        }
        else if (property == "ssl-identity") {
            repoSettings.sslIdentityFile = value;
        }
    }

    return repoSettings;
}

bool FossilClient::synchronousSetSetting(const QString &workingDirectory,
                                         const QString &property, const QString &value, bool isGlobal)
{
    // set a repository property to the given value
    // if no value is given, unset the property

    if (workingDirectory.isEmpty() || property.isEmpty())
        return false;

    QStringList args;
    if (value.isEmpty())
        args << "unset" << property;
    else
        args << "settings" << property << value;

    if (isGlobal)
        args << "--global";

    const Utils::SynchronousProcessResponse response = vcsFullySynchronousExec(workingDirectory, args);
    return (response.result == Utils::SynchronousProcessResponse::Finished);
}


bool FossilClient::synchronousConfigureRepository(const QString &workingDirectory, const RepositorySettings &newSettings,
                                                  const RepositorySettings &currentSettings)
{
    if (workingDirectory.isEmpty())
        return false;

    // apply updated settings vs. current setting if given
    const bool applyAll = (currentSettings == RepositorySettings());

    if (!newSettings.user.isEmpty()
        && (applyAll
            || newSettings.user != currentSettings.user)
        && !synchronousSetUserDefault(workingDirectory, newSettings.user)){
        return false;
    }

    if ((applyAll
         || newSettings.sslIdentityFile != currentSettings.sslIdentityFile)
        && !synchronousSetSetting(workingDirectory, "ssl-identity", newSettings.sslIdentityFile)){
        return false;
    }

    if (applyAll
        || newSettings.autosync != currentSettings.autosync) {
        QString value;
        switch (newSettings.autosync) {
        case RepositorySettings::AutosyncOff:
            value = "off";
            break;
        case RepositorySettings::AutosyncOn:
            value = "on";
            break;
        case RepositorySettings::AutosyncPullOnly:
            value = "pullonly";
            break;
        }

        if (!synchronousSetSetting(workingDirectory, "autosync", value))
            return false;
    }

    return true;
}

QString FossilClient::synchronousUserDefaultQuery(const QString &workingDirectory)
{
    if (workingDirectory.isEmpty())
        return QString();

    const QStringList args({"user", "default"});

    const Utils::SynchronousProcessResponse response = vcsFullySynchronousExec(workingDirectory, args);
    if (response.result != Utils::SynchronousProcessResponse::Finished)
        return QString();

    QString output = sanitizeFossilOutput(response.stdOut());

    return output.trimmed();
}

bool FossilClient::synchronousSetUserDefault(const QString &workingDirectory, const QString &userName)
{
    if (workingDirectory.isEmpty() || userName.isEmpty())
        return false;

    // set repository-default user
    const QStringList args({"user", "default", userName, "--user", userName});
    const Utils::SynchronousProcessResponse response = vcsFullySynchronousExec(workingDirectory, args);
    return (response.result == Utils::SynchronousProcessResponse::Finished);
}

QString FossilClient::synchronousGetRepositoryURL(const QString &workingDirectory)
{
    if (workingDirectory.isEmpty())
        return QString();

    const QStringList args("remote-url");

    const Utils::SynchronousProcessResponse response = vcsFullySynchronousExec(workingDirectory, args);
    if (response.result != Utils::SynchronousProcessResponse::Finished)
        return QString();

    QString output = sanitizeFossilOutput(response.stdOut());
    output = output.trimmed();

    // Fossil returns "off" when no remote-url is set.
    if (output.isEmpty() || output.toLower() == "off")
        return QString();

    return output;
}

QString FossilClient::synchronousTopic(const QString &workingDirectory)
{
    if (workingDirectory.isEmpty())
        return QString();

    // return current branch name

    const BranchInfo branchInfo = synchronousCurrentBranch(workingDirectory);
    if (branchInfo.name().isEmpty())
        return QString();

    return branchInfo.name();
}

bool FossilClient::synchronousCreateRepository(const QString &workingDirectory, const QStringList &extraOptions)
{
    VcsBase::VcsOutputWindow *outputWindow = VcsBase::VcsOutputWindow::instance();

    // init repository file of the same name as the working directory
    // use the configured default repository location for path
    // use the configured default user for admin

    const QString repoName = QDir(workingDirectory).dirName().simplified();
    const QString repoPath = settings().stringValue(FossilSettings::defaultRepoPathKey);
    const QString adminUser = settings().stringValue(FossilSettings::userNameKey);

    if (repoName.isEmpty() || repoPath.isEmpty())
        return false;

    // @TODO: handle spaces in the path
    // @TODO: what about --template options?

    const Utils::FileName repoFilePath = Utils::FileName::fromString(repoPath)
            .appendPath(Utils::FileName::fromString(repoName, Constants::FOSSIL_FILE_SUFFIX).toString());
    QStringList args(vcsCommandString(CreateRepositoryCommand));
    if (!adminUser.isEmpty())
        args << "--admin-user" << adminUser;
    args << extraOptions << repoFilePath.toUserOutput();
    Utils::SynchronousProcessResponse response = vcsFullySynchronousExec(workingDirectory, args);
    if (response.result != Utils::SynchronousProcessResponse::Finished)
        return false;

    QString output = sanitizeFossilOutput(response.stdOut());
    outputWindow->append(output);

    // check out the created repository file into the working directory

    args.clear();
    response.clear();
    output.clear();

    args << "open" << repoFilePath.toUserOutput();
    response = vcsFullySynchronousExec(workingDirectory, args);
    if (response.result != Utils::SynchronousProcessResponse::Finished)
        return false;

    output = sanitizeFossilOutput(response.stdOut());
    outputWindow->append(output);

    // set user default to admin if specified

    if (!adminUser.isEmpty()) {
        args.clear();
        response.clear();
        output.clear();

        args << "user" << "default" << adminUser << "--user" << adminUser;
        response = vcsFullySynchronousExec(workingDirectory, args);
        if (response.result != Utils::SynchronousProcessResponse::Finished)
            return false;

        QString output = sanitizeFossilOutput(response.stdOut());
        outputWindow->append(output);
    }

    resetCachedVcsInfo(workingDirectory);

    return true;
}

bool FossilClient::synchronousMove(const QString &workingDir,
                                   const QString &from, const QString &to,
                                   const QStringList &extraOptions)
{
    // Fossil move does not rename actual file on disk, only changes it in repo
    // So try to move the actual file first, then move it in repo to preserve
    // history in case actual move fails.

    if (!QFile::rename(from, to))
        return false;

    QStringList args(vcsCommandString(MoveCommand));
    args << extraOptions << from << to;
    const Utils::SynchronousProcessResponse response = vcsFullySynchronousExec(workingDir, args);
    return (response.result == Utils::SynchronousProcessResponse::Finished);
}

bool FossilClient::synchronousPull(const QString &workingDir, const QString &srcLocation, const QStringList &extraOptions)
{
    const QString remoteLocation = (!srcLocation.isEmpty() ? srcLocation : synchronousGetRepositoryURL(workingDir));
    if (remoteLocation.isEmpty())
        return false;

    QStringList args({vcsCommandString(PullCommand), remoteLocation});
    args << extraOptions;
    // Disable UNIX terminals to suppress SSH prompting
    const unsigned flags =
            VcsBase::VcsCommand::SshPasswordPrompt
            | VcsBase::VcsCommand::ShowStdOut
            | VcsBase::VcsCommand::ShowSuccessMessage;
    const Utils::SynchronousProcessResponse resp = vcsSynchronousExec(workingDir, args, flags);
    const bool success = (resp.result == Utils::SynchronousProcessResponse::Finished);
    if (success)
        emit changed(QVariant(workingDir));
    return success;
}

bool FossilClient::synchronousPush(const QString &workingDir, const QString &dstLocation, const QStringList &extraOptions)
{
    const QString remoteLocation = (!dstLocation.isEmpty() ? dstLocation : synchronousGetRepositoryURL(workingDir));
    if (remoteLocation.isEmpty())
        return false;

    QStringList args({vcsCommandString(PushCommand), remoteLocation});
    args << extraOptions;
    // Disable UNIX terminals to suppress SSH prompting
    const unsigned flags =
            VcsBase::VcsCommand::SshPasswordPrompt
            | VcsBase::VcsCommand::ShowStdOut
            | VcsBase::VcsCommand::ShowSuccessMessage;
    const Utils::SynchronousProcessResponse resp = vcsSynchronousExec(workingDir, args, flags);
    return (resp.result == Utils::SynchronousProcessResponse::Finished);
}

void FossilClient::commit(const QString &repositoryRoot, const QStringList &files,
                          const QString &commitMessageFile, const QStringList &extraOptions)
{
    VcsBaseClient::commit(repositoryRoot, files, commitMessageFile,
                          QStringList(extraOptions) << "-M" << commitMessageFile);
}

VcsBase::VcsBaseEditorWidget *FossilClient::annotate(
        const QString &workingDir, const QString &file, const QString &revision,
        int lineNumber, const QStringList &extraOptions)
{
    // 'fossil annotate' command has a variant 'fossil blame'.
    // blame command attributes a committing username to source lines,
    // annotate shows line numbers

    QString vcsCmdString = vcsCommandString(AnnotateCommand);
    const Core::Id kind = vcsEditorKind(AnnotateCommand);
    const QString id = VcsBase::VcsBaseEditor::getTitleId(workingDir, QStringList(file), revision);
    const QString title = vcsEditorTitle(vcsCmdString, id);
    const QString source = VcsBase::VcsBaseEditor::getSource(workingDir, file);

    VcsBase::VcsBaseEditorWidget *editor = createVcsEditor(kind, title, source,
                                                  VcsBase::VcsBaseEditor::getCodec(source),
                                                  vcsCmdString.toLatin1().constData(), id);

    auto *fossilEditor = qobject_cast<FossilEditorWidget *>(editor);
    QTC_ASSERT(fossilEditor, return editor);

    if (!fossilEditor->editorConfig()) {
        if (VcsBase::VcsBaseEditorConfig *editorConfig = createAnnotateEditor(fossilEditor)) {
            editorConfig->setBaseArguments(extraOptions);
            // editor has been just created, createVcsEditor() didn't set a configuration widget yet
            connect(editorConfig, &VcsBase::VcsBaseEditorConfig::commandExecutionRequested,
                    [=]() {
                        const int line = VcsBase::VcsBaseEditor::lineNumberOfCurrentEditor();
                        return this->annotate(workingDir, file, revision, line, editorConfig->arguments());
                    } );
            fossilEditor->setEditorConfig(editorConfig);
        }
    }
    QStringList effectiveArgs = extraOptions;
    if (VcsBase::VcsBaseEditorConfig *editorConfig = fossilEditor->editorConfig())
        effectiveArgs = editorConfig->arguments();

    VcsBase::VcsCommand *cmd = createCommand(workingDir, fossilEditor);

    // here we introduce a "|BLAME|" meta-option to allow both annotate and blame modes
    int pos = effectiveArgs.indexOf("|BLAME|");
    if (pos != -1) {
        vcsCmdString = "blame";
        effectiveArgs.removeAt(pos);
    }
    QStringList args(vcsCmdString);
    if (!revision.isEmpty()
        && supportedFeatures().testFlag(AnnotateRevisionFeature))
        args << "-r" << revision;

    args << effectiveArgs << file;

    // When version list requested, ignore the source line.
    if (args.contains("--log"))
        lineNumber = -1;
    cmd->setCookie(lineNumber);

    enqueueJob(cmd, args);
    return fossilEditor;
}

bool FossilClient::isVcsFileOrDirectory(const Utils::FileName &fileName) const
{
    // false for any dir or file other than fossil checkout db-file
    return fileName.toFileInfo().isFile()
           && !fileName.fileName().compare(Constants::FOSSILREPO,
                                           Utils::HostOsInfo::fileNameCaseSensitivity());
}

QString FossilClient::findTopLevelForFile(const QFileInfo &file) const
{
    const QString repositoryCheckFile = Constants::FOSSILREPO;
    return file.isDir() ?
                VcsBase::VcsBasePlugin::findRepositoryForDirectory(file.absoluteFilePath(),
                                                                   repositoryCheckFile) :
                VcsBase::VcsBasePlugin::findRepositoryForDirectory(file.absolutePath(),
                                                                   repositoryCheckFile);
}

bool FossilClient::managesFile(const QString &workingDirectory, const QString &fileName) const
{
    const QStringList args({"finfo", fileName});
    const Utils::SynchronousProcessResponse response = vcsFullySynchronousExec(workingDirectory, args);
    if (response.result != Utils::SynchronousProcessResponse::Finished)
        return false;
    QString output = sanitizeFossilOutput(response.stdOut());
    return !output.startsWith("no history for file", Qt::CaseInsensitive);
}

unsigned int FossilClient::binaryVersion() const
{
    static unsigned int cachedBinaryVersion = 0;
    static QString cachedBinaryPath;

    const QString currentBinaryPath = settings().binaryPath().toString();

    if (currentBinaryPath.isEmpty())
        return 0;

    // Invalidate cache on failed version result.
    // Assume that fossil client options have been changed and will change again.
    if (!cachedBinaryVersion
        || currentBinaryPath != cachedBinaryPath) {
        cachedBinaryVersion = synchronousBinaryVersion();
        if (cachedBinaryVersion)
            cachedBinaryPath = currentBinaryPath;
        else
            cachedBinaryPath.clear();
    }

    return cachedBinaryVersion;
}

QString FossilClient::binaryVersionString() const
{
    const unsigned int version = binaryVersion();

    // Fossil itself does not report patch version, only maj.min
    // Here we include the patch part for general convention consistency

    return makeVersionString(version);
}

FossilClient::SupportedFeatures FossilClient::supportedFeatures() const
{
    // use for legacy client support to test for feature presence
    // e.g. supportedFeatures().testFlag(TimelineWidthFeature)

    SupportedFeatures features = AllSupportedFeatures; // all inclusive by default (~0U)

    const unsigned int version = binaryVersion();

    if (version < 0x20400) {
        features &= ~AnnotateRevisionFeature;
        if (version < 0x13000)
            features &= ~TimelinePathFeature;
        if (version < 0x12900)
            features &= ~DiffIgnoreWhiteSpaceFeature;
        if (version < 0x12800) {
            features &= ~AnnotateBlameFeature;
            features &= ~TimelineWidthFeature;
        }
    }

    return features;
}

void FossilClient::view(const QString &source, const QString &id, const QStringList &extraOptions)
{
    QStringList args("diff");

    const QFileInfo fi(source);
    const QString workingDirectory = fi.isFile() ? fi.absolutePath() : source;

    RevisionInfo revisionInfo = synchronousRevisionQuery(workingDirectory,id);

    args << "--from" << revisionInfo.parentId
         << "--to" << revisionInfo.id
         << "-v"
         << extraOptions;

    const Core::Id kind = vcsEditorKind(DiffCommand);
    const QString title = vcsEditorTitle(vcsCommandString(DiffCommand), id);

    VcsBase::VcsBaseEditorWidget *editor = createVcsEditor(kind, title, source,
                                                           VcsBase::VcsBaseEditor::getCodec(source), "view", id);
    editor->setWorkingDirectory(workingDirectory);

    enqueueJob(createCommand(workingDirectory, editor), args);
}

class FossilLogHighlighter : QSyntaxHighlighter
{
public:
    explicit FossilLogHighlighter(QTextDocument *parent);
    virtual void highlightBlock(const QString &text) final;

private:
    const QRegularExpression m_revisionIdRx;
    const QRegularExpression m_dateRx;
};

FossilLogHighlighter::FossilLogHighlighter(QTextDocument * parent) :
    QSyntaxHighlighter(parent),
    m_revisionIdRx(Constants::CHANGESET_ID),
    m_dateRx("([0-9]{4}-[0-9]{2}-[0-9]{2})")
{
    QTC_CHECK(m_revisionIdRx.isValid());
    QTC_CHECK(m_dateRx.isValid());
}

void FossilLogHighlighter::highlightBlock(const QString &text)
{
    // Match the revision-ids and dates -- highlight them for convenience.

    // Format revision-ids
    QRegularExpressionMatchIterator i = m_revisionIdRx.globalMatch(text);
    while (i.hasNext()) {
        const QRegularExpressionMatch revisionIdMatch = i.next();
        QTextCharFormat charFormat = format(0);
        charFormat.setForeground(Qt::darkBlue);
        //charFormat.setFontItalic(true);
        setFormat(revisionIdMatch.capturedStart(0), revisionIdMatch.capturedLength(0), charFormat);
    }

    // Format dates
    i = m_dateRx.globalMatch(text);
    while (i.hasNext()) {
        const QRegularExpressionMatch dateMatch = i.next();
        QTextCharFormat charFormat = format(0);
        charFormat.setForeground(Qt::darkBlue);
        charFormat.setFontWeight(QFont::DemiBold);
        setFormat(dateMatch.capturedStart(0), dateMatch.capturedLength(0), charFormat);
    }
}

void FossilClient::log(const QString &workingDir, const QStringList &files,
                       const QStringList &extraOptions,
                       bool enableAnnotationContextMenu)
{
    // Show timeline for both repository and a file or path (--path <file-or-path>)
    // When used for log repository, the files list is empty

    // LEGACY:fallback to log current file with legacy clients
    SupportedFeatures features = supportedFeatures();
    if (!files.isEmpty()
        && !features.testFlag(TimelinePathFeature)) {
        logCurrentFile(workingDir, files, extraOptions, enableAnnotationContextMenu);
        return;
    }

    const QString vcsCmdString = vcsCommandString(LogCommand);
    const Core::Id kind = vcsEditorKind(LogCommand);
    const QString id = VcsBase::VcsBaseEditor::getTitleId(workingDir, files);
    const QString title = vcsEditorTitle(vcsCmdString, id);
    const QString source = VcsBase::VcsBaseEditor::getSource(workingDir, files);
    VcsBase::VcsBaseEditorWidget *editor = createVcsEditor(kind, title, source,
                                                           VcsBase::VcsBaseEditor::getCodec(source),
                                                           vcsCmdString.toLatin1().constData(), id);

    auto *fossilEditor = qobject_cast<FossilEditorWidget *>(editor);
    QTC_ASSERT(fossilEditor, return);

    fossilEditor->setFileLogAnnotateEnabled(enableAnnotationContextMenu);

    if (!fossilEditor->editorConfig()) {
        if (VcsBase::VcsBaseEditorConfig *editorConfig = createLogEditor(fossilEditor)) {
            editorConfig->setBaseArguments(extraOptions);
            // editor has been just created, createVcsEditor() didn't set a configuration widget yet
            connect(editorConfig, &VcsBase::VcsBaseEditorConfig::commandExecutionRequested,
                [=]() { this->log(workingDir, files, editorConfig->arguments(), enableAnnotationContextMenu); } );
            fossilEditor->setEditorConfig(editorConfig);
        }
    }
    QStringList effectiveArgs = extraOptions;
    if (VcsBase::VcsBaseEditorConfig *editorConfig = fossilEditor->editorConfig())
        effectiveArgs = editorConfig->arguments();

    //@TODO: move highlighter and widgets to fossil editor sources.

    new FossilLogHighlighter(fossilEditor->document());

    QStringList args(vcsCmdString);
    args << effectiveArgs;
    if (!files.isEmpty())
         args << "--path" << files;
    enqueueJob(createCommand(workingDir, fossilEditor), args);
}

void FossilClient::logCurrentFile(const QString &workingDir, const QStringList &files,
                                  const QStringList &extraOptions,
                                  bool enableAnnotationContextMenu)
{
    // Show commit history for the given file/file-revision
    // NOTE: 'fossil finfo' shows full history from all branches.

    // With newer clients, 'fossil timeline' can handle both repository and file
    SupportedFeatures features = supportedFeatures();
    if (features.testFlag(TimelinePathFeature)) {
        log(workingDir, files, extraOptions, enableAnnotationContextMenu);
        return;
    }

    const QString vcsCmdString = "finfo";
    const Core::Id kind = vcsEditorKind(LogCommand);
    const QString id = VcsBase::VcsBaseEditor::getTitleId(workingDir, files);
    const QString title = vcsEditorTitle(vcsCmdString, id);
    const QString source = VcsBase::VcsBaseEditor::getSource(workingDir, files);
    VcsBase::VcsBaseEditorWidget *editor = createVcsEditor(kind, title, source,
                                                           VcsBase::VcsBaseEditor::getCodec(source),
                                                           vcsCmdString.toLatin1().constData(), id);

    auto *fossilEditor = qobject_cast<FossilEditorWidget *>(editor);
    QTC_ASSERT(fossilEditor, return);

    fossilEditor->setFileLogAnnotateEnabled(enableAnnotationContextMenu);

    if (!fossilEditor->editorConfig()) {
        if (VcsBase::VcsBaseEditorConfig *editorConfig = createLogEditor(fossilEditor)) {
            editorConfig->setBaseArguments(extraOptions);
            // editor has been just created, createVcsEditor() didn't set a configuration widget yet
            connect(editorConfig, &VcsBase::VcsBaseEditorConfig::commandExecutionRequested,
                [=]() { this->logCurrentFile(workingDir, files, editorConfig->arguments(), enableAnnotationContextMenu); } );
            fossilEditor->setEditorConfig(editorConfig);
        }
    }
    QStringList effectiveArgs = extraOptions;
    if (VcsBase::VcsBaseEditorConfig *editorConfig = fossilEditor->editorConfig())
        effectiveArgs = editorConfig->arguments();

    //@TODO: move highlighter and widgets to fossil editor sources.

    new FossilLogHighlighter(fossilEditor->document());

    QStringList args(vcsCmdString);
    args << effectiveArgs << files;
    enqueueJob(createCommand(workingDir, fossilEditor), args);
}

void FossilClient::revertFile(const QString &workingDir,
                              const QString &file,
                              const QString &revision,
                              const QStringList &extraOptions)
{
    QStringList args(vcsCommandString(RevertCommand));
    if (!revision.isEmpty())
        args << "-r" << revision;

    args << extraOptions << file;

    // Indicate file list
    VcsBase::VcsCommand *cmd = createCommand(workingDir);
    cmd->setCookie(QStringList(workingDir + "/" + file));
    connect(cmd, &VcsBase::VcsCommand::success, this, &VcsBase::VcsBaseClient::changed, Qt::QueuedConnection);
    enqueueJob(cmd, args);
}

void FossilClient::revertAll(const QString &workingDir, const QString &revision, const QStringList &extraOptions)
{
    // Fossil allows whole tree revert to latest revision (effectively undoing uncommitted changes).
    // However it disallows revert to a specific revision for the whole tree, only for selected files.
    // Use checkout --force command for such case.
    // NOTE: all uncommitted changes will not be backed up by checkout, unlike revert.
    //       Thus undo for whole tree revert should not be possible.

    QStringList args;
    if (revision.isEmpty()) {
        args << vcsCommandString(RevertCommand)
             << extraOptions;

    } else {
        args << "checkout" << revision
             << "--force"
             << extraOptions;
    }

    // Indicate repository change
    VcsBase::VcsCommand *cmd = createCommand(workingDir);
    cmd->setCookie(QStringList(workingDir));
    connect(cmd, &VcsBase::VcsCommand::success, this, &VcsBase::VcsBaseClient::changed, Qt::QueuedConnection);
    enqueueJob(createCommand(workingDir), args);
}

QString FossilClient::sanitizeFossilOutput(const QString &output) const
{
#if defined(Q_OS_WIN) || defined(Q_OS_CYGWIN)
    // Strip possible extra '\r' in output from the Fossil client on Windows.

    // Fossil client contained a long-standing bug which caused an extraneous '\r'
    // added to output lines from certain commands in excess of the expected <CR/LF>.
    // While the output appeared normal on a terminal, in non-interactive context
    // it would get incorrectly split, resulting in extra empty lines.
    // Bug fix is fairly recent, so for compatibility we need to strip the '\r'.
    QString result(output);
    return result.remove('\r');
#else
    return output;
#endif
}

QString FossilClient::vcsCommandString(VcsCommandTag cmd) const
{
    // override specific client commands
    // otherwise return baseclient command

    switch (cmd) {
    case RemoveCommand: return QString("rm");
    case MoveCommand: return QString("mv");
    case LogCommand: return QString("timeline");

    default: return VcsBaseClient::vcsCommandString(cmd);
    }
}

Core::Id FossilClient::vcsEditorKind(VcsCommandTag cmd) const
{
    switch (cmd) {
    case AnnotateCommand:
        return Constants::ANNOTATELOG_ID;
    case DiffCommand:
        return Constants::DIFFLOG_ID;
    case LogCommand:
        return Constants::FILELOG_ID;
    default:
        return Core::Id();
    }
}

QStringList FossilClient::revisionSpec(const QString &revision) const
{
    // Pass the revision verbatim.
    // Fossil uses a variety of ways to spec the revisions.
    // In most cases revision is passed directly (SHA1) or via tag.
    // Tag name may need to be prefixed with tag: to disambiguate it from hex (beef).
    // Handle the revision option per specific command (e.g. diff, revert ).

    QStringList args;
    if (!revision.isEmpty())
        args << revision;
    return args;
}

FossilClient::StatusItem FossilClient::parseStatusLine(const QString &line) const
{
    StatusItem item;

    // Ref: fossil source 'src/checkin.c' status_report()
    // Expect at least one non-leading blank space.

    int pos = line.indexOf(' ');

    if (line.isEmpty() || pos < 1)
        return StatusItem();

    QString label(line.left(pos));
    QString flags;

    if (label == "EDITED")
        flags = Constants::FSTATUS_EDITED;
    else if (label == "ADDED")
        flags = Constants::FSTATUS_ADDED;
    else if (label == "RENAMED")
        flags = Constants::FSTATUS_RENAMED;
    else if (label == "DELETED")
        flags = Constants::FSTATUS_DELETED;
    else if (label == "MISSING")
        flags = "Missing";
    else if (label == "ADDED_BY_MERGE")
        flags = Constants::FSTATUS_ADDED_BY_MERGE;
    else if (label == "UPDATED_BY_MERGE")
        flags = Constants::FSTATUS_UPDATED_BY_MERGE;
    else if (label == "ADDED_BY_INTEGRATE")
        flags = Constants::FSTATUS_ADDED_BY_INTEGRATE;
    else if (label == "UPDATED_BY_INTEGRATE")
        flags = Constants::FSTATUS_UPDATED_BY_INTEGRATE;
    else if (label == "CONFLICT")
        flags = "Conflict";
    else if (label == "EXECUTABLE")
        flags = "Set Exec";
    else if (label == "SYMLINK")
        flags = "Set Symlink";
    else if (label == "UNEXEC")
        flags = "Unset Exec";
    else if (label == "UNLINK")
        flags = "Unset Symlink";
    else if (label == "NOT_A_FILE")
        flags = Constants::FSTATUS_UNKNOWN;


    if (flags.isEmpty())
        return StatusItem();

    // adjust the position to the last space before the file name
    for (int size = line.size(); (pos+1) < size && line[pos+1].isSpace(); ++pos) {}

    item.flags = flags;
    item.file = line.mid(pos + 1);

    return item;
}

VcsBase::VcsBaseEditorConfig *FossilClient::createAnnotateEditor(VcsBase::VcsBaseEditorWidget *editor)
{
    return new FossilAnnotateConfig(this, editor->toolBar());
}

VcsBase::VcsBaseEditorConfig *FossilClient::createLogCurrentFileEditor(VcsBase::VcsBaseEditorWidget *editor)
{
    SupportedFeatures features = supportedFeatures();

    if (features.testFlag(TimelinePathFeature))
        return createLogEditor(editor);

    return new FossilLogCurrentFileConfig(this, editor->toolBar());
}

VcsBase::VcsBaseEditorConfig *FossilClient::createLogEditor(VcsBase::VcsBaseEditorWidget *editor)
{
    return new FossilLogConfig(this, editor->toolBar());
}

} // namespace Internal
} // namespace Fossil

#include "fossilclient.moc"
