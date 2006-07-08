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

#include "KWQKCursor.h"
#include "KWQLogging.h"
#include "KWQDict.h"

#include <gdk/gdk.h>
#if 0
// Simple NSDictionary and NSCursor calls shouldn't need protection,
// but creating a cursor with a bad image might throw...


@interface KWQKCursorBundleDummy : NSObject { }
@end
@implementation KWQKCursorBundleDummy
@end

@interface NSCursor (WebCoreCursorAdditions)
+ (NSCursor *)_WebCore_cursorWithName:(NSString *)name hotSpot:(NSPoint)hotSpot;
@end

@implementation NSCursor (WebCoreCursorAdditions)

+ (NSCursor *)_WebCore_cursorWithName:(NSString *)name hotSpot:(NSPoint)hotSpot
{
    static NSMutableDictionary *nameToCursor = nil;
    if (!nameToCursor) {
        nameToCursor = [[NSMutableDictionary alloc] init];
    }
    
    KWQ_BLOCK_EXCEPTIONS;
    NSCursor * cursor = [nameToCursor objectForKey:name];
    if (!cursor) { 
	NSImage *cursorImage = [[NSImage alloc] initWithContentsOfFile:
            [[NSBundle bundleForClass:[KWQKCursorBundleDummy class]]
            pathForResource:name ofType:@"tiff"]];
        if (cursorImage) {
            cursor = [[NSCursor alloc] initWithImage:cursorImage hotSpot:hotSpot];
            [cursorImage release];
            [nameToCursor setObject:cursor forKey:name];
            [cursor release];
        }

    }
    return cursor;
    KWQ_UNBLOCK_EXCEPTIONS;
    
    return nil;
}

@end
#endif

struct CursorWrapper{
    GdkCursor* cursor;

    CursorWrapper(GdkCursor* c):cursor(c) {
    }
    ~CursorWrapper() {
	if (cursor) gdk_cursor_unref(cursor);
    }
};
    
    
GdkCursor* _cursorWithName(const gchar* name, const QPoint& hotSpot)
{
    static QDict<CursorWrapper> cursors;
    QString fileName(name);
    CursorWrapper* w = cursors.find(fileName);
    if (w){
	if (w->cursor) gdk_cursor_ref(w->cursor); // emulates creation of new cursor
	return w->cursor;
    }

    QString path(PKGDATADIR);
    path.append("/");
    path.append(fileName);
    path.append(".tiff");
    GdkPixbuf* buf = gdk_pixbuf_new_from_file(path.latin1(), NULL);
    if (!buf) {
	ERROR("couldn't find cursor image %s",path.latin1());
	return 0;
    }
    GdkCursor* cursor = 0;
#if (GTK_MAJOR_VERSION >= 2) && (GTK_MINOR_VERSION >=4)
    cursor =  gdk_cursor_new_from_pixbuf(GDK_DISPLAY(), buf, hotSpot.x(), hotSpot.y());
#else
    GdkPixmap *pixmap;
    GdkBitmap *mask;
    gdk_pixbuf_render_pixmap_and_mask(buf, &pixmap, &mask, 100);
    GdkColor bg = { 0, 65535, 65535, 65535 }; /* White. */
    GdkColor fg = { 0, 0, 0, 0 }; /* Black. */
    cursor = gdk_cursor_new_from_pixmap(mask, mask, &fg, &bg, hotSpot.x(), hotSpot.y());
    g_object_unref(pixmap);
    g_object_unref(mask);
#endif
    g_object_unref(buf);

    cursors.insert(fileName, new CursorWrapper(cursor));

    if (cursor) gdk_cursor_ref(cursor); // emulates creation of new cursor

    return cursor;
}

void KCursor::setAutoHideCursor(QWidget *w, bool enable) {}

QCursor KCursor::arrowCursor() { return QCursor(); }
QCursor KCursor::crossCursor() { return QCursor(gdk_cursor_new(GDK_CROSSHAIR)); }
QCursor KCursor::handCursor() { return QCursor( _cursorWithName("linkCursor", QPoint(6, 1))); }
QCursor KCursor::sizeAllCursor() { return QCursor( _cursorWithName("moveCursor", QPoint(7, 7))); }
QCursor KCursor::sizeHorCursor() { return QCursor(); }
QCursor KCursor::sizeVerCursor() { return QCursor(); }
QCursor KCursor::sizeBDiagCursor() { return QCursor(); }
QCursor KCursor::sizeFDiagCursor() { return QCursor(); }
QCursor KCursor::ibeamCursor() { return QCursor( gdk_cursor_new(GDK_XTERM)); } //IBeamCursor
QCursor KCursor::waitCursor() { return QCursor( _cursorWithName("waitCursor", QPoint(7, 7))); }
QCursor KCursor::whatsThisCursor() { return QCursor(_cursorWithName("helpCursor", QPoint(8, 8))); }

QCursor KCursor::eastResizeCursor() { return QCursor(_cursorWithName("eastResizeCursor", QPoint(14, 7))); }
QCursor KCursor::northResizeCursor() { return QCursor(_cursorWithName("northResizeCursor", QPoint(7, 1))); }
QCursor KCursor::northEastResizeCursor() { return QCursor(_cursorWithName("northEastResizeCursor", QPoint(14, 1))); }
QCursor KCursor::northWestResizeCursor() { return QCursor(_cursorWithName("northWestResizeCursor", QPoint(0, 0))); }
QCursor KCursor::southResizeCursor() { return QCursor(_cursorWithName("southResizeCursor", QPoint(7, 14))); }
QCursor KCursor::southEastResizeCursor() { return QCursor(_cursorWithName("southEastResizeCursor", QPoint(14, 14))); }
QCursor KCursor::southWestResizeCursor() { return QCursor( _cursorWithName("southWestResizeCursor", QPoint(1, 14))); }
QCursor KCursor::westResizeCursor() { return QCursor(_cursorWithName("westResizeCursor", QPoint(1, 7))); }
