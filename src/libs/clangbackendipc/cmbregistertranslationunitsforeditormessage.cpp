/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://www.qt.io/licensing.  For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file.  Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
****************************************************************************/

#include "cmbregistertranslationunitsforeditormessage.h"

#include "container_common.h"

#include <QDataStream>
#include <QDebug>

#include <ostream>

namespace ClangBackEnd {

RegisterTranslationUnitForEditorMessage::RegisterTranslationUnitForEditorMessage(const QVector<FileContainer> &fileContainers,
                                                                                 const Utf8String &currentEditorFilePath,
                                                                                 const Utf8StringVector &visibleEditorFilePaths)
    : fileContainers_(fileContainers),
      currentEditorFilePath_(currentEditorFilePath),
      visibleEditorFilePaths_(visibleEditorFilePaths)
{
}

const QVector<FileContainer> &RegisterTranslationUnitForEditorMessage::fileContainers() const
{
    return fileContainers_;
}

const Utf8String &RegisterTranslationUnitForEditorMessage::currentEditorFilePath() const
{
    return currentEditorFilePath_;
}

const Utf8StringVector &RegisterTranslationUnitForEditorMessage::visibleEditorFilePaths() const
{
    return visibleEditorFilePaths_;
}

QDataStream &operator<<(QDataStream &out, const RegisterTranslationUnitForEditorMessage &message)
{
    out << message.fileContainers_;
    out << message.currentEditorFilePath_;
    out << message.visibleEditorFilePaths_;
    return out;
}

QDataStream &operator>>(QDataStream &in, RegisterTranslationUnitForEditorMessage &message)
{
    in >> message.fileContainers_;
    in >> message.currentEditorFilePath_;
    in >> message.visibleEditorFilePaths_;

    return in;
}

bool operator==(const RegisterTranslationUnitForEditorMessage &first, const RegisterTranslationUnitForEditorMessage &second)
{
    return first.fileContainers_ == second.fileContainers_
        && first.currentEditorFilePath_ == second.currentEditorFilePath_
        && first.visibleEditorFilePaths_ == second.visibleEditorFilePaths_;
}

bool operator<(const RegisterTranslationUnitForEditorMessage &first, const RegisterTranslationUnitForEditorMessage &second)
{
    return compareContainer(first.fileContainers_, second.fileContainers_)
        && first.currentEditorFilePath_ < second.currentEditorFilePath_
        && compareContainer(first.visibleEditorFilePaths_, second.visibleEditorFilePaths_);
}

QDebug operator<<(QDebug debug, const RegisterTranslationUnitForEditorMessage &message)
{
    debug.nospace() << "RegisterTranslationUnitForEditorMessage(";

    for (const FileContainer &fileContainer : message.fileContainers())
        debug.nospace() << fileContainer<< ", ";

    debug.nospace() << message.currentEditorFilePath()  << ", ";

    for (const Utf8String &visibleEditorFilePath : message.visibleEditorFilePaths())
        debug.nospace() << visibleEditorFilePath << ", ";

    debug.nospace() << ")";

    return debug;
}

void PrintTo(const RegisterTranslationUnitForEditorMessage &message, ::std::ostream* os)
{
    *os << "RegisterTranslationUnitForEditorMessage(";

    for (const FileContainer &fileContainer : message.fileContainers())
        PrintTo(fileContainer, os);

    *os << message.currentEditorFilePath().constData()  << ", ";

    auto visiblePaths = message.visibleEditorFilePaths();

    std::copy(visiblePaths.cbegin(), visiblePaths.cend(), std::ostream_iterator<Utf8String>(*os, ", "));

    *os << ")";
}

} // namespace ClangBackEnd

