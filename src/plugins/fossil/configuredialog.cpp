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

#include "configuredialog.h"
#include "ui_configuredialog.h"

#include "fossilsettings.h"

#include <QtCore/QDir>

namespace Fossil {
namespace Internal {

class ConfigureDialogPrivate {
public:
    ConfigureDialogPrivate() {}

    RepositorySettings settings() {
        m_settings.user = m_ui.userLineEdit->text().trimmed();
        m_settings.sslIdentityFile = m_ui.sslIdentityFilePathChooser->path();
        m_settings.autosync =
                (m_ui.disableAutosyncCheckBox->isChecked() ? RepositorySettings::AutosyncOff
                                                           : RepositorySettings::AutosyncOn);
        return m_settings;
    }

    void updateUi() {
        m_ui.userLineEdit->setText(m_settings.user.trimmed());
        m_ui.userLineEdit->selectAll();
        m_ui.sslIdentityFilePathChooser->setPath(QDir::toNativeSeparators(m_settings.sslIdentityFile));
        m_ui.disableAutosyncCheckBox->setChecked(m_settings.autosync == RepositorySettings::AutosyncOff);
    }

    Ui::ConfigureDialog m_ui;
    RepositorySettings m_settings;
};

ConfigureDialog::ConfigureDialog(QWidget *parent):
    QDialog(parent),
    d(new ConfigureDialogPrivate)
{
    d->m_ui.setupUi(this);
    d->m_ui.sslIdentityFilePathChooser->setExpectedKind(Utils::PathChooser::File);
    d->m_ui.sslIdentityFilePathChooser->setPromptDialogTitle(tr("SSL/TLS Identity Key"));
    setWindowTitle(tr("Configure Repository"));
    d->updateUi();
}

ConfigureDialog::~ConfigureDialog()
{
    delete d;
}

const RepositorySettings ConfigureDialog::settings() const
{
    return d->settings();
}

void ConfigureDialog::setSettings(const RepositorySettings &settings)
{
    d->m_settings = settings;
    d->updateUi();
}

void ConfigureDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        d->m_ui.retranslateUi(this);
        break;
    default:
        break;
    }
}

} // namespace Internal
} // namespace Fossil
