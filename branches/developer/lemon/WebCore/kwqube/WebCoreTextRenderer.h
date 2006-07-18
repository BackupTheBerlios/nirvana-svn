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

#ifndef WebCoreTextRenderer_h
#define WebCoreTextRenderer_h

#include "glib.h"
//#include <gdk/gdk.h>

typedef char gchar;

#include "KWQUBECGContext.h"  // CGContext
#include "WebCoreUnicode.h" // UniChar
#include "WebCoreFoundation.h"

class NSFont;

#ifndef ROUND_TO_INT
#define ROUND_TO_INT(x) (unsigned int)((x)+.5)
#endif

struct WebCoreTextStyle
{
    //GdkColor textColor;
    //GdkColor backgroundColor;
    rgb_color textColor;
    rgb_color backgroundColor;
    
    int letterSpacing;
    int wordSpacing;
    int padding;
    unsigned smallCaps:1;
    unsigned rtl:1;
    unsigned visuallyOrdered:1;    
    unsigned applyRounding:1;
    unsigned attemptFontSubstitution:1;
    const gchar* const* families;
    
    WebCoreTextStyle()
	: letterSpacing(0)
	,wordSpacing(0)
	,padding(0)
	,smallCaps(false)
	,rtl(false)
	,applyRounding(true)
	,attemptFontSubstitution(true)
	,families(0)
    { }
};

struct WebCoreTextRun
{
    const UniChar *characters;
    unsigned int length;
    int from;
    int to;
    WebCoreTextRun(const UniChar* chars, unsigned int len, int afrom, int ato);
    ~WebCoreTextRun();
	
};

struct WebCoreTextGeometry
{
    int x,y;   
    float selectionY;
    float selectionHeight;
    float selectionMinX;
    float selectionMaxX;
    bool useFontMetricsForSelectionYAndHeight : 1;

    WebCoreTextGeometry(int ax, int ay, float aselectionY, float aselectionHeight, 
		     float aselectionMinX, float aselectionMaxX, bool useFontMetrics)
	: x(ax) 
	  , y(ay)
	  , selectionY(aselectionY)
	  , selectionHeight(aselectionHeight)
	  , selectionMinX(aselectionMinX)
	  , selectionMaxX(aselectionMaxX)
	  , useFontMetricsForSelectionYAndHeight(useFontMetrics)
    {}

    ~WebCoreTextGeometry()
    {}
};

class WebCoreTextRenderer : public WebCoreNSObject
{
public:
    WebCoreTextRenderer() : ascent(0), descent(0), lineSpacing(0), xHeight(0)  {}
    virtual ~WebCoreTextRenderer() {};    

    // vertical metrics
    int ascent;
    int descent;
    int lineSpacing;
    float xHeight;

    // horizontal metrics
    virtual float floatWidthForRun(const WebCoreTextRun *run, const WebCoreTextStyle *style, float *widths) = 0;    
    
    // drawing
    virtual void drawRun(const WebCoreTextRun *run, const WebCoreTextStyle *style, int x, int y) =0;
    virtual void drawHighlightForRun(const WebCoreTextRun *run, const WebCoreTextStyle *style, const WebCoreTextGeometry* geom) =0;
    virtual void drawLineForCharacters(int x, int y, float yOffset, int width, rgb_color *color)=0;
    
    // selection point check
    virtual int pointToOffset(const WebCoreTextRun *run, const WebCoreTextStyle *style, int x, bool reversed, bool includePartialGlyphs) =0;

    virtual void setContext(CGContextRef context) = 0;

private:
    WebCoreTextRenderer(const WebCoreTextRenderer&);
    WebCoreTextRenderer& operator=(const WebCoreTextRenderer&);
};

#endif
