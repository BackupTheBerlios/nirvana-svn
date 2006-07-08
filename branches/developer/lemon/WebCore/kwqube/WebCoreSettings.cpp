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

#include "WebCoreSettings.h"

#include "KWQKHTMLPart.h"
#include "KWQKHTMLSettings.h"
#include "WebCoreBridge.h"

#include <math.h> // rint()

WebCoreSettings::WebCoreSettings()
{
    ksettings = new KHTMLSettings();
}

WebCoreSettings::~WebCoreSettings()
{
    delete ksettings;    
}


void WebCoreSettings::_updateAllViews()
{
    for (QPtrListIterator<KWQKHTMLPart> it(KWQKHTMLPart::instances()); it.current(); ++it) {
        KWQKHTMLPart *part = it.current();
        if (part->settings() == ksettings) {
            part->bridge()->setNeedsReapplyStyles();
        }
    }
}

void WebCoreSettings::setStandardFontFamily(const gchar* f)
{
    QString s(f);
    if (ksettings->stdFontName() == s) {
        return;
    }
    ksettings->setStdFontName(s);
    _updateAllViews();
}

const gchar* WebCoreSettings::standardFontFamily()
{
    return ksettings->stdFontName().latin1();
}

void WebCoreSettings::setFixedFontFamily(const gchar *f)
{
    QString s(f);    
    if (ksettings->fixedFontName() == s) {
        return;
    }
    ksettings->setFixedFontName(QString(s));
    _updateAllViews();
}

const gchar* WebCoreSettings::fixedFontFamily()
{
    return ksettings->fixedFontName().latin1();
}

void WebCoreSettings::setSerifFontFamily(const gchar* f)
{
    QString s(f);
    if (ksettings->serifFontName() == s) {
        return;
    }

    ksettings->setSerifFontName(s);
    _updateAllViews();
}

const gchar* WebCoreSettings::serifFontFamily()
{
    return ksettings->serifFontName().latin1();
}

void WebCoreSettings::setSansSerifFontFamily(const gchar* f)
{
    QString s(f);
    if (ksettings->sansSerifFontName() == s) {
        return;
    }
    ksettings->setSansSerifFontName(s);
    _updateAllViews();
}

const gchar* WebCoreSettings::sansSerifFontFamily()
{
    return ksettings->sansSerifFontName().latin1();
}

void WebCoreSettings::setCursiveFontFamily(const gchar* f)
{
    QString s(f);
    if (ksettings->cursiveFontName() == s) {
        return;
    }
    ksettings->setCursiveFontName(s);
    _updateAllViews();
}

const gchar* WebCoreSettings::cursiveFontFamily()
{
    return ksettings->cursiveFontName().latin1();
}

void WebCoreSettings::setFantasyFontFamily(const gchar* f)
{
    QString s(f);
    if (ksettings->fantasyFontName() == s) {
        return;
    }
    ksettings->setFantasyFontName(s);
    _updateAllViews();
}

const gchar* WebCoreSettings::fantasyFontFamily()
{
    return ksettings->fantasyFontName().latin1();
}

void WebCoreSettings::setMinimumFontSize(float size)
{
    if (ksettings->minFontSize() == size) {
        return;
    }

    ksettings->setMinFontSize((int)rint(size));
    _updateAllViews();
}

float WebCoreSettings::minimumFontSize()
{
    return ksettings->minFontSize();
}

void WebCoreSettings::setMinimumLogicalFontSize(float size)
{
    if (ksettings->minLogicalFontSize() == size) {
        return;
    }
    ksettings->setMinLogicalFontSize((int)rint(size));
    _updateAllViews();
}

float WebCoreSettings::minimumLogicalFontSize()
{
    return ksettings->minLogicalFontSize();
}

void WebCoreSettings::setDefaultFontSize(float size)
{
    if (ksettings->mediumFontSize() == size) {
        return;
    }
    ksettings->setMediumFontSize((int)rint(size));
    _updateAllViews();
}

float WebCoreSettings::defaultFontSize()
{
    return ksettings->mediumFontSize();
}

void WebCoreSettings::setDefaultFixedFontSize(float size)
{
    if (ksettings->mediumFixedFontSize() == size) {
        return;
    }
    ksettings->setMediumFixedFontSize((int)rint(size));
    _updateAllViews();
}

float WebCoreSettings::defaultFixedFontSize()
{
    return ksettings->mediumFixedFontSize();
}

void WebCoreSettings::setJavaEnabled(bool enabled)
{
    ksettings->setIsJavaEnabled(enabled);
}

bool WebCoreSettings::javaEnabled()
{
    return ksettings->isJavaEnabled();
}

void WebCoreSettings::setPluginsEnabled(bool enabled)
{
    ksettings->setArePluginsEnabled(enabled);
}

bool WebCoreSettings::pluginsEnabled()
{
    return ksettings->isPluginsEnabled();
}

void WebCoreSettings::setJavaScriptEnabled(bool enabled)
{
    ksettings->setIsJavaScriptEnabled(enabled);
}

bool WebCoreSettings::javaScriptEnabled()
{
    return ksettings->isJavaScriptEnabled();
}

void WebCoreSettings::setJavaScriptCanOpenWindowsAutomatically(bool enabled)
{
    ksettings->setJavaScriptCanOpenWindowsAutomatically(enabled);   
}

bool WebCoreSettings::javaScriptCanOpenWindowsAutomatically()
{
    return ksettings->JavaScriptCanOpenWindowsAutomatically();
}

void WebCoreSettings::setWillLoadImagesAutomatically(bool load)
{
    ksettings->setAutoLoadImages(load);
}

bool WebCoreSettings::willLoadImagesAutomatically()
{
    return ksettings->autoLoadImages();
}

void WebCoreSettings::setUserStyleSheetLocation(const gchar* f)
{
    QString s(f);
    if (ksettings->userStyleSheet() == s) {
        return;
    }
    ksettings->setUserStyleSheet(s);
    _updateAllViews();
}

const gchar* WebCoreSettings::userStyleSheetLocation()
{
    return ksettings->userStyleSheet().latin1();
}

void WebCoreSettings::setShouldPrintBackgrounds(bool enabled)
{
    ksettings->setShouldPrintBackgrounds(enabled);
}

bool WebCoreSettings::shouldPrintBackgrounds()
{
    return ksettings->shouldPrintBackgrounds();
}

void WebCoreSettings::setDefaultTextEncoding(const gchar* f)
{
    QString s(f);
    if (ksettings->encoding() == s) {
        return;
    }
    ksettings->setEncoding(s);
}

const gchar* WebCoreSettings::defaultTextEncoding()
{
    return ksettings->encoding().latin1();
}

KHTMLSettings* WebCoreSettings::settings()
{
    return ksettings;
}

