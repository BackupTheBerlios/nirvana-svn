#include <stdio.h>

#include "aframe.h"

#include <kdebug.h>

QFrame::QFrame( QWidget* pcParent, const char* pzName ) : QWidget( pcParent, pzName )
{
	kdDebug(10) << "QFrame::" << __FUNCTION__ << "()" << endl;
}

QFrame::~QFrame()
{
	kdDebug(10) << "QFrame::" << __FUNCTION__ << "()" << endl;
}

void QFrame::setFrameStyle( int )
{
	kdDebug(10) << "QFrame::" << __FUNCTION__ << "()" << endl;
}

int QFrame::frameStyle() const
{
	kdDebug(10) << "QFrame::" << __FUNCTION__ << "()" << endl;
    return( NoFrame );
}

#include "aframe.moc"
