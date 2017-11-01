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
        m_exactChangesetId(Constants::CHANGESET_ID_EXACT),
        m_firstChangesetId(QString("\n") + Constants::CHANGESET_ID + " "),
        m_nextChangesetId(m_firstChangesetId)
    {
        QTC_ASSERT(m_exactChangesetId.isValid(), return);
        QTC_ASSERT(m_firstChangesetId.isValid(), return);
        QTC_ASSERT(m_nextChangesetId.isValid(), return);
    }


    const QRegularExpression m_exactChangesetId;
    const QRegularExpression m_firstChangesetId;
    const QRegularExpression m_nextChangesetId;
};

FossilEditorWidget::FossilEditorWidget() :
    d(new FossilEditorWidgetPrivate)
{
    setAnnotateRevisionTextFormat(tr("&Annotate %1"));
    setAnnotatePreviousRevisionTextFormat(tr("Annotate &Parent Revision %1"));

    const QRegExp exactDiffFileIdPattern(Constants::DIFFFILE_ID_EXACT);
    QTC_ASSERT(exactDiffFileIdPattern.isValid(), return);
    setDiffFilePattern(exactDiffFileIdPattern);

    const QRegExp logChangePattern("^.*\\[([0-9a-f]{5,40})\\]");
    QTC_ASSERT(logChangePattern.isValid(), return);
    setLogEntryPattern(logChangePattern);
}

FossilEditorWidget::~FossilEditorWidget()
{
    delete d;
}

QSet<QString> FossilEditorWidget::annotationChanges() const
{

    const QString txt = toPlainText();
    if (txt.isEmpty())
        return QSet<QString>();

    // extract changeset id at the beginning of each annotated line:
    // <changeid> ...:

    QSet<QString> changes;

    QRegularExpressionMatch firstChangesetIdMatch = d->m_firstChangesetId.match(txt);
    if (firstChangesetIdMatch.hasMatch()) {
        QString changeId = firstChangesetIdMatch.captured(1);
        changes.insert(changeId);

        QRegularExpressionMatchIterator i = d->m_nextChangesetId.globalMatch(txt);
        while (i.hasNext()) {
            const QRegularExpressionMatch nextChangesetIdMatch = i.next();
            changeId = nextChangesetIdMatch.captured(1);
            changes.insert(changeId);
        }
    }
    return changes;
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


VcsBase::BaseAnnotationHighlighter *FossilEditorWidget::createAnnotationHighlighter(const QSet<QString> &changes) const
{
    return new FossilAnnotationHighlighter(changes);
}

} // namespace Internal
} // namespace Fossil
