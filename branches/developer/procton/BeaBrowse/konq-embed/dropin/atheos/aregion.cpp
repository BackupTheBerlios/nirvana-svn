
#include <stdio.h>
#include "aregion.h"

#include <kdebug.h>

QRegion::QRegion()
{
	kdDebug(10) << "QRegion::" << __FUNCTION__ << "1()" << endl;
}

QRegion::QRegion( int x, int y, int w, int h, RegionType = Rectangle )
{
	kdDebug(10) << "QRegion::" << __FUNCTION__ << "2()" << endl;
}

QRegion::QRegion( const QPointArray &, bool winding=FALSE )
{
	kdDebug(10) << "QRegion::" << __FUNCTION__ << "3()" << endl;
}

QRegion::~QRegion()
{
	kdDebug(10) << "QRegion::" << __FUNCTION__ << "()" << endl;
}

QRegion& QRegion::operator=( const QRegion & )
{
	kdDebug(10) << "QRegion::" << __FUNCTION__ << "()" << endl;
    return( *this );
}

bool QRegion::contains( const QPoint &p ) const
{
	kdDebug(10) << "QRegion::" << __FUNCTION__ << "()" << endl;
    printf( "Warning: QRegion::contains() not implemented\n" );
    return( false );
}
