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

#include "KWQLogging.h"

#include "KWQResourceLoader.h"
#include "KWIQResponse.h"
#include "KWQLoader.h"
#include "KWQString.h"
#include "KWQKJobClasses.h"
#include "KWQFoundationExtras.h"

#include "KWQDict.h"
class KWQResourceLoader;

namespace KIO {
    
    // The allocations and releases in TransferJobPrivate are
    // definitely Cocoa-exception-free (either simple Foundation
    // classes or our own KWQResourceLoader which avoides doing work
    // in dealloc

class TransferJobPrivate
{
public:
    TransferJobPrivate(const KURL &kurl)
        : status(0)
	, metaData(0)
	, URL(kurl)
	, loader(0)
	, method("GET")
	, response(0)
	, assembledResponseHeaders(true)
    {
    }

    TransferJobPrivate(const KURL& kurl, const QByteArray &_postData)
        : status(0)
        , metaData(0)
	, URL(kurl)
	, loader(0)
	, method("POST")
	, postData(_postData)
	, response(0)
	, assembledResponseHeaders(true)
     {
     }
   
    ~TransferJobPrivate()
    {
	KWQReleaseResponse(response);
        //KWQRelease(metaData);
	delete metaData;
        KWQRelease(loader);
    }
    
    int status;
    QDict<QString> *metaData;
    KURL URL;
    KWQResourceLoader *loader;
    QString method;
    QByteArray postData;

    KWIQResponse *response;
    bool assembledResponseHeaders;
    QString responseHeaders;    
};

TransferJob::TransferJob(const KURL &url, bool reload, bool showProgressInfo)
    : d(new TransferJobPrivate(url)),
      m_data(this, SIGNAL(data(KIO::Job*, const char*, int))),
      m_redirection(this, SIGNAL(redirection(KIO::Job*, const KURL&))),
      m_result(this, SIGNAL(result(KIO::Job*))),
      m_receivedResponse(this, SIGNAL(receivedResponse(KIO::Job*, KWIQResponse *)))
{
}

TransferJob::TransferJob(const KURL &url, const QByteArray &postData, bool showProgressInfo)
    : d(new TransferJobPrivate(url, postData)),
      m_data(this, SIGNAL(data(KIO::Job*, const char*, int))),
      m_redirection(this, SIGNAL(redirection(KIO::Job*, const KURL&))),
      m_result(this, SIGNAL(result(KIO::Job*))),
      m_receivedResponse(this, SIGNAL(receivedResponse(KIO::Job*, KWIQResponse *)))    
{
}

TransferJob::~TransferJob()
{
    // This will cancel the handle, and who knows what that could do
    if (d->loader) d->loader->jobWillBeDeallocated();
    delete d;
}

bool TransferJob::isErrorPage() const
{
    return d->status != 0;
}

int TransferJob::error() const
{
    return d->status;
}

void TransferJob::setError(int e)
{
    d->status = e;
}

QString TransferJob::errorText() const
{
    LOG(NotYetImplemented, "not yet implemented");
    return QString::null;
}

void TransferJob::assembleResponseHeaders() const
{
    if (!d->response)
	return;
    
    if (!d->assembledResponseHeaders) {
	d->responseHeaders = d->response->headers();
	d->assembledResponseHeaders = true;
    }
}

QString TransferJob::queryMetaData(const QString &key) const
{
    if (key == "HTTP-Headers") {
	assembleResponseHeaders();
	return d->responseHeaders;
    }
    
    if (!d->metaData)
	return QString::null;
    
    QString* value = d->metaData->find(key);
    return value ? *value : QString::null;
}

void TransferJob::addMetaData(const QString &key, const QString &value)
{
    if (!d->metaData){
	d->metaData = new QDict<QString>();
	d->metaData->setAutoDelete(true);
    }
    
    d->metaData->insert(key, new QString(value));
}

void TransferJob::addMetaData(const QMap<QString, QString> &keysAndValues)
{
    if (!d->metaData){
	d->metaData = new QDict<QString>();
	d->metaData->setAutoDelete(true);
    }
    
    QMapConstIterator<QString, QString> it = keysAndValues.begin();
    QMapConstIterator<QString, QString> end = keysAndValues.end();
    while (it != end) {
	d->metaData->insert(it.key(), new QString(it.data()));
        ++it;
    }
}

void TransferJob::kill()
{
    delete this;
}

void TransferJob::setLoader(KWQResourceLoader *loader)
{
    KWQRetain(loader);
    KWQRelease(d->loader);
    d->loader = loader;
}

KURL TransferJob::url() const
{
    return d->URL;
}

QByteArray TransferJob::postData() const
{
    return d->postData;
}

QString TransferJob::method() const
{
    return d->method;
}

void TransferJob::emitData(const char *data, int size)
{
    m_data.call(this, data, size);
}

void TransferJob::emitRedirection(const KURL &url)
{
    m_redirection.call(this, url);
}

void TransferJob::emitResult()
{
    m_result.call(this);
}

void TransferJob::emitReceivedResponse(KWIQResponse *response)
{
    d->assembledResponseHeaders = false;
    d->response = response;
    KWQRetainResponse(d->response);

    m_receivedResponse.call(this, response);
}

} // namespace KIO
