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

#include <QRegExp>
#include <QString>
#include <QTextCursor>
#include <QTextBlock>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

using namespace Fossil::Internal;
using namespace Fossil;

FossilEditorWidget::FossilEditorWidget() :
      m_exactChangesetId(QLatin1String(Constants::CHANGESET_ID_EXACT)),
      m_exactDiffFileId(QLatin1String(Constants::DIFFFILE_ID_EXACT)),
      m_firstChangesetId(QString(QLatin1String("\n%1 ")).arg(QLatin1String(Constants::CHANGESET_ID))),
      m_nextChangesetId(QString(QLatin1String("\n%1 ")).arg(QLatin1String(Constants::CHANGESET_ID)))
{
    QTC_ASSERT(m_exactChangesetId.isValid(), return);
    QTC_ASSERT(m_exactDiffFileId.isValid(), return);
    QTC_ASSERT(m_firstChangesetId.isValid(), return);
    QTC_ASSERT(m_nextChangesetId.isValid(), return);

    setAnnotateRevisionTextFormat(tr("&Annotate %1"));
    setAnnotatePreviousRevisionTextFormat(tr("Annotate &Parent Revision %1"));

    setDiffFilePattern(m_exactDiffFileId);
    QRegExp logChangePattern(QLatin1String("^.*\\[([0-9a-f]{5,40})\\]"));
    QTC_ASSERT(logChangePattern.isValid(), return);
    setLogEntryPattern(logChangePattern);

    //VcsBase::DiffHighlighter *dh = createDiffHighlighter(); // own implementation
    //baseTextDocument()->setSyntaxHighlighter(dh);

    //connect(this, SIGNAL(textChanged()), this, SLOT(slotChangeContents()));
}

//void FossilEditor::slotChangeContents()
//{
//    QPlainTextEdit te(toPlainText().replace(QLatin1Char('\n'), QLatin1String("\r\n")));
//    QByteArray qba(toPlainText().toStdString().c_str());
//    disconnect(this, SIGNAL(textChanged()), this, SLOT(slotChangeContents()));
//    //setPlainText(toPlainText());
//    connect(this, SIGNAL(textChanged()), this, SLOT(slotChangeContents()));
//    qDebug() << "slotChangeContents():: document:" << Utils::SynchronousProcess::normalizeNewlines(toPlainText());
//    //qDebug() << "slotChangeContents():: document:" << qba.toHex();
//}


QSet<QString> FossilEditorWidget::annotationChanges() const
{
    QSet<QString> changes;
    const QString txt = toPlainText();
    if (txt.isEmpty())
        return changes;

    // extract changeset id at the beginning of each annotated line:
    // <changeid> ...:

    if (m_firstChangesetId.indexIn(txt) != -1) {
        changes.insert(m_firstChangesetId.cap(1));
        int pos = 0;
        while ((pos = m_nextChangesetId.indexIn(txt, pos)) != -1) {
            pos += m_nextChangesetId.matchedLength();
            changes.insert(m_nextChangesetId.cap(1));
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
        if (m_exactChangesetId.exactMatch(change))
            return change;
    }
    return QString();
}


VcsBase::BaseAnnotationHighlighter *FossilEditorWidget::createAnnotationHighlighter(const QSet<QString> &changes) const
{
    return new FossilAnnotationHighlighter(changes);
}
