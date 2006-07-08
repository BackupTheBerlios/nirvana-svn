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
#include <gtk/gtk.h>

#if 0 // no jni support
#include <JavaScriptCore/jni_jsobject.h>
#endif
#include <JavaScriptCore/object.h>
#include <JavaScriptCore/property_map.h>

#include <JavaScriptCore/interpreter.h> // SavedBuiltins

#include "WebCoreBridge.h"
#include "GLibHelpers.h"

#include "csshelper.h"
#include "dom_node.h"
#include "dom_docimpl.h"
#include "dom_nodeimpl.h"
#include "html_documentimpl.h"
#include "html_formimpl.h"
#include "html_imageimpl.h"
#include "htmlattrs.h"
#include "htmltags.h"
#include "khtml_part.h"
#include "khtmlview.h"
#include "kjs_proxy.h"
#include "kjs_window.h"
#include "loader.h"
#include "render_frames.h"
#include "render_image.h"
#include "render_object.h"
#include "render_canvas.h"
#include "render_style.h"
#include "render_replaced.h"

#include "KWQLogging.h"
#include "KWQCharsets.h"
#include "KWQDOMNode.h"
#include "KWQFont.h"
#include "KWQFrame.h"
#include "KWQLoader.h"
#include "KWQPageState.h"
#include "KWQRenderTreeDebug.h"
//#include "KWQView.h"
#include "KWQPrinter.h"

//#include "WebCoreDOMPrivate.h"
//#include "WebCoreImageRenderer.h"
//#include "WebCoreTextRendererFactory.h"
#include "WebCoreViewFactory.h"
#include "WebCoreSettings.h"

//#include <AppKit/NSView.h>

#include "KWQKJob.h"
#include "KWQKHTMLSettings.h"
#include "KWQKHTMLFactory.h"

#include "WebCoreHistory.h"
#include "WebCoreUnicode.h"

using DOM::DocumentImpl;
using DOM::Node;
using DOM::NodeImpl;

using khtml::Decoder;
using khtml::parseURL;
using khtml::RenderImage;
using khtml::RenderObject;
using khtml::RenderPart;
using khtml::RenderStyle;
using khtml::RenderCanvas;

using KJS::SavedProperties;
using KJS::SavedBuiltins;

using KParts::URLArgs;

#if 0 // KWIQ: no jni support
using KJS::Bindings::RootObject;
#endif

gchar* WebCoreElementFrameKey = "WebElementFrame";
gchar* WebCoreElementImageAltStringKey = "WebElementImageAltString";
gchar* WebCoreElementImageKey = "WebElementImage";
gchar* WebCoreElementImageRectKey = "WebElementImageRect";
gchar* WebCoreElementImageURLKey = "WebElementImageURL";
gchar* WebCoreElementIsSelectedKey = "WebElementIsSelected";
gchar* WebCoreElementLinkURLKey = "webelementlinkurl";
gchar* WebCoreElementLinkTargetFrameKey = "WebElementTargetFrame";
gchar* WebCoreElementLinkLabelKey = "WebElementLinkLabel";
gchar* WebCoreElementLinkTitleKey = "WebElementLinkTitle";
gchar* WebCoreElementNameKey = "WebElementName";
gchar* WebCoreElementTitleKey = "WebCoreElementTitle"; // not in WebKit API for now, could be in API some day

gchar * KWIQPageCacheStateKey = "KWIQPageCacheState";

#if 0
static RootObject *rootForGtkWidget(void *v)
{
    GtkWidget *widget = GTK_WIDGET (v);
    WebCoreBridge *aBridge = WebCoreViewFactory::sharedFactory().bridgeForGtkWidget(widget);
    if (aBridge) {
        KWQKHTMLPart *part = aBridge->part();
        RootObject *root = new RootObject(v);    // The root gets deleted by JavaScriptCore.
        
        root->setRootObjectImp (static_cast<KJS::ObjectImp *>(KJS::Window::retrieveWindow(part)));
        root->setInterpreter (KJSProxy::proxy(part)->interpreter());
        part->addPluginRootObject (root);
            
        return root;
    }
    return 0;
}
#endif

static bool initializedObjectCacheSize = false;
static bool initializedKJS = false;

WebCoreBridge::WebCoreBridge()
    :_part(0)
    ,_renderPart(0)
    ,_renderPartArena(0)    
    ,_drawSelectionOnly(false)
    ,_url(0)
    ,_name(0)
    ,_referrer(0)
    ,_renderTree(0)
    ,_jsEvalString(0)
    ,_encoding(0)
{
    _part = new KWQKHTMLPart;
    _part->setBridge(this);

    // getObjectCacheSize cannot be called from constructor 
    
    if (!initializedKJS) {
#if 0 //no jni support
        KJS::Bindings::RootObject::setFindRootObjectForNativeHandleFunction (rootForGtkWidget);
#endif 
        initializedKJS = true;
    }
    
    _shouldCreateRenderers = true;

}

void WebCoreBridge::init()
{
    if (!initializedObjectCacheSize) {
	// getObjectCacheSize cannot be called from constructor, call it here (a bit dangerous)
        khtml::Cache::setSize(getObjectCacheSize());
        initializedObjectCacheSize = true;
    }
}

void WebCoreBridge::initializeSettings(WebCoreSettings *settings)
{
    _part->setSettings(settings->settings());
}

WebCoreBridge::~WebCoreBridge()
{
    LOG(KwiqLog, " this:%x, part():%x framename %s", this, part(), name() );
    removeFromFrame();
    
    if (_renderPart) {
        _renderPart->deref(_renderPartArena);
    }

    // free temporary strings
    freeString(_url);
    freeString(_name);
    freeString(_referrer);
    freeString(_renderTree);
    freeString(_jsEvalString);
    freeString(_encoding);
    
    _part->setBridge(0);
    _part->deref();
}

KWQKHTMLPart* WebCoreBridge::part()
{
    return _part;
}

void WebCoreBridge::setRenderPart(KHTMLRenderPart *newPart)
{
    RenderArena *arena = newPart->ref();
    if (_renderPart) {
        _renderPart->deref(_renderPartArena);
    }
    _renderPart = newPart;
    _renderPartArena = arena;
}

KHTMLRenderPart * WebCoreBridge::renderPart()
{
    return _renderPart;
}

void WebCoreBridge::setParent(WebCoreBridge *parent)
{
    _part->setParent(parent->part());
}

void WebCoreBridge::provisionalLoadStarted()
{
    _part->provisionalLoadStarted();
}

void WebCoreBridge::openURL(const gchar *_URL, 
			 bool reload, 
			 const gchar *_contentType,
			 const gchar *refresh, 
			 GTime _lastModified, 
			 KWIQPageCache* pageCache)
{
    KURL URL(_URL);
    QString contentType(_contentType);
    QDateTime lastModified(_lastModified);

    if (pageCache) {
	KWQPageState* state = static_cast<KWQPageState*>(pageCache->state());
	_part->openURLFromPageCache(state);
        state->invalidate();
        return;
    }
      
    // arguments
    URLArgs args(_part->browserExtension()->urlArgs());
    args.reload = reload;
    if (!contentType.isEmpty()) {
        args.serviceType = QString::fromUtf8(_contentType);
    }
    _part->browserExtension()->setURLArgs(args);

    // opening the URL
    if (_part->didOpenURL(URL)) {
        // things we have to set up after calling didOpenURL
        if (refresh) {
            _part->addMetaData("http-refresh", QString::fromUtf8(refresh));
        }
	if (lastModified.isValid()) {
	    QString modifiedString = lastModified.toString("%a %b %d %Y %H:%M:%S");
	    _part->addMetaData("modified", modifiedString);
	}
    }
}

void WebCoreBridge::setEncoding(const gchar *encoding, bool userChosen)
{
    _part->setEncoding(QString::fromUtf8(encoding), userChosen);
}

void WebCoreBridge::addData(const char* data, int length)
{
    DocumentImpl *doc = _part->xmlDocImpl();
    
    // Document may be NULL if the part is about to redirect
    // as a result of JS executing during load, i.e. one frame
    // changing another's location before the frame's document
    // has been created. 
    if (doc){
        doc->setShouldCreateRenderers(shouldCreateRenderers());
        _part->addData(data, length);
    }
}

void WebCoreBridge::closeURL()
{
    _part->closeURL();
}

void WebCoreBridge::didNotOpenURL(const gchar *URL)
{
    _part->didNotOpenURL(KURL(URL));
}

void WebCoreBridge::saveDocumentState()
{
    DocumentImpl *doc = _part->xmlDocImpl();
    if (doc != 0){
        QStringList list = doc->docState();
	GList* documentState = 0;
        
        for (uint i = 0; i < list.count(); i++){
            QString s = list[i];
            documentState = g_list_append(documentState, g_strdup(s.utf8()));
        }
        saveDocumentState(documentState); 
	GList* iter = documentState;
	while (iter) {
	    g_free(iter->data);
	    iter = g_list_next(iter);
	}
	g_list_free(documentState);
    }
}

void WebCoreBridge::restoreDocumentState()
{
    DocumentImpl *doc = _part->xmlDocImpl();
    
    if (doc != 0){
        GList *docState = documentState();  
	GList *iter = docState;
        QStringList s;
	while (iter) {
	    ASSERT(iter->data);
	    gchar *string = static_cast<gchar*>(iter->data);
	    s.append(QString::fromUtf8(string));
	    iter = g_list_next(iter);
        }            
        doc->setRestoreState(s);
    }
}

void WebCoreBridge::scrollToAnchorWithURL(const gchar *URL)
{
    _part->scrollToAnchor(QString::fromUtf8(URL));
}

bool WebCoreBridge::saveDocumentToPageCache()
{
    DocumentImpl *doc = _part->xmlDocImpl();
    if (!doc) {
        return false;
    }
    
    _part->clearTimers();

    SavedProperties *windowProperties = new SavedProperties;
    _part->saveWindowProperties(windowProperties);

    SavedProperties *locationProperties = new SavedProperties;
    _part->saveLocationProperties(locationProperties);

    SavedBuiltins *interpreterBuiltins = new SavedBuiltins;
    _part->saveInterpreterBuiltins(*interpreterBuiltins);
    
    KWQPageState *pageState = new KWQPageState(doc,
					       _part->m_url,
					       windowProperties, 
					       locationProperties,
					       interpreterBuiltins);


    pageState->setPausedActions(_part->pauseActions((const void *)pageState));

    return saveDocumentToPageCache(pageState);
}

bool WebCoreBridge::canCachePage()
{
    return _part->canCachePage();
}

void WebCoreBridge::end()
{
    _part->end();
}

void WebCoreBridge::createKHTMLViewWithGtkWidget(GtkWidget* widget, int marginWidth, int marginHeight)
{
    // If we own the view, delete the old one - otherwise the render _part will take care of deleting the view.
    removeFromFrame();

    KHTMLView *kview = new KHTMLView(_part, 0);
    _part->setView(kview);
    kview->deref();
    
    kview->setGtkWidget(widget);

    if (marginWidth >= 0)
        kview->setMarginWidth(marginWidth);
    if (marginHeight >= 0)
        kview->setMarginHeight(marginHeight);
}

void WebCoreBridge::scrollToAnchor(const gchar *a)
{
    _part->gotoAnchor(QString::fromUtf8(a));
}

#if 0
QString WebCoreBridge::selectedString()
{
    QString text = _part->selectedText();
    text.replace('\\', _part->backslashAsCurrencySymbol());
    return text;
}

void WebCoreBridge::selectAll()
{
    _part->selectAll();
}

void WebCoreBridge::deselectAll()
{
    _part->slotClearSelection();
}

void deselectText()
{
    _part->slotClearSelection();
}
#endif
bool WebCoreBridge::isFrameSet()
{
    return _part->isFrameSet();
}

QString styleSheetForPrinting(KWQKHTMLPart* _part)
{
    if (!_part->settings()->shouldPrintBackgrounds()) {
        return QString::fromLatin1("* { background-image: none !important; background-color: white !important;}");
    }
    return QString();
}

void WebCoreBridge::reapplyStylesForDeviceType(WebCoreDeviceType deviceType)
{
    _part->setMediaType(deviceType == WebCoreDeviceScreen ? "screen" : "print");
    DocumentImpl *doc = _part->xmlDocImpl();
    if (doc) {
        static QPaintDevice screen;
        static QPrinter printer;
    	doc->setPaintDevice(deviceType == WebCoreDeviceScreen ? &screen : &printer);
        if (deviceType != WebCoreDeviceScreen) {
            doc->setPrintStyleSheet(styleSheetForPrinting(_part));
        }

    }
    _part->reparseConfiguration();
}

static bool nowPrinting(WebCoreBridge *self)
{
    DocumentImpl *doc = self->part()->xmlDocImpl();
    return doc && doc->paintDevice() && doc->paintDevice()->devType() == QInternal::Printer;
}

// Set or unset the printing mode in the view.  We only toy with this if we're printing.
void WebCoreBridge::_setupRootForPrinting(bool onOrOff)
{
    if (nowPrinting(this)) {
        RenderCanvas *root = static_cast<khtml::RenderCanvas *>(_part->xmlDocImpl()->renderer());
        if (root) {
            root->setPrintingMode(onOrOff);
        }
    }
}

void WebCoreBridge::forceLayoutAdjustingViewSize(bool flag)
{
    _setupRootForPrinting(true);
    _part->forceLayout();
    if (flag) {
	adjustViewSize();
    }
    _setupRootForPrinting(false);
}

void WebCoreBridge::forceLayoutWithMinimumPageWidth(float minPageWidth, float maximumPageWidth, float maxPageWidth, bool flag)
{
    _setupRootForPrinting(true);
    _part->forceLayoutWithPageWidthRange(minPageWidth, maxPageWidth);
    if (flag) {
	adjustViewSize();
    }
    _setupRootForPrinting(false);

}

void WebCoreBridge::sendResizeEvent()
{
    _part->sendResizeEvent();
}

void WebCoreBridge::drawRect(GdkRectangle * _rect, QPainter *p)
{
    QRect rect(_rect);
    _setupRootForPrinting(true);
    _part->paint(p, rect);
    _setupRootForPrinting(false);
}

void WebCoreBridge::drawRect(GdkRectangle * rect, CGContextRef gc)
{
    QPainter painter(nowPrinting(this));
    painter.setContext(gc);
    painter.setUsesInactiveTextBackgroundColor(_part->usesInactiveTextBackgroundColor());
#if 0
    painter.setDrawsFocusRing(_part->showsFirstResponder());
#endif

    drawRect(rect, &painter);
}

#if 0
// Used by pagination code called from AppKit when a standalone web page is printed.
-(NSArray*)computePageRectsWithPrintWidth:(float)printWidth printHeight:(float)printHeight
{
    [self _setupRootForPrinting:YES];
    NSMutableArray* pages = [NSMutableArray arrayWithCapacity:5];
	if (printWidth == 0 || printHeight == 0) {
		return pages;
	}
	
    KHTMLView* view = _part->view();
    NSView* documentView = view->getDocumentView();
    if (!documentView)
        return pages;
	
    float currPageHeight = printHeight;
    float docHeight = NSHeight([documentView bounds]);
    float docWidth = NSWidth([documentView bounds]);
    
    // We need to give the part the opportunity to adjust the page height at each step.
    for (float i = 0; i < docHeight; i += currPageHeight) {
        float proposedBottom = kMin(docHeight, i + printHeight);
        _part->adjustPageHeight(&proposedBottom, i, proposedBottom, i);
        currPageHeight = kMax(1.0f, proposedBottom - i);
        for (float j = 0; j < docWidth; j += printWidth) {
            NSValue* val = [NSValue valueWithRect: NSMakeRect(j, i, printWidth, currPageHeight)];
            [pages addObject: val];
        }
    }
    [self _setupRootForPrinting:NO];
    
    return pages;
}
#endif

// This is to support the case where a webview is embedded in the view that's being printed
void WebCoreBridge::adjustPageHeightNew(float *newBottom,float oldTop, float oldBottom, float bottomLimit)
{
    _setupRootForPrinting(true);
    _part->adjustPageHeight(newBottom, oldTop, oldBottom, bottomLimit);
    _setupRootForPrinting(false);
}

#if 0 // dom api not implemented
- (NSObject *)copyDOMNode:(NodeImpl *)node copier:(id <WebCoreDOMTreeCopier>)copier
{
    NSMutableArray *children = [[NSMutableArray alloc] init];
    for (NodeImpl *child = node->firstChild(); child; child = child->nextSibling()) {
        [children addObject:[self copyDOMNode:child copier:copier]];
    }
    NSObject *copiedNode = [copier nodeWithName:node->nodeName().string().getNSString()
                                          value:node->nodeValue().string().getNSString()
                                         source:node->recursive_toHTML(1).getNSString()
                                       children:children];
    [children release];
    return copiedNode;
}

- (NSObject *)copyDOMTree:(id <WebCoreDOMTreeCopier>)copier
{
    DocumentImpl *doc = _part->xmlDocImpl();
    if (!doc) {
        return nil;
    }
    return [self copyDOMNode:doc copier:copier];
}

- (NSObject *)copyRenderNode:(RenderObject *)node copier:(id <WebCoreRenderTreeCopier>)copier
{
    NSMutableArray *children = [[NSMutableArray alloc] init];
    for (RenderObject *child = node->firstChild(); child; child = child->nextSibling()) {
        [children addObject:[self copyRenderNode:child copier:copier]];
    }
          
    NSString *name = [[NSString alloc] initWithUTF8String:node->renderName()];
    
    RenderPart *nodeRenderPart = dynamic_cast<RenderPart *>(node);
    QWidget *widget = nodeRenderPart ? nodeRenderPart->widget() : 0;
    NSView *view = widget ? widget->getView() : nil;
    
    int nx, ny;
    node->absolutePosition(nx,ny);
    NSObject *copiedNode = [copier nodeWithName:name
                                       position:NSMakePoint(nx,ny)
                                           rect:NSMakeRect(node->xPos(), node->yPos(), node->width(), node->height())
                                           view:view
                                       children:children];
    
    [name release];
    [children release];
    
    return copiedNode;
}

- (NSObject *)copyRenderTree:(id <WebCoreRenderTreeCopier>)copier
{
    RenderObject *renderer = _part->renderer();
    if (!renderer) {
        return nil;
    }
    return [self copyRenderNode:renderer copier:copier];
}

#endif

void WebCoreBridge::removeFromFrame()
{
    LOG(KwiqLog, "this: %x, part:%x", this,part());
    _part->setView(0);
}

void WebCoreBridge::installInFrame(GtkWidget *widget)
{
    // If this isn't the main frame, it must have a render _part set, or it
    // won't ever get installed in the view hierarchy.
    ASSERT(this == mainFrame() || _renderPart != NULL);
    
    _part->view()->setGtkWidget(widget);
    if (_renderPart) {
        _renderPart->setWidget(_part->view());
        // Now the render part owns the view, so we don't any more.
    }

    _part->view()->initScrollBars();
}

void WebCoreBridge::mouseDown(GdkEvent *_event)
{
    QMouseEvent event(QEvent::MouseButtonPress, _event);
    _part->mouseDown(&event);
}
void WebCoreBridge::mouseUp(GdkEvent *_event)
{
    QMouseEvent event(QEvent::MouseButtonRelease, _event);
    _part->mouseUp(&event);
}
void WebCoreBridge::mouseDragged(GdkEventMotion *_event)
{
    QMouseEvent event( _event);
    _part->mouseDragged(&event);
}

void WebCoreBridge::mouseMoved(GdkEventMotion *_event)
{
    QMouseEvent event(_event);
    _part->mouseMoved(&event);
}

#if 0
- (BOOL)sendContextMenuEvent:(NSEvent *)event
{
    return _part->sendContextMenuEvent(event);
}

- (id <WebDOMElement>)elementForView:(NSView *)view
{
    // FIXME: implemented currently for only a subset of the KWQ widgets
    if ([view conformsToProtocol:@protocol(KWQWidgetHolder)]) {
        NSView <KWQWidgetHolder> *widgetHolder = view;
        QWidget *widget = [widgetHolder widget];
        if (widget != nil) {
            NodeImpl *node = static_cast<const RenderWidget *>(widget->eventFilterObject())->element();
            return [WebCoreDOMElement elementWithImpl:static_cast<ElementImpl *>(node)];
        }
    }
    return nil;
}

static NSView *viewForElement(DOM::ElementImpl *elementImpl)
{
    RenderObject *renderer = elementImpl->renderer();
    if (renderer && renderer->isWidget()) {
        QWidget *widget = static_cast<const RenderWidget *>(renderer)->widget();
        if (widget) {
            return widget->getView();
        }
    }
    return nil;
}

static HTMLInputElementImpl *inputElementFromDOMElement(id <WebDOMElement>element)
{
    ASSERT([(NSObject *)element isKindOfClass:[WebCoreDOMElement class]]);
    DOM::ElementImpl *domElement = [(WebCoreDOMElement *)element elementImpl];
    if (domElement && idFromNode(domElement) == ID_INPUT) {
        return static_cast<HTMLInputElementImpl *>(domElement);
    }
    return nil;
}

static HTMLFormElementImpl *formElementFromDOMElement(id <WebDOMElement>element)
{
    ASSERT([(NSObject *)element isKindOfClass:[WebCoreDOMElement class]]);
    DOM::ElementImpl *domElement = [(WebCoreDOMElement *)element elementImpl];
    if (domElement && idFromNode(domElement) == ID_FORM) {
        return static_cast<HTMLFormElementImpl *>(domElement);
    }
    return nil;
}

- (id <WebDOMElement>)elementWithName:(NSString *)name inForm:(id <WebDOMElement>)form
{
    HTMLFormElementImpl *formElement = formElementFromDOMElement(form);
    if (formElement) {
        QPtrList<HTMLGenericFormElementImpl> elements = formElement->formElements;
        QString targetName = QString::fromNSString(name);
        for (unsigned int i = 0; i < elements.count(); i++) {
            HTMLGenericFormElementImpl *elt = elements.at(i);
            // Skip option elements, other duds
            if (elt->name() == targetName) {
                return [WebCoreDOMElement elementWithImpl:elt];
            }
        }
    }
    return nil;
}

- (BOOL)elementDoesAutoComplete:(id <WebDOMElement>)element
{
    HTMLInputElementImpl *inputElement = inputElementFromDOMElement(element);
    return inputElement != nil
        && inputElement->inputType() == HTMLInputElementImpl::TEXT
        && inputElement->autoComplete();
}

- (BOOL)elementIsPassword:(id <WebDOMElement>)element
{
    HTMLInputElementImpl *inputElement = inputElementFromDOMElement(element);
    return inputElement != nil
        && inputElement->inputType() == HTMLInputElementImpl::PASSWORD;
}

- (id <WebDOMElement>)formForElement:(id <WebDOMElement>)element;
{
    HTMLInputElementImpl *inputElement = inputElementFromDOMElement(element);
    if (inputElement) {
        HTMLFormElementImpl *formElement = inputElement->form();
        if (formElement) {
            return [WebCoreDOMElement elementWithImpl:formElement];
        }
    }
    return nil;
}

- (id <WebDOMElement>)currentForm
{
    HTMLFormElementImpl *formElement = _part->currentForm();
    return formElement ? [WebCoreDOMElement elementWithImpl:formElement] : nil;
}

- (NSArray *)controlsInForm:(id <WebDOMElement>)form
{
    NSMutableArray *results = nil;
    HTMLFormElementImpl *formElement = formElementFromDOMElement(form);
    if (formElement) {
        QPtrList<HTMLGenericFormElementImpl> elements = formElement->formElements;
        for (unsigned int i = 0; i < elements.count(); i++) {
            if (elements.at(i)->isEnumeratable()) {		// Skip option elements, other duds
                NSView *view = viewForElement(elements.at(i));
                if (view) {
                    if (!results) {
                        results = [NSMutableArray arrayWithObject:view];
                    } else {
                        [results addObject:view];
                    }
                }
            }
        }
    }
    return results;
}

- (NSString *)searchForLabels:(NSArray *)labels beforeElement:(id <WebDOMElement>)element
{
    ASSERT([(NSObject *)element isKindOfClass:[WebCoreDOMElement class]]);
    return _part->searchForLabelsBeforeElement(labels, [(WebCoreDOMElement *)element elementImpl]);
}

- (NSString *)matchLabels:(NSArray *)labels againstElement:(id <WebDOMElement>)element
{
    ASSERT([(NSObject *)element isKindOfClass:[WebCoreDOMElement class]]);
    return _part->matchLabelsAgainstElement(labels, [(WebCoreDOMElement *)element elementImpl]);
}
#endif

void WebCoreBridge::elementAtPoint(int x, int y, WebCoreElementInfo* element)
{
    QPoint point(x,y);

    RenderObject *renderer = _part->renderer();
    if (!renderer) {
        return;
    }
    RenderObject::NodeInfo nodeInfo(true, true);
    renderer->layer()->nodeAtPoint(nodeInfo, (int)point.x(), (int)point.y());
    
    bool issel = _part->isPointInsideSelection((int)point.x(), (int)point.y())? true : false;
    element->isSelected = issel;
                
    // Find the title in the nearest enclosing DOM node.
    // For <area> tags in image maps, walk the tree for the <area>, not the <img> using it.
    for (NodeImpl *titleNode = nodeInfo.innerNode(); titleNode; titleNode = titleNode->parentNode()) {
        if (titleNode->isElementNode()) {
            const DOM::DOMString title = static_cast<DOM::ElementImpl *>(titleNode)->getAttribute(ATTR_TITLE);
            if (!title.isNull()) {
                // We found a node with a title.
                QString titleText(title.string());
                titleText.replace('\\', _part->backslashAsCurrencySymbol());		
                element->linkTitle = g_strdup(titleText.utf8());
                break;
            }
        }
    }

    NodeImpl *URLNode = nodeInfo.URLElement();
    if (URLNode) {
        DOM::ElementImpl *e = static_cast<DOM::ElementImpl *>(URLNode);
        
        DOM::DOMString title = e->getAttribute(ATTR_TITLE);
        if (!title.isEmpty()) {
            QString titleText(title.string());
            titleText.replace('\\', _part->backslashAsCurrencySymbol());
            element->linkTitle = g_strdup(titleText.utf8());
        }
        
        DOM::DOMString link = e->getAttribute(ATTR_HREF);
        if (!link.isNull()) {
            if (e->firstChild()) {
                DOM::Range r(_part->document());
                r.setStartBefore(e->firstChild());
                r.setEndAfter(e->lastChild());
                QString t(_part->text(r));
                if (!t.isEmpty()) {
                    element->linkLabel = g_strdup(t.utf8());
                } 
            }
            element->linkURL = g_strdup(_part->xmlDocImpl()->completeURL(parseURL(link).string()).utf8());
        }
        
        DOM::DOMString target = e->getAttribute(ATTR_TARGET);
        if (target.isEmpty() && _part->xmlDocImpl()) {
            target = _part->xmlDocImpl()->baseTarget();
        }
        if (!target.isEmpty()) {
            element->linkTarget = g_strdup(target.string().utf8());
        }
    }

    NodeImpl *node = nodeInfo.innerNonSharedNode();
    if (node && node->renderer() && node->renderer()->isImage()) {
        RenderImage *r = static_cast<RenderImage *>(node->renderer());
#if 0
        NSImage *image = r->pixmap().image();
#endif
        // Only return image information if there is an image.
        if (!r->pixmap().isNull() /* && !isDisplayingError(&r->pixmap().image())*/) {
#if 0
            element->qpixmap = r->pixmap();
#endif
            
            int x, y;
            if (r->absolutePosition(x, y)) {
                QRect rect(x, y, r->contentWidth(), r->contentHeight());
                element->imageRect.x = rect.x(); element->imageRect.y = rect.y();
                element->imageRect.width = rect.width(); element->imageRect.height = rect.height();
            }
            
            DOM::ElementImpl *i = static_cast<DOM::ElementImpl*>(node);
    
            // FIXME: Code copied from RenderImage::updateFromElement; should share.
            DOM::DOMString attr;
            if (idFromNode(i) == ID_OBJECT) {
                attr = i->getAttribute(ATTR_DATA);
            } else {
                attr = i->getAttribute(ATTR_SRC);
            }
            if (!attr.isEmpty()) {
                QString URLString = parseURL(attr).string();
                element->imageURL = g_strdup(_part->xmlDocImpl()->completeURL(URLString).utf8());
            }
            
            // FIXME: Code copied from RenderImage::updateFromElement; should share.
            DOM::DOMString alt;
            if (idFromNode(i) == ID_INPUT)
                alt = static_cast<DOM::HTMLInputElementImpl *>(i)->altText();
            else if (idFromNode(i) == ID_IMG)
                alt = static_cast<DOM::HTMLImageElementImpl *>(i)->altText();
            if (!alt.isNull()) {
                QString altText = alt.string();
                altText.replace('\\', _part->backslashAsCurrencySymbol());
                element->imageAltText = g_strdup(altText.utf8());
            }
        }
    }
}


bool WebCoreBridge::searchFor(const gchar *string, bool forward, bool caseFlag, bool wrapFlag)
{
    return _part->findString(string, forward, caseFlag, wrapFlag);
}

void WebCoreBridge::jumpToSelection()
{
    _part->jumpToSelection();
}

void WebCoreBridge::setTextSizeMultiplier(float multiplier)
{
    int newZoomFactor = (int)(multiplier * 100);

    if (_part->zoomFactor() == newZoomFactor) {
        return;
    }
    _part->setZoomFactor(newZoomFactor);
    // setZoomFactor will trigger a timed layout, but we want to do the layout before
    // we do any drawing. This takes care of that. Without this we redraw twice.
    setNeedsLayout();
}

const gchar* WebCoreBridge::textEncoding()
{
    assignToString(&_encoding, _part->encoding().utf8());
    return _encoding;
}


GtkWidget *WebCoreBridge::nextKeyView()
{
    DocumentImpl *doc = _part->xmlDocImpl();
    if (!doc) {
        return 0;
    }
    return _part->nextKeyView(doc->focusNode(), KWQSelectingNext);
}

GtkWidget *WebCoreBridge::previousKeyView()
{
    DocumentImpl *doc = _part->xmlDocImpl();
    if (!doc) {
        return 0;
    }
    return _part->nextKeyView(doc->focusNode(), KWQSelectingPrevious);
}

GtkWidget *WebCoreBridge::nextKeyViewInsideWebFrameViews()
{
    return _part->nextKeyViewInFrameHierarchy(0, KWQSelectingNext);
}

GtkWidget *WebCoreBridge::previousKeyViewInsideWebFrameViews()
{
    return _part->nextKeyViewInFrameHierarchy(0, KWQSelectingPrevious);
}


const gchar * WebCoreBridge::stringByEvaluatingJavaScriptFromString(const gchar* string)
{
    _part->createEmptyDocument();
    assignToString(&_jsEvalString, _part->executeScript(QString::fromUtf8(string), true).asString().utf8());
    return _jsEvalString;
}

#if 0
- (id<WebDOMDocument>)DOMDocument
{
    return [WebCoreDOMDocument documentWithImpl:_part->xmlDocImpl()];
}

- (void)setSelectionFrom:(id<WebDOMNode>)start startOffset:(int)startOffset to:(id<WebDOMNode>)end endOffset:(int) endOffset
{
    WebCoreDOMNode *startNode = start;
    WebCoreDOMNode *endNode = end;
    _part->xmlDocImpl()->setSelection([startNode impl], startOffset, [endNode impl], endOffset);
}

- (NSAttributedString *)selectedAttributedString
{
    return _part->attributedString(_part->selectionStart(), _part->selectionStartOffset(), _part->selectionEnd(), _part->selectionEndOffset());
}

- (NSAttributedString *)attributedStringFrom:(id<WebDOMNode>)start startOffset:(int)startOffset to:(id<WebDOMNode>)end endOffset:(int)endOffset
{
    WebCoreDOMNode *startNode = start;
    WebCoreDOMNode *endNode = end;
    return _part->attributedString([startNode impl], startOffset, [endNode impl], endOffset);
}

- (id<WebDOMNode>)selectionStart
{
    return [WebCoreDOMNode nodeWithImpl:_part->selectionStart()];
}

- (int)selectionStartOffset
{
    return _part->selectionStartOffset();
}

- (id<WebDOMNode>)selectionEnd
{
    return [WebCoreDOMNode nodeWithImpl:_part->selectionEnd()];
}

- (int)selectionEndOffset
{
    return _part->selectionEndOffset();
}

- (NSRect)selectionRect
{
    return _part->selectionRect(); 
}

- (NSRect)visibleSelectionRect
{
    KHTMLView *view = _part->view();
    if (!view) {
        return NSZeroRect;
    }
    NSView *documentView = view->getDocumentView();
    if (!documentView) {
        return NSZeroRect;
    }
    return NSIntersectionRect(_part->selectionRect(), [documentView visibleRect]); 
}

- (NSImage *)selectionImage
{
    NSView *view = _part->view()->getDocumentView();
    if (!view) {
        return nil;
    }

    NSRect rect = [self visibleSelectionRect];
    NSRect bounds = [view bounds];
    NSImage *selectionImage = [[[NSImage alloc] initWithSize:rect.size] autorelease];
    [selectionImage setFlipped:YES];
    [selectionImage lockFocus];

    [NSGraphicsContext saveGraphicsState];
    CGContextTranslateCTM((CGContext *)[[NSGraphicsContext currentContext] graphicsPort],
                          -(NSMinX(rect) - NSMinX(bounds)), -(NSMinY(rect) - NSMinY(bounds)));

    _drawSelectionOnly = YES;
    [view drawRect:rect];
    _drawSelectionOnly = NO;

    [NSGraphicsContext restoreGraphicsState];
        
    [selectionImage unlockFocus];
    [selectionImage setFlipped:NO];

    return selectionImage;
}
#endif

void WebCoreBridge::setName(const gchar* name)
{
    _part->setName(QString::fromUtf8(name));
}

const gchar* WebCoreBridge::name()
{
    assignToString(&_name, _part->name().utf8());
    return _name;
}

const gchar* WebCoreBridge::URL()
{
    assignToString(&_url, _part->url().url().utf8());
    return _url;
}

const gchar* WebCoreBridge::referrer()
{
    // Do not allow file URLs to be used as referrers as that is potentially a security issue
    QString referrer = _part->referrer();
    bool isFileURL = referrer.lower().find("file:",0,false);
    assignToString(&_referrer, isFileURL ? 0 : referrer.utf8() );
    return _referrer;
}

#if 0
+ (NSString *)stringWithData:(NSData *)data textEncoding:(CFStringEncoding)textEncoding
{
    if (textEncoding == kCFStringEncodingInvalidId || textEncoding == kCFStringEncodingISOLatin1) {
        textEncoding = kCFStringEncodingWindowsLatin1;
    }
    return QTextCodec(textEncoding).toUnicode((const char*)[data bytes], [data length]).getNSString();
}

+ (NSString *)stringWithData:(NSData *)data textEncodingName:(NSString *)textEncodingName
{
    CFStringEncoding textEncoding = KWQCFStringEncodingFromIANACharsetName([textEncodingName lossyCString]);
    return [WebCoreBridge stringWithData:data textEncoding:textEncoding];
}

#endif

void updateAllViews()
{
    for (QPtrListIterator<KWQKHTMLPart> it(KWQKHTMLPart::instances()); it.current(); ++it) {
        KWQKHTMLPart *part = it.current();
        part->bridge()->setNeedsReapplyStyles();
    }
}

bool WebCoreBridge::needsLayout()
{
    RenderObject *renderer = _part->renderer();
    return renderer ? renderer->needsLayout() : false;
}

void WebCoreBridge::setNeedsLayout()
{
    RenderObject *renderer = _part->renderer();
    if (renderer)
        renderer->setNeedsLayout(true);
}

#if 0
- (BOOL)interceptKeyEvent:(NSEvent *)event toView:(NSView *)view
{
    return _part->keyEvent(event);
}
#endif

const gchar* WebCoreBridge::renderTreeAsExternalRepresentation()
{
    assignToString(&_renderTree, externalRepresentation(_part->renderer()).utf8());
    return _renderTree;
}

void WebCoreBridge::setUsesInactiveTextBackgroundColor(bool uses)
{
    _part->setUsesInactiveTextBackgroundColor(uses);
}

bool WebCoreBridge::usesInactiveTextBackgroundColor()
{
    return _part->usesInactiveTextBackgroundColor();
}

bool WebCoreBridge::interceptKeyEvent(GdkEventKey *_event, bool autoRepeat)
{
    QKeyEvent event(_event, autoRepeat);
    return _part->keyEvent(&event);
}

void WebCoreBridge::setShouldCreateRenderers(bool f)
{
    _shouldCreateRenderers = f;
}

bool WebCoreBridge::shouldCreateRenderers()
{
    return _shouldCreateRenderers;
}

int WebCoreBridge::numPendingOrLoadingRequests()
{
    DocumentImpl *doc = _part->xmlDocImpl();
    
    if (doc)
        return KWQNumberOfPendingOrLoadingRequests (doc->docLoader());
    return 0;
}

void WebCoreBridge::bodyBackgroundColor(GdkColor* outColor)
{
    _part->bodyBackgroundColor().getGdkColor(outColor);
}

void WebCoreBridge::adjustViewSize()
{
    KHTMLView *view = _part->view();
    if (view)
        view->adjustViewSize();
}

bool WebCoreBridge::hasStaticBackground()
{
    KHTMLView *view = _part->view();
    if (view) 
	return view->hasStaticBackground();
    return false;
}

WebCoreElementInfo::WebCoreElementInfo()
    : isSelected(false), linkTitle(0), linkLabel(0), linkURL(0), linkTarget(0), imageURL(0),imageAltText(0) 
{
    imageRect.x = imageRect.y = imageRect.width = imageRect.height = -1;
}


#if 0
// MOUSE EVENT STUFF

static int
get_qt_button_state( GdkModifierType state )
{
    int qstate = Qt::NoButton;
    
    if (state & GDK_BUTTON1_MASK)
	qstate|=Qt::LeftButton;
    if (state & GDK_BUTTON2_MASK)
	qstate|=Qt::MidButton;
    if (state & GDK_BUTTON3_MASK)	
	qstate|=Qt::RightButton;
    
    if (state & GDK_SHIFT_MASK)
	qstate|=Qt::ShiftButton;
    if (state & GDK_CONTROL_MASK)
	qstate|=Qt::ControlButton;

    if (state & GDK_MOD1_MASK) 
	qstate|=Qt::AltButton;

    return qstate;
}

static int 
get_qt_button_state_num( int button)
{
    if (button == 1)
	return Qt::LeftButton;
    if (button == 2)
	return Qt::MidButton;
    if (button == 3)
	return Qt::RightButton;

    // unknown button
    return Qt::NoButton;
}

    static GdkEventType last_event = GDK_BUTTON_RELEASE;
    static int click_count = 1;
    BridgeImpl *bridge = static_cast<BridgeImpl*>(data);
    assert(bridge);

    GdkModifierType state = (GdkModifierType) event->state;
    int x,y;
    x = (int) event->x;
    y = (int) event->y;

    _map_to_parent_window(widget->window, event->window, x, y);

#if DEBUG
    g_printerr(" sending : (%d;%d)\n", x,y);
#endif

    switch (event->type) { 
    case GDK_3BUTTON_PRESS:
	++click_count;
	/* fall through */
    case GDK_2BUTTON_PRESS:
	++click_count;
	/* fall through */
    case GDK_BUTTON_PRESS:
    {
	bridge->mouseDown(event); 
	break;
    }
    case GDK_BUTTON_RELEASE:
    {
	bridge->mouseUp(event); 
	click_count = 1;
	break;
    }
    default:
	break;	
    }
	

    last_event = event->type;

    switch (event->type) { 
    case GDK_3BUTTON_PRESS:
	++click_count;
	/* fall through */
    case GDK_2BUTTON_PRESS:
	++click_count;
	/* fall through */
    case GDK_BUTTON_PRESS:
    {
	QMouseEvent qmm(QEvent::MouseButtonPress, QPoint(x,y), get_qt_button_state(state) | get_qt_button_state_num(event->button), click_count);
	break;
    }
    case GDK_BUTTON_RELEASE:
    {
	QMouseEvent qmm(QEvent::MouseButtonRelease, QPoint(x,y), get_qt_button_state(state) | get_qt_button_state_num(event->button), click_count);
	click_count = 1;
	break;
    }
    default:
	break;	
    }
#endif

 
