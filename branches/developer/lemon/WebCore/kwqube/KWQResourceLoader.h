/*
 * Copyright (C) 2003 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef __KWQRESOURCELOADER_H__
#define __KWQRESOURCELOADER_H__

#include "WebCoreResourceLoader.h"

namespace khtml {
    class Loader;
}

namespace KIO {
    class TransferJob;
}

class KURL;
class KWIQResponse;

class  KWQResourceLoader : public WebCoreResourceLoader
{
    KIO::TransferJob *_job;    
    WebCoreResourceHandle* _handle;
    void finishJobAndHandle(void);
    
 public:
    KWQResourceLoader(KIO::TransferJob *job);
    virtual ~KWQResourceLoader();
    
    void setHandle(WebCoreResourceHandle* handle);
    void jobWillBeDeallocated(void);
    
    void receivedResponse(KWIQResponse* response);
    void redirectedToURL(KURL &url);    
    void addData(const void *data,unsigned int length);
								 
    // Either finish, reportError, or cancel must be called before the
    // loader is released, but never more than one.    
    void finish(void);
    void reportError(void);
    void cancel(void);

};


#endif //!__KWQRESOURCELOADER_H__
