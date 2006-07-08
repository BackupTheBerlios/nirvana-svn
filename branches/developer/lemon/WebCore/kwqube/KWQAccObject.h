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

namespace DOM {
    class HTMLAreaElementImpl;
}

namespace khtml {
    class RenderObject;
}

class KWQAccObject
{
    khtml::RenderObject* m_renderer;
    DOM::HTMLAreaElementImpl* m_areaElement;
    void* m_data;
 public:
    KWQAccObject(khtml::RenderObject* renderer) :m_renderer(renderer), m_data(0) {};
    bool detached() {return m_renderer != 0;};
    void detach() { m_renderer = 0; };
    void* data() { return m_data; }
    void setData(void* data) { m_data=data;};
    KWQAccObject *firstChild(){return 0;};
    KWQAccObject *lastChild(){return 0;}
    KWQAccObject *previousSibling(){return 0;}
    KWQAccObject *nextSibling(){return 0;}
    KWQAccObject *parentObject(){return 0;}   
 
};

