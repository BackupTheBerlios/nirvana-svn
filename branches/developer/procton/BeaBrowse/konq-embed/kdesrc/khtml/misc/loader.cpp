/*
    This file is part of the KDE libraries

    Copyright (C) 1998 Lars Knoll (knoll@mpi-hd.mpg.de)
    Copyright (C) 2000 Dirm Mueller (mueller@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

    This class provides all functionality needed for loading images, style sheets and html
    pages from the web. It has a memory cache for these objects.
*/

#undef CACHE_DEBUG
//#define CACHE_DEBUG

#include "loader.h"

// up to which size is a picture for sure cacheable
#define MAXCACHEABLE 40*1024
// default cache size
#define DEFCACHESIZE 512*1024
// maximum number of files the loader will try to load in parallel
#define MAX_REQUEST_JOBS 4


#include <qtextstream.h>
#ifndef __BEOS__
#include <qasyncio.h>
#include <qasyncimageio.h>
#else
#include <qrect.h>
#endif // __BEOS__
#include <qpainter.h>
#include <qbitmap.h>
#include <qmovie.h>

#include <kio/job.h>
#include <kio/jobclasses.h>
#include <kglobal.h>
#ifndef __BEOS__
#include <kimageio.h>
#endif
#include <kiconloader.h>
#include <kdebug.h>
#include "khtml_factory.h"

#include "css/css_stylesheetimpl.h"

#ifdef __BEOS__
#include <Bitmap.h>
#include <BitmapStream.h>
#include <TranslatorRoster.h>
#endif

using namespace khtml;
using namespace DOM;

void CachedObject::finish()
{
    if( m_size > MAXCACHEABLE )
    {
        m_status = Uncacheable;
        //Cache::flush(true); // Force flush.
    }
    else
        m_status = Cached;
    KURL url(m_url.string());
    if (m_expireDate && url.protocol().startsWith("http"))
    {
	KIO::http_update_cache(url, false, m_expireDate);
#ifdef CACHE_DEBUG
	kdDebug(6060) << " Setting expire date for image "<<m_url.string()<<" to " << m_expireDate << endl;
#endif
    }
#ifdef CACHE_DEBUG
    else kdDebug(6060) << " No expire date for image "<<m_url.string()<<endl;
#endif
}

void CachedObject::setExpireDate(int _expireDate)
{
    // assert(_expireDate);
    if ( _expireDate != m_expireDate && (m_status == Uncacheable || m_status == Cached))
    {
	finish();
    }
    m_expireDate = _expireDate;
}

void CachedObject::setRequest(Request *_request)
{
    m_request = _request;
    if (canDelete() && m_free)
        delete this;
}

// -------------------------------------------------------------------------------------------

CachedCSSStyleSheet::CachedCSSStyleSheet(const DOMString &url, const DOMString &baseURL, bool reload, int _expireDate)
    : CachedObject(url, CSSStyleSheet, reload, _expireDate)
{
    // It's css we want.
    setAccept( QString::fromLatin1("text/css") );
    // load the file
    Cache::loader()->load(this, baseURL, false);
    loading = true;
}

CachedCSSStyleSheet::~CachedCSSStyleSheet()
{
}

void CachedCSSStyleSheet::ref(CachedObjectClient *c)
{
    // make sure we don't get it twice...
    m_clients.remove(c);
    m_clients.append(c);

    if(!loading) c->setStyleSheet( m_url, m_sheet );
}

void CachedCSSStyleSheet::deref(CachedObjectClient *c)
{
    m_clients.remove(c);
    if ( canDelete() && m_free )
      delete this;
}

void CachedCSSStyleSheet::data( QBuffer &buffer, bool eof )
{
    if(!eof) return;
    m_size = buffer.buffer().size();

    buffer.close();
    buffer.open( IO_ReadOnly );
    QTextStream t( &buffer );
    QString data = t.read();

    m_sheet = DOMString(data);
    loading = false;

    checkNotify();
}

void CachedCSSStyleSheet::checkNotify()
{
    if(loading) return;

#ifdef CACHE_DEBUG
    kdDebug( 6060 ) << "CachedCSSStyleSheet:: finishedLoading " << m_url.string() << endl;
#endif

    CachedObjectClient *c;
    for ( c = m_clients.first(); c != 0; c = m_clients.next() )
        c->setStyleSheet( m_url, m_sheet );
}


void CachedCSSStyleSheet::error( int /*err*/, const char */*text*/ )
{
    loading = false;
    checkNotify();
}

// -------------------------------------------------------------------------------------------

CachedScript::CachedScript(const DOMString &url, const DOMString &baseURL, bool reload, int _expireDate)
    : CachedObject(url, Script, reload, _expireDate)
{
    // It's javascript we want.
    setAccept( QString::fromLatin1("application/x-javascript") );
    // load the file
    Cache::loader()->load(this, baseURL, false);
    loading = true;
}

CachedScript::~CachedScript()
{
}

void CachedScript::ref(CachedObjectClient *c)
{
    // make sure we don't get it twice...
    m_clients.remove(c);
    m_clients.append(c);

    if(!loading) c->notifyFinished(this);
}

void CachedScript::deref(CachedObjectClient *c)
{
    m_clients.remove(c);
    if ( canDelete() && m_free )
      delete this;
}

void CachedScript::data( QBuffer &buffer, bool eof )
{
    if(!eof) return;
    buffer.close();
    buffer.open( IO_ReadOnly );
    QTextStream t( &buffer );
    QString data = t.read();

    m_script = DOMString(data);
    loading = false;
    m_size = buffer.buffer().size();

    checkNotify();
}

void CachedScript::checkNotify()
{
    if(loading) return;

    CachedObjectClient *c;
    for ( c = m_clients.first(); c != 0; c = m_clients.next() ) {
        c->notifyFinished(this);
    }
}


void CachedScript::error( int /*err*/, const char */*text*/ )
{
    loading = false;
    checkNotify();
}

// ------------------------------------------------------------------------------------------

namespace khtml
{

  /*
     * Defines the DataSource for incremental loading of images.
     */
#ifndef __BEOS__    
    class ImageSource : public QDataSource
    {
    public:
        ImageSource(QByteArray buf);

        /**
         * Overload QDataSource::readyToSend() and returns the number
         * of bytes ready to send if not eof instead of returning -1.
         */
        int readyToSend();

        /*!
          Reads and sends a block of data.
        */
        void sendTo(QDataSink*, int count);

        /**
         * Sets the EOF state.
         */
        void setEOF( bool state );

        /*!
          KHTMLImageSource's is rewindable.
        */
        bool rewindable() const;

        /*!
          Enables rewinding.  No special action is taken.
        */
        void enableRewind(bool on);

        /*
          Calls reset() on the QIODevice.
        */
        void rewind();

        /*
          Indicates that the buffered data is no longer
          needed.
        */
        void cleanBuffer();

    private:
        QByteArray buffer;
        unsigned int pos;
        bool eof     : 1;
        bool rew     : 1;
        bool rewable : 1;
    };
#endif // __BEOS__    
}


/*!
  This Class defines the DataSource for incremental loading of images.
*/
#ifndef __BEOS__    
ImageSource::ImageSource(QByteArray buf)
{
  buffer = buf;
  rew = false;
  pos = 0;
  eof = false;
  rewable = true;
}

/**
 * Overload QDataSource::readyToSend() and returns the number
 * of bytes ready to send if not eof instead of returning -1.
*/
int ImageSource::readyToSend()
{
    if(eof && pos == buffer.size())
        return -1;

    return  buffer.size() - pos;
}

/*!
  Reads and sends a block of data.
*/
void ImageSource::sendTo(QDataSink* sink, int n)
{
    sink->receive((const uchar*)&buffer.at(pos), n);

    pos += n;

    // buffer is no longer needed
    if(eof && pos == buffer.size() && !rewable)
    {
        buffer.resize(0);
        pos = 0;
    }
}

/**
 * Sets the EOF state.
 */
void ImageSource::setEOF( bool state )
{
    eof = state;
}

// ImageSource's is rewindable.
bool ImageSource::rewindable() const
{
    return rewable;
}

// Enables rewinding.  No special action is taken.
void ImageSource::enableRewind(bool on)
{
    rew = on;
}

// Calls reset() on the QIODevice.
void ImageSource::rewind()
{
    pos = 0;
    if (!rew) {
        QDataSource::rewind();
    } else
        ready();
}


void ImageSource::cleanBuffer()
{
    // if we need to be able to rewind, buffer is needed
    if(rew)
        return;

    rewable = false;

    // buffer is no longer needed
    if(eof && pos == buffer.size())
    {
        buffer.resize(0);
        pos = 0;
    }
}
#endif // __BEOS__

static QString buildAcceptHeader()
{
#ifdef __BEOS__
    return( "" );
#else // __BEOS__
    QString result = KImageIO::mimeTypes( KImageIO::Reading ).join(", ");
    if (result.right(2) == ", ")
        result = result.left(result.length()-2);
    return result;
#endif // __BEOS__
}

// -------------------------------------------------------------------------------------

CachedImage::CachedImage(const DOMString &url, const DOMString &baseURL, bool reload, int _expireDate)
    : QObject(), CachedObject(url, Image, reload, _expireDate)
{
    static const QString &acceptHeader = KGlobal::staticQString( buildAcceptHeader() );

#ifndef __BEOS__
    m = 0;
#endif // __BEOS__
    p = 0;
    pixPart = 0;
    bg = 0;
#ifdef __BEOS__
	mMem = NULL;
#else    
    typeChecked = false;
#endif    
    isFullyTransparent = false;
    errorOccured = false;
    formatType = 0;
    m_status = Unknown;
    m_size = 0;
    imgSource = 0;
    m_baseURL = baseURL;
    setAccept( acceptHeader );

    if ( Cache::autoloadImages() )
        load();
}

CachedImage::~CachedImage()
{
    clear();
}

void CachedImage::ref( CachedObjectClient *c )
{
    //    kdDebug( 6060 ) << this << " CachedImage::ref(" << c << ") " << endl;

    // make sure we don't get it twice...
    m_clients.remove(c);
    m_clients.append(c);
#ifdef __BEOS__
//    printf( "Warning: CachedImage::ref() no support for movies\n" );
#else    
    if( m ) {
        m->unpause();
        if( m->finished() )
            m->restart();
    }
#endif
    // for mouseovers, dynamic changes
    if( m_status != Pending && !valid_rect().isNull())
        c->setPixmap( pixmap(), valid_rect(), this, 0L);

}

void CachedImage::deref( CachedObjectClient *c )
{
#ifdef CACHE_DEBUG
    kdDebug( 6060 ) << this << " CachedImage::deref(" << c << ") " << endl;
#endif
    m_clients.remove( c );
#ifndef __BEOS__
    if(m && m_clients.isEmpty() && m->running())
        m->pause();
#endif // __BEOS__

    if ( canDelete() && m_free )
        delete this;
}

//#define BGMINWIDTH      32
//#define BGMINHEIGHT     32
#define BGMINWIDTH      128
#define BGMINHEIGHT     128

const QPixmap &CachedImage::tiled_pixmap(const QColor& newc)
{
//    return pixmap();
    if (bg)
        return *bg;

    const QPixmap &r = pixmap();

    if (r.isNull()) return r;

    // no error indication for background images
    if(errorOccured) return *Cache::nullPixmap;

//    if ((r.width() < BGMINWIDTH) || (r.height() < BGMINHEIGHT) || newc != bgColor)
    {
        bool isvalid = newc.isValid();
        QSize s(pixmap_size());
        int w = ((BGMINWIDTH  / s.width())+1) * s.width();
        int h = ((BGMINHEIGHT / s.height())+1) * s.height();

//	printf( "COPY BG TO %d, %d\n", w, h );
        bg = new QPixmap(w, h);
        QPixmap pix = pixmap();
        QPainter p(bg);
        if(isvalid) p.fillRect(0, 0, w, h, newc);
        p.drawTiledPixmap(0, 0, w, h, pix);
        if(!isvalid && pix.mask())
        {
            // unfortunately our avoid transparency trick doesn't work here
            // we need to create a mask.

#ifdef __BEOS__
//	    printf( "Warning: CachedImage::tiled_pixmap() no support for QBitmap\n" );
#else
            QBitmap newmask(w, h);
            QPainter pm(&newmask);
            pm.drawTiledPixmap(0, 0, w, h, *pix.mask());
            bg->setMask(newmask);
#endif
        }

        return *bg;
    }

    return r;
}

const QPixmap &CachedImage::pixmap( ) const
{
    if(errorOccured)
        return *Cache::brokenPixmap;

#ifndef __BEOS__
    if(m)
    {
        if(m->framePixmap().size() != m->getValidRect().size() && m->getValidRect().size().isValid())
        {
            // pixmap is not yet completely loaded, so we
            // return a clipped version. asserting here
            // that the valid rect is always from 0/0 to fullwidth/ someheight
            if(!pixPart) pixPart = new QPixmap(m->getValidRect().size());

            (*pixPart) = m->framePixmap();
            pixPart->resize(m->getValidRect().size());
            return *pixPart;
        }
        else
            return m->framePixmap();
    }
    else if(p)
        return *p;
#else
    if(p)
        return *p;    
#endif
    return *Cache::nullPixmap;
}


QSize CachedImage::pixmap_size() const
{
#ifdef __BEOS__
    return p ? p->size() : QSize();
#else
    return (m ? m->framePixmap().size() : ( p ? p->size() : QSize()));
#endif
}


QRect CachedImage::valid_rect() const
{
#ifdef __BEOS__
//    printf( "Warning: CachedImage::valid_rect() no suport for movies\n" );
    return p ? p->GetValidRect() : QRect();
#else
    return m ? m->getValidRect() : ( p ? p->rect() : QRect());
#endif
}


void CachedImage::do_notify(const QPixmap& p, const QRect& r)
{
    // do not chang the hack with the update list unless you know what you are doing.
    // When removing this hack, directory listings as eg produced by apache will
    // get *really* slow
    QList<CachedObjectClient> updateList;
    CachedObjectClient *c;
    
    for ( c = m_clients.first(); c != 0; c = m_clients.next() ) {
#ifdef CACHE_DEBUG
        kdDebug( 6060 ) << "found a client to update..." << endl;
#endif
        bool manualUpdate = false; // set the pixmap, dont update yet.
        c->setPixmap( p, r, this, &manualUpdate);
        if (manualUpdate)
            updateList.append(c);
    }
    for ( c = updateList.first(); c != 0; c = updateList.next() ) {
        bool manualUpdate = true; // Update!
        // Actually we want to do c->updateSize()
        // This is a terrible hack which does the same.
        // updateSize() does not exist in CachecObjectClient only
        // in RenderBox()
        c->setPixmap( p, r, this, &manualUpdate);
    }
//    p = &_p;
//#ifdef __BEOS__
//        QSize s = pixmap_size();
//        m_size = s.width() * s.height() * 2;
//#endif    
}


void CachedImage::movieUpdated( const QRect& /*r*/ )
{
#ifdef CACHE_DEBUG
    qDebug("movie updated %d/%d/%d/%d, pixmap size %d/%d", r.x(), r.y(), r.right(), r.bottom(),
           m->framePixmap().size().width(), m->framePixmap().size().height());
#endif

#ifdef __BEOS__
//    printf( "CachedImage::movieUpdated() no support for movies\n" );
#else
    do_notify(m->framePixmap(), r);
#endif
}

void CachedImage::movieStatus(int /*status*/)
{
#ifdef CACHE_DEBUG
    //qDebug("movieStatus(%d)", status);
#endif
#ifdef __BEOS__
//    printf( "CachedImage::movieStatus() no support for movies\n" );
#else
    if((status == QMovie::EndOfFrame) || (status == QMovie::EndOfMovie))
    {
        // just another Qt 2.2.0 bug. we cannot call
        // QMovie::frameImage if we're after QMovie::EndOfMovie
        if(status == QMovie::EndOfFrame)
        {
            const QImage& im = m->frameImage();
            if(im.width() < 5 && im.height() < 5 && im.hasAlphaBuffer()) // only evaluate for small images
            {
                QImage am = im.createAlphaMask();
                if(am.depth() == 1)
                {
                    bool solid = false;
                    for(int y = 0; y < am.height(); y++)
                        for(int x = 0; x < am.width(); x++)
                            if(am.pixelIndex(x, y)) {
                                solid = true;
                                break;
                            }

                    isFullyTransparent = (!solid);
                }

            }

            // we have to delete our tiled bg variant here
            // because the frame has changed (in order to keep it in sync)
            delete bg;
            bg = 0;
        }

        if(status == QMovie::EndOfMovie)
        {
            // the movie has ended and it doesn't loop nor is it an animation,
            // so there is no need to keep the buffer in memory
            if(imgSource && m->frameNumber() == 1)
            {
                imgSource->cleanBuffer();
                delete p;
                p = new QPixmap(m->framePixmap());
                m->disconnectUpdate( this, SLOT( movieUpdated( const QRect &) ));
                m->disconnectStatus( this, SLOT( movieStatus(int)));
                delete m;
                m = 0;
                imgSource = 0;
            }
        }

#ifdef CACHE_DEBUG
//        QRect r(valid_rect());
//        qDebug("movie Status frame update %d/%d/%d/%d, pixmap size %d/%d", r.x(), r.y(), r.right(), r.bottom(),
//               pixmap().size().width(), pixmap().size().height());
#endif
            do_notify(pixmap(), valid_rect());
    }
#endif // __BEOS__
}

void CachedImage::clear()
{
#ifndef __BEOS__
    delete m;   m = 0;
#endif
    delete p;   p = 0;
    delete bg;  bg = 0;
    delete pixPart; pixPart = 0;

    formatType = 0;
#ifdef __BEOS__
    delete mMem; mMem = 0;
#else    
    typeChecked = false;
#endif    
    m_size = 0;

    // No need to delete imageSource - QMovie does it for us
    imgSource = 0;
}

#ifdef __BEOS__
#define min(a,b) (((a)<(b)) ? (a) : (b))
void CachedImage::data ( QBuffer &_buffer, bool eof )
{
#warning CachedImage::data not complete.
#ifdef CACHE_DEBUG
    kdDebug( 6060 ) << "in CachedImage::data(buffersize " << _buffer.buffer().size() <<", eof=" << eof << endl;
#endif
//    if ( eof == false && _buffer.size() - last_byte_added < 1024 ) {
//			return;
//    }
  if ( mMem == NULL ) {
		mMem = new BMallocIO();
		mMem->SetBlockSize( 512 );	//For no specific reason.
		mMem->Seek( 0, SEEK_SET );
	}
	mMem->Write( _buffer.buffer().data(), _buffer.size() );
	
	if( !eof )
		return;
		
	BTranslatorRoster* roster = BTranslatorRoster::Default();
	BBitmapStream stream;
	BBitmap *result; 
	
	if( roster->Translate( mMem, NULL, NULL, &stream, B_TRANSLATOR_BITMAP) != B_OK )
		return;
	
	stream.DetachBitmap(&result);
  delete mMem; mMem = 0;
	
	if( p )
		delete p;
	
	p = new QPixmap( result );
	do_notify( *p, p->GetValidRect() );
//Need to do more here, I guess.
}
#if 0	
			if ( nError >= 0 ) {
			    translator->AddData( (const void*)_buffer.buffer().data(), _buffer.size(), eof );
			    last_byte_added = _buffer.size();
			} else {
			    if ( eof ) {
						error(0,"");
			    }
			    return;
			}
    }
    translator->AddData( ((const char*)_buffer.buffer().data()) + last_byte_added, _buffer.size() - last_byte_added, eof );
    last_byte_added = _buffer.size();
    if ( p == NULL ) {
				os::BitmapHeader sBmHeader;
			if ( translator->Read( &sBmHeader, sizeof(sBmHeader) ) != sizeof( sBmHeader ) ) {
			    return;
			}
			os::Bitmap* pcBitmap = new os::Bitmap( sBmHeader.bh_bounds.Width() + 1, sBmHeader.bh_bounds.Height() + 1, os::CS_RGB32 );
			memset( pcBitmap->LockRaster(), -1, pcBitmap->GetBytesPerRow() * int(pcBitmap->GetBounds().Height() + 1) );
			p = new QPixmap( pcBitmap );
			p->SetIsTransparent( true );
    }

    int nError = 1;

    int nStartY = 100000;
    int nEndY   = 0;
    
    enum {BUFFER_SIZE=1024*32};
    uint8 anBuffer[int(BUFFER_SIZE)];
    
    for ( ;; ) {
			nError = translator->Read( anBuffer, BUFFER_SIZE );
			if ( nError <= 0 ) {
			    break;
			}
			uint nBytesInBuffer = nError;
			int nOffset = 0;
			while ( nBytesInBuffer > 0 ) {
			    if ( m_nHeaderBytesRead == 0 || m_nHeaderBytesRead < sizeof(m_sCurrentFrame) ) {
						int nCurLen = min( sizeof(m_sCurrentFrame) - m_nHeaderBytesRead, nBytesInBuffer );
						memcpy( ((char*)&m_sCurrentFrame) + m_nHeaderBytesRead, anBuffer + nOffset, nCurLen );
						m_nHeaderBytesRead += nCurLen;
						nBytesInBuffer     -= nCurLen;
						nOffset            += nCurLen;
			    }
			    uint8* pDstRaster = p->GetBitmap()->LockRaster();
			    if ( m_nHeaderBytesRead == sizeof(m_sCurrentFrame) ) {
						while( m_nBytesAddedToFrame < m_sCurrentFrame.bf_data_size ) {
						    int x = m_nBytesAddedToFrame % m_sCurrentFrame.bf_bytes_per_row;
						    int y = m_nBytesAddedToFrame / m_sCurrentFrame.bf_bytes_per_row;
						    y += m_sCurrentFrame.bf_frame.top;
						    if ( y < nStartY ) {
									nStartY = y;
						    }
						    if ( y > nEndY ) {
									nEndY = y;
						    }
							
						    int nCurLen = min( nBytesInBuffer, m_sCurrentFrame.bf_bytes_per_row - x );
						    if ( nCurLen <= 0 ) {
									break;
						    }
						    nBytesInBuffer -= nCurLen;
						    if ( y >= 0 && y <= p->GetBitmap()->GetBounds().bottom ) {
									memcpy( pDstRaster + y * p->GetBitmap()->GetBytesPerRow() + x, anBuffer + nOffset, nCurLen );
						    }
						    nOffset += nCurLen;
						    m_nBytesAddedToFrame += nCurLen;
						}
						if ( m_nBytesAddedToFrame == m_sCurrentFrame.bf_data_size ) {
						    m_nHeaderBytesRead = 0;
						    m_nBytesAddedToFrame = 0;
						}
			    } else {
						break;
			    }
			}
    }

    if ( eof ) {
			if ( translator == NULL ) {
			    error(0,"");
			} else {
			    delete translator;
			    translator = NULL;
			}
    }
    
    if ( nStartY <= nEndY ) {
			p->ExpandValidRect( QRect( QPoint( m_sCurrentFrame.bf_frame.left, nStartY ), QPoint( m_sCurrentFrame.bf_frame.right, nEndY ) ) );
		
			do_notify( *p, p->GetValidRect() );
    }
}
#endif //0

void CachedImage::error( int /*err*/, const char */*text*/ )
{
#ifdef CACHE_DEBUG
    kdDebug(6060) << "CahcedImage::error" << endl;
#endif

    clear();
#ifndef __BEOS__    
    typeChecked = true;
#endif    
    errorOccured = true;
    do_notify(pixmap(), QRect(0, 0, 16, 16));
}

#else // __BEOS__

void CachedImage::data ( QBuffer &_buffer, bool eof )
{
#ifdef CACHE_DEBUG
    kdDebug( 6060 ) << "in CachedImage::data(buffersize " << _buffer.buffer().size() <<", eof=" << eof << endl;
#endif
    if ( !typeChecked )
    {
        formatType = QImageDecoder::formatName( (const uchar*)_buffer.buffer().data(), _buffer.size());
        typeChecked = true;

        if ( formatType )  // movie format exists
        {
            imgSource = new ImageSource( _buffer.buffer());
            m = new QMovie( imgSource, 1024);
            m->connectUpdate( this, SLOT( movieUpdated( const QRect &) ));
            m->connectStatus( this, SLOT( movieStatus(int)));
        }
    }

    if ( imgSource )
    {
        imgSource->setEOF(eof);
        imgSource->maybeReady();
    }

    if(eof)
    {
        // QMovie currently doesn't support all kinds of image formats
        // so we need to use a QPixmap here when we finished loading the complete
        // picture and display it then all at once.
        if(typeChecked && !formatType)
        {
#ifdef CACHE_DEBUG
            kdDebug(6060) << "CachedImage::data(): reloading as pixmap:" << endl;
#endif
            p = new QPixmap( _buffer.buffer() );
            // set size of image.
#ifdef CACHE_DEBUG
            kdDebug(6060) << "CachedImage::data(): image is null: " << p->isNull() << endl;
#endif
                if(p->isNull())
                {
                    errorOccured = true;
                    do_notify(pixmap(), QRect(0, 0, 16, 16)); // load "broken image" icon
                }
                else
                    do_notify(*p, p->rect());
        }

        QSize s = pixmap_size();
        m_size = s.width() * s.height() * 2;
    }
}


void CachedImage::error( int /*err*/, const char */*text*/ )
{
#ifdef CACHE_DEBUG
    kdDebug(6060) << "CahcedImage::error" << endl;
#endif

    clear();
    typeChecked = true;
    errorOccured = true;
    do_notify(pixmap(), QRect(0, 0, 16, 16));
}

#endif // __BEOS__




void CachedImage::load()
{
    Cache::loader()->load(this, m_baseURL, true);
}

// ------------------------------------------------------------------------------------------
#if 0
Request::Request(CachedObject *_object, const DOM::DOMString &baseURL, bool _incremental)
{
    object = _object;
    object->setRequest(this);
    incremental = _incremental;
    m_baseURL = baseURL;
}

Request::~Request()
{
    object->setRequest(0);
}
#endif
// ------------------------------------------------------------------------------------------

DocLoader::DocLoader()
{
  reloading = false;
  m_expireDate = 0;
}

DocLoader::~DocLoader()
{

}

void DocLoader::setExpireDate(int _expireDate)
{
    m_expireDate = _expireDate;
}

CachedImage *DocLoader::requestImage( const DOM::DOMString &url, const DOM::DOMString &baseUrl)
{
    if (reloading) {
        QString fullURL = Cache::completeURL( url, baseUrl ).url();
        if (!reloadedURLs.contains(fullURL)) {
            CachedObject *existing = Cache::cache->find(fullURL);
            if (existing)
                Cache::removeCacheEntry(existing);
	    reloadedURLs.append(fullURL);
            return Cache::requestImage(url,baseUrl,true,m_expireDate);
        }
    }

    return Cache::requestImage(url,baseUrl,false, m_expireDate);
}

CachedCSSStyleSheet *DocLoader::requestStyleSheet( const DOM::DOMString &url, const DOM::DOMString &baseUrl)
{
    if (reloading) {
        QString fullURL = Cache::completeURL( url, baseUrl ).url();
        if (!reloadedURLs.contains(fullURL)) {
            CachedObject *existing = Cache::cache->find(fullURL);
            if (existing)
                Cache::removeCacheEntry(existing);
	    reloadedURLs.append(fullURL);
            return Cache::requestStyleSheet(url,baseUrl,true,m_expireDate);
        }
    }

    return Cache::requestStyleSheet(url,baseUrl,false,m_expireDate);
}

CachedScript *DocLoader::requestScript( const DOM::DOMString &url, const DOM::DOMString &baseUrl)
{
    if (reloading) {
        QString fullURL = Cache::completeURL( url, baseUrl ).url();
        if (!reloadedURLs.contains(fullURL)) {
            CachedObject *existing = Cache::cache->find(fullURL);
            if (existing)
                Cache::removeCacheEntry(existing);
	    reloadedURLs.append(fullURL);
            return Cache::requestScript(url,baseUrl,true,m_expireDate);
        }
    }

    return Cache::requestScript(url,baseUrl,false,m_expireDate);
}


// ------------------------------------------------------------------------------------------
#ifndef __BEOS__
Loader::Loader() : QObject()
{
    m_requestsPending.setAutoDelete( true );
    m_requestsLoading.setAutoDelete( true );
}

Loader::~Loader()
{
}

void Loader::load(CachedObject *object, const DOMString &baseURL, bool incremental)
{
    Request *req = new Request(object, baseURL, incremental);
    m_requestsPending.append(req);

    servePendingRequests();
}

void Loader::servePendingRequests()
{
  if ( m_requestsLoading.count() >= MAX_REQUEST_JOBS )
    return;
  if ( m_requestsPending.count() == 0 )
      return;

  // get the first pending request
  Request *req = m_requestsPending.take(0);

#ifdef CACHE_DEBUG
  kdDebug( 6060 ) << "starting Loader url=" << req->object->url().string() << endl;
#endif

  KIO::TransferJob* job = KIO::get( req->object->url().string(), req->object->reload(), false /*no GUI*/);

  if (!req->object->accept().isEmpty())
     job->addMetaData("accept", req->object->accept());
  job->addMetaData("referrer", req->m_baseURL.string());

  connect( job, SIGNAL( result( KIO::Job * ) ), this, SLOT( slotFinished( KIO::Job * ) ) );
  connect( job, SIGNAL( data( KIO::Job*, const QByteArray &)),
           SLOT( slotData( KIO::Job*, const QByteArray &)));

  m_requestsLoading.insert(job, req);
}

void Loader::slotFinished( KIO::Job* job )
{
  Request *r = m_requestsLoading.take( job );
  KIO::TransferJob* j = static_cast<KIO::TransferJob*>(job);

  if ( !r )
    return;

  if (j->error() || j->isErrorPage())
  {
      r->object->error( job->error(), job->errorText().ascii() );
#ifdef __BEOS__
      printf( "Warning: Loader::slotFinished:1() not emiting\n" );
#else
      emit requestFailed( r->m_baseURL, r->object );
#endif      
  }
  else
  {
      r->object->data(r->m_buffer, true);
#ifdef __BEOS__
      printf( "Warning: Loader::slotFinished:2() not emiting\n" );
#else
      emit requestDone( r->m_baseURL, r->object );
#endif      
  }

  r->object->finish();

#ifdef CACHE_DEBUG
  kdDebug( 6060 ) << "Loader:: JOB FINISHED " << r->object->url().string() << endl;
#endif

  delete r;
  servePendingRequests();
}

void Loader::slotData( KIO::Job*job, const QByteArray &data )
{
    Request *r = m_requestsLoading[job];
    if(!r) {
        kdDebug( 6060 ) << "got data for unknown request!" << endl;
        return;
    }

    if ( !r->m_buffer.isOpen() )
        r->m_buffer.open( IO_WriteOnly );
    r->m_buffer.writeBlock( data.data(), data.size() );

    if(r->incremental)
        r->object->data( r->m_buffer, false );
}

int Loader::numRequests( const DOMString &baseURL ) const
{
    int res = 0;

    QListIterator<Request> pIt( m_requestsPending );
    for (; pIt.current(); ++pIt )
        if ( pIt.current()->m_baseURL == baseURL )
            res++;

    QPtrDictIterator<Request> lIt( m_requestsLoading );
    for (; lIt.current(); ++lIt )
        if ( lIt.current()->m_baseURL == baseURL )
            res++;

    return res;
}

int Loader::numRequests( const DOMString &baseURL, CachedObject::Type type ) const
{
    int res = 0;

    QListIterator<Request> pIt( m_requestsPending );
    for (; pIt.current(); ++pIt )
        if ( pIt.current()->m_baseURL == baseURL && pIt.current()->object->type() == type )
            res++;

    QPtrDictIterator<Request> lIt( m_requestsLoading );
    for (; lIt.current(); ++lIt )
        if ( lIt.current()->m_baseURL == baseURL && pIt.current()->object->type() == type )
            res++;

    return res;
}

void Loader::cancelRequests( const DOMString &baseURL )
{
    //kdDebug( 6060 ) << "void Loader::cancelRequests( " << baseURL.string() << " )" << endl;
    //kdDebug( 6060 ) << "got " << m_requestsPending.count() << " pending requests" << endl;
    QListIterator<Request> pIt( m_requestsPending );
    while ( pIt.current() )
    {
        if ( pIt.current()->m_baseURL == baseURL )
        {
            //kdDebug( 6060 ) << "cancelling pending request for " << pIt.current()->object->url().string() << endl;
            Cache::removeCacheEntry( pIt.current()->object );
            m_requestsPending.remove( pIt );
        }
        else
            ++pIt;
    }

    //kdDebug( 6060 ) << "got " << m_requestsLoading.count() << "loading requests" << endl;

    QPtrDictIterator<Request> lIt( m_requestsLoading );
    while ( lIt.current() )
    {
        if ( lIt.current()->m_baseURL == baseURL )
        {
            //kdDebug( 6060 ) << "cancelling loading request for " << lIt.current()->object->url().string() << endl;
            KIO::Job *job = static_cast<KIO::Job *>( lIt.currentKey() );
            Cache::removeCacheEntry( lIt.current()->object );
            m_requestsLoading.remove( lIt.currentKey() );
            job->kill();
        }
        else
            ++lIt;
    }
}

KIO::Job *Loader::jobForRequest( const DOM::DOMString &url ) const
{
    QPtrDictIterator<Request> it( m_requestsLoading );

    for (; it.current(); ++it )
    {
        CachedObject *obj = it.current()->object;

        if ( obj && obj->url() == url )
            return static_cast<KIO::Job *>( it.currentKey() );
    }

    return 0;
}
#endif // __BEOS__
// ----------------------------------------------------------------------------

#ifndef __BEOS__

QDict<CachedObject> *Cache::cache = 0;
Cache::LRUList *Cache::lru = 0;
Loader *Cache::m_loader = 0;

int Cache::maxSize = DEFCACHESIZE;
int Cache::flushCount = 0;

QPixmap *Cache::nullPixmap = 0;
QPixmap *Cache::brokenPixmap = 0;

bool Cache::s_autoloadImages = true;

void Cache::init()
{
    if(!cache)
    {
        cache = new QDict<CachedObject>(401, true);
    }
    if(!lru)
    {
        lru = new LRUList;
    }
    if(!nullPixmap)
        nullPixmap = new QPixmap;

#ifdef __BEOS__
    printf( "Cache::init() does not create Cache::brokenPixmap\n" );
#else
    if(!brokenPixmap)
        brokenPixmap = new QPixmap(KHTMLFactory::instance()->iconLoader()->loadIcon("file_broken", KIcon::FileSystem, 0, KIcon::DisabledState));
#endif
    
    if(!m_loader)
        m_loader = new Loader();
}

void Cache::clear()
{
    if ( !cache ) return;
#ifdef CACHE_DEBUG
    kdDebug( 6060 ) << "Cache: CLEAR!" << endl;
    statistics();
#endif
    cache->setAutoDelete( true );
    delete cache; cache = 0;
    delete lru;   lru = 0;
    delete nullPixmap; nullPixmap = 0;
    delete brokenPixmap; brokenPixmap = 0;
    delete m_loader;   m_loader = 0;
}

CachedImage *Cache::requestImage( const DOMString & url, const DOMString &baseUrl, bool reload, int _expireDate )
{
    // this brings the _url to a standard form...
    KURL kurl = completeURL( url, baseUrl );
    if( kurl.isMalformed() )
    {
#ifdef CACHE_DEBUG
      kdDebug( 6060 ) << "Cache: Malformed url: " << kurl.url() << endl;
#endif
      return 0;
    }

    CachedObject *o = 0;
    if (!reload)
        o = cache->find(kurl.url());
    if(!o)
    {
#ifdef CACHE_DEBUG
        kdDebug( 6060 ) << "Cache: new: " << kurl.url() << endl;
#endif
        CachedImage *im = new CachedImage(kurl.url(), baseUrl, reload, _expireDate);
        cache->insert( kurl.url(), im );
        lru->append( kurl.url() );
        flush();
        return im;
    }

    o->setExpireDate(_expireDate);

    if(!o->type() == CachedObject::Image)
    {
#ifdef CACHE_DEBUG
        kdDebug( 6060 ) << "Cache::Internal Error in requestImage url=" << kurl.url() << "!" << endl;
#endif
        return 0;
    }

#ifdef CACHE_DEBUG
    if( o->status() == CachedObject::Pending )
        kdDebug( 6060 ) << "Cache: loading in progress: " << kurl.url() << endl;
    else
        kdDebug( 6060 ) << "Cache: using cached: " << kurl.url() << ", status " << o->status() << endl;
#endif

    lru->touch( kurl.url() );
    return static_cast<CachedImage *>(o);
}

CachedCSSStyleSheet *Cache::requestStyleSheet( const DOMString & url, const DOMString &baseUrl, bool reload, int _expireDate)
{
    // this brings the _url to a standard form...
    KURL kurl = completeURL( url, baseUrl );
    if( kurl.isMalformed() )
    {
      kdDebug( 6060 ) << "Cache: Malformed url: " << kurl.url() << endl;
      return 0;
    }

    CachedObject *o = cache->find(kurl.url());
    if(!o)
    {
#ifdef CACHE_DEBUG
        kdDebug( 6060 ) << "Cache: new: " << kurl.url() << endl;
#endif
        CachedCSSStyleSheet *sheet = new CachedCSSStyleSheet(kurl.url(), baseUrl, reload, _expireDate);
        cache->insert( kurl.url(), sheet );
        lru->append( kurl.url() );
        flush();
        return sheet;
    }

    o->setExpireDate(_expireDate);

    if(!o->type() == CachedObject::CSSStyleSheet)
    {
#ifdef CACHE_DEBUG
        kdDebug( 6060 ) << "Cache::Internal Error in requestStyleSheet url=" << kurl.url() << "!" << endl;
#endif
        return 0;
    }

#ifdef CACHE_DEBUG
    if( o->status() == CachedObject::Pending )
        kdDebug( 6060 ) << "Cache: loading in progress: " << kurl.url() << endl;
    else
        kdDebug( 6060 ) << "Cache: using cached: " << kurl.url() << endl;
#endif

    lru->touch( kurl.url() );
    return static_cast<CachedCSSStyleSheet *>(o);
}

CachedScript *Cache::requestScript( const DOM::DOMString &url, const DOM::DOMString &baseUrl, bool reload, int _expireDate)
{
    // this brings the _url to a standard form...
    KURL kurl = completeURL( url, baseUrl );
    if( kurl.isMalformed() )
    {
      kdDebug( 6060 ) << "Cache: Malformed url: " << kurl.url() << endl;
      return 0;
    }

    CachedObject *o = cache->find(kurl.url());
    if(!o)
    {
#ifdef CACHE_DEBUG
        kdDebug( 6060 ) << "Cache: new: " << kurl.url() << endl;
#endif
        CachedScript *script = new CachedScript(kurl.url(), baseUrl, reload, _expireDate);
        cache->insert( kurl.url(), script );
        lru->append( kurl.url() );
        flush();
        return script;
    }

    o->setExpireDate(_expireDate);

    if(!o->type() == CachedObject::Script)
    {
#ifdef CACHE_DEBUG
        kdDebug( 6060 ) << "Cache::Internal Error in requestScript url=" << kurl.url() << "!" << endl;
#endif
        return 0;
    }

#ifdef CACHE_DEBUG
    if( o->status() == CachedObject::Pending )
        kdDebug( 6060 ) << "Cache: loading in progress: " << kurl.url() << endl;
    else
        kdDebug( 6060 ) << "Cache: using cached: " << kurl.url() << endl;
#endif

    lru->touch( kurl.url() );
    return static_cast<CachedScript *>(o);
}

void Cache::flush(bool force)
{
    if (force)
       flushCount = 0;
    // Don't flush for every image.
    if (!lru || (lru->count() < (uint) flushCount))
       return;

    init();

#ifdef CACHE_DEBUG
    //statistics();
    kdDebug( 6060 ) << "Cache: flush()" << endl;
#endif

    int cacheSize = 0;

    for ( QStringList::Iterator it = lru->fromLast(); it != lru->end(); )
    {
        QString url = *it;
        --it; // Update iterator, we might delete the current entry later on.
        CachedObject *o = cache->find( url );

        if( !o->canDelete() || o->status() == CachedObject::Persistent ) {
               continue; // image is still used or cached permanently
	       // in this case don't count it for the size of the cache.
        }

        if( o->status() != CachedObject::Uncacheable )
        {
           cacheSize += o->size();

           if( cacheSize < maxSize )
               continue;
        }

        o->setFree( true );

        lru->remove( url );
        cache->remove( url );

        if ( o->canDelete() )
           delete o;
    }
    flushCount = lru->count()+10; // Flush again when the cache has grown.
#ifdef CACHE_DEBUG
    //statistics();
#endif
}

void Cache::setSize( int bytes )
{
    maxSize = bytes;
    // may be we need to clear parts of the cache
    flushCount = 0;
    flush(true);
}

void Cache::statistics()
{
    CachedObject *o;
    // this function is for debugging purposes only
    init();

    int size = 0;
    int msize = 0;
    int movie = 0;
    int stylesheets = 0;
    QDictIterator<CachedObject> it(*cache);
    for(it.toFirst(); it.current(); ++it)
    {
        o = it.current();
        if(o->type() == CachedObject::Image)
        {
#ifndef __BEOS__	    
            CachedImage *im = static_cast<CachedImage *>(o);
            if(im->m != 0)
            {
                movie++;
                msize += im->size();
            }
#endif	    
        }
        else
        {
            if(o->type() == CachedObject::CSSStyleSheet)
                stylesheets++;

        }
        size += o->size();
    }
    size /= 1024;

    kdDebug( 6060 ) << "------------------------- image cache statistics -------------------" << endl;
    kdDebug( 6060 ) << "Number of items in cache: " << cache->count() << endl;
    kdDebug( 6060 ) << "Number of items in lru  : " << lru->count() << endl;
    kdDebug( 6060 ) << "Number of cached images: " << cache->count()-movie << endl;
    kdDebug( 6060 ) << "Number of cached movies: " << movie << endl;
    kdDebug( 6060 ) << "Number of cached stylesheets: " << stylesheets << endl;
    kdDebug( 6060 ) << "pixmaps:   allocated space approx. " << size << " kB" << endl;
    kdDebug( 6060 ) << "movies :   allocated space approx. " << msize/1024 << " kB" << endl;
    kdDebug( 6060 ) << "--------------------------------------------------------------------" << endl;
}

KURL Cache::completeURL( const DOMString &_url, const DOMString &_baseUrl )
{
    QString url = _url.string();
    QString baseUrl = _baseUrl.string();
    KURL orig(baseUrl);
    KURL u( orig, url );
    return u;
}

void Cache::removeCacheEntry( CachedObject *object )
{
  QString key = object->url().string();

  // this indicates the deref() method of CachedObject to delete itself when the reference counter
  // drops down to zero
  object->setFree( true );

  cache->remove( key );
  lru->remove( key );

  if ( object->canDelete() )
     delete object;
}

void Cache::autoloadImages( bool enable )
{
  if ( enable == s_autoloadImages )
    return;

  s_autoloadImages = enable;

  QDictIterator<CachedObject> it( *cache );
  for (; it.current(); ++it )
    if ( it.current()->type() == CachedObject::Image )
    {
      CachedImage *img = static_cast<CachedImage *>( it.current() );

      CachedObject::Status status = img->status();
      if ( status != CachedObject::Unknown ||
           status == CachedObject::Cached ||
           status == CachedObject::Uncacheable ||
           status == CachedObject::Pending )
        continue;

      img->load();
    }
}
#endif // __BEOS__

#include "loader.moc"
