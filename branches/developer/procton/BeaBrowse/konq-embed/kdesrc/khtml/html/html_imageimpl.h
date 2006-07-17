/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * $Id: html_imageimpl.h,v 1.1.1.1 2002/04/27 11:01:51 procton Exp $
 */
#ifndef HTML_IMAGEIMPL_H
#define HTML_IMAGEIMPL_H

#include "html_inlineimpl.h"
#include "misc/khtmllayout.h"

#include <qregion.h>

namespace DOM {

class DOMString;

class HTMLImageElementImpl
    : public HTMLElementImpl
{
public:
    HTMLImageElementImpl(DocumentImpl *doc);

    ~HTMLImageElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() const { return IMGStartTag; }
    virtual tagStatus endTag() const { return IMGEndTag; }

    virtual void parseAttribute(AttrImpl *);

    virtual bool mouseEvent( int _x, int _y,
                             int _tx, int _ty,
                             MouseEvent *ev );

    virtual void attach(KHTMLView *w);
    virtual void applyChanges(bool top=true, bool force=true);
    virtual void recalcStyle();

    bool isServerMap() const;

protected:
    bool ismap;

    /**
     * The URL of this image.
     */
    DOMString imageURL;

    // text to display as long as the image isn't available
    DOMString alt;

    DOMString usemap;
};


//------------------------------------------------------------------



class HTMLAreaElementImpl : public HTMLAnchorElementImpl
{
public:

    enum Shape { Default, Poly, Rect, Circle, Unknown };

    HTMLAreaElementImpl(DocumentImpl *doc);
    ~HTMLAreaElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual tagStatus startTag() const { return AREAStartTag; }
    virtual tagStatus endTag() const { return AREAEndTag; }

    void parseAttribute(AttrImpl *attr);

    bool isDefault() const { return shape==Default; }
    bool isNoref() const { return nohref && !href; }

    bool mapMouseEvent(int x_, int y_, int width_, int height_,
                       MouseEvent *ev );

protected:

    QRegion getRegion(int width_, int height);
    QRegion region;
    QList<khtml::Length>* coords;
    int lastw, lasth;
    Shape shape  : 3;
    bool nohref  : 1;
};


// -------------------------------------------------------------------------

class HTMLMapElementImpl : public HTMLElementImpl
{
public:
    HTMLMapElementImpl(DocumentImpl *doc);

    ~HTMLMapElementImpl();

    virtual const DOMString nodeName() const;
    virtual ushort id() const;

    virtual DOMString getName() const { return name; }

    virtual void parseAttribute(AttrImpl *attr);

    virtual tagStatus startTag() const { return MAPStartTag; }
    virtual tagStatus endTag() const { return MAPEndTag; }

    bool mapMouseEvent(int x_, int y_, int width_, int height_,
                       MouseEvent *ev );
private:

    QString name;
};


}; //namespace

#endif
