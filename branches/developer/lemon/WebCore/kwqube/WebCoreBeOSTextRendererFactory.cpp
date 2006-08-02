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
#include <assert.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h> //pango stuff

#include "XftTextRendererFactory.h"

void XftTextRendererFactory::useAsSharedFactory()
{
    static XftTextRendererFactory singleton;
    m_sharedFactory = &singleton;
}


XftNSFont::XftNSFont(XftTextRendererFactory* acreator, XftFont* afont, gpointer ahash)
    :xftFont(afont)
     ,hash(ahash)
     ,creator(acreator)
{
    assert(xftFont);

    xdisplay = GDK_DISPLAY(); //FIXME: How to propagate this
    ascent = xftFont->ascent;
    descent = xftFont->descent;
    lineSpacing = xftFont->height;

    // measure x height
    XftChar8 charx = 'x';
    XGlyphInfo extents;    
    XftTextExtents8(xdisplay,
		    xftFont,
		    &charx,
		    1,
		    &extents);
    
    xHeight = extents.height;

    // measure space width. Needed because we don't draw '\n' -chars, just skip them
    charx = ' ';
    XftTextExtents8(xdisplay,
		    xftFont,
		    &charx,
		    1,
		    &extents);
    
    spaceWidth = extents.xOff; // .width == width of the glyph, .xOff == where the next glyph starts

    int spacing = 0;
    if (FcPatternGetInteger(xftFont->pattern, FC_SPACING, 0, &spacing) == FcResultMatch) {
	fixedPitch = (spacing == FC_MONO);
    } else {
	fixedPitch = false;
    }
}

XftNSFont::~XftNSFont()
{
    if (xftFont)
	XftFontClose(xdisplay, xftFont);     

    if (creator) creator->fontDeleted(this);
}

gpointer XftNSFont::createHash(const gchar * const * families, NSFontTraitMask traits, float size)
{
    int hash = 0;

    for (int i = 0;families[i]; i++)
	hash ^=  g_direct_hash(families[i]);	

    hash ^= (((int)traits) << 16);
    hash ^= ((int)size*100);
    return (gpointer) hash;
}

/**
 * Loads Xft font
 * from fontconfig(3)
 * Fontconfig  performs matching by measuring the distance from a provided
 * pattern to all of the available  fonts  in  the  system.   The  closest
 * matching  font  is  selected.   This ensures that a font will always be
 * returned, but doesn't ensure that it is  anything  like  the  requested
 * pattern.
 *
 * cannot fail (sure..)
 * @return font (must be freed)
 */

static 
XftFont* try_load_font(const char* const * family, double size, int weight, int slant)
{
    assert(family);

    FcPattern *pattern,*matched;
    FcResult result;

    pattern = FcPatternBuild(0,
			     FC_WEIGHT, FcTypeInteger, weight,
			     FC_SLANT,  FcTypeInteger, slant,
			     FC_PIXEL_SIZE, FcTypeDouble, size,
			     FC_DPI, FcTypeDouble, 96.0,
			     FC_SCALE, FcTypeDouble, 1.0,
			     NULL);
    while (*family) {
	FcPatternAddString(pattern, FC_FAMILY, reinterpret_cast<const FcChar8*>(*family));
	family++;
    }

    matched = XftFontMatch(GDK_DISPLAY(),0,pattern, &result);

    if (!matched) {
	FcPatternDestroy(pattern);
	//g_warning("%s Font Matching Failed.", __PRETTY_FUNCTION__);
	return 0;
    }
    
    XftFont *f = XftFontOpenPattern(GDK_DISPLAY(), matched);

    // FontConfig should destroy the pattern
    return f;
}

extern "C" {
static

void nsfonts_key_destroy(gpointer data)
{

}

void nsfonts_value_destroy(gpointer data)
{
}

void nsfonts_value_detach(gpointer key, gpointer value, gpointer user_data)
{

    XftNSFont *f = static_cast<XftNSFont*>(value);
    assert(f);
    f->detach();
}
}

XftTextRendererFactory::XftTextRendererFactory()
{
    nsfonts = g_hash_table_new_full(g_direct_hash, 
				    g_direct_equal, 
				    nsfonts_key_destroy, 
				    nsfonts_value_destroy);

}

XftTextRendererFactory::~XftTextRendererFactory()
{
    g_hash_table_foreach(nsfonts, nsfonts_value_detach, this);
    g_hash_table_destroy(nsfonts);
}

// Assume families[0..] are atomic string ptrs
// assumes hashing is perfect and no clashes occur. FIXME!!!!

NSFont* XftTextRendererFactory::fontWithFamilies(const gchar * const * families, NSFontTraitMask traits, float size)
{
    assert(families);

    gpointer hash = XftNSFont::createHash(families, traits, size);
    XftNSFont* font = static_cast<XftNSFont*>(g_hash_table_lookup(nsfonts, hash));

    if (!font) {
	// font cache miss
	int w,s;        
	w = FC_WEIGHT_MEDIUM;
	s = FC_SLANT_ROMAN;
	
	if (traits & NSBoldFontMask)
	    w = FC_WEIGHT_BOLD;
	
	if (traits & NSItalicFontMask) 
	    s = FC_SLANT_ITALIC;
	

	XftFont* xftfont = try_load_font(families, size, w, s);
	assert(xftfont);
	font = new XftNSFont(this, xftfont, hash);

        //FIXME: store families too, in case of hashing clashes
	g_hash_table_insert(nsfonts, hash, font);
    }

    return font;
}

XftTextRenderer* XftTextRendererFactory::rendererWithFont(NSFont *nsfont, bool isPrinterFont)
{
    assert(nsfont);

    XftNSFont* font = static_cast<XftNSFont*>(nsfont); 

    return new XftTextRenderer(this, font);
}

void XftTextRendererFactory::rendererDeleted(XftTextRenderer* renderer)
{

}

void XftTextRendererFactory::fontDeleted(XftNSFont* font)
{
    bool ret =  g_hash_table_remove(nsfonts, font->hash);
    assert(ret);
}

bool XftTextRendererFactory::isFontFixedPitch(NSFont* nsfont)
{
    XftNSFont* font = static_cast<XftNSFont*>(nsfont);     
    return font->fixedPitch;
}
