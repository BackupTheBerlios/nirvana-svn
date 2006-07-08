#ifndef WebCoreBridge_h
#define WebCoreBridge_h
    
#include <glib.h>
#include <gdk/gdk.h> // Gdk Events

#include "WebCoreKeyboardAccess.h"

class KWQKHTMLPart;
class KHTMLView;
class RenderArena;
class QPainter;
class CGContext;

typedef CGContext* CGContextRef;

namespace khtml {
    class RenderPart;
    class RenderObject;
}

typedef khtml::RenderPart KHTMLRenderPart;

typedef enum {
    WebCoreDeviceScreen,
    WebCoreDevicePrinter
} WebCoreDeviceType;

typedef enum {
    WebSelectByMoving,
    WebSelectByExtending
} WebSelectionAlteration;

typedef enum {
    WebSelectForward,
    WebSelectBackward,
    WebSelectRight,
    WebSelectLeft
} WebSelectionDirection;

typedef enum {
    WebSelectByCharacter,
    WebSelectByWord,
    WebSelectByLine,
    WebSelectByParagraph
} WebSelectionGranularity;

class WebCoreSettings;
class WebCoreResourceHandle;
class WebCoreResourceLoader;
class KWIQResponse;
class KWIQPageCache;
class KWIQPageState;
class WebCoreElementInfo;

namespace NRCit {
class DOMCSSStyleDeclaration;
class DOMDocument;
class DOMDocumentFragment;
class DOMElement;
class DOMHTMLElement;
class DOMNode;
class DOMRange;
} 
class WebCoreSettings;
class WebScriptObject;


class NSEvent;
class NSView;

// REMOVEUS
class QMouseEvent;
class QKeyEvent;


typedef struct _GtkWidget GtkWidget;

extern gchar* WebCoreElementFrameKey;
extern gchar* WebCoreElementImageAltStringKey;
extern gchar* WebCoreElementImageKey;
extern gchar* WebCoreElementImageRectKey;
extern gchar* WebCoreElementImageURLKey;
extern gchar* WebCoreElementIsSelectedKey;
extern gchar* WebCoreElementLinkURLKey;
extern gchar* WebCoreElementLinkTargetFrameKey;
extern gchar* WebCoreElementLinkLabelKey;
extern gchar* WebCoreElementLinkTitleKey;
extern gchar* WebCoreElementTitleKey;

// WebCoreBridge objects are used by WebCore to abstract away operations that need
// to be implemented by library clients, for example WebKit. The objects are also
// used in the opposite direction, for simple access to WebCore functions without dealing
// directly with the KHTML C++ classes.

// A WebCoreBridge creates and holds a reference to a KHTMLPart.

// The WebCoreBridge interface contains methods for use by the non-WebCore side of the bridge.

class WebCoreBridge
{    
    // @interface -- the core side of the bridge  
  
private:
    KWQKHTMLPart* _part;
    KHTMLRenderPart* _renderPart;
    RenderArena *_renderPartArena;
    bool _drawSelectionOnly;
    bool _shouldCreateRenderers;

    // const gchar wrappers;
    gchar * _url;
    gchar * _name;
    gchar * _referrer;
    gchar * _renderTree;
    gchar * _jsEvalString;
    gchar * _encoding;

    void _setupRootForPrinting(bool onOrOff);
    void drawRect(GdkRectangle* rect, QPainter *p);
public:
    WebCoreBridge();
    virtual ~WebCoreBridge();
    void init();

    void initializeSettings(WebCoreSettings *settings);    

    void setRenderPart(KHTMLRenderPart *renderPart);
    KHTMLRenderPart* renderPart();

    void setName(const gchar* name);
    const gchar* name();

    KWQKHTMLPart* part();
    
    void setParent(WebCoreBridge* parent);

    void provisionalLoadStarted();

    void openURL(const gchar* URL, 
		 bool reload, 
		 const gchar* contentType, 
		 const gchar* refresh, 
		 GTime lastModified, 
		 KWIQPageCache *pageCache);

    void setEncoding(const gchar* encoding, bool userChosen);
    void addData(const char* data, int length);
    void closeURL();

    void didNotOpenURL(const gchar* URL);

    void saveDocumentState();
    void restoreDocumentState();

    bool canCachePage();
    bool saveDocumentToPageCache();

    void end();

    const gchar* URL();
    const gchar* referrer();
    
    void installInFrame(GtkWidget* widget);
    void removeFromFrame() ;

    void scrollToAnchor(const gchar* a);
    void scrollToAnchorWithURL(const gchar* a);

    void createKHTMLViewWithGtkWidget(GtkWidget* widget, int marginWidth, int marginHeight);

    bool isFrameSet();

    void reapplyStylesForDeviceType(WebCoreDeviceType deviceType);
    void forceLayoutAdjustingViewSize(bool adjustSizeFlag);
    void forceLayoutWithMinimumPageWidth(float minPageWidth, float maximumPageWidth, float maxPageWidth, bool flag);
    void sendResizeEvent();
    bool needsLayout();
    void setNeedsLayout();

    void drawRect(GdkRectangle *rect, CGContextRef gc);
    void adjustPageHeightNew(float *newBottom,float oldTop, float oldBottom, float bottomLimit);
#if 0
    NSArray* computePageRectsWithPrintWidth(float printWidth, float printHeight);
#endif

    void setUsesInactiveTextBackgroundColor(bool uses);
    bool usesInactiveTextBackgroundColor();
    void mouseDown(GdkEvent * event);
    void mouseUp(GdkEvent * event);
    void mouseMoved(GdkEventMotion * event);    
    void mouseDragged(GdkEventMotion * event);

    GtkWidget* nextKeyView();
    GtkWidget* previousKeyView();

    GtkWidget* nextKeyViewInsideWebFrameViews();
    GtkWidget* previousKeyViewInsideWebFrameViews();

#if 0
    NSObject* copyDOMTree(WebCoreDOMTreeCopier& copier);
    NSObject* copyRenderTree(WebCoreRenderTreeCopier copier);
#endif
    const gchar* renderTreeAsExternalRepresentation();
    
    void elementAtPoint(int x, int y, WebCoreElementInfo* info);

    bool searchFor(const gchar* string, bool forward, bool caseSensitive, bool wrap);
    void jumpToSelection();

    void setTextSizeMultiplier(float multiplier);
    const gchar* textEncoding();

    const gchar* stringByEvaluatingJavaScriptFromString(const gchar* string);
#if 0    
    WebDOMDocument* DOMDocument();
#endif

    bool interceptKeyEvent(GdkEventKey *event, bool autoRepeat);

    void setShouldCreateRenderers(bool f);
    bool shouldCreateRenderers();
    int numPendingOrLoadingRequests();

    void bodyBackgroundColor(GdkColor* outColor);

    void adjustViewSize();

    void updateAllViews();

    // KWIQ
    bool hasStaticBackground();    

    // @protocol --  the client side of the bridge
     
public:
    // returns WebCoreBridge objects 
    virtual GList* childFrames()=0; 
    
    virtual WebCoreBridge* mainFrame()=0;
    
    virtual WebCoreBridge* findFrameNamed(const gchar* name)=0;

    // Creates a name for an frame unnamed in the HTML. It should produce repeatable results for loads of the same frameset.  
    virtual const gchar* generateFrameName()=0; 

    virtual void frameDetached()=0;

    virtual GtkWidget* documentView()=0;
    
    virtual void loadURL(const gchar* URL, const gchar* referrer, bool reload, bool onLoadEvent, const gchar* target, NSEvent *event, NRCit::DOMElement* form, GHashTable* formValues)=0;
    virtual void postWithURL(const gchar* URL, const gchar* referrer, const gchar* target, GByteArray* data, const gchar* contentType, NSEvent *event, NRCit::DOMElement *form, GHashTable *formValues)=0;

    virtual WebCoreBridge* createWindowWithURL(const gchar* URL, const gchar* name)=0;
    virtual void showWindow()=0;

    virtual const gchar* userAgentForURL(const gchar* URL)=0;

    virtual void setTitle(const gchar* title)=0;
    virtual void setStatusText(const gchar* status)=0;

    virtual void setIconURL(const gchar* URL)=0;
    virtual void setIconURL(const gchar* URL, const gchar* string)=0;

    virtual WebCoreBridge* createChildFrameNamed(const gchar* frameName, 
				      const gchar* URL,
				      KHTMLRenderPart *renderPart,
				      bool allowsScrolling, 
				      int marginWidth, 
				      int marginHeight)=0;


    virtual bool areToolbarsVisible()=0;
    virtual void setToolbarsVisible(bool visible)=0;
    virtual bool isStatusBarVisible()=0;
    virtual void setStatusBarVisible(bool visible)=0;
    virtual bool areScrollbarsVisible()=0;
    virtual void setScrollbarsVisible(bool visible)=0;

    virtual void setWindowFrame(GdkRectangle* rect)=0;
    virtual void windowFrame(GdkRectangle* outRect)=0;
    virtual void setWindowContentRect(GdkRectangle* rect)=0;
    virtual void windowContentRect(GdkRectangle* outRect)=0;

    virtual void setWindowIsResizable(bool resizable)=0;
    virtual bool windowIsResizable()=0;
    
    virtual GtkWidget *firstResponder()=0;
    virtual void makeFirstResponder(GtkWidget *view)=0;

    virtual void closeWindowSoon()=0;

    virtual void runJavaScriptAlertPanelWithMessage(const gchar* message)=0;
    virtual bool runJavaScriptConfirmPanelWithMessage(const gchar* message)=0;
    virtual bool runJavaScriptTextInputPanelWithPrompt(const gchar* prompt, 
						       const gchar* defaultText, 
						       gchar** result)=0;
    virtual void addMessageToConsole(const gchar* message, unsigned lineNumber, const gchar* sourceURL)=0;

    virtual WebCoreResourceHandle* startLoadingResource(WebCoreResourceLoader *loader, 
						    const gchar* URL, 
                                                    GHashTable* headers, 
						    GByteArray* requestBody)=0;
    virtual WebCoreResourceHandle* startLoadingResource(WebCoreResourceLoader *loader,
						    const gchar* URL, 
						    GHashTable* headers)=0;
    virtual void objectLoadedFromCacheWithURL(const gchar* URL, KWIQResponse* response, unsigned bytes)=0;
#if 0
    virtual NSData* syncLoadResourceWithURL(NSURL *URL,
					    NSDictionary *requestHeaders, 
					    NSData *postData, 
					    NSURL **finalNSURL, 
					    NSDictionary **responseHeaderDict, 
					    int *statusCode)=0;
#endif


    virtual bool isReloading()=0;


    virtual void reportClientRedirectToURL(const gchar* URL, 
					   double seconds, 
					   GTime date, 
					   bool lockHistory, 
					   bool isJavaScriptFormAction)=0;
    virtual void reportClientRedirectCancelled(bool cancelWithLoadInProgress)=0;

    virtual void focusWindow()=0;
    virtual void unfocusWindow()=0;

    virtual GtkWidget* nextKeyViewOutsideWebFrameViews()=0;
    virtual GtkWidget* previousKeyViewOutsideWebFrameViews()=0;

    virtual bool defersLoading()=0;
    virtual void setDefersLoading(bool loading)=0;
    virtual void saveDocumentState(GList* documentState)=0; // ownership of documentState is transferred 
    virtual GList* documentState()=0;  // ownership of documentState is transferred  (again)


    virtual void setNeedsReapplyStyles()=0;

    virtual const gchar* requestedURLString()=0;
    virtual const gchar* incomingReferrer()=0;

    virtual GtkWidget* widgetForPluginWithURL(const gchar* URL,
					    GList* attributesArray,
					    const gchar* baseURLString,
					    const gchar* MIMEType) = 0;

    virtual bool saveDocumentToPageCache(KWIQPageState *documentInfo)=0;

    virtual int getObjectCacheSize()=0;

    virtual bool frameRequiredForMIMEType(const gchar* mimeType, const gchar* URL)=0;

    virtual void loadEmptyDocumentSynchronously()=0;

    virtual const gchar* MIMETypeForPath(const gchar* path)=0;

    virtual void handleMouseDragged(NSEvent *event)=0;
    virtual void handleAutoscrollForMouseDragged(NSEvent *event)=0;
    virtual bool mayStartDragWithMouseDragged(NSEvent *event)=0;

    virtual int historyLength()=0;
    virtual void goBackOrForward(int distance)=0;

    virtual void setHasBorder(bool hasBorder)=0;

    virtual WebCoreKeyboardUIMode keyboardUIMode()=0;
 
    virtual void didSetName(const gchar* name) =0;

    virtual void print()=0;
#if 0
    virtual jobject pollForAppletInView(NSView *view)=0;
    NSUndoManager *undoManager;
#endif
    
    virtual void issueCutCommand()=0;
    virtual void issueCopyCommand()=0;
    virtual void issuePasteCommand()=0;
    virtual void respondToChangedSelection()=0;
    virtual void respondToChangedContents()=0;
#if 0
    virtual bool interceptEditingKeyEvent(NSEvent *event)=0;
    virtual void setIsSelected(bool isSelected, NSView *view)=0;
    virtual bool isEditable()=0;
    virtual bool shouldBeginEditing(DOMRange *range)=0;
    virtual bool shouldEndEditing(DOMRange *range)=0;
#endif
    virtual const gchar* overrideMediaType()=0;
    virtual void windowObjectCleared()=0;    
};

/* POD struct (for now)
 * strings must be freed */ 
struct WebCoreElementInfo
{
    gboolean isSelected;
    gchar* linkTitle;
    gchar* linkLabel;
    gchar* linkURL;
    gchar* linkTarget;
    // x = y = w = h= -1 if there's no image */
    GdkRectangle imageRect;
    gchar* imageURL;
    gchar* imageAltText;
    WebCoreElementInfo();
};

#endif
