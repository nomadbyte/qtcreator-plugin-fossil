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

#ifndef FOSSILCONSTANTS_H
#define FOSSILCONSTANTS_H

#include <QtCore/QtGlobal>

namespace Fossil {
namespace Constants {

const char * const VCS_ID_FOSSIL = "I.Fossil";

const char * const FOSSIL = "fossil";
#if defined(Q_OS_WIN32) || defined(Q_OS_CYGWIN)
const char * const FOSSILREPO = "_FOSSIL_";
#else
const char * const FOSSILREPO = ".fslckout";
#endif
const char * const FOSSILDEFAULT = "fossil";

const char * const FOSSIL_FILE_SUFFIX = ".fossil";
const char * const FOSSIL_FILE_FILTER = "Fossil Repositories (*.fossil *.fsl);;All Files (*)";

//changeset identifiers
const char * const CHANGESET_ID = "([0-9a-f]{5,40})"; // match and capture
const char * const CHANGESET_ID_EXACT = "[0-9a-f]{5,40}"; // match

//diff chunk identifiers
const char * const DIFFFILE_ID_EXACT = "[+]{3} (.*)\\s*";  // match and capture

//BaseEditorParameters
const char * const COMMANDLOG_ID = "Fossil Command Log Editor";
const char * const COMMANDLOG_DISPLAY_NAME = QT_TRANSLATE_NOOP("VCS", "Fossil Command Log Editor");
const char * const COMMANDLOG = "Fossil Command Log Editor";
const char * const COMMANDAPP = "application/vnd.nokia.text.scs_fossil_commandlog";
const char * const COMMANDEXT = "vcsFossilCommand";

const char * const FILELOG_ID = "Fossil File Log Editor";
const char * const FILELOG_DISPLAY_NAME = QT_TRANSLATE_NOOP("VCS", "Fossil File Log Editor");
const char * const FILELOG = "Fossil File Log Editor";
const char * const LOGAPP = "application/vnd.nokia.text.scs_fossil_log";
const char * const LOGEXT = "vcsFossilLog";

const char * const ANNOTATELOG_ID = "Fossil Annotation Editor";
const char * const ANNOTATELOG_DISPLAY_NAME = QT_TRANSLATE_NOOP("VCS", "Fossil Annotation Editor");
const char * const ANNOTATELOG = "Fossil Annotation Editor";
const char * const ANNOTATEAPP = "application/vnd.nokia.text.scs_fossil_annotatelog";
const char * const ANNOTATEEXT = "vcsFossilAnnotate";

const char * const DIFFLOG_ID = "Fossil Diff Editor";
const char * const DIFFLOG_DISPLAY_NAME = QT_TRANSLATE_NOOP("VCS", "Fossil Diff Editor");
const char * const DIFFLOG = "Fossil Diff Editor";
const char * const DIFFAPP = "text/x-patch";
const char * const DIFFEXT = "diff";

//SubmitEditorParameters
const char * const COMMIT_ID = "Fossil Commit Log Editor";
const char * const COMMIT_DISPLAY_NAME = QT_TRANSLATE_NOOP("VCS", "Fossil Commit Log Editor");
const char * const COMMITMIMETYPE = "application/vnd.nokia.text.scs_fossil_commitlog";

//menu items
//File menu actions
const char * const ADD = "Fossil.AddSingleFile";
const char * const DELETE = "Fossil.DeleteSingleFile";
const char * const ANNOTATE = "Fossil.Annotate";
const char * const DIFF = "Fossil.DiffSingleFile";
const char * const LOG = "Fossil.LogSingleFile";
const char * const REVERT = "Fossil.RevertSingleFile";
const char * const STATUS = "Fossil.Status";

//directory menu Actions
const char * const DIFFMULTI = "Fossil.Action.DiffMulti";
const char * const REVERTMULTI = "Fossil.Action.RevertAll";
const char * const STATUSMULTI = "Fossil.Action.StatusMulti";
const char * const LOGMULTI = "Fossil.Action.LogMulti";

//repository menu actions
const char * const PULL = "Fossil.Action.Pull";
const char * const PUSH = "Fossil.Action.Push";
const char * const UPDATE = "Fossil.Action.Update";
const char * const COMMIT = "Fossil.Action.Commit";
const char * const CONFIGURE_REPOSITORY = "Fossil.Action.Settings";
const char * const CREATE_REPOSITORY = "Fossil.Action.CreateRepository";

//submit editor actions
const char * const DIFFEDITOR = "Fossil.Action.Editor.Diff";

} // namespace Constants
} // namespace Fossil

#endif // FOSSILCONSTANTS_H
