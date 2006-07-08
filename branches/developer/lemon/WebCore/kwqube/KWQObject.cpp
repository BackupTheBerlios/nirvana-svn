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
#include "KWQObject.h"

#include "KWQVariant.h"
#include "KWQAssertions.h"
#include "KWQPtrList.h"
#include "KWQPtrDict.h"

class KWQObjectTimerTarget;
typedef QPtrList<KWQObjectTimerTarget> PausedTimerList;

const QObject *QObject::_sender;
bool QObject::_defersTimers = false;

static QPtrDict<PausedTimerList> allPausedTimers;
static QPtrList<KWQObjectTimerTarget> deferredTimers;
static bool deferringTimers = false;


struct _initer {
    _initer() {
	allPausedTimers.setAutoDelete(true);
    }
};
static _initer _hack_initer;


class KWQObjectTimerTarget
{
    GTimeVal firesAt;
    guint sid; // GSource id

public:
    QObject* target;
    guint remainingTime; 
    int timerId;
    guint interval; // in millisec

    KWQObjectTimerTarget(QObject* target, int timerId);
    ~KWQObjectTimerTarget();

    // timer interface
    void scheduleWithInterval(int intervalMS);
    void scheduleWithFractionInterval(int firstIntervalMS, int intervalMS);
    void invalidate();
    GTimeVal* fireTime() { return &firesAt; }

    void sendTimerEvent();
    void timerFired();

private:
    void addTimeout(guint intervalMS, GSourceFunc func, gpointer data);
};

KWQSignal *QObject::findSignal(const char *signalName) const
{
    for (KWQSignal *signal = _signalListHead; signal; signal = signal->_next) {
        if (KWQNamesMatch(signalName, signal->_name)) {
            return signal;
        }
    }
    return 0;
}


void QObject::connect(const QObject *sender, const char *signalName, const QObject *receiver, const char *member)
{
    // FIXME: Assert that sender is not NULL rather than doing the if statement.
    if (!sender) {
        return;
    }
    KWQSignal *signal = sender->findSignal(signalName);
    if (!signal) {
#if !ERROR_DISABLED
        if (1
            && !KWQNamesMatch(member, SIGNAL(setStatusBarText(const QString &)))
            && !KWQNamesMatch(member, SLOT(slotHistoryChanged()))
            && !KWQNamesMatch(member, SLOT(slotJobPercent(KIO::Job *, unsigned long)))
            && !KWQNamesMatch(member, SLOT(slotJobSpeed(KIO::Job *, unsigned long)))
            && !KWQNamesMatch(member, SLOT(slotScrollBarMoved()))
            && !KWQNamesMatch(member, SLOT(slotShowDocument(const QString &, const QString &)))
            && !KWQNamesMatch(member, SLOT(slotViewCleared())) // FIXME: Should implement this one!
            )
	ERROR("connecting member %s to signal %s, but that signal was not found", member, signalName);
#endif
        return;
    }
        
    signal->connect(KWQSlot(const_cast<QObject *>(receiver), member));
}

void QObject::disconnect(const QObject *sender, const char *signalName, const QObject *receiver, const char *member)
{
    // FIXME: Assert that sender is not NULL rather than doing the if statement.
    if (!sender)
        return;
    
    KWQSignal *signal = sender->findSignal(signalName);
    if (!signal) {
        // FIXME: ERROR
        return;
    }
    signal->disconnect(KWQSlot(const_cast<QObject *>(receiver), member));
}

KWQObjectSenderScope::KWQObjectSenderScope(const QObject *o)
    : _savedSender(QObject::_sender)
{
    QObject::_sender = o;
}

KWQObjectSenderScope::~KWQObjectSenderScope()
{
    QObject::_sender = _savedSender;
}

QObject::QObject(QObject *parent, const char *name)
    : _signalListHead(0), _signalsBlocked(false)
    , _destroyed(this, SIGNAL(destroyed()))
    , _eventFilterObject(0)
    , _class_type(0)
{
    QOBJECT_TYPE(QObject);
    _guardedPtrDummyList.append(this);
    _timers.setAutoDelete(false);
}

QObject::~QObject()
{
    _destroyed.call();
    ASSERT(_signalListHead == &_destroyed);
    killTimers();
}

void QObject::timerEvent(QTimerEvent *te)
{
}

bool QObject::event(QEvent *)
{
    return false;
}

KWQObjectTimerTarget* _find(QPtrList<KWQObjectTimerTarget> *l, int id)
{
    QPtrListIterator<KWQObjectTimerTarget> it(*l);
    KWQObjectTimerTarget *p = 0;

    while((p=it.current())) {
	if (p->timerId == id) 
	    return p;	
	++it;
    }

    return p;    
}

void QObject::pauseTimer (int timerId, const void *key)
{
    KWQObjectTimerTarget* target = _find(&_timers, timerId);
    if (target) {
	GTimeVal *tv = target->fireTime();
	GTimeVal diff;
	g_get_current_time(&diff);

	diff.tv_sec = tv->tv_sec - diff.tv_sec;
	diff.tv_usec = tv->tv_usec - diff.tv_usec;
	
	PausedTimerList* pausedTimers = allPausedTimers.find(const_cast<void*>(key));

	if (!pausedTimers) {
	    pausedTimers = new PausedTimerList;
	    allPausedTimers.insert(const_cast<void*>(key), pausedTimers);
	}
		
	if (diff.tv_sec < 0)
	    diff.tv_sec = 0;
	if (diff.tv_usec < 0)
	    diff.tv_usec =0;

	target->remainingTime = (diff.tv_sec*1000) + (diff.tv_usec/1000);
	
	pausedTimers->append(target);

	target->invalidate();
	_timers.removeRef(target);
    }
   
}

static int nextTimerID = 1;

void QObject::clearPausedTimers (const void *key)
{
    PausedTimerList* pausedTimers = allPausedTimers.find(const_cast<void*>(key));
    if (pausedTimers) {
	pausedTimers->setAutoDelete(true); //delete all paused timers
	allPausedTimers.remove(const_cast<void*>(key)); // pausedTimers entry deleted 
    }
}

void QObject::resumeTimers (const void *key, QObject *_target)
{
    PausedTimerList* pausedTimers = allPausedTimers.find(const_cast<void*>(key));

    if (!pausedTimers)
	return;

    int maxId = MAX(0, nextTimerID);

    QPtrListIterator<KWQObjectTimerTarget> it(*pausedTimers);
    KWQObjectTimerTarget *target = 0;

    while((target=it.current())) {
	target->target = _target;
	target->scheduleWithFractionInterval(target->remainingTime, target->interval);

	maxId = MAX(maxId, target->timerId);

	_timers.append(target);

	++it;
    }
    nextTimerID = maxId+1;

    allPausedTimers.remove(const_cast<void*>(key));  // pausedTimers entry deleted, do not delete contents
}

int QObject::startTimer(int milliseconds)
{
    KWQObjectTimerTarget *target = new KWQObjectTimerTarget(this, nextTimerID);
   
    target->scheduleWithInterval(milliseconds);
    
    _timers.append(target);

    return nextTimerID++;
}

void QObject::killTimer(int timerId)
{
    if (timerId == 0) {
        return;
    }

    KWQObjectTimerTarget *p = _find(&_timers, timerId);
    if (p) {
	deferredTimers.removeRef(p);
	_timers.removeRef(p); 
	delete p; 
    } 
}

void QObject::killTimers()
{
    deferredTimers.clear(); // not deleted
    _timers.setAutoDelete(true);
    _timers.clear(); 
    _timers.setAutoDelete(false);
}

static void stopDeferringTimers()
{
    ASSERT(deferringTimers);
    deferredTimers.first();
    while (deferredTimers.current() != 0) {
	// remove before sending the timer event, in case the timer
	// callback cancels the timer - we don't want to remove too
	// much in that case.
	KWQObjectTimerTarget *timerTarget = deferredTimers.take();
        timerTarget->sendTimerEvent();
    }

    deferringTimers = false;
}

void QObject::setDefersTimers(bool defers)
{
    if (defers) {
        _defersTimers = true;
        deferringTimers = true;
	// NOTE: apples version is async:
        // [NSObject cancelPreviousPerformRequestsWithTarget:[KWQObjectTimerTarget class]];
        return;
    }
    
    if (_defersTimers) {
        _defersTimers = false;
        if (deferringTimers) {
	    // NOTE: Apples' version is async:
	    // [KWQObjectTimerTarget performSelector:@selector(stopDeferringTimers) withObject:nil afterDelay:0];
	    stopDeferringTimers();
        }
    }
}

extern "C" { 
static gboolean 
frac_interval_timeout(gpointer data)
{
    KWQObjectTimerTarget *p = static_cast<KWQObjectTimerTarget*>(data);
    p->scheduleWithInterval(p->interval);
    p->timerFired(); 
    return FALSE; // remove source
    
}
static gboolean 
interval_timeout(gpointer data)
{
    KWQObjectTimerTarget *p = static_cast<KWQObjectTimerTarget*>(data);
    p->timerFired(); 
    return TRUE; // don't remove source
}
}

KWQObjectTimerTarget::KWQObjectTimerTarget(QObject* t, int _timerId)
    :sid(0)
     ,target(t)
     ,timerId(_timerId)

     ,interval(0)
{    
}

KWQObjectTimerTarget::~KWQObjectTimerTarget()
{
    invalidate();
}

void KWQObjectTimerTarget::addTimeout(guint intervalMS, GSourceFunc func, gpointer data)
{
    glong secs = (glong) (intervalMS/1000);
    glong usecs = (glong) ((intervalMS-(secs*1000)))*1000;

    g_get_current_time(&firesAt);
    firesAt.tv_sec +=secs;
    firesAt.tv_usec +=usecs;

    // "If interval is 0, then the timer event occurs once every time there are no 
    // more window system events to process." --QT API Reference
    if (interval==0) 
	sid = g_idle_add_full(G_PRIORITY_HIGH_IDLE, func, data, NULL);    
    else 
	sid = g_timeout_add_full(G_PRIORITY_HIGH_IDLE, intervalMS, func, data, NULL);
}

void KWQObjectTimerTarget::scheduleWithInterval(int intervalMS)
{
    invalidate();
    interval = intervalMS;
    addTimeout(intervalMS,  (GSourceFunc) ::interval_timeout, this);
}

void KWQObjectTimerTarget::scheduleWithFractionInterval(int firstIntervalMS, int intervalMS)
{
    invalidate();
    interval = intervalMS;
    addTimeout(firstIntervalMS, (GSourceFunc)::frac_interval_timeout, this);    
}

void KWQObjectTimerTarget::invalidate()
{
    if (sid!=0)
	g_source_remove(sid);    
}

void KWQObjectTimerTarget::sendTimerEvent()
{
    QTimerEvent event(timerId);
    target->timerEvent(&event);
}

void KWQObjectTimerTarget::timerFired()
{
    if (deferringTimers) {
	if (!deferredTimers.containsRef(this)) {
	    deferredTimers.append(this);
	}
    } else {
        sendTimerEvent();
    }
}

bool QObject::inherits(const char *className) const
{
    if (this==0) return false;
    
    if (strcmp(className, "KHTMLPart") == 0) {
	return QOBJECT_IS_A(this, KHTMLPart);
    }
    if (strcmp(className, "KHTMLView") == 0) {
	return QOBJECT_IS_A(this, KHTMLView);	
    }
    if (strcmp(className, "KParts::Factory") == 0) {
	return QOBJECT_IS_A(this, KParts::Factory);
    }
    if (strcmp(className, "KParts::ReadOnlyPart") == 0) {
	return QOBJECT_IS_A(this, KParts::ReadOnlyPart);
    }
    
    if (strcmp(className, "QFrame") == 0) {
	return QOBJECT_IS_A(this, QFrame);
    }
    if (strcmp(className, "QScrollView") == 0) {
	return QOBJECT_IS_A(this, QScrollView);
    }
    
    ERROR("class name %s not recognized", className);
    return false;
}
