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

#include "clonewizard.h"
#include "clonewizardpage.h"
#include "clonerepositorypanel.h"
#include "cloneoptionspanel.h"
#include "constants.h"
#include "fossilplugin.h"
#include "fossilclient.h"
#include "fossilsettings.h"

#include <coreplugin/iversioncontrol.h>
#include <vcsbase/checkoutjobs.h>
#include <vcsbase/vcsbaseconstants.h>
#include <vcsbase/vcsconfigurationpage.h>

#include <QtCore/QUrl>
#include <QtCore/QDir>
#include <QtCore/QDebug>

using namespace Fossil::Internal;

CloneWizard::CloneWizard(QObject *parent)
    : VCSBase::BaseCheckoutWizard(parent),
      m_icon(QIcon(QLatin1String(":/fossil/images/fossil.png")))
{
    setId(QLatin1String(Constants::VCS_ID_FOSSIL));
}

QIcon CloneWizard::icon() const
{
    return m_icon;
}

QString CloneWizard::description() const
{
    return tr("Clones a Fossil branch and tries to load the contained project.");
}

QString CloneWizard::displayName() const
{
    return tr("Fossil Clone");
}

QList<QWizardPage *> CloneWizard::createParameterPages(const QString &path)
{
    QList<QWizardPage *> wizardPageList;
    const Core::IVersionControl *vc = FossilPlugin::instance()->versionControl();
    if (!vc->isConfigured())
        wizardPageList.append(new VCSBase::VcsConfigurationPage(vc));
    CloneWizardPage *page = new CloneWizardPage;
    page->setPath(path);
    wizardPageList.append(page);
    return wizardPageList;
}

QSharedPointer<VCSBase::AbstractCheckoutJob> CloneWizard::createJob(const QList<QWizardPage *> &parameterPages,
                                                                    QString *checkoutPath)
{
    const CloneWizardPage *page = qobject_cast<const CloneWizardPage *>(parameterPages.front());

    if (!page)
        return QSharedPointer<VCSBase::AbstractCheckoutJob>();

    const FossilClient *client = FossilPlugin::instance()->client();
    const FossilSettings &settings = FossilPlugin::instance()->settings();
    *checkoutPath = FossilClient::buildPath(page->path(), page->directory(), QString());

    const CloneRepositoryPanel *repositoryPanel = page->cloneRepositoryPanel();
    const CloneOptionsPanel *optionsPanel = page->cloneOptionsPanel();

//    Core::FileManager *fileManager = Core::ICore::instance()->fileManager();

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

    const QString cloneRepositoryFile = repositoryPanel->cloneRepository();
    const QString cloneRepositoryFileNative(QDir::toNativeSeparators(cloneRepositoryFile));
    const CloneRepositoryPanel::RepositoryType cloneRepositoryType = repositoryPanel->cloneRepositoryType();

    QFileInfo cloneRepository(cloneRepositoryFile);

    // Check when requested to clone a local repository and clone-into repository file is the same
    // or not specified.
    // In this case handle it as local fossil checkout request.
    QUrl url(page->repository());
    bool isLocalCheckoutOnly = false;

    switch (cloneRepositoryType) {
    case CloneRepositoryPanel::CheckoutRepository:
        isLocalCheckoutOnly = true;
        break;
    default:
        if (url.isLocalFile() || url.isRelative()) {
            QFileInfo sourcePath(url.path());
            isLocalCheckoutOnly = (sourcePath.canonicalFilePath() == cloneRepository.canonicalFilePath());
        }
        break;
    }

    // set clone repository admin user to configured user name
    // OR override it with the specified user from clone panel
    QString adminUser = optionsPanel->adminUser();
    bool disableAutosync = optionsPanel->isAutosyncOffOptionEnabled();
    QString checkoutBranch = repositoryPanel->checkoutBranch();

    // first create the checkout directory,
    // as it needs to become a working directory for wizard command jobs

    QDir checkoutDir(*checkoutPath);
    checkoutDir.mkpath(*checkoutPath);

    // Setup the wizard page job
    VCSBase::ProcessCheckoutJob *job = new VCSBase::ProcessCheckoutJob;

    QStringList extraOptions;
    QStringList args;

    if (!isLocalCheckoutOnly
        && !cloneRepository.exists()) {
        // SSL Identity
        QString sslIdentityFile = optionsPanel->sslIdentityFile();

        if (optionsPanel->isIncludePrivateOptionEnabled())
            extraOptions << QLatin1String("--private");
        if (!sslIdentityFile.isEmpty())
            extraOptions << QLatin1String("--ssl-identity") << QDir::toNativeSeparators(sslIdentityFile);
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

        args << client->vcsCommandString(FossilClient::CloneCommand)
             << extraOptions
             << page->repository()
             << cloneRepositoryFileNative;

        job->addStep(settings.stringValue(FossilSettings::binaryPathKey), args, *checkoutPath);
    }

    // check out the cloned repository file into the working copy directory;
    // by default the latest revision is checked out

    extraOptions.clear();
    args.clear();

    args << QLatin1String("open") << cloneRepositoryFileNative;

    if (!checkoutBranch.isEmpty())
        args << checkoutBranch;
    args << extraOptions;

    job->addStep(settings.stringValue(FossilSettings::binaryPathKey), args, *checkoutPath);

    // set user default to admin user if specified
    if (!isLocalCheckoutOnly
        && !adminUser.isEmpty()) {

        extraOptions.clear();
        args.clear();

        QString currentUser = adminUser;

        args << QLatin1String("user") << QLatin1String("default") << currentUser
             << QLatin1String("--user") << adminUser;

        job->addStep(settings.stringValue(FossilSettings::binaryPathKey), args, *checkoutPath);
    }

    // turn-off autosync if requested
    if (!isLocalCheckoutOnly
        && disableAutosync) {
        extraOptions.clear();
        args.clear();

        args << QLatin1String("settings")
             << QLatin1String("autosync") << QLatin1String("off");

        job->addStep(settings.stringValue(FossilSettings::binaryPathKey), args, *checkoutPath);
    }

    return QSharedPointer<VCSBase::AbstractCheckoutJob>(job);
}
