/****************************************************************************
** $Id: qpicture.cpp,v 1.1.1.1 2002/04/27 11:02:20 procton Exp $
**
** Implementation of QPicture class
**
** Created : 940802
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qpicture.h"

#ifndef QT_NO_PICTURE

#include "qpainter.h"
#include "qpixmap.h"
#include "qimage.h"
#include "qfile.h"
#include "qdatastream.h"
#include "qpaintdevicemetrics.h"


// NOT REVISED
/*!
  \class QPicture qpicture.h
  \brief The QPicture class is a paint device that records and replays QPainter
  commands.

  \ingroup drawing

  A picture serializes painter commands to an IO device in a
  platform-independent format.	A picture created under Windows
  can be read on a Sun SPARC.

  Pictures are called meta-files on some platforms.

  Qt pictures use a proprietary binary format.	Unlike native picture
  (meta-file) formats on many window systems, Qt pictures have no
  limitations regarding the contents.  Everything that can be painted can
  also be stored in a picture (fonts, pixmaps, regions, transformed
  graphics etc.)

  Example of how to record a picture:
  \code
    QPicture  pic;
    QPainter  p;
    p.begin( &pic );				// paint in picture
    p.drawEllipse( 10,20, 80,70 );		// draw an ellipse
    p.end();					// painting done
    pic.save( "drawing.pic" );			// save picture
  \endcode

  Example of how to replay a picture:
  \code
    QPicture  pic;
    pic.load( "drawing.pic" );			// load picture
    QPainter  p;
    p.begin( &myWidget );			// paint in myWidget
    p.drawPicture( pic );			// draw the picture
    p.end();					// painting done
  \endcode

*/


static const char  *mfhdr_tag = "QPIC";		// header tag
static const Q_UINT16 mfhdr_maj = 3;		// major version #
static const Q_UINT16 mfhdr_min = 0;		// minor version #


/*!
  Constructs an empty picture.

  The \a formatVersion parameter may be used to create a QPicture that
  can be read by applications that are compiled with earlier versions of
  Qt. Currently supported is \a formatVersion == 1, which will make a
  QPicture which is stored in a format that is binary compatible with
  Qt 1.x programs, and \a formatVersion == 2, for a format compatible
  with Qt 2.0.x programs.

  Reading of pictures generated by earlier versions of Qt is supported and
  needs no special coding; the format is automatically detected.
*/

QPicture::QPicture( int formatVer )
    : QPaintDevice( QInternal::Picture | QInternal::ExternalDevice )	
    // set device type
{
    if ( formatVer != (int)mfhdr_maj ) {
	formatMajor = formatVer;
	formatMinor = 0;
	formatOk = FALSE;
    }
    else {
	resetFormat();
    }
}

/*!
  Destructs the picture.
*/
QPicture::~QPicture()
{
}


/*!
  \fn bool QPicture::isNull() const
  Returns TRUE if the picture contains no data, otherwise FALSE.
*/

/*!
  \fn uint QPicture::size() const
  Returns the size of the picture data.
  \sa data()
*/

/*!
  \fn const char* QPicture::data() const
  Returns a pointer to the picture data.  The returned pointer is null
  if the picture contains no data.
  \sa size(), isNull()
*/

/*!
  Sets the picture data directly from \a data and \a size. This function
  copies the input data.
  \sa data(), size()
*/

void QPicture::setData( const char* data, uint size )
{
    QByteArray a( size );
    memcpy( a.data(), data, size );
    pictb.setBuffer( a );			// set byte array in buffer
    resetFormat();				// we'll have to check
}


/*!
  Loads a picture from the file specified by \e fileName and returns TRUE
  if successful, otherwise FALSE.

  \sa save()
*/

bool QPicture::load( const QString &fileName )
{
    QByteArray a;
    QFile f( fileName );
    if ( !f.open(IO_ReadOnly) )
	return FALSE;
    a.resize( (uint)f.size() );
    f.readBlock( a.data(), (uint)f.size() );	// read file into byte array
    f.close();
    pictb.setBuffer( a );			// set byte array in buffer
    resetFormat();				// we'll have to check
    return TRUE;
}

/*!
  Saves a picture to the file specified by \e fileName and returns TRUE
  if successful, otherwise FALSE.

  \sa load()
*/

bool QPicture::save( const QString &fileName )
{
    QFile f( fileName );
    if ( !f.open( IO_WriteOnly ) )
	return FALSE;
    f.writeBlock( pictb.buffer().data(), pictb.buffer().size() );
    f.close();
    return TRUE;
}


/*!
  Replays the picture using \e painter and returns TRUE if successful, or
  FALSE if the internal picture data is inconsistent.

  This function does exactly the same as QPainter::drawPicture().
*/

bool QPicture::play( QPainter *painter )
{
    if ( pictb.size() == 0 )			// nothing recorded
	return TRUE;

    pictb.open( IO_ReadOnly );			// open buffer device
    QDataStream s;
    s.setDevice( &pictb );			// attach data stream to buffer

    if ( !formatOk ) {				// first time we read it
	char mf_id[4];				// picture header tag
	s.readRawBytes( mf_id, 4 );		// read actual tag
	if ( memcmp(mf_id, mfhdr_tag, 4) != 0 ) { // wrong header id
#if defined(CHECK_RANGE)
	    qWarning( "QPicture::play: Incorrect header" );
#endif
	    pictb.close();
	    return FALSE;
	}

	int cs_start = sizeof(Q_UINT32);		// pos of checksum word
	int data_start = cs_start + sizeof(Q_UINT16);
	Q_UINT16 cs,ccs;
	QByteArray buf = pictb.buffer();	// pointer to data
	s >> cs;				// read checksum
	ccs = qChecksum( buf.data() + data_start, buf.size() - data_start );
	if ( ccs != cs ) {
#if defined(CHECK_STATE)
	    qWarning( "QPicture::play: Invalid checksum %x, %x expected",
		     ccs, cs );
#endif
	    pictb.close();
	    return FALSE;
	}

	Q_UINT16 major, minor;
	s >> major >> minor;			// read version number
	if ( major > mfhdr_maj ) {		// new, incompatible version
#if defined(CHECK_RANGE)
	    qWarning( "QPicture::play: Incompatible version %d.%d",
		     major, minor);
#endif
	    pictb.close();
	    return FALSE;
	}
	formatOk = TRUE;			// picture seems to be ok
	formatMajor = major;
	formatMinor = minor;
    } else {
	s.device()->at( 10 );			// go directly to the data
    }

    s.setVersion( formatMajor );

    Q_UINT8  c, clen;
    Q_UINT32 nrecords;
    s >> c >> clen;
    if ( c == PdcBegin ) {
	s >> nrecords;
	if ( !exec( painter, s, nrecords ) )
	    c = 0;
    }
    if ( c !=  PdcBegin ) {
#if defined(CHECK_RANGE)
	qWarning( "QPicture::play: Format error" );
#endif
	pictb.close();
	return FALSE;
    }
    pictb.close();
    return TRUE;				// no end-command
}


/*!
  \internal
  Iterates over the internal picture data and draws the picture using
  \e painter.
*/

bool QPicture::exec( QPainter *painter, QDataStream &s, int nrecords )
{
#if defined(DEBUG)
    int		strm_pos;
#endif
    Q_UINT8	c;				// command id
    Q_UINT8	tiny_len;			// 8-bit length descriptor
    Q_INT32	len;				// 32-bit length descriptor
    Q_INT16	i_16, i1_16, i2_16;		// parameters...
    Q_INT8	i_8;
    Q_UINT32	ul;
    QCString	str1;
    QString	str;
    QPoint	p, p1, p2;
    QRect	r;
    QPointArray a;
    QColor	color;
    QFont	font;
    QPen	pen;
    QBrush	brush;
    QRegion	rgn;
#ifndef QT_NO_TRANSFORMATIONS
    QWMatrix	matrix;
#endif

    while ( nrecords-- && !s.eof() ) {
	s >> c;					// read cmd
	s >> tiny_len;				// read param length
	if ( tiny_len == 255 )			// longer than 254 bytes
	    s >> len;
	else
	    len = tiny_len;
#if defined(DEBUG)
	strm_pos = s.device()->at();
#endif
	switch ( c ) {				// exec cmd
	    case PdcNOP:
		break;
	    case PdcDrawPoint:
		s >> p;
		painter->drawPoint( p );
		break;
	    case PdcMoveTo:
		s >> p;
		painter->moveTo( p );
		break;
	    case PdcLineTo:
		s >> p;
		painter->lineTo( p );
		break;
	    case PdcDrawLine:
		s >> p1 >> p2;
		painter->drawLine( p1, p2 );
		break;
	    case PdcDrawRect:
		s >> r;
		painter->drawRect( r );
		break;
	    case PdcDrawRoundRect:
		s >> r >> i1_16 >> i2_16;
		painter->drawRoundRect( r, i1_16, i2_16 );
		break;
	    case PdcDrawEllipse:
		s >> r;
		painter->drawEllipse( r );
		break;
	    case PdcDrawArc:
		s >> r >> i1_16 >> i2_16;
		painter->drawArc( r, i1_16, i2_16 );
		break;
	    case PdcDrawPie:
		s >> r >> i1_16 >> i2_16;
		painter->drawPie( r, i1_16, i2_16 );
		break;
	    case PdcDrawChord:
		s >> r >> i1_16 >> i2_16;
		painter->drawChord( r, i1_16, i2_16 );
		break;
	    case PdcDrawLineSegments:
		s >> a;
		painter->drawLineSegments( a );
		break;
	    case PdcDrawPolyline:
		s >> a;
		painter->drawPolyline( a );
		break;
	    case PdcDrawPolygon:
		s >> a >> i_8;
		painter->drawPolygon( a, i_8 );
		break;
	    case PdcDrawQuadBezier:
		s >> a;
		painter->drawQuadBezier( a );
		break;
	    case PdcDrawText:
		s >> p >> str1;
		painter->drawText( p, str1 );
		break;
	    case PdcDrawTextFormatted:
		s >> r >> i_16 >> str1;
		painter->drawText( r, i_16, str1 );
		break;
	    case PdcDrawText2:
		s >> p >> str;
		painter->drawText( p, str );
		break;
	    case PdcDrawText2Formatted:
		s >> r >> i_16 >> str;
		painter->drawText( r, i_16, str );
		break;
	    case PdcDrawPixmap: {
		QPixmap pixmap;
		s >> p >> pixmap;
		painter->drawPixmap( p, pixmap );
		}
		break;
	    case PdcDrawImage: {
		QImage image;
		s >> p >> image;
		painter->drawImage( p, image );
		}
		break;
	    case PdcBegin:
		s >> ul;			// number of records
		if ( !exec( painter, s, ul ) )
		    return FALSE;
		break;
	    case PdcEnd:
		if ( nrecords == 0 )
		    return TRUE;
		break;
	    case PdcSave:
		painter->save();
		break;
	    case PdcRestore:
		painter->restore();
		break;
	    case PdcSetBkColor:
		s >> color;
		painter->setBackgroundColor( color );
		break;
	    case PdcSetBkMode:
		s >> i_8;
		painter->setBackgroundMode( (Qt::BGMode)i_8 );
		break;
	    case PdcSetROP:
		s >> i_8;
		painter->setRasterOp( (Qt::RasterOp)i_8 );
		break;
	    case PdcSetBrushOrigin:
		s >> p;
		painter->setBrushOrigin( p );
		break;
	    case PdcSetFont:
		s >> font;
		painter->setFont( font );
		break;
	    case PdcSetPen:
		s >> pen;
		painter->setPen( pen );
		break;
	    case PdcSetBrush:
		s >> brush;
		painter->setBrush( brush );
		break;
	    case PdcSetTabStops:
		s >> i_16;
		painter->setTabStops( i_16 );
		break;
	    case PdcSetTabArray:
		s >> i_16;
		if ( i_16 == 0 ) {
		    painter->setTabArray( 0 );
		} else {
		    int *ta = new int[i_16];
		    CHECK_PTR( ta );
		    for ( int i=0; i<i_16; i++ ) {
			s >> i1_16;
			ta[i] = i1_16;
		    }
		    painter->setTabArray( ta );
		    delete [] ta;
		}
		break;
	    case PdcSetVXform:
		s >> i_8;
#ifndef QT_NO_TRANSFORMATIONS
		painter->setViewXForm( i_8 );
#endif
		break;
	    case PdcSetWindow:
		s >> r;
#ifndef QT_NO_TRANSFORMATIONS
		painter->setWindow( r );
#endif
		break;
	    case PdcSetViewport:
		s >> r;
#ifndef QT_NO_TRANSFORMATIONS
		painter->setViewport( r );
#endif
		break;
	    case PdcSetWXform:
		s >> i_8;
#ifndef QT_NO_TRANSFORMATIONS
		painter->setWorldXForm( i_8 );
#endif
		break;
	    case PdcSetWMatrix:
#ifndef QT_NO_TRANSFORMATIONS	// #### fix me!
		s >> matrix >> i_8;
		painter->setWorldMatrix( matrix, i_8 );
#endif
		break;
#ifndef QT_NO_TRANSFORMATIONS
	    case PdcSaveWMatrix:
		painter->saveWorldMatrix();
		break;
	    case PdcRestoreWMatrix:
		painter->restoreWorldMatrix();
		break;
#endif
	    case PdcSetClip:
		s >> i_8;
		painter->setClipping( i_8 );
		break;
	    case PdcSetClipRegion:
		s >> rgn;
		painter->setClipRegion( rgn );
		break;
	    default:
#if defined(CHECK_RANGE)
		qWarning( "QPicture::play: Invalid command %d", c );
#endif
		if ( len )			// skip unknown command
		    s.device()->at( s.device()->at()+len );
	}
#if defined(DEBUG)
	//qDebug( "device->at(): %i, strm_pos: %i len: %i", s.device()->at(), strm_pos, len );
	ASSERT( s.device()->at() - strm_pos == len );
#endif
    }
    return FALSE;
}


/*!
  \internal
  Records painter commands and stores them in the pictb buffer.
*/

bool QPicture::cmd( int c, QPainter *, QPDevCmdParam *p )
{
    QDataStream s;
    s.setDevice( &pictb );
    s.setVersion( formatMajor );
    if ( c ==  PdcBegin ) {			// begin; write header
	QByteArray empty( 0 );
	pictb.setBuffer( empty );		// reset byte array in buffer
	pictb.open( IO_WriteOnly );
	s.writeRawBytes( mfhdr_tag, 4 );
	s << (Q_UINT16)0 << (Q_UINT16)formatMajor << (Q_UINT16)formatMinor;
	s << (Q_UINT8)c << (Q_UINT8)sizeof(Q_INT32);
	trecs = 0;
	s << (Q_UINT32)trecs;			// total number of records
	formatOk = FALSE;
	return TRUE;
    } else if ( c == PdcEnd ) {		// end; calc checksum and close
	trecs++;
	s << (Q_UINT8)c << (Q_UINT8)0;
	QByteArray buf = pictb.buffer();
	int cs_start = sizeof(Q_UINT32);		// pos of checksum word
	int data_start = cs_start + sizeof(Q_UINT16);
	int nrecs_start = data_start + 2*sizeof(Q_INT16) + 2*sizeof(Q_UINT8);
	int pos = pictb.at();
	pictb.at( nrecs_start );
	s << (Q_UINT32)trecs;			// write number of records
	pictb.at( cs_start );
	Q_UINT16 cs = (Q_UINT16)qChecksum( buf.data()+data_start, pos-data_start );
	s << cs;				// write checksum
	pictb.close();
	return TRUE;
    }
    trecs++;
    s << (Q_UINT8)c;				// write cmd to stream
    s << (Q_UINT8)0;				// write dummy length info
    int pos = (int)pictb.at();			// save position
    switch ( c ) {
	case PdcDrawPoint:
	case PdcMoveTo:
	case PdcLineTo:
	case PdcSetBrushOrigin:
	    s << *p[0].point;
	    break;
	case PdcDrawLine:
	    s << *p[0].point << *p[1].point;
	    break;
	case PdcDrawRect:
	case PdcDrawEllipse:
	    s << *p[0].rect;
	    break;
	case PdcDrawRoundRect:
	case PdcDrawArc:
	case PdcDrawPie:
	case PdcDrawChord:
	    s << *p[0].rect << (Q_INT16)p[1].ival << (Q_INT16)p[2].ival;
	    break;
	case PdcDrawLineSegments:
	case PdcDrawPolyline:
	case PdcDrawQuadBezier:
	    s << *p[0].ptarr;
	    break;
	case PdcDrawPolygon:
	    s << *p[0].ptarr << (Q_INT8)p[1].ival;
	    break;
	case PdcDrawText2:
	    if ( formatMajor == 1 ) {
		pictb.at( pos - 2 );
		s << (Q_UINT8)PdcDrawText << (Q_UINT8)0;
		QCString str1( (*p[1].str).latin1() );
		s << *p[0].point << str1;
	    }
	    else {
		s << *p[0].point << *p[1].str;
	    }
	    break;
	case PdcDrawText2Formatted:
	    if ( formatMajor == 1 ) {
		pictb.at( pos - 2 );
		s << (Q_UINT8)PdcDrawTextFormatted << (Q_UINT8)0;
		QCString str1( (*p[2].str).latin1() );
		s << *p[0].rect << (Q_INT16)p[1].ival << str1;
	    }
	    else {
		s << *p[0].rect << (Q_INT16)p[1].ival << *p[2].str;
	    }
	    break;
	case PdcDrawPixmap:
	    s << *p[0].point;
	    s << *p[1].pixmap;
	    break;
	case PdcDrawImage:
	    s << *p[0].point;
	    s << *p[1].image;
	    break;
	case PdcSave:
	case PdcRestore:
	    break;
	case PdcSetBkColor:
	    s << *p[0].color;
	    break;
	case PdcSetBkMode:
	case PdcSetROP:
	    s << (Q_INT8)p[0].ival;
	    break;
	case PdcSetFont:
	    s << *p[0].font;
	    break;
	case PdcSetPen:
	    s << *p[0].pen;
	    break;
	case PdcSetBrush:
	    s << *p[0].brush;
	    break;
	case PdcSetTabStops:
	    s << (Q_INT16)p[0].ival;
	    break;
	case PdcSetTabArray:
	    s << (Q_INT16)p[0].ival;
	    if ( p[0].ival ) {
		int *ta = p[1].ivec;
		for ( int i=0; i<p[0].ival; i++ )
		    s << (Q_INT16)ta[i];
	    }
	    break;
	case PdcSetUnit:
	case PdcSetVXform:
	case PdcSetWXform:
	case PdcSetClip:
	    s << (Q_INT8)p[0].ival;
	    break;
#ifndef QT_NO_TRANSFORMATIONS
	case PdcSetWindow:
	case PdcSetViewport:
	    s << *p[0].rect;
	    break;
	case PdcSetWMatrix:
	    s << *p[0].matrix << (Q_INT8)p[1].ival;
	    break;
#endif
	case PdcSetClipRegion:
	    s << *p[0].rgn;
	    break;
#if defined(CHECK_RANGE)
	default:
	    qWarning( "QPicture::cmd: Command %d not recognized", c );
#endif
    }
    int newpos = (int)pictb.at();		// new position
    int length = newpos - pos;
    if ( length < 255 ) {			// write 8-bit length
	pictb.at(pos - 1);			// position to right index
	s << (Q_UINT8)length;
    } else {					// write 32-bit length
	s << (Q_UINT32)0;				// extend the buffer
	pictb.at(pos - 1);			// position to right index
	s << (Q_UINT8)255;			// indicate 32-bit length
	char *p = pictb.buffer().data();
	memmove( p+pos+4, p+pos, length );	// make room for 4 byte
	s << (Q_UINT32)length;
	newpos += 4;
    }
    pictb.at( newpos );				// set to new position
    return TRUE;
}


/*!
  Internal implementation of the virtual QPaintDevice::metric() function.

  Use the QPaintDeviceMetrics class instead.

  A picture has the following hard coded values:
  width=640, height=480, dpi = 72, numcolors=16777216 and depth=24.
*/

int QPicture::metric( int m ) const
{
    int val;
    switch ( m ) {
	// ### ### ### ### ### ### hard coded value!!!
	case QPaintDeviceMetrics::PdmWidth:
	    val = 640;
	    break;
	case QPaintDeviceMetrics::PdmHeight:
	    val = 480;
	    break;
	case QPaintDeviceMetrics::PdmWidthMM:
	    val = 225;
	    break;
	case QPaintDeviceMetrics::PdmHeightMM:
	    val = 169;
	    break;
	case QPaintDeviceMetrics::PdmDpiX:
	    val = 72;
	    break;
	case QPaintDeviceMetrics::PdmDpiY:
	    val = 72;
	    break;
	case QPaintDeviceMetrics::PdmNumColors:
	    val = 16777216;
	    break;
	case QPaintDeviceMetrics::PdmDepth:
	    val = 24;
	    break;
	default:
	    val = 0;
#if defined(CHECK_RANGE)
	    qWarning( "QPicture::metric: Invalid metric command" );
#endif
    }
    return val;
}


/*****************************************************************************
  QPainter member functions
 *****************************************************************************/

/*!
  Replays the picture \e pic.

  This function does exactly the same as QPicture::play().
*/

void QPainter::drawPicture( const QPicture &pic )
{
    ((QPicture*)&pic)->play( (QPainter*)this );
}

/*!
  Makes this picture be a deep copy of \a p.
*/
QPicture& QPicture::operator= (const QPicture& p)
{
    setData(p.data(),p.size());
    return *this;
}


/*!
  \internal

  Sets formatOk to FALSE and resets the format version numbers to default
*/

void QPicture::resetFormat()
{
    formatOk = FALSE;
    formatMajor = mfhdr_maj;
    formatMinor = mfhdr_min;
}


/*****************************************************************************
  QPicture stream functions
 *****************************************************************************/

/*!
  \relates QPicture

  Writes a QPicture to the stream and returns a reference to the stream.
*/

QDataStream &operator<<( QDataStream &s, const QPicture &r )
{
    // just write the whole buffer to the stream
    return s.writeRawBytes ( r.pictb.buffer().data(), r.pictb.buffer().size() );
}

/*!
  \relates QPicture

  Reads a QPicture from the stream and returns a reference to the stream.
*/

QDataStream &operator>>( QDataStream &s, QPicture &r )
{
    Q_INT8 c;
    QDataStream sr;

    // "init"; this code is similar to the beginning of QPicture::cmd()
    sr.setDevice( &r.pictb );
    sr.setVersion( r.formatMajor );
    QByteArray empty( 0 );
    r.pictb.setBuffer( empty );
    r.pictb.open( IO_WriteOnly );

    // read all input and put it in the buffer (since it is a stream, I
    // don't know the size; so I have to loop until the end)
    while ( !s.atEnd() ) {
	s >> c;
	sr << c;
    }

    // "cleanup"
    r.pictb.close();
    r.resetFormat();

    return s;
}

#endif // QT_NO_PICTURE
