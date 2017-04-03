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

#ifndef FOSSILCONSTANTS_H
#define FOSSILCONSTANTS_H

#include <QtGlobal>

namespace Fossil {
namespace Constants {

const char VCS_ID_FOSSIL[] = "I.Fossil";

const char FOSSIL[] = "fossil";
#if defined(Q_OS_WIN) || defined(Q_OS_CYGWIN)
const char FOSSILREPO[] = "_FOSSIL_";
#else
const char FOSSILREPO[] = ".fslckout";
#endif
const char FOSSILDEFAULT[] = "fossil";
const char FOSSIL_CONTEXT[] = "Fossil Context";

const char FOSSIL_FILE_SUFFIX[] = ".fossil";
const char FOSSIL_FILE_FILTER[] = "Fossil Repositories (*.fossil *.fsl);;All Files (*)";

//changeset identifiers
const char CHANGESET_ID[] = "([0-9a-f]{5,40})"; // match and capture
const char CHANGESET_ID_EXACT[] = "[0-9a-f]{5,40}"; // match

//diff chunk identifiers
const char DIFFFILE_ID_EXACT[] = "[+]{3} (.*)\\s*";  // match and capture

//BaseEditorParameters
const char FILELOG_ID[] = "Fossil File Log Editor";
const char FILELOG_DISPLAY_NAME[] = QT_TRANSLATE_NOOP("VCS", "Fossil File Log Editor");
const char LOGAPP[] = "text/vnd.qtcreator.fossil.log";

const char ANNOTATELOG_ID[] = "Fossil Annotation Editor";
const char ANNOTATELOG_DISPLAY_NAME[] = QT_TRANSLATE_NOOP("VCS", "Fossil Annotation Editor");
const char ANNOTATEAPP[] = "text/vnd.qtcreator.fossil.annotation";

const char DIFFLOG_ID[] = "Fossil Diff Editor";
const char DIFFLOG_DISPLAY_NAME[] = QT_TRANSLATE_NOOP("VCS", "Fossil Diff Editor");
const char DIFFAPP[] = "text/x-patch";

//SubmitEditorParameters
const char COMMIT_ID[] = "Fossil Commit Log Editor";
const char COMMIT_DISPLAY_NAME[] = QT_TRANSLATE_NOOP("VCS", "Fossil Commit Log Editor");
const char COMMITMIMETYPE[] = "text/vnd.qtcreator.fossil.commit";

//menu items
//File menu actions
const char ADD[] = "Fossil.AddSingleFile";
const char DELETE[] = "Fossil.DeleteSingleFile";
const char ANNOTATE[] = "Fossil.Annotate";
const char DIFF[] = "Fossil.DiffSingleFile";
const char LOG[] = "Fossil.LogSingleFile";
const char REVERT[] = "Fossil.RevertSingleFile";
const char STATUS[] = "Fossil.Status";

//directory menu Actions
const char DIFFMULTI[] = "Fossil.Action.DiffMulti";
const char REVERTMULTI[] = "Fossil.Action.RevertAll";
const char STATUSMULTI[] = "Fossil.Action.StatusMulti";
const char LOGMULTI[] = "Fossil.Action.LogMulti";

//repository menu actions
const char PULL[] = "Fossil.Action.Pull";
const char PUSH[] = "Fossil.Action.Push";
const char UPDATE[] = "Fossil.Action.Update";
const char COMMIT[] = "Fossil.Action.Commit";
const char CONFIGURE_REPOSITORY[] = "Fossil.Action.Settings";
const char CREATE_REPOSITORY[] = "Fossil.Action.CreateRepository";

//submit editor actions
const char DIFFEDITOR[] = "Fossil.Action.Editor.Diff";

// File status hint
const char FSTATUS_ADDED[] = "Added";
const char FSTATUS_ADDED_BY_MERGE[] = "Added by Merge";
const char FSTATUS_ADDED_BY_INTEGRATE[] = "Added by Integrate";
const char FSTATUS_DELETED[] = "Deleted";
const char FSTATUS_EDITED[] = "Edited";
const char FSTATUS_UPDATED_BY_MERGE[] = "Updated by Merge";
const char FSTATUS_UPDATED_BY_INTEGRATE[] = "Updated by Integrate";
const char FSTATUS_RENAMED[] = "Renamed";
const char FSTATUS_UNKNOWN[] = "Unknown";

} // namespace Constants
} // namespace Fossil

#endif // FOSSILCONSTANTS_H
