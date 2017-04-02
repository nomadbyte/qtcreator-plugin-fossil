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

#ifndef CLONEOPTIONSPANEL_H
#define CLONEOPTIONSPANEL_H

#include <QtGui/QWidget>

namespace Fossil {
namespace Internal {

class CloneOptionsPanelPrivate;

class CloneOptionsPanel : public QWidget
{
    Q_OBJECT

public:
    CloneOptionsPanel(QWidget *parent = 0);
    ~CloneOptionsPanel();

    bool isOptionsEnabled() const;
    bool isIncludePrivateOptionEnabled() const;
    bool isAutosyncOffOptionEnabled() const;
    QString sslIdentityFile() const;
    QString adminUser() const;

    void setFields(const QString &sslIdentityFile, const QString &adminUser);
    bool isValid() const;

private:
    CloneOptionsPanelPrivate *d;
};

} // namespace Internal
} // namespace Fossil

#endif // CLONEOPTIONSPANEL_H
