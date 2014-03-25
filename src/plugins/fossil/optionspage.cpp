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

#include "optionspage.h"
#include "constants.h"
#include "fossilsettings.h"
#include "fossilplugin.h"

#include <coreplugin/icore.h>
#include <utils/pathchooser.h>
#include <vcsbase/vcsbaseconstants.h>

#include <QTextStream>

using namespace Fossil::Internal;
using namespace Fossil;

OptionsPageWidget::OptionsPageWidget(QWidget *parent)
    : QWidget(parent)
{
    m_ui.setupUi(this);
    m_ui.commandChooser->setExpectedKind(Utils::PathChooser::ExistingCommand);
    m_ui.commandChooser->setPromptDialogTitle(tr("Fossil Command"));
    m_ui.defaultRepoPathChooser->setExpectedKind(Utils::PathChooser::ExistingDirectory);
    m_ui.defaultRepoPathChooser->setPromptDialogTitle(tr("Fossil Repositories"));
    m_ui.sslIdentityFilePathChooser->setExpectedKind(Utils::PathChooser::File);
    m_ui.sslIdentityFilePathChooser->setPromptDialogTitle(tr("SSL/TLS Identity Key"));
}

FossilSettings OptionsPageWidget::settings() const
{
    FossilSettings s = FossilPlugin::instance()->settings();
    s.setValue(FossilSettings::binaryPathKey, m_ui.commandChooser->rawPath());
    s.setValue(FossilSettings::defaultRepoPathKey, m_ui.defaultRepoPathChooser->path());
    s.setValue(FossilSettings::userNameKey, m_ui.defaultUsernameLineEdit->text().trimmed());
    s.setValue(FossilSettings::sslIdentityFileKey, m_ui.sslIdentityFilePathChooser->path());
    s.setValue(FossilSettings::logCountKey, m_ui.logEntriesCount->value());
    s.setValue(FossilSettings::timelineWidthKey, m_ui.logEntriesWidth->value());
    s.setValue(FossilSettings::timeoutKey, m_ui.timeout->value());
    s.setValue(FossilSettings::disableAutosyncKey, m_ui.disableAutosyncCheckBox->isChecked());
    return s;
}

void OptionsPageWidget::setSettings(const FossilSettings &s)
{
    m_ui.commandChooser->setPath(s.stringValue(FossilSettings::binaryPathKey));
    m_ui.defaultRepoPathChooser->setPath(s.stringValue(FossilSettings::defaultRepoPathKey));
    m_ui.defaultUsernameLineEdit->setText(s.stringValue(FossilSettings::userNameKey));
    m_ui.sslIdentityFilePathChooser->setPath(s.stringValue(FossilSettings::sslIdentityFileKey));
    m_ui.logEntriesCount->setValue(s.intValue(FossilSettings::logCountKey));
    m_ui.logEntriesWidth->setValue(s.intValue(FossilSettings::timelineWidthKey));
    m_ui.timeout->setValue(s.intValue(FossilSettings::timeoutKey));
    m_ui.disableAutosyncCheckBox->setChecked(s.boolValue(FossilSettings::disableAutosyncKey));
}

QString OptionsPageWidget::searchKeywords() const
{
    QString rc;
    QLatin1Char sep(' ');
    QTextStream(&rc)
            << sep << m_ui.configGroupBox->title()
            << sep << m_ui.commandLabel->text()
            << sep << m_ui.repoGroupBox->title()
            << sep << m_ui.defaultRepoPathLabel->text()
            << sep << m_ui.userGroupBox->title()
            << sep << m_ui.defaultUsernameLabel->text()
            << sep << m_ui.sslIdentityFileLabel->text()
            << sep << m_ui.miscGroupBox->title()
            << sep << m_ui.showLogEntriesLabel->text()
            << sep << m_ui.timeoutSecondsLabel->text()
            << sep << m_ui.disableAutosyncCheckBox->text()
               ;
    rc.remove(QLatin1Char('&'));
    return rc;
}

OptionsPage::OptionsPage()
{
    setId(Constants::VCS_ID_FOSSIL);
    setDisplayName(tr("Fossil"));
}

QWidget *OptionsPage::createPage(QWidget *parent)
{
    if (!m_optionsPageWidget)
        m_optionsPageWidget = new OptionsPageWidget(parent);
    m_optionsPageWidget->setSettings(FossilPlugin::instance()->settings());
    if (m_searchKeywords.isEmpty())
        m_searchKeywords = m_optionsPageWidget->searchKeywords();
    return m_optionsPageWidget;
}

void OptionsPage::apply()
{
    if (!m_optionsPageWidget)
        return;
    FossilPlugin *plugin = FossilPlugin::instance();
    const FossilSettings newSettings = m_optionsPageWidget->settings();
    if (newSettings != plugin->settings()) {
        //assume success and emit signal that settings are changed;
        plugin->setSettings(newSettings);
        newSettings.writeSettings(Core::ICore::settings());
        emit settingsChanged();
    }
}

bool OptionsPage::matches(const QString &s) const
{
    return m_searchKeywords.contains(s, Qt::CaseInsensitive);
}
