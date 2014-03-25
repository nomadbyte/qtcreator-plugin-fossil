/**************************************************************************
**  This file is part of Fossil VCS plugin for Qt Creator
**
**  Copyright (c) 2013 - 2014, Artur Shepilko, <qtc-fossil@nomadbyte.com>.
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

#include "fossilcommitwidget.h"
#include "branchinfo.h"

#include <texteditor/texteditorsettings.h>
#include <texteditor/fontsettings.h>
#include <texteditor/texteditorconstants.h>

#include <utils/qtcassert.h>

#include <QSyntaxHighlighter>
#include <QTextEdit>

#include <QDebug>
#include <QRegExp>
#include <QDir>

//TODO: Check to see when the Highlighter has been moved to a base class and use that instead

namespace Fossil {
namespace Internal {

// Retrieve the comment char format from the text editor.
static QTextCharFormat commentFormat()
{
    const TextEditor::FontSettings settings = TextEditor::TextEditorSettings::instance()->fontSettings();
    return settings.toTextCharFormat(TextEditor::C_COMMENT);
}

// Highlighter for Fossil submit messages.
// Marks up [ticket-id] fields in the message.
class FossilSubmitHighlighter : QSyntaxHighlighter
{
public:
    explicit FossilSubmitHighlighter(QTextEdit *parent);
    void highlightBlock(const QString &text);

private:
    const QTextCharFormat m_commentFormat;
    const QRegExp m_keywordRx;
};

FossilSubmitHighlighter::FossilSubmitHighlighter(QTextEdit * parent) :
    QSyntaxHighlighter(parent),
    m_commentFormat(commentFormat()),
    m_keywordRx(QLatin1String("\\[([0-9a-f]{5,40})\\]"))
{
    QTC_CHECK(m_keywordRx.isValid());
}

void FossilSubmitHighlighter::highlightBlock(const QString &text)
{
    // Fossil commit message allows listing of [ticket-id],
    // where ticket-id is a partial SHA1.
    // Match the ticket-ids and highlight them for convenience.

    //const QTextBlock block = currentBlock();

    // Format keywords
    for (int start = 0;
         (start = m_keywordRx.indexIn(text, start)) != -1;
         start += m_keywordRx.matchedLength()) {
        QTextCharFormat charFormat = format(0);
        //charFormat.setForeground(Qt::darkGreen);
        //charFormat.setFontWeight(QFont::DemiBold);
        charFormat.setFontItalic(true);
        setFormat(start, m_keywordRx.matchedLength(), charFormat);
    }
}


FossilCommitWidget::FossilCommitWidget(QWidget *parent) :
    VcsBase::SubmitEditorWidget(parent),
    m_spaceChar(QLatin1Char(' ')),
    m_commaChar(QLatin1Char(',')),
    m_tagsSeparator(QString(m_commaChar).append(m_spaceChar)),
    m_commitPanel(new QWidget)
{
    m_commitPanelUi.setupUi(m_commitPanel);
    insertTopWidget(m_commitPanel);
    new FossilSubmitHighlighter(descriptionEdit());
    m_branchValidator = new QRegExpValidator(QRegExp(QLatin1String("[^\\n]*")), this);

    connect(m_commitPanelUi.branchLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(branchChanged()));
}

void FossilCommitWidget::setFields(const QString &repoPath, const BranchInfo &branch,
                                   const QStringList &tags, const QString &userName)
{
    m_commitPanelUi.localRootLineEdit->setText(QDir::toNativeSeparators(repoPath));
    m_commitPanelUi.currentBranchLineEdit->setText(branch.name());
    QString tagsText = tags.join(m_tagsSeparator);
    m_commitPanelUi.currentTagsLineEdit->setText(tagsText);
    m_commitPanelUi.authorLineEdit->setText(userName);

    branchChanged();
}

QString FossilCommitWidget::newBranch() const
{
    const QString branchName = m_commitPanelUi.branchLineEdit->text().trimmed();
    return branchName;
}

QStringList FossilCommitWidget::tags() const
{
    QString tagsText = m_commitPanelUi.tagsLineEdit->text().trimmed();
    if (tagsText.isEmpty())
        return QStringList();

    QStringList tags;

    tagsText.replace(m_commaChar, m_spaceChar);
    tags = tagsText.split(m_spaceChar, QString::SkipEmptyParts);

    return tags;
}

QString FossilCommitWidget::committer() const
{
    const QString author = m_commitPanelUi.authorLineEdit->text();
    if (author.isEmpty())
        return QString();

    QString user = author;
    return user;
}

bool FossilCommitWidget::isPrivateOptionEnabled() const
{
    return m_commitPanelUi.isPrivateCheckBox->isChecked();
}

bool FossilCommitWidget::canSubmit() const
{
    QString message = cleanupDescription(descriptionText()).trimmed();

    if (m_commitPanelUi.invalidBranchLabel->isVisible() || message.isEmpty())
        return false;

    return VcsBase::SubmitEditorWidget::canSubmit();
}

void FossilCommitWidget::branchChanged()
{
    m_commitPanelUi.invalidBranchLabel->setVisible(!isValidBranch());

    updateSubmitAction();
}

bool FossilCommitWidget::isValidBranch() const
{
    int pos = m_commitPanelUi.branchLineEdit->cursorPosition();
    QString text = m_commitPanelUi.branchLineEdit->text();
    return m_branchValidator->validate(text, pos) == QValidator::Acceptable;
}

} // namespace Internal
} // namespace Fossil
