#include <stdio.h>

#include "apen.h"

#include <kdebug.h>

QPen::QPen()
{
	kdDebug(10) << "QPaintDevice::" << __FUNCTION__ << "()" << endl;
}

QPen::QPen( const QColor& color, uint /*width*/=0, PenStyle /*style*/=SolidLine )
{
	kdDebug(10) << "QPaintDevice::" << __FUNCTION__ << "()" << endl;
    m_color = color;
}

const QColor& QPen::color() const
{
	kdDebug(10) << "QPaintDevice::" << __FUNCTION__ << "()" << endl;
    return m_color;
}
