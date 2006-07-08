#include "WebCoreViewFactory.h"


WebCoreViewFactory* WebCoreViewFactory::m_sharedFactory = 0;


WebCoreViewFactory::WebCoreViewFactory()
{
}

WebCoreViewFactory::~WebCoreViewFactory()
{
}

const GList* WebCoreViewFactory::pluginsInfo()
{
    return 0;
}

void WebCoreViewFactory::refreshPlugins(bool reloadPages)
{
    
}

const gchar* WebCoreViewFactory::inputElementAltText()
{
    return "Input";
}

const gchar* WebCoreViewFactory::resetButtonDefaultLabel()
{
    return "Reset";
}

const gchar* WebCoreViewFactory::searchableIndexIntroduction()
{
    return "This is a searchable index. Enter search keywords:";
} 

const gchar* WebCoreViewFactory::submitButtonDefaultLabel()
{
    return "Submit";
}

const gchar* WebCoreViewFactory::fileButtonDefaultLabel()
{
    return "Browse...";
}

const gchar* WebCoreViewFactory::defaultLanguageCode()
{
    return "en";
}


WebCoreViewFactory* WebCoreViewFactory::sharedFactory()
{
    static WebCoreViewFactory singleton;
    if (!m_sharedFactory) 
	m_sharedFactory = &singleton;

    return m_sharedFactory;
}
