/*
 * Copyright (C) 2001, 2002 Apple Computer, Inc.  All rights reserved.
 * Copyright (C) 2003 Nokia Corporation.  All rights reserved.
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

#include "KWQRegion.h"

QRegion::QRegion(const QRect &rect)
{
    r=rect;
    t=Rectangle;
}

QRegion::QRegion(int x, int y, int w, int h, RegionType _t)
{
    
    r = QRect(x,y,w,h);
    t = _t;
    
}

QRegion::QRegion(const QPointArray &arr)
{
    // ### poly
}

QRegion::~QRegion()
{
}

QRegion::QRegion(const QRegion &other)
{
    r=other.r;
    t=other.t;
}

QRegion &QRegion::operator=(const QRegion &other)
{
    if (&other == this)
	return *this;
    r=other.r;
    t=other.t;
    return *this;
}

bool QRegion::contains(const QPoint &point) const
{
    if ( t == Rectangle)
	return const_cast<QRegion*>(this)->r.contains(point.x(),point.y());
    else {
	int r2x = (r.width() * r.width()) / 4;
	int r2y = (r.height() * r.height()) / 4;
	
	if (!r2x || !r2y)  /*avoid division by zero*/
	    return false;

	int cx = (r.x() + r.width()) / 2;
	int cy = (r.y() + r.height()) / 2;
	int dx = point.x() - cx;
	int dy = point.y() - cy;
	return r2x >= ((dx*dx) + ((r2y * dy * dy) / r2x));
    }
}

void QRegion::translate(int deltaX, int deltaY)
{
    r.setX(r.x()+deltaX);
    r.setY(r.y()+deltaY);
}

QRect QRegion::boundingRect() const
{
    return r;
}
