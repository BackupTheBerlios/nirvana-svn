#include <stdio.h>

#include "apixmap.h"
#include <qpaintdevicemetrics.h>
#include <Bitmap.h>
#include <View.h>
#include "amutex.h"

#include <kdebug.h>

class QPixmapPrivate
{
public:
	BBitmap* m_pcBitmap;
	BView*	m_pcView;
	BRect	m_cValidRect;
	bool	m_bIsTranstarent;
};

QPixmap::QPixmap()
{
	kdDebug(10) << "QPixmap::" << __FUNCTION__ << "1()" << endl;
	d = new QPixmapPrivate;
	d->m_pcBitmap = NULL;
	d->m_pcView   = new BView( BRect( 0, 0, -1, -1 ), "bitmap_view", B_FOLLOW_LEFT|B_FOLLOW_TOP, 0 );
	d->m_cValidRect = BRect( 100000.0f, 100000.0f, 0.0f, 0.0f );
	d->m_bIsTranstarent = false;
}

QPixmap::QPixmap( const QPixmap& cOther )
{
	kdDebug(10) << "QPixmap::" << __FUNCTION__ << "2()" << endl;
	d = new QPixmapPrivate;
	d->m_pcBitmap = NULL;
	d->m_pcView   = new BView( BRect( 0, 0, -1, -1 ), "bitmap_view", B_FOLLOW_LEFT|B_FOLLOW_TOP, 0 );
	d->m_cValidRect = BRect( 100000.0f, 100000.0f, 0.0f, 0.0f );
	d->m_bIsTranstarent = false;
	*this = cOther;
}

QPixmap::QPixmap( int w, int h )
{
	kdDebug(10) << "QPixmap::" << __FUNCTION__ << "3()" << endl;
	d = new QPixmapPrivate;
	if ( w > 0 && h > 0 ) {
		d->m_pcBitmap = new BBitmap( BRect(0,0,w, h), B_RGB16, true );
	} else {
		d->m_pcBitmap = NULL;
	}
	d->m_pcView   = new BView( BRect( 0, 0, w - 1, h - 1 ), "bitmap_view", B_FOLLOW_LEFT|B_FOLLOW_TOP, 0 );
	if ( d->m_pcBitmap != NULL ) {
		d->m_pcBitmap->AddChild( d->m_pcView );
	}
	d->m_cValidRect = BRect( 100000.0f, 100000.0f, 0.0f, 0.0f );
	d->m_bIsTranstarent = false;
//	d->m_pcView->FillRect( d->m_pcView->GetBounds(), os::Color32_s( 255, 255, 255 ) );
}

QPixmap::QPixmap( BBitmap* pcBitmap )
{
	kdDebug(10) << "QPixmap::" << __FUNCTION__ << "4()" << endl;
	d = new QPixmapPrivate;
	d->m_pcBitmap = new BBitmap( pcBitmap->Bounds(), pcBitmap->ColorSpace(), true);
	d->m_pcView   = new BView( pcBitmap->Bounds(), "bitmap_view", B_FOLLOW_LEFT|B_FOLLOW_TOP, 0 );
	d->m_pcBitmap->AddChild( d->m_pcView );
	
	if( d->m_pcView->LockLooper() ) {
		d->m_pcView->DrawBitmap( d->m_pcBitmap, pcBitmap->Bounds() );
		d->m_pcView->Sync();
		d->m_pcView->UnlockLooper();
	}
	
	d->m_cValidRect = BRect( 100000.0f, 100000.0f, 0.0f, 0.0f );
	d->m_bIsTranstarent = false;
	
#warning Can cause problems. Should find another solution.
	delete pcBitmap; //?
	
}

QPixmap::~QPixmap()
{
	kdDebug(10) << "QPixmap::" << __FUNCTION__ << "()" << endl;
	delete d->m_pcBitmap;
	d->m_pcBitmap = (BBitmap*) 0x12345678;//??
	d->m_pcBitmap = 0;
	delete d;
}

QPixmap& QPixmap::operator=( const QPixmap& cOther )
{
	kdDebug(10) << "QPixmap::" << __FUNCTION__ << "()" << endl;
	d->m_bIsTranstarent = cOther.d->m_bIsTranstarent;
	d->m_cValidRect = cOther.d->m_cValidRect;
	if ( cOther.d->m_pcBitmap != NULL ) {
		resize( cOther.width(), cOther.height() );
	
		if ( d->m_pcBitmap != NULL ) {
			d->m_pcBitmap->RemoveChild( d->m_pcView );
			delete d->m_pcBitmap;
		}
		d->m_pcBitmap = new BBitmap( cOther.d->m_pcBitmap->Bounds(), cOther.d->m_pcBitmap->ColorSpace(), true);
		d->m_pcView->ResizeTo( cOther.width() - 1, cOther.height() - 1 );
		d->m_pcBitmap->AddChild( d->m_pcView );
		
		BRect cBounds = cOther.d->m_pcBitmap->Bounds();
	
		if( d->m_pcView->LockLooper() ) {
			d->m_pcView->SetDrawingMode( B_OP_COPY );
			d->m_pcView->DrawBitmap( cOther.d->m_pcBitmap, cBounds );
			d->m_pcView->Sync();
			d->m_pcView->UnlockLooper();
		}
	} else {
		if ( d->m_pcBitmap != NULL ) {
			d->m_pcBitmap->RemoveChild( d->m_pcView );
			delete d->m_pcBitmap;
			d->m_pcBitmap = NULL;
		}
	}
	return( *this );
}

QSize QPixmap::size() const
{
	kdDebug(10) << "QPixmap::" << __FUNCTION__ << "()" << endl;
	if ( d->m_pcBitmap != NULL ) {
		BRect cBounds = d->m_pcBitmap->Bounds();
		QSize cSize( cBounds.Width() + 1, cBounds.Height() + 1 );
		return( cSize );
	} else {
		return( QSize( 0, 0 ) );
	}
}

QRect QPixmap::rect() const
{
	kdDebug(10) << "QPixmap::" << __FUNCTION__ << "()" << endl;
	if ( d->m_pcBitmap != NULL ) {
		BRect cBounds = d->m_pcBitmap->Bounds();
		QRect cRect( QPoint( cBounds.left, cBounds.top ), QPoint( cBounds.right, cBounds.bottom ) );
		return( cRect );
	} else {
		return( QRect( 0, 0, 0, 0 ) );
	}
}

int QPixmap::width() const
{
	kdDebug(10) << "QPixmap::" << __FUNCTION__ << "()" << endl;
	return( size().width() );
}

int QPixmap::height() const
{
	kdDebug(10) << "QPixmap::" << __FUNCTION__ << "()" << endl;
	return( size().height() );
}

void QPixmap::resize( int w, int h )
{
	kdDebug(10) << "QPixmap::" << __FUNCTION__ << "()" << endl;
	QSize cOldSize = size();
	if ( cOldSize.width() == w && cOldSize.height() == h ) {
		return;
	}
	if ( d->m_pcBitmap != NULL ) {
		d->m_pcBitmap->RemoveChild( d->m_pcView );
		delete d->m_pcBitmap;
	}
	if ( w > 0 && h > 0 ) {
		d->m_pcBitmap = new BBitmap( BRect( 0,0, w, h), B_RGB16, true );
		d->m_pcView->ResizeTo( w - 1, h - 1 );
		d->m_pcBitmap->AddChild( d->m_pcView );
//		d->m_pcView->FillRect( d->m_pcView->GetBounds(), os::Color32_s( 255, 255, 255 ) );
	
	} else {
		d->m_pcBitmap = NULL;
	}
}

bool QPixmap::isNull() const
{
	kdDebug(10) << "QPixmap::" << __FUNCTION__ << "()" << endl;
	return( d->m_pcBitmap == NULL );
}
bool QPixmap::IsTransparent() const
{
	kdDebug(10) << "QPixmap::" << __FUNCTION__ << "()" << endl;
	return( d->m_bIsTranstarent );
}

void QPixmap::SetIsTransparent( bool bTrans )
{
	kdDebug(10) << "QPixmap::" << __FUNCTION__ << "()" << endl;
	d->m_bIsTranstarent = bTrans;
}

QBitmap* QPixmap::mask() const
{
	kdDebug(10) << "QPixmap::" << __FUNCTION__ << "()" << endl;
	printf( "\tWarning: QPixmap::%s() not implemented\n", __FUNCTION__ );
	return( 0 );
}

int QPixmap::metric( int id ) const
{
	kdDebug(10) << "QPixmap::" << __FUNCTION__ << "()" << endl;
	switch( id )
	{
	case QPaintDeviceMetrics::PdmWidth:
		printf( "Warning: QPixmap::metric( PdmWidth ) not implemented\n" );
		return( 200 );
	case QPaintDeviceMetrics::PdmHeight:
		printf( "Warning: QPixmap::metric( PdmHeight ) not implemented\n" );
		return( 200 );
	case QPaintDeviceMetrics::PdmWidthMM:
		printf( "Warning: QPixmap::metric( PdmWidthMM ) not implemented\n" );
		return( 200 );
	case QPaintDeviceMetrics::PdmHeightMM:
		printf( "Warning: QPixmap::metric( PdmHeightMM ) not implemented\n" );
		return( 200 );
	case QPaintDeviceMetrics::PdmNumColors:
		printf( "Warning: QPixmap::metric( PdmNumColors ) not implemented\n" );
		return( 65536 );
	case QPaintDeviceMetrics::PdmDepth:
		printf( "Warning: QPixmap::metric( PdmDepth ) not implemented\n" );
		return( 16 );
	case QPaintDeviceMetrics::PdmDpiX:
		printf( "Warning: QPixmap::metric( PdmDpiX ) not implemented\n" );
		return( 72 );
	case QPaintDeviceMetrics::PdmDpiY:
		printf( "Warning: QPixmap::metric( PdmDpiY ) not implemented\n" );
		return( 72 );
	default:
		printf( "Error: QPixmap::metric( %d ) unknown ID\n", id );
		return( -1 );
	}
}

void QPixmap::ExpandValidRect( const QRect& cRect ) const
{
	kdDebug(10) << "QPixmap::" << __FUNCTION__ << "()" << endl;
	d->m_cValidRect = d->m_cValidRect | BRect( cRect.left(), cRect.top(), cRect.right(), cRect.bottom() );
}

QRect QPixmap::GetValidRect() const
{
	kdDebug(10) << "QPixmap::" << __FUNCTION__ << "()" << endl;
	return( QRect( QPoint( d->m_cValidRect.left, d->m_cValidRect.top ), QPoint( d->m_cValidRect.right, d->m_cValidRect.bottom ) ) );
}

BView* QPixmap::GetView() const
{
	kdDebug(10) << "QPixmap::" << __FUNCTION__ << "()" << endl;
	return( d->m_pcView );
}

BBitmap* QPixmap::GetBitmap() const
{
	kdDebug(10) << "QPixmap::" << __FUNCTION__ << "()" << endl;
	return( d->m_pcBitmap );
}
