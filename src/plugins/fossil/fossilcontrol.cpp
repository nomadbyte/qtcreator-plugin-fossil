/**************************************************************************
**  This file is part of Fossil VCS plugin for Qt Creator
**
**  Copyright (c) 2013 - 2016, Artur Shepilko, <qtc-fossil@nomadbyte.com>.
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

#include "constants.h"
#include "fossilcontrol.h"
#include "fossilclient.h"
#include "fossilplugin.h"
#include "wizard/fossiljsextension.h"

#include <vcsbase/vcsbaseclientsettings.h>
#include <vcsbase/vcsbaseconstants.h>
#include <vcsbase/vcscommand.h>

#include <QFileInfo>
#include <QProcessEnvironment>
#include <QVariant>
#include <QStringList>
#include <QMap>
#include <QDir>
#include <QUrl>

using namespace Fossil::Internal;

FossilControl::FossilControl(FossilClient *client)
    : m_client(client)
{
}

QString FossilControl::displayName() const
{
    return tr("Fossil");
}

Core::Id FossilControl::id() const
{
    return Core::Id(Constants::VCS_ID_FOSSIL);
}

bool FossilControl::managesDirectory(const QString &directory, QString *topLevel) const
{
    QFileInfo dir(directory);
    const QString topLevelFound = m_client->findTopLevelForFile(dir);
    if (topLevel)
        *topLevel = topLevelFound;
    return !topLevelFound.isEmpty();
}

bool FossilControl::managesFile(const QString &workingDirectory, const QString &fileName) const
{
    return m_client->managesFile(workingDirectory, fileName);
}

bool FossilControl::isConfigured() const
{
    const Utils::FileName binary = m_client->vcsBinary();
    if (binary.isEmpty())
        return false;

    QFileInfo fi = binary.toFileInfo();
    if ( !(fi.exists() && fi.isFile() && fi.isExecutable()) )
        return false;

    // Local repositories default path must be set and exist
    const QString repoPath = m_client->settings().stringValue(FossilSettings::defaultRepoPathKey);
    if (repoPath.isEmpty())
        return false;

    QDir dir(repoPath);
    if (!dir.exists())
        return false;

    return true;
}

bool FossilControl::supportsOperation(Operation operation) const
{
    bool supported = isConfigured();

    switch (operation) {
    case Core::IVersionControl::AddOperation:
    case Core::IVersionControl::DeleteOperation:
    case Core::IVersionControl::MoveOperation:
    case Core::IVersionControl::CreateRepositoryOperation:
    case Core::IVersionControl::AnnotateOperation:
    case Core::IVersionControl::InitialCheckoutOperation:
        break;
    case Core::IVersionControl::SnapshotOperations:
        supported = false;
        break;
    }
    return supported;
}

bool FossilControl::vcsOpen(const QString &filename)
{
    Q_UNUSED(filename)
    return true;
}

bool FossilControl::vcsAdd(const QString &filename)
{
    const QFileInfo fi(filename);
    return m_client->synchronousAdd(fi.absolutePath(), fi.fileName());
}

bool FossilControl::vcsDelete(const QString &filename)
{
    const QFileInfo fi(filename);
    return m_client->synchronousRemove(fi.absolutePath(), fi.fileName());
}

bool FossilControl::vcsMove(const QString &from, const QString &to)
{
    const QFileInfo fromInfo(from);
    const QFileInfo toInfo(to);
    return m_client->synchronousMove(fromInfo.absolutePath(),
                                     fromInfo.absoluteFilePath(),
                                     toInfo.absoluteFilePath());
}

bool FossilControl::vcsCreateRepository(const QString &directory)
{
    return m_client->synchronousCreateRepository(directory);
}

bool FossilControl::vcsAnnotate(const QString &file, int line)
{
    const QFileInfo fi(file);
    m_client->annotate(fi.absolutePath(), fi.fileName(), QString(), line);
    return true;
}

QString FossilControl::vcsTopic(const QString &directory)
{
    return m_client->synchronousTopic(directory);
}

Core::ShellCommand *FossilControl::createInitialCheckoutCommand(const QString &sourceUrl,
                                                                const Utils::FileName &baseDirectory,
                                                                const QString &localName,
                                                                const QStringList &extraArgs)
{
    QMap<QString, QString> options;
    FossilJsExtension::parseArgOptions(extraArgs, options);

    // Two operating modes:
    //  1) CloneCheckout:
    //  -- clone from remote-URL or a local-fossil a repository  into a local-clone fossil.
    //  -- open/checkout the local-clone fossil
    //  The local-clone fossil must not point to an existing repository.
    //  Clone URL may be either schema-based (http, ssh, file) or an absolute local path.
    //
    //  2) LocalCheckout:
    //  -- open/checkout an existing local fossil
    //  Clone URL is an absoulte local path and is the same as the local fossil.

    QString checkoutPath = FossilClient::buildPath(baseDirectory.toString(), localName, QString());
    QString fossilFile = options.value(QLatin1String("fossil-file"));
    Utils::FileName fossilFileName = Utils::FileName::fromUserInput(QDir::fromNativeSeparators(fossilFile));
    const QString fossilFileNative = fossilFileName.toUserOutput();
    QFileInfo cloneRepository(fossilFileName.toString());

    // Check when requested to clone a local repository and clone-into repository file is the same
    // or not specified.
    // In this case handle it as local fossil checkout request.
    QUrl url(sourceUrl);
    bool isLocalRepository = (options.value(QLatin1String("repository-type")) == QLatin1String("localRepo"));

    if (url.isLocalFile() || url.isRelative()) {
        QFileInfo sourcePath(url.path());
        isLocalRepository = (sourcePath.canonicalFilePath() == cloneRepository.canonicalFilePath());
    }

    // set clone repository admin user to configured user name
    // OR override it with the specified user from clone panel
    QString adminUser = options.value(QLatin1String("admin-user"));
    bool disableAutosync = (options.value(QLatin1String("settings-autosync")) == QLatin1String("off"));
    QString checkoutBranch = options.value(QLatin1String("branch-tag"));

    // first create the checkout directory,
    // as it needs to become a working directory for wizard command jobs

    QDir checkoutDir(checkoutPath);
    checkoutDir.mkpath(checkoutPath);

    // Setup the wizard page command job
    auto command = new VcsBase::VcsCommand(checkoutDir.path(), m_client->processEnvironment());

    QStringList extraOptions;
    QStringList args;

    if (!isLocalRepository
        && !cloneRepository.exists()) {

        QString sslIdentityFile = options.value(QLatin1String("ssl-identity"));
        Utils::FileName sslIdentityFileName = Utils::FileName::fromUserInput(QDir::fromNativeSeparators(sslIdentityFile));
        bool includePrivate = (options.value(QLatin1String("include-private")) == QLatin1String("true"));

        if (includePrivate)
            extraOptions << QLatin1String("--private");
        if (!sslIdentityFile.isEmpty())
            extraOptions << QLatin1String("--ssl-identity") << sslIdentityFileName.toUserOutput();
        if (!adminUser.isEmpty())
            extraOptions << QLatin1String("--admin-user") << adminUser;

        // Fossil allows saving the remote address and login. This is used to
        // facilitate autosync (commit/update) functionality.
        // When no password is given, it prompts for that.
        // When both username and password are specified, it prompts whether to
        // save them.
        // NOTE: In non-interactive context, these prompts won't work.
        // Fossil currently does not support SSH_ASKPASS way for login query.
        //
        // Alternatively, "--once" option does not save the remote details.
        // In such case remote details must be provided on the command-line every
        // time. This also precludes autosync.
        //
        // So here we want Fossil to save the remote details when specified.

        args << m_client->vcsCommandString(FossilClient::CloneCommand)
             << extraOptions
             << sourceUrl
             << fossilFileNative;

        command->addJob(m_client->vcsBinary(), args, -1);
    }

    // check out the cloned repository file into the working copy directory;
    // by default the latest revision is checked out

    extraOptions.clear();
    args.clear();

    args << QLatin1String("open") << fossilFileNative;

    if (!checkoutBranch.isEmpty())
        args << checkoutBranch;
    args << extraOptions;

    command->addJob(m_client->vcsBinary(), args, -1);

    // set user default to admin user if specified
    if (!isLocalRepository
        && !adminUser.isEmpty()) {

        extraOptions.clear();
        args.clear();

        QString currentUser = adminUser;

        args << QLatin1String("user") << QLatin1String("default") << currentUser
             << QLatin1String("--user") << adminUser;

        command->addJob(m_client->vcsBinary(), args, -1);
    }

    // turn-off autosync if requested
    if (!isLocalRepository
        && disableAutosync) {
        extraOptions.clear();
        args.clear();

        args << QLatin1String("settings")
             << QLatin1String("autosync") << QLatin1String("off");

        command->addJob(m_client->vcsBinary(), args, -1);
    }

    return command;
}

void FossilControl::changed(const QVariant &v)
{
    switch (v.type()) {
    case QVariant::String:
        emit repositoryChanged(v.toString());
        break;
    case QVariant::StringList:
        emit filesChanged(v.toStringList());
        break;
    default:
        break;
    }
}
