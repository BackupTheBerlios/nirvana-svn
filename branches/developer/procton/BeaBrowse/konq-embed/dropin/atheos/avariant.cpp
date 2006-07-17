
#include <qvariant.h>

#include <kdebug.h>

class QVariant::Private
{
public:
    QVariant::Type m_eType;
    bool	   m_bBool;
};

QVariant::QVariant()
{
	kdDebug(10) << "QVariant::" << __FUNCTION__ << "1()" << endl;
    d = new Private;
    d->m_eType = Invalid;
}

QVariant::QVariant( const QString& )
{
	kdDebug(10) << "QVariant::" << __FUNCTION__ << "2()" << endl;
    d = new Private;
    d->m_eType = String;
}

QVariant::QVariant( bool bValue, int )
{
	kdDebug(10) << "QVariant::" << __FUNCTION__ << "3()" << endl;
    d = new Private;
    d->m_eType = Bool;
    d->m_bBool = bValue;
}

QVariant::QVariant( double )
{
	kdDebug(10) << "QVariant::" << __FUNCTION__ << "4()" << endl;
    d = new Private;
    d->m_eType = QVariant::Double;
}

QVariant::Type QVariant::type() const
{
	kdDebug(10) << "QVariant::" << __FUNCTION__ << "()" << endl;
    return( d->m_eType );
}

bool QVariant::toBool() const
{
	kdDebug(10) << "QVariant::" << __FUNCTION__ << "()" << endl;
    return( d->m_eType );
}
