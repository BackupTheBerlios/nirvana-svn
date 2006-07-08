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
#include "KWQPushButton.h"
#include "KWIQMacros.h"
// We empirically determined that buttons have these extra pixels on all
// sides. It would be better to get this info from AppKit somehow.
#define TOP_MARGIN 0 //4
#define BOTTOM_MARGIN 0 //6
#define LEFT_MARGIN 0
#define RIGHT_MARGIN 0


enum {
    topMargin,
    bottomMargin,
    leftMargin,
    rightMargin,
    baselineFudgeFactor
};

QPushButton::QPushButton(QWidget * parent, const char * name)
    :QButton(parent,name)
{
    GtkWidget *w = gtk_button_new_with_label("");
    setGtkWidget(w);
}

QPushButton::QPushButton(const QString &text, QWidget * parent, const char * name)
    :QButton(parent,name)
{
    QCString cstr = text.utf8();
    
    GtkWidget *w =
	gtk_button_new_with_label(static_cast<const char*>(cstr));
    
    setGtkWidget(w);
}

QSize QPushButton::sizeHint() const 
{
    return QButton::sizeHint();
}

QRect QPushButton::frameGeometry() const
{
    return QButton::frameGeometry();
}

void QPushButton::setFrameGeometry(const QRect &r)
{
    QButton::setFrameGeometry(r);
}

int QPushButton::baselinePosition(int height) const
{
    return (int) ((15.0f/20.0f)*(float)height);
#if 0
    GtkWidget *w = getGtkWidget();
    PangoContext *pc= gtk_widget_get_pango_context(w);
    PangoFontDescription *fd = pango_context_get_font_description(pc);
    PangoFontMetrics *fm = pango_context_get_metrics(pc,fd,NULL); //lang=NULL    
    
    float ascender = pango_font_metrics_get_ascent(fm) / PANGO_SCALE;
    float descender =pango_font_metrics_get_descent(fm) / PANGO_SCALE;
    
    return (int)ceil(- TOP_MARGIN
        + ((height() + TOP_MARGIN + BOTTOM_MARGIN) - (ascender - descender)) / 2.0
        + ascender - VERTICAL_FUDGE_FACTOR);
#endif
}

void QPushButton::setFont(const QFont &font)
{
    //g_warning("QPushButton::setFont: size:%d, family:%s, text: %s", font.pixelSize(), font.family().latin1(),text().latin1());
    QButton::setFont(font);
    
#if 0    
    GtkWidget *button = getGtkWidget();
    
    if (button){
	PangoFontDescription *fd;
	GtkWidget *child,*newchild;
	PangoAttrList* al;
	PangoAttribute* afd;
	
	child = gtk_bin_get_child(GTK_BIN(button));
	newchild = gtk_label_new(text().utf8());
	fd = createPangoFontDescription(&font);
	gtk_widget_modify_font(button, fd);
	gtk_widget_modify_font(child, fd);
	gtk_container_remove(GTK_CONTAINER(button),child);
	gtk_container_add(GTK_CONTAINER(button),newchild);
	child = newchild;
	
	

	afd  = pango_attr_font_desc_new(fd);

	al = gtk_label_get_attributes( GTK_LABEL(child) );
	if (al == NULL)
	{
	    al = pango_attr_list_new();
	    pango_attr_list_insert(al,afd);
	}else
	{
	    pango_attr_list_change(al,afd);
	}
	
	gtk_label_set_attributes( GTK_LABEL(child), al );
	gtk_widget_show( child );
	//g_warning("child:%x fd,%x",child,fd);	
	pango_font_description_free(fd);
    }
#endif
}

const int *QPushButton::dimensions() const
{
    // We empirically determined these dimensions.
    // It would be better to get this info from AppKit somehow.
    static const int w[3][5] = {
        { 4, 7, 6, 6, 2 },
        { 4, 6, 5, 5, 2 },
        { 0, 1, 1, 1, 1 }
    };

    return w[0];
 
}
