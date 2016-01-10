/**************************************************************************
**  This file is part of Fossil VCS plugin for Qt Creator
**
**  Copyright (c) 2013 - 2016, Artur Shepilko, <qtc-fossil@nomadbyte.com>.
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

#ifndef PULLORPUSHDIALOG_H
#define PULLORPUSHDIALOG_H

#include <QtGui/QDialog>

namespace Fossil {
namespace Internal {

namespace Ui {
class PullOrPushDialog;
}

class PullOrPushDialog : public QDialog
{
    Q_OBJECT

public:
    enum Mode {
        PullMode,
        PushMode
    };

    explicit PullOrPushDialog(Mode mode, QWidget *parent = 0);
    ~PullOrPushDialog();

    // Common parameters and options
    QString remoteLocation() const;
    bool isRememberOptionEnabled() const;
    bool isPrivateOptionEnabled() const;
    void setDefaultRemoteLocation(const QString &url);
    void setLocalBaseDirectory(const QString &dir);
    // Pull-specific options
    // Push-specific options

protected:
    void changeEvent(QEvent *e);

private:
    Mode m_mode;
    Ui::PullOrPushDialog *m_ui;
};

} // namespace Internal
} // namespace Fossil

#endif // PULLORPUSHDIALOG_H
