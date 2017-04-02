/**************************************************************************
**  This file is part of Fossil VCS plugin for Qt Creator
**
**  Copyright (c) 2013 - 2017, Artur Shepilko <qtc-fossil@nomadbyte.com>.
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

#include "clonewizardpage.h"
#include "cloneoptionspanel.h"
#include "clonerepositorypanel.h"

#include <utils/qtcassert.h>

#include <QtCore/QUrl>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QDebug>

using namespace Fossil::Internal;

CloneWizardPage::CloneWizardPage(QWidget *parent)
    : VCSBase::BaseCheckoutWizardPage(parent),
      m_repositoryPanel(new CloneRepositoryPanel),
      m_optionsPanel(new CloneOptionsPanel)
{
    setTitle(tr("Location"));
    setSubTitle(tr("Specify repository URL, local clone fossil, and checkout destination."));
    setRepositoryLabel(tr("Clone URL:"));
    setBranchSelectorVisible(false);
    addRepositoryControl(m_repositoryPanel);
    addLocalControl(m_optionsPanel);

    connect(this, SIGNAL(checkoutDirectoryNameGenerated(QString, QString)),
            m_repositoryPanel, SLOT(triggerRepositoryNameChange(QString)));
    connect(m_repositoryPanel, SIGNAL(cloneRepositoryNameEdited(QString)),
            this, SLOT(slotCloneRepositoryNameEdited(QString)));
    connect(m_repositoryPanel, SIGNAL(checkoutRepositoryEdited(QString)),
            this, SLOT(slotCheckoutRepositoryEdited(QString)));
    connect(m_repositoryPanel, SIGNAL(checkoutBranchEdited(QString)),
            this, SLOT(slotCheckoutBranchEdited()));
}

const CloneRepositoryPanel *CloneWizardPage::cloneRepositoryPanel() const
{
    return m_repositoryPanel;
}

const CloneOptionsPanel *CloneWizardPage::cloneOptionsPanel() const
{
    return m_optionsPanel;
}

QString CloneWizardPage::directoryFromRepository(const QString &repository) const
{
    // URL must be in one of the following form: ([...] mean optional)
    //   HTTP/HTTPS protocol:
    //     http[s]://[userid[:password]@]host[:port][/path]
    //   SSH protocol:
    //     ssh://[userid[:password]@]host[:port]/path/to/repo.fossil[?fossil=path/to/fossil.exe]
    //   Filesystem:
    //     [file://]path/to/repo.fossil
    //   Note: For ssh and filesystem, path must have an extra leading
    //         '/' to use an absolute path.
    // Match:
    // -- the repository basename when given
    // -- or the directory name
    // -- otherwise default to hostname
    // When branch is specified -- append.

    QString result;
    QString localName;
    QString remoteName = repository.trimmed();
    QString branch = m_repositoryPanel->checkoutBranch();


    // chop off a possible trailing slash
    if (remoteName.endsWith(QLatin1Char('/')))
        remoteName.remove(remoteName.size() - 1);

    const QRegExp fossilFileRx(QLatin1String("(.*)\\.(fossil|fsl|fos)"));
    QTC_ASSERT(fossilFileRx.isValid(), return QString());


    QString host, dirname, file;

    QUrl url(remoteName);

    if (!url.isRelative())
        host = url.host();

    if (!url.path().isEmpty()) {
        // avoid getting "." for empty path
        QFileInfo fileInfo(url.path());
        QDir dir(fileInfo.path());
        if (!dir.isRoot())
            dirname = dir.dirName();
        file = fileInfo.fileName();
    }

    if (!file.isEmpty())
        localName = file;
    else if (!dirname.isEmpty())
        localName = dirname;
    else if (!host.isEmpty())
        localName = host;

    // chop off the fossil extension if any
    if (fossilFileRx.exactMatch(localName))
        localName = fossilFileRx.cap(1);

    // replace disallowed characters
    const QRegExp disallowedRx(QLatin1String("[\\W]"));
    const QChar dashChar(QLatin1Char('-'));
    QTC_ASSERT(fossilFileRx.isValid(), return QString());

    localName.replace(disallowedRx, QString(dashChar));

    // announce the checkout directory name change
    emit checkoutDirectoryNameGenerated(localName, branch);

    // now use the updated clone repository name
    // append branch when given
    result = localName;
    if (!m_repositoryPanel->cloneRepositoryName().isEmpty())
        result = m_repositoryPanel->cloneRepositoryName();

    if (!result.isEmpty() && !branch.isEmpty())
        result.append(QString("-%1").arg(branch));

    return result;
}

bool CloneWizardPage::checkIsValid() const
{
    return m_repositoryPanel->isValid()
           && m_optionsPanel->isValid()
           && BaseCheckoutWizardPage::checkIsValid();
}

void CloneWizardPage::slotCloneRepositoryNameEdited(const QString &name)
{
    QString result = name;
    QString branch = m_repositoryPanel->checkoutBranch();

    if (!result.isEmpty() && !branch.isEmpty())
        result.append(QString("-%1").arg(branch));

    setDirectory(result);
}

void CloneWizardPage::slotCheckoutRepositoryEdited(const QString &path)
{
    setRepository(path);
}

void CloneWizardPage::slotCheckoutBranchEdited()
{
    // force a change of the repository URL to re-trigger directoryFromRepository()
    QString file = repository();
    setRepository(QString());
    setRepository(file);
}
