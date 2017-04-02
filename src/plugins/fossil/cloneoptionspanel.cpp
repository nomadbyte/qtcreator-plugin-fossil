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

#include "cloneoptionspanel.h"
#include "ui_cloneoptionspanel.h"
#include "fossilplugin.h"
#include "fossilclient.h"
#include "fossilsettings.h"

#include <utils/pathchooser.h>

#include <QtCore/QtDebug>

namespace Fossil {
namespace Internal {

namespace Ui {
class CloneOptionsPanel;
}

struct CloneOptionsDefault
{
    QString sslIdentityFilePath;
    QString adminUser;
    bool disableAutosync;
    bool includePrivate;

    CloneOptionsDefault()
        : disableAutosync(false), includePrivate(false) {}

    CloneOptionsDefault(const FossilSettings &settings) :
        sslIdentityFilePath(settings.stringValue(FossilSettings::sslIdentityFileKey)),
        adminUser(settings.stringValue(FossilSettings::userNameKey)),
        disableAutosync(settings.boolValue(FossilSettings::disableAutosyncKey)),
        includePrivate(false) {}
};

class CloneOptionsPanelPrivate
{
public:
    CloneOptionsPanelPrivate()
        : m_default(FossilPlugin::instance()->settings()) {}

    Ui::CloneOptionsPanel m_ui;
    CloneOptionsDefault m_default;

};

CloneOptionsPanel::CloneOptionsPanel(QWidget *parent)
    : QWidget(parent),
      d(new CloneOptionsPanelPrivate)
{
    d->m_ui.setupUi(this);

    d->m_ui.sslIdentityFilePathChooser->setExpectedKind(Utils::PathChooser::File);
    d->m_ui.sslIdentityFilePathChooser->setPromptDialogTitle(tr("SSL/TLS Identity Key"));
    d->m_ui.sslIdentityFilePathChooser->setPath(d->m_default.sslIdentityFilePath);
    d->m_ui.adminUserLineEdit->setText(d->m_default.adminUser);
    d->m_ui.disableAutosyncCheckBox->setChecked(d->m_default.disableAutosync);
    d->m_ui.includePrivateCheckBox->setChecked(d->m_default.includePrivate);
}

CloneOptionsPanel::~CloneOptionsPanel()
{
    delete d;
}

bool CloneOptionsPanel::isOptionsEnabled() const
{
    return d->m_ui.optionsGroupBox->isChecked();
}

bool CloneOptionsPanel::isIncludePrivateOptionEnabled() const
{
    return isOptionsEnabled() ? d->m_ui.includePrivateCheckBox->isChecked() : d->m_default.includePrivate;
}

bool CloneOptionsPanel::isAutosyncOffOptionEnabled() const
{
    return isOptionsEnabled() ? d->m_ui.disableAutosyncCheckBox->isChecked() : d->m_default.disableAutosync;
}

QString CloneOptionsPanel::sslIdentityFile() const
{
    return isOptionsEnabled() ? d->m_ui.sslIdentityFilePathChooser->path() : d->m_default.sslIdentityFilePath;
}

QString CloneOptionsPanel::adminUser() const
{
    return isOptionsEnabled() ? d->m_ui.adminUserLineEdit->text().simplified() : d->m_default.adminUser;
}

void CloneOptionsPanel::setFields(const QString &sslIdentityFile, const QString &adminUser)
{
    d->m_ui.sslIdentityFilePathChooser->setPath(sslIdentityFile);
    d->m_ui.adminUserLineEdit->setText(adminUser);
}

bool CloneOptionsPanel::isValid() const
{
    return true;
}

} // namespace Internal
} // namespace Fossil
