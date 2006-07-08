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

#include "KWQButton.h"

#include "KWQAssertions.h"
//#include "KWQCheckBox.h"
//#include "KWQKHTMLPart.h"
//#include "KWQNSViewExtras.h"
//#include "WebCoreBridge.h"


//#include "render_form.h"



static void
button_clicked(GtkButton *button, gpointer user_data)
{
    QButton* btn = static_cast<QButton*>(user_data);
    btn->clicked();
}

QButton::QButton(QWidget * parent, const char * name, int f)
    : QWidget(parent,name,f)
      ,m_clicked(this, SIGNAL(clicked()))
      ,m_handlerid(0)
      ,m_group(this)
{

}

QButton::~QButton()
{
    GtkWidget *w = getGtkWidget();
    
    if (w && m_handlerid){
	g_signal_handler_disconnect(w, m_handlerid);
   } 
}

void QButton::setGtkWidget(GtkWidget* widget)
{
    ASSERT(widget);
    ASSERT(GTK_IS_BUTTON(widget));
    
    GtkWidget *w = getGtkWidget();
    
    if (w == widget)
	return;
    
    if (w && m_handlerid) {
	g_signal_handler_disconnect(w, m_handlerid);
    }
    
    QWidget::setGtkWidget(widget);
    
    if (widget) {
	m_handlerid = g_signal_connect(G_OBJECT(widget),
				       "clicked",
				       G_CALLBACK(::button_clicked),
				       this);
    }  
}

void QButton::setText(const QString &s)
{
    QCString cstr = s.utf8();
    GtkWidget *w = getGtkWidget();
#if 0
    gtk_button_set_label(GTK_BUTTON (w), static_cast<const char*>(cstr));
#endif
    GtkWidget *l = gtk_bin_get_child( GTK_BIN(w) );
    gtk_label_set_text( GTK_LABEL(l), static_cast<const char*>(cstr) );
}

QString QButton::text() const
{
    const gchar* cstr = gtk_button_get_label(GTK_BUTTON (getGtkWidget()));    
    if (cstr)
	return QString::fromUtf8(cstr);
    else
	return QString();
}

void QButton::clicked()
{
    // Order of signals is:
    //   1) signals in subclasses (stateChanged, not sure if there are any others)
    //   2) mouse up
    //   3) clicked
    // Proper behavior of check boxes, at least, depends on this order.
    // [above is WebCore-specific comment, KWIQ status unknown]
    m_clicked.call();
}

void QButton::click()
{
    GtkButton *b = GTK_BUTTON( getGtkWidget() );
    ASSERT(b);
    gtk_button_clicked(b);
}

void QButton::setFont(const QFont &f)
{
    QWidget::setFont(f);
#if 0
    //FIXME INCLUDE ME
    GtkWidget *w = getGtkWidget();
    ASSERT(w);
    
    GtkRcStyle *style = gtk_widget_get_modifier_style( GTK_WIDGET(w) );
    PangoFontDescription *fd = style->font_desc;

    GtkWidget *child = gtk_bin_get_child( GTK_BIN(w) );
    ASSERT(child);
    gtk_widget_modify_font(child, fd);

    gtk_widget_show( child );
#endif    
}

void QButton::setWritingDirection(QPainter::TextDirection d)
{
    QWidget::setWritingDirection(d);
}

QWidget::FocusPolicy QButton::focusPolicy() const
{
    return QWidget::focusPolicy();
}  

