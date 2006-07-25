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

#include "KWQCursor.h"
#include "KWQLogging.h"
//#include "KWQPixmap.h"
//#include "WebCoreImageRenderer.h"

#include <AppKit.h>

//#include <gdk/gdk.h>
QCursor::QCursor()
    : cursor(NULL)
{
}

QCursor::QCursor(BCursor *cur)
//QCursor::QCursor(GdkCursor *cur)
    : cursor(cur)
{
}
/*
QCursor::QCursor(const QPixmap &pixmap)
    : cursor(NULL)
{
    ERROR("not yet implemented");

    GdkPixbuf* buf = pixmap.image()->handle();
    QPoint hotSpot(0,0);
#if (GTK_MAJOR_VERSION >= 2) && (GTK_MINOR_VERSION >=4)
    cursor =  gdk_cursor_new_from_pixbuf(GDK_DISPLAY(), buf, hotSpot.x(), hotSpot.y());
#else
    GdkPixmap *gdkpixmap;
    GdkBitmap *mask;
    gdk_pixbuf_render_pixmap_and_mask(buf, &gdkpixmap, &mask, 100);
    GdkColor fg = { 0, 65535, 65535, 65535 }; // White.
    GdkColor bg = { 0, 0, 0, 0 }; // Black.
    cursor = gdk_cursor_new_from_pixmap(mask, mask, &fg, &bg, hotSpot.x(), hotSpot.y());
    g_object_unref(gdkpixmap);
    g_object_unref(mask);
#endif

}
*/
QCursor::QCursor(const QCursor &other)
    : cursor(other.cursor)
{
    //if (cursor)
    //	gdk_cursor_ref(cursor);

}

QCursor::~QCursor()
{
    //if (cursor) 
    //	gdk_cursor_unref(cursor);
}
      
QCursor &QCursor::operator=(const QCursor &other)
{
/*
    if (other.cursor) 
	gdk_cursor_ref(other.cursor);
    if (cursor) 
	gdk_cursor_unref(cursor);
*/
    cursor = other.cursor;
    return *this;
}

BCursor *QCursor::handle() const
{
    return cursor;
}
