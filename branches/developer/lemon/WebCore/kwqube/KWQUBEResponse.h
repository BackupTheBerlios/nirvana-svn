
#ifndef KWIQRESPONSE_H
#define KWIQRESPONSE_H

#include "WebCoreFoundation.h"

typedef int32 GTime;

class KWIQResponse : public WebCoreNSObject
{
public:
    virtual ~KWIQResponse() {}
    
    virtual const gchar* getMimeType() =0;
    virtual const gchar* const* allHeaderFields() =0;
    virtual const gchar* headers() = 0;
    virtual GTime expiresTime()=0;
};

#endif
