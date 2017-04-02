/**************************************************************************
**  This file is part of Fossil VCS plugin for Qt Creator
**
**  Copyright (c) 2013 - 2017, Artur Shepilko, <qtc-fossil@nomadbyte.com>.
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

#include <vcsbase/vcsbaseplugin.h>
#include <vcsbase/vcsbaseeditor.h>
#include <vcsbase/vcsbaseeditorparameterwidget.h>
#include <vcsbase/vcsbaseoutputwindow.h>
#include <vcsbase/command.h>

#include <utils/synchronousprocess.h>
#include <utils/qtcassert.h>

#include <QSyntaxHighlighter>

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QMap>

#include <QDebug>

namespace Fossil {
namespace Internal {

unsigned FossilClient::makeVersionNumber(int major, int minor, int patch)
{
    const unsigned version = (QString().setNum(major).toUInt(0,16) << 16) +
                             (QString().setNum(minor).toUInt(0,16) << 8) +
                             (QString().setNum(patch).toUInt(0,16));

    return version;
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
    : VcsBase::VcsBaseClient(settings)
{
}

FossilSettings *FossilClient::settings() const
{
    return dynamic_cast<FossilSettings *>(VcsBase::VcsBaseClient::settings());
}

unsigned int FossilClient::synchronousBinaryVersion() const
{
    if (settings()->binaryPath().isEmpty())
        return 0;

    QStringList args;
    args << QLatin1String("version");

    QByteArray outputData;
    if (!vcsFullySynchronousExec(QString(), args, &outputData))
        return 0;

    QString output = QString::fromLocal8Bit(outputData);
    output.remove(QLatin1Char('\r'));

    if (output.endsWith(QLatin1Char('\n')))
        output.truncate(output.size()-1);

    // fossil version:
    // "This is fossil version 1.27 [ccdefa355b] 2013-09-30 11:47:18 UTC"
    QRegExp versionPattern(QLatin1String("^[^\\d]+(\\d+)\\.(\\d+).*$"));
    QTC_ASSERT(versionPattern.isValid(), return 0);
    QTC_ASSERT(versionPattern.exactMatch(output), return 0);
    const int major = versionPattern.cap(1).toInt();
    const int minor = versionPattern.cap(2).toInt();
    const int patch = 0;
    return makeVersionNumber(major,minor,patch);
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
        return RevisionInfo();

    QStringList args;
    args << QLatin1String("info");
    if (!id.isEmpty())
        args << id;

    QByteArray outputData;
    if (!vcsFullySynchronousExec(workingDirectory, args, &outputData))
        return RevisionInfo();
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
    QTC_ASSERT(revisionIdRx.isValid(), return RevisionInfo());
    QTC_ASSERT(parentIdRx.isValid(), return RevisionInfo());

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
               return RevisionInfo());

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

        QString property = fields.at(0);
        QString value;
        if (fields.size() >= 3)
            value = fields.at(2);
        QString lcValue = value.toLower();

        if (property == QLatin1String("autosync")) {
            if (lcValue == QLatin1String("on")
                || lcValue == QLatin1String("1"))
                repoSettings.autosync = RepositorySettings::AutosyncOn;
            else if (lcValue == QLatin1String("off")
                     || lcValue == QLatin1String("0"))
                     repoSettings.autosync = RepositorySettings::AutosyncOff;
            else if (lcValue == QLatin1String("pullonly")
                     || lcValue == QLatin1String("2"))
                     repoSettings.autosync = RepositorySettings::AutosyncPullOnly;
        }
        else if (property == QLatin1String("ssl-identity")) {
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
    if (workingDirectory.isEmpty())
        return false;

    // apply updated settings vs. current setting if given
    bool applyAll = (currentSettings == RepositorySettings());

    if (!newSettings.user.isEmpty()
        && (applyAll
            || newSettings.user != currentSettings.user)
            && !synchronousSetUserDefault(workingDirectory, newSettings.user)) {
        return false;
    }

    if ((applyAll
         || newSettings.sslIdentityFile != currentSettings.sslIdentityFile)
        && !synchronousSetSetting(workingDirectory,QLatin1String("ssl-identity"), newSettings.sslIdentityFile)) {
        return false;
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

        if (!synchronousSetSetting(workingDirectory,QLatin1String("autosync"), value))
            return false;
    }

    return true;
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

    if (output.isEmpty() || output.toLower() == QLatin1String("off"))
        return QString();

    return output;
}

struct TopicData
{
    QDateTime timeStamp;
    QString topic;
};

QString FossilClient::synchronousTopic(const QString &workingDirectory)
{
    static QMap<QString, TopicData> topicCache;

    if (workingDirectory.isEmpty())
        return QString();

    // return current branch name

    const QString topLevel = findTopLevelForFile(workingDirectory);
    QFileInfo currentStateFile(topLevel + QLatin1String("/")
                               + QLatin1String(Constants::FOSSILREPO));

    TopicData &data = topicCache[workingDirectory];
    QDateTime lastModified = currentStateFile.lastModified();
    if (lastModified == data.timeStamp)
        return data.topic;

    BranchInfo branchInfo = synchronousBranchQuery(workingDirectory);
    if (branchInfo.name().isEmpty())
        return QString();

    data.timeStamp = lastModified;
    return data.topic = branchInfo.name();
}

bool FossilClient::synchronousCreateRepository(const QString &workingDirectory, const QStringList &extraOptions)
{
    VcsBase::VcsBaseOutputWindow *outputWindow = VcsBase::VcsBaseOutputWindow::instance();

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
            VcsBase::VcsBasePlugin::SshPasswordPrompt
            | VcsBase::VcsBasePlugin::ShowStdOutInLogWindow
            | VcsBase::VcsBasePlugin::ShowSuccessMessage;
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
            VcsBase::VcsBasePlugin::SshPasswordPrompt
            | VcsBase::VcsBasePlugin::ShowStdOutInLogWindow
            | VcsBase::VcsBasePlugin::ShowSuccessMessage;
    const Utils::SynchronousProcessResponse resp = vcsSynchronousExec(workingDir, args, flags);
    return (resp.result == Utils::SynchronousProcessResponse::Finished);
}

void FossilClient::commit(const QString &repositoryRoot, const QStringList &files,
                          const QString &commitMessageFile, const QStringList &extraOptions)
{
    VcsBaseClient::commit(repositoryRoot, files, commitMessageFile,
                          QStringList(extraOptions) << QLatin1String("-M") << commitMessageFile);
}

void FossilClient::annotate(const QString &workingDir, const QString &file,
                            const QString revision, int lineNumber,
                            const QStringList &extraOptions)
{
    // 'fossil annotate' command has a variant 'fossil blame'
    // blame command attributes committing username to source lines
    // annotate shows line numbers

    Q_UNUSED(lineNumber)
    QString vcsCmdString = vcsCommandString(AnnotateCommand);
    const Core::Id kind = vcsEditorKind(AnnotateCommand);
    const QString id = VcsBase::VcsBaseEditorWidget::getSource(workingDir, QStringList(file));
    const QString title = vcsEditorTitle(vcsCmdString, id);
    const QString source = VcsBase::VcsBaseEditorWidget::getSource(workingDir, file);

    VcsBase::VcsBaseEditorWidget *editor = createVcsEditor(kind, title, source, true,
                                                           vcsCmdString.toLatin1().constData(), id);

    QStringList effectiveArgs = extraOptions;
    VcsBase::VcsBaseEditorParameterWidget *paramWidget = editor->configurationWidget();
    if (!paramWidget && (paramWidget = createAnnotateEditor(workingDir, file, revision, lineNumber, extraOptions))) {
        // editor has been just created, createVcsEditor() didn't set a configuration widget yet
        editor->setConfigurationWidget(paramWidget);
    }

    if (paramWidget) {
        paramWidget->setBaseArguments(extraOptions);
        effectiveArgs = paramWidget->arguments();
    }

    VcsBase::Command *cmd = createCommand(workingDir, editor);

    // here we introduce a "|BLAME|" meta-option to allow both annotate and blame modes
    QRegExp blameMetaOptionRx = QRegExp(QLatin1String("\\|BLAME\\|"));
    QTC_ASSERT(blameMetaOptionRx.isValid(), return);

    int foundAt = effectiveArgs.indexOf(blameMetaOptionRx);
    if (foundAt != -1) {
        vcsCmdString = QLatin1String("blame");
        effectiveArgs.removeAt(foundAt);
    }
    QStringList args;
    args << vcsCmdString
         << revisionSpec(revision)
         << effectiveArgs << file;

    // When version list requested, ignore the source line.
    if (args.indexOf(QLatin1String("--log")) != -1)
        lineNumber = -1;
    cmd->setCookie(lineNumber);

    enqueueJob(cmd, args);
}

QString FossilClient::findTopLevelForFile(const QFileInfo &file) const
{
    const QString repositoryCheckFile =
            QLatin1String(Constants::FOSSILREPO);
    return file.isDir() ?
                VcsBase::VcsBasePlugin::findRepositoryForDirectory(file.absoluteFilePath(),
                                                                   repositoryCheckFile) :
                VcsBase::VcsBasePlugin::findRepositoryForDirectory(file.absolutePath(),
                                                                   repositoryCheckFile);
}

bool FossilClient::managesFile(const QString &workingDirectory, const QString &fileName) const
{
    QStringList args(QLatin1String("finfo"));
    args << fileName;
    QByteArray stdOut;
    if (!vcsFullySynchronousExec(workingDirectory, args, &stdOut))
        return false;
    return !stdOut.toLower().startsWith("no history for file");
}

unsigned int FossilClient::binaryVersion() const
{
    static unsigned int cachedBinaryVersion = 0;
    static QString cachedBinaryPath;

    const QString currentBinaryPath = settings()->binaryPath();

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

    if (version < 0x13000) {
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
    QStringList args(QLatin1String("diff"));

    const QFileInfo fi(source);
    const QString workingDirectory = fi.isFile() ? fi.absolutePath() : source;

    RevisionInfo revisionInfo = synchronousRevisionQuery(workingDirectory,id);

    args << QLatin1String("--from") << revisionInfo.parentId
         << QLatin1String("--to") << revisionInfo.id
         << QLatin1String("-v")
         << extraOptions;

    const Core::Id kind = vcsEditorKind(DiffCommand);
    const QString title = vcsEditorTitle(vcsCommandString(DiffCommand), id);

    VcsBase::VcsBaseEditorWidget *editor = createVcsEditor(kind, title, source,
                                                           true, "view", id);
    editor->setWorkingDirectory(workingDirectory);

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
    QTC_CHECK(m_revisionIdRx.isValid());
    QTC_CHECK(m_dateRx.isValid());
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
    const QString id = VcsBase::VcsBaseEditorWidget::getTitleId(workingDir, files);
    const QString title = vcsEditorTitle(vcsCmdString, id);
    const QString source = VcsBase::VcsBaseEditorWidget::getSource(workingDir, files);
    VcsBase::VcsBaseEditorWidget *editor = createVcsEditor(kind, title, source, true,
                                                           vcsCmdString.toLatin1().constData(), id);
    editor->setFileLogAnnotateEnabled(enableAnnotationContextMenu);

    QStringList effectiveArgs = extraOptions;
    VcsBase::VcsBaseEditorParameterWidget *paramWidget = editor->configurationWidget();
    if (!paramWidget && (paramWidget = createLogEditor(workingDir, files, extraOptions))) {
        // editor has been just created, createVcsEditor() didn't set a configuration widget yet
        editor->setConfigurationWidget(paramWidget);
    }

    if (paramWidget) {
        paramWidget->setBaseArguments(extraOptions);
        effectiveArgs = paramWidget->arguments();
    }

    //@TODO: move highlighter and widgets to fossil editor sources.

    new FossilLogHighlighter(editor->document());

    QStringList args;
    args << vcsCmdString << effectiveArgs;
    if (!files.isEmpty())
         args << QLatin1String("--path") << files;
    enqueueJob(createCommand(workingDir, editor), args);
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

    const QString vcsCmdString = QLatin1String("finfo");
    const Core::Id kind = vcsEditorKind(LogCommand);
    const QString id = VcsBase::VcsBaseEditorWidget::getTitleId(workingDir, files);
    const QString title = vcsEditorTitle(vcsCmdString, id);
    const QString source = VcsBase::VcsBaseEditorWidget::getSource(workingDir, files);
    VcsBase::VcsBaseEditorWidget *editor = createVcsEditor(kind, title, source, true,
                                                           vcsCmdString.toLatin1().constData(), id);
    editor->setFileLogAnnotateEnabled(enableAnnotationContextMenu);

    QStringList effectiveArgs = extraOptions;
    VcsBase::VcsBaseEditorParameterWidget *paramWidget = editor->configurationWidget();
    if (!paramWidget && (paramWidget = createLogCurrentFileEditor(workingDir, files, extraOptions))) {
        // editor has been just created, createVcsEditor() didn't set a configuration widget yet
        editor->setConfigurationWidget(paramWidget);
    }

    if (paramWidget) {
        paramWidget->setBaseArguments(extraOptions);
        effectiveArgs = paramWidget->arguments();
    }

    //@TODO: move highlighter and widgets to fossil editor sources.

    new FossilLogHighlighter(editor->document());

    QStringList args;
    args << vcsCmdString << effectiveArgs << files;
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
    VcsBase::Command *cmd = createCommand(workingDir);
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
    VcsBase::Command *cmd = createCommand(workingDir);
    cmd->setCookie(QStringList(workingDir));
    connect(cmd, SIGNAL(success(QVariant)), this, SIGNAL(changed(QVariant)), Qt::QueuedConnection);
    enqueueJob(createCommand(workingDir), args);
}

QString FossilClient::vcsCommandString(VcsCommand cmd) const
{
    // override specific client commands
    // otherwise return baseclient command

    switch (cmd) {
    case RemoveCommand: return QLatin1String("rm");
    case MoveCommand: return QLatin1String("mv");
    case LogCommand: return QLatin1String("timeline");

    default: return VcsBaseClient::vcsCommandString(cmd);
    }
}

Core::Id FossilClient::vcsEditorKind(VcsCommand cmd) const
{
    switch (cmd) {
    case AnnotateCommand:
        return Constants::ANNOTATELOG;
    case DiffCommand:
        return Constants::DIFFLOG;
    case LogCommand:
        return Constants::FILELOG;
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
    else if (label == QLatin1String("ADDED_BY_INTEGRATE"))
        flags = QLatin1String("Added by Integrate");
    else if (label == QLatin1String("UPDATED_BY_INTEGRATE"))
        flags = QLatin1String("Updated by Integrate");
    else if (label == QLatin1String("CONFLICT"))
        flags = QLatin1String("Conflict");
    else if (label == QLatin1String("EXECUTABLE"))
        flags = QLatin1String("Set Exec");
    else if (label == QLatin1String("SYMLINK"))
        flags = QLatin1String("Set Symlink");
    else if (label == QLatin1String("UNEXEC"))
        flags = QLatin1String("Unset Exec");
    else if (label == QLatin1String("UNLINK"))
        flags = QLatin1String("Unset Symlink");
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
                            const QStringList &options,
                            const QString &inRevision = QString(),
                            int inLineNumber = -1) :
        workingDir(workDir), files(inFiles), extraOptions(options),
        revision(inRevision), lineNumber(inLineNumber)
    {
    }

    QString workingDir;
    QStringList files;
    QStringList extraOptions;
    QString revision;
    int lineNumber;
};

// Parameter widget controlling whitespace diff mode, associated with a parameter
class FossilDiffParameterWidget : public VcsBase::VcsBaseEditorParameterWidget
{
    Q_OBJECT
public:
    FossilDiffParameterWidget(FossilClient *client,
                              const FossilCommandParameters &p, QWidget *parent = 0) :
        VcsBase::VcsBaseEditorParameterWidget(parent), m_client(client), m_params(p)
    {
        QTC_ASSERT(m_client, return);
        FossilClient::SupportedFeatures features = m_client->supportedFeatures();

        if (features.testFlag(FossilClient::DiffIgnoreWhiteSpaceFeature)) {
            mapSetting(addToggleButton(QLatin1String("-w"), tr("Ignore All Whitespace")),
                       m_client->settings()->boolPointer(FossilSettings::diffIgnoreAllWhiteSpaceKey));
            mapSetting(addToggleButton(QLatin1String("--strip-trailing-cr"), tr("Strip Trailing CR")),
                       m_client->settings()->boolPointer(FossilSettings::diffStripTrailingCRKey));
        }
    }

    QStringList arguments() const
    {
        // pass args to Fossil verbatim
        return VcsBaseEditorParameterWidget::arguments();
    }

    void executeCommand()
    {
        m_client->diff(m_params.workingDir, m_params.files, m_params.extraOptions);
    }

private:
    FossilClient *m_client;
    FossilCommandParameters m_params;
};

VcsBase::VcsBaseEditorParameterWidget *FossilClient::createDiffEditor(
    const QString &workingDir, const QStringList &files, const QStringList &extraOptions)
{
    const FossilCommandParameters parameters(workingDir, files, extraOptions);
    return new FossilDiffParameterWidget(this, parameters);
}


// Parameter widget controlling annotate/blame mode
class FossilAnnotateParameterWidget : public VcsBase::VcsBaseEditorParameterWidget
{
    Q_OBJECT
public:
    FossilAnnotateParameterWidget(FossilClient *client,
                              const FossilCommandParameters &p, QWidget *parent = 0) :
        VcsBase::VcsBaseEditorParameterWidget(parent), m_client(client), m_params(p)
    {
        QTC_ASSERT(m_client, return);
        FossilSettings *settings = m_client->settings();
        FossilClient::SupportedFeatures features = m_client->supportedFeatures();

        if (features.testFlag(FossilClient::AnnotateBlameFeature)) {
            mapSetting(addToggleButton(QLatin1String("|BLAME|"), tr("Show Committers")),
                       settings->boolPointer(FossilSettings::annotateShowCommittersKey));
        }

        // Force listVersions setting to false by default.
        // This way the annotated line number would not get offset by the version list.
        settings->setValue(FossilSettings::annotateListVersionsKey, false);

        mapSetting(addToggleButton(QLatin1String("--log"), tr("List Versions")),
                   settings->boolPointer(FossilSettings::annotateListVersionsKey));
    }

    QStringList arguments() const
    {
        // pass args to Fossil verbatim
        return VcsBaseEditorParameterWidget::arguments();
    }

    void executeCommand()
    {
        FossilSettings *settings = m_client->settings();

        QString file;
        if (!m_params.files.isEmpty())
            file = m_params.files[0];

        // Save the current line number unless ListVersions requested (jumps to top).
        if (!settings->boolPointer(FossilSettings::annotateListVersionsKey))
            m_params.lineNumber = VcsBase::VcsBaseEditorWidget::lineNumberOfCurrentEditor();

        m_client->annotate(m_params.workingDir, file, m_params.revision,
                           m_params.lineNumber, m_params.extraOptions);
    }

private:
    FossilClient *m_client;
    FossilCommandParameters m_params;
};

VcsBase::VcsBaseEditorParameterWidget *FossilClient::createAnnotateEditor(
        const QString &workingDir,
        const QString &file,
        const QString &revision,
        int lineNumber,
        const QStringList &extraOptions)
{
    const FossilCommandParameters parameters(workingDir, QStringList(file), extraOptions,
                                             revision, lineNumber);
    return new FossilAnnotateParameterWidget(this, parameters);
}


class FossilLogCurrentFileParameterWidget : public VcsBase::VcsBaseEditorParameterWidget
{
    Q_OBJECT
public:
    FossilLogCurrentFileParameterWidget(FossilClient *client,
                             const FossilCommandParameters &p, QWidget *parent = 0) :
        VcsBase::VcsBaseEditorParameterWidget(parent), m_client(client), m_params(p)
    {
        QTC_ASSERT(m_client, return);
    }

    void executeCommand()
    {
        m_client->logCurrentFile(m_params.workingDir, m_params.files, m_params.extraOptions);
    }

private:
    FossilClient *m_client;
    FossilCommandParameters m_params;
};

VcsBase::VcsBaseEditorParameterWidget *FossilClient::createLogCurrentFileEditor(
    const QString &workingDir, const QStringList &files, const QStringList &extraOptions)
{
    SupportedFeatures features = supportedFeatures();

    if (features.testFlag(TimelinePathFeature))
        return createLogEditor(workingDir, files, extraOptions);

    const FossilCommandParameters parameters(workingDir, files, extraOptions);
    return new FossilLogCurrentFileParameterWidget(this, parameters);
}

class FossilLogParameterWidget : public VcsBase::VcsBaseEditorParameterWidget
{
    Q_OBJECT
public:
    FossilLogParameterWidget(FossilClient *client,
                             const FossilCommandParameters &p, QWidget *parent = 0) :
        VcsBase::VcsBaseEditorParameterWidget(parent), m_client(client), m_params(p)
    {
        QTC_ASSERT(m_client, return);

        // ancestors/descendants filter
        // This is a positional argument not an option.
        // Normally it takes the checkin/branch/tag as an additional parameter
        // (trunk by default)
        // So we kludge this by coding it as a meta-option (pipe-separated),
        // then parse it out in arguments.
        // All-choice is a blank argument with no additional parameters
        QList<ComboBoxItem> lineageFilterChoices;
        lineageFilterChoices << ComboBoxItem(tr("Ancestors"), QLatin1String("ancestors"))
                        << ComboBoxItem(tr("Descendants"), QLatin1String("descendants"))
                        << ComboBoxItem(tr("Unfiltered"), QLatin1String(""));
        mapSetting(addComboBox(QStringList(QLatin1String("|LINEAGE|%1|current")), lineageFilterChoices),
                   m_client->settings()->stringPointer(FossilSettings::timelineLineageFilterKey));

        // show files
        mapSetting(addToggleButton(QLatin1String("-showfiles"), tr("Verbose"),
                                   tr("Show files changed in each revision")),
                   m_client->settings()->boolPointer(FossilSettings::timelineVerboseKey));

        // option: -t <val>
        QList<ComboBoxItem> itemTypeChoices;
        itemTypeChoices << ComboBoxItem(tr("All Items"), QLatin1String("all"))
                        << ComboBoxItem(tr("File Commits"), QLatin1String("ci"))
                        << ComboBoxItem(tr("Technical Notes"), QLatin1String("e"))
                        << ComboBoxItem(tr("Tags"), QLatin1String("g"))
                        << ComboBoxItem(tr("Tickets"), QLatin1String("t"))
                        << ComboBoxItem(tr("Wiki Commits"), QLatin1String("w"));

        // here we setup the ComboBox to map to the "-t <val>", which will produce
        // the enquoted option-values (e.g "-t all").
        // Fossil expects separate arguments for option and value ( i.e. "-t" "all")
        // so we need to handle the splitting explicitly in arguments().
        mapSetting(addComboBox(QStringList(QLatin1String("-t %1")), itemTypeChoices),
                   m_client->settings()->stringPointer(FossilSettings::timelineItemTypeKey));
    }

    QStringList arguments() const
    {
        const QChar pipeChar(QLatin1Char('|'));
        const QChar spaceChar(QLatin1Char(' '));
        QStringList args;

        // split "-t val" => "-t" "val"
        foreach (const QString &arg, VcsBaseEditorParameterWidget::arguments()) {
            if (arg.startsWith(QLatin1String("-t"))) {
                args << arg.split(spaceChar);

            } else if (arg.startsWith(pipeChar)){
                // meta-option: "|OPT|val|extra1|..."
                QStringList params = arg.split(pipeChar);
                QString option = params[1];
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
        m_client->log(m_params.workingDir, m_params.files, m_params.extraOptions);
    }

private:
    FossilClient *m_client;
    FossilCommandParameters m_params;
};

VcsBase::VcsBaseEditorParameterWidget *FossilClient::createLogEditor(
    const QString &workingDir, const QStringList &files, const QStringList &extraOptions)
{
    const FossilCommandParameters parameters(workingDir, files, extraOptions);
    return new FossilLogParameterWidget(this, parameters);
}

} // namespace Internal
} // namespace Fossil

#include "fossilclient.moc"
