/**************************************************************************
**  This file is part of Fossil VCS plugin for Qt Creator
**
**  Copyright (c) 2013, Artur Shepilko.
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

#ifndef FOSSILCONTROL_H
#define FOSSILCONTROL_H

#include <coreplugin/iversioncontrol.h>

QT_BEGIN_NAMESPACE
class QVariant;
QT_END_NAMESPACE

namespace Fossil {
namespace Internal {

class FossilClient;

//Implements just the basics of the Version Control Interface
//FossilClient handles all the work
class FossilControl: public Core::IVersionControl
{
    Q_OBJECT

public:
    explicit FossilControl(FossilClient *fossilClient);

    QString displayName() const;
    QString id() const;

    bool managesDirectory(const QString &filename, QString *topLevel = 0) const;
    bool isConfigured() const;
    bool supportsOperation(Operation operation) const;
    bool vcsOpen(const QString &fileName);
    bool vcsAdd(const QString &filename);
    bool vcsDelete(const QString &filename);
    bool vcsMove(const QString &from, const QString &to);
    bool vcsCreateRepository(const QString &directory);
    bool vcsCheckout(const QString &directory, const QByteArray &url);
    QString vcsGetRepositoryURL(const QString &directory);
    QString vcsCreateSnapshot(const QString &topLevel);
    QStringList vcsSnapshots(const QString &topLevel);
    bool vcsRestoreSnapshot(const QString &topLevel, const QString &name);
    bool vcsRemoveSnapshot(const QString &topLevel, const QString &name);
    bool vcsAnnotate(const QString &file, int line);

public slots:
    // To be connected to the VCSTask's success signal to emit the repository/
    // files changed signals according to the variant's type:
    // String -> repository, StringList -> files
    void changed(const QVariant &);
    void emitConfigurationChanged();

private:
    FossilClient *m_client;
};

} // namespace Internal
} // namespace Fossil

#endif // FOSSILCONTROL_H
