#include <stdio.h>

#include <Application.h>

#include "afont.h"

#include <map>

#include <kdebug.h>

class QFontInternal
{
public:
    RFont*	m_pcFont;
};

static std::map<int,RFont*> g_cFonts;

RFont* create_font( int nSize = 10 )
{
    std::map<int,RFont*>::iterator i = g_cFonts.find( nSize );

    if ( i == g_cFonts.end() ) {
		RFont* pcFont = new RFont(); // os::Font( sProp );
		pcFont->f()->SetSize( nSize );
		pcFont->AddRef();
		g_cFonts.insert( std::make_pair( nSize, pcFont ) );
		return( pcFont );
    } else {
		(*i).second->AddRef();
		return( (*i).second );
    }
}

QFont::QFont()
{
	kdDebug(10) << "QFont::" << __FUNCTION__ << "1()" << endl;
    d = new QFontInternal;
    d->m_pcFont = create_font(); //   new os::Font( DEFAULT_FONT_REGULAR );
}
QFont::QFont( const QString&, int pointSize, int , bool )
{
	kdDebug(10) << "QFont::" << __FUNCTION__ << "2()" << endl;
    d = new QFontInternal;
    d->m_pcFont = create_font( pointSize ); // new os::Font( DEFAULT_FONT_REGULAR );
}

QFont::QFont( const QFont& cOther )
{
	kdDebug(10) << "QFont::" << __FUNCTION__ << "3()" << endl;
    d = new QFontInternal;
    d->m_pcFont = create_font( cOther.pointSize() ); // new os::Font( *cOther.d->m_pcFont );
}

QFont::~QFont()
{
	kdDebug(10) << "QFont::" << __FUNCTION__ << "()" << endl;
    d->m_pcFont->RemoveRef();
    delete d;
}

void QFont::setFamily( const QString &)
{
	kdDebug(10) << "QFont::" << __FUNCTION__ << "()" << endl;
    printf( "Warning: QFont::%s() not implemented\n", __FUNCTION__ );
}

QString QFont::family()	const
{
	kdDebug(10) << "QFont::" << __FUNCTION__ << "()" << endl;
    printf( "Warning: QFont::%s() not implemented\n", __FUNCTION__ );
    return QString();
}

void QFont::setCharSet( CharSet )
{
	kdDebug(10) << "QFont::" << __FUNCTION__ << "()" << endl;
    printf( "Warning: QFont::%s() not implemented\n", __FUNCTION__ );
}

QFont::CharSet QFont::charSet() const
{
	kdDebug(10) << "QFont::" << __FUNCTION__ << "()" << endl;
    printf( "Warning: QFont::%s() not implemented\n", __FUNCTION__ );
    return( ISO_8859_1 );
}
    

bool QFont::italic() const
{
	kdDebug(10) << "QFont::" << __FUNCTION__ << "()" << endl;
    printf( "Warning: QFont::%s() not implemented\n", __FUNCTION__ );
    return( false );
}

void QFont::setItalic( bool )
{
	kdDebug(10) << "QFont::" << __FUNCTION__ << "()" << endl;
    printf( "Warning: QFont::%s() not implemented\n", __FUNCTION__ );
}

int QFont::weight() const
{
	kdDebug(10) << "QFont::" << __FUNCTION__ << "()" << endl;
    printf( "Warning: QFont::%s() not implemented\n", __FUNCTION__ );
    return( Normal );
}

void QFont::setWeight( int )
{
	kdDebug(10) << "QFont::" << __FUNCTION__ << "()" << endl;
    printf( "Warning: QFont::%s() not implemented\n", __FUNCTION__ );
}

void QFont::setPointSize( int nSize )
{
	kdDebug(10) << "QFont::" << __FUNCTION__ << "()" << endl;
    d->m_pcFont->RemoveRef();
    d->m_pcFont = create_font( nSize );
//    d->m_pcFont->SetSize( nSize );
}

void QFont::setPixelSize( int )
{
	kdDebug(10) << "QFont::" << __FUNCTION__ << "()" << endl;
    printf( "Warning: QFont::%s() not implemented\n", __FUNCTION__ );
}
    
int QFont::pixelSize() const
{
	kdDebug(10) << "QFont::" << __FUNCTION__ << "()" << endl;
    font_height sHeight;
    d->m_pcFont->f()->GetHeight( &sHeight );
    return( int(sHeight.ascent + sHeight.descent + sHeight.leading) + 1 );
}

int QFont::pointSize() const
{
	kdDebug(10) << "QFont::" << __FUNCTION__ << "()" << endl;
    return( int( d->m_pcFont->f()->Size() ) );
}
    
QFont& QFont::operator=( const QFont& cOther )
{
	kdDebug(10) << "QFont::" << __FUNCTION__ << "()" << endl;
    d->m_pcFont->RemoveRef();
    d->m_pcFont = create_font( int(cOther.d->m_pcFont->f()->Size()) ); // new os::Font( *cOther.d->m_pcFont );
//    printf( "Create/copy font\n" );
    return( *this );
}

bool QFont::operator==( const QFont& cOther ) const
{
	kdDebug(10) << "QFont::" << __FUNCTION__ << "()" << endl;
    return( *d->m_pcFont == *cOther.d->m_pcFont );
}

bool QFont::operator!=( const QFont& cOther ) const
{
	kdDebug(10) << "QFont::" << __FUNCTION__ << "()" << endl;
    return( *d->m_pcFont != *cOther.d->m_pcFont );
}


RFont* QFont::GetFont() const
{
	kdDebug(10) << "QFont::" << __FUNCTION__ << "()" << endl;
    return( d->m_pcFont );
}

void QFont::SetFont( RFont* pcFont )
{
	kdDebug(10) << "QFont::" << __FUNCTION__ << "()" << endl;
//    printf( "Set font\n" );
    d->m_pcFont->RemoveRef();
    d->m_pcFont = pcFont;
    d->m_pcFont->AddRef();
}
