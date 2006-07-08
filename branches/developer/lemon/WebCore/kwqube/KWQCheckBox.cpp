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
#include "KWQCheckBox.h"

// We empirically determined that check boxes have these dimensions.
// It would be better to get this info from AppKit somehow.

#define TOP_MARGIN 4
#define BOTTOM_MARGIN 3
#define LEFT_MARGIN 3
#define RIGHT_MARGIN 3

#define WIDTH 12
#define HEIGHT 12

#define BASELINE_MARGIN 4
enum {
    topMargin,
    bottomMargin,
    leftMargin,
    rightMargin,
    baselineFudgeFactor,
    dimWidth,
    dimHeight
};

QCheckBox::QCheckBox(QWidget * parent, const char * name, int f)
    : QButton(parent, name,f)
      ,m_stateChanged(this, SIGNAL(stateChanged(int)))

{
    GtkWidget *button = gtk_check_button_new();
    setGtkWidget(button);
}

QSize QCheckBox::sizeHint() const 
{
    return QButton::sizeHint();
}

QRect QCheckBox::frameGeometry() const
{
    return QButton::frameGeometry();
}

void QCheckBox::setFrameGeometry(const QRect &r)
{
    QButton::setFrameGeometry(r);
}

void QCheckBox::setChecked(bool isChecked)
{    
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(getGtkWidget()), isChecked );
}

bool QCheckBox::isChecked()
{
    return gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(getGtkWidget()) );
}

void QCheckBox::clicked()
{
    // Note that it's important to give the stateChanged signal before
    // the clicked signal so that the corresponding JavaScript messages
    // go in the right order. A test for this at the time of this writing
    // was the languages radio buttons and check boxes at google.com prefs.
    m_stateChanged.call(isChecked() ? 2 : 0);
    QButton::clicked();
}

int QCheckBox::baselinePosition(int aheight) const
{
    return (int) ((15.0f/20.0f)*(float) aheight);
}

#if 0
const int *QCheckBox::dimensions() const
{
    // We empirically determined these dimensions.
    // It would be better to get this info from AppKit somehow.
    static const int w[3][7] = {
        { 3, 4, 2, 4, 2, 14, 14 },
        { 4, 3, 3, 3, 2, 12, 12 },
        { 4, 3, 3, 3, 2, 10, 10 },
    };

    return w[0];
}
#endif
