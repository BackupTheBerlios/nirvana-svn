#include <stdio.h>

#include "acursor.h"

#include <Application.h>

#include <kdebug.h>

QCursor::QCursor()
{
	kdDebug(10) << "QCursor::" << __FUNCTION__ << "1()" << endl;
    printf( "Warning: QCursor::QCursor:1() not implemented\n" );
}

QCursor::QCursor( QCursorShape )
{
	kdDebug(10) << "QCursor::" << __FUNCTION__ << "2()" << endl;
    printf( "Warning: QCursor::QCursor:2() not implemented\n" );
}

QPoint QCursor::pos()
{
	kdDebug(10) << "QCursor::" << __FUNCTION__ << "()" << endl;
    printf( "Warning: QCursor::pos() not implemented\n" );
    return( QPoint( 0, 0 ) );
}
