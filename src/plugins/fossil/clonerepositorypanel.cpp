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

#include "clonerepositorypanel.h"
#include "ui_clonerepositorypanel.h"

#include "constants.h"
#include "fossilplugin.h"
#include "fossilclient.h"
#include "fossilcontrol.h"
#include "fossilsettings.h"

namespace Fossil {
namespace Internal {

namespace Ui {
class CloneRepositoryPanel;
}

class CloneRepositoryPanelPrivate {
public:
    CloneRepositoryPanelPrivate() : m_repositoryNameEdited(false) {}

    void updateUi() {
        if (m_ui.cloneRepoButton->isChecked()) {
            m_ui.checkoutRepoGroup->setEnabled(false);
            m_ui.cloneRepoGroup->setEnabled(true);

        } else if (m_ui.checkoutRepoButton->isChecked()) {
            m_ui.cloneRepoGroup->setEnabled(false);
            m_ui.checkoutRepoGroup->setEnabled(true);
            m_ui.checkoutRepoFilePathChooser->setPath(QString());
            m_ui.checkoutRepoFilePathChooser->setBaseDirectory(m_defaultLocalRepoPath);
        }
    }

    void updateConfiguration() {
        const FossilSettings &settings = FossilPlugin::instance()->settings();
        m_defaultLocalRepoPath = settings.stringValue(FossilSettings::defaultRepoPathKey);
    }

    Ui::CloneRepositoryPanel m_ui;
    QString m_defaultLocalRepoPath;
    bool m_repositoryNameEdited;
};

CloneRepositoryPanel::CloneRepositoryPanel(QWidget *parent) :
    QWidget(parent),
    d(new CloneRepositoryPanelPrivate)
{
    d->m_ui.setupUi(this);

    d->updateConfiguration();

    d->m_ui.checkoutRepoFilePathChooser->setExpectedKind(Utils::PathChooser::File);
    d->m_ui.checkoutRepoFilePathChooser->setBaseDirectory(d->m_defaultLocalRepoPath);
    d->m_ui.checkoutRepoFilePathChooser->setPromptDialogFilter(tr(Constants::FOSSIL_FILE_FILTER));

    connect(d->m_ui.cloneRepoButton, SIGNAL(toggled(bool)),
            this, SLOT(slotCloneRepositoryToggled(bool)));
    connect(d->m_ui.checkoutRepoButton, SIGNAL(toggled(bool)),
            this, SLOT(slotCheckoutRepositoryToggled(bool)));
    connect(d->m_ui.cloneRepoNameLineEdit, SIGNAL(textEdited(QString)),
            this, SLOT(slotCloneRepositoryNameEdited(QString)));
    connect(d->m_ui.checkoutRepoFilePathChooser, SIGNAL(changed(QString)),
            this, SLOT(slotCheckoutRepositoryChanged(QString)));
    connect(d->m_ui.branchLineEdit, SIGNAL(textEdited(QString)),
            this, SLOT(slotBranchEdited(QString)));

    d->updateUi();

    connect(FossilPlugin::instance()->versionControl(), SIGNAL(configurationChanged()),
            this, SLOT(update()));
}

CloneRepositoryPanel::~CloneRepositoryPanel()
{
    delete d;
}

QString CloneRepositoryPanel::cloneRepository() const
{
    switch (cloneRepositoryType()) {
    case CloneRepository:
        if (d->m_ui.cloneRepoNameLineEdit->text().isEmpty())
            return QString();

        return FossilClient::buildPath(d->m_defaultLocalRepoPath,
                                       d->m_ui.cloneRepoNameLineEdit->text().trimmed(),
                                       QLatin1String(Constants::FOSSIL_FILE_SUFFIX));
    case CheckoutRepository:
        return d->m_ui.checkoutRepoFilePathChooser->path().trimmed();
    default:
        return QString();
    }

}

QString CloneRepositoryPanel::cloneRepositoryName() const
{
    QFileInfo file(cloneRepository());

    return file.baseName();
}

CloneRepositoryPanel::RepositoryType CloneRepositoryPanel::cloneRepositoryType() const
{
    if (d->m_ui.cloneRepoButton->isChecked())
        return CloneRepository;
    else if (d->m_ui.checkoutRepoButton->isChecked())
        return CheckoutRepository;
    else
        return InvalidRepository;
}

QString CloneRepositoryPanel::checkoutBranch() const
{
    return d->m_ui.branchLineEdit->text().trimmed();
}

bool CloneRepositoryPanel::isValid() const
{
    return (!cloneRepository().isEmpty()
            && cloneRepositoryType() != InvalidRepository);
}

void CloneRepositoryPanel::update()
{
    d->updateConfiguration();
    d->updateUi();
}

void CloneRepositoryPanel::triggerRepositoryNameChange(const QString &name)
{
    if (!d->m_repositoryNameEdited)
        d->m_ui.cloneRepoNameLineEdit->setText(name);
}

void CloneRepositoryPanel::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        d->m_ui.retranslateUi(this);
        break;
    default:
        break;
    }
}

void CloneRepositoryPanel::slotCloneRepositoryToggled(bool toggled)
{
    Q_UNUSED(toggled);
    d->updateUi();
    emit cloneRepositoryNameEdited(cloneRepositoryName());
}

void CloneRepositoryPanel::slotCheckoutRepositoryToggled(bool toggled)
{
    Q_UNUSED(toggled);
    d->updateUi();
}

void CloneRepositoryPanel::slotCloneRepositoryNameEdited(const QString &name)
{
    d->m_repositoryNameEdited = true;
    emit cloneRepositoryNameEdited(name);
}

void CloneRepositoryPanel::slotCheckoutRepositoryChanged(const QString &path)
{
    emit checkoutRepositoryEdited(path.trimmed());
}

void CloneRepositoryPanel::slotBranchEdited(const QString &branch)
{
    emit checkoutBranchEdited(branch);
}

} // namespace Internal
} // namespace Fossil
