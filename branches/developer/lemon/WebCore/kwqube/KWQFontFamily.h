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

#ifndef QFONTFAMILY_H
#define QFONTFAMILY_H

#include "KWQString.h"
#include "glib.h"

// #include "xml/dom_atomicstring.h"
// KWIQ: NSStrings here refer to gchar*

// LEMON: There is no QFontFamily in Qt
// but it is present in Cocoa. So it is
// historical decision by Apple.

class KWQFontFamily {
public:
    KWQFontFamily();
    ~KWQFontFamily() { if (_next) _next->deref();  }
    KWQFontFamily(const KWQFontFamily &);    
    KWQFontFamily &operator=(const KWQFontFamily &);
    
    // LEMON - I prefer to remove dependence from upper KDE layer
    // void setFamily(const DOM::AtomicString &);
    // const DOM::AtomicString& family() const { return _family; }
    void setFamily(const QString &);
    const QString& family() const { return _family; }
    QString* getFamilyRef() { return &_family; } 
    bool familyIsEmpty() const { return _family.isEmpty(); }
    const gchar* getNSFamily() const;
    KWQFontFamily *next() { return _next; }
    const KWQFontFamily *next() const { return _next; }
    void appendFamily(KWQFontFamily *family) 
    {
        if (family) family->ref();
        if (_next) _next->deref(); 
        _next = family; 
    }
    
    bool operator==(const KWQFontFamily &) const;
    bool operator!=(const KWQFontFamily &x) const { return !(*this == x); }
    
    void ref() { _refCnt++; }
    void deref() { _refCnt--; if (_refCnt == 0) delete this; }
    
private:
    // LEMON - unexpected dependance removal
    // DOM::AtomicString _family;
    QString _family;
    KWQFontFamily *_next;
    int _refCnt;
    mutable gchar *_NSFamily;
};


// Macro to create a stack array containing non-retained NSString names
// of CSS font families.  This can be used to avoid allocations in
// performance critical code.  Create a NSSString ** name families
// and populates with a NSString * for each family name.  Null terminates
// the array.
#define CREATE_FAMILY_ARRAY(font,families)\
int __numFamilies = 0;\
{\
    const KWQFontFamily *__ff = (font).firstFamily();\
    while (__ff)\
    {\
        __numFamilies++;\
        __ff = __ff->next();\
    }\
}\
const gchar* families[__numFamilies+1];\
{\
    int __i = 0;\
    const KWQFontFamily *__ff = (font).firstFamily();\
    while (__ff)\
    {\
        families[__i++] = __ff->getNSFamily();\
        __ff = __ff->next();\
    }\
    families[__i] = 0;\
}

#endif
