/*
 * Copyright (C) 2001, 2002 Apple Computer, Inc.  All rights reserved.
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
#include <string.h>

#include "KWQPtrDictImpl.h"
#include "KWQAssertions.h"
#include "KWQMap.h"

typedef void (* DeleteFunction) (void *);

class KWQPtrDictPrivate
{
public:
    KWQPtrDictPrivate(int size, DeleteFunction);
    KWQPtrDictPrivate(const KWQPtrDictPrivate &dp);
    ~KWQPtrDictPrivate();
    
    QMap<void*,void*> map;
    DeleteFunction del;
    KWQPtrDictIteratorPrivate *iterators;    
};

class KWQPtrDictIteratorPrivate
{
public:
    KWQPtrDictIteratorPrivate(KWQPtrDictPrivate *);
    ~KWQPtrDictIteratorPrivate();
    
    void remove(void *key);
    void dictDestroyed();
    
    uint count;
    uint pos;
    void **keys;
    void **values;
    KWQPtrDictPrivate *dict;
    KWQPtrDictIteratorPrivate *next;
    KWQPtrDictIteratorPrivate *prev;
};

KWQPtrDictPrivate::KWQPtrDictPrivate(int size, DeleteFunction deleteFunc)    : 
      del(deleteFunc),
      iterators(0)
{
}

KWQPtrDictPrivate::KWQPtrDictPrivate(const KWQPtrDictPrivate &dp)
    : map(dp.map),
    del(dp.del),
    iterators(0)
{
}

KWQPtrDictPrivate::~KWQPtrDictPrivate()
{
    for (KWQPtrDictIteratorPrivate *it = iterators; it; it = it->next) {
        it->dictDestroyed();
    }
}



KWQPtrDictImpl::KWQPtrDictImpl(int size, void (*deleteFunc)(void *))
    :d(new KWQPtrDictPrivate(size, deleteFunc))
{
}

KWQPtrDictImpl::KWQPtrDictImpl(const KWQPtrDictImpl &di) :
        d(new KWQPtrDictPrivate(*di.d))
{
}

KWQPtrDictImpl::~KWQPtrDictImpl()
{
    delete d;
}

void KWQPtrDictImpl::clear(bool deleteItems)
{
    if (deleteItems) {
	ASSERT(d->del);
        DeleteFunction deleteFunc = d->del;
	
	QMap<void*,void*>::Iterator iter = d->map.begin();
	
	while ( iter != d->map.end()){
	    deleteFunc(*iter);
	    ++iter;
	}
    }
    
    d->map.clear();
}

uint KWQPtrDictImpl::count() const
{
    return d->map.count();
}

void KWQPtrDictImpl::insert(void *key, const void *value)
{
    d->map.insert(key,const_cast<void*const>(value));
}

bool KWQPtrDictImpl::remove(void *key, bool deleteItem)
{
    QMap<void*,void*>::Iterator iter = d->map.find(key);
    
    if (iter == d->map.end())
	return false;

    void *value = *iter;
    
    d->map.remove(iter);
    
    if (deleteItem)
    {		
	d->del( value );
    }
    
    for (KWQPtrDictIteratorPrivate *it = d->iterators; it; it = it->next) {
        it->remove(key);
    }

    return true;
}

void *KWQPtrDictImpl::find(void *key) const
{
    return *(d->map.find(key));
}

void KWQPtrDictImpl::swap(KWQPtrDictImpl &di)
{
    KWQPtrDictPrivate *tmp;

    tmp = di.d;
    di.d = d;
    d = tmp;
}

KWQPtrDictImpl &KWQPtrDictImpl::assign(const KWQPtrDictImpl &di, bool deleteItems)
{
    KWQPtrDictImpl tmp(di);
    
    if (deleteItems) {
	clear(true);
    }

    swap(tmp);

    return *this;
}


void *KWQPtrDictImpl::take(void *key)
{
    QMap<void*,void*>::Iterator iter = d->map.find(key);
    if (iter == d->map.end())
	return 0;
    
    void *value = *iter;
    d->map.remove(iter);
    
    for (KWQPtrDictIteratorPrivate *it = d->iterators; it; it = it->next) {
        it->remove(key);
    }
    
    return value;
}

KWQPtrDictIteratorPrivate::KWQPtrDictIteratorPrivate(KWQPtrDictPrivate *d) :
    count(d->map.count()),
    pos(0),
    keys(new void * [count]),
    values(new void * [count]),
    dict(d),
    next(d->iterators),
    prev(0)
{
    d->iterators = this;
    if (next) {
        next->prev = this;
    }
    
    unsigned int i = 0;
    QMap<void*,void*>::Iterator it = d->map.begin();
    QMap<void*,void*>::Iterator end = d->map.end();    
    while (it != end) {
	keys[i] = it.key();
	values[i] = it.data();
	++i;
	++it;
    }
    ASSERT(i==count);	
}

KWQPtrDictIteratorPrivate::~KWQPtrDictIteratorPrivate()
{
    if (prev) {
        prev->next = next;
    } else if (dict) {
        dict->iterators = next;
    }
    if (next) {
        next->prev = prev;
    }
    
    delete [] keys;
    delete [] values;
}



KWQPtrDictIteratorImpl::KWQPtrDictIteratorImpl(const KWQPtrDictImpl &di)
    :d(new KWQPtrDictIteratorPrivate(di.d))
{
}

KWQPtrDictIteratorImpl::~KWQPtrDictIteratorImpl()
{
    delete d;
}

uint KWQPtrDictIteratorImpl::count() const
{
    return d->count;
}

void *KWQPtrDictIteratorImpl::current() const
{
    if (d->pos >= d->count) {
	return NULL;
    }
    return d->values[d->pos];
}

void *KWQPtrDictIteratorImpl::currentKey() const
{
    if (d->pos >= d->count) {
	return NULL;
    }
    return d->keys[d->pos];
}

void *KWQPtrDictIteratorImpl::toFirst()
{
    d->pos = 0;
    return current();
}

void *KWQPtrDictIteratorImpl::operator++()
{
    ++d->pos;
    return current();
}

void KWQPtrDictIteratorPrivate::remove(void *key)
{
    for (uint i = 0; i < count; ) {
        if (keys[i] != key) {
            ++i;
        } else {
            --count;
            if (pos > i) {
                --pos;
            }
            memmove(&keys[i], &keys[i+1], sizeof(keys[i]) * (count - i));
            memmove(&values[i], &values[i+1], sizeof(values[i]) * (count - i));
        }
    }
}

void KWQPtrDictIteratorPrivate::dictDestroyed()
{
    count = 0;
    dict = 0;
}
