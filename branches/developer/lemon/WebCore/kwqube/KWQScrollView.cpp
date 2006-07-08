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

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include "GObjectMisc.h"

#include "KWQScrollView.h"
#include "KWQLogging.h"
#include "KWQApplication.h"


class QScrollViewPrivate
{
public:
    GPtr<GtkWidget> contents;
};

QScrollView::QScrollView(QWidget *parent, const char *name, int flags )
    :QFrame(parent, name, flags)
     ,_staticBackground(false)
     , d(new QScrollViewPrivate())
{
    QOBJECT_TYPE(QScrollView);
    _ha = 0;
    _va = 0;
}

QScrollView::~QScrollView()
{
    delete d;
}

void QScrollView::setGtkWidget(GtkWidget* widget)
{
    _ha = 0;
    _va = 0;
    d->contents = 0;

    if (widget) {
	GtkScrolledWindow *sw = GTK_SCROLLED_WINDOW (widget);
	GtkWidget *child = gtk_bin_get_child (GTK_BIN (sw));
	ASSERT(sw);
	ASSERT(child);

	d->contents = child;
	_ha = gtk_scrolled_window_get_hadjustment(sw);
	_va = gtk_scrolled_window_get_vadjustment(sw);	
    }

    QFrame::setGtkWidget(widget);
    return;
}

QWidget* QScrollView::viewport() const
{
    return const_cast<QScrollView *>(this);
}

int QScrollView::visibleWidth() const
{
    if (!d->contents) return 0;
    return d->contents->allocation.width;
}

int QScrollView::visibleHeight() const
{
    if (!d->contents) return 0;
    return d->contents->allocation.height;
}

int QScrollView::contentsWidth() const
{
    if (!d->contents) return 0;
    if (!GTK_IS_LAYOUT(d->contents)) return visibleWidth();
    guint width =0;
    gtk_layout_get_size(GTK_LAYOUT (d->contents), &width, NULL);

    return width;
}

int QScrollView::contentsHeight() const
{
    if (!d->contents) return 0;
    if (!GTK_IS_LAYOUT(d->contents)) return visibleHeight();
    guint height =0;
    gtk_layout_get_size(GTK_LAYOUT (d->contents), NULL, &height);
    return height;
}

int QScrollView::contentsX() const
{
    if (!_ha) return 0;
    return (int)_ha->value;
}

int QScrollView::contentsY() const
{
    if (!_va) return 0;
    return (int)_va->value;
}

int QScrollView::childX(QWidget *w)
{
    return w->x();
}

int QScrollView::childY(QWidget *w)
{
    return w->y();
}

void QScrollView::scrollBy(int dx, int dy)
{
    setContentsPos(contentsX() + dx, contentsY() + dy);
}

void QScrollView::setContentsPos(int x, int y)
{
    if (!_ha||!_va) return;

    x = (x < 0) ? 0 : x;
    y = (y < 0) ? 0 : y;
    
    gtk_adjustment_set_value(_ha, x);
    gtk_adjustment_set_value(_va, y);
    gtk_adjustment_value_changed(_ha);
    gtk_adjustment_value_changed(_va);
}


static GtkPolicyType
_get_policytype(QScrollView::ScrollBarMode mode)
{
    switch (mode){
    case QScrollView::AlwaysOn:
	return GTK_POLICY_ALWAYS;
    case QScrollView::AlwaysOff:
	return GTK_POLICY_NEVER;
    case QScrollView::Auto:
	return GTK_POLICY_AUTOMATIC;
    }
    /* NEVER REACHED */
    return GTK_POLICY_AUTOMATIC;
}

void QScrollView::setVScrollBarMode(ScrollBarMode mode)
{
    GtkPolicyType h,v;    
    GtkScrolledWindow* w = GTK_SCROLLED_WINDOW (getGtkWidget());
    if (!w) {
	ASSERT(w);
	return;
    }

    gtk_scrolled_window_get_policy(w, &h, NULL);
    v = _get_policytype(mode);
    gtk_scrolled_window_set_policy(w, h, v);
}

void QScrollView::setHScrollBarMode(ScrollBarMode mode)
{
    GtkScrolledWindow* w = GTK_SCROLLED_WINDOW (getGtkWidget());
    if (!w) {
	ASSERT(w);
	return;
    }

    GtkPolicyType h,v;    
    gtk_scrolled_window_get_policy(w, NULL, &v);
    h = _get_policytype(mode);
    gtk_scrolled_window_set_policy(w, h, v);
}

void QScrollView::setScrollBarsMode(ScrollBarMode mode)
{
    GtkScrolledWindow *w = GTK_SCROLLED_WINDOW (getGtkWidget());
    GtkPolicyType v;    
    v = _get_policytype(mode);
    gtk_scrolled_window_set_policy(w, v, v);
}

static QScrollView::ScrollBarMode
_get_scrollbarmode(GtkPolicyType p)
{
    switch (p) {
    case GTK_POLICY_ALWAYS:
	return QScrollView::AlwaysOn;
    case GTK_POLICY_NEVER:
	return QScrollView::AlwaysOff;
    case GTK_POLICY_AUTOMATIC:
	return QScrollView::Auto;
    }
    
    /* NOT REACHED */
    ASSERT(0);
    return QScrollView::Auto;
}

QScrollView::ScrollBarMode
QScrollView::vScrollBarMode() const
{
    GtkScrolledWindow* w = GTK_SCROLLED_WINDOW (getGtkWidget());
    if (!w) return QScrollView::Auto;

    GtkPolicyType v;
    gtk_scrolled_window_get_policy(w, NULL, &v );
    return _get_scrollbarmode(v);
}

QScrollView::ScrollBarMode
QScrollView::hScrollBarMode() const
{
    GtkScrolledWindow* w = GTK_SCROLLED_WINDOW (getGtkWidget());
    if (!w) return QScrollView::Auto;

    GtkPolicyType h;
    gtk_scrolled_window_get_policy(w, &h, NULL);
    return _get_scrollbarmode(h);
}

bool QScrollView::hasVerticalScrollBar() const
{
    switch(vScrollBarMode()){
    case AlwaysOff:
	return false;
    case AlwaysOn:
	return true;
    case Auto:
	return ((_va->upper - _va->lower) > _va->page_size);
    }
    // NOT REACHED
    ASSERT(0);
    return false;
}

bool QScrollView::hasHorizontalScrollBar() const
{
    switch(hScrollBarMode()){
    case AlwaysOff:
	return false;
    case AlwaysOn:
    /* FIXME:KWIQ: Howto get scrollbar status in gtk ? */
    case Auto:
	return ((_ha->upper - _ha->lower) > _ha->page_size);
    }
    // NOT REACHED
    ASSERT(0);
    return false;    
}

void QScrollView::suppressScrollBars(bool suppressed,  bool repaintOnUnsuppress)
{
    LOG(KwiqLog, "not implemented");
}

void QScrollView::addChild(QWidget* child, int x, int y)
{
    ASSERT(child != this);
    ASSERT(child);
    ASSERT(GTK_IS_LAYOUT(d->contents));

    GtkWidget *w = child->getGtkWidget();
    ASSERT(w);

    GtkWidget *cur_cont = gtk_widget_get_parent(w);

    if (cur_cont != d->contents) {
	if (cur_cont) 
	    gtk_container_remove(GTK_CONTAINER (cur_cont), w);

	gtk_layout_put(GTK_LAYOUT(d->contents), w, x, y);

	if (isVisible())
	    child->show();
    }
    else {
	child->move(x, y); 	// child already in container, so move it
    }
}

void QScrollView::removeChild(QWidget* child)
{
    ASSERT(child);
    g_return_if_fail(d->contents);
    g_return_if_fail(child != this);
    g_return_if_fail(GTK_IS_CONTAINER(d->contents));
    
    GtkWidget *w = child->getGtkWidget();
    gtk_container_remove(GTK_CONTAINER(d->contents), w);

    child->hide();
}

void QScrollView::resizeContents(int w, int h)
{
    if (!d->contents) return;

    if (w < 1) w = 1;
    if (h < 1) h = 1;

    if (GTK_IS_LAYOUT (d->contents)) {
	guint curw,curh;
	gtk_layout_get_size(GTK_LAYOUT (d->contents), &curw, &curh);
	if (curw == (guint)w && curh == (guint)h)
	    return;

	gtk_layout_set_size(GTK_LAYOUT (d->contents), w, h);

    } else {
	gtk_widget_set_size_request(d->contents, w, h);
    }
}

void QScrollView::updateContents(int x, int y, int w, int h, bool now)    
{
    if (d->contents) {
	GdkWindow *win = d->contents->window;
	if (GTK_IS_LAYOUT (d->contents)) {
	    win = GTK_LAYOUT (d->contents)->bin_window;
	}

	if (win) { 
	    GdkRectangle r;
	    r.x = x;
	    r.y = y;
	    r.width = w;
	    r.height = h;
	    gdk_window_invalidate_rect(win, &r, TRUE);
	    if (now) 
		gdk_window_process_updates(win, TRUE);
	}
#if DEBUG
	else {
	    LOG(KwiqLog, "no window found");
	}	    
#endif	
    }
}

void QScrollView::updateContents(const QRect &rect, bool now)
{    
    updateContents(rect.x(),rect.y(),rect.width(),rect.height(), now);
}

void QScrollView::repaintContents(int x, int y, int w, int h, bool erase)
{
}

QPoint QScrollView::contentsToViewport(const QPoint &p)
{
    int vx, vy;
    contentsToViewport(p.x(), p.y(), vx, vy);
    return QPoint(vx, vy);
}

void QScrollView::contentsToViewport(int x, int y, int& vx, int& vy)
{
    vx = x - contentsX();
    vy = y - contentsY();
}

void QScrollView::viewportToContents(int vx, int vy, int& x, int& y)
{    
    x =  vx + contentsX();
    y =  vy + contentsY();
}

void QScrollView::resize(int w,int h)
{
    QFrame::resize(w,h);
}

void QScrollView::setStaticBackground(bool b)
{
    _staticBackground = b;
}

void QScrollView::resizeEvent(QResizeEvent * /*e*/ )
{
}

void QScrollView::setContentsPosRecursive(int x, int y)
{

}

void QScrollView::ensureVisible(int x,int y)
{
    // Note that the definition of ensureVisible in trolltech documentation says:
    // "Scrolls the content so that the point (x, y) is visible with at least 
    // 50-pixel margins (if possible, otherwise centered).", which is
    // not what we're doing here.
    ensureVisible(x,y,0,0);
}

void QScrollView::ensureVisible(int x,int y,int w,int h)
{
    // Note that the definition of ensureVisible in trolltech documentation says:
    // "Scrolls the content so that the point (x, y) is visible with at least 
    // 50-pixel margins (if possible, otherwise centered).", which is
    // not what we're doing here.
    gtk_adjustment_clamp_page(_ha, x - (w/2), x + (w/2));
    gtk_adjustment_clamp_page(_va, y - (h/2), y + (h/2));
}

void QScrollView::ensureRectVisibleCentered(const QRect &rect)
{
    ensureVisible(rect.x(), rect.y(), rect.width(), rect.height());
}


GtkWidget* QScrollView::getDocumentView() const
{
    return getGtkWidget();
}
