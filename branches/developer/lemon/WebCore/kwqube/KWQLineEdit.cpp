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
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>

#include "KWQLineEdit.h"
#include "KWQLogging.h"
//#include "KWQTextField.h"

#define LINE_EDIT_HEIGHT_PADDING 3


extern "C" {
static void
changed(GtkWidget *widget, QLineEdit* edit)
{
    edit->setEdited(true);
    edit->textChanged();
}

#if 0 // removed compiler warning --psalmi
static void
clicked(GtkWidget *widget, QLineEdit* edit)
{
    edit->clicked();
}
#endif

static gboolean
keypressed(GtkWidget *widget, GdkEventKey *event, QLineEdit* edit)
{
    if (event->keyval == GDK_Return)
	edit->returnPressed();
    
    return FALSE;	
}
}

QLineEdit::QLineEdit(Type type, QWidget * parent, const char * name, int f)
    : QWidget(parent,name,f)
    , m_returnPressed(this, SIGNAL(returnPressed()))
    , m_textChanged(this, SIGNAL(textChanged(const QString &)))
    , m_clicked(this, SIGNAL(clicked()))
    , m_performSearch(this, SIGNAL(performSearch()))
    , m_type(type)
{
    GtkWidget *entry = gtk_entry_new();
    setGtkWidget(entry);
    setEdited(false);

    gint hid;
    hid = g_signal_connect( G_OBJECT(entry),
			    "changed",
			    G_CALLBACK(::changed),
			    this );
#if 0 // QLineEdit emiting clicked signal disabled atm. theres no "clicked" signal in gtk
    hid = g_signal_connect( G_OBJECT(entry),
			    "clicked",
			    G_CALLBACK(::clicked),
			    this );
#endif

    hid = g_signal_connect( G_OBJECT(entry),
			    "key-press-event",
			    G_CALLBACK(::keypressed),
			    this );

    // hide passwords 
    gtk_entry_set_visibility( GTK_ENTRY(getGtkWidget()), type != Password );

}

QLineEdit::~QLineEdit()
{
    
}
 

void QLineEdit::setCursorPosition(int p)
{
    // Don't do anything here.
    //gtk_editable_set_position( GTK_EDITABLE(getGtkWidget()), p );
}

int QLineEdit::cursorPosition() const
{
    // Not needed.  We ignore setCursorPosition().
    return 0;
    //return gtk_editable_get_position( GTK_EDITABLE(getGtkWidget()) );
}

void QLineEdit::setFont(const QFont &font)
{
    QWidget::setFont(font);
}

void QLineEdit::setText(const QString &s)
{
    GtkWidget *w = getGtkWidget();
    QCString str = s.utf8();
    gtk_entry_set_text( GTK_ENTRY(w),
		        static_cast<const char*>(str) );
    setEdited(false);
}

QString QLineEdit::text()
{
    GtkWidget *w = getGtkWidget();

    return QString::fromUtf8( gtk_entry_get_text(GTK_ENTRY(w)) );
}

void QLineEdit::setMaxLength(int len)
{
    GtkWidget *w = getGtkWidget();
    gtk_entry_set_max_length( GTK_ENTRY(w), len );
}

bool QLineEdit::isReadOnly() const
{
    GtkWidget *w = getGtkWidget();
    return !gtk_editable_get_editable( GTK_EDITABLE(w) );
}

void QLineEdit::setReadOnly(bool flag)
{
    GtkWidget *w = getGtkWidget();
    gtk_editable_set_editable( GTK_EDITABLE(w), !flag );
}

int QLineEdit::maxLength() const
{
    GtkWidget *w = getGtkWidget();
    return gtk_entry_get_max_length( GTK_ENTRY(w) );
}

void QLineEdit::selectAll()
{
    GtkWidget *w = getGtkWidget();
    gtk_editable_select_region( GTK_EDITABLE(w), 0, -1);
}

bool QLineEdit::edited() const
{
    return m_edited;
}

void QLineEdit::setEdited(bool flag)
{
    m_edited = flag;
}

/**
 *  has side effects: sets the width of chars to the desired value
 *  Qt lacks interface to set  (approximate) size according to how many
 *  characters QLineEdit contains.
 *
 *  this doesn't save old width_chars, set new, set old,
 *  because gtk queues resize on set
 */
QSize QLineEdit::sizeForCharacterWidth(int numCharacters) const
{
    ASSERT(numCharacters > 0);

    GtkWidget *w;
    GtkRequisition req;
    w = getGtkWidget();
    gtk_entry_set_width_chars( GTK_ENTRY(w), numCharacters );
    
    gtk_widget_size_request( w, &req );
    return QSize( req.width, req.height );
}

int QLineEdit::baselinePosition( int height) const
{
    return QWidget::baselinePosition(height);
}

void QLineEdit::clicked()
{
    m_clicked.call();
}

void QLineEdit::setAlignment(AlignmentFlags alignment)
{
#if 0
    KWQ_BLOCK_EXCEPTIONS;

    NSTextField *textField = (NSTextField *)getView();
    [textField setAlignment:KWQNSTextAlignmentForAlignmentFlags(alignment)];

    KWQ_UNBLOCK_EXCEPTIONS;
#endif
}

void QLineEdit::setWritingDirection(QPainter::TextDirection direction)
{
    QWidget::setWritingDirection(direction);
}

QWidget::FocusPolicy QLineEdit::focusPolicy() const
{
    FocusPolicy policy = QWidget::focusPolicy();
    return policy == TabFocus ? StrongFocus : policy;
}

bool QLineEdit::checksDescendantsForFocus() const
{
    return true;
}

#if 0
NSTextAlignment KWQNSTextAlignmentForAlignmentFlags(Qt::AlignmentFlags a)
{
    switch (a) {
        default:
            ERROR("unsupported alignment");
        case Qt::AlignLeft:
            return NSLeftTextAlignment;
        case Qt::AlignRight:
            return NSRightTextAlignment;
        case Qt::AlignHCenter:
            return NSCenterTextAlignment;
    }
}
#endif

void QLineEdit::setLiveSearch(bool liveSearch)
{
    if (m_type != Search)
        return;

    LOG(KwiqLog, "void QLineEdit::setLiveSearch(bool) Not implemented");

//     NSSearchField *searchField = (NSSearchField *)getView();
//     [[searchField cell] setSendsWholeSearchString:!liveSearch];
}

void QLineEdit::setAutoSaveName(const QString& name)
{
    if (m_type != Search)
        return;
    
    LOG(KwiqLog,"void QLineEdit::setAutoSaveName(QString&) Not implemented");

//     QString autosave;
//     if (!name.isEmpty())
//         autosave = "com.apple.WebKit.searchField:" + name;
    
//     NSSearchField *searchField = (NSSearchField *)getView();
//     [searchField setRecentsAutosaveName:autosave.getNSString()];
}

void QLineEdit::setMaxResults(int maxResults)
{
    if (m_type != Search)
        return;

    LOG(KwiqLog,"void QLineEdit::setMaxResults(int) Not implemented");
        
//     NSSearchField *searchField = (NSSearchField *)getView();
//     id searchCell = [searchField cell];
//     if (!maxResults) {
//         [searchCell setSearchButtonCell:nil];
//         [searchCell setSearchMenuTemplate:nil];
//     }
//     else {
//         NSMenu* cellMenu = [searchCell searchMenuTemplate];
//         NSButtonCell* buttonCell = [searchCell searchButtonCell];
//         if (!buttonCell)
//             [searchCell resetSearchButtonCell];
//         if (!cellMenu)
//             [searchCell setSearchMenuTemplate:[[WebCoreViewFactory sharedFactory] cellMenuForSearchField]];
//     }
    
//     [searchCell setMaximumRecents:maxResults];
}

void QLineEdit::setPlaceholderString(const QString& placeholder)
{
    if (m_type != Search)
        return;

    LOG(KwiqLog,"void QLineEdit::setPlaceholderString(QString&) Not implemented");

//     NSSearchField *searchField = (NSSearchField *)getView();
//     [[searchField cell] setPlaceholderString:placeholder.getNSString()];
}

void QLineEdit::addSearchResult()
{
    if (m_type != Search)
        return;

    LOG(KwiqLog,"void QLineEdit::addSearchResult() Not implemented");

//     NSSearchField *searchField = (NSSearchField *)getView();
//     [[searchField cell] _addStringToRecentSearches:[searchField stringValue]];
}

