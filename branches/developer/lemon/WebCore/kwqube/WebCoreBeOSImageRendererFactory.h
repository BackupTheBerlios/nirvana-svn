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
#ifndef ImageRendererFactory_h
#define ImageRendererFactory_h
#include "NRCore/WebCoreImageRendererFactory.h"

class ImageRendererFactory : public WebCoreImageRendererFactory
{    

  // GHashTable mimes;
  // GList* exts;

public:
    ImageRendererFactory();
    ~ImageRendererFactory();

    static void useAsSharedFactory();
    
    WebCoreImageRenderer* imageRenderer();
    WebCoreImageRenderer* imageRendererWithMIMEType(const gchar* MIMEType);
    WebCoreImageRenderer* imageRendererWithBytes(const gchar* bytes, unsigned length);
    WebCoreImageRenderer* imageRendererWithBytes(const gchar* bytes, unsigned length, const gchar* MIMEType);
    WebCoreImageRenderer* imageRendererWithSize(GdkRectangle* size);
    WebCoreImageRenderer* imageRendererWithName(const gchar* name);
    GList* supportedMIMETypes(); // ownership not transferred
    
    bool supports(const gchar* mime);
    void ref();
    void unref();
    WebCoreImageRenderer* refOrCopyIfNeeded();
    
    int CGCompositeOperationInContext(CGContextRef context);
    void setCGCompositeOperation(CGContextRef context, int op);

    void setCGCompositeOperationFromString(CGContextRef context, const gchar* op);
   
private:    
    WebCoreImageRenderer* imageRendererWithBytesAndLoader(const gchar* bytes, unsigned length, GdkPixbufLoader* ldr);
    GList* mimeTypes;
};


#endif
