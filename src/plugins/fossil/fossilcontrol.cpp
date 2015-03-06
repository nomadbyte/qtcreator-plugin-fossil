/**************************************************************************
**  This file is part of Fossil VCS plugin for Qt Creator
**
**  Copyright (c) 2013 - 2015, Artur Shepilko, <qtc-fossil@nomadbyte.com>.
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

#include "constants.h"
#include "fossilcontrol.h"
#include "fossilclient.h"

#include <vcsbase/vcsbaseclientsettings.h>
#include <vcsbase/vcsbaseconstants.h>

#include <QFileInfo>
#include <QVariant>
#include <QStringList>
#include <QDir>

using namespace Fossil::Internal;

FossilControl::FossilControl(FossilClient *client)
    : m_client(client)
{
}

QString FossilControl::displayName() const
{
    return tr("Fossil");
}

Core::Id FossilControl::id() const
{
    return Core::Id(Constants::VCS_ID_FOSSIL);
}

bool FossilControl::managesDirectory(const QString &directory, QString *topLevel) const
{
    QFileInfo dir(directory);
    const QString topLevelFound = m_client->findTopLevelForFile(dir);
    if (topLevel)
        *topLevel = topLevelFound;
    return !topLevelFound.isEmpty();
}

bool FossilControl::managesFile(const QString &workingDirectory, const QString &fileName) const
{
    return m_client->managesFile(workingDirectory, fileName);
}

bool FossilControl::isConfigured() const
{
    const QString binary = m_client->settings()->binaryPath();
    if (binary.isEmpty())
        return false;

    QFileInfo fi(binary);
    if ( !(fi.exists() && fi.isFile() && fi.isExecutable()) )
        return false;

    // Local repositories default path must be set and exist
    const QString repoPath = m_client->settings()->stringValue(FossilSettings::defaultRepoPathKey);
    if (repoPath.isEmpty())
        return false;

    QDir dir(repoPath);
    if (!dir.exists())
        return false;

    return true;
}

bool FossilControl::supportsOperation(Operation operation) const
{
    bool supported = isConfigured();

    switch (operation) {
    case Core::IVersionControl::AddOperation:
    case Core::IVersionControl::DeleteOperation:
    case Core::IVersionControl::MoveOperation:
    case Core::IVersionControl::CreateRepositoryOperation:
    case Core::IVersionControl::AnnotateOperation:
    case Core::IVersionControl::GetRepositoryRootOperation:
        break;
    case Core::IVersionControl::CheckoutOperation:
    case Core::IVersionControl::SnapshotOperations:
        supported = false;
        break;
    }
    return supported;
}

bool FossilControl::vcsOpen(const QString &filename)
{
    Q_UNUSED(filename)
    return true;
}

bool FossilControl::vcsAdd(const QString &filename)
{
    const QFileInfo fi(filename);
    return m_client->synchronousAdd(fi.absolutePath(), fi.fileName());
}

bool FossilControl::vcsDelete(const QString &filename)
{
    const QFileInfo fi(filename);
    return m_client->synchronousRemove(fi.absolutePath(), fi.fileName());
}

bool FossilControl::vcsMove(const QString &from, const QString &to)
{
    const QFileInfo fromInfo(from);
    const QFileInfo toInfo(to);
    return m_client->synchronousMove(fromInfo.absolutePath(),
                                     fromInfo.absoluteFilePath(),
                                     toInfo.absoluteFilePath());
}

bool FossilControl::vcsCreateRepository(const QString &directory)
{
    return m_client->synchronousCreateRepository(directory);
}

bool FossilControl::vcsAnnotate(const QString &file, int line)
{
    const QFileInfo fi(file);
    m_client->annotate(fi.absolutePath(), fi.fileName(), QString(), line);
    return true;
}

QString FossilControl::vcsTopic(const QString &directory)
{
    return m_client->synchronousTopic(directory);
}

bool FossilControl::vcsCheckout(const QString &directory, const QByteArray &url)
{
    Q_UNUSED(directory);
    Q_UNUSED(url);
    return false;
}

QString FossilControl::vcsGetRepositoryURL(const QString &directory)
{
    return m_client->synchronousGetRepositoryURL(directory);
}

void FossilControl::changed(const QVariant &v)
{
    switch (v.type()) {
    case QVariant::String:
        emit repositoryChanged(v.toString());
        break;
    case QVariant::StringList:
        emit filesChanged(v.toStringList());
        break;
    default:
        break;
    }
}

void FossilControl::emitConfigurationChanged()
{
    emit configurationChanged();
}
