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

#include "KWQKHTMLPartBrowserExtension.h"
#include "khtml_part.h"
#include "WebCoreBridge.h"


KHTMLPartBrowserExtension::KHTMLPartBrowserExtension(KHTMLPart *part)
    : _part(KWQ(part)), _browserInterface(_part)
{
}

void KHTMLPartBrowserExtension::openURLRequest(const KURL &url, 
					       const KParts::URLArgs &args)
{
    if (url.protocol().lower() == "javascript") {
	_part->createEmptyDocument();
	_part->replaceContentsWithScriptResult(url);
     } else {
	_part->openURLRequest(url, args);
    }
}

void KHTMLPartBrowserExtension::openURLNotify()
{
}

void KHTMLPartBrowserExtension::createNewWindow(const KURL &url, 
						const KParts::URLArgs &urlArgs) 
{
    createNewWindow(url, urlArgs, KParts::WindowArgs(), NULL);
}

void KHTMLPartBrowserExtension::createNewWindow(const KURL &url, 
						const KParts::URLArgs &urlArgs, 
						const KParts::WindowArgs &winArgs, 
						KParts::ReadOnlyPart *&part)
{
    createNewWindow(url, urlArgs, winArgs, &part);
}

void KHTMLPartBrowserExtension::createNewWindow(const KURL &url, 
						const KParts::URLArgs &urlArgs, 
						const KParts::WindowArgs &winArgs, 
						KParts::ReadOnlyPart **partResult)
{

    WebCoreBridge *bridge;

    if (!urlArgs.frameName.isEmpty()) {
        // FIXME: is utf8 right format here? -- psalmi
	bridge = _part->bridge()->findFrameNamed(urlArgs.frameName.utf8());
	if (bridge) {
	    if (!url.isEmpty()) {
		bridge->loadURL(url.url().utf8(), _part->bridge()->referrer(), urlArgs.reload, false, 0,0,0,0);
	    }
	    bridge->focusWindow();
	    if (partResult) *partResult = bridge->part();
	    return;
	}
    }

    // FIXME: is utf8 right one here -- psalmi
    bridge = _part->bridge()->createWindowWithURL(url.url().utf8(), urlArgs.frameName.utf8());
    if (!bridge) {
	if (partResult) 
	    *partResult = 0;
	return;
    }
    
   
    if (!winArgs.toolBarsVisible) {
	bridge->setToolbarsVisible(false);
    }
    
    if (!winArgs.statusBarVisible) {
	bridge->setStatusBarVisible(false);
    }
    
    if (!winArgs.scrollbarsVisible) {
	bridge->setScrollbarsVisible(false);
    }
    
    if (!winArgs.resizable) {
	bridge->setWindowIsResizable(false);
    }
    
    if (winArgs.xSet || winArgs.ySet || winArgs.widthSet || winArgs.heightSet) {
        GdkRectangle * frame = g_new0(GdkRectangle, 1);
	bridge->windowFrame(frame);
        GdkRectangle * contentRect = g_new0(GdkRectangle, 1);
        bridge->windowContentRect(contentRect);

	if (winArgs.xSet) {
	    frame->x = winArgs.x;
	}
	
	if (winArgs.ySet) {
	    //float heightForFlip = NSMaxY([[[NSScreen screens] objectAtIndex:0] frame]);
	    //frame.origin.y = heightForFlip - (winArgs.y + frame.size.height);
	    frame->y = winArgs.y + frame->height;
	}
	
	if (winArgs.widthSet) {
	    frame->width = frame->width + winArgs.width - contentRect->width;
	}
	
	if (winArgs.heightSet) {
	    float heightDelta = winArgs.height - contentRect->height;
	    frame->height = int( frame->height +  heightDelta );
	    frame->y = int( frame->y - heightDelta );
	}

        bridge->setWindowFrame(frame);
    }
    
    bridge->showWindow();
    
    if (partResult) *partResult = bridge->part();
}

void KHTMLPartBrowserExtension::setIconURL(const KURL &url)
{
    // FIXME: utf8??  -- psalmi
    _part->bridge()->setIconURL(url.url().utf8());
}

void KHTMLPartBrowserExtension::setTypedIconURL(const KURL &url, const QString &type)
{
    // FIXME: utf8??  -- psalmi
    _part->bridge()->setIconURL(url.url().utf8(), type.utf8());
}
