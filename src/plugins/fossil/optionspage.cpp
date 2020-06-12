/**************************************************************************
**  This file is part of Fossil VCS plugin for Qt Creator
**
**  Copyright (c) 2013 - 2020, Artur Shepilko, <qtc-fossil@nomadbyte.com>.
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
#include "ui_optionspage.h"

#include <coreplugin/icore.h>
#include <utils/pathchooser.h>
#include <vcsbase/vcsbaseconstants.h>

namespace Fossil {
namespace Internal {

class OptionsPageWidget final : public Core::IOptionsPageWidget
{
    Q_DECLARE_TR_FUNCTIONS(Fossil::Internal::OptionsPageWidget)

public:
    OptionsPageWidget(const std::function<void()> &onApply, FossilSettings *settings);
    void apply() final;

private:
    Ui::OptionsPage m_ui;
    const std::function<void()> m_onApply;
    FossilSettings *m_settings;
};

void OptionsPageWidget::apply()
{
    FossilSettings s = *m_settings;
    s.setValue(FossilSettings::binaryPathKey, m_ui.commandChooser->rawPath());
    s.setValue(FossilSettings::defaultRepoPathKey, m_ui.defaultRepoPathChooser->path());
    s.setValue(FossilSettings::userNameKey, m_ui.defaultUsernameLineEdit->text().trimmed());
    s.setValue(FossilSettings::sslIdentityFileKey, m_ui.sslIdentityFilePathChooser->path());
    s.setValue(FossilSettings::logCountKey, m_ui.logEntriesCount->value());
    s.setValue(FossilSettings::timelineWidthKey, m_ui.logEntriesWidth->value());
    s.setValue(FossilSettings::timeoutKey, m_ui.timeout->value());
    s.setValue(FossilSettings::disableAutosyncKey, m_ui.disableAutosyncCheckBox->isChecked());
    if (*m_settings == s)
        return;

    *m_settings = s;
    m_onApply();
}

OptionsPageWidget::OptionsPageWidget(const std::function<void()> &onApply, FossilSettings *settings) :
    m_onApply(onApply),
    m_settings(settings)
{
    m_ui.setupUi(this);
    m_ui.commandChooser->setExpectedKind(Utils::PathChooser::ExistingCommand);
    m_ui.commandChooser->setPromptDialogTitle(tr("Fossil Command"));
    m_ui.commandChooser->setHistoryCompleter("Fossil.Command.History");
    m_ui.commandChooser->setPath(m_settings->stringValue(FossilSettings::binaryPathKey));
    m_ui.defaultRepoPathChooser->setExpectedKind(Utils::PathChooser::ExistingDirectory);
    m_ui.defaultRepoPathChooser->setPromptDialogTitle(tr("Fossil Repositories"));
    m_ui.defaultRepoPathChooser->setPath(m_settings->stringValue(FossilSettings::defaultRepoPathKey));
    m_ui.sslIdentityFilePathChooser->setExpectedKind(Utils::PathChooser::File);
    m_ui.sslIdentityFilePathChooser->setPromptDialogTitle(tr("SSL/TLS Identity Key"));
    m_ui.sslIdentityFilePathChooser->setPath(m_settings->stringValue(FossilSettings::sslIdentityFileKey));
    m_ui.defaultUsernameLineEdit->setText(m_settings->stringValue(FossilSettings::userNameKey));
    m_ui.logEntriesCount->setValue(m_settings->intValue(FossilSettings::logCountKey));
    m_ui.logEntriesWidth->setValue(m_settings->intValue(FossilSettings::timelineWidthKey));
    m_ui.timeout->setValue(m_settings->intValue(FossilSettings::timeoutKey));
    m_ui.disableAutosyncCheckBox->setChecked(m_settings->boolValue(FossilSettings::disableAutosyncKey));
}

OptionsPage::OptionsPage(const std::function<void()> &onApply, FossilSettings *settings)
{
    setId(Constants::VCS_ID_FOSSIL);
    setDisplayName(OptionsPageWidget::tr("Fossil"));
    setWidgetCreator([onApply, settings]() { return new OptionsPageWidget(onApply, settings); });
    setCategory(VcsBase::Constants::VCS_SETTINGS_CATEGORY);
}

} // Internal
} // Fossil
