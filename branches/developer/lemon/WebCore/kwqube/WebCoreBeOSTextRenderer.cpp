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
#include <glib.h>
#include <gdk/gdkx.h>
#include <gdk/gdk.h>
#include <fontconfig/fontconfig.h>
#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>

#include "XftTextRenderer.h"
#include "XftTextRendererFactory.h"
#include "GdkXftContext.h"

#define USE_XFT_DRAWRECT

static
UniChar*
dupCharsAndRemoveLF(const UniChar* chars, unsigned int length)
{
    assert(length>0);
    assert(chars);

    UniChar uc_space(' ');
    UniChar* dup = new UniChar[length];
    const gunichar2* uc_chars = reinterpret_cast<const gunichar2*>(chars);

    for(unsigned int i = 0; i < length; i++) {
	if (g_unichar_isspace(uc_chars[i]))
	    dup[i] = uc_space;
	else
	    dup[i] = chars[i];
    }

    return dup;
}

// notice: this code is assuming that there is no consecutive spaces

// [a,b[ range of chars that make a word, ie "abcd dabcd" would produce (0,4) (5,9) 
typedef struct _WordRange { unsigned short start; unsigned short stop; } WordRange;

static
int scanRunForWords(const WebCoreTextRun* run, int from, WordRange* words, int maxWords, int* nWords, int* totWords)
{
    assert(run);
    assert(run->from <= run->to);
    assert(nWords);

    *nWords = 0;
    int tot = 0;

    if (run->from == run->to)
	return from;

    const gunichar2* ucs = reinterpret_cast<const gunichar2*>(run->characters);

    // scan until end of run or we run out of word slots
    int start, stop;
    start = from;
    while (start < run->to && tot < maxWords) {

	// move the stop to point at the last char before space or at the end of the buffer
	// stop == start if there's consecutive spaces ('words of lenght == 0')
	for (stop = start; stop < run->to && !g_unichar_isspace(*(ucs + stop)); stop++) /* empty */; 

	words[tot].start = start;
	words[tot].stop = stop;
	++tot;

	start = stop + 1; // skip the space 
	++(*nWords);
    }
    
    if (totWords) {
	*totWords = tot;
	if (start < run->to) { 
	    // ran out of words before finished scanning
	    // scan the total amount of words

	    for(int pos = start; pos < run->to; pos++) {
		if (g_unichar_isspace(*(ucs + pos))) 
		    ++(*totWords);
	    }
	}
    }

    return start;
}

inline static
void getXRenderColorFromGdkColor(const GdkColor* gdkc, XRenderColor* xrc)
{
    xrc->red = gdkc->red;
    xrc->blue = gdkc->blue;
    xrc->green = gdkc->green;
    xrc->alpha = 0xffff;
}

XftTextRenderer::XftTextRenderer(XftTextRendererFactory* aowner, XftNSFont *afont)
    : owner(aowner)
    , gdkxoff(0)
    , gdkyoff(0)
    , xftdraw(0)
    , font(afont)
    , xclip(0)
{  
    font->retain();

    ascent = font->ascent;
    descent = font->descent;
    lineSpacing = font->lineSpacing;
    xHeight = font->xHeight;

    /** fixme: figure out how to propagate this */
    xdisplay = GDK_DISPLAY();


}

XftTextRenderer::~XftTextRenderer()
{
    if (xclip)
	XDestroyRegion(xclip);

    font->release();

    if (owner) 
	owner->rendererDeleted(this);
}

void XftTextRenderer::setContext(CGContextRef acontext)
{
    GdkXftContext* context = static_cast<GdkXftContext*>(acontext);
    assert(context);
    xftdraw = context->xftDraw();
    xvisual = context->XVisual();
    xcmap = context->XColormap();
    gdkxoff = gdkyoff = 0;
    context->xftTranslate(&gdkxoff, &gdkyoff);
}

float XftTextRenderer::measureRange(const WebCoreTextRun *run, const WebCoreTextStyle *style, int from, int to, float* widths)
{
    XGlyphInfo extents;

    if (style->letterSpacing == 0 && !widths) {
	XftTextExtents16( xdisplay,
			  font->xftFont,
			  reinterpret_cast<const XftChar16*>(run->characters + from),
			  to - from,
			  &extents);
	return extents.xOff;
    }

    float w, totalLen = 0.0f;
    int i = 0;
    while (from < to) {
	XftTextExtents16(xdisplay,
			 font->xftFont,
			 reinterpret_cast<const XftChar16*>(run->characters + from),
			 1,
			 &extents);

	w = extents.xOff + style->letterSpacing;
	totalLen += w;
	if (widths)
	    widths[i++] = w;
	++from;
    }    
    return totalLen;
}

float XftTextRenderer::drawRange(const WebCoreTextRun *run, const WebCoreTextStyle *style, int from, int to, int x, int y, const XftColor* color, bool shouldMeasure)
{
    int width = 0;
    XGlyphInfo extents;
    if (style->letterSpacing) {
	while (from<to) {
	    XftTextExtents16(xdisplay, 
			     font->xftFont,
			     reinterpret_cast<const XftChar16*>(run->characters + from),     
			     1,
			     &extents);

	    XftDrawString16(xftdraw,
			    color,
			    font->xftFont,
			    x,
			    y,
			    reinterpret_cast<const XftChar16*>(run->characters + from),
			    1);
	    x += extents.xOff + style->letterSpacing;
	    ++from;
	    width += extents.xOff + style->letterSpacing;
	}
	return width;
    } else {
	if (shouldMeasure) {
	    XftTextExtents16(xdisplay, 
			     font->xftFont,
			     reinterpret_cast<const XftChar16*>(run->characters + from),
			     to - from,			     
			     &extents);
	    width += extents.xOff;
	}

	XftDrawString16(xftdraw,
			color,
			font->xftFont,
			x,
			y,
			reinterpret_cast<const XftChar16*>(run->characters + from),
			to - from);
    }

    return width;
}

float XftTextRenderer::floatWidthForRun(const WebCoreTextRun *run,
					const WebCoreTextStyle *style,
					float *widths)
{


    assert(run);
    assert(style);
    assert(style->families);
    
    if (run->length == 0)
	return 0.0f;

    const int wordBufSz = 10;	// size of scan buffer
    WordRange words[wordBufSz]; // scan buffer
    int nWords;			// number of words in scan buffer
    int totWords = 0;		// total word count



    // needs always totWords because wordSpacing is applied only to spaces in the middle, not
    // for the last word. Is this correct practice?
    scanRunForWords(run, run->from, words, wordBufSz, &nWords, &totWords);

    // run->from == run->to == 0
    if (totWords == 0)
	return 0.0f;  
    
    const float wordPadding = (totWords>1) ? style->padding / (totWords-1) : 0.0f; 	// padding between words

    float totalWidth = 0.0f;
//    float wordWidth = 0.0f;
    int wordsMeasured = 0;
    int wordIndex;
    int whiteScanFrom = run->from;

    totalWidth += (words[0].start - run->from) * (font->spaceWidth+style->wordSpacing + style->letterSpacing);
    while (1) {
	// iterate over scan buffer
	for (wordIndex = 0; wordIndex < nWords; wordIndex++) {
	    totalWidth += (words[wordIndex].start - whiteScanFrom) * (font->spaceWidth+style->wordSpacing+style->letterSpacing);
		
	    totalWidth += measureRange(run, style, words[wordIndex].start, words[wordIndex].stop, widths);

	    if (wordsMeasured > 0) {  
		// has space-char after the word
		if (widths) {
		    widths[words[wordIndex].stop] = wordPadding + font->spaceWidth + style->wordSpacing + style->letterSpacing;
		}
	    }
	    whiteScanFrom = words[wordIndex].stop;
	    ++wordsMeasured;
	}

	if (nWords > 0 && words[nWords-1].stop+1 < run->to) {
            // update scan buffer (will happen if number of spaces > wordBufSz)
	    scanRunForWords(run, words[nWords-1].stop+1, words, wordBufSz, &nWords, NULL); 	
	} else 
	    break;
    }
    totalWidth += (run->to - words[wordIndex-1].stop) * (font->spaceWidth+style->wordSpacing+style->letterSpacing);

    return totalWidth + style->padding;
}

// static
// int indexToSeparator(const XftChar16* str, unsigned int length)
// {
//     const gunichar2* uc_chars = reinterpret_cast<const gunichar2*>(str);

//     unsigned int i;
//     for( i = 0; i < length; i++) {
// 	if (g_unichar_isspace(*uc_chars))
// 	    break;
// 	uc_chars++;
//     }
//     return i;
// }

void XftTextRenderer::drawRun(const WebCoreTextRun *run, const WebCoreTextStyle *style, int x, int y)
{
    assert(run);
    assert(style);
    assert(style->families);
    
    if (run->length == 0) 
	return;

    XRenderColor xrc;
    XftColor c;
  

    getXRenderColorFromGdkColor(&style->textColor, &xrc);

    XftColorAllocValue(xdisplay,
		       xvisual,
		       xcmap,
		       &xrc,
		       &c);

    x -=  gdkxoff;
    y -=  gdkyoff;

    const int wordBufSz = 10;	// size of scan buffer
    WordRange words[wordBufSz]; // scan buffer
    int nWords;			// number of words in scan buffer
    int totWords = 0;		// total word count



    // needs always totWords because wordSpacing is applied only to spaces in the middle, not
    // for the last word. Is this correct practice?
    scanRunForWords(run, run->from, words, wordBufSz, &nWords, &totWords);

    if (totWords == 0) 
	return;

    float wordPadding =  style->padding / totWords;

    float wordWidth = 0.0f;
    int wordsDrawn = 0;
    int wordIndex;

    while (1) {

	// iterate over scan buffer
	for (wordIndex = 0; wordIndex < nWords; wordIndex++) {

	    wordWidth = drawRange(run, style, words[wordIndex].start, words[wordIndex].stop, x, y, &c, true);

	    // not last word
	    wordWidth += wordPadding;
	    wordWidth += font->spaceWidth + style->wordSpacing;

	    x += (int)wordWidth;

	    ++wordsDrawn;
	} 

	if (nWords > 0 && words[nWords-1].stop+1 < run->to) {
            // update scan buffer (will happen if number of spaces > wordBufSz)
	    scanRunForWords(run, words[nWords-1].stop+1, words, wordBufSz, &nWords, NULL); 	
	} else 
	    break;
    }


    XftColorFree(xdisplay,
		 xvisual,
		 xcmap,
		 &c);

    
    if (style->rtl){
	g_warning("RTL painting not implemented"); 	// FIXME:: Handle right-to-left text
    }
}

void XftTextRenderer::drawHighlightForRun(const WebCoreTextRun *run, const WebCoreTextStyle *style, const WebCoreTextGeometry *geom)
{
    assert(run);
    assert(style);
    assert(style->families);

    if (run->length == 0) 
	return ;

    int x=0, y=0, w=0, h=0;
    XGlyphInfo extents;

    UniChar* uchars = dupCharsAndRemoveLF(run->characters, run->length);

    x = (int)geom->selectionMinX;
    y = (int)geom->y;

    if (run->from > 0) {
	XftTextExtents16( xdisplay, 
			  font->xftFont,
			  reinterpret_cast<const XftChar16*>(uchars),
			  run->from,
			  &extents);
	
	x += extents.xOff /* + style->padding*/; // xOff == offset where the next char starts
    }

    XftTextExtents16( xdisplay, 
		      font->xftFont,
		      reinterpret_cast<const XftChar16*>(uchars + run->from),
		      run->to - run->from,
		      &extents);

    w = (int)extents.width;

    h = (int)geom->selectionHeight;

    drawRect(x,y,w,h, &style->textColor);


    delete[] uchars;
}

void XftTextRenderer::drawLineForCharacters(int x, int y, float yOffset, int width, GdkColor* color)
{
}

// selection point check
int XftTextRenderer::pointToOffset(const WebCoreTextRun *run, const WebCoreTextStyle *style, int x, bool reversed, bool includePartialGlyphs)
{
    assert(run);
    assert(style);
    assert(style->families);

    if (run->length == 0) 
	return 0;

    XGlyphInfo extents;

    UniChar* uchars = dupCharsAndRemoveLF(run->characters, run->length);

    // binary search for offset in string for pixel position x
    int start, end;
    start = run->from;
    end = run->to;

    int xpos;
    int len_mid;    
    while (start < end) {
	len_mid = ((end - start)+1) / 2;
	XftTextExtents16(xdisplay,
			 font->xftFont,
			 reinterpret_cast<const XftChar16*>(uchars + start),
			 len_mid,
			 &extents);
	
	xpos = extents.xOff;
	if (xpos < x) {
	    start += len_mid;
	    x -= xpos;
	} else if (xpos > x) {
	    end -= len_mid;
	}else {
	    start += len_mid;
	    break;
	}
    }

    delete [] uchars;

    return start - run->from;
}


void XftTextRenderer::drawRect(int x, int y, int w, int h, const GdkColor* color)
{
    
#if defined(USE_XFT_DRAWRECT)
    g_warning("(%d,%d,%d,%d)", x,y,w,h);
    XftColor xft_c;
    XRenderColor xrender_c;
    
    getXRenderColorFromGdkColor(color, &xrender_c);    

    XftColorAllocValue(xdisplay,
		       xvisual,
		       xcmap,
		       &xrender_c,
		       &xft_c);

    x -= gdkxoff;
    y -= gdkyoff;

    XftDrawRect(xftdraw, &xft_c, x, y, w, h);
    
    XftColorFree(xdisplay,
		 xvisual,
		 xcmap,
		 &xft_c);
#else
    gdk_gc_set_rgb_fg_color(gdkgc, const_cast<GdkColor*>(color));

    x -= gdkxoff;
    y -= gdkyoff;

    gdk_gc_set_fill (gdkgc, GDK_SOLID);
    gdk_draw_rectangle(gdkdrawable, gdkgc, TRUE, x, y, w, h);

#endif
}



