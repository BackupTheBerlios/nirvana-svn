#include <stdio.h>

#include <qpaintdevicemetrics.h>
#include <qscrollview.h>

#include <kdebug.h>

#include <math.h>

class QWidgetPrivate
{
public:
	int m_nHideCount;
};

QWidget::QWidget( QWidget* pcParent, const char* pzName )
	:BView( BRect( 0, 0, 200, 200 ), pzName, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_FRAME_EVENTS | B_FULL_UPDATE_ON_RESIZE )
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	d = new QWidgetPrivate;

	d->m_nHideCount = 1;
//	Show( false );
	
	if ( pcParent != NULL ) {
		QScrollView* pcSView = dynamic_cast<QScrollView*>(pcParent);
		if ( pcSView != NULL ) {
			pcParent = pcSView->viewport();
			if ( pcParent == NULL ) {
				pcParent = pcSView;
			}
		}
		pcParent->AddChild( this );
		
		float height = pcParent->Bounds().Height()/(pcParent->CountChildren());
		float width = pcParent->Bounds().Width();
		for( int i=0; i<pcParent->CountChildren(); i++ ){
			BView *v = pcParent->ChildAt( i );
			if( v ) {
				v->ResizeTo( width, floorf(height) );
				v->MoveTo( 0, ceilf(height*i) );
				v->Frame().PrintToStream();
			}
		}
		
	}
}

QWidget::~QWidget()
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	delete d;
}

void QWidget::clearFocus()
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	if( LockLooper() ) {
		MakeFocus( false );
		UnlockLooper();
	}
}

void QWidget::show()
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	if ( --d->m_nHideCount == 0 ) {
		if( LockLooper() ) {
			Show();
			UnlockLooper();
		}
	}
}

void QWidget::hide()
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	if ( d->m_nHideCount++ == 0 ) {
		if( LockLooper() ) {
			Hide();
			UnlockLooper();
		}
	}
}

void QWidget::setFocusProxy( QWidget * )
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	printf( "\tWarning: QWidget::%s() not implemented\n", __FUNCTION__ );
}

bool QWidget::focusNextPrevChild( bool )
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	printf( "\tWarning: QWidget::%s() not implemented\n", __FUNCTION__ );
	return( false );
}

QWidget* QWidget::focusWidget() const
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	printf( "\tWarning: QWidget::%s() not implemented\n", __FUNCTION__ );
	return( NULL );
}

QPoint QWidget::mapToGlobal( const QPoint& cPos )
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	BPoint cScrPos;
	if( LockLooper() ) {
		cScrPos = ConvertToScreen( BPoint( cPos.x(), cPos.y() ) );
		UnlockLooper();
	}
	return( QPoint( cScrPos.x, cScrPos.y ) );
}

QPoint QWidget::mapFromGlobal( const QPoint& cScrPos )
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	BPoint cPos;
	if( LockLooper() ) {
		cPos = ConvertFromScreen( BPoint( cScrPos.x(), cScrPos.y() ) );
		UnlockLooper();
	}
	return( QPoint( cPos.x, cPos.y ) );
}

QSize QWidget::sizeHint() const
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	return( QSize( 0.0f, 0.0f ) );
}

void QWidget::setFont( const QFont & )
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	printf( "\tWarning: QWidget::%s() not implemented\n", __FUNCTION__ );
}
	
void QWidget::resize( int w, int h )
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	ResizeTo( w - 1, h - 1 );
}
	
int QWidget::x()
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	int fx(0);
	if( LockLooper() ) {
		fx = int( Frame().left );
		UnlockLooper();
	}
	return( fx );
}

int QWidget::y()
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	int fy(0);
	if( LockLooper() ) {
		fy = int( Frame().top );
		UnlockLooper();
	}
	return( fy );
}
	
int QWidget::width()
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	int fwidth(0);
	if( LockLooper() ) {
		fwidth = int(Frame().Width() + 1);
		UnlockLooper();
	}
	return( fwidth );
}

int QWidget::height()
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	int fheight(0);
	if( LockLooper() ) {
		fheight = int(Frame().Height() + 1);
		UnlockLooper();
	}
	return( fheight );
}

QPoint QWidget::pos()
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	BRect cFrame;
	if( LockLooper() ) {
		cFrame = Frame();
		UnlockLooper();
	}
	return( QPoint( cFrame.left, cFrame.top ) );
}

void QWidget::move( const QPoint& cPos )
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	if( LockLooper() ) {
		MoveTo( cPos.x(), cPos.y() );
		UnlockLooper();
	}
}

QSize QWidget::size()
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	BRect cFrame;
	if( LockLooper() ) {
		cFrame = Frame();
		UnlockLooper();
	}
	return( QSize( cFrame.Width() + 1, cFrame.Height() + 1 ) );
}

void QWidget::resize( const QSize& cSize )
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	if( LockLooper() ) {
		ResizeTo( cSize.width() - 1, cSize.height() - 1 );
		UnlockLooper();
	}
}

void QWidget::erase()
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	if( LockLooper() ) {
		rgb_color oldColor = HighColor();
		SetHighColor( 0,0,0 );
		FillRect( Bounds() );
		SetHighColor( oldColor );
		Sync();
		UnlockLooper();
	}
//	printf( "Warning: QWidget::%s() not implemented\n", __FUNCTION__ );
}
/*
const QPalette& QWidget::palette() const
{
	printf( "Warning: QWidget::%s() not implemented\n", __FUNCTION__ );
	return( m_cPalette );
}
*/  
	
QWidget* QWidget::topLevelWidget()
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
//	printf( "Warning: QWidget::%s() not implemented\n", __FUNCTION__ );
	QWidget* pcParent = this;
	for (;;) {
		QWidget* pcTmp;
		if( LockLooper() ) {
			pcTmp = dynamic_cast<QWidget*>(pcParent->Parent());
			UnlockLooper();
		}
		if ( pcTmp == NULL ) {
			break;
		}
		pcParent = pcTmp;
	}
	return( pcParent );
}

void QWidget::setFocus()
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	if( LockLooper() ) {
		MakeFocus( true );
		UnlockLooper();
	}
}

bool QWidget::isTopLevel() const
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	printf( "\tWarning: QWidget::%s() not implemented\n", __FUNCTION__ );
	return 0;
}

bool QWidget::isEnabled() const
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	printf( "\tWarning: QWidget::%s() not implemented\n", __FUNCTION__ );
	return 1 ;
}

void QWidget::setMouseTracking( bool )
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	printf( "\tWarning: QWidget::%s() not implemented\n", __FUNCTION__ );
}

void QWidget::setBackgroundMode( BackgroundMode )
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	printf( "\tarning: QWidget::%s() not implemented\n", __FUNCTION__ );
}

void QWidget::setCursor( const QCursor& )
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	printf( "\tWarning: QWidget::%s() not implemented\n", __FUNCTION__ );
}
	
void QWidget::setFocusPolicy( FocusPolicy )
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	printf( "\tWarning: QWidget::%s() not implemented\n", __FUNCTION__ );
}

void QWidget::setAcceptDrops( bool /*on*/ )
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	printf( "\tWarning: QWidget::%s() not implemented\n", __FUNCTION__ );
}

void QWidget::focusInEvent( QFocusEvent * )
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	printf( "\tWarning: QWidget::%s() not implemented\n", __FUNCTION__ );
}

void QWidget::focusOutEvent( QFocusEvent * )
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	printf( "\tWarning: QWidget::%s() not implemented\n", __FUNCTION__ );
}

void QWidget::setEnabled( bool /*bEnabled*/ )
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	printf( "\tWarning: QWidget::%s() not implemented\n", __FUNCTION__ );
}

void QWidget::setAutoMask(bool)
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	printf( "\tWarning: QWidget::%s() not implemented\n", __FUNCTION__ );
}

void QWidget::repaint()
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	if( LockLooper() ) {
		Invalidate();
		Sync();
		UnlockLooper();
	}
}


int QWidget::metric( int id ) const
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	switch( id )
	{
	case QPaintDeviceMetrics::PdmWidth:
		return( int(Frame().Width()) );
	case QPaintDeviceMetrics::PdmHeight:
		return( int(Frame().Height()) );
	case QPaintDeviceMetrics::PdmWidthMM:
		printf( "Warning: QWidget::metric( PdmWidthMM ) not implemented\n" );
		return( 200 );
	case QPaintDeviceMetrics::PdmHeightMM:
		printf( "Warning: QWidget::metric( PdmHeightMM ) not implemented\n" );
		return( 200 );
	case QPaintDeviceMetrics::PdmNumColors:
		printf( "Warning: QWidget::metric( PdmNumColors ) not implemented\n" );
		return( 65536 );
	case QPaintDeviceMetrics::PdmDepth:
		printf( "Warning: QWidget::metric( PdmDepth ) not implemented\n" );
		return( 16 );
	case QPaintDeviceMetrics::PdmDpiX:
		printf( "Warning: QWidget::metric( PdmDpiX ) not implemented\n" );
		return( 72 );
	case QPaintDeviceMetrics::PdmDpiY:
		printf( "Warning: QWidget::metric( PdmDpiY ) not implemented\n" );
		return( 72 );
	default:
		printf( "Error: QWidget::metric( %d ) unknown ID\n", id );
		return( -1 );
	}
}


BPoint QWidget::GetPreferredSize( bool bLargest ) const
{
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	return( (bLargest) ? BPoint( 1000000.0f, 1000000.0f ) : BPoint( 0.0f, 0.0f ) );
}

void 
QWidget::FrameResized(float aWidth, float aHight)
{
	float height = Bounds().Height()/(CountChildren());
	float width = Bounds().Width();
	for( int i=0; i<CountChildren(); i++ ){
		BView *v = ChildAt( i );
		if( v ) {
			v->ResizeTo( width, floorf(height) );
			v->MoveTo( 0, ceilf(height*i) );
		}
	}
}

void 
QWidget::AttachedToWindow()
{
	printf( "\n\tQWidget::%s\n", __FUNCTION__ );
	kdDebug(10) << "QWidget::" << __FUNCTION__ << "()" << endl;
	BView::AttachedToWindow();
}

#include "awidget.moc"
