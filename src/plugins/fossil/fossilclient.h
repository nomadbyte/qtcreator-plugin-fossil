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

#ifndef FOSSILCLIENT_H
#define FOSSILCLIENT_H

#include "fossilsettings.h"
#include "branchinfo.h"
#include "revisioninfo.h"
#include <vcsbase/vcsbaseclient.h>

#include <QList>


namespace Fossil {
namespace Internal {

class FossilSettings;

class FossilClient : public VcsBase::VcsBaseClient
{
    Q_OBJECT

public:
    enum SupportedFeature {
        AnnotateBlameFeature = 0x2,
        TimelineWidthFeature = 0x4,
        AllSupportedFeatures =  // | all defined features
            AnnotateBlameFeature
            | TimelineWidthFeature
    };
    Q_DECLARE_FLAGS(SupportedFeatures, SupportedFeature)

    static unsigned makeVersionNumber(int major, int minor, int patch);
    static QString makeVersionString(unsigned version);
    static QString buildPath(const QString &path, const QString &baseName, const QString &suffix);

    FossilClient(FossilSettings *settings);

    FossilSettings *settings() const;

    unsigned int synchronousBinaryVersion() const;
    BranchInfo synchronousBranchQuery(const QString &workingDirectory, QList<BranchInfo> *allBranches = 0);
    RevisionInfo synchronousRevisionQuery(const QString &workingDirectory, const QString &id = QString());
    QStringList synchronousTagQuery(const QString &workingDirectory, const QString &id = QString());
    RepositorySettings synchronousSettingsQuery(const QString &workingDirectory);
    bool synchronousSetSetting(const QString &workingDirectory, const QString &property,
                               const QString &value = QString(), bool isGlobal = false);
    bool synchronousConfigureRepository(const QString &workingDirectory, const RepositorySettings &newSettings,
                                        const RepositorySettings &currentSettings = RepositorySettings());
    QString synchronousUserDefaultQuery(const QString &workingDirectory);
    bool synchronousSetUserDefault(const QString &workingDirectory, const QString &userName);
    QString synchronousGetRepositoryURL(const QString &workingDirectory);
    bool synchronousCreateRepository(const QString &workingDirectory,
                                     const QStringList &extraOptions = QStringList());
    bool synchronousMove(const QString &workingDir,
                         const QString &from, const QString &to,
                         const QStringList &extraOptions = QStringList());
    bool synchronousPull(const QString &workingDir,
                         const QString &srcLocation,
                         const QStringList &extraOptions = QStringList());
    bool synchronousPush(const QString &workingDir,
                         const QString &dstLocation,
                         const QStringList &extraOptions = QStringList());
    void commit(const QString &repositoryRoot, const QStringList &files,
                const QString &commitMessageFile, const QStringList &extraOptions = QStringList());
    void annotate(const QString &workingDir, const QString &file,
                  const QString revision = QString(), int lineNumber = -1,
                  const QStringList &extraOptions = QStringList());
    void view(const QString &source, const QString &id,
              const QStringList &extraOptions = QStringList());
    void logRepository(const QString &workingDir, const QStringList &extraOptions = QStringList());
    void revertFile(const QString &workingDir, const QString &file,
                    const QString &revision = QString(),
                    const QStringList &extraOptions = QStringList());
    void revertAll(const QString &workingDir, const QString &revision = QString(),
                   const QStringList &extraOptions = QStringList());
    QString findTopLevelForFile(const QFileInfo &file) const;
    bool managesFile(const QString &workingDirectory, const QString &fileName) const;
    unsigned int binaryVersion() const;
    QString binaryVersionString() const;
    SupportedFeatures supportedFeatures() const;

protected:
    QString vcsCommandString(VcsCommand cmd) const;
    Core::Id vcsEditorKind(VcsCommand cmd) const;
    QStringList revisionSpec(const QString &revision) const;
    VcsBase::VcsBaseEditorParameterWidget *createDiffEditor(const QString &workingDir,
                                                            const QStringList &files,
                                                            const QStringList &extraOptions);
    VcsBase::VcsBaseEditorParameterWidget *createAnnotateEditor(const QString &workingDir,
                                                                const QString &file,
                                                                const QString &revision,
                                                                int lineNumber,
                                                                const QStringList &extraOptions);
    VcsBase::VcsBaseEditorParameterWidget *createLogEditor(const QString &workingDir,
                                                           const QStringList &files,
                                                           const QStringList &extraOptions);
    VcsBase::VcsBaseEditorParameterWidget *createLogRepositoryEditor(const QString &workingDir,
                                                                     const QStringList &files,
                                                                     const QStringList &extraOptions);
    StatusItem parseStatusLine(const QString &line) const;

private:
    friend class CloneWizard;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(FossilClient::SupportedFeatures)

} // namespace Internal
} // namespace Fossil

#endif // FOSSILCLIENT_H