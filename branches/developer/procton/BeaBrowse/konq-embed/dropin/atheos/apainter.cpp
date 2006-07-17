#include <stdio.h>
#include <assert.h>

#include <kdebug.h>

#include "apainter.h"
#include <qwidget.h>
#include "apixmap.h"

#include <Looper.h>
#include <Bitmap.h>
#include <Font.h>
#include <Shape.h>

#include <stack>

//debug
#include <OS.h>

#define BULLET_WIDHT  6
#define BULLET_HEIGHT 6
#if 0
uint8 g_anBullet8[] = {
	0xff,0xff,0xff,0x00,0x00,0xff,0xff,0xff,
	0xff,0xff,0x00,0x00,0x00,0x00,0xff,0xff,
	0xff,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0xff,0x00,0x00,0x00,0x00,0x00,0x00,0xff,
	0xff,0xff,0x00,0x00,0x00,0x00,0xff,0xff,
	0xff,0xff,0xff,0x00,0x00,0xff,0xff,0xff,
};
uint8 g_anBullet[] = {
	0xff,0x00,0x00,0x00,0x00,0xff,
	0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,
	0xff,0x00,0x00,0x00,0x00,0xff
};

static BBitmap* g_pcBulletBitmap = NULL;
#endif

class QPainterPrivate
{
public:
	QPainterPrivate()
	{
		m_nXOffset = 0;
		m_nYOffset = 0;
		m_eRasterOp = Qt::CopyROP;
	}
	QPaintDevice* m_pcDevice;
	BView*	  m_pcView;
	QFont		m_font;
	QPen	 m_pen;
	QBrush	 m_brush;
	Qt::RasterOp m_eRasterOp;
	float		 m_nXOffset;
	float		 m_nYOffset;
};
/*
QPainter::QPainter()
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "1()" << endl;
	d = new QPainterPrivate;
	d->m_pcDevice = NULL;
	d->m_pcView   = NULL;
}
*/
QPainter::QPainter( QWidget* pcWidget )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "2()" << endl;
	d = new QPainterPrivate;
	d->m_pcDevice = pcWidget;
	d->m_pcView   = pcWidget;
}

QPainter::QPainter( QPixmap* pcPixmap )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "3()" << endl;
	d = new QPainterPrivate;
	d->m_pcDevice = pcPixmap;
	d->m_pcView   = pcPixmap->GetView();
}

QPainter::~QPainter()
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	if( d->m_pcView != NULL ) {
		if( d->m_pcView->LockLooper() ) {
			d->m_pcView->Flush();
			d->m_pcView->UnlockLooper();
		}
	}
	delete d;
}
   
QFontMetrics QPainter::fontMetrics() const
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	return( QFontMetrics( d->m_font.GetFont() ) );
}

void QPainter::setFont( const QFont& cFont )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	d->m_font = cFont;
}

const QFont& QPainter::font() const
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	return( d->m_font );
}

bool QPainter::hasClipping() const
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	printf( "Warning:: QPainter::%s() not impelemented\n", __FUNCTION__ );
	return( false );
}

void QPainter::setClipping( bool )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	printf( "Warning:: QPainter::%s() not impelemented\n", __FUNCTION__ );
}
	
void QPainter::setPen( const QColor& cColor )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	if ( !d->m_pcView)
		return;
	
	if( d->m_pcView->LockLooper() ) {
		d->m_pcView->SetHighColor( cColor.red(), cColor.green(), cColor.blue() );
		d->m_pcView->UnlockLooper();
	}
}

void QPainter::setPen( PenStyle eStyle )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	if ( eStyle != 0 ) {
		printf( "Warning:: QPainter::%s:2( %d ) not impelemented\n", __FUNCTION__, eStyle );
	}
}

void QPainter::setPen( const QPen& cPen )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
//	printf( "Warning:: QPainter::%s:3() not impelemented\n", __FUNCTION__ );
		setPen( cPen.color() );
}

void QPainter::setBrush( const QColor& cColor )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "1()" << endl;
	if ( !d->m_pcView)
		return;

	if( d->m_pcView->LockLooper() ) {
		d->m_pcView->SetHighColor( cColor.red(), cColor.green(), cColor.blue() );
		d->m_pcView->UnlockLooper();
	}
	d->m_pen = QPen( cColor );
}

void QPainter::setBrush( BrushStyle )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "2()" << endl;
	printf("\tWarning: QPainter::setBrush() not implemented.\n");
}

void QPainter::setBrush( const QBrush& cBrush )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "3()" << endl;
	setBrush( cBrush.color() );
}
	
const QPen& QPainter::pen() const
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	if ( d->m_pcView) {
		rgb_color sCol;
		if( d->m_pcView->LockLooper() ) {
			sCol = d->m_pcView->HighColor();
			d->m_pcView->UnlockLooper();
		}
		d->m_pen = QPen( QColor( sCol.red, sCol.green, sCol.blue ) );
	}
	return( d->m_pen );
}
const QBrush& QPainter::brush() const
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	if ( d->m_pcView) {
		rgb_color sCol;
		if( d->m_pcView->LockLooper()) {
			sCol = d->m_pcView->HighColor();
			d->m_pcView->UnlockLooper();
		}
		d->m_brush = QBrush( QColor( sCol.red, sCol.green, sCol.blue ) );
	}
	return( d->m_brush );
}

void QPainter::setRasterOp( RasterOp eOp )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	d->m_eRasterOp = eOp;
	if ( !d->m_pcView)
		return;
	
	if( d->m_pcView->LockLooper() ) {
	
		switch( eOp )
		{
		case Qt::CopyROP:
			d->m_pcView->SetDrawingMode( B_OP_COPY );
			break;
		case Qt::XorROP:
			d->m_pcView->SetDrawingMode( B_OP_INVERT );
			break;
		default:
			printf( "Warning: QPainter::setRasterOp() unknown mode %d set\n", eOp );
			break;		
		}
		d->m_pcView->UnlockLooper();
	}		
}

Qt::RasterOp QPainter::rasterOp() const
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	return( d->m_eRasterOp );
}

void QPainter::save()
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	if( d->m_pcView->LockLooper() ) {
		d->m_pcView->PushState();
		d->m_pcView->UnlockLooper();
	}
}

void QPainter::restore()
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	if( d->m_pcView->LockLooper() ) {
		d->m_pcView->PopState();
		d->m_pcView->UnlockLooper();
	}
}
	
void QPainter::begin( QPixmap* pcPixmap )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	d->m_pcDevice = pcPixmap;
	d->m_pcView   = pcPixmap->GetView();
	if( d->m_pcView ) {
		d->m_pcView->LockLooper();
	}
}

void QPainter::end()
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	if ( d->m_pcView != NULL ) {
		d->m_pcView->Sync();
		if( d->m_pcView->Looper() && d->m_pcView->Looper()->IsLocked() ) {
			d->m_pcView->UnlockLooper();
		}
	}

}

QPaintDevice* QPainter::device() const
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	return( d->m_pcDevice );
}

void QPainter::setClipRect( const QRect & )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	printf( "Warning:: QPainter::%s() not impelemented\n", __FUNCTION__ );
}

void QPainter::scale( double sx, double sy )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	printf( "Warning:: QPainter::scale(%f, %f) not impelemented\n", sx, sy );
}

void QPainter::translate(float dx,float dy )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	d->m_nXOffset = dx;
	d->m_nYOffset = dy;
}

void QPainter::drawText( int x, int y, const QString& cString, int len )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "1()" << endl;
	if ( !d->m_pcView)
		return;
	kdDebug() << "QPainter::drawText1() View: " << d->m_pcView->Name()
		<< ",Looper: " << (d->m_pcView->Looper()? d->m_pcView->Looper()->Name(): "NULL")
		<< "." << endl;

	if( d->m_pcView->LockLooper() ) {

		drawing_mode eOldMode = d->m_pcView->DrawingMode();
		d->m_pcView->SetDrawingMode( B_OP_COPY );
		d->m_pcView->SetFont( d->m_font.GetFont()->f() );
	
		QCString cUtf8Str = cString.utf8();
		const char* pzStr = cUtf8Str.data();
		int nByteLen;
		if ( len == -1 ) {
			nByteLen = strlen( pzStr );
		} else {
			nByteLen = len;
		}
		
		printf("view %s ", d->m_pcView->Name()); d->m_pcView->Frame().PrintToStream();
		d->m_pcView->DrawString( pzStr, nByteLen, BPoint( x + d->m_nXOffset, y + d->m_nYOffset ) );
		d->m_pcView->SetDrawingMode( eOldMode );
		d->m_pcView->Sync();
		d->m_pcView->UnlockLooper();
	}
	
}

void QPainter::drawText( int x, int y, int w, int h, int flags,
			 const QString& cString, int len, QRect */*br=0*/,
			 char **/*internal*/ )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "2()" << endl;
	if ( !d->m_pcView)
		return;
	kdDebug() << "QPainter::drawText2() View: " << d->m_pcView->Name()
		<< ",Looper: " << (d->m_pcView->Looper()? d->m_pcView->Looper()->Name(): "NULL")
		<< "." << endl;
	
	font_height sHeight;
	d->m_font.GetFont()->f()->GetHeight( &sHeight );
	QRect cStrRect = QFontMetrics( d->m_font ).boundingRect( cString, len );
	if ( flags & Qt::AlignRight ) {
		x += w - cStrRect.width();
	}
	if ( h == 0 ) {
		y = y + int(sHeight.ascent);
	} else {
		y = y + h / 2 - cStrRect.height() / 2 - int(sHeight.ascent);
	}
	drawText( x, y, cString, len );
	
}

void QPainter::drawLine( int x1, int y1, int x2, int y2 )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	if ( !d->m_pcView)
		return;
	printf("QPainter::drawLine() View: %s, Looper: %s.\n", d->m_pcView->Name(), d->m_pcView->Looper()? d->m_pcView->Looper()->Name(): "NULL" );
	

	if( d->m_pcView->LockLooper() ){
		d->m_pcView->StrokeLine( BPoint( x1 + d->m_nXOffset, y1 + d->m_nYOffset ), BPoint( x2 + d->m_nXOffset, y2 + d->m_nYOffset ) );
		d->m_pcView->Sync();
		d->m_pcView->UnlockLooper();
	}
}

void QPainter::drawRect( int x, int y, int w, int h )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	if ( !d->m_pcView)
		return;
	printf("QPainter::drawRect() View: %s, Looper: %s.\n", d->m_pcView->Name(), d->m_pcView->Looper()? d->m_pcView->Looper()->Name(): "NULL" );
	
	
	if( d->m_pcView->LockLooper() ) {
		d->m_pcView->StrokeRect( BRect(x, y, x+w-1, y+h-1) );
		d->m_pcView->Sync();
		d->m_pcView->UnlockLooper();
	}
}

void QPainter::drawEllipse( int x, int y, int w, int h )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	if ( !d->m_pcView)
		return;
	printf("QPainter::drawEllipse() View: %s, Looper: %s.\n", d->m_pcView->Name(), d->m_pcView->Looper()? d->m_pcView->Looper()->Name(): "NULL" );
	
	if( d->m_pcView->LockLooper() ) {
		d->m_pcView->StrokeEllipse( BPoint(x,y), w, h );
		d->m_pcView->Sync();
		d->m_pcView->UnlockLooper();
	}
}

void QPainter::drawArc( int x, int y, int w, int h, int a, int alen )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	if ( !d->m_pcView)
		return;
	printf("QPainter::drawArc() View: %s, Looper: %s.\n", d->m_pcView->Name(), d->m_pcView->Looper()? d->m_pcView->Looper()->Name(): "NULL" );
	
	if( d->m_pcView->LockLooper() ) {	
		d->m_pcView->StrokeArc( BPoint(x,y), w, h, a, alen );
		d->m_pcView->Sync();
		d->m_pcView->UnlockLooper();
	}	
}

void QPainter::drawLineSegments( const QPointArray& cPoints, int index, int nlines )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	if ( !d->m_pcView)
		return;
	printf("QPainter::drawLineSegments() View: %s, Looper: %s.\n", d->m_pcView->Name(), d->m_pcView->Looper()? d->m_pcView->Looper()->Name(): "NULL" );
	
	
	if ( nlines == -1 ) {
		nlines = cPoints.count() - index;
	} else {
		nlines *= 2;
	}
	BShape shape;
	for ( int i = index ; i < nlines + index ; i += 2 ) {
		shape.MoveTo(BPoint( cPoints[i].x() + d->m_nXOffset, cPoints[i].y() + d->m_nYOffset ) );
		shape.LineTo( BPoint( cPoints[i+1].x() + d->m_nXOffset, cPoints[i+1].y() + d->m_nYOffset ) );
	}
	
	if( d->m_pcView->LockLooper() ) {
		d->m_pcView->StrokeShape( &shape );
		d->m_pcView->Sync();
		d->m_pcView->UnlockLooper();
	}
}

void QPainter::drawPolyline( const QPointArray& cPoints, int index, int npoints )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	if( !d->m_pcView)
		return;
	printf("QPainter::drawPolyline() View: %s, Looper: %s.\n", d->m_pcView->Name(), d->m_pcView->Looper()? d->m_pcView->Looper()->Name(): "NULL" );
	
	
	if ( npoints == -1 ) {
		npoints = cPoints.count() - index;
	}

	BShape shape;
	shape.MoveTo( BPoint(cPoints[index].x() + d->m_nXOffset, cPoints[index].y() + d->m_nYOffset) );
	for ( int i = index + 1 ; i < npoints + index ; ++i ) {
		shape.LineTo( BPoint( cPoints[i].x() + d->m_nXOffset, cPoints[i].y() + d->m_nYOffset ) );
	}
	
	if( d->m_pcView->LockLooper() ) {
		d->m_pcView->StrokeShape( &shape );
		d->m_pcView->Sync();
		d->m_pcView->UnlockLooper();
	}
}

void QPainter::drawPolygon( const QPointArray& cPoints, bool /*winding*/, int index, int npoints )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	if( d->m_pcView )
		return;
	printf("QPainter::drawPolygon() View: %s, Looper: %s.\n", d->m_pcView->Name(), d->m_pcView->Looper()? d->m_pcView->Looper()->Name(): "NULL" );
	
	if ( npoints == -1 ) {
		npoints = cPoints.count() - index;
	}

	BShape shape;
	shape.MoveTo( BPoint(cPoints[index].x() + d->m_nXOffset, cPoints[index].y() + d->m_nYOffset) );
	for ( int i = index + 1 ; i < npoints + index ; ++i ) {
//	printf( "%d: %d, %d\n", i, cPoints[i].x(), cPoints[i].y() );
		shape.LineTo( BPoint( cPoints[i].x() + d->m_nXOffset, cPoints[i].y() + d->m_nYOffset ) );
	}

	if( d->m_pcView->LockLooper() ) {
		d->m_pcView->StrokeShape( &shape );
		d->m_pcView->Sync();
		d->m_pcView->UnlockLooper();
	}
}


void QPainter::drawPixmap (int x, int y, const QPixmap& cPixmap, int sx, int sy, int sw, int sh )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "1()" << endl;
	if ( !d->m_pcView)
		return;
	printf("QPainter::drawPixmap() View: %s, Looper: %s.\n", d->m_pcView->Name(), d->m_pcView->Looper()? d->m_pcView->Looper()->Name(): "NULL" );
	
	BBitmap* pcBitmap = cPixmap.GetBitmap();
	if ( pcBitmap == NULL ) {
		return;
	}
//	pcBitmap->Sync();
	
	BRect source = pcBitmap->Bounds();
//	BRect source( sx, sy, sx+sw-1, sy+sh-1 );
//	source.OffsetBy(d->m_nXOffset,d->m_nYOffset);
	BRect dest(x, y, sx-x+sw-1, sy-y+sh-1 );
	dest.OffsetBy(d->m_nXOffset,d->m_nYOffset);
	drawing_mode eOldMode;

	if(d->m_pcView->LockLooper() ) {
		eOldMode = d->m_pcView->DrawingMode();
		if ( cPixmap.IsTransparent() ) {
			d->m_pcView->SetDrawingMode( B_OP_BLEND );
		} else {
			d->m_pcView->SetDrawingMode( B_OP_COPY );
		}
	
		printf("view %s ", d->m_pcView->Name()); d->m_pcView->Frame().PrintToStream();
		d->m_pcView->ResizeTo( 100, 100 );
		printf("view %s ", d->m_pcView->Name()); d->m_pcView->Frame().PrintToStream();
		dest=d->m_pcView->Bounds();
		
		d->m_pcView->DrawBitmap( pcBitmap, source, dest);
		d->m_pcView->SetDrawingMode( eOldMode );
		d->m_pcView->Sync();
		d->m_pcView->UnlockLooper();
	}
	
}

void QPainter::drawPixmap( const QPoint& cPos, const QPixmap& cPixmap )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "2()" << endl;
	drawPixmap( cPos.x(), cPos.y(), cPixmap, 0, 0, cPixmap.width(), cPixmap.height() );
}

void QPainter::drawPixmap( const QPoint& cPos, const QPixmap& cPixmap, const QRect &sr )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "3()" << endl;
	drawPixmap( cPos.x(), cPos.y(), cPixmap, sr.x(), sr.y(), sr.width(), sr.height() );
}

void QPainter::fillRect( int x, int y, int w, int h, const QColor& cColor )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
  if( !d->m_pcView )
		return;
	printf("QPainter::fillRect() View: %s, Looper: %s.\n", d->m_pcView->Name(), d->m_pcView->Looper()? d->m_pcView->Looper()->Name(): "NULL" );
		
	x += d->m_nXOffset;
	y += d->m_nYOffset;

	if( d->m_pcView->LockLooper() ) {
		rgb_color old = d->m_pcView->HighColor();
		d->m_pcView->SetHighColor( cColor.red(), cColor.green(), cColor.blue() );
		d->m_pcView->FillRect( BRect( x, y, x + w - 1, y + h - 1 ) );
		d->m_pcView->SetHighColor( old );
		d->m_pcView->Sync();
		d->m_pcView->UnlockLooper();
	}
}

void QPainter::drawTiledPixmap( int x, int y, int w, int h, const QPixmap& cPixmap, int sx, int sy )
{
	kdDebug(10) << "QPainter::" << __FUNCTION__ << "()" << endl;
	if ( !d->m_pcView)
		return;
	
	printf("QPainter::drawTiledPixmap() View: %s, Looper: %s.\n", d->m_pcView->Name(), d->m_pcView->Looper()? d->m_pcView->Looper()->Name(): "NULL" );

	BBitmap* pcBitmap = cPixmap.GetBitmap();
	if ( pcBitmap == NULL ) {
		return;
	}
	
	//pcBitmap->Sync();

	x += d->m_nXOffset;
	y += d->m_nYOffset;
	
	QSize cSize = cPixmap.size();
//	printf( "Draw %d, %d  (%d, %d)\n", w, h, cSize.width(), cSize.height() );

	drawing_mode eOldMode;
	if( d->m_pcView->LockLooper() ) {
		eOldMode = d->m_pcView->DrawingMode();

		if ( cPixmap.IsTransparent() ) {
			d->m_pcView->SetDrawingMode( B_OP_BLEND );
		} else {
			d->m_pcView->SetDrawingMode( B_OP_COPY );
		}
		d->m_pcView->UnlockLooper();
	}		
	sx = sx % cSize.width();
	sy = sy % cSize.height();
	
	for ( int yi = 0 ; ; ) {
		int _sx = sx;
		int nDeltaY = cSize.height() - sy;
		if ( yi + nDeltaY > h ) {
			nDeltaY = h - yi;
		}
		if ( nDeltaY <= 0 ) {
			break;
		}
		for ( int xi = 0 ; ; ) {
			int nDeltaX = cSize.width() - _sx;
			if ( xi + nDeltaX > w ) {
				nDeltaX = w - xi;
			}
			if ( nDeltaX <= 0 ) {
				break;
			}
			BRect cBounds( 0, 0, nDeltaX - 1, nDeltaY - 1 );
			
			if( d->m_pcView->LockLooper() ) {
				d->m_pcView->DrawBitmapAsync( pcBitmap, (cBounds.OffsetBySelf( BPoint( _sx, sy ))) & pcBitmap->Bounds(), cBounds.OffsetBySelf(BPoint( x + xi, y + yi )) );
				d->m_pcView->UnlockLooper();
			}
	
			_sx = 0;
			xi += nDeltaX;
		}
		yi += nDeltaY;
		sy = 0;
	}

	if( d->m_pcView->LockLooper() ) {
		d->m_pcView->SetDrawingMode( eOldMode );
//	d->m_pcView->Flush();
		d->m_pcView->Sync();
	}	
}





inline double qcos( double a )
{
	double r;
	__asm__ (
	"fcos"
	: "=t" (r) : "0" (a) );
	return(r);
}

inline double qsin( double a )
{
	double r;
	__asm__ (
	"fsin"
	: "=t" (r) : "0" (a) );
	return(r);
}

double qsincos( double a, bool calcCos=FALSE )
{
	return calcCos ? qcos(a) : qsin(a);
}




void qDrawShadePanel( QPainter *p, int x, int y, int w, int h,
			  const QColorGroup &g, bool sunken,
			  int lineWidth, const QBrush *fill )
{
	if ( w == 0 || h == 0 )
	return;
	if ( !( w > 0 && h > 0 && lineWidth >= 0 ) ) {
#if defined(CHECK_RANGE)
		qWarning( "qDrawShadePanel() Invalid parameters." );
#endif
	}
//	printf( "draw panel (%d,%d - %d,%d)\n", x, y, w, h );
	QPen oldPen = p->pen();			// save pen
	QPointArray a( 4*lineWidth );
	if ( sunken )
		p->setPen( g.dark() );
	else
		p->setPen( g.light() );
	int x1, y1, x2, y2;
	int i;
	int n = 0;
	x1 = x;
	y1 = y2 = y;
	x2 = x+w-2;
	for ( i=0; i<lineWidth; i++ ) {		// top shadow
		a.setPoint( n++, x1, y1++ );
		a.setPoint( n++, x2--, y2++ );
	}
	x2 = x1;
	y1 = y+h-2;
	for ( i=0; i<lineWidth; i++ ) {		// left shadow
		a.setPoint( n++, x1++, y1 );
		a.setPoint( n++, x2++, y2-- );
	}
	p->drawLineSegments( a );
	n = 0;
	if ( sunken )
		p->setPen( g.light() );
	else
		p->setPen( g.dark() );
	x1 = x;
	y1 = y2 = y+h-1;
	x2 = x+w-1;
	for ( i=0; i<lineWidth; i++ ) {		// bottom shadow
		a.setPoint( n++, x1++, y1-- );
		a.setPoint( n++, x2, y2-- );
	}
	x1 = x2;
	y1 = y;
	y2 = y+h-lineWidth-1;
	for ( i=0; i<lineWidth; i++ ) {		// right shadow
		a.setPoint( n++, x1--, y1++ );
		a.setPoint( n++, x2--, y2 );
	}
	p->drawLineSegments( a );
	if ( fill ) {				// fill with fill color
		QBrush oldBrush = p->brush();
		p->setPen( Qt::NoPen );
		p->setBrush( *fill );
		p->drawRect( x+lineWidth, y+lineWidth, w-lineWidth*2, h-lineWidth*2 );
		p->setBrush( oldBrush );
	}
	p->setPen( oldPen );			// restore pen
}
