/*
 * Copyright (C) 2002 Apple Computer, Inc.  All rights reserved.
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

#include <stdio.h>
#include "KWQSignal.h"

#include "KWQObject.h"
#if KWQUBE
// FIXME:KWIQ: remove logging when done with QSignal debuggin
#include "KWQLogging.h"
#else
#include "KWQAssertions.h"
#endif

#define SANDBOX
#ifndef SANDBOX

using KIO::Job;
using khtml::CachedObject;
using khtml::DocLoader;

#endif

KWQSignal::KWQSignal(QObject *object, const char *name)
    : _object(object), _next(object->_signalListHead), _name(name)
{
    object->_signalListHead = this;
}

KWQSignal::~KWQSignal()
{
    KWQSignal **prev = &_object->_signalListHead;
    KWQSignal *signal;
    while ((signal = *prev)) {
        if (signal == this) {
            *prev = _next;
            break;
        }
        prev = &signal->_next;
    }
}

void KWQSignal::connect(KWQSlot *slot)
{
//    g_printerr("KWQSignal::connect() - %s\n",_name);
#if !ERROR_DISABLED
    if (_slots.contains(slot)) {
        ERROR("connecting the same slot to a signal twice, %s", _name);
    }
#endif
    printf("KWQSignal::connect() [%s]\n", slot->version()); fflush(stdout);
    _slots.append(slot);
}

void KWQSignal::disconnect(KWQSlot *slot)
{
#if !ERROR_DISABLED
    if (!_slots.contains(slot)
            && !KWQNamesMatch(_name, SIGNAL(finishedParsing()))
            && !KWQNamesMatch(_name, SIGNAL(requestDone(khtml::DocLoader *, khtml::CachedObject *)))
            && !KWQNamesMatch(_name, SIGNAL(requestFailed(khtml::DocLoader *, khtml::CachedObject *)))
            && !KWQNamesMatch(_name, SIGNAL(requestStarted(khtml::DocLoader *, khtml::CachedObject *)))
            ) {
        ERROR("disconnecting a signal that wasn't connected, %s", _name);
    }
#endif
    _slots.remove(slot);
}

void KWQSignal::call() 
{
    printf("KWQSignal::call() - %s\n",_name);
    if (!_object->_signalsBlocked) {
        QValueListIterator<KWQSlot> begin = _slots.begin();
        QValueListIterator<KWQSlot> end = _slots.end();
        for (QValueListIterator<KWQSlot> it = begin; it != end; ++it) {
	    KWQSlot *slot = it.value();
	    slot->call();
        }
    }
}

void KWQSignal::call(bool b) 
{
    //LOG(KwiqLog,"KWQSignal::call(bool) - %s\n",_name);
    if (!_object->_signalsBlocked) {
        KWQObjectSenderScope senderScope(_object);
        QValueListPtr<KWQSlot> copiedSlots(_slots);
        QValueListConstIterator<KWQSlot> end = copiedSlots.end();
        for (QValueListConstIterator<KWQSlot> it = copiedSlots.begin(); it != end; ++it) {
	    KWQSlot *slot = (KWQSlot *)it.value();
            slot->call(b);
        }
    }

}

void KWQSignal::call(int i) 
{
//    LOG(KwiqLog,"KWQSignal::call(int) - %s\n",_name);
    if (!_object->_signalsBlocked) {
        KWQObjectSenderScope senderScope(_object);
        QValueListPtr<KWQSlot> copiedSlots(_slots);
        QValueListConstIterator<KWQSlot> end = copiedSlots.end();
        for (QValueListConstIterator<KWQSlot> it = copiedSlots.begin(); it != end; ++it) {
//            (*it).call(i);
        }
    }
}

void KWQSignal::call(const QString &s)
{
//    LOG(KwiqLog,"KWQSignal::call(QString) - %s\n",_name);
    if (!_object->_signalsBlocked) {
        KWQObjectSenderScope senderScope(_object);
        QValueListPtr<KWQSlot> copiedSlots(_slots);
        QValueListConstIterator<KWQSlot> end = copiedSlots.end();
        for (QValueListConstIterator<KWQSlot> it = copiedSlots.begin(); it != end; ++it) {
//            (*it).call(s);
        }
    }
}

#ifndef SANDBOX

void KWQSignal::call(Job *j) const
{
//    LOG(KwiqLog,"KWQSignal::call(Job*) - %s\n",_name);
    if (!_object->_signalsBlocked) {
        KWQObjectSenderScope senderScope(_object);
        QValueList<KWQSlot> copiedSlots(_slots);
        QValueListConstIterator<KWQSlot> end = copiedSlots.end();
        for (QValueListConstIterator<KWQSlot> it = copiedSlots.begin(); it != end; ++it) {
            (*it).call(j);
        }
    }
}

void KWQSignal::call(DocLoader *l, CachedObject *o) const
{
//    LOG(KwiqLog,"KWQSignal::call(DocLoader*) - %s\n",_name);
    if (!_object->_signalsBlocked) {
        KWQObjectSenderScope senderScope(_object);
        QValueList<KWQSlot> copiedSlots(_slots);
        QValueListConstIterator<KWQSlot> end = copiedSlots.end();
        for (QValueListConstIterator<KWQSlot> it = copiedSlots.begin(); it != end; ++it) {
            (*it).call(l, o);
        }
    }
}


void KWQSignal::call(Job *j, const char *d, int s) const
{
    if (!_object->_signalsBlocked) {
        KWQObjectSenderScope senderScope(_object);
        QValueList<KWQSlot> copiedSlots(_slots);
        QValueListConstIterator<KWQSlot> end = copiedSlots.end();
        for (QValueListConstIterator<KWQSlot> it = copiedSlots.begin(); it != end; ++it) {
            (*it).call(j, d, s);
        }
    }
}

void KWQSignal::call(Job *j, const KURL &u) const
{
    if (!_object->_signalsBlocked) {
        KWQObjectSenderScope senderScope(_object);
        QValueList<KWQSlot> copiedSlots(_slots);
        QValueListConstIterator<KWQSlot> end = copiedSlots.end();
        for (QValueListConstIterator<KWQSlot> it = copiedSlots.begin(); it != end; ++it) {
            (*it).call(j, u);
        }
    }
}
#if !KWQUBE
void KWQSignal::call(Job *j, void *d) const
{
    if (!_object->_signalsBlocked) {
        KWQObjectSenderScope senderScope(_object);
        QValueList<KWQSlot> copiedSlots(_slots);
        QValueListConstIterator<KWQSlot> end = copiedSlots.end();
        for (QValueListConstIterator<KWQSlot> it = copiedSlots.begin(); it != end; ++it) {
            (*it).call(j, d);
        }
    }
}
#else
void KWQSignal::call(Job *j, KWIQResponse *d) const
{
    if (!_object->_signalsBlocked) {
        KWQObjectSenderScope senderScope(_object);
        QValueList<KWQSlot> copiedSlots(_slots);
        QValueListConstIterator<KWQSlot> end = copiedSlots.end();
        for (QValueListConstIterator<KWQSlot> it = copiedSlots.begin(); it != end; ++it) {
            (*it).call(j, d);
        }
    }
}
#endif

#endif /* SANDBOX */
