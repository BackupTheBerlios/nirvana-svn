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
#ifndef WebCoreImageRendererFactory_h
#define WebCoreImageRendererFactory_h

//#include <gdk/gdk.h>

typedef char gchar;

#include <InterfaceKit.h>
#include <SupportKit.h>
#include "KWQUBECGContext.h"

class WebCoreImageRenderer;

class WebCoreImageRendererFactory 
{

public:
    
    static WebCoreImageRendererFactory* sharedFactory() { return m_sharedFactory; };

    virtual WebCoreImageRenderer* imageRenderer()=0;
    virtual WebCoreImageRenderer* imageRendererWithMIMEType(const gchar* MIMEType)=0;
    virtual WebCoreImageRenderer* imageRendererWithBytes(const gchar* bytes, unsigned length)=0;
    virtual WebCoreImageRenderer* imageRendererWithBytes(const gchar* bytes, unsigned length, const gchar* MIMEType)=0;
    //virtual WebCoreImageRenderer* imageRendererWithSize(GdkRectangle* size)=0;
    virtual WebCoreImageRenderer* imageRendererWithSize(BRect* size)=0;
    virtual WebCoreImageRenderer* imageRendererWithName(const gchar* name)=0;

    //virtual GList* supportedMIMETypes()=0;
    virtual BList* supportedMIMETypes()=0;
    virtual bool supports(const gchar* mime)=0;

    virtual int CGCompositeOperationInContext(CGContextRef context)=0;
    virtual void setCGCompositeOperation(CGContextRef context, int op)=0;
    virtual void setCGCompositeOperationFromString(CGContextRef context, const gchar* op)=0;

protected:    
    WebCoreImageRendererFactory() {};
    virtual ~WebCoreImageRendererFactory() {};

    static WebCoreImageRendererFactory* m_sharedFactory;

private:
    WebCoreImageRendererFactory(const WebCoreImageRendererFactory&);
    WebCoreImageRendererFactory& operator=(const WebCoreImageRendererFactory&);

};

#endif
