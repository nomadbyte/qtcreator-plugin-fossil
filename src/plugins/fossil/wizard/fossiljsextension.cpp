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

#include "fossiljsextension.h"
#include "../constants.h"
#include "../fossilclient.h"
#include "../fossilplugin.h"

#include <coreplugin/iversioncontrol.h>
#include <coreplugin/vcsmanager.h>

#include <vcsbase/vcsbaseclientsettings.h>
#include <vcsbase/vcsbaseconstants.h>

using namespace Core;

namespace Fossil {
namespace Internal {


class FossilJsExtensionPrivate {

public:
    FossilJsExtensionPrivate() :
        m_vscId(Constants::VCS_ID_FOSSIL) { }

    FossilClient *client() const {
        return FossilPlugin::instance()->client();
    }

    Core::Id m_vscId;
};


void FossilJsExtension::parseArgOptions(const QStringList &args, QMap<QString, QString> &options)
{
    options.clear();

    foreach (const QString &arg, args) {
        if (arg.isEmpty()) continue;

        QStringList opt = arg.split('|', QString::KeepEmptyParts);
        options.insert(opt[0], opt.size() > 1 ? opt[1] : QString());
    }
}

FossilJsExtension::FossilJsExtension() :
    d(new FossilJsExtensionPrivate)
{ }

FossilJsExtension::~FossilJsExtension()
{
    delete d;
}

bool FossilJsExtension::isConfigured() const
{
    IVersionControl *vc = VcsManager::versionControl(d->m_vscId);
    return vc && vc->isConfigured();
}

QString FossilJsExtension::displayName() const
{
    IVersionControl *vc = VcsManager::versionControl(d->m_vscId);
    return vc ? vc->displayName() : QString();
}

QString FossilJsExtension::defaultAdminUser() const
{
    if (!isConfigured())
        return QString();

    VcsBase::VcsBaseClientSettings &settings = d->client()->settings();
    return settings.stringValue(FossilSettings::userNameKey);
}

QString FossilJsExtension::defaultSslIdentityFile() const
{
    if (!isConfigured())
        return QString();

    VcsBase::VcsBaseClientSettings &settings = d->client()->settings();
    return settings.stringValue(FossilSettings::sslIdentityFileKey);
}

QString FossilJsExtension::defaultLocalRepoPath() const
{
    if (!isConfigured())
        return QString();

    VcsBase::VcsBaseClientSettings &settings = d->client()->settings();
    return settings.stringValue(FossilSettings::defaultRepoPathKey);
}

bool FossilJsExtension::defaultDisableAutosync() const
{
    if (!isConfigured())
        return false;

    VcsBase::VcsBaseClientSettings &settings = d->client()->settings();
    return settings.boolValue(FossilSettings::disableAutosyncKey);
}

} // namespace Internal
} // namespace Fossil

