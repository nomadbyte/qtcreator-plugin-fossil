/**************************************************************************
**  This file is part of Fossil VCS plugin for Qt Creator
**
**  Copyright (c) 2013, Artur Shepilko.
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
#include "constants.h"

#include <vcsbase/vcsbaseclientsettings.h>
#include <vcsbase/vcsbaseplugin.h>
#include <vcsbase/vcsbaseeditor.h>
#include <vcsbase/vcsbaseeditorparameterwidget.h>
#include <vcsbase/vcsbaseoutputwindow.h>
#include <vcsbase/command.h>

#include <utils/synchronousprocess.h>
#include <utils/qtcassert.h>

#include <QtGui/QSyntaxHighlighter>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QTextStream>
#include <QtCore/QtDebug>
#include <QtCore/QMap>

namespace Fossil {
namespace Internal {

QString FossilClient::buildPath(const QString &path, const QString &baseName, const QString &suffix)
{
    // Intended use: handle the path in POSIX format, convert via QDir::toNativeSeparators() at use site.
    const QChar separator(QLatin1Char('/'));

    QString result = path;
    if (!baseName.isEmpty()) {
        if (!result.isEmpty() && !result.endsWith(QDir::separator()))
            result += separator;
        result += baseName;
    }
    // Add suffix unless user specified something else
    const QChar dot = QLatin1Char('.');
    if (!suffix.isEmpty() && !baseName.contains(dot)) {
        if (!suffix.startsWith(dot))
            result += dot;
        result += suffix;
    }
    return result;
}

FossilClient::FossilClient(FossilSettings *settings)
    : VCSBase::VCSBaseClient(settings)
{
}

FossilSettings *FossilClient::settings() const
{
    return dynamic_cast<FossilSettings *>(VCSBase::VCSBaseClient::settings());
}

BranchInfo FossilClient::synchronousBranchQuery(const QString &workingDirectory, QList<BranchInfo> *allBranches)
{
    // Return details of the current branch.
    // When passed allBranches list arg, populate it with all branches.

    const BranchInfo nullBranch(QString(),false);
    const QString currentBranchIndicator(QLatin1String("*")); // '* current-branch'

    BranchInfo currentBranch(nullBranch);
    bool getAllBranches = false;
    bool getClosedBranches = false;
    QMap<QString, BranchInfo> branchesByName;

    if (allBranches != 0) {
        getAllBranches = true;
        allBranches->clear();
    }

    if (workingDirectory.isEmpty())
        return nullBranch;

    // - retrieve a list of branches, parse out branch names and current indicator
    // - query open branches first, if got no current, query the closed branches
    // - if requested, store the all branch info (both open and closed)

    do {

        QStringList args;
        args << QLatin1String("branch") << QLatin1String("list");

        if (getClosedBranches)
             args << QLatin1String("--closed");

        QByteArray outputData;
        if (!vcsFullySynchronousExec(workingDirectory, args, &outputData))
            return nullBranch;

        QString output = QString::fromLocal8Bit(outputData);
        output.remove(QLatin1Char('\r'));

        if (output.endsWith(QLatin1Char('\n')))
            output.truncate(output.size()-1);

        const QStringList lines = output.split(QLatin1Char('\n'));

        QStringList::const_iterator cend = lines.constEnd();
        for (QStringList::const_iterator cit = lines.constBegin(); cit != cend; ++cit) {
            QString line = *cit;
            if (line.isEmpty()) continue;

            // parse branch line:
            //   branch
            // * current-branch

            bool isCurrent = (line.startsWith(currentBranchIndicator));
            QString name = line.mid(2);
            bool isClosed = getClosedBranches;
            BranchInfo::BranchFlags flags = BranchInfo::Public
                    | (isClosed ? BranchInfo::Closed : BranchInfo::Open);

            if (name.isEmpty()) continue;

            BranchInfo branch(name, isCurrent, flags);

            if (isCurrent) {
                currentBranch = branch;

                if (!getAllBranches)
                    break;
            }

            if (getAllBranches)
                branchesByName.insert(name, branch);
        }

        getClosedBranches = (!getClosedBranches
                             && (getAllBranches || currentBranch.name().isEmpty()));

    } while (getClosedBranches);


    if (allBranches)
        *allBranches = branchesByName.values();

    return currentBranch;
}

RevisionInfo FossilClient::synchronousRevisionQuery(const QString &workingDirectory, const QString &id)
{
    // Query details of the given revision/check-out id,
    // if none specified, provide information about current revision
    if (workingDirectory.isEmpty())
        return RevisionInfo(QString(),QString());

    QStringList args;
    args << QLatin1String("info");
    if (!id.isEmpty())
        args << id;

    QByteArray outputData;
    if (!vcsFullySynchronousExec(workingDirectory, args, &outputData))
        return RevisionInfo(QString(),QString());
    QString output = QString::fromLocal8Bit(outputData);
    output.remove(QLatin1Char('\r'));

    if (output.endsWith(QLatin1Char('\n')))
        output.truncate(output.size()-1);

    QString revisionId;
    QString parentId;

    QRegExp revisionIdRx(QLatin1String("uuid:\\s*([0-9a-f]{5,40})"));
    const QRegExp currentRevisionIdRx(QLatin1String("checkout:\\s*([0-9a-f]{5,40})"));
    const QRegExp parentIdRx(QLatin1String("parent:\\s*([0-9a-f]{5,40})"));
    if (id.isEmpty())
        revisionIdRx = currentRevisionIdRx;
    QTC_ASSERT(revisionIdRx.isValid(), return RevisionInfo(QString(),QString()));
    QTC_ASSERT(parentIdRx.isValid(), return RevisionInfo(QString(),QString()));

    //NOTE: parent does not exist for the root (initial empty check-in)
    //      yet in this context we set parentId to its own revisionId

    if (revisionIdRx.indexIn(output) != -1) {
        revisionId = revisionIdRx.cap(1);

        if (parentIdRx.indexIn(output) != -1) {
            parentId = parentIdRx.cap(1);
        } else {
            parentId = revisionId;  // root
        }
    }

    // make sure id at least partially matches the retrieved revisionId
    QTC_ASSERT(revisionId.left(id.size()).compare(id,Qt::CaseInsensitive) == 0,
               return RevisionInfo(QString(),QString()));

    return RevisionInfo(revisionId, parentId);
}

QStringList FossilClient::synchronousTagQuery(const QString &workingDirectory, const QString &id)
{
    // Return a list of tags for the given revision.
    // If no revision specified, all defined tags are listed.
    // Tag list includes branch names.

    QStringList tags;

    if (workingDirectory.isEmpty())
        return QStringList();

    QStringList args;
    args << QLatin1String("tag") << QLatin1String("list");
    if (!id.isEmpty())
        args << id;

    QByteArray outputData;
    if (!vcsFullySynchronousExec(workingDirectory, args, &outputData))
        return QStringList();

    QString output = QString::fromLocal8Bit(outputData);
    output.remove(QLatin1Char('\r'));

    if (output.endsWith(QLatin1Char('\n')))
        output.truncate(output.size()-1);

    tags = output.split(QLatin1Char('\n'));

    return tags;
}

RepositorySettings FossilClient::synchronousSettingsQuery(const QString &workingDirectory)
{
    RepositorySettings repoSettings;

    if (workingDirectory.isEmpty())
        return RepositorySettings();

    repoSettings.user = synchronousUserDefaultQuery(workingDirectory);
    if (repoSettings.user.isEmpty())
        repoSettings.user = settings()->stringValue(FossilSettings::userNameKey);

    QStringList args;
    args << QLatin1String("settings");

    QByteArray outputData;
    if (!vcsFullySynchronousExec(workingDirectory, args, &outputData))
        return RepositorySettings();
    QString output = QString::fromLocal8Bit(outputData);
    output.remove(QLatin1Char('\r'));

    if (output.endsWith(QLatin1Char('\n')))
        output.truncate(output.size()-1);

    const QStringList lines = output.split(QLatin1Char('\n'));

    QStringList::const_iterator cend = lines.constEnd();
    for (QStringList::const_iterator cit = lines.constBegin(); cit != cend; ++cit) {
        QString line = *cit;
        if (line.isEmpty()) continue;

        // parse settings line:
        // <setting-name> <(local|global)> <value>
        const QChar spaceChar(QLatin1Char(' '));
        QStringList fields = line.split(spaceChar, QString::SkipEmptyParts);

        QString property = fields[0];
        QString value;
        if (fields.size() >= 3)
            value = fields[2];

        if (property == QLatin1String("autosync")) {
            if (value == QLatin1String("on")
                || value == QLatin1String("1"))
                repoSettings.autosync = RepositorySettings::AutosyncOn;
            else if (value == QLatin1String("off")
                     || value == QLatin1String("0"))
                     repoSettings.autosync = RepositorySettings::AutosyncOff;
            else if (value == QLatin1String("pullonly")
                     || value == QLatin1String("2"))
                     repoSettings.autosync = RepositorySettings::AutosyncPullOnly;
        }

        if (property == QLatin1String("ssl-identity")) {
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
    QStringList extraOptions;
    if (value.isEmpty())
        args << QLatin1String("unset") << property;
    else
        args << QLatin1String("settings") << property << value;

    if (isGlobal)
        extraOptions << QLatin1String("--global");

    args << extraOptions;

    QByteArray outputData;
    return vcsFullySynchronousExec(workingDirectory, args, &outputData);
}


bool FossilClient::synchronousConfigureRepository(const QString &workingDirectory, const RepositorySettings &newSettings,
                                                  const RepositorySettings &currentSettings)
{
    bool result = false;

    if (workingDirectory.isEmpty())
        return false;

    // apply updated settings vs. current setting if given
    bool applyAll = (currentSettings == RepositorySettings());

    if (!newSettings.user.isEmpty()
        && (applyAll
            || newSettings.user != currentSettings.user)) {
        result &= synchronousSetUserDefault(workingDirectory, newSettings.user);
    }

    if (applyAll
        || newSettings.sslIdentityFile != currentSettings.sslIdentityFile) {
        result &= synchronousSetSetting(workingDirectory,QLatin1String("ssl-identity"), newSettings.sslIdentityFile);
    }

    if (applyAll
        || newSettings.autosync != currentSettings.autosync) {
        QString value;
        switch (newSettings.autosync) {
        case RepositorySettings::AutosyncOff:
            value = QLatin1String("off");
            break;
        case RepositorySettings::AutosyncOn:
            value = QLatin1String("on");
            break;
        case RepositorySettings::AutosyncPullOnly:
            value = QLatin1String("pullonly");
            break;
        }

        result &= synchronousSetSetting(workingDirectory,QLatin1String("autosync"), value);
    }

    return result;
}

QString FossilClient::synchronousUserDefaultQuery(const QString &workingDirectory)
{
    if (workingDirectory.isEmpty())
        return QString();

    QStringList args;
    args << QLatin1String("user") << QLatin1String("default");

    QByteArray outputData;
    if (!vcsFullySynchronousExec(workingDirectory, args, &outputData))
        return QString();
    QString output = QString::fromLocal8Bit(outputData);
    output.remove(QLatin1Char('\r'));

    if (output.endsWith(QLatin1Char('\n')))
        output.truncate(output.size()-1);

    return output;
}

bool FossilClient::synchronousSetUserDefault(const QString &workingDirectory, const QString &userName)
{
    if (workingDirectory.isEmpty() || userName.isEmpty())
        return false;

    // set repository-default user
    QStringList args;
    args << QLatin1String("user") << QLatin1String("default") << userName
         << QLatin1String("--user") << userName;
    QByteArray outputData;
    return vcsFullySynchronousExec(workingDirectory, args, &outputData);
}

QString FossilClient::synchronousGetRepositoryURL(const QString &workingDirectory)
{
    if (workingDirectory.isEmpty())
        return QString();

    QStringList args;
    args << QLatin1String("remote-url");

    QByteArray outputData;
    if (!vcsFullySynchronousExec(workingDirectory, args, &outputData))
        return QString();
    QString output = QString::fromLocal8Bit(outputData);
    output.remove(QLatin1Char('\r'));

    if (output.endsWith(QLatin1Char('\n')))
        output.truncate(output.size()-1);

    // Fossil returns "off" when no remote-url is set.

    if (output.isEmpty() || output == QLatin1String("off"))
        return QString();

    return output;
}

bool FossilClient::synchronousCreateRepository(const QString &workingDirectory, const QStringList &extraOptions)
{
    VCSBase::VCSBaseOutputWindow *outputWindow = VCSBase::VCSBaseOutputWindow::instance();

    // init repository file of the same name as the working directory
    // use the configured default repository location for path
    // use the configured default user for admin

    QString repoName = QDir(workingDirectory).dirName().simplified();
    QString repoPath = settings()->stringValue(FossilSettings::defaultRepoPathKey);
    QString adminUser = settings()->stringValue(FossilSettings::userNameKey);

    if (repoName.isEmpty() || repoPath.isEmpty())
        return false;

    // @TODO: handle spaces in the path
    // @TODO: what about --template options?

    QString repoFile = buildPath(repoPath, repoName, QLatin1String(Constants::FOSSIL_FILE_SUFFIX));
    QString repoFileNative(QDir::toNativeSeparators(repoFile));
    QStringList args(vcsCommandString(CreateRepositoryCommand));
    if (!adminUser.isEmpty())
        args << QLatin1String("--admin-user") << adminUser;
    args << extraOptions << repoFileNative;
    QByteArray outputData;
    if (!vcsFullySynchronousExec(workingDirectory, args, &outputData))
        return false;
    QString output = QString::fromLocal8Bit(outputData);
    output.remove(QLatin1Char('\r'));
    outputWindow->append(output);


    // check out the created repository file into the working directory

    args.clear();
    outputData.clear();
    output.clear();

    args << QLatin1String("open") << repoFileNative;
    if (!vcsFullySynchronousExec(workingDirectory, args, &outputData))
        return false;
    output = QString::fromLocal8Bit(outputData);
    output.remove(QLatin1Char('\r'));
    outputWindow->append(output);


    // set user default to admin if specified

    if (!adminUser.isEmpty()) {
        args.clear();
        outputData.clear();
        output.clear();

        args << QLatin1String("user") << QLatin1String("default") << adminUser
             << QLatin1String("--user") << adminUser;
        QByteArray outputData;
        if (!vcsFullySynchronousExec(workingDirectory, args, &outputData))
            return false;
        QString output = QString::fromLocal8Bit(outputData);
        output.remove(QLatin1Char('\r'));
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

    QStringList args;
    args << vcsCommandString(MoveCommand) << extraOptions << from << to;
    QByteArray stdOut;
    return vcsFullySynchronousExec(workingDir, args, &stdOut);
}

bool FossilClient::synchronousPull(const QString &workingDir, const QString &srcLocation, const QStringList &extraOptions)
{
    QString remoteLocation(srcLocation);
    if (remoteLocation.isEmpty())
        remoteLocation = synchronousGetRepositoryURL(workingDir);

    if (remoteLocation.isEmpty())
        return false;

    QStringList args;
    args << vcsCommandString(PullCommand) << remoteLocation << extraOptions;
    // Disable UNIX terminals to suppress SSH prompting
    const unsigned flags =
            VCSBase::VCSBasePlugin::SshPasswordPrompt
            | VCSBase::VCSBasePlugin::ShowStdOutInLogWindow
            | VCSBase::VCSBasePlugin::ShowSuccessMessage;
    const Utils::SynchronousProcessResponse resp = vcsSynchronousExec(workingDir, args, flags);
    const bool success = (resp.result == Utils::SynchronousProcessResponse::Finished);
    if (success)
        emit changed(QVariant(workingDir));
    return success;
}

bool FossilClient::synchronousPush(const QString &workingDir, const QString &dstLocation, const QStringList &extraOptions)
{
    QString remoteLocation(dstLocation);
    if (remoteLocation.isEmpty())
        remoteLocation = synchronousGetRepositoryURL(workingDir);

    if (remoteLocation.isEmpty())
        return false;

    QStringList args;
    args << vcsCommandString(PushCommand) << remoteLocation << extraOptions;
    // Disable UNIX terminals to suppress SSH prompting
    const unsigned flags =
            VCSBase::VCSBasePlugin::SshPasswordPrompt
            | VCSBase::VCSBasePlugin::ShowStdOutInLogWindow
            | VCSBase::VCSBasePlugin::ShowSuccessMessage;
    const Utils::SynchronousProcessResponse resp = vcsSynchronousExec(workingDir, args, flags);
    return (resp.result == Utils::SynchronousProcessResponse::Finished);
}

void FossilClient::commit(const QString &repositoryRoot, const QStringList &files,
                          const QString &commitMessageFile, const QStringList &extraOptions)
{
    VCSBaseClient::commit(repositoryRoot, files, commitMessageFile,
                          QStringList(extraOptions) << QLatin1String("-M") << commitMessageFile);
}

void FossilClient::annotate(const QString &workingDir, const QString &file,
                            const QString revision, int lineNumber,
                            const QStringList &extraOptions)
{
    VCSBaseClient::annotate(workingDir, file, revision, lineNumber,
                            QStringList(extraOptions) << QLatin1String("--log"));
}

QString FossilClient::findTopLevelForFile(const QFileInfo &file) const
{
    const QString repositoryCheckFile =
            QLatin1String(Constants::FOSSILREPO);
    return file.isDir() ?
                VCSBase::VCSBasePlugin::findRepositoryForDirectory(file.absoluteFilePath(),
                                                                   repositoryCheckFile) :
                VCSBase::VCSBasePlugin::findRepositoryForDirectory(file.absolutePath(),
                                                                   repositoryCheckFile);
}

void FossilClient::view(const QString &source, const QString &id, const QStringList &extraOptions)
{
    QStringList args(QLatin1String("diff"));

    const QFileInfo fi(source);
    const QString workingDirectory = fi.isFile() ? fi.absolutePath() : source;

    RevisionInfo revisionInfo = synchronousRevisionQuery(workingDirectory,id);

    args << QLatin1String("--from") << revisionInfo.parentId
         << QLatin1String("--to") << revisionInfo.id
         << QLatin1String("-v")
         << extraOptions;

    const QString kind = vcsEditorKind(DiffCommand);
    const QString title = vcsEditorTitle(vcsCommandString(DiffCommand), id);

    VCSBase::VCSBaseEditorWidget *editor = createVCSEditor(kind, title, source,
                                                           true, "view", id);
    enqueueJob(createCommand(workingDirectory, editor), args);
}

class FossilLogHighlighter : QSyntaxHighlighter
{
public:
    explicit FossilLogHighlighter(QTextDocument *parent);
    virtual void highlightBlock(const QString &text);

private:
    const QRegExp m_revisionIdRx;
    const QRegExp m_dateRx;
};

FossilLogHighlighter::FossilLogHighlighter(QTextDocument * parent) :
    QSyntaxHighlighter(parent),
    m_revisionIdRx(QLatin1String(Constants::CHANGESET_ID)),
    m_dateRx(QLatin1String("([0-9]{4}-[0-9]{2}-[0-9]{2})"))
{
    Q_ASSERT(m_revisionIdRx.isValid());
    Q_ASSERT(m_dateRx.isValid());
}

void FossilLogHighlighter::highlightBlock(const QString &text)
{
    // Match the revision-ids and dates -- highlight them for convenience.

    //const QTextBlock block = currentBlock();

    // Format revision-ids
    for (int start = 0;
         (start = m_revisionIdRx.indexIn(text, start)) != -1;
         start += m_revisionIdRx.matchedLength()) {
        QTextCharFormat charFormat = format(0);
        charFormat.setForeground(Qt::darkBlue);
        //charFormat.setFontItalic(true);
        setFormat(start, m_revisionIdRx.matchedLength(), charFormat);
    }

    // Format dates
    for (int start = 0;
         (start = m_dateRx.indexIn(text, start)) != -1;
         start += m_dateRx.matchedLength()) {
        QTextCharFormat charFormat = format(0);
        charFormat.setForeground(Qt::darkBlue);
        charFormat.setFontWeight(QFont::DemiBold);
        setFormat(start, m_dateRx.matchedLength(), charFormat);
    }
}

void FossilClient::logRepository(const QString &workingDir, const QStringList &extraOptions)
{
    const QStringList files;

    const QString vcsCmdString = QLatin1String("timeline");
    const QString kind = vcsEditorKind(LogCommand);
    const QString id = VCSBase::VCSBaseEditorWidget::getTitleId(workingDir, files);
    const QString title = vcsEditorTitle(vcsCmdString, id);
    const QString source = VCSBase::VCSBaseEditorWidget::getSource(workingDir, files);

    VCSBase::VCSBaseEditorWidget *editor = createVCSEditor(kind, title, source, true,
                                                           vcsCmdString.toLatin1().constData(), id);

    VCSBase::VCSBaseEditorParameterWidget *paramWidget = createLogRepositoryEditor(workingDir, files, extraOptions);
    if (paramWidget != 0)
        editor->setConfigurationWidget(paramWidget);

    //@TODO: move highlighter and widgets to fossil editor sources.

    //new FossilLogHighlighter(editor->document());

    QStringList args;
    const QStringList paramArgs = paramWidget != 0 ? paramWidget->arguments() : QStringList();
    args << vcsCmdString << extraOptions << paramArgs << files;
    enqueueJob(createCommand(workingDir, editor), args);
}

void FossilClient::revertFile(const QString &workingDir,
                              const QString &file,
                              const QString &revision,
                              const QStringList &extraOptions)
{
    QStringList args(vcsCommandString(RevertCommand));
    if (!revision.isEmpty())
        args << QLatin1String("-r") << revision;

    args << extraOptions << file;

    // Indicate file list
    VCSBase::Command *cmd = createCommand(workingDir);
    cmd->setCookie(QStringList(workingDir + QLatin1Char('/') + file));
    connect(cmd, SIGNAL(success(QVariant)), this, SIGNAL(changed(QVariant)), Qt::QueuedConnection);
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
        args << QLatin1String("checkout") << revision
             << QLatin1String("--force")
             << extraOptions;
    }

    // Indicate repository change
    VCSBase::Command *cmd = createCommand(workingDir);
    cmd->setCookie(QStringList(workingDir));
    connect(cmd, SIGNAL(success(QVariant)), this, SIGNAL(changed(QVariant)), Qt::QueuedConnection);
    enqueueJob(createCommand(workingDir), args);
}

QString FossilClient::vcsCommandString(VCSCommand cmd) const
{
    // override specific client commands
    // otherwise return baseclient command

    switch (cmd) {
    case RemoveCommand: return QLatin1String("rm");
    case MoveCommand: return QLatin1String("mv");
    case LogCommand: return QLatin1String("finfo");

    default: return VCSBaseClient::vcsCommandString(cmd);
    }
}

QString FossilClient::vcsEditorKind(VCSCommand cmd) const
{
    switch(cmd) {
    case AnnotateCommand:
        return QLatin1String(Constants::ANNOTATELOG);
    case DiffCommand:
        return QLatin1String(Constants::DIFFLOG);
    case LogCommand:
        return QLatin1String(Constants::FILELOG);
    default:
        return QString();
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
    const QChar spaceChar(QLatin1Char(' '));

    StatusItem item;

    // Ref: fossil source 'src/checkin.c' status_report()
    // Expect at least one non-leading blank space.

    int pos = line.indexOf(spaceChar);

    if (line.isEmpty() || pos < 1)
        return StatusItem();

    QString label(line.left(pos));
    QString flags;

    if (label == QLatin1String("EDITED"))
        flags = QLatin1String("Edited");
    else if (label == QLatin1String("ADDED"))
        flags = QLatin1String("Added");
    else if (label == QLatin1String("RENAMED"))
        flags = QLatin1String("Renamed");
    else if (label == QLatin1String("DELETED"))
        flags = QLatin1String("Deleted");
    else if (label == QLatin1String("MISSING"))
        flags = QLatin1String("Missing");
    else if (label == QLatin1String("ADDED_BY_MERGE"))
        flags = QLatin1String("Added by Merge");
    else if (label == QLatin1String("UPDATED_BY_MERGE"))
        flags = QLatin1String("Updated by Merge");
    else if (label == QLatin1String("CONFLICT"))
        flags = QLatin1String("Conflict");
    else if (label == QLatin1String("NOT_A_FILE"))
        flags = QLatin1String("Unknown");

    if (flags.isEmpty())
        return StatusItem();

    // adjust the position to the last space before the file name
    for (int size = line.size(); (pos+1) < size && line[pos+1].isSpace(); ++pos) {}

    item.flags = flags;
    item.file = line.mid(pos + 1);

    return item;
}

// Collect all parameters required for a diff or log to be able to associate
// them with an editor and re-run the command with parameters.
struct FossilCommandParameters
{
    FossilCommandParameters(const QString &workDir,
                            const QStringList &inFiles,
                            const QStringList &options) :
        workingDir(workDir), files(inFiles), extraOptions(options)
    {
    }

    QString workingDir;
    QStringList files;
    QStringList extraOptions;
};

// Parameter widget controlling whitespace diff mode, associated with a parameter
class FossilDiffParameterWidget : public VCSBase::VCSBaseEditorParameterWidget
{
    Q_OBJECT
public:
    FossilDiffParameterWidget(FossilClient *client,
                              const FossilCommandParameters &p, QWidget *parent = 0) :
        VCSBase::VCSBaseEditorParameterWidget(parent), m_client(client), m_params(p)
    {
        // NOTE: fossil diff does not support whitespace ignore option, may be next revisions
//        mapSetting(addToggleButton(QLatin1String("-w"), tr("Ignore whitespace")),
//                   m_client->settings()->boolPointer(FossilSettings::diffIgnoreWhiteSpaceKey));
    }

    QStringList arguments() const
    {
        // pass args to Fossil verbatim
        return VCSBaseEditorParameterWidget::arguments();
    }

    void executeCommand()
    {
        m_client->diff(m_params.workingDir, m_params.files, m_params.extraOptions);
    }

private:
    FossilClient *m_client;
    const FossilCommandParameters m_params;
};

VCSBase::VCSBaseEditorParameterWidget *FossilClient::createDiffEditor(
    const QString &workingDir, const QStringList &files, const QStringList &extraOptions)
{
    const FossilCommandParameters parameters(workingDir, files, extraOptions);
    return new FossilDiffParameterWidget(this, parameters);
}

class FossilLogParameterWidget : public VCSBase::VCSBaseEditorParameterWidget
{
    Q_OBJECT
public:
    FossilLogParameterWidget(FossilClient *client,
                             const FossilCommandParameters &p, QWidget *parent = 0) :
        VCSBase::VCSBaseEditorParameterWidget(parent), m_client(client), m_params(p)
    {
    }

    void executeCommand()
    {
        m_client->log(m_params.workingDir, m_params.files, m_params.extraOptions);
    }

private:
    FossilClient *m_client;
    const FossilCommandParameters m_params;
};

VCSBase::VCSBaseEditorParameterWidget *FossilClient::createLogEditor(
    const QString &workingDir, const QStringList &files, const QStringList &extraOptions)
{
    const FossilCommandParameters parameters(workingDir, files, extraOptions);
    return new FossilLogParameterWidget(this, parameters);
}

class FossilLogRepositoryParameterWidget : public VCSBase::VCSBaseEditorParameterWidget
{
    Q_OBJECT
public:
    FossilLogRepositoryParameterWidget(FossilClient *client,
                             const FossilCommandParameters &p, QWidget *parent = 0) :
        VCSBase::VCSBaseEditorParameterWidget(parent), m_client(client), m_params(p)
    {
        // setup space-separated mapping to result in: "option value"
        setComboBoxOptionTemplate(QStringList() << QLatin1String("%{option}") << QLatin1String("%{value}"));

        // ancestors/descendants filter
        // This is a positional argument not an option.
        // Normally it takes the checkin/branch/tag as an additional parameter
        // (trunk by default)
        // So we kludge this by coding it as a meta-option (pipe-separated),
        // then parse it out in arguments.
        // All-choice is a blank argument with no additional parameters
        QList<ComboBoxItem> lineageFilterChoices;
        lineageFilterChoices << ComboBoxItem(tr("Ancestors"), QLatin1String("|LINEAGE|ancestors|current"))
                        << ComboBoxItem(tr("Descendants"), QLatin1String("|LINEAGE|descendants|current"))
                        << ComboBoxItem(tr("Unfiltered"), QLatin1String("|LINEAGE|"));
        mapSetting(addComboBox(QLatin1String("|META-OPTION"), lineageFilterChoices),
                   m_client->settings()->stringPointer(FossilSettings::timelineLineageFilterKey));

        // show files
        mapSetting(addToggleButton(QLatin1String("-showfiles"), tr("Verbose"),
                                   tr("Show files changed in each revision")),
                   m_client->settings()->boolPointer(FossilSettings::timelineVerboseKey));

        // option: -t <val>
        QList<ComboBoxItem> itemTypeChoices;
        itemTypeChoices << ComboBoxItem(tr("All Items"), QLatin1String("all"))
                        << ComboBoxItem(tr("File Commits"), QLatin1String("ci"))
                        << ComboBoxItem(tr("Events"), QLatin1String("e"))
                        << ComboBoxItem(tr("Tags"), QLatin1String("g"))
                        << ComboBoxItem(tr("Tickets"), QLatin1String("t"))
                        << ComboBoxItem(tr("Wiki Commits"), QLatin1String("w"));
        mapSetting(addComboBox(QLatin1String("-t"), itemTypeChoices),
                   m_client->settings()->stringPointer(FossilSettings::timelineItemTypeKey));
    }

    QStringList arguments() const
    {
        const QChar pipeChar(QLatin1Char('|'));
        QStringList args;

        foreach (const QString &arg, VCSBaseEditorParameterWidget::arguments()) {
            if (arg.startsWith(pipeChar)){
                // meta-option: "|META-OPTION" "|OPT|val|extra1|..."
                QStringList params = arg.split(pipeChar);
                QString option = params[1];
                if (option == QLatin1String("META-OPTION"))
                    continue; // skip

                for (int i = 2; i < params.size(); ++i) {
                    if (option == QLatin1String("LINEAGE") && params[i].isEmpty()) {
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

    void executeCommand()
    {
        m_client->logRepository(m_params.workingDir, m_params.extraOptions);
    }

private:
    FossilClient *m_client;
    const FossilCommandParameters m_params;
};

VCSBase::VCSBaseEditorParameterWidget *FossilClient::createLogRepositoryEditor(
    const QString &workingDir, const QStringList &files, const QStringList &extraOptions)
{
    const FossilCommandParameters parameters(workingDir, files, extraOptions);
    return new FossilLogRepositoryParameterWidget(this, parameters);
}

} // namespace Internal
} // namespace Fossil

#include "fossilclient.moc"
