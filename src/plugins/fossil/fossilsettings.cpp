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

#include "fossilsettings.h"
#include "constants.h"

#include <QSettings>

namespace Fossil {
namespace Internal {

const QString FossilSettings::defaultRepoPathKey("defaultRepoPath");
const QString FossilSettings::sslIdentityFileKey("sslIdentityFile");
const QString FossilSettings::diffIgnoreAllWhiteSpaceKey("diffIgnoreAllWhiteSpace");
const QString FossilSettings::diffStripTrailingCRKey("diffStripTrailingCR");
const QString FossilSettings::annotateShowCommittersKey("annotateShowCommitters");
const QString FossilSettings::annotateListVersionsKey("annotateListVersions");
const QString FossilSettings::timelineWidthKey("timelineWidth");
const QString FossilSettings::timelineLineageFilterKey("timelineLineageFilter");
const QString FossilSettings::timelineVerboseKey("timelineVerbose");
const QString FossilSettings::timelineItemTypeKey("timelineItemType");
const QString FossilSettings::disableAutosyncKey("disableAutosync");

FossilSettings::FossilSettings()
{
    setSettingsGroup(Constants::FOSSIL);
    // Override default binary path
    declareKey(binaryPathKey, Constants::FOSSILDEFAULT);
    declareKey(defaultRepoPathKey, "");
    declareKey(sslIdentityFileKey, "");
    declareKey(diffIgnoreAllWhiteSpaceKey, false);
    declareKey(diffStripTrailingCRKey, false);
    declareKey(annotateShowCommittersKey, false);
    declareKey(annotateListVersionsKey, false);
    declareKey(timelineWidthKey, 0);
    declareKey(timelineLineageFilterKey, "");
    declareKey(timelineVerboseKey, false);
    declareKey(timelineItemTypeKey, "all");
    declareKey(disableAutosyncKey, true);
}

RepositorySettings::RepositorySettings()
    : autosync(AutosyncOn)
{
}

} // namespace Internal
} // namespace Fossil
