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

#include <vcsbase/vcsbaseclientsettings.h>

namespace Fossil {
namespace Internal {

class FossilSettings : public VcsBase::VcsBaseClientSettings
{
public:
    static const QString defaultRepoPathKey;
    static const QString sslIdentityFileKey;
    static const QString diffIgnoreAllWhiteSpaceKey;
    static const QString diffStripTrailingCRKey;
    static const QString annotateShowCommittersKey;
    static const QString annotateListVersionsKey;
    static const QString timelineWidthKey;
    static const QString timelineLineageFilterKey;
    static const QString timelineVerboseKey;
    static const QString timelineItemTypeKey;
    static const QString disableAutosyncKey;

    FossilSettings();
};

struct RepositorySettings
{
    enum AutosyncMode {AutosyncOff = 0, AutosyncOn = 1, AutosyncPullOnly};

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

} // namespace Internal
} // namespace Fossil
