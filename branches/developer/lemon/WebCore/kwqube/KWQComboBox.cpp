/*
 * Copyright (C) 2004 Apple Computer, Inc.  All rights reserved.
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

#ifdef GTK_DISABLE_DEPRECATED
#undef GTK_DISABLE_DEPRECATED //FIXME: GtkList is deprecated
#endif
#include <glib.h>
#include <gtk/gtk.h>

#include "KWQComboBox.h"
#include "KWQApplication.h"

#include "KWQLogging.h"

extern "C" {
static void
select_child(GtkList *list, GtkWidget* widget, gpointer data )
{
    QComboBox* this_ = static_cast<QComboBox*>(data);
    this_->on_select_child();
}
};
	
void
QComboBox::on_select_child()
{
    updateCurrentItem();    
}


QComboBox::QComboBox(QWidget * parent, const char * name, int f)
    : QWidget( parent, name, f )
      ,_width(0)
      ,_widthGood(false)
      , _currentItem(0)
      ,_menuPopulated(true) 
      //, labelFont(0)
      ,_activated(this, SIGNAL(activated(int)))
{
    GtkWidget *combo = gtk_combo_new();
    setGtkWidget(combo);
    
    _items = NULL;

    gtk_editable_set_editable( GTK_EDITABLE( GTK_COMBO(combo)->entry ), false );
    gtk_combo_set_value_in_list( GTK_COMBO(combo), TRUE, TRUE );
    g_signal_connect( G_OBJECT( GTK_COMBO(combo)->list ),
		      "select_child",
		      G_CALLBACK( ::select_child ),
		      this);

    gtk_entry_set_width_chars( GTK_ENTRY(GTK_COMBO(combo)->entry), 1 );
}

QComboBox::~QComboBox()
{
    /* GtkCombo destorys the labels that are created to the _items list */
    g_list_free(_items);
}

#if !KWIQ
void QComboBox::setTitle(NSMenuItem *menuItem, const QListBoxItem &title)
{
    if (title.isGroupLabel) {
        NSDictionary *attributes = [[NSDictionary alloc] initWithObjectsAndKeys:labelFont(), NSFontAttributeName, nil];
        NSAttributedString *string = [[NSAttributedString alloc] initWithString:title.string.getNSString() attributes:attributes];
        [menuItem setAttributedTitle:string];
        [string release];
        [attributes release];
        [menuItem setAction:NULL /*@selector(fakeSelectorForDisabledItem)*/];
    } else {
        [menuItem setTitle:title.string.getNSString()];
    }
}
#endif

void QComboBox::appendItem(const QString &text, bool isLabel)
{
    GtkCombo *combo = GTK_COMBO(getGtkWidget());
    ASSERT(combo);
    
    QCString cstr = text.utf8();
    
    guint curlen = gtk_entry_get_width_chars( GTK_ENTRY(combo->entry) );
    
    if ( curlen < text.length() )
	gtk_entry_set_width_chars( GTK_ENTRY(combo->entry), text.length()+1 );

    /* create new list item to be added to the list. */
    /* new listitem is deleted when GtkCombo is deleted */ 
    GtkWidget *item = gtk_list_item_new_with_label( static_cast<const char*>(cstr) );
    
    /* set the parent font */       
    GtkRcStyle *style = gtk_widget_get_modifier_style( GTK_WIDGET(combo) );
    PangoFontDescription *fd = style->font_desc;    
    GtkWidget *item_label = gtk_bin_get_child( GTK_BIN(item) );
    gtk_widget_modify_font(item_label, fd);

    gtk_widget_show( item );
    
    _items = g_list_insert(_items, item, -1);
    gtk_container_add( GTK_CONTAINER(combo->list), item );
}


QSize QComboBox::sizeHint() const 
{        
    return QWidget::sizeHint();
#if 0
    GtkWidget *w = getGtkWidget();
    ASSERT(w);
    
    GtkRequisition req;
    gtk_widget_size_request( GTK_COMBO(w)->list, &req );    
    gint width = req.width;
    gtk_widget_size_request( w, &req );

    return QSize( MAX( width, req.width ), req.height );
#endif
}

QRect QComboBox::frameGeometry() const
{
    return QWidget::frameGeometry();
}

void QComboBox::setFrameGeometry(const QRect &r)
{
    QWidget::setFrameGeometry(r);
}

int QComboBox::baselinePosition(int aheight) const
{
    return QWidget::baselinePosition(aheight);
#if 0    
    return (int)ceil( -dimensions()[topMargin] + dimensions()[baselineFudgeFactor] + [[button font] ascender]);
#endif    
}

void QComboBox::clear()
{
    if (g_list_length(_items) == 0) return;
    GtkWidget *w = getGtkWidget();
    
    gtk_list_clear_items( GTK_LIST(GTK_COMBO(w)->list), 0, -1 );
    
    g_list_free(_items);
    _items = 0;

    _currentItem = 0;
    _menuPopulated = true;
}

void QComboBox::setCurrentItem(int index)
{
    GtkWidget *w = getGtkWidget();
    gtk_list_select_item( GTK_LIST(GTK_COMBO(w)->list), index );
}

bool QComboBox::updateCurrentItem() const
{
    GtkWidget *combo = getGtkWidget();
    GList* sel = GTK_LIST(GTK_COMBO(combo)->list)->selection;
    int i = -1;
    if (sel) 
	i = g_list_index(_items, sel->data);
    
    if (_currentItem == i)
        return false;
    
    _currentItem = i;
    return true;
}

// void QComboBox::itemSelected() //FIXME: KWIQ: call this from select event
// {
//     if (updateCurrentItem()) {
//         _activated.call(_currentItem);
//     }
// }

void QComboBox::setFont(const QFont &font)
{
    QWidget::setFont(font);
    
    GtkWidget *w = getGtkWidget();
    GtkWidget *entry = GTK_COMBO(w)->entry;
    GtkWidget *list = GTK_COMBO(w)->list;

    GtkRcStyle *style = gtk_widget_get_modifier_style(GTK_WIDGET(w));
    PangoFontDescription *fd = style->font_desc;
    
    gtk_widget_modify_font( entry, fd );
    gtk_widget_modify_font( list, fd );

    GList* cur = _items;
    while (cur) {
	if (cur->data) {
	    gtk_widget_modify_font(GTK_WIDGET(cur->data), fd);
	}
	cur = cur->next;
    }   

}

const int *QComboBox::dimensions() const
{
    // We empirically determined these dimensions.
    // It would be better to get this info from AppKit somehow.
    static const int w[3][7] = {
        { 2, 3, 3, 3, 4, 34, 9 },
        { 1, 3, 3, 3, 3, 31, 5 },
        { 0, 0, 1, 1, 2, 32, 0 }
    };
#if 0    
    NSControl * const button = static_cast<NSControl *>(getView());

    KWQ_BLOCK_EXCEPTIONS;
    return  w[[[button cell] controlSize]];
    KWQ_UNBLOCK_EXCEPTIONS;
#endif
    return w[0];
}

bool QComboBox::hasFocus() const
{
    return GTK_WIDGET_HAS_FOCUS (GTK_COMBO (getGtkWidget())->entry);

}

void QComboBox::setFocus()
{
    if (hasFocus()) {
        return;
    }
    g_return_if_fail(GTK_WIDGET_CAN_FOCUS (GTK_COMBO (getGtkWidget())->entry));
    gtk_widget_grab_focus(GTK_COMBO (getGtkWidget())->entry);
}

QWidget::FocusPolicy QComboBox::focusPolicy() const
{
    return QWidget::focusPolicy();
}

void QComboBox::setWritingDirection(QPainter::TextDirection direction)
{
    QWidget::setWritingDirection(direction);
}

void QComboBox::populateMenu()
{
    if (!_menuPopulated) {
#if !KWIQ // KWIQ fixme: lazy initialization required here
        KWQ_BLOCK_EXCEPTIONS;

        KWQPopUpButton *button = getView();
	[button setPopulatingMenu:YES];
        [button removeAllItems];
        QValueListConstIterator<KWQListBoxItem> i = const_cast<const QValueList<KWQListBoxItem> &>(_items).begin();
        QValueListConstIterator<KWQListBoxItem> e = const_cast<const QValueList<KWQListBoxItem> &>(_items).end();
        for (; i != e; ++i) {
            // We must add the item with no title and then set the title because
            // addItemWithTitle does not allow duplicate titles.
            [button addItemWithTitle:@""];
            NSMenuItem *menuItem = [button lastItem];
            setTitle(menuItem, *i);
        }
        [button selectItemAtIndex:_currentItem];
	[button setPopulatingMenu:NO];

        KWQ_UNBLOCK_EXCEPTIONS;
#endif
        
        _menuPopulated = true;
    }
}

void QComboBox::populate()
{
    populateMenu();
}
