/*
 * Copyright (c) 2004 Nokia. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * Neither the name of Nokia nor the names of its contributors may be
 * used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef XftTextRenderer_h
#define XftTextRenderer_h

#include <gdk/gdk.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

#include "NRCore/WebCoreTextRenderer.h"

class XftNSFont;
class XftTextRendererFactory;

// class assumes that in each point, CGContext's clip region is relative to origin
// and sets the gc clip origin to 0,0 after clip origin changes
class XftTextRenderer : public WebCoreTextRenderer
{
 public:

    float floatWidthForRun(const WebCoreTextRun *run, const WebCoreTextStyle *style, float *widths);

    void setContext(CGContextRef context);
    
    void drawRun(const WebCoreTextRun *run, const WebCoreTextStyle *style, int x, int y);
    void drawHighlightForRun(const WebCoreTextRun *run, const WebCoreTextStyle *style, const WebCoreTextGeometry *geom);
    void drawLineForCharacters(int x, int y, float yOffset, int width, GdkColor* color);
    
    int pointToOffset(const WebCoreTextRun *run, const WebCoreTextStyle *style, int x, bool reversed, bool includePartialGlyphs);

    
    void detach() {owner = 0;}

    ~XftTextRenderer();
 protected:
    XftTextRenderer(XftTextRendererFactory* owner, XftNSFont *font);


 private:
    void drawRect(int x, int y, int w, int h, const GdkColor* color);
    float measureRange(const WebCoreTextRun *run, const WebCoreTextStyle *style, int from, int to, float* widths);
    float drawRange(const WebCoreTextRun *run, const WebCoreTextStyle *style, int from, int to, int x, int y, const XftColor* color, bool shouldMeasure);
    // prevent copying 
    XftTextRenderer(const XftTextRenderer&);
    XftTextRenderer &operator=(const XftTextRenderer&);

    XftTextRendererFactory* owner;
	
    GdkWindow *gdkwindow;
    GdkDrawable *gdkdrawable;
    GdkGC *gdkgc;
    gint gdkxoff;
    gint gdkyoff;

    XftDraw *xftdraw;
    XftNSFont *font;

    Display *xdisplay;
    Colormap xcmap;
    Visual *xvisual;
    
    Region xclip;

    CGContextRef context;
    
    friend class XftTextRendererFactory;    
};

#endif
