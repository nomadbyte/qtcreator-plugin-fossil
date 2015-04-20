/**************************************************************************
**  This file is part of Fossil VCS plugin for Qt Creator
**
**  Copyright (c) 2013 - 2015, Artur Shepilko, <qtc-fossil@nomadbyte.com>.
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

#ifndef FOSSILPLUGIN_H
#define FOSSILPLUGIN_H

#include "fossilsettings.h"

#include <vcsbase/vcsbaseclientsettings.h>
#include <vcsbase/vcsbaseclient.h>
#include <vcsbase/vcsbaseplugin.h>
#include <coreplugin/icontext.h>

#include <QtCore/QFileInfo>
#include <QtCore/QHash>
#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE
class QAction;
class QTemporaryFile;
QT_END_NAMESPACE

namespace Core {
class ActionManager;
class ActionContainer;
class ICore;
class Id;
class IVersionControl;
class IEditorFactory;
class IEditor;
} // namespace Core

namespace Utils {
class ParameterAction;
} // namespace Utils

namespace VCSBase {
class VCSBaseSubmitEditor;
}

namespace Locator {
class CommandLocator;
}

namespace Fossil {
namespace Internal {

class OptionsPage;
class FossilClient;
class FossilControl;
class FossilEditor;

class FossilPlugin : public VCSBase::VCSBasePlugin
{
    Q_OBJECT

public:
    FossilPlugin();
    ~FossilPlugin();
    bool initialize(const QStringList &arguments, QString *errorMessage);

    static FossilPlugin *instance();
    FossilClient *client() const;

    const FossilSettings &settings() const;
    void setSettings(const FossilSettings &settings);

private slots:
    // File menu action slots
    void addCurrentFile();
    void annotateCurrentFile();
    void diffCurrentFile();
    void logCurrentFile();
    void revertCurrentFile();
    void statusCurrentFile();

    // Directory menu action slots
    void diffRepository();
    void logRepository();
    void revertAll();
    void statusMulti();

    // Repository menu action slots
    void pull();
    void push();
    void update();
    void configureRepository();
    void commit();
    void showCommitWidget(const QList<VCSBase::VCSBaseClient::StatusItem> &status);
    void commitFromEditor();
    void diffFromEditorSelected(const QStringList &files);
    void createRepository();

protected:
    void updateActions(VCSBase::VCSBasePlugin::ActionState);
    bool submitEditorAboutToClose(VCSBase::VCSBaseSubmitEditor *submitEditor);

private:
    // Methods
    void createMenu();
    void createSubmitEditorActions();
    void createSeparator(const Core::Context &context, const Core::Id &id);
    void createFileActions(const Core::Context &context);
    void createDirectoryActions(const Core::Context &context);
    void createRepositoryActions(const Core::Context &context);
    void deleteCommitLog();

    // Variables
    static FossilPlugin *m_instance;
    FossilSettings m_settings;
    OptionsPage *m_optionsPage;
    FossilClient *m_client;

    Core::ICore *m_core;
    Locator::CommandLocator *m_commandLocator;
    Core::ActionManager *m_actionManager;
    Core::ActionContainer *m_fossilContainer;

    QList<QAction *> m_repositoryActionList;
    QTemporaryFile *m_changeLog;

    // Menu Items (file actions)
    Utils::ParameterAction *m_addAction;
    Utils::ParameterAction *m_deleteAction;
    Utils::ParameterAction *m_annotateFile;
    Utils::ParameterAction *m_diffFile;
    Utils::ParameterAction *m_logFile;
    Utils::ParameterAction *m_renameFile;
    Utils::ParameterAction *m_revertFile;
    Utils::ParameterAction *m_statusFile;

    // Submit editor actions
    QAction *m_editorCommit;
    QAction *m_editorDiff;
    QAction *m_editorUndo;
    QAction *m_editorRedo;
    QAction *m_menuAction;

    QString m_submitRepository;
};

} // namespace Internal
} // namespace Fossil

#endif // FOSSILPLUGIN_H
