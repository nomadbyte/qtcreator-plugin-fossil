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

#ifndef FOSSILEDITOR_H
#define FOSSILEDITOR_H

#include <vcsbase/vcsbaseeditor.h>

#include <QtCore/QRegExp>

QT_BEGIN_NAMESPACE
class QVariant;
QT_END_NAMESPACE

namespace Fossil {
namespace Internal {

class FossilEditor : public VCSBase::VCSBaseEditorWidget
{
    Q_OBJECT

public:
    explicit FossilEditor(const VCSBase::VCSBaseEditorParameters *type, QWidget *parent);

public slots:
    // Matches the signature of the finished signal of VCSBase::Command
    void commandFinishedGotoLine(bool ok, int exitCode, const QVariant &v);

private:
    QSet<QString> annotationChanges() const;
    QString changeUnderCursor(const QTextCursor &cursor) const;
    VCSBase::DiffHighlighter *createDiffHighlighter() const;
    VCSBase::BaseAnnotationHighlighter *createAnnotationHighlighter(const QSet<QString> &changes) const;
    QString fileNameFromDiffSpecification(const QTextBlock &diffFileSpec) const;

    const QRegExp m_exactChangesetId;
    const QRegExp m_exactDiffFileId;
    const QRegExp m_firstChangesetId;
    const QRegExp m_nextChangesetId;
};

} // namespace Internal
} // namespace Fossil

#endif // FOSSILEDITOR_H
