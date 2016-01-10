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

#include "fossileditor.h"
#include "annotationhighlighter.h"
#include "constants.h"
#include "fossilplugin.h"
#include "fossilclient.h"

#include <coreplugin/editormanager/editormanager.h>
#include <utils/qtcassert.h>
#include <vcsbase/diffhighlighter.h>

#include <QtCore/QRegExp>
#include <QtCore/QString>
#include <QtGui/QTextCursor>
#include <QtGui/QTextBlock>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QDebug>

using namespace Fossil::Internal;
using namespace Fossil;

FossilEditor::FossilEditor(const VCSBase::VCSBaseEditorParameters *type, QWidget *parent)
    : VCSBase::VCSBaseEditorWidget(type, parent),
      m_exactChangesetId(QLatin1String(Constants::CHANGESET_ID_EXACT)),
      m_exactDiffFileId(QLatin1String(Constants::DIFFFILE_ID_EXACT)),
      m_firstChangesetId(QString("\n%1 ").arg(QLatin1String(Constants::CHANGESET_ID))),
      m_nextChangesetId(QString("\n%1 ").arg(QLatin1String(Constants::CHANGESET_ID)))
{
    QTC_ASSERT(m_exactChangesetId.isValid(), return);
    QTC_ASSERT(m_exactDiffFileId.isValid(), return);
    QTC_ASSERT(m_firstChangesetId.isValid(), return);
    QTC_ASSERT(m_nextChangesetId.isValid(), return);

    setAnnotateRevisionTextFormat(tr("Annotate %1"));
    setAnnotatePreviousRevisionTextFormat(tr("Annotate parent revision %1"));
}

QSet<QString> FossilEditor::annotationChanges() const
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

QString FossilEditor::changeUnderCursor(const QTextCursor &cursorIn) const
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

VCSBase::DiffHighlighter *FossilEditor::createDiffHighlighter() const
{
    return new VCSBase::DiffHighlighter(m_exactDiffFileId);
}

VCSBase::BaseAnnotationHighlighter *FossilEditor::createAnnotationHighlighter(const QSet<QString> &changes) const
{
    return new FossilAnnotationHighlighter(changes);
}

QString FossilEditor::fileNameFromDiffSpecification(const QTextBlock &inBlock) const
{
    // Check for:
    // +++ filename.cpp
    for (QTextBlock  block = inBlock; block.isValid(); block = block.previous()) {
        const QString line = block.text();
        if (m_exactDiffFileId.exactMatch(line))
            return findDiffFile(m_exactDiffFileId.cap(1), FossilPlugin::instance()->versionControl());
    }

    return QString();
}
