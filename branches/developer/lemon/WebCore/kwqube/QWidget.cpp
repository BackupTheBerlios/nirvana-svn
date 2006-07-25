/*
    Copyright (c) 2006 Maxim Sokhtasky (maxim@sokhatsky.com)
*/

#include <AppKit.h>
#include "KWQWidget.h"
#include "KWQKHTMLPart.h"
#include "KWQLogging.h"
#include "KWQWindowWidget.h"
#include "KWQEvent.h"
#include "KWQNamespace.h"
#include "KWQApplication.h"

//#include <gtk/gtk.h>
//#include "WebCoreBridge.h"
//#include "khtmlview.h"
//#include "render_canvas.h"
//#include "render_replaced.h"
//#include "render_style.h"
//#include "GObjectMisc.h"

/** callbacks */
//extern "C" {
//static gboolean focus_inout(GtkWidget *widget, GdkEventFocus *event, gpointer data);
//}
//using khtml::RenderWidget;

class QWidgetPrivate {
public:
    QWidgetPrivate() :inPaint(0), geom(QRect(0,0,0,0)) {}
    QStyle *style;
    QFont font;
    QPalette pal;
    //GPtr<GtkWidget> safe_widget;
    BView* safe_widget;    
    bool visible;
    QWidget* parent;
    int inPaint;
    QRect geom;
    QCursor currentCursor;
};

//QWidget::QWidget(QWidget * parent, const char * name , int f ) 
//    : data(new QWidgetPrivate)
//      ,_widget(0)
QWidget::QWidget( QWidget* parent, const char* name, int iiii )
    :
    BView(BRect( 0, 0, 200, 200 ), name, B_FOLLOW_ALL_SIDES,
	B_WILL_DRAW | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE ),
    data(new QWidgetPrivate())
{
    QOBJECT_TYPE(QWidget);

    // LEMON: put widget into parent just now

    if (parent) {
	/*
	QScrollView* scroll_view = dynamic_cast<QScrollView*>(parent);
	if (scroll_view) {
	    parent = scroll_view->viewport();
	    if (parent == NULL) parent = scroll_view;
	}
	*/
	parent->AddChild(this);
	float height = parent->Bounds().Height()/(parent->CountChildren());
	float width = parent->Bounds().Width();
	for (int i=0; i<parent->CountChildren(); i++ ) {
	    BView *v = parent->ChildAt( i );
	    if (v)
	    {
		v->ResizeTo( width, floorf(height) );
	        v->MoveTo( 0, ceilf(height*i) );
		v->Frame().PrintToStream();
	    }
	}
    }

    static QStyle defaultStyle;
    data->style = &defaultStyle;
    data->parent = parent;
    //data->widget = KWQRetain(view);
    data->visible = true;
    
}
/*
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
*/

QWidget::~QWidget() 
{
/*
    if (_widget) {
	// window of widget
	if (_widget->window) 
	    gdk_window_set_cursor(_widget->window, NULL); 	

	data->safe_widget = 0; // unrefs etc
    }
*/    
    delete data;
}

QSize QWidget::sizeHint()
{
    // May be overriden by subclasses.
    //ASSERT(_widget);
    /*
    GtkRequisition r;
    gtk_widget_size_request(_widget, &r);
    return QSize(r.width, r.height);
    */
    return( QSize( 0.0f, 0.0f ) );
}

void QWidget::resize(int w, int h) 
{
    //ASSERT(_widget);
    LOG(KwiqLog, "this:%x size:%d,%d", (int)this, w, h);
    ResizeTo( w - 1, h - 1 ); 
    /*
    QSize s(size());
    if (s.width()!=w || s.height()!=h)
	gtk_widget_set_size_request(_widget, w, h);
    */
}

void QWidget::setActiveWindow() 
{
// LEMON: Is that parent or what ?
//    KWQKHTMLPart::bridgeForWidget(this)->focusWindow();
}

void QWidget::setEnabled(bool enabled)
{
//    gtk_widget_set_sensitive(_widget, enabled);
}

bool QWidget::isEnabled()
{
//    return GTK_WIDGET_IS_SENSITIVE(_widget);
    return true;
}

long QWidget::winId()
{
    return (long)this;
}

int QWidget::x()
{
    //ASSERT(_widget);
    //return  _widget->allocation.x;
    int fx(0);
    if ( LockLooper() )
    {
	fx = int( Frame().left );
	UnlockLooper();
    }
    return( fx ); 
}

int QWidget::y()
{
    //ASSERT(_widget);
    //return _widget->allocation.y;
    int fy(0);
    if ( LockLooper() ) {
	fy = int( Frame().top );
	UnlockLooper();
    }
    return( fy ); 
}

int QWidget::width()
{ 
    //ASSERT(_widget);
    //return _widget->allocation.width;
    
    int fwidth(0);
    if( LockLooper() ) {
	fwidth = int(Frame().Width() + 1);
	UnlockLooper();
    }
    return( fwidth ); 
}

int QWidget::height()
{
    //ASSERT(_widget);
    //return _widget->allocation.height;
    
    int fheight(0);
    if( LockLooper() ) {
	fheight = int(Frame().Height() + 1);
	UnlockLooper();
    }
    return( fheight );  
}

QSize QWidget::size() 
{
    //return QSize(_widget->allocation.width, _widget->allocation.height);
    BRect frame;
    if( LockLooper() ) {
	frame = Frame();
	UnlockLooper();
    }
    return( QSize( frame.Width() + 1, frame.Height() + 1 ) ); 
}

void QWidget::resize(const QSize &s) 
{
    resize(s.width(), s.height());
}

QPoint QWidget::pos()
{
    //return QPoint(_widget->allocation.x, _widget->allocation.y);
    BRect frame;
    if( LockLooper() ) {
	frame = Frame();
	UnlockLooper();
    }
    return( QPoint( frame.left, frame.top ) ); 
}

void QWidget::move(int x, int y) 
{
/*
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
*/

    // LEMON: what to do with Layout ?
    if( LockLooper() ) {
	MoveTo( x, y );
	UnlockLooper();
    }  
}

void QWidget::move(const QPoint &p) 
{
    move(p.x(), p.y());
}

QRect QWidget::frameGeometry() 
{
    /*
    if (!_widget){
	g_warning("widget was zero");
	return QRect(-1,-1,-1,-1);
	ASSERT(_widget);
    }
    */
    BRect rect = Frame();
    return QRect(&rect);
}

void QWidget::setFrameGeometry(const QRect &rect)
{
/*
    LOG(KwiqLog, "this:%x, _widget==0, rect:(%d,%d; %d,%d)",
	(int)this, 
	rect.x(),
	rect.y(),
	rect.width(), 
	rect.height());

    ASSERT(_widget);
*/
    //g_object_freeze_notify(G_OBJECT (_widget));
    if (LockLooper()) {
        move(rect.x(), rect.y());
	resize(rect.width(), rect.height());
	UnlockLooper();
    }
    //g_object_thaw_notify(G_OBJECT (_widget));
}


int QWidget::baselinePosition(int height)
{
    return (int)((16.0f/20.0f)*height);
}

bool QWidget::hasFocus() 
{    
    //if (!_widget) return  false;
    //return GTK_WIDGET_HAS_FOCUS (_widget);
    return IsFocus();
}

void QWidget::setFocus()
{
    if (hasFocus()) {
        return;
    }
    
    MakeFocus();
/*
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
*/
}

void QWidget::clearFocus()
{
    if (!hasFocus()) {
        return;
    }

    //KWQKHTMLPart::clearDocumentFocus(this);
}

bool QWidget::checksDescendantsForFocus() 
{
    return false;
}

QWidget::FocusPolicy QWidget::focusPolicy() 
{
    // This provides support for controlling the widgets that take 
    // part in tab navigation. Widgets must:
    // 1. not be hidden by css
    // 2. be enabled
    // 3. accept first responder
/*
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
*/
    return TabFocus;
}

const QPalette& QWidget::palette() 
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

QStyle &QWidget::style() 
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

QFont QWidget::font() 
{
    return data->font;
}

void QWidget::setFont(const QFont &font)
{
    //g_warning("QWidget::setFont: size:%d, family:%s", font.pixelSize(), font.family().latin1());
    data->font = font;
    /*
    if (_widget){
	PangoFontDescription *fd =
        	createPangoFontDescription(&font);
	gtk_widget_modify_font(_widget, fd);
	pango_font_description_free(fd); //should be freed or not?
    }
    */
    
    BFont *ghm_font = (BFont *)createPangoFontDescription(&font);
    SetFont(ghm_font);
    
}

void QWidget::constPolish() 
{
}

bool QWidget::isVisible() 
{
/*
    if (!_widget) 
	return false;

    return GTK_WIDGET_VISIBLE(_widget);
*/
}

void QWidget::setCursor(const QCursor &cur)
{
/*
    if (!_widget) return;
    if (_widget->window) {
	gdk_window_set_cursor(_widget->window, cur.handle()); // NULL means parent
    }
*/
    data->currentCursor = cur;
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

bool QWidget::hasMouseTracking() 
{
    return true;
}

void QWidget::show()
{
/*
    if (!_widget) return;

    if (!data || isVisible())
        return;

    gtk_widget_show_all( _widget );
*/
    Show();
}

void QWidget::hide()
{
/*    
    if (!data || !isVisible())
        return;

    gtk_widget_hide( _widget );
*/
    Hide();
}

QPoint QWidget::mapFromGlobal(const QPoint &p) 
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

void QWidget::setBView(BView* widget)
{
    _widget = widget;
    data->safe_widget = widget;
}

/*
void QWidget::setGtkWidget(BView* widget)
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
*/

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

//void QWidget::paint(QPainter *p, const QRect &r)
//{
//    if (p->paintingDisabled()) {
//        return;
//    }
    
//    data->inPaint++;
    
    /*
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
    */
    
//    data->inPaint--;
//}

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

//void QWidget::setWritingDirection(QPainter::TextDirection direction)
//{
/*
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
	
    //pango_context_set_base_dir( pc, pdir );
*/
//}

// caller should destroy description
//PangoFontDescription* createPangoFontDescription(const QFont* font) 

BFont* createPangoFontDescription(const QFont* font)
{
    BFont* descr;
    descr = new BFont();
/*
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
 */ 
   return descr;
}

/*
extern "C" {

// @return \TRUE if stops focus in event propagation, \FALSE if propagation continues 
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

} 
*/
