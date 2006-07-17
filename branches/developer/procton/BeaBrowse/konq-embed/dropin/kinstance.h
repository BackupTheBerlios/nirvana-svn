#ifndef __kinstance_h__
#define __kinstance_h__

// dummy

class KStandardDirs;
class KConfig;
class KAboutData;

#ifdef __BEOS__
#include <qcstring.h>
#else
#include <kiconloader.h>
#endif

class KInstance
{
public:
    KInstance( KAboutData *data );
    KInstance( const char *name );
    ~KInstance() {}

#ifndef __BEOS__    
    KStandardDirs *dirs() const { return m_dirs; }
#endif
    KConfig *config() const;

    QCString instanceName() const { return m_name; }

#ifndef __BEOS__
    KIconLoader *iconLoader() const { return KIconLoader::self(); }
#endif
    
private:
#ifndef __BEOS__    
    KStandardDirs *m_dirs;
#endif    
    mutable KConfig *m_config;
    QCString m_name;
};

#endif
