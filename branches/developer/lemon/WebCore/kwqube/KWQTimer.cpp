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
#include <glib.h>

#include "KWQTimer.h"
#include "KWQAssertions.h"

QTimer::QTimer()
    : m_timer(0), m_single(false), m_monitorFunction(0), m_timeoutSignal(this, SIGNAL(timeout()))
{
}

bool QTimer::isActive() const
{
    return m_timer!=0;
}

extern "C" {
static bool
timeout_cb(gpointer data)
{
    QTimer *this_ = static_cast<QTimer*>(data);
    
    if (!this_->isActive())
	return FALSE;

    this_->fire();

    if (!this_->isActive())
	return FALSE;

    return TRUE;
}
}

void QTimer::start(int msec, bool singleShot)
{
    stop();
    
    m_single = singleShot;
    
    if (msec == 0)
	m_timer = g_idle_add_full(G_PRIORITY_DEFAULT,
				  (GSourceFunc) &::timeout_cb,
				  this,
				  NULL);
    else	
	m_timer = g_timeout_add_full(G_PRIORITY_DEFAULT, 
				     msec,
				     (GSourceFunc) &::timeout_cb,
				     this,
				     NULL);    

    if (m_monitorFunction) {
        m_monitorFunction(m_monitorFunctionContext);
    }
}

void QTimer::stop()
{
    if (m_timer == 0) 
        return;
    
    gboolean s = g_source_remove(m_timer);
    ASSERT(s);
    
    m_timer = 0;

    if (m_monitorFunction) {
        m_monitorFunction(m_monitorFunctionContext);
    }
    
}

void QTimer::setMonitor(void (*monitorFunction)(void *context), void *context)
{
    ASSERT(!m_monitorFunction || (monitorFunction == 0));
    m_monitorFunction = monitorFunction;
    m_monitorFunctionContext = context;
}

void QTimer::fire()
{
    if (m_single) 
	m_timer = 0;

    m_timeoutSignal.call();
}

extern "C" {
static bool
singleshot_cb(gpointer user_data)
{    
    ASSERT(user_data);
    KWQSlot* slot = static_cast<KWQSlot*>(user_data);

    slot->call();
    delete slot;

    return FALSE;
}
}

void QTimer::singleShot(int msec, QObject *receiver, const char *member)
{
    KWQSlot* slot = new KWQSlot(receiver, member);

    if (msec==0)
	g_idle_add_full(G_PRIORITY_DEFAULT_IDLE,
			(GSourceFunc) &::singleshot_cb,
			slot,
			NULL);
    else	
	g_timeout_add_full(G_PRIORITY_DEFAULT, 
			   msec,
			   (GSourceFunc)&::singleshot_cb,
			   slot,
			   NULL);
}

