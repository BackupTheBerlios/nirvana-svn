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
#ifndef KWIQSETTINGS_H
#define KWIQSETTINGS_H

#include <glib.h>

class KHTMLSettings;

class WebCoreSettings 
{
 public:
  WebCoreSettings();
  virtual ~WebCoreSettings();

  void setMinimumFontSize(float a);
  float minimumFontSize();

  void setMinimumLogicalFontSize(float a);
  float minimumLogicalFontSize() ;

  void setDefaultFontSize(float a);
  float defaultFontSize() ;

  void setDefaultFixedFontSize(float a);
  float defaultFixedFontSize();
  
  void setJavaEnabled(bool a);
  bool javaEnabled() ;

  void setPluginsEnabled(bool a) ;
  bool pluginsEnabled();

  void setJavaScriptEnabled(bool a);
  bool javaScriptEnabled();

  void setJavaScriptCanOpenWindowsAutomatically(bool a) ;
  bool javaScriptCanOpenWindowsAutomatically();

  void setWillLoadImagesAutomatically(bool a);
  bool willLoadImagesAutomatically();

  void setShouldPrintBackgrounds(bool a);
  bool shouldPrintBackgrounds();

  void  setUserStyleSheetLocation(const gchar* location);  
  const gchar* userStyleSheetLocation();

  void setDefaultTextEncoding(const gchar* encoding);
  const gchar* defaultTextEncoding();

  void setStandardFontFamily(const gchar* s);
  const gchar* standardFontFamily();

  void setFixedFontFamily(const gchar* s);
  const gchar* fixedFontFamily();  
  
  void setSerifFontFamily(const gchar* s);
  const gchar* serifFontFamily();
  
  void setSansSerifFontFamily(const gchar* s);
  const gchar* sansSerifFontFamily();

  void setCursiveFontFamily(const gchar* s);
  const gchar* cursiveFontFamily();

  void setFantasyFontFamily(const gchar* s);
  const gchar* fantasyFontFamily();

  KHTMLSettings* settings();

 private:
  void _updateAllViews();
   
  KHTMLSettings *ksettings;
};

#endif
