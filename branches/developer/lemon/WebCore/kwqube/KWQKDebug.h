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

#ifndef KDEBUG_H_
#define KDEBUG_H_

#include <stdlib.h>
#include <stdio.h>

#include "KWQString.h"
#include "KWQTextStream.h"

class kdbgstream;

typedef kdbgstream & (*KDBGFUNC)(kdbgstream &);
#if NDEBUG
class kdbgstream {
public:
    kdbgstream(unsigned int area = 0, unsigned int level = 0, bool print = true) { }

    kdbgstream &operator<<(int) { return *this; }
    kdbgstream &operator<<(unsigned int) { return *this; }
    kdbgstream &operator<<(long) { return *this; }
    kdbgstream &operator<<(double) { return *this; }
    kdbgstream &operator<<(const char *) { return *this; }
    kdbgstream &operator<<(const void *) { return *this; }
    kdbgstream &operator<<(const QString &) { return *this; }
    kdbgstream &operator<<(const QCString &) { return *this; }
    kdbgstream &operator<<(const time_t&) { return *this; }
    kdbgstream &operator<<(KDBGFUNC) { return *this; }
};

inline kdbgstream &endl(kdbgstream &s) { return s; }

inline kdbgstream kdDebug(int area = 0) { return kdbgstream(); }
inline kdbgstream kdWarning(int area = 0) { return kdbgstream(); }
inline kdbgstream kdWarning(bool cond, int area = 0) { return kdbgstream(); }
inline kdbgstream kdError(int area = 0) { return kdbgstream(); }
inline kdbgstream kdError(bool cond, int area = 0) { return kdbgstream(); }
inline kdbgstream kdFatal(int area = 0) { return kdbgstream(); }
inline kdbgstream kdFatal(bool cond, int area = 0) { return kdbgstream(); }
inline QString kdBacktrace() { return QString(); }

#else


class kdbgstream {
public:
    kdbgstream(unsigned int area = 0, unsigned int level = 0, bool print = true) { }
#if KWQUBE
    kdbgstream &operator<<(int a) { printf("%d",a); return *this; }
    kdbgstream &operator<<(unsigned int a) { printf("%d", a);return *this; }
    kdbgstream &operator<<(time_t&) {return *this; }
    kdbgstream &operator<<(double d) { printf("%f",(float) d);  return *this; }
    kdbgstream &operator<<(const char *str) { printf("%s", str);  return *this; }
    kdbgstream &operator<<(const void *) {  return *this; }
    kdbgstream &operator<<(const QString &qstr) { printf("%s",qstr.latin1());  return *this; }
    kdbgstream &operator<<(const QCString &qstr) { printf("%s",qstr.data());  return *this; }
    kdbgstream &operator<<(const time_t&t) { printf("%d", (int) t); return *this; }
    kdbgstream &operator<<(KDBGFUNC f) {  return f(*this); }
#endif
};
#if KWQUBE
inline kdbgstream &endl(kdbgstream &s) { printf("\n"); return s; }
#else
inline kdbgstream &endl(kdbgstream &s) { return s; }
#endif
inline kdbgstream kdDebug(int area = 0) { return kdbgstream(); }
inline kdbgstream kdWarning(int area = 0) { return kdbgstream(); }
inline kdbgstream kdWarning(bool cond, int area = 0) { return kdbgstream(); }
inline kdbgstream kdError(int area = 0) { return kdbgstream(); }
inline kdbgstream kdError(bool cond, int area = 0) { return kdbgstream(); }
inline kdbgstream kdFatal(int area = 0) { return kdbgstream(); }
inline kdbgstream kdFatal(bool cond, int area = 0) { return kdbgstream(); }
inline QString kdBacktrace() { return QString(); }

#endif

#endif
