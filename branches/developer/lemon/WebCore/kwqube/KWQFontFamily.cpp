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

#include "KWQFontFamily.h"
#include "xml/dom_stringimpl.h"

using DOM::AtomicString;
using DOM::DOMStringImpl;

KWQFontFamily::KWQFontFamily()
    : _next(0)
    , _refCnt(0)
    , _NSFamily(0)
{
}

KWQFontFamily::KWQFontFamily(const KWQFontFamily& other)
    : _family(other._family)
    , _next(other._next)
    , _refCnt(0)
    , _NSFamily(other._NSFamily)
{
    if (_next)
        _next->ref();
}

KWQFontFamily& KWQFontFamily::operator=(const KWQFontFamily& other)
{
    if (other._next)
        other._next->ref();
    if (_next)
        _next->deref();
    _family = other._family;
    _next = other._next;
    _NSFamily = other._NSFamily;
    return *this;
}

// Wrapper class to destroy GHashTable when it goes out of scope
// (reduces valgrind errors with static tables)
struct GHashHolder
{
    GHashTable* table;
    GHashHolder(GHashTable* atable)
	: table(atable)
    {	
    }

    ~GHashHolder()
    {
	g_hash_table_destroy(table);
    }
};

extern "C"{
void families_keys_destroy(gpointer data)
{
    DOMStringImpl* value = static_cast<DOMStringImpl*>(data);
    value->deref();    
}

void families_values_destroy(gpointer data)
{
    gchar* str = static_cast<gchar*>(data);
    g_free(str);
}

};
// should the atomic string be reffed?
const gchar* KWQFontFamily::getNSFamily() const
{
    if (!_NSFamily) {
        // Use an immutable copy of the name, but keep a set of
        // all family names so we don't end up with too many objects.
        static GHashHolder families(g_hash_table_new_full(g_direct_hash,
							  g_direct_equal, 
							  families_keys_destroy, 
							  families_values_destroy));
	
        _NSFamily = static_cast<gchar*>(g_hash_table_lookup(families.table, _family.implementation()));
        if (!_NSFamily) {
            _NSFamily = g_strdup(_family.string().utf8());
	    _family.implementation()->ref();
            g_hash_table_insert(families.table,_family.implementation() , _NSFamily); 
        }
    }
    return _NSFamily;
}


void KWQFontFamily::setFamily(const AtomicString &family)
{
    _family = family;
    _NSFamily = 0;
}

bool KWQFontFamily::operator==(const KWQFontFamily &compareFontFamily) const
{
    if ((!_next && compareFontFamily._next) || 
        (_next && !compareFontFamily._next) ||
        ((_next && compareFontFamily._next) && (*_next != *(compareFontFamily._next)))) {
        return false;
    }

    return _family==compareFontFamily._family;
}
