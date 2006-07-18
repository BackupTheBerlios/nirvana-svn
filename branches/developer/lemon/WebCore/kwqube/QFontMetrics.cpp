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

#include "KWQFontMetrics.h"

#include "KWQFont.h"
#include "KWQLogging.h"

#include "WebCoreTextRendererFactory.h"
#include "WebCoreTextRenderer.h"
#include "KWQFoundationExtras.h"

typedef struct _PangoFontMetrics PangoFontMetrics;
struct QFontMetricsPrivate
{
    QFontMetricsPrivate(const QFont &font)
        :  refCount(0), _font(font), _renderer(0)
    {

    }

    ~QFontMetricsPrivate()
    {
	KWQRelease(_renderer);
    }

    WebCoreTextRenderer* getRenderer() 
    { 
	if (!_renderer) {
	    _renderer = WebCoreTextRendererFactory::sharedFactory()->rendererWithFont(_font.getNSFont(), 
										      _font.isPrinterFont());
	    KWQRetain(_renderer);
	}

	return _renderer;
    }

    const QFont &font() const { return _font; }    
    void setFont(const QFont &font)
    {
        if (_font == font) {
            return;
        }
        _font = font;

	KWQRelease(_renderer);
	_renderer = 0;
    }

    int refCount;
    
private:
    QFont _font;
    WebCoreTextRenderer* _renderer;

    QFontMetricsPrivate(const QFontMetricsPrivate& o);
    QFontMetricsPrivate& operator=(const QFontMetricsPrivate&);
};

QFontMetrics::QFontMetrics()
{

}
QFontMetrics::QFontMetrics(const QFont &font)
    : data(new QFontMetricsPrivate(font))
{
}

QFontMetrics::QFontMetrics(const QFontMetrics &other)
    : data(other.data)
{
}

QFontMetrics::~QFontMetrics()
{
}

QFontMetrics &QFontMetrics::operator=(const QFontMetrics &other)
{
    data = other.data;
    return *this;
}

void QFontMetrics::setFont(const QFont &font)
{
    if (data.isNull())
	data = KWQRefPtr<QFontMetricsPrivate>(new QFontMetricsPrivate(font));
    else {
	data->setFont(font);
    }
}

int QFontMetrics::ascent() const
{
    if (data.isNull()) {
        ERROR("called ascent on an empty QFontMetrics");
        return 0;
    }

    return data->getRenderer()->ascent;
}

int QFontMetrics::descent() const
{
    if (data.isNull()) {
        ERROR("called descent on an empty QFontMetrics");
        return 0;
    }

    return data->getRenderer()->descent;
}

int QFontMetrics::height() const
{
    // According to Qt documentation: 
    // "This is always equal to ascent()+descent()+1 (the 1 is for the base line)."
    // We DO NOT match the Qt behavior here.  This is intentional.
    return ascent() + descent();
}

int QFontMetrics::lineSpacing() const
{
    if (data.isNull()) {
        ERROR("called lineSpacing on an empty QFontMetrics");
        return 0;
    }
    return data->getRenderer()->lineSpacing;
}

float QFontMetrics::xHeight() const
{
    if (data.isNull()) {
        ERROR("called xHeight on an empty QFontMetrics");
        return 0;
    }    
    
    return data->getRenderer()->xHeight;
}

int QFontMetrics::width(QChar qc) const
{
    return width(&qc, 1);
}

int QFontMetrics::charWidth(const QString &s, int pos) const
{
    return width(s[pos]);
}

int QFontMetrics::width(char c) const
{
    QChar qc(c);    
    return width(&qc,1);
}

int QFontMetrics::width(const QString &qstring, int len) const
{
    if (len==-1)
	len = qstring.length();
            
    return width(qstring.unicode(),len);
}

int QFontMetrics::width(const QChar *uchars, int len) const
{
    return ROUND_TO_INT(floatWidth(uchars,len,0,len,0,0,false));
}


float QFontMetrics::floatWidth(const QChar *uchars, int slen, int pos, int len,
                               int letterSpacing, int wordSpacing, bool smallCaps) const
{
    if (data.isNull()) {
        ERROR("called floatWidth on an empty QFontMetrics");
        return 0;
    }

    return floatCharacterWidths(uchars, slen, pos, len, 0, 0, letterSpacing, wordSpacing, smallCaps);
}


float QFontMetrics::floatCharacterWidths(const QChar *uchars, int slen, int pos, int len, int toAdd, float *buffer, int letterSpacing, int wordSpacing, bool smallCaps) const
{

    if (data.isNull()) {
        ERROR("called floatCharacterWidths on an empty QFontMetrics");
        return 0;
    }

    WebCoreTextRun run( (const UniChar *)uchars, slen, pos, pos+len);

    CREATE_FAMILY_ARRAY(data->font(), families);
    
    WebCoreTextStyle style;
    style.letterSpacing = letterSpacing;
    style.wordSpacing = wordSpacing;
    style.smallCaps = smallCaps;
    style.padding = toAdd;
    style.families = families;

    return ROUND_TO_INT(data->getRenderer()->floatWidthForRun(&run, &style, buffer)); //run, style, widths
}

int QFontMetrics::checkSelectionPoint (QChar *s, int slen, int pos, int len, int toAdd, int letterSpacing, int wordSpacing, bool smallCaps, int x, bool reversed, bool includePartialGlyphs) const
{
    if (data.isNull()) {
        ERROR("called floatWidth on an empty QFontMetrics");
        return 0;
    }
    
    WebCoreTextRun run( (const UniChar *)s, slen, pos, pos+len);

    CREATE_FAMILY_ARRAY(data->font(), families);
    
    WebCoreTextStyle style;
    style.letterSpacing = letterSpacing;
    style.wordSpacing = wordSpacing;
    style.smallCaps = smallCaps;
    style.padding = toAdd;
    style.families = families;

    return data->getRenderer()->pointToOffset(&run, &style, x, reversed, includePartialGlyphs);
}

QRect QFontMetrics::boundingRect(const QString &qstring, int len) const
{
    return QRect(0, 0, width(qstring, len), height());
}

QRect QFontMetrics::boundingRect(int x, int y, int width, int height, int flags, const QString &str) const
{
    // FIXME: need to support word wrapping?
    return QRect(x, y, width, height).intersect(boundingRect(str));
}

QSize QFontMetrics::size(int, const QString &qstring) const
{
    return QSize(width(qstring), height());
}
