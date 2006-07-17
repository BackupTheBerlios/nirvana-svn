#include <stdio.h>

#include "afontdatabase.h"

#include <kdebug.h>

QFontDatabase::QFontDatabase()
{
	kdDebug(10) << "QFontDatabase::" << __FUNCTION__ << "()" << endl;
}

QString QFontDatabase::styleString( const QFont &)
{
	kdDebug(10) << "QFontDatabase::" << __FUNCTION__ << "()" << endl;
    return( m_style );
}

bool QFontDatabase::isSmoothlyScalable( const QString &/*family*/,
					const QString &style = QString::null,
					const QString &charSet = QString::null ) const
{
	kdDebug(10) << "QFontDatabase::" << __FUNCTION__ << "()" << endl;
    if ( style == charSet) return( true ); return( true );
}

QValueList<int> QFontDatabase::smoothSizes( const QString &/*family*/,
					    const QString &style,
					    const QString &charSet = QString::null )
{
	kdDebug(10) << "QFontDatabase::" << __FUNCTION__ << "()" << endl;
    if ( style == charSet) return( m_sizes ); return( m_sizes );
}
