#include <stdio.h>
#include <math.h>

#include "afontmetrics.h"
#include "afont.h"
#include <Font.h>

#include <kdebug.h>

class QFontMetricsInternal
{
public:
    RFont* m_pcFont;
};

QFontMetrics::QFontMetrics()
{
	kdDebug(10) << "QFontMetrics::" << __FUNCTION__ << "1()" << endl;
    d = new QFontMetricsInternal;
    d->m_pcFont = NULL;
}

QFontMetrics::QFontMetrics( const QFont& cFont )
{
	kdDebug(10) << "QFontMetrics::" << __FUNCTION__ << "2()" << endl;
    d = new QFontMetricsInternal;
    d->m_pcFont = cFont.GetFont();
    d->m_pcFont->AddRef();
}

QFontMetrics::QFontMetrics( const QFontMetrics& cOther )
{
	kdDebug(10) << "QFontMetrics::" << __FUNCTION__ << "3()" << endl;
    d = new QFontMetricsInternal;
    d->m_pcFont = cOther.d->m_pcFont;
    if ( d->m_pcFont != NULL ) {
		d->m_pcFont->AddRef();
    }
}

QFontMetrics::QFontMetrics( RFont* pcFont )
{
	kdDebug(10) << "QFontMetrics::" << __FUNCTION__ << "4()" << endl;
    d = new QFontMetricsInternal;
    d->m_pcFont = pcFont;
    d->m_pcFont->AddRef();
}

QFontMetrics::~QFontMetrics()
{
	kdDebug(10) << "QFontMetrics::" << __FUNCTION__ << "()" << endl;
    if ( d->m_pcFont != NULL ) {
		d->m_pcFont->RemoveRef();
    }
    delete d;
}

QFontMetrics& QFontMetrics::operator=( const QFontMetrics& cOther)
{
	kdDebug(10) << "QFontMetrics::" << __FUNCTION__ << "()" << endl;
    if ( d->m_pcFont != NULL ) {
		d->m_pcFont->RemoveRef();
    }
    d->m_pcFont = cOther.d->m_pcFont;
    if ( d->m_pcFont != NULL ) {
		d->m_pcFont->AddRef();
    }
    return( *this );
}


void QFontMetrics::setFont( const QFont& cFont )
{
	kdDebug(10) << "QFontMetrics::" << __FUNCTION__ << "()" << endl;
    if ( d->m_pcFont != NULL ) {
		d->m_pcFont->RemoveRef();
    }
    d->m_pcFont = cFont.GetFont();
    if ( d->m_pcFont != NULL ) {
		d->m_pcFont->AddRef();
    }
    
}

int QFontMetrics::height() const
{
	kdDebug(10) << "QFontMetrics::" << __FUNCTION__ << "()" << endl;
    if ( d->m_pcFont == NULL ) {
		return( 0 );
    }
    font_height sHeight;
    d->m_pcFont->f()->GetHeight( &sHeight );
    return( int(sHeight.ascent + sHeight.descent + sHeight.leading) + 1 );
}

int QFontMetrics::width( const QString& cString, int len ) const
{
	kdDebug(10) << "QFontMetrics::" << __FUNCTION__ << "()" << endl;
    if ( d->m_pcFont == NULL ) {
		return( 0 );
    }
    QCString cUtf8Str = cString.utf8();
    const char* pzStr = cUtf8Str.data();
    int nByteLen;
    if ( len == -1 ) {
		nByteLen = strlen( pzStr );
    } else {
		nByteLen = len;
    }
    return( int( ceil( d->m_pcFont->f()->StringWidth( pzStr, nByteLen ) ) ) );
}

int QFontMetrics::width( QChar cChar ) const
{
	kdDebug(10) << "QFontMetrics::" << __FUNCTION__ << "()" << endl;
    return( width( QString( cChar ) ) );
}
int QFontMetrics::width( char c ) const
{
	kdDebug(10) << "QFontMetrics::" << __FUNCTION__ << "()" << endl;
    return width( (QChar) c );
}

int QFontMetrics::ascent() const
{
	kdDebug(10) << "QFontMetrics::" << __FUNCTION__ << "()" << endl;
    if ( d->m_pcFont == NULL ) {
		return( 0 );
    }
    font_height sHeight;
    d->m_pcFont->f()->GetHeight( &sHeight );
    return( int(sHeight.ascent) );
}

int QFontMetrics::descent() const
{
	kdDebug(10) << "QFontMetrics::" << __FUNCTION__ << "()" << endl;
    if ( d->m_pcFont == NULL ) {
		return( 0 );
    }
    font_height sHeight;
    d->m_pcFont->f()->GetHeight( &sHeight );
    return( int(sHeight.descent) );
}
    
int QFontMetrics::rightBearing( QChar c ) const
{
	kdDebug(10) << "QFontMetrics::" << __FUNCTION__ << "()" << endl;
    return( width( c ) );
}
    
QRect QFontMetrics::boundingRect( const QString& cStr, int len ) const
{
	kdDebug(10) << "QFontMetrics::" << __FUNCTION__ << "()" << endl;
    int nWidth = width( cStr, len );

    font_height sHeight;
    d->m_pcFont->f()->GetHeight( &sHeight );
    
    return( QRect( QPoint( 0, -(sHeight.ascent + sHeight.leading) ), QPoint( nWidth - 1, sHeight.descent ) ) );
}

QRect QFontMetrics::boundingRect( QChar cChar ) const
{
	kdDebug(10) << "QFontMetrics::" << __FUNCTION__ << "()" << endl;
    return( boundingRect( QString( cChar ) ) );
}
