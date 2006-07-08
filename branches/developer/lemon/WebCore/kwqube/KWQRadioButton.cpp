/*
 * Copyright (C) 2003 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */


#include <gtk/gtk.h>

#include "KWQRadioButton.h"

enum {
    topMargin,
    bottomMargin,
    leftMargin,
    rightMargin,
    baselineFudgeFactor,
    dimWidth,
    dimHeight
};

QRadioButton::QRadioButton(QWidget * parent, const char * name, int f)
    : QButton(parent,name,f)
{
    GtkWidget *btn =  gtk_radio_button_new(NULL);
    setGtkWidget(btn);

    setChecked(false);
}

QRadioButton::~QRadioButton()
{
}

QSize QRadioButton::sizeHint() const 
{
    return  QButton::sizeHint();
}

QRect QRadioButton::frameGeometry() const
{
    return QButton::frameGeometry();
}

void QRadioButton::setFrameGeometry(const QRect &r)
{
    QButton::setFrameGeometry(r);
}

void QRadioButton::setChecked(bool isChecked)
{
    GtkWidget *btn = getGtkWidget();
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(btn), isChecked ? TRUE:FALSE);
}

bool QRadioButton::isChecked() const
{
    GtkWidget *btn = getGtkWidget();
    return gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(btn));
}

// adhoc baseline: size==20 => baseline=15
int QRadioButton::baselinePosition(int height) const
{    
    return (int) ((15.0f/20.0f)*(float)height);
}

#if 0
const int *QRadioButton::dimensions() const
{
    // We empirically determined these dimensions.
    // It would be better to get this info from AppKit somehow.
    static const int w[3][7] = {
        { 2, 4, 2, 2, 2, 14, 15 },
        { 3, 3, 2, 2, 2, 12, 13 },
        { 1, 2, 0, 0, 2, 10, 10 },
    };

    return w[0];
}
#endif
