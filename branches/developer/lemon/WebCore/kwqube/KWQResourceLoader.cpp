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
#include "KWQResourceLoader.h"
#include "KWIQResponse.h"
#include "KWQLogging.h"
#include "KWQKJobClasses.h"
#include "loader.h"

using khtml::Loader;
using KIO::TransferJob;
KWQResourceLoader::KWQResourceLoader(KIO::TransferJob *job)
    : _job(job)
    , _handle(0)
{
    ASSERT(job);
    
    job->setLoader(this);
    job->addMetaData("expire-date","0");
}

KWQResourceLoader::~KWQResourceLoader()
{
    finishJobAndHandle();
}

void KWQResourceLoader::setHandle(WebCoreResourceHandle* handle)
{
    ASSERT(!_handle);
    handle->retain();
    _handle = handle;
}

void KWQResourceLoader::receivedResponse(KWIQResponse* response)
{
    ASSERT(response);
    ASSERT(_job);    
    _job->emitReceivedResponse(response);
}

void KWQResourceLoader::redirectedToURL(KURL &url)
{
    ASSERT(_job);
    _job->emitRedirection( KURL(url) );
}

void KWQResourceLoader::addData(const void *data, unsigned int length)
{
    ASSERT(data);
    ASSERT(_job);
    _job->emitData((const char *)data, length);
}

void KWQResourceLoader::jobWillBeDeallocated(void)
{
    WebCoreResourceHandle * handle = _handle;	
    _handle = 0;
    _job = 0;

    if (handle) {
	handle->cancel();
	handle->release();
    }
}

void KWQResourceLoader::finishJobAndHandle(void)
{
    TransferJob *job = _job;
    WebCoreResourceHandle *handle = _handle;

    _job = 0;
    _handle = 0;
    
    if (job) {
	job->emitResult();
    }
    
    delete job; // will call jobWillBeDeallocated
    if (handle) handle->release();
}

void KWQResourceLoader::cancel(void)
{
    if (_job) {
        _job->setError(1);
    }
    finishJobAndHandle();
}

void KWQResourceLoader::reportError(void)
{
    ASSERT(_job);
    _job->setError(1);
    finishJobAndHandle();
}

void KWQResourceLoader::finish(void)
{
    ASSERT(_job);
    ASSERT(_handle);
    finishJobAndHandle();
}
