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

#include "KWQPageState.h"

#include <JavaScriptCore/property_map.h>
#include <JavaScriptCore/interpreter.h>
#include "kjs_window.h"

#include "dom_docimpl.h"
#include "khtmlview.h"


#include "KWQAssertions.h"
#include "KWQKHTMLPart.h"

using DOM::DocumentImpl;

using khtml::RenderObject;

using KJS::SavedProperties;
using KJS::SavedBuiltins;

KWQPageState::KWQPageState(DocumentImpl *doc, const KURL &u, SavedProperties * wp, SavedProperties *lp, SavedBuiltins * interpreterBuiltins)
{
    ASSERT(doc);
    ASSERT(doc->view());
    ASSERT(!doc->inPageCache());

    doc->ref();
    _document = doc;
    _document->setInPageCache(true);
    _document->view()->ref();
    _URL = new KURL(u);
    _windowProperties = wp;
    _locationProperties = lp;
    _interpreterBuiltins = interpreterBuiltins;
}

void KWQPageState::setPausedActions(QMap<int, KJS::ScheduledAction*> *pa)
{
    _pausedActions = pa;
}

QMap<int, KJS::ScheduledAction*> * KWQPageState::pausedActions()
{
    return _pausedActions;
}

void KWQPageState::_cleanupPausedActions()
{
    if (_pausedActions){
        QMapIterator<int,KJS::ScheduledAction*> it = _pausedActions->begin();        
	QMapIterator<int,KJS::ScheduledAction*> end = _pausedActions->end();
        for (; it != end; ++it) {
            KJS::ScheduledAction *action = *it;
            delete action;
        }
        delete _pausedActions;
        _pausedActions = 0;
    }

    QObject::clearPausedTimers(this);
}

// Called when the KWQPageState is restored.  It relinquishs ownership
// of objects to core.
void KWQPageState::clear()
{
    _document = 0;

    delete _URL;
    _URL = 0;
    delete _windowProperties;
    _windowProperties = 0;
    delete _locationProperties;
    _locationProperties = 0;
    delete _interpreterBuiltins;
    _interpreterBuiltins = 0;
    _cleanupPausedActions();    
}

void KWQPageState::invalidate()
{
    // Should only ever invalidate once.
    ASSERT(_document);
    ASSERT(!_document->inPageCache());

    _document->view()->deref();
    _document->deref();

    clear();
}   

KWQPageState::~KWQPageState()
{
    if (_document) {
        ASSERT(_document->inPageCache());
        ASSERT(_document->view());
	
        KHTMLView *view = _document->view();

        KWQKHTMLPart::clearTimers(view);

        bool detached = _document->renderer() == 0;	
        _document->setInPageCache(false);

        if (detached) {
            _document->detach();
        }
        _document->deref();
        
        if (view) {
            view->clearPart();
	    view->deref();
        }
    }

    clear();
}

DocumentImpl * KWQPageState::document()
{
    return _document;
}

KURL * KWQPageState::URL()
{
    return _URL;
}

SavedProperties * KWQPageState::windowProperties()
{
    return _windowProperties;
}

SavedProperties * KWQPageState::locationProperties()
{
    return _locationProperties;
}

SavedBuiltins * KWQPageState::interpreterBuiltins()
{
    return _interpreterBuiltins;
}
