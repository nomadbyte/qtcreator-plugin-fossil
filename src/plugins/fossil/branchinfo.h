/**************************************************************************
**  This file is part of Fossil VCS plugin for Qt Creator
**
**  Copyright (c) 2013 - 2015, Artur Shepilko, <qtc-fossil@nomadbyte.com>.
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

#ifndef BRANCHINFO_H
#define BRANCHINFO_H

#include <QString>
#include <QDebug>

namespace Fossil {
namespace Internal {


class BranchInfo
{
public:
    enum BranchFlag {
        Open = 0x01,
        Closed = 0x02,
        Public = 0x04,
        Private = 0x08
    };
    Q_DECLARE_FLAGS(BranchFlags, BranchFlag)

    explicit BranchInfo(const QString &name, bool isCurrent = false, BranchFlags flags = BranchFlags(Open|Public));


public:
    const QString& name() const;
    bool isCurrent() const;
    bool isClosed() const;
    bool isPrivate() const;

    friend QDebug operator<< (QDebug dbg, const BranchInfo& rh);


private:
    QDebug printDebug(QDebug dbg) const;

    QString m_name;
    bool m_isCurrent;
    BranchFlags m_flags;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(BranchInfo::BranchFlags)


inline QDebug operator<< (QDebug dbg, const BranchInfo& rh) { return rh.printDebug(dbg); }

} // namespace Internal
} // namespace Fossil


#endif // BRANCHINFO_H
