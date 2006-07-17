#ifndef __kapp_h__
#define __kapp_h__

#ifndef __BEOS__
#include <qapplication.h>
#endif

#include <stdlib.h>

#include "kinstance.h"

#ifdef __BEOS__
#include <qstring.h>
#endif

#define kapp KApplication::self()

class DCOPClient;

/*#ifdef __BEOS__*/
class KApplication : public KInstance
/*#else
class KApplication : public QApplication, public KInstance
#endif*/
{
#ifndef __BEOS__
    Q_OBJECT
#endif
public:
    KApplication( int argc, char **argv, const char *name );
    virtual ~KApplication();

    static KApplication *self() { return s_self; }

#ifndef __BEOS__
    static bool startServiceByDesktopName( const QString &, const QStringList &, QString * );

    static bool startServiceByDesktopPath( const QString & );
    DCOPClient *dcopClient() const { return m_dcopClient; }
#endif
    
#ifdef __BEOS__
/*    QString	     translate( const char *, const char * ) const { return( "" ); }
    QString	     translate( const char *, const char *,
				const char * ) const { return( "" ); }*/
    
#endif
    
    void invokeBrowser( const QString & ) {}
    void invokeMailer( const QString &, const QString & ) {}

    int random() { return rand(); } // ### ;-)

private:
    static KApplication *s_self;

#ifndef __BEOS__
    DCOPClient *m_dcopClient;
#endif
};


#ifdef __BEOS__
extern KApplication* qApp;
#endif

// ### FIXME: checkAccess copyright by Kalle!

bool checkAccess( const QString &pathname, int mode);

#endif
