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

#pragma once

#include <vcsbase/wizard/vcsjsextension.h>

#include <QStringList>
#include <QMap>
#include <QObject>

namespace Fossil {
namespace Internal {

class FossilJsExtensionPrivate;
class FossilSettings;

class FossilJsExtension : public QObject
{
    Q_OBJECT

public:
    static void parseArgOptions(const QStringList &args, QMap<QString, QString> &options);

    FossilJsExtension(FossilSettings *settings);
    ~FossilJsExtension();

    Q_INVOKABLE bool isConfigured() const;
    Q_INVOKABLE QString displayName() const;
    Q_INVOKABLE QString defaultAdminUser() const;
    Q_INVOKABLE QString defaultSslIdentityFile() const;
    Q_INVOKABLE QString defaultLocalRepoPath() const;
    Q_INVOKABLE bool defaultDisableAutosync() const;

private:
    FossilJsExtensionPrivate *d = nullptr;
};

} // namespace Internal
} // namespace Fossil
