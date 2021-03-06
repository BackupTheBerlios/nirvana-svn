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

#ifndef KWQSLOT_H
#define KWQSLOT_H

#include "KWQGuardedPtr.h"

class QString;

#define SANDBOX
#ifndef SANDBOX

class KURL;

namespace KIO {
    class Job;
}

namespace khtml {
    class CachedObject;
    class DocLoader;
}
#if KWQUBE
class KWIQResponse;
#endif

#endif

// Like strcmp, but ignores spaces.
bool KWQNamesMatch(const char *a, const char *b);

class KWQSlot {
public:
    KWQSlot() : m_object(0) { }
    KWQSlot(QObject *, const char *member);
    
    virtual char *version() const { return "KWQSlot"; }
    bool isEmpty() const { return !m_object; }
    void clear() { m_object = 0; }

    virtual void call();
    virtual void call(bool);
    virtual void call(int);
    virtual void call(const QString &);

#ifndef SANDBOX    
    void call(KIO::Job *) const;
    void call(khtml::DocLoader *, khtml::CachedObject *) const;
    void call(KIO::Job *, const char *, int) const;
    void call(KIO::Job *, const KURL &) const;
#if !KWQUBE
    void call(KIO::Job *, void *) const;
#else
    void call(KIO::Job *, KWIQResponse *) const;
#endif

#endif /* SANDBOX */

    friend bool operator==(const KWQSlot &, const KWQSlot &);

//protected: // LEMON: private -> protected
    QGuardedPtr<QObject> m_object;
    int m_function;
};

inline bool operator!=(const KWQSlot &a, const KWQSlot &b) { return !(a == b); }

#endif
