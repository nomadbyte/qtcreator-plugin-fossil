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

#include "fossilsettings.h"
#include "branchinfo.h"
#include "revisioninfo.h"

#include <vcsbase/vcsbaseclient.h>

#include <QList>

namespace Fossil {
namespace Internal {

class FossilSettings;
class FossilPluginPrivate;

class FossilClient : public VcsBase::VcsBaseClient
{
    Q_OBJECT
public:
    enum SupportedFeature {
        AnnotateBlameFeature = 0x2,
        TimelineWidthFeature = 0x4,
        DiffIgnoreWhiteSpaceFeature = 0x8,
        TimelinePathFeature = 0x10,
        AnnotateRevisionFeature = 0x20,
        InfoHashFeature = 0x40,
        AllSupportedFeatures =  // | all defined features
            AnnotateBlameFeature
            | TimelineWidthFeature
            | DiffIgnoreWhiteSpaceFeature
            | TimelinePathFeature
            | AnnotateRevisionFeature
            | InfoHashFeature
    };
    Q_DECLARE_FLAGS(SupportedFeatures, SupportedFeature)

    static unsigned makeVersionNumber(int major, int minor, int patch);
    static QString makeVersionString(unsigned version);

    explicit FossilClient(FossilSettings *settings);

    unsigned int synchronousBinaryVersion() const;
    BranchInfo synchronousCurrentBranch(const QString &workingDirectory);
    QList<BranchInfo> synchronousBranchQuery(const QString &workingDirectory);
    RevisionInfo synchronousRevisionQuery(const QString &workingDirectory, const QString &id = QString(),
                                          bool getCommentMsg = false) const;
    QStringList synchronousTagQuery(const QString &workingDirectory, const QString &id = QString());
    RepositorySettings synchronousSettingsQuery(const QString &workingDirectory);
    bool synchronousSetSetting(const QString &workingDirectory, const QString &property,
                               const QString &value = QString(), bool isGlobal = false);
    bool synchronousConfigureRepository(const QString &workingDirectory, const RepositorySettings &newSettings,
                                        const RepositorySettings &currentSettings = RepositorySettings());
    QString synchronousUserDefaultQuery(const QString &workingDirectory);
    bool synchronousSetUserDefault(const QString &workingDirectory, const QString &userName);
    QString synchronousGetRepositoryURL(const QString &workingDirectory);
    QString synchronousTopic(const QString &workingDirectory);
    bool synchronousCreateRepository(const QString &workingDirectory,
                                     const QStringList &extraOptions = QStringList()) final;
    bool synchronousMove(const QString &workingDir,
                         const QString &from, const QString &to,
                         const QStringList &extraOptions = QStringList()) final;
    bool synchronousPull(const QString &workingDir,
                         const QString &srcLocation,
                         const QStringList &extraOptions = QStringList()) final;
    bool synchronousPush(const QString &workingDir,
                         const QString &dstLocation,
                         const QStringList &extraOptions = QStringList()) final;
    void commit(const QString &repositoryRoot, const QStringList &files,
                const QString &commitMessageFile, const QStringList &extraOptions = QStringList()) final;
    VcsBase::VcsBaseEditorWidget *annotate(
            const QString &workingDir, const QString &file, const QString &revision = QString(),
            int lineNumber = -1, const QStringList &extraOptions = QStringList()) final;
    void log(const QString &workingDir, const QStringList &files = QStringList(),
             const QStringList &extraOptions = QStringList(),
             bool enableAnnotationContextMenu = false) final;
    void logCurrentFile(const QString &workingDir, const QStringList &files = QStringList(),
                        const QStringList &extraOptions = QStringList(),
                        bool enableAnnotationContextMenu = false);
    void revertFile(const QString &workingDir, const QString &file,
                    const QString &revision = QString(),
                    const QStringList &extraOptions = QStringList()) final;
    void revertAll(const QString &workingDir, const QString &revision = QString(),
                   const QStringList &extraOptions = QStringList()) final;
    bool isVcsFileOrDirectory(const Utils::FilePath &filePath) const;
    QString findTopLevelForFile(const QFileInfo &file) const final;
    bool managesFile(const QString &workingDirectory, const QString &fileName) const;
    unsigned int binaryVersion() const;
    QString binaryVersionString() const;
    SupportedFeatures supportedFeatures() const;
    void view(const QString &source, const QString &id,
              const QStringList &extraOptions = QStringList()) final;

private:
    static QList<BranchInfo> branchListFromOutput(const QString &output, const BranchInfo::BranchFlags defaultFlags = {});
    static QStringList parseRevisionCommentLine(const QString &commentLine);

    QString sanitizeFossilOutput(const QString &output) const;
    QString vcsCommandString(VcsCommandTag cmd) const final;
    Core::Id vcsEditorKind(VcsCommandTag cmd) const final;
    QStringList revisionSpec(const QString &revision) const final;
    StatusItem parseStatusLine(const QString &line) const final;
    VcsBase::VcsBaseEditorConfig *createAnnotateEditor(VcsBase::VcsBaseEditorWidget *editor);
    VcsBase::VcsBaseEditorConfig *createLogCurrentFileEditor(VcsBase::VcsBaseEditorWidget *editor);
    VcsBase::VcsBaseEditorConfig *createLogEditor(VcsBase::VcsBaseEditorWidget *editor);

    friend class FossilPluginPrivate;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(FossilClient::SupportedFeatures)

} // namespace Internal
} // namespace Fossil
