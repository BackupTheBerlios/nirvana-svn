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

#include "KWQTextEdit.h"

#include "KWQAssertions.h"
//#include "KWQTextArea.h"

#include "KWQLogging.h"

extern "C" {
#if 0 // remove warnings --psalmi
static void 
button_press(GtkWidget* widget, GdkEventButton *event, gpointer data)
{
    QTextEdit* edit = static_cast<QTextEdit*>(data);
    edit->clicked();
}
#endif

static void 
text_changed(GtkWidget *textbuffer, gpointer data){
    QTextEdit* edit = static_cast<QTextEdit*>(data);
    edit->textChanged();
}
}

QTextEdit::QTextEdit(QWidget * parent, const char * name, int f)
    : QScrollView(parent,name,f)
    , _clicked(this, SIGNAL(clicked()))
    , _textChanged(this, SIGNAL(textChanged()))
{
    _textview = gtk_text_view_new();
    _textbuf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(_textview));
#if 0
    g_signal_connect( G_OBJECT(_textview),
		      "button_press_event", 
		      G_CALLBACK(::button_press),
		      this );
#endif    
    g_signal_connect( G_OBJECT(_textbuf),
		      "changed", 
		      G_CALLBACK(::text_changed),
		      this );

    GtkWidget *sw = gtk_scrolled_window_new (NULL,NULL);
    gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW (sw), GTK_SHADOW_IN);    
    gtk_container_add(GTK_CONTAINER (sw), _textview);
    setGtkWidget(sw);

    setReadOnly(false);
    setVScrollBarMode(QScrollView::Auto);
    setHScrollBarMode(QScrollView::Auto);
}


void QTextEdit::setText(const QString &string)
{
    QCString cstr( string.utf8() );
    gtk_text_buffer_set_text(_textbuf,
			     static_cast<const char*>(cstr),
			     cstr.length());
}

QString QTextEdit::text() const
{
    GtkTextIter s,e;
    gtk_text_buffer_get_start_iter(_textbuf, &s);
    gtk_text_buffer_get_end_iter(_textbuf, &e);
    
    gchar *cbuf = gtk_text_buffer_get_text(_textbuf, &s, &e, FALSE);
    QString str(QString::fromUtf8(cbuf));
    g_free(cbuf);
    
    return str;
}

QString QTextEdit::textWithHardLineBreaks() const
{
    LOG(NotYetImplemented, "KWIQ");
    return text();
}

void QTextEdit::getCursorPosition(int *paragraph, int *index) const
{
    LOG(NotYetImplemented, "KWIQ");
#if 0
    GtkTextMark *mark = gtk_text_buffer_get_insert(_textbuf);
    GtkTextIter s;
    gtk_text_buffer_get_iter_at_mark(_textbuf,&s,mark);
    // FIXME: QTextEdit uses paragraph:index, gtktextview uses line,offset
    *index = gtk_text_iter_get_offset(&s);
    *paragraph = gtk_text_iter_get_line(&s);    
#endif
}

void QTextEdit::setCursorPosition(int paragraph, int index)
{
    LOG(NotYetImplemented,"KWIQ");
#if 0
    GtkTextMark *mark = gtk_text_buffer_get_insert(_textbuf);
    GtkTextIter s;
    gtk_text_buffer_get_iter_at_mark(_textbuf,&s,mark);
    // FIXME: QTextEdit uses paragraph:index, gtktextview uses line,offset

    gtk_text_iter_set_line(&s, paragraph);
    gtk_text_iter_set_offset(&s,index);
#endif
}

QTextEdit::WrapStyle QTextEdit::wordWrap() const
{
    bool wrap = gtk_text_view_get_wrap_mode( GTK_TEXT_VIEW(_textview) ) == GTK_WRAP_WORD ;
    return wrap ? WidgetWidth : NoWrap;
}

void QTextEdit::setWordWrap(WrapStyle style)
{
    GtkWrapMode wrap =  (style == WidgetWidth) ? GTK_WRAP_WORD : GTK_WRAP_NONE;
    gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW(_textview), wrap );
}

void QTextEdit::setTextFormat(TextFormat)
{
}

void QTextEdit::setTabStopWidth(int)
{
}

bool QTextEdit::isReadOnly() const
{
    return !gtk_text_view_get_editable(GTK_TEXT_VIEW (_textview));
}

void QTextEdit::setReadOnly(bool flag)
{
    gtk_text_view_set_editable(GTK_TEXT_VIEW (_textview), flag?FALSE:TRUE);
}

bool QTextEdit::isDisabled() const
{
    return !QWidget::isEnabled();
}

void QTextEdit::setDisabled(bool flag)
{
    QWidget::setEnabled(!flag);
}
     
void QTextEdit::selectAll()
{
    GtkTextIter s,e;
    GtkTextMark *sel_mark;
    gtk_text_buffer_get_start_iter( _textbuf, &s );
    gtk_text_buffer_get_end_iter( _textbuf, &e );
    gtk_text_buffer_place_cursor( _textbuf, &s );
    
    sel_mark = gtk_text_buffer_get_selection_bound( _textbuf );
    gtk_text_buffer_move_mark( _textbuf, sel_mark, &e );
}

void QTextEdit::setFont(const QFont &font)
{
    QWidget::setFont(font);
}

void QTextEdit::clicked()
{
    _clicked.call();
}

void QTextEdit::setAlignment(AlignmentFlags alignment)
{
#if 0
    KWQ_BLOCK_EXCEPTIONS;

    KWQTextArea *textArea = getView();
    [textArea setAlignment:KWQNSTextAlignmentForAlignmentFlags(alignment)];

    KWQ_UNBLOCK_EXCEPTIONS;
#endif
    LOG(NotYetImplemented,"KWIQ");
}

void QTextEdit::setWritingDirection(QPainter::TextDirection direction)
{
    QWidget::setWritingDirection(direction);
}
 
QSize QTextEdit::sizeWithColumnsAndRows(int numColumns, int numRows) const
{
    LOG(NotYetImplemented,"KWIQ");
    //FIXME: KWIQ: implement sizeWithColumnsAndRows
    return QSize((int)(numColumns * 15), (int)(numRows * 15));
}

bool QTextEdit::checksDescendantsForFocus() const
{
    return true;
}

bool QTextEdit::hasFocus() const
{
    return GTK_WIDGET_HAS_FOCUS (_textview);
}

void QTextEdit::setFocus()
{
    if (hasFocus()) {
        return;
    }
    g_return_if_fail(GTK_WIDGET_CAN_FOCUS (_textview));
    gtk_widget_grab_focus(_textview);
}
