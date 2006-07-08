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
#ifndef KWQPageState_h
#define KWQPageState_h

#include "KWQMap.h"

#include "KWIQPageCache.h"

class KURL;

namespace DOM {
    class DocumentImpl;
}

namespace KJS {
    class SavedProperties;
    class SavedBuiltins;
    class ScheduledAction;
}


class KWQPageState : public KWIQPageState
{
    DOM::DocumentImpl *_document;
    KURL *_URL;
    KJS::SavedProperties *_windowProperties;
    KJS::SavedProperties *_locationProperties;
    KJS::SavedBuiltins *_interpreterBuiltins;
    QMap<int, KJS::ScheduledAction*> *_pausedActions;
    void _cleanupPausedActions();
    void clear();
 public:    
    KWQPageState(DOM::DocumentImpl *doc, const KURL &u,KJS::SavedProperties *windowproperties, KJS::SavedProperties *locationproperties, KJS::SavedBuiltins * interpreterBuiltins);
    ~KWQPageState();

    DOM::DocumentImpl * document();
    KURL * URL();
    KJS::SavedProperties * windowProperties();
    KJS::SavedProperties * locationProperties();
    KJS::SavedBuiltins *interpreterBuiltins();
    void setPausedActions(QMap<int, KJS::ScheduledAction*> *pa);
    QMap<int, KJS::ScheduledAction*>* pausedActions();
    void invalidate();

};

#endif
