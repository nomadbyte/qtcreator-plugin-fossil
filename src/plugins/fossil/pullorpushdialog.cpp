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

#include "pullorpushdialog.h"
#include "ui_pullorpushdialog.h"

#include "constants.h"

#include <utils/qtcassert.h>


using namespace Fossil::Internal;

PullOrPushDialog::PullOrPushDialog(Mode mode, QWidget *parent) :
    QDialog(parent),
    m_mode(mode),
    m_ui(new Ui::PullOrPushDialog)
{
    m_ui->setupUi(this);
    m_ui->localPathChooser->setExpectedKind(Utils::PathChooser::File);
    m_ui->localPathChooser->setPromptDialogFilter(tr(Constants::FOSSIL_FILE_FILTER));

    switch (m_mode) {
    case PullMode:
        this->setWindowTitle(tr("Pull Source"));
        break;
    case PushMode:
        this->setWindowTitle(tr("Push Destination"));
        break;
    }

    // select URL text in line edit when clicking the radio button
    m_ui->localButton->setFocusProxy(m_ui->localPathChooser);
    m_ui->urlButton->setFocusProxy(m_ui->urlLineEdit);
    connect(m_ui->urlButton, SIGNAL(clicked(bool)), m_ui->urlLineEdit, SLOT(selectAll()));

    this->adjustSize();
}

PullOrPushDialog::~PullOrPushDialog()
{
    delete m_ui;
}

QString PullOrPushDialog::remoteLocation() const
{
    if (m_ui->defaultButton->isChecked())
        return QString();
    if (m_ui->localButton->isChecked())
        return m_ui->localPathChooser->path();
    return m_ui->urlLineEdit->text();
}

bool PullOrPushDialog::isRememberOptionEnabled() const
{
    if (m_ui->defaultButton->isChecked())
        return false;
    return m_ui->rememberCheckBox->isChecked();
}

bool PullOrPushDialog::isPrivateOptionEnabled() const
{
    return m_ui->privateCheckBox->isChecked();
}

void PullOrPushDialog::setDefaultRemoteLocation(const QString &url)
{
    m_ui->urlLineEdit->setText(url);
}

void PullOrPushDialog::setLocalBaseDirectory(const QString &dir)
{
    m_ui->localPathChooser->setBaseDirectory(dir);
}

void PullOrPushDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
