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

#include "optionspage.h"
#include "constants.h"
#include "fossilclient.h"
#include "fossilsettings.h"
#include "fossilplugin.h"

#include <coreplugin/icore.h>
#include <utils/pathchooser.h>
#include <vcsbase/vcsbaseconstants.h>

#include <QTextStream>

using namespace Fossil::Internal;
using namespace Fossil;

OptionsPageWidget::OptionsPageWidget(QWidget *parent) :
    VcsClientOptionsPageWidget(parent)
{
    m_ui.setupUi(this);
    m_ui.commandChooser->setExpectedKind(Utils::PathChooser::ExistingCommand);
    m_ui.commandChooser->setPromptDialogTitle(tr("Fossil Command"));
    m_ui.commandChooser->setHistoryCompleter("Fossil.Command.History");
    m_ui.defaultRepoPathChooser->setExpectedKind(Utils::PathChooser::ExistingDirectory);
    m_ui.defaultRepoPathChooser->setPromptDialogTitle(tr("Fossil Repositories"));
    m_ui.sslIdentityFilePathChooser->setExpectedKind(Utils::PathChooser::File);
    m_ui.sslIdentityFilePathChooser->setPromptDialogTitle(tr("SSL/TLS Identity Key"));
}

VcsBase::VcsBaseClientSettings OptionsPageWidget::settings() const
{
    VcsBase::VcsBaseClientSettings s = FossilPlugin::instance()->client()->settings();
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

void OptionsPageWidget::setSettings(const VcsBase::VcsBaseClientSettings &s)
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

OptionsPage::OptionsPage(Core::IVersionControl *control, QObject *parent) :
    VcsClientOptionsPage(control, FossilPlugin::instance()->client(), parent)
{
    setId(Constants::VCS_ID_FOSSIL);
    setDisplayName(tr("Fossil"));
    setWidgetFactory([]() { return new OptionsPageWidget; });
}
