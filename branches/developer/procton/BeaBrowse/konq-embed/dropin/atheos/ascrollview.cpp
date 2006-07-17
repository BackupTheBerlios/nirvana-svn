#include <stdio.h>

#include <qpoint.h>
#include "ascrollview.h"
#include "apainter.h"

#include <ScrollBar.h>
#include <Message.h>

#include <kdebug.h>

enum { ID_VSCROLL, ID_HSCROLL };

class ViewPort : public QWidget
{
public:
	ViewPort( QScrollView* pcParent ) : QWidget( pcParent, "sv_viewport" ) { m_pcParent = pcParent; }

	virtual void Draw( const BRect& cUpdateRect )
	{
		QPainter cPainter( this );
		m_pcParent->drawContents( &cPainter, cUpdateRect.left, cUpdateRect.top, cUpdateRect.Width() + 1, cUpdateRect.Height() + 1 );
	}
private:
	QScrollView* m_pcParent;
	
};

class QScrollViewPrivate
{
public:
	ViewPort*			   m_pcViewPort;
	BScrollBar*		   m_pcHScrollBar;
	BScrollBar*		   m_pcVScrollBar;
	QScrollView::ScrollBarMode m_eVScrollBarMode;
	QScrollView::ScrollBarMode m_eHScrollBarMode;
	int				   m_nContentWidth;
	int				   m_nContentHeight;
};



QScrollView::QScrollView( QWidget* pcParent, const char* pzName, int /*flags*/ ) : QFrame( pcParent, pzName )
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	d = new QScrollViewPrivate;
	d->m_pcHScrollBar = NULL;
	d->m_pcVScrollBar = NULL;
	d->m_pcViewPort = NULL;
	d->m_pcViewPort = new ViewPort( this );
	d->m_pcViewPort->SetResizingMode( B_FOLLOW_ALL );
	d->m_pcViewPort->ResizeTo( Bounds().Width(), Bounds().Height() /*os::Rect( 0.0f, 0.0f, 100000.0f, 100000.0f )*/ );
	d->m_eHScrollBarMode = Auto;
	d->m_eVScrollBarMode = Auto;
	d->m_nContentWidth  = 0;
	d->m_nContentHeight = 0;
}

QScrollView::~QScrollView()
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	delete d;
}

QWidget* QScrollView::viewport()
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	return( d->m_pcViewPort );
}


void QScrollView::setContentsPos( int x, int y )
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	if( LockLooper() ){
		if ( x < 0 ) {
			x = 0;
		} else if ( x > contentsWidth() - visibleWidth() ) {
			x = contentsWidth() - visibleWidth();
		}
		if ( y < 0 ) {
			y = 0;
		} else if ( y > contentsHeight() - visibleHeight() ) {
			y = contentsHeight() - visibleHeight();
		}
		d->m_pcViewPort->ScrollTo( x, y );
		Flush();
		UpdateScrollBars();
		UnlockLooper();
	}
}

void QScrollView::resizeContents( int w, int h )
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	if( LockLooper() ) {
//	d->m_pcViewPort->ResizeTo( w - 1, h - 1 );
		d->m_nContentWidth  = w;
		d->m_nContentHeight = h;
		float x(0),y(0);
		if( d->m_pcHScrollBar )
			x = d->m_pcHScrollBar->Value();
		if( d->m_pcHScrollBar )
			y = d->m_pcVScrollBar->Value();
	
		if ( w - visibleWidth() ) {
			x = 0.0f;
		} else if ( x > w - visibleWidth() ) {
			x = w - visibleWidth();
		}
		
		if ( h < visibleHeight() ) {
			y = 0.0f;
		} else if ( y > h - visibleHeight() ) {
			y = h - visibleHeight();
		
		}
		d->m_pcViewPort->ScrollTo( x, y );
		Flush();
		UpdateScrollBars();
		UnlockLooper();
	}

}

void QScrollView::scrollBy( int x, int y )
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	if( LockLooper() ) {
		d->m_pcViewPort->ScrollBy( x, y );
		Flush();
		UpdateScrollBars();
		UnlockLooper();
	}
}

void QScrollView::contentsToViewport(int x, int y, int& vx, int& vy)
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	vx = x; // + contentsX();
	vy = y; // + contentsY();
}

void QScrollView::viewportToContents(int vx, int vy, int& x, int& y)
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	x = vx + contentsX();
	y = vy + contentsY();
}

int QScrollView::contentsX()
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	if( d->m_pcHScrollBar ) {
		int value(0);
		if( LockLooper() ) {
			value = int(d->m_pcHScrollBar->Value());
			UnlockLooper();
		}
		return( value );
	}
	return 0;
}

int QScrollView::contentsY()
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	if( d->m_pcVScrollBar ) {
		int value(0);
		if( LockLooper() ) {
			value = int(d->m_pcVScrollBar->Value());
			UnlockLooper();
		}
		return( value );
	}
	return 0;
}

int QScrollView::contentsWidth() const
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	return( d->m_nContentWidth );
//	return( int(d->m_pcViewPort->GetFrame().Width()) + 1 );
}

int QScrollView::contentsHeight() const
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	return( d->m_nContentHeight );
//	return( int(d->m_pcViewPort->GetFrame().Height()) + 1 );
}

void QScrollView::ensureVisible( int x, int y )
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	ensureVisible( x, y, 0, 0 );
}

void QScrollView::ensureVisible(int x, int y, int xmargin, int ymargin)
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	int nOldX = contentsX();
	int nOldY = contentsY();
	
	if ( x < nOldX + xmargin ) {
		nOldX = x - xmargin;
	}
	if ( x > nOldX + visibleWidth() - xmargin ) {
		nOldX = x - visibleWidth() + xmargin;
	}
	if ( y < nOldY + ymargin ) {
		nOldY = y - ymargin;
	}
	if ( y > nOldY + visibleHeight() - ymargin ) {
		nOldY = y - visibleHeight() + ymargin;
	}
	setContentsPos( nOldX, nOldY );
}

	
int QScrollView::visibleWidth()
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	int width(0);
	if( LockLooper() ) {
		width = int(d->m_pcViewPort->Frame().Width()) + 1 ;
		UnlockLooper();
	}
	return( width );
//	return( int(GetFrame().Width()) + 1 );
}

int QScrollView::visibleHeight()
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	int height(0);
	if( LockLooper() ) {
		height = int(d->m_pcViewPort->Frame().Height()) + 1 ;
		UnlockLooper();
	}
	return( height );
//	return( int(GetFrame().Height()) + 1 );
}

void QScrollView::setVScrollBarMode( ScrollBarMode eMode )
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	d->m_eVScrollBarMode = eMode;
	UpdateScrollBars();
}

void QScrollView::setHScrollBarMode( ScrollBarMode eMode )
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	d->m_eHScrollBarMode = eMode;
	UpdateScrollBars();
}

void QScrollView::setStaticBackground( bool )
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	printf( "Warning: QScrollView::%s() not implemented\n", __FUNCTION__ );
}

bool QScrollView::focusNextPrevChild( bool /*next*/ )
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	printf( "Warning: QScrollView::%s() not implemented\n", __FUNCTION__ );
	return( false );
}

void QScrollView::addChild( QWidget* child, int x, int y )
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	if( LockLooper() ){
		if ( child->Window() != Window() ) {	//??
			d->m_pcViewPort->AddChild( child );
		}
		child->MoveTo( x, y );
		UnlockLooper();
	}
}

void QScrollView::enableClipper( bool )
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	printf( "Warning: QScrollView::%s() not implemented\n", __FUNCTION__ );
}

void QScrollView::updateContents( int x, int y, int w, int h )
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	if( LockLooper() ){
		d->m_pcViewPort->Invalidate( BRect( x, y, x+w-1, y+h-1 ) );
		d->m_pcViewPort->Flush();
		UnlockLooper();
	}
}

void QScrollView::repaintContents( int x, int y, int w, int h, bool /*erase*/ )
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	QPainter cPainter( d->m_pcViewPort );

	drawContents( &cPainter, x, y, w, h );
}

void QScrollView::Draw( const BRect& cUpdateRect )
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	FillRect( cUpdateRect );
	QPainter cPainter( d->m_pcViewPort );
	drawContents( &cPainter, cUpdateRect.left, cUpdateRect.top, cUpdateRect.Width() + 1, cUpdateRect.Height() + 1 );
}

void QScrollView::UpdateScrollBars()
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	if( LockLooper() ) {	//A bit risky.
		if ( d->m_pcVScrollBar == NULL ) {
			if ( (d->m_eVScrollBarMode == Auto && visibleHeight() < contentsHeight()) || d->m_eVScrollBarMode == AlwaysOn ) {
				d->m_pcVScrollBar = new BScrollBar( BRect(0,0,B_V_SCROLL_BAR_WIDTH,0), "vscroll", this,  0, 0, B_VERTICAL );
				// new BMessage( ID_VSCROLL ),
				BRect cFrame = Bounds();
				if ( d->m_pcHScrollBar != NULL ) {
					cFrame.bottom -= d->m_pcHScrollBar->Bounds().Height() + 1;
				}
	//			d->m_pcVScrollBar->SetFrame( cFrame );
				AddChild( d->m_pcVScrollBar );
				if ( d->m_pcHScrollBar != NULL ) {
					d->m_pcHScrollBar->ResizeBy( -(cFrame.Width() + 1), 0 );
				}
				d->m_pcViewPort->ResizeBy( -(cFrame.Width() + 1), 0 );
			}
		} else {
			if ( (d->m_eVScrollBarMode == Auto && visibleHeight() >= contentsHeight()) || d->m_eVScrollBarMode == AlwaysOff ) {
				if ( d->m_pcHScrollBar != NULL ) {
					d->m_pcHScrollBar->ResizeBy( d->m_pcVScrollBar->Bounds().Width() + 1, 0 );
				}
				d->m_pcViewPort->ResizeBy( d->m_pcVScrollBar->Bounds().Width() + 1, 0 );
				RemoveChild( d->m_pcVScrollBar );
				delete d->m_pcVScrollBar;
				d->m_pcVScrollBar = NULL;
			}
		}
		if ( d->m_pcHScrollBar == NULL ) {
			if ( (d->m_eHScrollBarMode == Auto && visibleWidth() < contentsWidth()) || d->m_eHScrollBarMode == AlwaysOn ) {
				d->m_pcHScrollBar = new BScrollBar( BRect(0,0,0,B_H_SCROLL_BAR_HEIGHT), "hscroll", this,  0, 0, B_HORIZONTAL );
				BRect cFrame = Bounds();
				if ( d->m_pcVScrollBar != NULL ) {
					cFrame.right -= d->m_pcVScrollBar->Bounds().Width() + 1;
				}
	//			d->m_pcHScrollBar->SetFrame( cFrame );
				AddChild( d->m_pcHScrollBar );
				d->m_pcHScrollBar->SetTarget( this );
		
				if ( d->m_pcVScrollBar != NULL ) {
					d->m_pcVScrollBar->ResizeBy( 0, -(cFrame.Height() + 1) );
				}
				d->m_pcViewPort->ResizeBy( 0, -(cFrame.Height() + 1) );
			}
		} else {
			if ( (d->m_eHScrollBarMode == Auto && visibleWidth() >= contentsWidth()) || d->m_eHScrollBarMode == AlwaysOff ) {
				if ( d->m_pcVScrollBar != NULL ) {
					d->m_pcVScrollBar->ResizeBy( 0, d->m_pcHScrollBar->Bounds().Height() + 1 );
				}
				d->m_pcViewPort->ResizeBy( 0, d->m_pcHScrollBar->Bounds().Height() + 1 );
				RemoveChild( d->m_pcHScrollBar );
				delete d->m_pcHScrollBar;
				d->m_pcHScrollBar = NULL;
			}
		}
	
		if ( d->m_pcHScrollBar != NULL ) {
	//		d->m_pcHScrollBar->SetMinMax( 0, contentsWidth() - visibleWidth() );
			d->m_pcHScrollBar->SetValue( contentsX());
			d->m_pcHScrollBar->SetProportion( float(visibleWidth()) / float(contentsWidth()) );
			d->m_pcHScrollBar->SetSteps( 10.0f, float(visibleWidth()) * 0.8f );
		}
		if ( d->m_pcVScrollBar != NULL ) {
	//		d->m_pcVScrollBar->SetMinMax( 0, contentsHeight() - visibleHeight() );
			d->m_pcVScrollBar->SetValue( contentsY());
			d->m_pcVScrollBar->SetProportion( float(visibleHeight()) / float(contentsHeight()) );
			d->m_pcVScrollBar->SetSteps( 10.0f, float(visibleHeight()) * 0.8f );
		}
		UnlockLooper();
	}
}

#if 0
void QScrollView::HandleMessage( os::Message* pcMessage )
{
	switch( pcMessage->GetCode() )
	{
	case ID_HSCROLL:
		if ( d->m_pcHScrollBar != NULL ) {
		setContentsPos( d->m_pcHScrollBar->GetValue(), contentsY() );
		}
		break;
	case ID_VSCROLL:
		if ( d->m_pcVScrollBar != NULL ) {
		setContentsPos( contentsX(), d->m_pcVScrollBar->GetValue() );
		}
		break;
	}
}
#endif
/*
void QScrollView::WheelMoved( const BPoint& cDelta )
{
	scrollBy( 0, cDelta.y * 30.0f );
}
*/
BPoint QScrollView::GetPreferredSize( bool bLargest ) const
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	return( (bLargest) ? BPoint( 1000000.0f, 1000000.0f ) : BPoint( 0.0f, 0.0f ) );
}

void QScrollView::ScrollTo(BPoint point)
{
	kdDebug(10) << "QScrollView::" << __FUNCTION__ << "()" << endl;
	setContentsPos( point.x, point.y );	
}

#include "ascrollview.moc"
