#ifndef WebCoreHistory_h
#define WebCoreHistory_h

#include <glib.h>

class WebCoreHistory
{
public:    
    
    virtual bool containsItemForURLUTF8(const gchar* url, int len)=0;
    virtual bool containsItemForURLLatin1(const gchar* url, int len)=0;
    
    static void setHistoryProvider(WebCoreHistory* h) { m_sharedProvider = h; }
    static WebCoreHistory* historyProvider() { return m_sharedProvider; }

protected:
    WebCoreHistory() {};
    virtual ~WebCoreHistory() {};
    
    static WebCoreHistory* m_sharedProvider;

private:

    
};


#endif
