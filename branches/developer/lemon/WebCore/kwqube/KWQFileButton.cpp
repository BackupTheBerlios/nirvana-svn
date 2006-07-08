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

#include "KWQFileButton.h"
#include "KWQLogging.h"

#include "KWQAssertions.h"
#include "KWQKHTMLPart.h"
#include "khtmlview.h"
#include "WebCoreViewFactory.h"
// NSString *WebCoreFileButtonFilenameChanged = @"WebCoreFileButtonFilenameChanged";
// NSString *WebCoreFileButtonClicked = @"WebCoreFileButtonClicked";


// listeners for text input events.
extern "C" {
// static void
// changed(GtkWidget *widget, KWQFileButton* btn)
// {
//     btn->filenameChanged(NULL);
// }

static gboolean
keypressed(GtkWidget *widget, GdkEventKey *event, KWQFileButton* btn)
{
    if (event->keyval == GDK_Return)
        btn->filenameChanged();
    
    return FALSE;	
}
    
static void
button_clicked(GtkButton *button, gpointer data)
{
    ASSERT(data);

    KWQFileButton * btn = static_cast<KWQFileButton *>(data);
    btn->clicked();
}

static void
filename_changed(GtkButton *button, gpointer data)
{
    ASSERT(data);

    KWQFileButton * btn = static_cast<KWQFileButton *>(data);
    btn->filenameChanged();
}

static void
close_selection(GtkButton *button, gpointer data)
{
    ASSERT(data);
    
    KWQFileButton * btn = static_cast<KWQFileButton *>(data);
    btn->closeSelection();
}
}



// @interface KWQFileButtonAdapter : NSObject
// {
//     KWQFileButton *button;
// }

// - initWithKWQFileButton:(KWQFileButton *)button;

// @end

KWQFileButton::KWQFileButton(KHTMLPart *part,  QWidget * parent, const char * name, int f)
    : QWidget(parent, name, f)
    , m_part(part)
    , m_clicked(this, SIGNAL(clicked()))
    , m_textChanged(this, SIGNAL(textChanged(const QString &)))
    , m_chooser(0)
//     , m_adapter([[KWQFileButtonAdapter alloc] initWithKWQFileButton:this])
{

    m_entry = gtk_entry_new();

    QCString cstr(WebCoreViewFactory::sharedFactory()->fileButtonDefaultLabel());    
    m_button = gtk_button_new_with_label(static_cast<const char*>(cstr));
   
    GtkWidget *hbox = gtk_hbox_new(false, 10);
    
    gtk_box_pack_start(GTK_BOX(hbox), m_entry, false, false, 0);
    gtk_box_pack_start(GTK_BOX(hbox), m_button, false, false, 0);
    
    setGtkWidget(hbox);

    gint hid;
    hid = g_signal_connect( G_OBJECT(m_entry),
			    "key-press-event",
			    G_CALLBACK(::keypressed),
			    this );
    hid = g_signal_connect( G_OBJECT(m_button),
			    "clicked",
			    G_CALLBACK(::button_clicked),
			    this );

#if 0 // QLineEdit emiting clicked signal disabled atm. theres no "clicked" signal in gtk
    hid = g_signal_connect( G_OBJECT(m_entry),
			    "clicked",
			    G_CALLBACK(::clicked),
			    this );
#endif
}

KWQFileButton::~KWQFileButton()
{
    if (m_chooser) {
        gtk_widget_destroy(m_chooser);
    }
//     _adapter->button = 0;
//     delete _adapter;
}

void KWQFileButton::setFilename(const QString &f)
{
    GtkWidget *edit = getGtkEntryWidget();
    QCString str = f.utf8();
    gtk_entry_set_text( GTK_ENTRY(edit), static_cast<const char*>(str) );
}

QString KWQFileButton::filename()
{
    GtkWidget *edit = getGtkEntryWidget();

    return QString::fromUtf8( gtk_entry_get_text(GTK_ENTRY(edit)) );
}


QSize KWQFileButton::sizeForCharacterWidth(int characters) const
{
    ASSERT(characters > 0);

    GtkWidget *w;
    GtkRequisition req, req2;

    // get the size from input
    w = getGtkEntryWidget();    
    gtk_entry_set_width_chars( GTK_ENTRY(w), characters );        
    gtk_widget_size_request( w, &req );

    // get the size from button
    w = getGtkButtonWidget();
    gtk_widget_size_request( w, &req2 );    
   
    return QSize( req.width + req2.width + 10, QMAX(req.height, req2.height) );
}

QRect KWQFileButton::frameGeometry() const
{
    return QWidget::frameGeometry();
}

void KWQFileButton::setFrameGeometry(const QRect &rect)
{
    QWidget::setFrameGeometry(rect);
}

int KWQFileButton::baselinePosition(int height) const
{
    return (int) ((15.0f/20.0f)*(float)height);
}

void KWQFileButton::filenameChanged()
{
    if (m_chooser) {
        const QString filename(gtk_file_selection_get_filename (GTK_FILE_SELECTION (m_chooser)));   
        setFilename(filename);    
        closeSelection();
    }

    m_textChanged.call(filename());
}

void KWQFileButton::closeSelection()
{
    if (m_chooser) {
        gtk_widget_destroy(m_chooser);
    }
    m_chooser = 0;
}

void KWQFileButton::clicked()
{
    if (m_chooser) return;
    
    /* Create the selector */    
    m_chooser = gtk_file_selection_new("Upload file");
    gtk_window_set_destroy_with_parent(GTK_WINDOW(m_chooser), true);
    
    g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (m_chooser)->ok_button),
                      "clicked",
                      G_CALLBACK (::filename_changed),
                      (gpointer) this);

    g_signal_connect (GTK_OBJECT (GTK_FILE_SELECTION (m_chooser)->cancel_button),
                      "clicked",
                      G_CALLBACK (::close_selection),
                      (gpointer) this); 

    /* Display that dialog */    
    gtk_widget_show (m_chooser);
    
    m_clicked.call();
}

QWidget::FocusPolicy KWQFileButton::focusPolicy() const
{
    FocusPolicy policy = QWidget::focusPolicy();
    return policy == TabFocus ? StrongFocus : policy;
}


// @implementation KWQFileButtonAdapter

// - initWithKWQFileButton:(KWQFileButton *)b
// {
//     [super init];
//     button = b;
//     [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(filenameChanged:)
//         name:WebCoreFileButtonFilenameChanged object:b->getView()];
//     [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(clicked:)
//         name:WebCoreFileButtonClicked object:b->getView()];
//     return self;
// }

// - (void)dealloc
// {
//     [[NSNotificationCenter defaultCenter] removeObserver:self];
//     [super dealloc];
// }

// - (void)filenameChanged:(NSNotification *)notification
// {
//     button->filenameChanged();
// }

// -(void)clicked:(NSNotification *)notification
// {
//     button->sendConsumedMouseUp();
//     button->clicked();
// }

// @end
