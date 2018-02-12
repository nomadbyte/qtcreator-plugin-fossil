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

#include "annotationhighlighter.h"
#include "constants.h"

#include <utils/qtcassert.h>

namespace Fossil {
namespace Internal {

FossilAnnotationHighlighter::FossilAnnotationHighlighter(const ChangeNumbers &changeNumbers,
                                                         QTextDocument *document) :
    VcsBase::BaseAnnotationHighlighter(changeNumbers, document),
    m_changesetIdPattern(Constants::CHANGESET_ID)
{
    QTC_CHECK(m_changesetIdPattern.isValid());
}

QString FossilAnnotationHighlighter::changeNumber(const QString &block) const
{
    QRegularExpressionMatch changesetIdMatch = m_changesetIdPattern.match(block);
    if (changesetIdMatch.hasMatch())
        return changesetIdMatch.captured(1);
    return QString();
}

} // namespace Internal
} // namespace Fossil
