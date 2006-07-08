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

#include <gtk/gtk.h>

#include "KWQWidget.h"

#include "KWQKHTMLPart.h"
#include "KWQLogging.h"
#include "KWQWindowWidget.h"
#include "WebCoreBridge.h"

#include "khtmlview.h"
#include "render_canvas.h"
#include "render_replaced.h"
#include "render_style.h"

#include "KWQEvent.h"
#include "KWQNamespace.h"
#include "KWQApplication.h"

#include "GObjectMisc.h"

/** callbacks */
extern "C" {
static gboolean focus_inout(GtkWidget *widget, GdkEventFocus *event, gpointer data);
}


using khtml::RenderWidget;


class QWidgetPrivate
{

public:
    QWidgetPrivate()
	:inPaint(0), geom(QRect(0,0,0,0)) {}

    QStyle *style;
    QFont font;
    QPalette pal;
    GPtr<GtkWidget> safe_widget;
    bool visible;

    QWidget* parent;
    int inPaint;
    QRect geom;
    QCursor currentCursor;

};


QWidget::QWidget(QWidget * parent, const char * name , int f ) 
    : data(new QWidgetPrivate)
      ,_widget(0)
{
    QOBJECT_TYPE(QWidget);

    static QStyle defaultStyle;
    data->style = &defaultStyle;
    data->parent = parent;
    //data->widget = KWQRetain(view);
    data->visible = true;
}

QWidget::QWidget(GtkWidget* awidget)
    : data(new QWidgetPrivate)
      ,_widget(0)
{
    QOBJECT_TYPE(QWidget);

    static QStyle defaultStyle;
    data->style = &defaultStyle;
    //data->widget = KWQRetain(view);
    data->visible = true;

    setGtkWidget(awidget);
}


QWidget::~QWidget() 
{
    if (_widget) {
	// window of widget
	if (_widget->window) 
	    gdk_window_set_cursor(_widget->window, NULL); 	

	data->safe_widget = 0; // unrefs etc
    }
    
    delete data;
}

QSize QWidget::sizeHint() const 
{
    // May be overriden by subclasses.
    ASSERT(_widget);
    GtkRequisition r;
    gtk_widget_size_request(_widget, &r);
    return QSize(r.width, r.height);
}

void QWidget::resize(int w, int h) 
{
    ASSERT(_widget);
    LOG(KwiqLog, "this:%x size:%d,%d", (int)this, w, h);
    QSize s(size());
    if (s.width()!=w || s.height()!=h)
	gtk_widget_set_size_request(_widget, w, h);
}

void QWidget::setActiveWindow() 
{
    KWQKHTMLPart::bridgeForWidget(this)->focusWindow();
}

void QWidget::setEnabled(bool enabled)
{
    gtk_widget_set_sensitive(_widget, enabled);
}

bool QWidget::isEnabled() const
{
    return GTK_WIDGET_IS_SENSITIVE(_widget);
}

long QWidget::winId() const
{
    return (long)this;
}

int QWidget::x() const
{
    ASSERT(_widget);
    return  _widget->allocation.x;
}

int QWidget::y() const 
{
    ASSERT(_widget);
    return _widget->allocation.y;
}

int QWidget::width() const 
{ 
    ASSERT(_widget);
    return _widget->allocation.width;
}

int QWidget::height() const 
{
    ASSERT(_widget);
    return _widget->allocation.height;
}

QSize QWidget::size() const 
{
    return QSize(_widget->allocation.width, _widget->allocation.height);
}

void QWidget::resize(const QSize &s) 
{
    resize(s.width(), s.height());
}

QPoint QWidget::pos() const 
{
    return QPoint(_widget->allocation.x, _widget->allocation.y);
}

void QWidget::move(int x, int y) 
{
    LOG(KwiqLog, "this:%x, _widget:%x, size:%d,%d",(int)this, (int)_widget, x, y);
    ASSERT(_widget);

    QPoint p(pos());
    if (p.x()!=x || p.y()!=y) {
	GtkLayout *layout = GTK_LAYOUT (gtk_widget_get_parent(_widget));
	if (layout) {
	    gtk_layout_move(layout, _widget, x, y);
	} else {
	    GtkContainer *parent = GTK_CONTAINER (gtk_widget_get_parent(_widget));	
	    if (parent) {
		GValue gval= {0,};
		g_value_init(&gval, G_TYPE_INT);
		g_value_set_int(&gval, x);
		gtk_container_child_set_property(parent, _widget, "x", &gval);  
		g_value_set_int(&gval, y);
		gtk_container_child_set_property(parent, _widget, "y", &gval);  
		g_value_unset(&gval);
	    }
	}
    }

}

void QWidget::move(const QPoint &p) 
{
    move(p.x(), p.y());
}

QRect QWidget::frameGeometry() const
{
    if (!_widget){
	g_warning("widget was zero");
	return QRect(-1,-1,-1,-1);
	ASSERT(_widget);
    }

    return QRect( _widget->allocation.x,
		  _widget->allocation.y,
		  _widget->allocation.width,
		  _widget->allocation.height);
}

void QWidget::setFrameGeometry(const QRect &rect)
{
    LOG(KwiqLog, "this:%x, _widget==0, rect:(%d,%d; %d,%d)",
	(int)this, 
	rect.x(),
	rect.y(),
	rect.width(), 
	rect.height());

    ASSERT(_widget);

    g_object_freeze_notify(G_OBJECT (_widget));
    move(rect.x(), rect.y());
    resize(rect.width(), rect.height());
    g_object_thaw_notify(G_OBJECT (_widget));
}


int QWidget::baselinePosition(int height) const
{
    return (int)((16.0f/20.0f)*height);
}

bool QWidget::hasFocus() const
{    
    if (!_widget) return  false;
    return GTK_WIDGET_HAS_FOCUS (_widget);
}

void QWidget::setFocus()
{
    if (hasFocus()) {
        return;
    }

    // KHTML will call setFocus on us without first putting us in our
    // superview and positioning us. Normally layout computes the position
    // and the drawing process positions the widget. Do both things explicitly.
    RenderWidget *renderWidget = 0;
    QObject *qo = const_cast<QObject *>(eventFilterObject());    
    if (qo && QOBJECT_IS_A(qo, khtml::RenderWidget))
	renderWidget = static_cast<RenderWidget *>(qo);
    int x, y;
    if (renderWidget) {
        if (renderWidget->canvas()->needsLayout()) {
            renderWidget->view()->layout();
        }
        if (renderWidget->absolutePosition(x, y)) {
            renderWidget->view()->addChild(this, x, y);
        }
    }

    if (!_widget)
	return;

    if (GTK_WIDGET_CAN_FOCUS(_widget)) {
        KWQKHTMLPart::bridgeForWidget(this)->makeFirstResponder(_widget);
    }
}

void QWidget::clearFocus()
{
    if (!hasFocus()) {
        return;
    }

    KWQKHTMLPart::clearDocumentFocus(this);
}

bool QWidget::checksDescendantsForFocus() const
{
    return false;
}

QWidget::FocusPolicy QWidget::focusPolicy() const
{
    // This provides support for controlling the widgets that take 
    // part in tab navigation. Widgets must:
    // 1. not be hidden by css
    // 2. be enabled
    // 3. accept first responder

    RenderWidget *widget = const_cast<RenderWidget *>
	(static_cast<const RenderWidget *>(eventFilterObject()));
    if (widget->style()->visibility() != khtml::VISIBLE)
        return NoFocus;

    if (!isEnabled())
        return NoFocus;
#if 0    
    KWQ_BLOCK_EXCEPTIONS;
    if (![getView() acceptsFirstResponder])
        return NoFocus;
    KWQ_UNBLOCK_EXCEPTIONS;
#endif    

    return TabFocus;
}

const QPalette& QWidget::palette() const
{
    return data->pal;
}

void QWidget::setPalette(const QPalette &palette)
{
    data->pal = palette;
}

void QWidget::unsetPalette()
{
    // Only called by RenderFormElement::layout, which I suspect will
    // have to be rewritten.  Do nothing.
}

QStyle &QWidget::style() const
{
    return *data->style;
}

void QWidget::setStyle(QStyle *style)
{
    // According to the Qt implementation 
    /*
    Sets the widget's GUI style to \a style. Ownership of the style
    object is not transferred.
    */
    data->style = style;
}

QFont QWidget::font() const
{
    return data->font;
}

void QWidget::setFont(const QFont &font)
{
    //g_warning("QWidget::setFont: size:%d, family:%s", font.pixelSize(), font.family().latin1());
    data->font = font;
    if (_widget){
	PangoFontDescription *fd =
        	createPangoFontDescription(&font);
	gtk_widget_modify_font(_widget, fd);
	pango_font_description_free(fd); //should be freed or not?
    }
}

void QWidget::constPolish() const
{
}

bool QWidget::isVisible() const
{
    if (!_widget) 
	return false;

    return GTK_WIDGET_VISIBLE(_widget);
}

void QWidget::setCursor(const QCursor &cur)
{
    if (!_widget) return;
    if (_widget->window) {
	gdk_window_set_cursor(_widget->window, cur.handle()); // NULL means parent
	data->currentCursor = cur;
    }
}


QCursor QWidget::cursor()
{
    //x doesn't support this..
    //in KHTMLView::viewPortMouseEvent this is needed for comparing current vs new

    return data->currentCursor;
}

void QWidget::unsetCursor()
{
    setCursor(QCursor());
}

bool QWidget::focusNextPrevChild(bool)
{
    // didn't find anything to focus to. return false, so the event can be
    // handled by the parent (by returning FALSE for focus -signal)
    return false;
}

bool QWidget::hasMouseTracking() const
{
    return true;
}

void QWidget::show()
{
    if (!_widget) return;

    if (!data || isVisible())
        return;

    gtk_widget_show_all( _widget );
}

void QWidget::hide()
{
    
    if (!data || !isVisible())
        return;

    gtk_widget_hide( _widget );
}

QPoint QWidget::mapFromGlobal(const QPoint &p) const
{
#if 0
    NSPoint bp = {0,0};

    KWQ_BLOCK_EXCEPTIONS;
    bp = [[KWQKHTMLPart::bridgeForWidget(this) window] convertScreenToBase:[data->view convertPoint:p toView:nil]];
    KWQ_UNBLOCK_EXCEPTIONS;
#endif
    LOG(NotYetImplemented, "KWIQ");
    return QPoint();
}

void QWidget::setGtkWidget(GtkWidget* widget)
{
    if (widget == _widget) return;
    
    _widget = widget;
    data->safe_widget = widget;
    
    if (_widget) {
	data->safe_widget.connect("focus-in-event",
			::focus_inout,
			this);
	data->safe_widget.connect("focus-out-event",
			::focus_inout,
			this);
	if (widget->window) 
	    gdk_window_set_cursor(widget->window, NULL);
    }
}

void QWidget::lockDrawingFocus()
{
    LOG(NotYetImplemented, "KWIQ");
}

void QWidget::unlockDrawingFocus()
{
    LOG(NotYetImplemented, "KWIQ");
}

void QWidget::disableFlushDrawing()
{
    LOG(NotYetImplemented, "KWIQ");
}

void QWidget::enableFlushDrawing()
{
    LOG(NotYetImplemented, "KWIQ");
}

void QWidget::setDrawingAlpha(float alpha)
{
    LOG(NotYetImplemented,"KWIQ");
}

void QWidget::paint(QPainter *p, const QRect &r)
{
    if (p->paintingDisabled()) {
        return;
    }
    data->inPaint++;
    GdkEventExpose eev;
    eev.type = GDK_EXPOSE;
    eev.window = _widget->window;
    eev.send_event = TRUE;
    eev.count = 0;
    eev.area.x =  r.x();
    eev.area.y =  r.y();
    eev.area.width =  r.width()+1;
    eev.area.height = r.height()+1;
    eev.region = gdk_region_rectangle(&eev.area);
    
    gtk_widget_send_expose(_widget, (GdkEvent*)&eev);
    gdk_region_destroy(eev.region);
    data->inPaint--;
}

void QWidget::sendConsumedMouseUp()
{
#if 0    
    khtml::RenderWidget *widget = const_cast<khtml::RenderWidget *>
	(static_cast<const khtml::RenderWidget *>(eventFilterObject()));

    KWQ_BLOCK_EXCEPTIONS;
    widget->sendConsumedMouseUp(QPoint([[NSApp currentEvent] locationInWindow]),
			      // FIXME: should send real state and button
			      0, 0);
#endif    
    LOG(NotYetImplemented, "KWIQ");
}

void QWidget::setWritingDirection(QPainter::TextDirection direction)
{
  
  
    GtkWidget *w = getGtkWidget();
    PangoContext *pc = gtk_widget_get_pango_context( w );
    PangoDirection pdir = pango_context_get_base_dir( pc );    
    
    switch (direction){
    case QPainter::LTR:
    {
        // do nothing as direction is already correct
        if ( pdir == PANGO_DIRECTION_LTR ) return;
        pdir = PANGO_DIRECTION_LTR;
    }
    case QPainter::RTL:
        // do nothing as direction is already correct
        if ( pdir == PANGO_DIRECTION_RTL ) return;
  	pdir = PANGO_DIRECTION_RTL;
    }
	
    pango_context_set_base_dir( pc, pdir );
}

// caller should destroy description
PangoFontDescription* createPangoFontDescription(const QFont* font) 
{
    PangoFontDescription* descr;
    descr = pango_font_description_new();

    pango_font_description_set_family(descr,
	font->family().latin1());
    
    pango_font_description_set_weight(descr,
	(font->weight()==QFont::Bold)?PANGO_WEIGHT_BOLD:PANGO_WEIGHT_NORMAL);
    
   pango_font_description_set_style(descr,
       font->italic()?PANGO_STYLE_ITALIC:PANGO_STYLE_NORMAL);
   
   if (font->pixelSize()<1){
       pango_font_description_set_size(descr,(int)(PANGO_SCALE*12));
       LOG(KwiqLog,"QFont::_size < 1");
   }
   else       
       pango_font_description_set_size(descr,(int)(PANGO_SCALE*font->pixelSize()));
   
   return descr;
}


extern "C" {

/** @return \TRUE if stops focus in event propagation, \FALSE if propagation continues */
static gboolean
focus_inout(GtkWidget *widget, GdkEventFocus *event, gpointer data)
{
#if DEBUG
    qDebug("%s: widget:%x data:%x in:%x\n",
	   __PRETTY_FUNCTION__,
	   widget,
	   data, event->in);
#endif

    QWidget *self = static_cast<QWidget*>(data);
    ASSERT(self);

    if (!self->eventFilterObject()) return FALSE;

    if (event->in == TRUE){
        QFocusEvent event(QEvent::FocusIn);
	const_cast<QObject *>(self->eventFilterObject())->eventFilter(self, &event);
    } else {
	QFocusEvent event(QEvent::FocusOut);
        const_cast<QObject *>(self->eventFilterObject())->eventFilter(self, &event);
    }
    return FALSE;
}

} /* extern "C" */
