/**************************************************************************
**  This file is part of Fossil VCS plugin for Qt Creator
**
**  Copyright (c) 2013 - 2020, Artur Shepilko, <qtc-fossil@nomadbyte.com>.
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

#include "fossileditor.h"
#include "annotationhighlighter.h"
#include "constants.h"
#include "fossilplugin.h"
#include "fossilclient.h"

#include <coreplugin/editormanager/editormanager.h>
#include <utils/qtcassert.h>
#include <utils/synchronousprocess.h>
#include <vcsbase/diffandloghighlighter.h>

#include <QRegularExpression>
#include <QRegExp>
#include <QString>
#include <QTextCursor>
#include <QTextBlock>
#include <QDir>
#include <QFileInfo>

namespace Fossil {
namespace Internal {

class FossilEditorWidgetPrivate
{
public:
    FossilEditorWidgetPrivate() :
        m_exactChangesetId(Constants::CHANGESET_ID_EXACT)
    {
        QTC_ASSERT(m_exactChangesetId.isValid(), return);
    }


    const QRegularExpression m_exactChangesetId;
};

FossilEditorWidget::FossilEditorWidget() :
    d(new FossilEditorWidgetPrivate)
{
    setAnnotateRevisionTextFormat(tr("&Annotate %1"));
    setAnnotatePreviousRevisionTextFormat(tr("Annotate &Parent Revision %1"));
    setDiffFilePattern(Constants::DIFFFILE_ID_EXACT);
    setLogEntryPattern("^.*\\[([0-9a-f]{5,40})\\]");
    setAnnotationEntryPattern(QString("^") + Constants::CHANGESET_ID + " ");
}

FossilEditorWidget::~FossilEditorWidget()
{
    delete d;
}

QString FossilEditorWidget::changeUnderCursor(const QTextCursor &cursorIn) const
{
    QTextCursor cursor = cursorIn;
    cursor.select(QTextCursor::WordUnderCursor);
    if (cursor.hasSelection()) {
        const QString change = cursor.selectedText();
        QRegularExpressionMatch exactChangesetIdMatch = d->m_exactChangesetId.match(change);
        if (exactChangesetIdMatch.hasMatch())
            return change;
    }
    return QString();
}

QString FossilEditorWidget::decorateVersion(const QString &revision) const
{
    static const int shortChangesetIdSize(10);
    static const int maxTextSize(120);

    const QFileInfo fi(source());
    const QString workingDirectory = fi.absolutePath();
    const FossilClient *client = FossilPlugin::client();
    RevisionInfo revisionInfo =
            client->synchronousRevisionQuery(workingDirectory, revision, true);

    // format: 'revision (committer "comment...")'
    QString output = revision.left(shortChangesetIdSize)
            + " (" + revisionInfo.committer
            + " \"" + revisionInfo.commentMsg.left(maxTextSize);

    if (output.size() > maxTextSize) {
        output.truncate(maxTextSize - 3);
        output.append("...");
    }
    output.append("\")");
    return output;
}

QStringList FossilEditorWidget::annotationPreviousVersions(const QString &revision) const
{
    QStringList revisions;
    const QFileInfo fi(source());
    const QString workingDirectory = fi.absolutePath();
    const FossilClient *client = FossilPlugin::client();
    RevisionInfo revisionInfo =
            client->synchronousRevisionQuery(workingDirectory, revision);
    if (revisionInfo.parentId.isEmpty())
        return QStringList();

    revisions.append(revisionInfo.parentId);
    revisions.append(revisionInfo.mergeParentIds);
    return revisions;
}

VcsBase::BaseAnnotationHighlighter *FossilEditorWidget::createAnnotationHighlighter(
        const QSet<QString> &changes) const
{
    return new FossilAnnotationHighlighter(changes);
}

} // namespace Internal
} // namespace Fossil
