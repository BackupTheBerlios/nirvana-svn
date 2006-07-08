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

#ifndef QBUTTON_H_
#define QBUTTON_H_

#include "KWQWidget.h"
#include "KWQString.h"
#include "KWQSignal.h"

#include "KWQButtonGroup.h"
#include <glib.h> /* gulong */


class QButton : public QWidget {
public:
    QButton(QWidget * parent = 0, const char * name = 0, int f = 0);
    ~QButton();

    virtual void setText(const QString &);
    QString text() const;

    void setWritingDirection(QPainter::TextDirection);
     
    virtual void clicked();
    virtual void click();

    // QWidget overrides
    virtual void setFont(const QFont &);
    virtual FocusPolicy focusPolicy() const;    
        
#if KWIQ
    virtual QButtonGroup *group() { return &m_group;}
    virtual void setGtkWidget(GtkWidget* widget);    
#endif    
 private:    
    KWQSignal m_clicked;
    
#if KWIQ
    gulong m_handlerid;
    QButtonGroup m_group;
#endif
    
};


#endif
