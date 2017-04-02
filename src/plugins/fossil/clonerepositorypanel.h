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

#ifndef CLONEREPOSITORYPANEL_H
#define CLONEREPOSITORYPANEL_H

#include <QtGui/QWidget>

namespace Fossil {
namespace Internal {

class CloneRepositoryPanelPrivate;

class CloneRepositoryPanel : public QWidget
{
    Q_OBJECT
public:
    enum RepositoryType { InvalidRepository = 0, CloneRepository, CheckoutRepository };

    explicit CloneRepositoryPanel(QWidget *parent = 0);
    ~CloneRepositoryPanel();

    QString cloneRepository() const;
    QString cloneRepositoryName() const;
    RepositoryType cloneRepositoryType() const;
    QString checkoutBranch() const;
    bool isValid() const;

Q_SIGNALS:
    void cloneRepositoryNameEdited(const QString&);
    void checkoutRepositoryEdited(const QString&);
    void checkoutBranchEdited(const QString&);

public slots:
    void update();
    void triggerRepositoryNameChange(const QString &name);

protected:
    void changeEvent(QEvent *e);

private slots:
    void slotCloneRepositoryToggled(bool toggled);
    void slotCheckoutRepositoryToggled(bool toggled);
    void slotCloneRepositoryNameEdited(const QString &name);
    void slotCheckoutRepositoryChanged(const QString &path);
    void slotBranchEdited(const QString &branch);

private:
    CloneRepositoryPanelPrivate *d;
};

} // namespace Internal
} // namespace Fossil

#endif // CLONEREPOSITORYPANEL_H
