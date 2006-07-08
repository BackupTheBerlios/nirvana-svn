#ifndef WebCoreTextRendererFactory_h
#define WebCoreTextRendererFactory_h

#include <glib.h>
#include "WebCoreFoundation.h"

class WebCoreTextRenderer;

class NSFont : public WebCoreNSObject
{
 public:
  
};

enum NSFontTraitMask
{
  NSBoldFontMask = 1,
  NSItalicFontMask = 2  
};

class WebCoreTextRendererFactory
{
    
public:
    static WebCoreTextRendererFactory* sharedFactory() { return m_sharedFactory; }

    virtual NSFont* fontWithFamilies(const gchar * const * families, NSFontTraitMask traits, float size) = 0;
    virtual WebCoreTextRenderer* rendererWithFont(NSFont *font, bool isPrinterFont) = 0;
    virtual bool isFontFixedPitch(NSFont* f) =0;
    
protected:
    WebCoreTextRendererFactory() {};
    virtual ~WebCoreTextRendererFactory() {};

    static WebCoreTextRendererFactory* m_sharedFactory;

private:
    WebCoreTextRendererFactory(const WebCoreTextRendererFactory&);
    WebCoreTextRendererFactory& operator=(const WebCoreTextRendererFactory&);
	
};


#endif
