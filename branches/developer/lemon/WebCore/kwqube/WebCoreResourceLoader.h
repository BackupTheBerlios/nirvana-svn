#ifndef WebCoreResourceLoader_h
#define WebCoreResourceLoader_h

#include "WebCoreFoundation.h"

class KURL;
class KWIQResponse;
/**@protocol*/
/** Client interface for resource loading
 * client implements loading and uses these functions to pass data to the engine
 */ 
class WebCoreResourceLoader : public WebCoreNSObject
{
public:
    virtual ~WebCoreResourceLoader() {}
    virtual void receivedResponse(KWIQResponse* response)=0;
    virtual void redirectedToURL(KURL &url)=0;    
    virtual void addData(const void *data,unsigned int length)=0;

    // Either finish, reportError, or cancel must be called before the
    // loader is released, but never more than one.    
    virtual void finish(void)=0;
    virtual void reportError(void)=0;
    virtual void cancel(void)=0;

};

/**@protocol*/
/**
 * Client provides a WebCoreResourceHandle per resource request
 *
 * library calls cancel() to stop the transfer,
 *               release() to give up the handle 
 */
class WebCoreResourceHandle : public WebCoreNSObject
{
public:
    virtual ~WebCoreResourceHandle() {}
    /** called when library wants to stop the transfer*/  
    virtual void cancel()=0;
    
};

#endif
