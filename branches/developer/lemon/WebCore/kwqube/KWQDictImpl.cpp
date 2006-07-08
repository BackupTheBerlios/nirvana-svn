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

#include "KWQDictImpl.h"
#include <CoreFoundation/CoreFoundation.h>
/*
 * No KWQDictImpl::~KWQDictImpl() because QDict::~QDict calls KWQDictImpl::clear()
 * on 
 */
KWQDictImpl::KWQDictImpl(int size, bool caseSensitive, void (*deleteFunc_)(void *))
    : deleteFunc(deleteFunc_)
    , modifyCase(!caseSensitive)
{

}

void KWQDictImpl::insert(const QString &key, const void *value)
{
    if (modifyCase)
	map.insert(key.lower(), const_cast<void*>(value));
    else
	map.insert(key, const_cast<void*>(value) );
}

bool KWQDictImpl::remove(const QString &key, bool deleteItem)
{
    QMapIterator<QString, void*> i;
    void* data;

    if (modifyCase)
	i = map.find(key.lower());
    else
	i = map.find(key);
    
    if (i == map.end())
	return false;
    
    data = *i;

    map.remove(i);    
    if (deleteItem && deleteFunc) {
      deleteFunc(data);
      return true;
    }
    return false;
}

void KWQDictImpl::clear(bool deleteItem)
{
    if (deleteItem)
    {	
	QMapIterator<QString,void*> i = map.begin();
	QMapIterator<QString,void*> end = map.end();
	void *data;
	while (i!=end)
	{
	    data=*i;
	    if (deleteFunc) deleteFunc(data);
	    ++i;
	}
    }

    map.clear();
}

uint KWQDictImpl::count() const
{
    return map.count();
}
    
void *KWQDictImpl::find(const QString &key) const
{
    QMapConstIterator<QString,void*> i;
    if (modifyCase)
	i = map.find(key.lower());
    else
	i = map.find(key);

    if (i == map.end())
	return 0;
    return *i;
}

uint KWQDictIteratorImpl::count() const
{
    return dict->map.count();
}

void* KWQDictIteratorImpl::current() const
{
    if (i == dict->map.end())
	return 0;
    return *i;
}

void* KWQDictIteratorImpl::toFirst()
{
    i=dict->map.begin();
    if (i == dict->map.end())
	return 0;
    
    return *i;
}
void* KWQDictIteratorImpl::operator++()
{
    ++i;
    if (i==dict->map.end())
	return 0;
    return *i;
}

QString KWQDictIteratorImpl::currentStringKey() const
{
    if (i == dict->map.end() )
	return QString();

    return QString(i.key());
}

