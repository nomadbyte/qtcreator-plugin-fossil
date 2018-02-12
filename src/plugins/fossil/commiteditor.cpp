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

#include "branchinfo.h"
#include "commiteditor.h"
#include "constants.h"
#include "fossilcommitwidget.h"

#include <coreplugin/idocument.h>
#include <vcsbase/submitfilemodel.h>
#include <utils/algorithm.h>
#include <utils/qtcassert.h>

namespace Fossil {
namespace Internal {

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
    QTC_ASSERT(fossilWidget, return);

    fossilWidget->setFields(repositoryRoot, branch, tags, userName);

    m_fileModel = new VcsBase::SubmitFileModel(this);
    m_fileModel->setRepositoryRoot(repositoryRoot);
    m_fileModel->setFileStatusQualifier([](const QString &status, const QVariant &)
                                           -> VcsBase::SubmitFileModel::FileStatusHint
    {
        if (status == Constants::FSTATUS_ADDED
            || status == Constants::FSTATUS_ADDED_BY_MERGE
            || status == Constants::FSTATUS_ADDED_BY_INTEGRATE) {
            return VcsBase::SubmitFileModel::FileAdded;
        } else if (status == Constants::FSTATUS_EDITED
            || status == Constants::FSTATUS_UPDATED_BY_MERGE
            || status == Constants::FSTATUS_UPDATED_BY_INTEGRATE) {
            return VcsBase::SubmitFileModel::FileModified;
        } else if (status == Constants::FSTATUS_DELETED) {
            return VcsBase::SubmitFileModel::FileDeleted;
        } else if (status == Constants::FSTATUS_RENAMED) {
            return VcsBase::SubmitFileModel::FileRenamed;
        }
        return VcsBase::SubmitFileModel::FileStatusUnknown;
    } );

    const QList<VcsBase::VcsBaseClient::StatusItem> toAdd = Utils::filtered(repoStatus,
                [](const VcsBase::VcsBaseClient::StatusItem &item) { return item.flags != Constants::FSTATUS_UNKNOWN; });
    for (const VcsBase::VcsBaseClient::StatusItem &item : toAdd)
            m_fileModel->addFile(item.file, item.flags);

    setFileModel(m_fileModel);
}

} // namespace Internal
} // namespace Fossil
