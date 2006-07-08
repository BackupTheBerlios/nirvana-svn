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
#include "KWQListBox.h"

#include "KWQAssertions.h"
#include "KWQLogging.h"


#define MIN_LINES 4 /* ensures we have a scroll bar */
#define BOTTOM_MARGIN = 1;
#define LEFT_MARGIN = 2;
#define RIGHT_MARGIN = 2;


extern "C" {
static void
selection_changed(GtkWidget *widget, gpointer data)
{
    QListBox* this_ = static_cast<QListBox*>(data);
    this_->selectionChanged();
}

#if 0    
static void
clicked(GtkWidget *widget, QLineEdit* edit)
{
    edit->clicked();
}
#endif

}


QListBox::QListBox(QWidget * parent, const char * name, int f)    
    : QScrollView(parent,name,f)
    , _changingSelection(false)
    , _enabled(true)
    , _widthGood(false)
    , _clicked(this, SIGNAL(clicked(QListBoxItem *)))
    , _selectionChanged(this, SIGNAL(selectionChanged()))
{
    _store = gtk_list_store_new( 1, G_TYPE_STRING );
    _treeview = gtk_tree_view_new_with_model( GTK_TREE_MODEL(_store) );
    g_object_unref(_store);
    
    GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
    GtkTreeViewColumn *column =
	gtk_tree_view_column_new_with_attributes( "",
						  renderer,
						  "text",
						  0,
						  NULL );
    
   /* Add the column to the view. */
   gtk_tree_view_append_column( GTK_TREE_VIEW(_treeview), column );
   gtk_tree_view_set_headers_visible(GTK_TREE_VIEW( _treeview), false );

   GtkWidget* swin = gtk_scrolled_window_new(NULL,NULL);
   gtk_container_add(GTK_CONTAINER (swin), _treeview);
   setGtkWidget(swin);

   gint hid;
   hid = g_signal_connect( G_OBJECT(_treeview),
			    "select-cursor-row",
			    G_CALLBACK(::selection_changed),
			    this );
   
   setVScrollBarMode(AlwaysOn);
}

QListBox::~QListBox()
{
}

// uint QListBox::count() const
// {
//     return (uint) gtk_tree_model_get_n_columns( GTK_TREE_MODEL(_store) );
// }

void QListBox::clear()
{
    gtk_list_store_clear( _store );
    _items.clear();
    _widthGood = false;
}

void QListBox::setSelectionMode(SelectionMode mode)
{
    gtk_tree_selection_set_mode(
	gtk_tree_view_get_selection( GTK_TREE_VIEW(_treeview) ),
	(mode == Single) ? GTK_SELECTION_SINGLE : GTK_SELECTION_MULTIPLE );
}

void QListBox::appendItem(const QString &text, bool isLabel)
{
    if (isLabel){
	LOG(NotYetImplemented, "KWIQ: QListBox::InsertItem with isLabel == true not implemented");
	return;
    }

    _items.append(QListBoxItem(text, isLabel));
    
    QCString cstr = text.utf8();
    
    GtkTreeIter iter;
    GValue* gv = g_value_init( g_new0(GValue, 1), G_TYPE_STRING );
    g_value_set_string( gv, static_cast<const char*>(cstr) );
    
    gtk_list_store_append( _store, &iter );    
    gtk_list_store_set_value( _store, &iter, 0, gv );

    _widthGood = false;
}

void QListBox::doneAppendingItems()
{
    // herehere, create gtk widgets delayed instead...
    
//     NSScrollView *scrollView = getView();
//     NSTableView *tableView = [scrollView documentView];
//     [tableView reloadData];

}

void QListBox::setSelected(int index, bool selectIt)
{
    _changingSelection = true;

    GtkTreeSelection *sel =
	gtk_tree_view_get_selection( GTK_TREE_VIEW(_treeview) );
    
//     bool multi = gtk_tree_selection_get_mode( sel ) == GTK_SELECTION_MULTIPLE;

    GtkTreePath *path = gtk_tree_path_new_from_indices( index, -1 );

    if (selectIt) {
	gtk_tree_selection_select_path( sel, path );
    } else {
	gtk_tree_selection_unselect_path( sel, path );
    }
    
    gtk_tree_path_free(path);    
    _changingSelection = false;
}

bool QListBox::isSelected(int index) const
{
    GtkTreeSelection *sel =
	gtk_tree_view_get_selection( GTK_TREE_VIEW(_treeview) );
    
    GtkTreePath *path = gtk_tree_path_new_from_indices( index, -1 );

    bool res = gtk_tree_selection_path_is_selected( sel, path );
    
    gtk_tree_path_free(path);
    return res;
}

void QListBox::setEnabled(bool enabled)
{
    _enabled = enabled;
}

bool QListBox::isEnabled()
{
    return _enabled;
}

QSize QListBox::sizeForNumberOfLines(int lines) const
{
    float width;
    if (_widthGood) {
        width = _width;
    } else {
        width = 12*7;
	//###FIXME: count the width
        /*NSCell *cell = [[[tableView tableColumns] objectAtIndex:0] dataCell];
        NSEnumerator *e = [_items objectEnumerator];
        NSString *text;
        while ((text = [e nextObject])) {
            [cell setStringValue:text];
            NSSize size = [cell cellSize];
            width = MAX(width, size.width);
	    }*/
        _width = width;
        _widthGood = true;
    }
    
/*    NSSize contentSize;
    contentSize.width = ceil(width);
    contentSize.height = ceil(([tableView rowHeight] + [tableView intercellSpacing].height) * MAX(MIN_LINES, lines));
    NSSize size = [NSScrollView frameSizeForContentSize:contentSize
    hasHorizontalScroller:NO hasVerticalScroller:YES borderType:NSBezelBorder];*/

    // hc.
    return QSize((int)width,12);
}

QWidget::FocusPolicy QListBox::focusPolicy() const
{
#if !KWIQ
    KWQ_BLOCK_EXCEPTIONS;
    
    // Add an additional check here.
    // For now, selects are only focused when full
    // keyboard access is turned on.
    unsigned keyboardUIMode = [KWQKHTMLPart::bridgeForWidget(this) keyboardUIMode];
    if ((keyboardUIMode & WebCoreKeyboardAccessFull) == 0)
        return NoFocus;
    
    KWQ_UNBLOCK_EXCEPTIONS;
#endif    
    return QScrollView::focusPolicy();
}

bool QListBox::checksDescendantsForFocus() const
{
    return true;
}

void QListBox::setWritingDirection(QPainter::TextDirection d)
{
    QWidget::setWritingDirection(d);
    //FIXME:KWIQ: QListBox setWritingDirection: for each entry do the magic
}

// bool QListBox::itemIsGroupLabel(int index) const
// {
//     ASSERT(index >= 0);

//     return false;
// }
