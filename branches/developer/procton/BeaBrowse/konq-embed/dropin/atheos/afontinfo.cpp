#include <stdio.h>

#include "afontinfo.h"
#include <qfont.h>

#include <kdebug.h>

class QFontInfoInternal
{
public:
    QFont m_cFont;
};

QFontInfo::QFontInfo( const QFont& cFont )
{
	kdDebug(10) << "QFontInfo::" << __FUNCTION__ << "()" << endl;
    d = new QFontInfoInternal;
    d->m_cFont = cFont;
}

QFontInfo::~QFontInfo()
{
	kdDebug(10) << "QFontInfo::" << __FUNCTION__ << "()" << endl;
    delete d;
}

int QFontInfo::pointSize() const
{
	kdDebug(10) << "QFontInfo::" << __FUNCTION__ << "()" << endl;
    return( d->m_cFont.pointSize() );
}

bool QFontInfo::fixedPitch() const
{
	kdDebug(10) << "QFontInfo::" << __FUNCTION__ << "()" << endl;
    return( false );
}
