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
#ifndef WebCoreImageRenderer_h
#define WebCoreImageRenderer_h

#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "WebCoreImageRendererFactory.h"

class WebCoreImageRenderer
{
public:
    virtual ~WebCoreImageRenderer(){}
    
    virtual bool incrementalLoadWithBytes(const void *bytes, unsigned length, bool isComplete)=0;
    virtual void size(GdkRectangle *outRect)=0;
    virtual void resize(GdkRectangle *size)=0;

    virtual void drawImageInRect(GdkRectangle *inRect, GdkRectangle *fromRect, NSCompositingOperation compositeOperator, CGContextRef context)=0;
    virtual void stopAnimation()=0;
    virtual void tileInRect(GdkRectangle *inRect, int fromX, int fromY, CGContextRef context) =0;
    virtual bool isNull()=0;

    virtual void ref()=0;
    virtual void unref()=0;
    virtual WebCoreImageRenderer* copy()=0;

    virtual void increaseUseCount()=0;
    virtual void decreaseUseCount()=0;
    virtual void flushRasterCache()=0;
    virtual GdkPixbuf* handle() const=0;
};


#endif
