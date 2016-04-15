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
** a written agreement between you and The Qt Company.  For licensing terms and
** conditions see http://www.qt.io/terms-conditions.  For further information
** use the contact form at http://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
****************************************************************************/

#ifndef QMLDESIGNER_QT5INFORMATIONNODEINSTANCESERVER_H
#define QMLDESIGNER_QT5INFORMATIONNODEINSTANCESERVER_H

#include "qt5nodeinstanceserver.h"
#include "tokencommand.h"

namespace QmlDesigner {

class Qt5InformationNodeInstanceServer : public Qt5NodeInstanceServer
{
    Q_OBJECT
public:
    explicit Qt5InformationNodeInstanceServer(NodeInstanceClientInterface *nodeInstanceClient);

    void reparentInstances(const ReparentInstancesCommand &command) override;
    void clearScene(const ClearSceneCommand &command) override;
    void createScene(const CreateSceneCommand &command) override;
    void completeComponent(const CompleteComponentCommand &command) override;
    void token(const TokenCommand &command) override;
    void removeSharedMemory(const RemoveSharedMemoryCommand &command) override;

protected:
    void collectItemChangesAndSendChangeCommands() override;
    void sendChildrenChangedCommand(const QList<ServerNodeInstance> &childList);
    void sendTokenBack();
    bool isDirtyRecursiveForNonInstanceItems(QQuickItem *item) const;
    bool isDirtyRecursiveForParentInstances(QQuickItem *item) const;

private:
    QSet<ServerNodeInstance> m_parentChangedSet;
    QList<ServerNodeInstance> m_completedComponentList;
    QList<TokenCommand> m_tokenList;
};

} // namespace QmlDesigner

#endif // QMLDESIGNER_QT5INFORMATIONNODEINSTANCESERVER_H
