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
#ifndef XftTextRendererFactory_h
#define XftTextRendererFactory_h

#include <glib.h>
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

#include "NRCore/WebCoreTextRendererFactory.h"
#include "XftTextRenderer.h" // returning

class XftTextRendererFactory;
class XftTextRenderer;


class XftNSFont : public NSFont
{
public:
    XftNSFont(XftTextRendererFactory* creator, XftFont* font, gpointer ahash);
    ~XftNSFont();
    static  gpointer createHash(const gchar * const * families, NSFontTraitMask traits, float size);

    void detach() { creator = 0; }
    XftFont *xftFont;
    int ascent;
    int descent;
    int lineSpacing;
    float xHeight;
    int spaceWidth;
    bool fixedPitch;
    gpointer hash;
    Display* xdisplay;
    XftTextRendererFactory* creator;
    
};

class XftTextRendererFactory : public WebCoreTextRendererFactory
{
    
public:

    NSFont* fontWithFamilies(const gchar * const * families, NSFontTraitMask traits, float size);

    XftTextRenderer* rendererWithFont(NSFont* font, bool isPrinterFont);     // co-variant return type

    bool isFontFixedPitch(NSFont* f);
    
    static void useAsSharedFactory();

    void rendererDeleted(XftTextRenderer* renderer);
    void fontDeleted(XftNSFont* font);
protected:
    XftTextRendererFactory();
    ~XftTextRendererFactory();

private:
    XftTextRendererFactory(const XftTextRendererFactory&);
    XftTextRendererFactory& operator=(const XftTextRendererFactory&);
    GHashTable* nsfonts;
};

#endif
