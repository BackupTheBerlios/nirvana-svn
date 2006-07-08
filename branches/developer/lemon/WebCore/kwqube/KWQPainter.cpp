/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright (C) 2001, 2002 Apple Computer, Inc.  All rights reserved.
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
#include <gdk/gdk.h>

#include "KWQPainter.h"

#include "KWQAssertions.h"
#include "KWQFontMetrics.h"
//#include "KWQKHTMLPart.h"
#include "KWQPaintDevice.h"
#include "KWQPixmap.h"
#include "KWQPointArray.h"
#include "KWQPrinter.h"
#include "KWQWidget.h"
#include "KWQPtrStack.h"

#include "WebCoreTextRendererFactory.h"
#include "WebCoreTextRenderer.h"
#include "WebCoreImageRenderer.h"
#include "KWIQCGContext.h"
#include "KWQFoundationExtras.h"

#include "KWQLogging.h"

/*
 *FIXME: will crash for example in resizing frames, because drawing context is not set.
 * Apple has global context, and they construct painter with statement like "QPainter painter;" 
 * for example  see render_frames.cpp:440
 *
 */

static inline
void initGdkRectangle(GdkRectangle* rect, int x, int y, int w, int h)
{
    rect->x=x;
    rect->y=y;
    rect->width=w;
    rect->height=h;

}

struct QPState {
    QPState() :  paintingDisabled(false),clip(0) { } 
    QFont font;
    QPen pen;
    QBrush brush;
    bool paintingDisabled;
    GdkRegion* clip;
};

struct QPainterPrivate {
    QPainterPrivate() : textRenderer(0), focusRingPath(0), focusRingWidth(0), focusRingOffset(0),
                        hasFocusRingColor(false), context(0) { }

    ~QPainterPrivate() { KWQRelease(textRenderer); delete focusRingPath; }
    QPState state;
    QPtrStack<QPState> stack;
    WebCoreTextRenderer* textRenderer;
    QFont textRendererFont;
    QRect *focusRingPath;
    int focusRingWidth;
    int focusRingOffset;
    bool hasFocusRingColor;
    QColor focusRingColor;

    CGContextRef context;
};



QPainter::QPainter() : data(new QPainterPrivate), _isForPrinting(false), _usesInactiveTextBackgroundColor(false), _drawsFocusRing(true)
{
}

QPainter::QPainter(bool forPrinting) : data(new QPainterPrivate), _isForPrinting(forPrinting), _usesInactiveTextBackgroundColor(false), _drawsFocusRing(true)
{
}

QPainter::~QPainter()
{
    delete data;
}

QPaintDevice *QPainter::device() const
{
    static QPrinter printer;
    static QPaintDevice screen;
    return _isForPrinting ? &printer : &screen;
}

const QFont &QPainter::font() const
{
    return data->state.font;
}

void QPainter::setFont(const QFont &aFont)
{
    data->state.font = aFont;
}

QFontMetrics QPainter::fontMetrics() const
{
    return data->state.font;
}

const QPen &QPainter::pen() const
{
    return data->state.pen;
}

void QPainter::setPen(const QPen &pen)
{
    data->state.pen = pen;
}

void QPainter::setPen(PenStyle style)
{
    data->state.pen.setStyle(style);
    data->state.pen.setColor(Qt::black);
    data->state.pen.setWidth(0);
}

void QPainter::setPen(QRgb rgb)
{
    data->state.pen.setStyle(SolidLine);
    data->state.pen.setColor(rgb);
    data->state.pen.setWidth(0);
}

void QPainter::setBrush(const QBrush &brush)
{
    data->state.brush = brush;
}

void QPainter::setBrush(BrushStyle style)
{
    data->state.brush.setStyle(style);
    data->state.brush.setColor(Qt::black);
}

void QPainter::setBrush(QRgb rgb)
{
    data->state.brush.setStyle(SolidPattern);
    data->state.brush.setColor(rgb);
}

const QBrush &QPainter::brush() const
{
    return data->state.brush;
}

QRect QPainter::xForm(const QRect &aRect) const
{
    // No difference between device and model coords, so the identity transform is ok.
    return aRect;
}

void QPainter::save()
{
    data->stack.push(new QPState(data->state));

    data->context->saveGraphicsState(); 
}

void QPainter::restore()
{
    if (data->stack.isEmpty()) {
        ERROR("ERROR void QPainter::restore() stack is empty");
        return;
    }

    QPState *ps = data->stack.pop();
    data->state = *ps;
    delete ps;

    data->context->restoreGraphicsState();

}

// Draws a filled rectangle with a stroked border.
void QPainter::drawRect(int x, int y, int w, int h)
{
    if (data->state.paintingDisabled)
        return;

    if (data->state.brush.style() != NoBrush) {
        _setColorFromBrush();
        gdk_draw_rectangle(data->context->drawable, data->context->gc, TRUE, x, y, w, h);
    }
    if (data->state.pen.style() != NoPen) {
        _setColorFromPen();
        gdk_draw_rectangle(data->context->drawable, data->context->gc, FALSE, x, y, w-1, h-1);
    }
}

void QPainter::_setColorFromBrush()
{
    ASSERT(data->state.brush.color().isValid());
    
    GdkColor color;
    QColor qc = data->state.brush.color();
    color.red = qc.red() *255;
    color.green = qc.green() * 255;
    color.blue = qc.blue() * 255;

    gdk_gc_set_rgb_fg_color(data->context->gc, &color);

    gdk_gc_set_fill (data->context->gc, GDK_SOLID);
}

void QPainter::_setColorFromPen()
{
    ASSERT(data->state.pen.color().isValid());
    
    GdkColor color;
    QColor qc = data->state.pen.color();
    color.red = qc.red() *255;
    color.green = qc.green() * 255;
    color.blue = qc.blue() * 255;
    //color.pixel = qc.rgb() & 0x00FFFFFF; 

    gdk_gc_set_rgb_fg_color(data->context->gc, &color);
}

// This is only used to draw borders.
// This is also used to draw lines under text
void QPainter::drawLine(int x1, int y1, int x2, int y2)
{
    if (data->state.paintingDisabled)
        return;

    PenStyle penStyle = data->state.pen.style();
    if (penStyle == NoPen)
        return;

    _setColorFromPen();

    gdk_gc_set_line_attributes (data->context->gc,
                                data->state.pen.width(),
                                GDK_LINE_SOLID,
                                GDK_CAP_BUTT,
                                GDK_JOIN_MITER);
    
    gdk_draw_line(data->context->drawable, data->context->gc, x1, y1, x2, y2);
}


// This method is only used to draw the little circles used in lists.
void QPainter::drawEllipse(int x, int y, int w, int h)
{
    if (data->state.paintingDisabled)
        return;

    if (data->state.brush.style() != NoBrush) {
        _setColorFromBrush();
        gdk_draw_arc(data->context->drawable, data->context->gc, TRUE, x, y, w, h, 0, 360*64);
    }
    if (data->state.pen.style() != NoPen) {
        _setColorFromPen();
        gdk_draw_arc(data->context->drawable, data->context->gc, FALSE, x, y, w, h, 0, 360*64);
    }
}


void QPainter::drawArc (int x, int y, int w, int h, int a, int alen)
{
    // Only supports arc on circles.  That's all khtml needs.
    
    if (data->state.paintingDisabled)
        return;

    if (data->state.pen.style() != NoPen) { 
        _setColorFromPen();
        gdk_draw_arc(data->context->drawable, data->context->gc, FALSE, x, y, w, h, 
                     (gint)((a*16)/64), (gint) ((alen*16)/64));
    }
}

void QPainter::drawLineSegments(const QPointArray &points, int index, int nlines)
{
    if (data->state.paintingDisabled)
        return;

    _drawPoints (points, 0, index, nlines, false);
}

void QPainter::drawPolyline(const QPointArray &points, int index, int npoints)
{
    _drawPoints (points, 0, index, npoints, false);
}

void QPainter::drawPolygon(const QPointArray &points, bool winding, int index,
                           int npoints)
{
    _drawPoints (points, winding, index, npoints, true);
}

void QPainter::drawConvexPolygon(const QPointArray &points)
{
    _drawPoints (points, false, 0, -1, true);
}

void QPainter::_drawPoints (const QPointArray &_points, bool winding, int index, int _npoints, bool fill)
{
    if (data->state.paintingDisabled)
        return;
	
    int npoints = _npoints != -1 ? _npoints : _points.size()-index;
	
    GdkPoint points[npoints];
    for (int i = 0; i < npoints; i++) {
        points[i].x = _points[index+i].x();
        points[i].y = _points[index+i].y();
    }    
    
    
    if (fill && data->state.brush.style() != NoBrush) {
        _setColorFromBrush();
    }    
    
    if (data->state.pen.style() != NoPen) {
        _setColorFromPen();
    }
    
    gdk_draw_polygon(data->context->drawable, data->context->gc, fill, points, npoints);
}


void QPainter::drawPixmap(const QPoint &p, const QPixmap &pix)
{
    drawPixmap(p.x(), p.y(), pix);
}


void QPainter::drawPixmap(const QPoint &p, const QPixmap &pix, const QRect &r)
{
    drawPixmap(p.x(), p.y(), pix, r.x(), r.y(), r.width(), r.height());
}

struct CompositeOperator
{
    const char *name;
    NSCompositingOperation value;
};

#define NUM_COMPOSITE_OPERATORS 14
struct CompositeOperator compositeOperators[NUM_COMPOSITE_OPERATORS] = {
    { "clear", NSCompositeClear },
    { "copy", NSCompositeCopy },
    { "source-over", NSCompositeSourceOver },
    { "source-in", NSCompositeSourceIn },
    { "source-out", NSCompositeSourceOut },
    { "source-atop", NSCompositeSourceAtop },
    { "destination-over", NSCompositeDestinationOver },
    { "destination-in", NSCompositeDestinationIn },
    { "destination-out", NSCompositeDestinationOut },
    { "destination-atop", NSCompositeDestinationAtop },
    { "xor", NSCompositeXOR },
    { "darker", NSCompositePlusDarker },
    { "highlight", NSCompositeHighlight },
    { "lighter", NSCompositePlusLighter }
};

int QPainter::getCompositeOperation(CGContextRef context)
{
    return (int) WebCoreImageRendererFactory::sharedFactory()->CGCompositeOperationInContext(context);
}

void QPainter::setCompositeOperation (CGContextRef context, QString op)
{
    WebCoreImageRendererFactory::sharedFactory()->setCGCompositeOperationFromString(context, op.latin1());
}

void QPainter::setCompositeOperation (CGContextRef context, int op)
{
    WebCoreImageRendererFactory::sharedFactory()->setCGCompositeOperation(context, op);
}

int QPainter::compositeOperatorFromString (QString aString)
{
    NSCompositingOperation op = NSCompositeSourceOver;
    
    if (aString.length()) {
        const char *operatorString = aString.ascii();
        int i;
        
        for (i = 0; i < NUM_COMPOSITE_OPERATORS; i++) {
            if (strcasecmp (operatorString, compositeOperators[i].name) == 0) {
                return compositeOperators[i].value;
            }
        }
    }
    return (int)op;
}

void QPainter::drawPixmap(const QPoint &p, const QPixmap &pix, const QRect &r, const QString &compositeOperator)
{
    drawPixmap(p.x(), p.y(), pix, r.x(), r.y(), r.width(), r.height(), compositeOperatorFromString(compositeOperator));
}

void QPainter::drawPixmap( int x, int y, const QPixmap &pixmap,
                           int sx, int sy, int sw, int sh, int compositeOperator, CGContextRef context)
{
    drawPixmap(x, y, sw, sh, pixmap, sx, sy, sw, sh, compositeOperator, context);
}


void QPainter::drawPixmap( int x, int y, int w, int h, const QPixmap &pixmap,
                           int sx, int sy, int sw, int sh, int compositeOperator, CGContextRef context )
{
    if (data->state.paintingDisabled)
        return;

    if (pixmap.isNull())
        return;

    if (!context) {
        context = data->context;
    } else {
    }
    
    if (sw < 0) sw = pixmap.width();
    if (sh < 0) sh = pixmap.height();
    if (w < 0) w = pixmap.width();
    if (h < 0) h = pixmap.height();
        
    GdkRectangle inRect;
    initGdkRectangle(&inRect, x, y, w, h);

    GdkRectangle fromRect;
    initGdkRectangle(&fromRect, sx, sy, w, h);
    pixmap.image()->drawImageInRect(&inRect, &fromRect, NSCompositingOperation(compositeOperator), context);
}

void QPainter::drawTiledPixmap( int x, int y, int w, int h,
                                const QPixmap &pixmap, int sx, int sy, CGContextRef context)
{
    ASSERT(w >= 0);
    ASSERT(h >= 0);

    if (data->state.paintingDisabled)
        return;

    if (pixmap.isNull())
        return;

    if (!context) {
        context = data->context;
    }

    /** sw = source width, sh = source height*/
    int sw = pixmap.width(), sh = pixmap.height(); 

    if (sx <0) sx = 0;
    if (sy <0) sy = 0;
    /* (due to translation) we might get negative -x (not drawn) 
        clip it to 0 and adjust other params too*/
    if (x < 0) {
        sx += -x;
        w += x;
        x = 0;
    }
    
    /* (due to translation) we might get negative -y (not drawn)  */
    if (y < 0) {
        sy += -y;
        h += y;
        y = 0;        
    }

    /* if source start value is bigger than source dim, adjust it to mean same but be within
       the source dim */
    if (sx >= sw) 
        sx %= sw;    
    if (sy >= sh)
        sy %= sh;

    if (w <= 0 || h <= 0)
        return;

    GdkRectangle inRect;    
    initGdkRectangle(&inRect, x, y, w, h);
    GdkRectangle fromRect;
    initGdkRectangle(&fromRect, x, y, w, h);

    pixmap.image()->tileInRect(&inRect, sx, sy, context);
        
}

void QPainter::_updateRenderer()
{
    if (data->textRenderer == 0 || data->state.font != data->textRendererFont) {
        data->textRendererFont = data->state.font;
        WebCoreTextRenderer* oldRenderer = data->textRenderer;
        data->textRenderer = 
            WebCoreTextRendererFactory::sharedFactory()->rendererWithFont(data->textRendererFont.getNSFont(),
                                                                          data->textRendererFont.isPrinterFont());
        
        KWQRetain(data->textRenderer);
        ASSERT(data->textRenderer);
        KWQRelease(oldRenderer);
    }
}

void QPainter::drawText(int x, int y, int, int, int alignmentFlags, const QString &qstring)
{
    if (data->state.paintingDisabled)
        return;

    // Avoid allocations, use stack array to pass font families.  Normally these
    // css fallback lists are small <= 3.
    CREATE_FAMILY_ARRAY(data->state.font, families);

    _updateRenderer();

    const UniChar* str = (const UniChar*)qstring.unicode();

    WebCoreTextRun run(str, qstring.length(), 0, qstring.length());
    WebCoreTextStyle style;
    data->state.pen.color().getGdkColor(&style.textColor);
    style.families = families;
    
    if (alignmentFlags & Qt::AlignRight)
        x -= ROUND_TO_INT(data->textRenderer->floatWidthForRun(&run, &style, 0));

    data->textRenderer->setContext(data->context);
    data->textRenderer->drawRun(&run, &style, x, y);

}

void QPainter::drawText(int x, int y, const QChar *str, int len, int from, int to, int toAdd, const QColor &backgroundColor, QPainter::TextDirection d, bool visuallyOrdered, int letterSpacing, int wordSpacing, bool smallCaps)    
{
    if (data->state.paintingDisabled || len <= 0)
        return;
    // Avoid allocations, use stack array to pass font families.  Normally these
    // css fallback lists are small <= 3.
    CREATE_FAMILY_ARRAY(data->state.font, families);

    _updateRenderer();

    if (from < 0)
        from = 0;
    if (to < 0)
        to = len;

    WebCoreTextRun run((const UniChar *)str, len, from, to);
    WebCoreTextStyle style;
    data->state.pen.color().getGdkColor(&style.textColor);
    backgroundColor.getGdkColor(&style.backgroundColor); // !isValid() => sets style.backgroundColor "nil"
    style.rtl = d == RTL ? true : false;
    style.visuallyOrdered =  visuallyOrdered;
    style.letterSpacing = letterSpacing;
    style.wordSpacing = wordSpacing;
    style.smallCaps = smallCaps;

    style.padding = toAdd;
    style.families = families;

    data->textRenderer->setContext(data->context);
    data->textRenderer->drawRun(&run, &style, x, y );
}

void QPainter::drawHighlightForText(int x, int minX, int maxX, int y, int h, 
                                    const QChar *str, int len, int from, int to, int toAdd,
                                    const QColor& backgroundColor, QPainter::TextDirection d, bool visuallyOrdered,
                                    int letterSpacing, int wordSpacing, bool smallCaps)

{
    if (data->state.paintingDisabled || len <= 0)
        return;

    // Avoid allocations, use stack array to pass font families.  Normally these
    // css fallback lists are small <= 3.
    CREATE_FAMILY_ARRAY(data->state.font, families);

    _updateRenderer();
    
    if (from < 0)
        from = 0;
    if (to < 0)
        to = len;

    WebCoreTextRun run((const UniChar *)str, len, from, to);
    WebCoreTextStyle style;
    data->state.pen.color().getGdkColor(&style.textColor);
    backgroundColor.getGdkColor(&style.backgroundColor); // !isValid() => sets style.backgroundcolor "nil"
    style.rtl = d == RTL ? true : false;
    style.visuallyOrdered = visuallyOrdered;        
    style.letterSpacing = letterSpacing;
    style.wordSpacing = wordSpacing;
    style.smallCaps = smallCaps;
    style.padding = toAdd;
    style.families = families;

    WebCoreTextGeometry geometry(x,y, y, h, minX, maxX, false);

    data->textRenderer->setContext(data->context);
    data->textRenderer->drawHighlightForRun(&run, &style, &geometry);
}

void QPainter::drawLineForText(int x, int y, int yOffset, int width)
{
    if (data->state.paintingDisabled)
        return;
	
    drawLine(x, y+yOffset+1, x+width, y+yOffset+1);
    
#if 0
    GdkColor col;
    data->state.pen.color().getGdkColor(&col);
    data->textRenderer->drawLineForCharacters(x, y,
                                              yOffset,
                                              width,
                                              col);
#endif
        

}

QColor QPainter::selectedTextBackgroundColor() const
{
    static QColor secondarySelectedControlColor("gray");
    static QColor selectedTextBackgroundColor(qRgba((int)(255*0.1f),
                                                    (int)(255*0.1f),
                                                    (int)(255*1.0f),
                                                    (int) (255 * 0.3f)));
    
    return _usesInactiveTextBackgroundColor ?  secondarySelectedControlColor : selectedTextBackgroundColor;
}

#if !KWIQ
void QPainter::_fillRect(float x, float y, float w, float h, const QColor& col)
{
    [col.getNSColor() set];
    NSRectFillUsingOperation(NSMakeRect(x,y,w,h), NSCompositeSourceOver);
}
#endif

void QPainter::fillRect(int x, int y, int w, int h, const QBrush &brush)
{
    if (data->state.paintingDisabled)
        return;

    if (brush.style() == SolidPattern) {
        QBrush old = this->brush();
        setBrush(brush);
        _setColorFromBrush();
        gdk_draw_rectangle(data->context->drawable, data->context->gc, TRUE, x, y, w, h);
        setBrush(old);
    }
}

void QPainter::addClip(const QRect &rect)
{
    GdkRectangle gr;        
    rect.getGdkRectangle(&gr);
    data->context->addClip(&gr);
}

Qt::RasterOp QPainter::rasterOp() const
{
    return CopyROP;
}

void QPainter::setRasterOp(RasterOp)
{
}

void QPainter::setPaintingDisabled(bool f)
{
    data->state.paintingDisabled = f;
}

bool QPainter::paintingDisabled() const
{
    return data->state.paintingDisabled;
}

void QPainter::setContext(CGContextRef context)
{
    if (context == data->context) 
        return;

    data->context = context;
    // FIXME: init with current state.
}

CGContextRef QPainter::currentContext()
{
    return data->context;
}

void QPainter::beginTransparencyLayer(float opacity)
{
#if 0
    [NSGraphicsContext saveGraphicsState];
    CGContextRef context = (CGContextRef)([[NSGraphicsContext currentContext] graphicsPort]);
    CGContextSetAlpha(context, opacity);
    CGContextBeginTransparencyLayer(context, 0);
#endif
}

void QPainter::endTransparencyLayer()
{
#if 0
    CGContextRef context = (CGContextRef)([[NSGraphicsContext currentContext] graphicsPort]);
    CGContextEndTransparencyLayer(context);
    [NSGraphicsContext restoreGraphicsState];
#endif
}

void QPainter::setShadow(int x, int y, int blur, const QColor& color)
{
#if !KWIQ
    // Check for an invalid color, as this means that the color was not set for the shadow
    // and we should therefore just use the default shadow color.
    CGContextRef context = (CGContextRef)([[NSGraphicsContext currentContext] graphicsPort]);
    if (!color.isValid())
        CGContextSetShadow(context, CGSizeMake(x,-y), blur); // y is flipped.
    else {
        NSColor* deviceColor = [color.getNSColor() colorUsingColorSpaceName: @"NSDeviceRGBColorSpace"];
        float red = [deviceColor redComponent];
        float green = [deviceColor greenComponent];
        float blue = [deviceColor blueComponent];
        float alpha = [deviceColor alphaComponent];
        const float components[] = { red, green, blue, alpha };

        CGContextSetShadowWithColor(context,
                                    CGSizeMake(x,-y), // y is flipped.
                                    blur,
                                    CGColorCreate(CGColorSpaceCreateDeviceRGB(),
                                                  components));
    }
#endif
}

void QPainter::clearShadow()
{
#if !KWIQ
    CGContextRef context = (CGContextRef)([[NSGraphicsContext currentContext] graphicsPort]);
    CGContextSetShadowWithColor(context, CGSizeZero, 0, NULL);
#endif
}

void QPainter::initFocusRing(int width, int offset)
{
#if 0
    if (!_drawsFocusRing)
        return;

    clearFocusRing();
    data->focusRingWidth = width;
    data->hasFocusRingColor = false;
    data->focusRingOffset = offset;
    data->focusRingPath = [[NSBezierPath alloc] init];
    [data->focusRingPath setWindingRule:NSNonZeroWindingRule];
#endif     
}


void QPainter::initFocusRing(int width, int offset, const QColor &color)
{
#if 0    
    if (!_drawsFocusRing)
        return;

    initFocusRing(width, offset);
    data->hasFocusRingColor = true;
    data->focusRingColor = color;
#endif    
}

void QPainter::addFocusRingRect(int x, int y, int width, int height)
{
#if 0
    if (!_drawsFocusRing)
        return;

    ASSERT(data->focusRingPath);
    NSRect rect = NSMakeRect(x, y, width, height);
    int offset = (data->focusRingWidth-1)/2 + data->focusRingOffset;
    rect = NSInsetRect(rect, -offset, -offset);
    [data->focusRingPath appendBezierPathWithRect:rect];
#endif    
}

void QPainter::drawFocusRing()
{
#if 0
    if (!_drawsFocusRing)
        return;
    
    ASSERT(data->focusRingPath);
    if (data->state.paintingDisabled)
        return;

    if ([data->focusRingPath elementCount] == 0) {
        ERROR("Request to draw focus ring with no control points");
        return;
    }
    
    NSRect bounds = [data->focusRingPath bounds];
    if (!NSIsEmptyRect(bounds)) {
        int radius = (data->focusRingWidth-1)/2;
        NSColor *color = data->hasFocusRingColor ? data->focusRingColor.getNSColor() : nil;
        [NSGraphicsContext saveGraphicsState];
        [[WebCoreGraphicsBridge sharedBridge] setFocusRingStyle:NSFocusRingOnly radius:radius color:color];
        [data->focusRingPath fill];
        [NSGraphicsContext restoreGraphicsState];   
    }
#endif    
}

void QPainter::clearFocusRing()
{
#if 0    
    if (data->focusRingPath) {
        [data->focusRingPath release];
        data->focusRingPath = nil;
    }
#endif
}



