/*
    Copyright (c) 2006 Maxim Sokhatsky (maxim@sokhatsky.com)
*/

#ifndef QCHECKBOX_H
#define QCHECKBOX_H

#include <AppKit.h>
#include "KWQButton.h"

class QCheckBox : public QButton {
public:
    QCheckBox(QWidget * parent = 0, const char * name = 0, int f = 0);
    void setChecked(bool);
    bool isChecked();
    QSize sizeHint();
    int baselinePosition(int height);
    QRect frameGeometry();
    void setFrameGeometry(const QRect &);
    virtual void clicked();
    virtual void MessageReceived(BMessage *message);

private:
    const int *dimensions();
    KWQSignal m_stateChanged;
    BCheckBox* check_box;
};

#endif
