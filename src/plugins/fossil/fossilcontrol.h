/**************************************************************************
**  This file is part of Fossil VCS plugin for Qt Creator
**
**  Copyright (c) 2013 - 2017, Artur Shepilko, <qtc-fossil@nomadbyte.com>.
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

    QString displayName() const override;
    Core::Id id() const override;

    bool managesDirectory(const QString &filename, QString *topLevel = 0) const override;
    bool managesFile(const QString &workingDirectory, const QString &fileName) const override;
    bool isConfigured() const override;
    bool supportsOperation(Operation operation) const override;
    bool vcsOpen(const QString &fileName) override;
    bool vcsAdd(const QString &filename) override;
    bool vcsDelete(const QString &filename) override;
    bool vcsMove(const QString &from, const QString &to) override;
    bool vcsCreateRepository(const QString &directory) override;
    bool vcsAnnotate(const QString &file, int line) override;
    Core::ShellCommand *createInitialCheckoutCommand(const QString &sourceUrl,
                                                     const Utils::FileName &baseDirectory,
                                                     const QString &localName,
                                                     const QStringList &extraArgs) override;

    // To be connected to the VcsTask's success signal to emit the repository/
    // files changed signals according to the variant's type:
    // String -> repository, StringList -> files
    void changed(const QVariant &);

private:
    FossilClient *const m_client;
};

} // namespace Internal
} // namespace Fossil
