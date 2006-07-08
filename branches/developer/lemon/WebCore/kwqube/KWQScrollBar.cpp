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
#include "KWQScrollBar.h"

// QScrollBar::setValue() will always cause one excess call to QScrollBar::setValue()
// 
extern "C" {
void value_changed(GtkAdjustment *adjustment, gpointer user_data)
{
    QScrollBar* self = static_cast<QScrollBar*>(user_data);
    self->setValue((int) adjustment->value);
}
}

QScrollBar::QScrollBar(Qt::Orientation orientation, QWidget* parent)
:m_valueChanged(this, SIGNAL(valueChanged(int)))
{
    m_orientation = orientation;
    //m_scroller = [[KWQScrollBar alloc] initWithQScrollBar:this];
    m_visibleSize = 0;
    m_totalSize = 0;
    m_currentPos = 0;
    m_lineStep = 0;
    m_pageStep = 0;
//    setView(m_scroller);
    setFocusPolicy(NoFocus);
    
    GtkWidget* sb;
    m_scroller = GTK_ADJUSTMENT (gtk_adjustment_new(m_currentPos,0, m_totalSize, m_lineStep, m_pageStep, m_visibleSize));
    if (orientation == Qt::Vertical) 
	sb = gtk_vscrollbar_new(m_scroller);
    else 
	sb = gtk_hscrollbar_new(m_scroller);
    setGtkWidget(sb);
    g_signal_connect(m_scroller, "value-changed", G_CALLBACK (::value_changed), this);
}

QScrollBar::~QScrollBar()
{

}

void QScrollBar::setValue(int v)
{
    int maxPos = m_totalSize - m_visibleSize;
    if (v < 0) v = 0;
    if (v > maxPos)
        v = maxPos;
    if (m_currentPos == v)
        return; // Our value stayed the same.

    m_currentPos = v;
    gtk_adjustment_set_value(m_scroller, (gdouble) v);
    valueChanged(); // Emit the signal that indicates our value has changed.
}

void QScrollBar::setSteps(int lineStep, int pageStep)
{
    m_lineStep = lineStep;
    m_pageStep = pageStep;
    m_scroller->step_increment = lineStep;
    m_scroller->page_increment = pageStep;
    gtk_adjustment_changed(m_scroller);
}

void QScrollBar::setKnobProportion(int visibleArea, int totalArea)
{
    m_visibleSize = visibleArea;
    m_totalSize = totalArea;
    m_scroller->upper = totalArea;
    m_scroller->page_size = visibleArea;
    gtk_adjustment_changed(m_scroller);
}

void QScrollBar::valueChanged()
{
    m_valueChanged.call(m_currentPos);
}
