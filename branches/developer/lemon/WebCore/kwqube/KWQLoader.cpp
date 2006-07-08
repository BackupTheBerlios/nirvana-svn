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

#include "KWQLoader.h"

#include "KWQKJobClasses.h"
#include "KWQLogging.h"
#include "KWQResourceLoader.h"
#include "KWQFoundationExtras.h"
#include "WebCoreBridge.h"
#include "khtml_part.h"
#include "loader.h"

#include "KWIQResponse.h"

using khtml::Cache;
using khtml::CachedObject;
using khtml::CachedImage;
using khtml::DocLoader;
using khtml::Loader;
using khtml::Request;
using KIO::TransferJob;

bool KWQServeRequest(Loader *loader, Request *request, TransferJob *job)
{
    LOG(Loading, "Serving request for base %s, url %s", 
        request->m_docLoader->part()->baseURL().url().latin1(),
        request->object->url().string().latin1());

    return KWQServeRequest(loader, request->m_docLoader, job);
}

extern "C"{
void headers_value_key_destroy(gpointer data)
{
    gchar *c = static_cast<gchar *>(data);
    g_free(c);
}
}

bool KWQServeRequest(Loader *loader, DocLoader *docLoader, TransferJob *job)
{
    KWQKHTMLPart *part = static_cast<KWQKHTMLPart *>(docLoader->part());
    WebCoreBridge *bridge = part->bridge();

    part->didTellBridgeAboutLoad(job->url().url());
    
    KWQResourceLoader *resourceLoader = new KWQResourceLoader(job);
    
    GHashTable *headers = 0; 

    QString headerString = job->queryMetaData("customHTTPHeader");
    
    if (!headerString.isEmpty()) {
	headers = g_hash_table_new_full(g_str_hash, g_str_equal,
					headers_value_key_destroy,
					headers_value_key_destroy);

	QStringList l = QStringList::split(QString("\n"), headerString);
	QValueList<QString>::Iterator it = l.begin();
	QValueList<QString>::Iterator end = l.end();
	QString str, key, value;
	int pos;
	while (it != end) {
	    str = (*it);
	    pos = str.find(":");
	    if (pos != -1) {
		key = str.left(pos).stripWhiteSpace();
		value = str.right(pos+1).stripWhiteSpace();
		g_hash_table_insert(headers, g_strdup(key.utf8()), g_strdup(value.utf8()));
	    }
	    ++it;
	}
    }

    WebCoreResourceHandle *handle;

    if (job->method() == "POST") {
	GByteArray* postData = 0;	
        const int size = job->postData().size();
	if (size) {
	    GByteArray* postData = g_byte_array_sized_new(size);
	    g_byte_array_append(postData, reinterpret_cast<guint8 *>(g_strdup(job->postData().data())), size);
	}

	handle = bridge->startLoadingResource(resourceLoader,
					      job->url().url().utf8(),
					      headers,
					      postData);
	if (postData)
	    g_byte_array_free(postData, true);
    } else {
	handle = bridge->startLoadingResource(resourceLoader,
					      job->url().url().utf8(),
					      headers );

    }
    if (headers) g_hash_table_destroy(headers);
    
    resourceLoader->setHandle(handle);
   
    return handle != 0;    
}

QByteArray KWQServeSynchronousRequest(Loader *loader, DocLoader *docLoader, TransferJob *job, KURL &finalURL, QString &responseHeaders)
{
#if 0
    KWQKHTMLPart *part = static_cast<KWQKHTMLPart *>(docLoader->part());
    WebCoreBridge *bridge = part->bridge();

    part->didTellBridgeAboutLoad(job->url().url());

    NSDictionary *headerDict = nil;
    QString headerString = job->queryMetaData("customHTTPHeader");

    if (!headerString.isEmpty()) {
	headerDict = [[NSDictionary alloc] _webcore_initWithHeaderString:headerString.getNSString()];
    }

    NSData *postData = nil;
    

    if (job->method() == "POST") {
	postData = [NSData dataWithBytesNoCopy:job->postData().data() length:job->postData().size() freeWhenDone:NO];
    }

    KURL returnedFinalURL;
    QDict<String> responseHeaderDict;
    int statusCode = 0;
    QByteArray resultData = bridge->syncLoadResourceWithURL(job->url().url(), headerDict, job->postData(),
							 &returnedFinalURL,
							 &responseHeaderDict,
							 &statusCode);
    
    job->kill();

    finalURL = returnedFinalURL;
    responseHeaders = KWQHeaderStringFromDictionary(responseHeaderDict, statusCode);

    QByteArray results(resultData->length());

    memcpy( results.data(), resultData->bytes(), resultData->length() );

    return results;
#endif
    return QByteArray();
}

int KWQNumberOfPendingOrLoadingRequests(khtml::DocLoader *dl)
{
    return Cache::loader()->numRequests(dl);
}

bool KWQCheckIfReloading(DocLoader *loader)
{
    return static_cast<KWQKHTMLPart *>(loader->part())->bridge()->isReloading();
}

void KWQCheckCacheObjectStatus(DocLoader *loader, CachedObject *cachedObject)
{
    // Return from the function for objects that we didn't load from the cache.
    if (!cachedObject)
        return;
    switch (cachedObject->status()) {
    case CachedObject::Persistent:
    case CachedObject::Cached:
    case CachedObject::Uncacheable:
        break;
    case CachedObject::NotCached:
    case CachedObject::Unknown:
    case CachedObject::New:
    case CachedObject::Pending:
        return;
    }
    
    ASSERT(cachedObject->response());
    
    // Notify the caller that we "loaded".
    KWQKHTMLPart *part = static_cast<KWQKHTMLPart *>(loader->part());

    QString urlString = cachedObject->url().string();

    if (!part->haveToldBridgeAboutLoad(urlString)) {
	WebCoreBridge *bridge = part->bridge();
	// FIXME: KWIQ: Apple has here cachedimage specific code, which
	// uses dynamic cast to get cachedImage->dataSize() instead of
	// cachedObject->size() 
	// why is that?
	bridge->objectLoadedFromCacheWithURL(cachedObject->url().string().utf8(),
					     cachedObject->response(),
					     cachedObject->size());
	part->didTellBridgeAboutLoad(urlString);	
    }
	
}

void KWQRetainResponse(KWIQResponse *r)
{
    KWQRetain(r);
}

void KWQReleaseResponse(KWIQResponse *r)
{
    KWQRelease(r);
}


QString KWQResponseMIMEType(KWIQResponse *r)
{
    return QString();

    ASSERT(r);
    return r->getMimeType();
}

QString KWQResponseHeaderString(KWIQResponse *r)
{
    return 0;
    ASSERT(r);    
    return r->headers();
}

time_t KWQCacheObjectExpiresTime(khtml::DocLoader *docLoader, KWIQResponse *response)
{
    return response->expiresTime();
}


KWQLoader::KWQLoader(Loader *loader)
    : _requestStarted(loader,SIGNAL(requestStarted(khtml::DocLoader *, khtml::CachedObject *)))
    , _requestDone(loader, SIGNAL(requestDone(khtml::DocLoader *, khtml::CachedObject *)))
    , _requestFailed(loader, SIGNAL(requestFailed(khtml::DocLoader *, khtml::CachedObject *)))
{
}
