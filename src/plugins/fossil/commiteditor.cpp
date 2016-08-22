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

#include "branchinfo.h"
#include "commiteditor.h"
#include "constants.h"
#include "fossilcommitwidget.h"

#include <coreplugin/idocument.h>
#include <vcsbase/submitfilemodel.h>

#include <QDebug>


using namespace Fossil::Internal;

CommitEditor::CommitEditor(const VcsBase::VcsBaseSubmitEditorParameters *parameters) :
    VcsBase::VcsBaseSubmitEditor(parameters, new FossilCommitWidget)
{
    document()->setPreferredDisplayName(tr("Commit Editor"));
}

FossilCommitWidget *CommitEditor::commitWidget()
{
    return static_cast<FossilCommitWidget *>(widget());
}

void CommitEditor::setFields(const QString &repositoryRoot, const BranchInfo &branch,
                             const QStringList &tags, const QString &userName,
                             const QList<VcsBase::VcsBaseClient::StatusItem> &repoStatus)
{
    FossilCommitWidget *fossilWidget = commitWidget();
    if (!fossilWidget)
        return;

    fossilWidget->setFields(repositoryRoot, branch, tags, userName);

    m_fileModel = new VcsBase::SubmitFileModel(this);
    m_fileModel->setRepositoryRoot(repositoryRoot);
    m_fileModel->setFileStatusQualifier([](const QString &status, const QVariant &)
                                           -> VcsBase::SubmitFileModel::FileStatusHint
    {
        if (status == QLatin1String(Constants::FSTATUS_ADDED)
            || status == QLatin1String(Constants::FSTATUS_ADDED_BY_MERGE)
            || status == QLatin1String(Constants::FSTATUS_ADDED_BY_INTEGRATE)) {
            return VcsBase::SubmitFileModel::FileAdded;
        } else if (status == QLatin1String(Constants::FSTATUS_EDITED)
            || status == QLatin1String(Constants::FSTATUS_UPDATED_BY_MERGE)
            || status == QLatin1String(Constants::FSTATUS_UPDATED_BY_INTEGRATE)) {
            return VcsBase::SubmitFileModel::FileModified;
        } else if (status == QLatin1String(Constants::FSTATUS_DELETED)) {
            return VcsBase::SubmitFileModel::FileDeleted;
        } else if (status == QLatin1String(Constants::FSTATUS_RENAMED)) {
            return VcsBase::SubmitFileModel::FileRenamed;
        }
        return VcsBase::SubmitFileModel::FileStatusUnknown;
    } );

    foreach (const VcsBase::VcsBaseClient::StatusItem &item, repoStatus)
        if (item.flags != QLatin1String(Constants::FSTATUS_UNKNOWN))
            m_fileModel->addFile(item.file, item.flags);
    setFileModel(m_fileModel);
}
