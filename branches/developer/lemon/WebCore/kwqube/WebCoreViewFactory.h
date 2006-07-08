#ifndef WebCoreViewFactory_h
#define WebCoreViewFactory_h
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

class WebCoreBridge;

class WebCorePluginInfo {

 public:
  virtual const gchar* name() = 0;
  virtual const gchar* filename()= 0;
  virtual const gchar* pluginDescription()= 0;
  virtual const GList* MIMETypes()= 0;  /** GList of gchar* */
  virtual const gchar* descriptionForMIMEType(const gchar* MIMEType)= 0;
  virtual const GList* extensionsForMIMEType(const gchar* MIMEType)= 0; /** GList of gchar* */
};

class WebCoreViewFactory
{
 public:
  static WebCoreViewFactory* sharedFactory();
  
  virtual const GList* pluginsInfo(); /** GList of WebCorePluginInfo* */
  virtual void refreshPlugins(bool reloadPages);

  virtual const gchar* inputElementAltText();
  virtual const gchar* resetButtonDefaultLabel();
  virtual const gchar* searchableIndexIntroduction(); 
  virtual const gchar* submitButtonDefaultLabel();
  virtual const gchar* fileButtonDefaultLabel();

  virtual const gchar* defaultLanguageCode();

#if 0
  virtual WebCoreBridge *bridgeForGtkWidget(GtkWidget*);
#endif
protected:
  static WebCoreViewFactory* m_sharedFactory;
  WebCoreViewFactory();
  virtual ~WebCoreViewFactory();
  
private:
  // disable copy and assignment constructors
  WebCoreViewFactory(const WebCoreViewFactory&);
  WebCoreViewFactory& operator=(const WebCoreViewFactory&);  
};

#endif
