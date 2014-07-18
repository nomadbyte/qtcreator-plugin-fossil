/**************************************************************************
**  This file is part of Fossil VCS plugin for Qt Creator
**
**  Copyright (c) 2013 - 2014, Artur Shepilko, <qtc-fossil@nomadbyte.com>.
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

#ifndef FOSSILSETTINGS_H
#define FOSSILSETTINGS_H

#include <vcsbase/vcsbaseclientsettings.h>

#include <QDebug>


namespace Fossil {
namespace Internal {

class FossilSettings : public VcsBase::VcsBaseClientSettings
{
public:
    static const QLatin1String defaultRepoPathKey;
    static const QLatin1String sslIdentityFileKey;
    static const QLatin1String diffIgnoreAllWhiteSpaceKey;
    static const QLatin1String diffStripTrailingCRKey;
    static const QLatin1String annotateShowCommittersKey;
    static const QLatin1String timelineWidthKey;
    static const QLatin1String timelineLineageFilterKey;
    static const QLatin1String timelineVerboseKey;
    static const QLatin1String timelineItemTypeKey;
    static const QLatin1String disableAutosyncKey;

    FossilSettings();
};

struct RepositorySettings
{
    enum AutosyncMode {AutosyncOff=0, AutosyncOn=1, AutosyncPullOnly};

    QString user;
    AutosyncMode autosync;
    QString sslIdentityFile;

    RepositorySettings();
};

inline bool operator== (const RepositorySettings &lh, const RepositorySettings &rh)
{
    return (lh.user == rh.user
            && lh.autosync == rh.autosync
            && lh.sslIdentityFile == rh.sslIdentityFile);
}

inline QDebug operator<< (QDebug dbg, const RepositorySettings& rh) {
    dbg.nospace() << "RepositorySettings("
                  << rh.user << ", "
                  << rh.autosync << ", "
                  << rh.sslIdentityFile
                  << ")";
    return dbg;
}

} // namespace Internal
} // namespace Fossil

#endif // FOSSILSETTINGS_H
