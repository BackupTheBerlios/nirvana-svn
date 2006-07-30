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
#ifndef ImageRenderer_h
#define ImageRenderer_h

#include <glib.h>
#include <gdk/gdk.h>

#include "NRCore/WebCoreImageRenderer.h"

#include "GObjectMisc.h"

class ImageRenderer : public WebCoreImageRenderer
{
public:
    ImageRenderer();
    ImageRenderer(const ImageRenderer&);
    ImageRenderer(GdkPixbufAnimation* anim);
    ImageRenderer(GdkPixbufLoader* ldr);
    ~ImageRenderer();
	
    bool incrementalLoadWithBytes(const void *bytes, unsigned length, bool isComplete);
    void size(GdkRectangle* outRect);
    void resize(GdkRectangle* size);
    void drawImageInRect(GdkRectangle* inRect, GdkRectangle* fromRect, NSCompositingOperation, CGContextRef context);
    void stopAnimation();
    void tileInRect(GdkRectangle *r, int x, int y, CGContextRef context);
    bool isNull();

    void increaseUseCount();
    void decreaseUseCount();
    void flushRasterCache();

    void ref() { refCnt++; }
    void unref() { if (!(--refCnt)) delete this;  }
    ImageRenderer* copy();
    
    GdkPixbuf* handle() const;
    
    /** glib signals emited by GdkPixbufLoader  */
    void areaPrepared();
    void areaUpdated(int x,int y, int w, int h);
    void loaderClosed();
    void sizePrepared(int width, int height);
    
    /** Timeout to change animation frame */
    void animateTimeout();   

    void animate();
private:
    void cache();
    void tileCache();
    void invalidate();
    void connectHandlers();
    void installAnimateTimeout();
    void removeAnimateTimeout();
    void realSize(GdkRectangle*);

    static const int updateTimeoutMS;
    int refCnt;
    GPtr<GdkPixbufLoader> loader;
    GdkPixbufAnimation* anim;
    GdkPixbufAnimationIter* iter;
    GdkPixbuf* pixbuf;
    /** cached data */
    GdkPixmap* pixmap;
    GdkBitmap* alpha;

    glong animate_id;
    bool cached;
    bool tileCached;
    GdkRectangle wantedSize;
    int offset;
    GTimeVal timeout_moment;
};

#endif
