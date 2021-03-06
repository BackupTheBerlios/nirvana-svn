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
#include <gtk/gtk.h>

#include "KWQKHTMLPart.h"

#if 0
#include "DOMInternal.h"
#include "KWQClipboard.h"
#endif
#include "KWQDOMNode.h"
#if 0
#include "KWQDummyView.h"
#include "KWQExceptions.h"
#endif
#include "KWQKJobClasses.h"
#include "KWQLogging.h"
#include "KWQPageState.h"
#include "KWQPrinter.h"
#include "KWQWindowWidget.h"
#if 0
#include "WebCoreBridge.h"
#include "WebCoreViewFactory.h"
#endif
#include "csshelper.h"
#include "dom2_eventsimpl.h"
#include "dom2_rangeimpl.h"
#include "dom_position.h"
#include "dom_selection.h"
#include "dom_textimpl.h"
#include "html_documentimpl.h"
#include "html_misc.h"
#include "htmltokenizer.h"
#include "khtml_text_operations.h"
#include "khtmlpart_p.h"
#include "khtmlview.h"
#include "kjs_binding.h"
#include "kjs_window.h"
#include "misc/htmlattrs.h"
#include "qscrollbar.h"
#include "render_canvas.h"
#include "render_frames.h"
#include "render_image.h"
#include "render_list.h"
#include "render_style.h"
#include "render_table.h"
#include "render_text.h"

#include <JavaScriptCore/identifier.h>
#include <JavaScriptCore/property_map.h>
#if 0
#include <JavaScriptCore/runtime.h>
#include <JavaScriptCore/runtime_root.h>
#include <JavaScriptCore/WebScriptObjectPrivate.h>
#endif
#include "WebCoreBridge.h"
#include "KWIQMacros.h"


#undef _KWQ_TIMING

using DOM::AtomicString;
using DOM::ClipboardEventImpl;
using DOM::DocumentImpl;
using DOM::DOMString;
using DOM::ElementImpl;
using DOM::EventImpl;
using DOM::HTMLDocumentImpl;
using DOM::HTMLElementImpl;
using DOM::HTMLFormElementImpl;
using DOM::HTMLFrameElementImpl;
using DOM::HTMLGenericFormElementImpl;
using DOM::HTMLTableCellElementImpl;
using DOM::Node;
using DOM::NodeImpl;
using DOM::Position;
using DOM::Range;
using DOM::RangeImpl;
using DOM::Selection;
using DOM::TextImpl;

using khtml::Cache;
using khtml::ChildFrame;
using khtml::Decoder;
using khtml::findPlainText;
using khtml::InlineTextBox;
using khtml::MouseDoubleClickEvent;
using khtml::MouseMoveEvent;
using khtml::MousePressEvent;
using khtml::MouseReleaseEvent;
using khtml::parseURL;
using khtml::PRE;
using khtml::RenderCanvas;
using khtml::RenderImage;
using khtml::RenderLayer;
using khtml::RenderListItem;
using khtml::RenderObject;
using khtml::RenderPart;
using khtml::RenderStyle;
using khtml::RenderTableCell;
using khtml::RenderText;
using khtml::RenderWidget;
using khtml::VISIBLE;

using KIO::Job;

using KJS::Interpreter;
using KJS::Location;
using KJS::SavedBuiltins;
using KJS::SavedProperties;
using KJS::ScheduledAction;
using KJS::Window;

#if 0
using KJS::Bindings::Instance;
#endif

using KParts::ReadOnlyPart;
using KParts::URLArgs;
class NSEvent
{
    int _clickCount;
public:
    NSEvent() : _clickCount(0) {};
    int clickCount() { return _clickCount;}
};

NSEvent *KWQKHTMLPart::_currentEvent = new NSEvent;
NSResponder *KWQKHTMLPart::_firstResponderAtMouseDownTime = NULL;

void KHTMLPart::completed()
{
    KWQ(this)->_completed.call();
}

void KHTMLPart::completed(bool arg)
{
    KWQ(this)->_completed.call(arg);
}

void KHTMLPart::nodeActivated(const Node &)
{
}

bool KHTMLPart::openURL(const KURL &URL)
{
    ASSERT_NOT_REACHED();
    return true;
}

void KHTMLPart::onURL(const QString &)
{
}

void KHTMLPart::setStatusBarText(const QString &status)
{
    KWQ(this)->setStatusBarText(status);
}

void KHTMLPart::started(Job *j)
{
    KWQ(this)->_started.call(j);
}

static void redirectionTimerMonitor(void *context)
{
    KWQKHTMLPart *kwq = static_cast<KWQKHTMLPart *>(context);
    kwq->redirectionTimerStartedOrStopped();
}

KWQKHTMLPart::KWQKHTMLPart()
    : _bridge(NULL)
    , _started(this, SIGNAL(started(KIO::Job *)))
    , _completed(this, SIGNAL(completed()))
    , _completedWithBool(this, SIGNAL(completed(bool)))
    , _mouseDownView(NULL)
    , _mouseDownWasInSubframe(false)
    , _sendingEventToSubview(false)
    , _mouseDownMayStartDrag(false)
    , _mouseDownMayStartSelect(false)
    , _formValuesAboutToBeSubmitted(0)
    , _formAboutToBeSubmitted(0)
    , _windowWidget(NULL)
    , _usesInactiveTextBackgroundColor(false)
    , _showsFirstResponder(true)
    , _drawSelectionOnly(false)
    , _elementToDraw(0)
{
    QOBJECT_TYPE(KWQKHTMLPart);
    // Must init the cache before connecting to any signals
    Cache::init();

    // The widget is made outside this class in our case.
    KHTMLPart::init( 0, DefaultGUI );

    mutableInstances().prepend(this);
    d->m_redirectionTimer.setMonitor(redirectionTimerMonitor, this);
}

KWQKHTMLPart::~KWQKHTMLPart()
{
    // ~KHTMLPart::KHTMLPart() calls d->m_redirectionTimer.stop()
    // would it be better idea to stop the timer here ? --ktkinnun
    d->m_redirectionTimer.setMonitor(0, 0);

#if 0    
    cleanupPluginRootObjects();
#endif    
    mutableInstances().remove(this);
    if (d->m_view) {
	d->m_view->deref();
    }
#if 0
    freeClipboard();
#endif
    if (_formValuesAboutToBeSubmitted) {
	g_hash_table_destroy(_formValuesAboutToBeSubmitted);
    }
    _formAboutToBeSubmitted = 0;
    delete _windowWidget;
 
}

#if 0
void KWQKHTMLPart::freeClipboard()
{
    if (_dragClipboard) {
        _dragClipboard->setAccessPolicy(KWQClipboard::Numb);
	_dragClipboard->deref();
        _dragClipboard = 0;
    }
}
#endif

void KWQKHTMLPart::setSettings (KHTMLSettings *settings)
{
    d->m_settings = settings;
}

QString KWQKHTMLPart::generateFrameName()
{
    return QString::fromUtf8(_bridge->generateFrameName());
}

void KWQKHTMLPart::provisionalLoadStarted()
{
    // we don't want to wait until we get an actual http response back
    // to cancel pending redirects, otherwise they might fire before
    // that happens.
    cancelRedirection(true);
}

bool KWQKHTMLPart::openURL(const KURL &url)
{
    bool onLoad = false;
    
    if (jScript() && jScript()->interpreter()) {
        KHTMLPart *rootPart = this;
        while (rootPart->parentPart() != 0)
            rootPart = rootPart->parentPart();
        KJS::ScriptInterpreter *interpreter = static_cast<KJS::ScriptInterpreter *>(KJSProxy::proxy(rootPart)->interpreter());
        DOM::Event *evt = interpreter->getCurrentEvent();
        
        if (evt) {
            onLoad = (evt->type() == "load");
        }
    }
    
    // FIXME: The lack of args here to get the reload flag from
    // indicates a problem in how we use KHTMLPart::processObjectRequest,
    // where we are opening the URL before the args are set up.
    // FIXME: utf8? -- psalmi
    _bridge->loadURL(url.url().utf8(),
		     _bridge->referrer(),		     
		     false,  	// reload: NO
		     onLoad,    // onLoadEvent: onLoad
		     "",	// target: nil
		     0,		        // triggeringEvent: nil
		     0,		// form: nil
		     0); // formValues
    
        
    return true;
}

void KWQKHTMLPart::openURLRequest(const KURL &url, const URLArgs &args)
{
    //FIXME: utf8? -- psalmi
    _bridge->loadURL(url.url().utf8(),
		     _bridge->referrer(),
		     args.reload, // reload: args.reload
		     false,	// onLoadEvent: false
		     args.frameName.utf8(), // frame: args.frameName
		     0,		// event: nil       
		     0,		// form: nil
		     0);	// formvalues: nil

}

void KWQKHTMLPart::didNotOpenURL(const KURL &URL)
{
    if (_submittedFormURL == URL) {
        _submittedFormURL = KURL();
    }
}

// Scans logically forward from "start", including any child frames
static HTMLFormElementImpl *scanForForm(NodeImpl *start)
{
    NodeImpl *n;
    for (n = start; n; n = n->traverseNextNode()) {
        NodeImpl::Id nodeID = idFromNode(n);
        if (nodeID == ID_FORM) {
            return static_cast<HTMLFormElementImpl *>(n);
        } else if (n->isHTMLElement()
                   && static_cast<HTMLElementImpl *>(n)->isGenericFormElement()) {
            return static_cast<HTMLGenericFormElementImpl *>(n)->form();
        } else if (nodeID == ID_FRAME || nodeID == ID_IFRAME) {
            NodeImpl *childDoc = static_cast<HTMLFrameElementImpl *>(n)->contentDocument();
            HTMLFormElementImpl *frameResult = scanForForm(childDoc);
            if (frameResult) {
                return frameResult;
            }
        }
    }
    return 0;
}

// We look for either the form containing the current focus, or for one immediately after it
HTMLFormElementImpl *KWQKHTMLPart::currentForm() const
{
    // start looking either at the active (first responder) node, or where the selection is
    NodeImpl *start = activeNode().handle();
    if (!start) {
        start = selectionStart();
    }

    // try walking up the node tree to find a form element
    NodeImpl *n;
    for (n = start; n; n = n->parentNode()) {
        if (idFromNode(n) == ID_FORM) {
            return static_cast<HTMLFormElementImpl *>(n);
        } else if (n->isHTMLElement()
                   && static_cast<HTMLElementImpl *>(n)->isGenericFormElement()) {
            return static_cast<HTMLGenericFormElementImpl *>(n)->form();
        }
    }

    // try walking forward in the node tree to find a form element
    if (!start) {
        start = xmlDocImpl();
    }
    return scanForForm(start);
}

#if 0 //FIXME: KWIQ: commented block
// Either get cached regexp or build one that matches any of the labels.
// The regexp we build is of the form:  (STR1|STR2|STRN)
QRegExp *regExpForLabels(NSArray *labels)
{
    // All the ObjC calls in this method are simple array and string
    // calls which we can assume do not raise exceptions


    // Parallel arrays that we use to cache regExps.  In practice the number of expressions
    // that the app will use is equal to the number of locales is used in searching.
    static const unsigned int regExpCacheSize = 4;
    static NSMutableArray *regExpLabels = nil;
    static QPtrList <QRegExp> regExps;
    static QRegExp wordRegExp = QRegExp("\\w");

    QRegExp *result;
    if (!regExpLabels) {
        regExpLabels = [[NSMutableArray alloc] initWithCapacity:regExpCacheSize];
    }
    unsigned int cacheHit = [regExpLabels indexOfObject:labels];
    if (cacheHit != NSNotFound) {
        result = regExps.at(cacheHit);
    } else {
        QString pattern("(");
        unsigned int numLabels = [labels count];
        unsigned int i;
        for (i = 0; i < numLabels; i++) {
            QString label = QString::fromNSString([labels objectAtIndex:i]);

            bool startsWithWordChar = false;
            bool endsWithWordChar = false;
            if (label.length() != 0) {
                startsWithWordChar = wordRegExp.search(label.at(0)) >= 0;
                endsWithWordChar = wordRegExp.search(label.at(label.length() - 1)) >= 0;
            }
            
            if (i != 0) {
                pattern.append("|");
            }
            // Search for word boundaries only if label starts/ends with "word characters".
            // If we always searched for word boundaries, this wouldn't work for languages
            // such as Japanese.
            if (startsWithWordChar) {
                pattern.append("\\b");
            }
            pattern.append(label);
            if (endsWithWordChar) {
                pattern.append("\\b");
            }
        }
        pattern.append(")");
        result = new QRegExp(pattern, false);
    }

    // add regexp to the cache, making sure it is at the front for LRU ordering
    if (cacheHit != 0) {
        if (cacheHit != NSNotFound) {
            // remove from old spot
            [regExpLabels removeObjectAtIndex:cacheHit];
            regExps.remove(cacheHit);
        }
        // add to start
        [regExpLabels insertObject:labels atIndex:0];
        regExps.insert(0, result);
        // trim if too big
        if ([regExpLabels count] > regExpCacheSize) {
            [regExpLabels removeObjectAtIndex:regExpCacheSize];
            QRegExp *last = regExps.last();
            regExps.removeLast();
            delete last;
        }
    }
    return result;
}

NSString *KWQKHTMLPart::searchForLabelsAboveCell(QRegExp *regExp, HTMLTableCellElementImpl *cell)
{
    RenderTableCell *cellRenderer = static_cast<RenderTableCell *>(cell->renderer());
    RenderTableCell *cellAboveRenderer = cellRenderer->table()->cellAbove(cellRenderer);

    if (cellAboveRenderer) {
        HTMLTableCellElementImpl *aboveCell =
            static_cast<HTMLTableCellElementImpl *>(cellAboveRenderer->element());

        if (aboveCell) {
            // search within the above cell we found for a match
            for (NodeImpl *n = aboveCell->firstChild(); n; n = n->traverseNextNode(aboveCell)) {
                if (idFromNode(n) == ID_TEXT
                    && n->renderer() && n->renderer()->style()->visibility() == VISIBLE)
                {
                    // For each text chunk, run the regexp
                    QString nodeString = n->nodeValue().string();
                    int pos = regExp->searchRev(nodeString);
                    if (pos >= 0) {
                        return nodeString.mid(pos, regExp->matchedLength()).getNSString();
                    }
                }
            }
        }
    }
    // Any reason in practice to search all cells in that are above cell?
    return nil;
}

NSString *KWQKHTMLPart::searchForLabelsBeforeElement(NSArray *labels, ElementImpl *element)
{
    QRegExp *regExp = regExpForLabels(labels);
    // We stop searching after we've seen this many chars
    const unsigned int charsSearchedThreshold = 500;
    // This is the absolute max we search.  We allow a little more slop than
    // charsSearchedThreshold, to make it more likely that we'll search whole nodes.
    const unsigned int maxCharsSearched = 600;
    // If the starting element is within a table, the cell that contains it
    HTMLTableCellElementImpl *startingTableCell = 0;
    bool searchedCellAbove = false;

    // walk backwards in the node tree, until another element, or form, or end of tree
    int unsigned lengthSearched = 0;
    NodeImpl *n;
    for (n = element->traversePreviousNode();
         n && lengthSearched < charsSearchedThreshold;
         n = n->traversePreviousNode())
    {
        NodeImpl::Id nodeID = idFromNode(n);
        if (nodeID == ID_FORM
            || (n->isHTMLElement()
                && static_cast<HTMLElementImpl *>(n)->isGenericFormElement()))
        {
            // We hit another form element or the start of the form - bail out
            break;
        } else if (nodeID == ID_TD && !startingTableCell) {
            startingTableCell = static_cast<HTMLTableCellElementImpl *>(n);
        } else if (nodeID == ID_TR && startingTableCell) {
            NSString *result = searchForLabelsAboveCell(regExp, startingTableCell);
            if (result) {
                return result;
            }
            searchedCellAbove = true;
        } else if (nodeID == ID_TEXT
                   && n->renderer() && n->renderer()->style()->visibility() == VISIBLE)
        {
            // For each text chunk, run the regexp
            QString nodeString = n->nodeValue().string();
            // add 100 for slop, to make it more likely that we'll search whole nodes
            if (lengthSearched + nodeString.length() > maxCharsSearched) {
                nodeString = nodeString.right(charsSearchedThreshold - lengthSearched);
            }
            int pos = regExp->searchRev(nodeString);
            if (pos >= 0) {
                return nodeString.mid(pos, regExp->matchedLength()).getNSString();
            } else {
                lengthSearched += nodeString.length();
            }
        }
    }

    // If we started in a cell, but bailed because we found the start of the form or the
    // previous element, we still might need to search the row above us for a label.
    if (startingTableCell && !searchedCellAbove) {
         return searchForLabelsAboveCell(regExp, startingTableCell);
    } else {
        return nil;
    }
}

NSString *KWQKHTMLPart::matchLabelsAgainstElement(NSArray *labels, ElementImpl *element)
{
    QString name = element->getAttribute(ATTR_NAME).string();
    // Make numbers and _'s in field names behave like word boundaries, e.g., "address2"
    name.replace(QRegExp("[[:digit:]]"), " ");
    name.replace('_', ' ');
    
    QRegExp *regExp = regExpForLabels(labels);
    // Use the largest match we can find in the whole name string
    int pos;
    int length;
    int bestPos = -1;
    int bestLength = -1;
    int start = 0;
    do {
        pos = regExp->search(name, start);
        if (pos != -1) {
            length = regExp->matchedLength();
            if (length >= bestLength) {
                bestPos = pos;
                bestLength = length;
            }
            start = pos+1;
        }
    } while (pos != -1);

    if (bestPos != -1) {
        return name.mid(bestPos, bestLength).getNSString();
    } else {
        return nil;
    }
}
#endif // if 0


// Search from the end of the currently selected location if we are first responder, or from
// the beginning of the document if nothing is selected or we're not first responder.
bool KWQKHTMLPart::findString(const QString& target, bool forward, bool caseFlag, bool wrapFlag)
{
    if (target.isEmpty()) {
        return false;
    }

    // Start on the correct edge of the selection, search to edge of document.
    Range searchRange(xmlDocImpl());
    searchRange.selectNodeContents(xmlDocImpl());
    if (selectionStart()) {
        if (forward) {
            searchRange.setStart(selectionEnd(), selectionEndOffset());
        } else {
            searchRange.setEnd(selectionStart(), selectionStartOffset());
        }
    }

    // Do the search once, then do it a second time to handle wrapped search.
    // Searches some or all of document twice in the failure case, but that's probably OK.
    Range resultRange = findPlainText(searchRange, target, forward, caseFlag);
    if (resultRange.collapsed() && wrapFlag) {
        searchRange.selectNodeContents(xmlDocImpl());
        resultRange = findPlainText(searchRange, target, forward, caseFlag);
        // If we got back to the same place we started, that doesn't count as success.
        if (resultRange == selection().toRange()) {
            return false;
        }
    }

    if (resultRange.collapsed()) {
        return false;
    }

    setSelection(resultRange);
    jumpToSelection();
    return true;
}

void KWQKHTMLPart::clearRecordedFormValues()
{
    if (_formValuesAboutToBeSubmitted) {
	g_hash_table_destroy(_formValuesAboutToBeSubmitted);
	_formValuesAboutToBeSubmitted = 0;
	_formAboutToBeSubmitted = 0;
    }
}

extern "C" {
void formValuesAboutToBeSubmitted_value_key_destroy(gpointer data)
{
    gchar *c = static_cast<gchar *>(data);
    g_free(c);
}
}

void KWQKHTMLPart::recordFormValue(const QString &name, const QString &value, DOM::HTMLFormElementImpl *element)
{
    if (!_formValuesAboutToBeSubmitted){
	_formValuesAboutToBeSubmitted = g_hash_table_new_full(g_str_hash, g_str_equal,
							      formValuesAboutToBeSubmitted_value_key_destroy,
							      formValuesAboutToBeSubmitted_value_key_destroy);
							      
	_formAboutToBeSubmitted = 0; // element; <-- wrap that with DOM api
    } else {
	//ASSERT(_formAboutToBeSubmitted->impl() == element);
    }
    g_hash_table_insert(_formValuesAboutToBeSubmitted, g_strdup(name.utf8()), g_strdup(value.utf8()));
}

void KWQKHTMLPart::submitForm(const KURL &url, const URLArgs &args)
{
    // The form multi-submit logic here is only right when we are submitting a form that affects this frame.
    // Eventually when we find a better fix we can remove this altogether.
    // FIXME: utf8? -- psalmi
    WebCoreBridge *target = args.frameName.isEmpty() ? _bridge : _bridge->findFrameNamed(args.frameName.utf8());
    if (!target) target = _bridge;
    KHTMLPart *targetPart = target->part();
    bool willReplaceThisFrame = false;
    for (KHTMLPart *p = this; p; p = p->parentPart()) {
        if (p == targetPart) {
            willReplaceThisFrame = true;
            break;
        }
    }
    if (willReplaceThisFrame) {
        // We do not want to submit more than one form from the same page,
        // nor do we want to submit a single form more than once.
        // This flag prevents these from happening.
        // Note that the flag is reset in setView()
        // since this part may get reused if it is pulled from the b/f cache.
        if (_submittedFormURL == url) {
            return;
        }
        _submittedFormURL = url;
    }

    if (!args.doPost()) {
        // FIXME: utf8? -- psalmi
        _bridge->loadURL(url.url().utf8(),
			 _bridge->referrer(),
			 args.reload,
			 false,
			 args.frameName.utf8(),
			 _currentEvent,
			 _formAboutToBeSubmitted,
			 _formValuesAboutToBeSubmitted);
	
    } else {
        ASSERT(args.contentType().startsWith("Content-Type: "));

        const int size = args.postData.size();
        GByteArray * postData = g_byte_array_sized_new(size);
        g_byte_array_append(postData, reinterpret_cast<guint8 *>(g_strdup(args.postData.data())), size);
        
        _bridge->postWithURL(url.url().utf8(),
			     _bridge->referrer(),
			     args.frameName.utf8(),
			     postData,
			     args.contentType().mid(14).utf8(),
			     _currentEvent,
			     _formAboutToBeSubmitted,
			     _formValuesAboutToBeSubmitted);

        g_byte_array_free(postData, true);
    }
    clearRecordedFormValues();    
}

void KWQKHTMLPart::setEncoding(const QString &name, bool userChosen)
{
    if (!d->m_workingURL.isEmpty()) {
        receivedFirstData();
    }
    d->m_encoding = name;
    d->m_haveEncoding = userChosen;
}

void KWQKHTMLPart::addData(const char *bytes, int length)
{
    ASSERT(d->m_workingURL.isEmpty());
    ASSERT(d->m_doc);
    ASSERT(d->m_doc->parsing());
    write(bytes, length);
}

void KHTMLPart::frameDetached()
{
    KWQ(this)->bridge()->frameDetached();

    // FIXME: There may be a better place to do this that works for KHTML too.
    FrameList& parentFrames = parentPart()->d->m_frames;
    FrameIt end = parentFrames.end();
    for (FrameIt it = parentFrames.begin(); it != end; ++it) {
        if ((*it).m_part == this) {
            parentFrames.remove(it);
            deref();
            break;
        }
    }
}

void KWQKHTMLPart::urlSelected(const KURL &url, int button, int state, const URLArgs &args)
{
    _bridge->loadURL(url.url().utf8(),
		     _bridge->referrer(),
		     args.reload,
		     false,
		     args.frameName.utf8(),
		     _currentEvent,
		     0,		
		     0);
}


class KWQPluginPart : public ReadOnlyPart
{
    virtual bool openURL(const KURL &) { return true; }
    virtual bool closeURL() { return true; }
public:
};

KParts::ReadOnlyPart *KWQKHTMLPart::createPart(const khtml::ChildFrame &child, const KURL &url, const QString &mimeType)
{
    KParts::ReadOnlyPart *part;
    
    bool needFrame = _bridge->frameRequiredForMIMEType(mimeType.utf8(), url.url().utf8());
    if (child.m_type == ChildFrame::Object && !needFrame) {
	KWQPluginPart *newPart = new KWQPluginPart;

	GList* params = 0;
        for (uint i = 0; i < child.m_params.count(); i++){
            QString s = child.m_params[i];
            params = g_list_append(params, g_strdup(s.utf8()));
        }

	newPart->setWidget(new QWidget( _bridge->widgetForPluginWithURL(url.url().utf8(), 
                                                                        params,
                                                                        d->m_doc->baseURL().utf8(),
                                                                        child.m_args.serviceType.utf8())));
	part = newPart;

	GList* iter = g_list_first(params);
	while (iter) {
	    g_free(iter->data);
	    iter = g_list_next(iter);
	}

        g_list_free(params);
    } else {
        LOG(Frames, "name %s", child.m_name.ascii());
	bool allowsScrolling = true;
	int marginWidth = -1;
	int marginHeight = -1;
        if (child.m_type != ChildFrame::Object) {
            DOM::HTMLFrameElementImpl *o = static_cast<DOM::HTMLFrameElementImpl *>(child.m_frame->element());
	    allowsScrolling = o->scrollingMode() != QScrollView::AlwaysOff;
             marginWidth = o->getMarginWidth();
             marginHeight = o->getMarginHeight();
	}
	WebCoreBridge *childBridge = _bridge->createChildFrameNamed(child.m_name.utf8(),
									 url.url().utf8(),
									 child.m_frame,
									 allowsScrolling,
									 marginWidth,
									 marginHeight);
	// This call needs to return an object with a ref, since the caller will expect to own it.
	// childBridge owns the only ref so far.
	childBridge->part()->ref();
        part = childBridge->part();
    }
    
    return part;
}

void KWQKHTMLPart::setView(KHTMLView *view)
{
    // Detach the document now, so any onUnload handlers get run - if
    // we wait until the view is destroyed, then things won't be
    // hooked up enough for some JavaScript calls to work.
    if (d->m_doc && view == NULL) {
	d->m_doc->detach();
    }

    if (view) {
	view->ref();
    }
    if (d->m_view) {
	d->m_view->deref();
    }
    d->m_view = view;
    setWidget(view);

    // Only one form submission is allowed per view of a part.
    // Since this part may be getting reused as a result of being
    // pulled from the back/forward cache, reset this flag.
    _submittedFormURL = KURL();
}

KHTMLView *KWQKHTMLPart::view() const
{
    return d->m_view;
}

void KWQKHTMLPart::setTitle(const DOM::DOMString &title)
{
    QString text = title.string();
    text.replace('\\', backslashAsCurrencySymbol());
    // FIXME: utf8? -- psalmi
    _bridge->setTitle(text.utf8());
};

void KWQKHTMLPart::setStatusBarText(const QString &status)
{
    QString text = status;
    text.replace('\\', backslashAsCurrencySymbol());
    // FIXME: utf8? -- psalmi
    _bridge->setStatusText(text.utf8());
}

void KWQKHTMLPart::scheduleClose()
{
    _bridge->closeWindowSoon();
}

void KWQKHTMLPart::unfocusWindow()
{
    _bridge->unfocusWindow();
}

void KWQKHTMLPart::jumpToSelection()
{
    const DOM::Selection &s = selection(); 
    
    // Assumes that selection will only ever be text nodes. This is currently
    // true, but will it always be so?
    if (!s.start().node()) {	
#if KWIQ 
        RenderText *rt = 0;
	if (s.start().node()->renderer() && s.start().node()->renderer()->isText())
	    rt = static_cast<RenderText *>(s.start().node()->renderer());
#else	
        RenderText *rt = dynamic_cast<RenderText *>(s.start().node()->renderer());
#endif	
        if (rt) {
            int x = 0, y = 0;
            rt->posOfChar(s.start().offset(), x, y);
            // The -50 offset is copied from KHTMLPart::findTextNext, which sets the contents position
            // after finding a matched text string.
           d->m_view->setContentsPos(x - 50, y - 50);
        }
/*
        I think this would be a better way to do this, to avoid needless horizontal scrolling,
        but it is not feasible until selectionRect() returns a tighter rect around the
        selected text.  Right now it works at element granularity.
 
        NSView *docView = d->m_view->getDocumentView();

        NSRect selRect = NSRect(selectionRect());
        NSRect visRect = [docView visibleRect];
        if (!NSContainsRect(visRect, selRect)) {
            // pad a bit so we overscroll slightly
            selRect = NSInsetRect(selRect, -10.0, -10.0);
            selRect = NSIntersectionRect(selRect, [docView bounds]);
            [docView scrollRectToVisible:selRect];
        }
*/
    }
}

void KWQKHTMLPart::redirectionTimerStartedOrStopped()
{
    // Don't report history navigations, just actual redirection.
    if (d->m_scheduledRedirection == historyNavigationScheduled) {
        return;
    }

    // FIXME: utf8? -- psalmi
    GTime time = 0;
    if (d->m_redirectionTimer.isActive()) {
	_bridge->reportClientRedirectToURL(d->m_redirectURL.utf8(),
					   d->m_delayRedirect,
					   //fireDate:[d->m_redirectionTimer.getNSTimer() fireDate],
					   time,
					   d->m_redirectLockHistory,
					   d->m_executingJavaScriptFormAction);
    } else {
        _bridge->reportClientRedirectCancelled(d->m_cancelWithLoadInProgress);
    }
}

void KWQKHTMLPart::paint(QPainter *p, const QRect &rect)
{
#ifndef NDEBUG
    bool fillWithRed;
    if (p->device()->devType() == QInternal::Printer)
        fillWithRed = false; // Printing, don't fill with red (can't remember why).
    else if (!xmlDocImpl() || xmlDocImpl()->ownerElement())
        fillWithRed = false; // Subframe, don't fill with red.
    else if (view() && view()->isTransparent())
        fillWithRed = false; // Transparent, don't fill with red.
    else if (_drawSelectionOnly)
        fillWithRed = false; // Selections are transparent, don't fill with red.
    else if (_elementToDraw != 0)
        fillWithRed = false; // Element images are transparent, don't fill with red.
    else
        fillWithRed = true;

    if (fillWithRed) {
        p->fillRect(rect.x(), rect.y(), rect.width(), rect.height(), QColor(0xFF, 0, 0));
    }
#endif

    if (renderer()) {
        // _elementToDraw is used to draw only one element
        RenderObject *eltRenderer = (_elementToDraw != 0) ? _elementToDraw.handle()->renderer() : 0;
        renderer()->layer()->paint(p, rect, _drawSelectionOnly, eltRenderer);
    } else {
        ERROR("called KWQKHTMLPart::paint with nil renderer");
    }
}

void KWQKHTMLPart::adjustPageHeight(float *newBottom, float oldTop, float oldBottom, float bottomLimit)
{
    RenderCanvas *root = static_cast<RenderCanvas *>(xmlDocImpl()->renderer());
    if (root) {
        // Use a printer device, with painting disabled for the pagination phase
        QPainter painter(true);
        painter.setPaintingDisabled(true);

        root->setTruncatedAt((int)floor(oldBottom));
        QRect dirtyRect(0, (int)floor(oldTop),
                        root->docWidth(), (int)ceil(oldBottom-oldTop));
        root->layer()->paint(&painter, dirtyRect);
        *newBottom = root->bestTruncatedAt();
        if (*newBottom == 0) {
            *newBottom = oldBottom;
        }
    } else {
        *newBottom = oldBottom;
    }
}

RenderObject *KWQKHTMLPart::renderer()
{
    DocumentImpl *doc = xmlDocImpl();
    return doc ? doc->renderer() : 0;
}

QString KWQKHTMLPart::userAgent() const
{
    // FIXME: utf8? -- psalmi
    return QString(_bridge->userAgentForURL(m_url.url().utf8()));
}

QString KWQKHTMLPart::mimeTypeForFileName(const QString &fileName) const
{
    // FIXME: utf8? -- psalmi
    return _bridge->MIMETypeForPath(fileName.utf8());
}


GtkWidget *KWQKHTMLPart::nextKeyViewInFrame(DOM::NodeImpl *node, KWQSelectionDirection direction)
{
    DocumentImpl *doc = xmlDocImpl();
    if (!doc) {
        return 0;
    }
    for (;;) {
        node = direction == KWQSelectingNext
            ? doc->nextFocusNode(node) : doc->previousFocusNode(node);
        if (!node) {
            return 0;
        }

        RenderWidget *renderWidget = 0;
	if (node->renderer() && node->renderer()->isWidget()) 
	    renderWidget = static_cast<RenderWidget *>(node->renderer());

        if (renderWidget) {
            QWidget *widget = renderWidget->widget();
            KHTMLView *childFrameWidget = 0;

	    if (widget && QOBJECT_IS_A(widget, KHTMLView)) childFrameWidget = static_cast<KHTMLView*>(widget);

            if (childFrameWidget) {
                GtkWidget *view = KWQ(childFrameWidget->part())->nextKeyViewInFrame(0, direction);
                if (view) {
                    return view;
                }
            } else if (widget) {
                GtkWidget *view = widget->getGtkWidget();
                // AppKit won't be able to handle scrolling and making us the first responder
                // well unless we are actually installed in the correct place. KHTML only does
                // that for visible widgets, so we need to do it explicitly here.
                int x, y;
                if (view && renderWidget->absolutePosition(x, y)) {
                    renderWidget->view()->addChild(widget, x, y);
                    return view;
                }
            }
        }
        else {
            doc->setFocusNode(node);
            if (view()) {
                view()->ensureRectVisibleCentered(node->getRect());
            }

            _bridge->makeFirstResponder(_bridge->documentView());
            return _bridge->documentView();
        }
    }
}


GtkWidget *KWQKHTMLPart::nextKeyViewInFrameHierarchy(DOM::NodeImpl *node, KWQSelectionDirection direction)
{
    GtkWidget *next = nextKeyViewInFrame(node, direction);
    if (next) {
        return next;
    }

    // remove focus from currently focused node
    DocumentImpl *doc = xmlDocImpl();
    if (doc) {
        doc->setFocusNode(0);
    }
    
    KWQKHTMLPart *parent = KWQ(parentPart());
    if (parent) {
        next = parent->nextKeyView(parent->childFrame(this)->m_frame->element(), direction);
        if (next) {
            return next;
        }
    }
    
    return 0;
}

GtkWidget *KWQKHTMLPart::nextKeyView(DOM::NodeImpl *node, KWQSelectionDirection direction)
{
    GtkWidget * next = nextKeyViewInFrameHierarchy(node, direction);
    if (next) {
        return next;
    }

    // Look at views from the top level part up, looking for a next key view that we can use.

    next = direction == KWQSelectingNext
        ? _bridge->nextKeyViewOutsideWebFrameViews()
        : _bridge->previousKeyViewOutsideWebFrameViews();

    if (next) {
        return next;
    }

    // If all else fails, make a loop by starting from 0.
    return nextKeyViewInFrameHierarchy(0, direction);
}

GtkWidget *KWQKHTMLPart::nextKeyViewForWidget(QWidget *startingWidget, KWQSelectionDirection direction)
{
    // Use the event filter object to figure out which RenderWidget owns this QWidget and get to the DOM.
    // Then get the next key view in the order determined by the DOM.
    DOM::NodeImpl *node = nodeForWidget(startingWidget);
    ASSERT(node);
    return partForNode(node)->nextKeyView(node, direction);
}

#if 0
bool KWQKHTMLPart::currentEventIsMouseDownInWidget(QWidget *candidate)
{
    KWQ_BLOCK_EXCEPTIONS;
    switch ([[NSApp currentEvent] type]) {
        case NSLeftMouseDown:
        case NSRightMouseDown:
        case NSOtherMouseDown:
            break;
        default:
            return NO;
    }
    KWQ_UNBLOCK_EXCEPTIONS;
    
    NodeImpl *node = nodeForWidget(candidate);
    ASSERT(node);
    return partForNode(node)->nodeUnderMouse() == node;
}
#endif

bool KWQKHTMLPart::currentEventIsKeyboardOptionTab()
{
#if 0
    KWQ_BLOCK_EXCEPTIONS;
    NSEvent *evt = [NSApp currentEvent];
    if ([evt type] != NSKeyDown) {
        return NO;
    }

    if (([evt modifierFlags] & NSAlternateKeyMask) == 0) {
        return NO;
    }
    
    NSString *chars = [evt charactersIgnoringModifiers];
    if ([chars length] != 1)
        return NO;
    
    const unichar tabKey = 0x0009;
    const unichar shiftTabKey = 0x0019;
    unichar c = [chars characterAtIndex:0];
    if (c != tabKey && c != shiftTabKey)
        return NO;
    
    KWQ_UNBLOCK_EXCEPTIONS;
    return YES;
#endif
    return true;
}

#if 0
bool KWQKHTMLPart::handleKeyboardOptionTabInView(NSView *view)
{
    if (KWQKHTMLPart::currentEventIsKeyboardOptionTab()) {
        if (([[NSApp currentEvent] modifierFlags] & NSShiftKeyMask) != 0) {
            [[view window] selectKeyViewPrecedingView:view];
        } else {
            [[view window] selectKeyViewFollowingView:view];
        }
        return YES;
    }
    
    return NO;
}
#endif
bool KWQKHTMLPart::tabsToLinks() const
{
    if (_bridge->keyboardUIMode() & WebCoreKeyboardAccessTabsToLinks)
        return !KWQKHTMLPart::currentEventIsKeyboardOptionTab();
    else
        return KWQKHTMLPart::currentEventIsKeyboardOptionTab();
}

bool KWQKHTMLPart::tabsToAllControls() const
{
    return _bridge->keyboardUIMode() & WebCoreKeyboardAccessFull;
}

#if 0
KJS::Bindings::RootObject *KWQKHTMLPart::bindingRootObject()
{
    if (!_bindingRoot) {
        _bindingRoot = new KJS::Bindings::RootObject(0);    // The root gets deleted by JavaScriptCore.
        KJS::ObjectImp *win = static_cast<KJS::ObjectImp *>(KJS::Window::retrieveWindow(this));
        _bindingRoot->setRootObjectImp (win);
        _bindingRoot->setInterpreter (KJSProxy::proxy(this)->interpreter());
        addPluginRootObject (_bindingRoot);
    }
    return _bindingRoot;
}

WebScriptObject *KWQKHTMLPart::windowScriptObject()
{
    if (!_windowScriptObject) {
        KJS::ObjectImp *win = static_cast<KJS::ObjectImp *>(KJS::Window::retrieveWindow(this));
        _windowScriptObject = KWQRetainNSRelease([[WebScriptObject alloc] _initWithObjectImp:win root:bindingRootObject()]);
    }

    return _windowScriptObject;
}
#endif

void KWQKHTMLPart::partClearedInBegin()
{
    _bridge->windowObjectCleared();
}

QMap<int, ScheduledAction*> *KWQKHTMLPart::pauseActions(const void *key)
{
    if (d->m_doc && d->m_jscript) {
        Window *w = Window::retrieveWindow(this);
        if (w && w->hasTimeouts()) {
            return w->pauseTimeouts(key);
        }
    }
    return 0;
}

void KWQKHTMLPart::resumeActions(QMap<int, ScheduledAction*> *actions, const void *key)
{
    if (d->m_doc && d->m_jscript && d->m_bJScriptEnabled) {
        Window *w = Window::retrieveWindow(this);
        if (w) {
            w->resumeTimeouts(actions, key);
        }
    }
}

bool KWQKHTMLPart::canCachePage()
{
    // Only save page state if:
    // 1.  We're not a frame or frameset.
    // 2.  The page has no unload handler.
    // 3.  The page has no password fields.
    // 4.  The URL for the page is not https.
    // 5.  The page has no applets.
    if (d->m_frames.count() ||
        parentPart() ||
        m_url.protocol().startsWith("https") || 
	(d->m_doc && (htmlDocument().applets().length() != 0 ||
                      d->m_doc->hasWindowEventListener(EventImpl::UNLOAD_EVENT) ||
		      d->m_doc->hasPasswordField()))) {
        return false;
    }
    return true;
}

void KWQKHTMLPart::saveWindowProperties(SavedProperties *windowProperties)
{
    Window *window = Window::retrieveWindow(this);
    if (window)
        window->saveProperties(*windowProperties);
}

void KWQKHTMLPart::saveLocationProperties(SavedProperties *locationProperties)
{
    Window *window = Window::retrieveWindow(this);
    if (window) {
        Interpreter::lock();
        Location *location = window->location();
        Interpreter::unlock();
        location->saveProperties(*locationProperties);
    }
}

void KWQKHTMLPart::restoreWindowProperties(SavedProperties *windowProperties)
{
    Window *window = Window::retrieveWindow(this);
    if (window)
        window->restoreProperties(*windowProperties);
}

void KWQKHTMLPart::restoreLocationProperties(SavedProperties *locationProperties)
{
    Window *window = Window::retrieveWindow(this);
    if (window) {
        Interpreter::lock();
        Location *location = window->location();
        Interpreter::unlock();
        location->restoreProperties(*locationProperties);
    }
}

void KWQKHTMLPart::saveInterpreterBuiltins(SavedBuiltins &interpreterBuiltins)
{
    if (jScript() && jScript()->interpreter()) {
	jScript()->interpreter()->saveBuiltins(interpreterBuiltins);
    }
}

void KWQKHTMLPart::restoreInterpreterBuiltins(const SavedBuiltins &interpreterBuiltins)
{
    if (jScript() && jScript()->interpreter()) {
	jScript()->interpreter()->restoreBuiltins(interpreterBuiltins);
    }
}

void KWQKHTMLPart::openURLFromPageCache(KWQPageState *state)
{
    DocumentImpl *doc = state->document();
    KURL *url = state->URL();
    SavedProperties *windowProperties = state->windowProperties();
    SavedProperties *locationProperties = state->locationProperties();
    SavedBuiltins *interpreterBuiltins = state->interpreterBuiltins();
    QMap<int, ScheduledAction*> *actions = state->pausedActions();
    
    cancelRedirection();

    // We still have to close the previous part page.
    if (!d->m_restored){
        closeURL();
    }
            
    d->m_bComplete = false;
    
    // Don't re-emit the load event.
    d->m_bLoadEventEmitted = true;
    
    // delete old status bar msg's from kjs (if it _was_ activated on last URL)
    if( d->m_bJScriptEnabled )
    {
        d->m_kjsStatusBarText = QString::null;
        d->m_kjsDefaultStatusBarText = QString::null;
    }

    ASSERT (url);
    
    m_url = *url;
    
    // initializing m_url to the new url breaks relative links when opening such a link after this call and _before_ begin() is called (when the first
    // data arrives) (Simon)
    if(m_url.protocol().startsWith( "http" ) && !m_url.host().isEmpty() && m_url.path().isEmpty()) {
        m_url.setPath("/");
        emit d->m_extension->setLocationBarURL( m_url.prettyURL() );
    }
    
    // copy to m_workingURL after fixing m_url above
    d->m_workingURL = m_url;
        
    emit started( 0L );
    
    // -----------begin-----------
    clear();

    doc->setInPageCache(false);

    d->m_bCleared = false;
    d->m_cacheId = 0;
    d->m_bComplete = false;
    d->m_bLoadEventEmitted = false;
    d->m_referrer = m_url.url();
    
    setView(doc->view());
    
    d->m_doc = doc;
    d->m_doc->ref();
    
    Decoder *decoder = doc->decoder();
    if (decoder) {
        decoder->ref();
    }
    if (d->m_decoder) {
        d->m_decoder->deref();
    }
    d->m_decoder = decoder;

    updatePolicyBaseURL();
        
    restoreWindowProperties (windowProperties);
    restoreLocationProperties (locationProperties);
    restoreInterpreterBuiltins (*interpreterBuiltins);

    if (actions)
        resumeActions (actions, state);
    
    checkCompleted();
}

KWQKHTMLPart *KWQKHTMLPart::partForWidget(const QWidget *widget)
{
    ASSERT_ARG(widget, widget);
    
    NodeImpl *node = nodeForWidget(widget);
    if (node) {
	return partForNode(node);
    }
    
    // Assume all widgets are either form controls, or KHTMLViews.
#if KWIQ
    const KHTMLView *view = 0;
    if (QOBJECT_IS_A(widget, KHTMLView)) view = static_cast<const KHTMLView*>(widget);
#else
    const KHTMLView *view = dynamic_cast<const KHTMLView *>(widget);
#endif
    ASSERT(view);
    return KWQ(view->part());
}

WebCoreBridge *KWQKHTMLPart::bridgeForWidget(const QWidget *widget)
{
    ASSERT_ARG(widget, widget);
    
    DOM::NodeImpl *node = nodeForWidget(widget);
    if (node) {
	return partForNode(node)->bridge() ;
    }
    
    // Assume all widgets are either form controls, or KHTMLViews.
#if KWIQ
    const KHTMLView *view = 0;
    if (QOBJECT_IS_A(widget, KHTMLView)) view = static_cast<const KHTMLView*>(widget);
#else
    const KHTMLView *view = dynamic_cast<const KHTMLView *>(widget);
#endif
    ASSERT(view);
    return KWQ(view->part())->bridge();
}

KWQKHTMLPart *KWQKHTMLPart::partForNode(DOM::NodeImpl *node)
{
    ASSERT_ARG(node, node);
    return KWQ(node->getDocument()->part());
}

#if 0
NSView *KWQKHTMLPart::documentViewForNode(DOM::NodeImpl *node)
{
    WebCoreBridge *bridge = partForNode(node)->bridge();
    return [bridge documentView];
}
#endif

DOM::NodeImpl *KWQKHTMLPart::nodeForWidget(const QWidget *widget)
{
    ASSERT_ARG(widget, widget);
    const QObject *o = widget->eventFilterObject();
    return o ? static_cast<const RenderWidget *>(o)->element() : 0;
}

void KWQKHTMLPart::setDocumentFocus(QWidget *widget)
{
    DOM::NodeImpl *node = nodeForWidget(widget);
    if (node) {
        node->getDocument()->setFocusNode(node);
    } else {
        ERROR("unable to clear focus because widget had no corresponding node");
    }
}

void KWQKHTMLPart::clearDocumentFocus(QWidget *widget)
{
    DOM::NodeImpl *node = nodeForWidget(widget);
    if (node) {
    	node->getDocument()->setFocusNode(0);
    } else {
        ERROR("unable to clear focus because widget had no corresponding node");
    }
}

void KWQKHTMLPart::saveDocumentState()
{
    // Do not save doc state if the page has a password field and a form that would be submitted
    // via https
    if (!(d->m_doc && d->m_doc->hasPasswordField() && d->m_doc->hasSecureForm())) {
         _bridge->saveDocumentState();
    }
}

void KWQKHTMLPart::restoreDocumentState()
{
    _bridge->restoreDocumentState();
}

QPtrList<KWQKHTMLPart> &KWQKHTMLPart::mutableInstances()
{
    static QPtrList<KWQKHTMLPart> instancesList;
    return instancesList;
}

void KWQKHTMLPart::updatePolicyBaseURL()
{
    // FIXME: docImpl() returns null for everything other than HTML documents; is this causing problems? -dwh
    if (parentPart() && parentPart()->docImpl()) {
        setPolicyBaseURL(parentPart()->docImpl()->policyBaseURL());
    } else {
        setPolicyBaseURL(m_url.url());
    }
}

void KWQKHTMLPart::setPolicyBaseURL(const DOMString &s)
{
    // FIXME: XML documents will cause this to return null.  docImpl() is
    // an HTMLdocument only. -dwh
    if (docImpl())
        docImpl()->setPolicyBaseURL(s);
    ConstFrameIt end = d->m_frames.end();
    for (ConstFrameIt it = d->m_frames.begin(); it != end; ++it) {
        ReadOnlyPart *subpart = (*it).m_part;
        static_cast<KWQKHTMLPart *>(subpart)->setPolicyBaseURL(s);
    }
}

QString KWQKHTMLPart::requestedURLString() const
{
    return _bridge->requestedURLString();
}

QString KWQKHTMLPart::incomingReferrer() const
{
    return _bridge->incomingReferrer();
}

void KWQKHTMLPart::forceLayout()
{
    KHTMLView *v = d->m_view;
    if (v) {
        v->layout();
        // We cannot unschedule a pending relayout, since the force can be called with
        // a tiny rectangle from a drawRect update.  By unscheduling we in effect
        // "validate" and stop the necessary full repaint from occurring.  Basically any basic
        // append/remove DHTML is broken by this call.  For now, I have removed the optimization
        // until we have a better invalidation stategy. -dwh
        //v->unscheduleRelayout();
    }
}

void KWQKHTMLPart::forceLayoutWithPageWidthRange(float minPageWidth, float maxPageWidth)
{
    // Dumping externalRepresentation(_part->renderer()).ascii() is a good trick to see
    // the state of things before and after the layout
    RenderCanvas *root = static_cast<RenderCanvas *>(xmlDocImpl()->renderer());
    if (root) {
        // This magic is basically copied from khtmlview::print
        int pageW = (int)ceil(minPageWidth);
        root->setWidth(pageW);
        root->setNeedsLayoutAndMinMaxRecalc();
        forceLayout();
        
        // If we don't fit in the minimum page width, we'll lay out again. If we don't fit in the
        // maximum page width, we will lay out to the maximum page width and clip extra content.
        // FIXME: We are assuming a shrink-to-fit printing implementation.  A cropping
        // implementation should not do this!
        int rightmostPos = root->rightmostPosition();
        if (rightmostPos > minPageWidth) {
            pageW = kMin(rightmostPos, (int)ceil(maxPageWidth));
            root->setWidth(pageW);
            root->setNeedsLayoutAndMinMaxRecalc();
            forceLayout();
        }
    }
}

void KWQKHTMLPart::sendResizeEvent()
{
    KHTMLView *v = d->m_view;
    if (v) {
        // Sending an event can result in the destruction of the view and part.
        // We ref so that happens after we return from the KHTMLView function.
        v->ref();
	QResizeEvent e;
	v->resizeEvent(&e);
        v->deref();
    }
}

void KWQKHTMLPart::sendScrollEvent()
{
    KHTMLView *v = d->m_view;
    if (v) {
        DocumentImpl *doc = xmlDocImpl();
        if (!doc)
            return;
        doc->dispatchHTMLEvent(EventImpl::SCROLL_EVENT, true, false);
    }
}

void KWQKHTMLPart::runJavaScriptAlert(const QString &message)
{
    QString text = message;
    text.replace('\\', backslashAsCurrencySymbol());
    _bridge->runJavaScriptAlertPanelWithMessage(text.utf8());
}

bool KWQKHTMLPart::runJavaScriptConfirm(const QString &message)
{
     QString text = message;
     text.replace('\\', backslashAsCurrencySymbol());
     return _bridge->runJavaScriptConfirmPanelWithMessage(text.utf8());
}

bool KWQKHTMLPart::runJavaScriptPrompt(const QString &prompt, const QString &defaultValue, QString &result)
{
    QString promptText = prompt;
    promptText.replace('\\', backslashAsCurrencySymbol());
    QString defaultValueText = defaultValue;
    defaultValueText.replace('\\', backslashAsCurrencySymbol());

    gchar * returnedText = 0;     
    bool ok = _bridge->runJavaScriptTextInputPanelWithPrompt(prompt.utf8(),
							     defaultValue.utf8(),
							     &returnedText);
    if (ok) {
	result = QString::fromUtf8(returnedText);
        result.replace(backslashAsCurrencySymbol(), '\\');
    }
    g_free(returnedText);

    return ok;
}

void KWQKHTMLPart::addMessageToConsole(const QString &message, unsigned lineNumber, const QString &sourceURL)
{
    _bridge->addMessageToConsole(message.utf8(), lineNumber, sourceURL.utf8());
}

void KWQKHTMLPart::createEmptyDocument()
{
    // Although it's not completely clear from the name of this function,
    // it does nothing if we already have a document, and just creates an
    // empty one if we have no document at all.
    if (!d->m_doc) {
        _bridge->loadEmptyDocumentSynchronously();

	if (parentPart() && (parentPart()->childFrame(this)->m_type == ChildFrame::IFrame ||
			     parentPart()->childFrame(this)->m_type == ChildFrame::Object)) {
	    d->m_doc->setBaseURL(parentPart()->d->m_doc->baseURL());
	}
    }
}

void KWQKHTMLPart::addMetaData(const QString &key, const QString &value)
{
    d->m_job->addMetaData(key, value);
}

bool KWQKHTMLPart::keyEvent(QKeyEvent *event)
{
    ASSERT(event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease);

    // Check for cases where we are too early for events -- possible unmatched key up
    // from pressing return in the location bar.
    DocumentImpl *doc = xmlDocImpl();
    if (!doc) {
        return false;
    }
    DOM::NodeImpl *node = doc->focusNode();
    if (!node && docImpl()) {
	node = docImpl()->body();
    }
    if (!node) {
        return false;
    }
    
    NSEvent *oldCurrentEvent = _currentEvent;
    _currentEvent = (NSEvent*) event;

    QKeyEvent qEvent(*event);
    bool result = !node->dispatchKeyEvent(&qEvent);

    // We want to send both a down and a press for the initial key event.
    // To get KHTML to do this, we send a second KeyPress QKeyEvent with "is repeat" set to true,
    // which causes it to send a press to the DOM.
    // That's not a great hack; it would be good to do this in a better way.
    if (event->type() == QEvent::KeyPress && !event->isAutoRepeat()) {
	QKeyEvent repeatEvent(*event);
        if (!node->dispatchKeyEvent(&repeatEvent)) {
	    result = true;
	}
    }

    ASSERT(_currentEvent == (NSEvent*)event);
    _currentEvent = oldCurrentEvent;

    return result;
}

// This does the same kind of work that KHTMLPart::openURL does, except it relies on the fact
// that a higher level already checked that the URLs match and the scrolling is the right thing to do.
void KWQKHTMLPart::scrollToAnchor(const KURL &URL)
{
    cancelRedirection();

    m_url = URL;
    started(0);

    if (!gotoAnchor(URL.encodedHtmlRef()))
        gotoAnchor(URL.ref());

    // It's important to model this as a load that starts and immediately finishes.
    // Otherwise, the parent frame may think we never finished loading.
    d->m_bComplete = false;
    checkCompleted();
}

bool KWQKHTMLPart::closeURL()
{
    saveDocumentState();
    return KHTMLPart::closeURL();
}

void KWQKHTMLPart::khtmlMousePressEvent(MousePressEvent *event)
{
    // If we got the event back, that must mean it wasn't prevented,
    // so it's allowed to start a drag or selection.
    _mouseDownMayStartDrag = true;
    _mouseDownMayStartSelect = true;

    if (!passWidgetMouseDownEventToWidget(event)) {
        // We don't do this at the start of mouse down handling (before calling into WebCore),
        // because we don't want to do it until we know we didn't hit a widget.
        GtkWidget *view = d->m_view->getDocumentView();

        if (_currentEvent->clickCount() <= 1 && _bridge->firstResponder() != view) {
            _bridge->makeFirstResponder(view);
        }

        KHTMLPart::khtmlMousePressEvent(event);
    }
}

void KWQKHTMLPart::khtmlMouseDoubleClickEvent(MouseDoubleClickEvent *event)
{
    if (!passWidgetMouseDownEventToWidget(event)) {
        KHTMLPart::khtmlMouseDoubleClickEvent(event);
    }
}

bool KWQKHTMLPart::passWidgetMouseDownEventToWidget(khtml::MouseEvent *event)
{
    // Figure out which view to send the event to.
    RenderObject *target = event->innerNode().handle() ? event->innerNode().handle()->renderer() : 0;
    if (!target)
        return false;

    QWidget* widget = RenderLayer::gScrollBar;
    if (!widget) {
        if (!target->isWidget())
            return false;
        widget = static_cast<RenderWidget *>(target)->widget();
    }

    // Doubleclick events don't exist in Cocoa.  Since passWidgetMouseDownEventToWidget will
    // just pass _currentEvent down to the widget,  we don't want to call it for events that
    // don't correspond to Cocoa events.  The mousedown/ups will have already been passed on as
    // part of the pressed/released handling.
    if (!MouseDoubleClickEvent::test(event))
        return passWidgetMouseDownEventToWidget(widget);
    else
        return true;
}

bool KWQKHTMLPart::passWidgetMouseDownEventToWidget(RenderWidget *renderWidget)
{
    return passWidgetMouseDownEventToWidget(renderWidget->widget());
}

bool KWQKHTMLPart::passWidgetMouseDownEventToWidget(QWidget* widget)
{
    // FIXME: this method always returns true

    if (!widget) {
        ERROR("hit a RenderWidget without a corresponding QWidget, means a frame is half-constructed");
        return true;
    }
#if 0
    KWQ_BLOCK_EXCEPTIONS;
    
    NSView *nodeView = widget->getView();
    ASSERT(nodeView);
    ASSERT([nodeView superview]);
    NSView *topView = nodeView;
    NSView *superview;
    while ((superview = [topView superview])) {
        topView = superview;
    }
    NSView *view = [nodeView hitTest:[[nodeView superview] convertPoint:[_currentEvent locationInWindow] fromView:topView]];
    if (view == nil) {
        ERROR("KHTML says we hit a RenderWidget, but AppKit doesn't agree we hit the corresponding NSView");
        return true;
    }
    
    if ([_bridge firstResponder] == view) {
        // In the case where we just became first responder, we should send the mouseDown:
        // to the NSTextField, not the NSTextField's editor. This code makes sure that happens.
        // If we don't do this, we see a flash of selected text when clicking in a text field.
        if (_firstResponderAtMouseDownTime != view && [view isKindOfClass:[NSTextView class]]) {
            NSView *superview = view;
            while (superview != nodeView) {
                superview = [superview superview];
                ASSERT(superview);
                if ([superview isKindOfClass:[NSControl class]]) {
                    NSControl *control = superview;
                    if ([control currentEditor] == view) {
                        view = superview;
                    }
                    break;
                }
            }
        }
    } else {
        // Normally [NSWindow sendEvent:] handles setting the first responder.
        // But in our case, the event was sent to the view representing the entire web page.
        if ([_currentEvent clickCount] <= 1 && [view acceptsFirstResponder] && [view needsPanelToBecomeKey]) {
            [_bridge makeFirstResponder:view];
        }
    }

    // We need to "defer loading" and defer timers while we are tracking the mouse.
    // That's because we don't want the new page to load while the user is holding the mouse down.
    
    BOOL wasDeferringLoading = [_bridge defersLoading];
    if (!wasDeferringLoading) {
        [_bridge setDefersLoading:YES];
    }
    BOOL wasDeferringTimers = QObject::defersTimers();
    if (!wasDeferringTimers) {
        QObject::setDefersTimers(true);
    }

    ASSERT(!_sendingEventToSubview);
    _sendingEventToSubview = true;
    [view mouseDown:_currentEvent];
    _sendingEventToSubview = false;
    
    if (!wasDeferringTimers) {
        QObject::setDefersTimers(false);
    }
    if (!wasDeferringLoading) {
        [_bridge setDefersLoading:NO];
    }

    // Remember which view we sent the event to, so we can direct the release event properly.
    _mouseDownView = view;
    _mouseDownWasInSubframe = false;

    KWQ_UNBLOCK_EXCEPTIONS;
#endif
    return true;
}

bool KWQKHTMLPart::lastEventIsMouseUp()
{
#if 0    
    // Many AK widgets run their own event loops and consume events while the mouse is down.
    // When they finish, currentEvent is the mouseUp that they exited on.  We need to update
    // the khtml state with this mouseUp, which khtml never saw.  This method lets us detect
    // that state.

    KWQ_BLOCK_EXCEPTIONS;
    NSEvent *currentEventAfterHandlingMouseDown = [NSApp currentEvent];
    if (_currentEvent != currentEventAfterHandlingMouseDown) {
        if ([currentEventAfterHandlingMouseDown type] == NSLeftMouseUp) {
            return true;
        }
    }
    KWQ_UNBLOCK_EXCEPTIONS;
#endif
    return false;
}
    
// Note that this does the same kind of check as [target isDescendantOf:superview].
// There are two differences: This is a lot slower because it has to walk the whole
// tree, and this works in cases where the target has already been deallocated.
#if 0    
static bool findViewInSubviews(NSView *superview, NSView *target)
{
    KWQ_BLOCK_EXCEPTIONS;
    NSEnumerator *e = [[superview subviews] objectEnumerator];
    NSView *subview;
    while ((subview = [e nextObject])) {
        if (subview == target || findViewInSubviews(subview, target)) {
            return true;
	}
    }
    KWQ_UNBLOCK_EXCEPTIONS;
    return false;
}
#endif    

NSView *KWQKHTMLPart::mouseDownViewIfStillGood()
{
#if 0    
    // Since we have no way of tracking the lifetime of _mouseDownView, we have to assume that
    // it could be deallocated already. We search for it in our subview tree; if we don't find
    // it, we set it to nil.
    NSView *mouseDownView = _mouseDownView;
    if (!mouseDownView) {
        return nil;
    }
    KHTMLView *topKHTMLView = d->m_view;
    NSView *topView = topKHTMLView ? topKHTMLView->getView() : nil;
    if (!topView || !findViewInSubviews(topView, mouseDownView)) {
        _mouseDownView = nil;
        return nil;
    }
    return mouseDownView;
#endif
    return 0;
}

#if 0
// The link drag hysteresis is much larger than the others because there
// needs to be enough space to cancel the link press without starting a link drag,
// and because dragging links is rare.
#define LinkDragHysteresis              40.0
#define ImageDragHysteresis              5.0
#define TextDragHysteresis               3.0
#define GeneralDragHysterisis            3.0

bool KWQKHTMLPart::dragHysteresisExceeded(float dragLocationX, float dragLocationY) const
{
    int dragX, dragY;
    d->m_view->viewportToContents((int)dragLocationX, (int)dragLocationY, dragX, dragY);
    float deltaX = QABS(dragX - _mouseDownX);
    float deltaY = QABS(dragY - _mouseDownY);

    float threshold = GeneralDragHysterisis;
    if (_dragSrcIsImage) {
        threshold = ImageDragHysteresis;
    } else if (_dragSrcIsLink) {
        threshold = LinkDragHysteresis;
    } else if (_dragSrcInSelection) {
        threshold = TextDragHysteresis;
    }
    return deltaX >= threshold || deltaY >= threshold;
}

// returns if we should continue "default processing", i.e., whether eventhandler canceled
bool KWQKHTMLPart::dispatchDragSrcEvent(int eventId, const QPoint &loc) const
{
    bool noDefaultProc = d->m_view->dispatchDragEvent(eventId, _dragSrc.handle(), loc, _dragClipboard);
    return !noDefaultProc;
}
#endif

void KWQKHTMLPart::khtmlMouseMoveEvent(MouseMoveEvent *event)
{
#if 0    
    // See WebCore-146.1/kwq/KWQKHTMLPart.mm
#endif
    KHTMLPart::khtmlMouseMoveEvent(event);
}

#if 0
void KWQKHTMLPart::dragSourceMovedTo(const QPoint &loc)
{
    if (!_dragSrc.isNull() && _dragSrcMayBeDHTML) {
        // for now we don't care if event handler cancels default behavior, since there is none
        dispatchDragSrcEvent(EventImpl::DRAG_EVENT, loc);
    }
}

void KWQKHTMLPart::dragSourceEndedAt(const QPoint &loc)
{
    if (!_dragSrc.isNull() && _dragSrcMayBeDHTML) {
        // for now we don't care if event handler cancels default behavior, since there is none
        dispatchDragSrcEvent(EventImpl::DRAGEND_EVENT, loc);
    }
    freeClipboard();
    _dragSrc = 0;
}

bool KWQKHTMLPart::dispatchCPPEvent(int eventId, KWQClipboard::AccessPolicy policy)
{
    NodeImpl *target = d->m_selection.start().element();
    if (!target) {
        target = docImpl()->body();
    }

    KWQClipboard *clipboard = new KWQClipboard(false, [NSPasteboard generalPasteboard], (KWQClipboard::AccessPolicy)policy);
    clipboard->ref();

    int exceptioncode = 0;
    EventImpl *evt = new ClipboardEventImpl(static_cast<EventImpl::EventId>(eventId), true, true, clipboard);
    evt->ref();
    target->dispatchEvent(evt, exceptioncode, true);
    bool noDefaultProcessing = evt->defaultPrevented();
    evt->deref();

    // invalidate clipboard here for security
    clipboard->setAccessPolicy(KWQClipboard::Numb);
    clipboard->deref();

    return !noDefaultProcessing;
}

// In the next three functions, for now, we send the "before" event to gain some compatibility with WinIE.
// WinIE uses onbeforecut and onbeforepaste to enables the cut and paste menu items, but we don't yet.  They
// also send onbeforecopy, apparently for symmetry, but it doesn't affect the menu items.  The return of these
// routines should not block the sending of non-"before" event.

bool KWQKHTMLPart::tryCut()
{
    // Must be done before oncut adds types and data to the pboard,
    // also done for security, as it erases data from the last copy/paste.
    [[NSPasteboard generalPasteboard] declareTypes:[NSArray array] owner:nil];

    dispatchCPPEvent(EventImpl::BEFORECUT_EVENT, KWQClipboard::Writable);
    return !dispatchCPPEvent(EventImpl::CUT_EVENT, KWQClipboard::Writable);
}

bool KWQKHTMLPart::tryCopy()
{
    // Must be done before oncopy adds types and data to the pboard,
    // also done for security, as it erases data from the last copy/paste.
    [[NSPasteboard generalPasteboard] declareTypes:[NSArray array] owner:nil];

    dispatchCPPEvent(EventImpl::BEFORECOPY_EVENT, KWQClipboard::Writable);
    return !dispatchCPPEvent(EventImpl::COPY_EVENT, KWQClipboard::Writable);
}

bool KWQKHTMLPart::tryPaste()
{
    dispatchCPPEvent(EventImpl::BEFOREPASTE_EVENT, KWQClipboard::Readable);
    return !dispatchCPPEvent(EventImpl::PASTE_EVENT, KWQClipboard::Readable);
}
#endif

void KWQKHTMLPart::khtmlMouseReleaseEvent(MouseReleaseEvent *event)
{
#if 0
    NSView *view = mouseDownViewIfStillGood();
    if (!view) {
        KHTMLPart::khtmlMouseReleaseEvent(event);
        return;
    }
    
    _sendingEventToSubview = true;
    KWQ_BLOCK_EXCEPTIONS;
    [view mouseUp:_currentEvent];
    KWQ_UNBLOCK_EXCEPTIONS;
    _sendingEventToSubview = false;
#endif
    KHTMLPart::khtmlMouseReleaseEvent(event);    
}

void KWQKHTMLPart::clearTimers(KHTMLView *view)
{
    if (view) {
        view->unscheduleRelayout();
        if (view->part()) {
            DocumentImpl* document = view->part()->xmlDocImpl();
            if (document && document->renderer() && document->renderer()->layer())
                document->renderer()->layer()->suspendMarquees();
        }
    }
}

void KWQKHTMLPart::clearTimers()
{
    clearTimers(d->m_view);
}

bool KWQKHTMLPart::passSubframeEventToSubframe(DOM::NodeImpl::MouseEvent &event)
{
#if 0
    KWQ_BLOCK_EXCEPTIONS;

    switch ([_currentEvent type]) {
    	case NSLeftMouseDown: {
            NodeImpl *node = event.innerNode.handle();
            if (!node) {
                return false;
            }
            RenderPart *renderPart = dynamic_cast<RenderPart *>(node->renderer());
            if (!renderPart) {
                return false;
            }
            if (!passWidgetMouseDownEventToWidget(renderPart)) {
                return false;
            }
            _mouseDownWasInSubframe = true;
            return true;
        }
        case NSLeftMouseUp: {
            if (!_mouseDownWasInSubframe) {
                return false;
            }
            NSView *view = mouseDownViewIfStillGood();
            if (!view) {
                return false;
            }
            ASSERT(!_sendingEventToSubview);
            _sendingEventToSubview = true;
	    [view mouseUp:_currentEvent];
	    _sendingEventToSubview = false;
            return true;
        }
        case NSLeftMouseDragged: {
            if (!_mouseDownWasInSubframe) {
                return false;
            }
            NSView *view = mouseDownViewIfStillGood();
            if (!view) {
                return false;
            }
            ASSERT(!_sendingEventToSubview);
            _sendingEventToSubview = true;
	    [view mouseDragged:_currentEvent];
	    _sendingEventToSubview = false;
            return true;
        }
        default:
            return false;
    }
    KWQ_UNBLOCK_EXCEPTIONS;
#endif
    /* too much log events */
    //LOG(NotYetImplemented,"KWIQ");
    return false;
}

void KWQKHTMLPart::mouseDown(QMouseEvent *event)
{
    KHTMLView *v = d->m_view;
    if (!v || _sendingEventToSubview) {
        return;
    }

    _mouseDownView = 0;

    NSEvent *oldCurrentEvent = _currentEvent;
    _currentEvent = (NSEvent*) event;

    //NSResponder *oldFirstResponderAtMouseDownTime = _firstResponderAtMouseDownTime;
    // Unlike other places in WebCore where we get the first
    // responder, in this case we must be talking about the real first
    // responder, so we could just ask the bridge's window, instead of
    // the bridge. It's unclear which is better.
    //_firstResponderAtMouseDownTime = [[_bridge firstResponder] retain];

    _mouseDownMayStartDrag = false;
    _mouseDownMayStartSelect = false;

    // Sending an event can result in the destruction of the view and part.
    // We ref so that happens after we return from the KHTMLView function.
    v->ref();
    QMouseEvent kEvent(*event);
    v->viewportMousePressEvent(&kEvent);
    v->deref();
    
    //[_firstResponderAtMouseDownTime release];
    //_firstResponderAtMouseDownTime = oldFirstResponderAtMouseDownTime;

    ASSERT(_currentEvent == (NSEvent*) event);
    _currentEvent = oldCurrentEvent;
}

void KWQKHTMLPart::mouseDragged(QMouseEvent *event)
{
    KHTMLView *v = d->m_view;
    if (!v || _sendingEventToSubview) {
        return;
    }

    NSEvent *oldCurrentEvent = _currentEvent;
    _currentEvent = (NSEvent*) event;

    // Sending an event can result in the destruction of the view and part.
    // We ref so that happens after we return from the KHTMLView function.
    v->ref();
    QMouseEvent kEvent(*event);
    v->viewportMouseMoveEvent(&kEvent);
    v->deref();
    
    ASSERT(_currentEvent == (NSEvent*) event);
    _currentEvent = oldCurrentEvent;
}


void KWQKHTMLPart::mouseUp(QMouseEvent *event)
{
    KHTMLView *v = d->m_view;
    if (!v || _sendingEventToSubview) {
        return;
    }

    NSEvent *oldCurrentEvent = _currentEvent;
    _currentEvent = (NSEvent*) event;

    // Sending an event can result in the destruction of the view and part.
    // We ref so that happens after we return from the KHTMLView function.
    v->ref();
    // Our behavior here is a little different that Qt. Qt always sends
    // a mouse release event, even for a double click. To correct problems
    // in khtml's DOM click event handling we do not send a release here
    // for a double click. Instead we send that event from KHTMLView's
    // viewportMouseDoubleClickEvent. Note also that the third click of
    // a triple click is treated as a single click, but the fourth is then
    // treated as another double click. Hence the "% 2" below.
    int clickCount = event->clickCount();
    if (clickCount > 0 && clickCount % 2 == 0) {
        QMouseEvent doubleClickEvent(*event);
        v->viewportMouseDoubleClickEvent(&doubleClickEvent);
    } else {
        QMouseEvent releaseEvent(*event);
        v->viewportMouseReleaseEvent(&releaseEvent);
    }
    v->deref();
    
    ASSERT(_currentEvent == (NSEvent*) event);
    _currentEvent = oldCurrentEvent;
    _mouseDownView = 0;
}

#if 0
/*
 A hack for the benefit of AK's PopUpButton, which uses the Carbon menu manager, which thus
 eats all subsequent events after it is starts its modal tracking loop.  After the interaction
 is done, this routine is used to fix things up.  When a mouse down started us tracking in
 the widget, we post a fake mouse up to balance the mouse down we started with. When a 
 key down started us tracking in the widget, we post a fake key up to balance things out.
 In addition, we post a fake mouseMoved to get the cursor in sync with whatever we happen to 
 be over after the tracking is done.
 */
void KWQKHTMLPart::sendFakeEventsAfterWidgetTracking(NSEvent *initiatingEvent)
{
    KWQ_BLOCK_EXCEPTIONS;

    _sendingEventToSubview = false;
    int eventType = [initiatingEvent type];
    ASSERT(eventType == NSLeftMouseDown || eventType == NSKeyDown);
    NSEvent *fakeEvent = nil;
    if (eventType == NSLeftMouseDown) {
        fakeEvent = [NSEvent mouseEventWithType:NSLeftMouseUp
                                location:[initiatingEvent locationInWindow]
                            modifierFlags:[initiatingEvent modifierFlags]
                                timestamp:[initiatingEvent timestamp]
                            windowNumber:[initiatingEvent windowNumber]
                                    context:[initiatingEvent context]
                                eventNumber:[initiatingEvent eventNumber]
                                clickCount:[initiatingEvent clickCount]
                                pressure:[initiatingEvent pressure]];
    
        mouseUp(fakeEvent);
    }
    else { // eventType == NSKeyDown
        fakeEvent = [NSEvent keyEventWithType:NSKeyUp
                                location:[initiatingEvent locationInWindow]
                           modifierFlags:[initiatingEvent modifierFlags]
                               timestamp:[initiatingEvent timestamp]
                            windowNumber:[initiatingEvent windowNumber]
                                 context:[initiatingEvent context]
                              characters:[initiatingEvent characters] 
             charactersIgnoringModifiers:[initiatingEvent charactersIgnoringModifiers] 
                               isARepeat:[initiatingEvent isARepeat] 
                                 keyCode:[initiatingEvent keyCode]];
        keyEvent(fakeEvent);
    }
    // FIXME:  We should really get the current modifierFlags here, but there's no way to poll
    // them in Cocoa, and because the event stream was stolen by the Carbon menu code we have
    // no up-to-date cache of them anywhere.
    fakeEvent = [NSEvent mouseEventWithType:NSMouseMoved
                                   location:[[_bridge window] convertScreenToBase:[NSEvent mouseLocation]]
                              modifierFlags:[initiatingEvent modifierFlags]
                                  timestamp:[initiatingEvent timestamp]
                               windowNumber:[initiatingEvent windowNumber]
                                    context:[initiatingEvent context]
                                eventNumber:0
                                 clickCount:0
                                   pressure:0];
    mouseMoved(fakeEvent);

    KWQ_UNBLOCK_EXCEPTIONS;
}
#endif

void KWQKHTMLPart::mouseMoved(QMouseEvent *event)
{
    KHTMLView *v = d->m_view;
    // Reject a mouse moved if the button is down - screws up tracking during autoscroll
    // These happen because WebKit sometimes has to fake up moved events.
    if (!v /* || d->m_bMousePressed*/ ) { //KWIQ:
        return;
    }
    
    NSEvent *oldCurrentEvent = _currentEvent;
    _currentEvent = (NSEvent*) event;
    
    // Sending an event can result in the destruction of the view and part.
    // We ref so that happens after we return from the KHTMLView function.
    v->ref();
    QMouseEvent kEvent(*event);
    v->viewportMouseMoveEvent(&kEvent);
    v->deref();
    
    ASSERT(_currentEvent == (NSEvent*) event);
    _currentEvent = oldCurrentEvent;
}

// Called as we walk up the element chain for nodes with CSS property -khtml-user-drag == auto
bool KWQKHTMLPart::shouldDragAutoNode(DOM::NodeImpl* node) const
{
    // We assume that WebKit only cares about dragging things that can be leaf nodes (text, images, urls).
    // This saves a bunch of expensive calls (creating WC and WK element dicts) as we walk farther up
    // the node hierarchy, and we also don't have to cook up a way to ask WK about non-leaf nodes
    // (since right now WK just hit-tests using a cached lastMouseDown).
    if (!node->hasChildNodes()) {
        return _bridge->mayStartDragWithMouseDragged(_currentEvent);
    } else {
        return false;
    }
}

#if 0
bool KWQKHTMLPart::sendContextMenuEvent(NSEvent *event)
{
    DocumentImpl *doc = d->m_doc;
    KHTMLView *v = d->m_view;
    if (!doc || !v) {
        return false;
    }

    KWQ_BLOCK_EXCEPTIONS;

    NSEvent *oldCurrentEvent = _currentEvent;
    _currentEvent = [event retain];
    
    QMouseEvent qev(QEvent::MouseButtonPress, event);

    int xm, ym;
    v->viewportToContents(qev.x(), qev.y(), xm, ym);

    NodeImpl::MouseEvent mev(qev.stateAfter(), NodeImpl::MousePress);
    doc->prepareMouseEvent(false, xm, ym, &mev);

    // Sending an event can result in the destruction of the view and part.
    // We ref so that happens after we return from the KHTMLView function.
    v->ref();
    bool swallowEvent = v->dispatchMouseEvent(EventImpl::CONTEXTMENU_EVENT,
        mev.innerNode.handle(), true, 0, &qev, true, NodeImpl::MousePress);
    v->deref();

    ASSERT(_currentEvent == event);
    [event release];
    _currentEvent = oldCurrentEvent;

    return swallowEvent;

    KWQ_UNBLOCK_EXCEPTIONS;

    return false;
}

struct ListItemInfo {
    unsigned start;
    unsigned end;
};

NSFileWrapper *KWQKHTMLPart::fileWrapperForElement(ElementImpl *e)
{
    KWQ_BLOCK_EXCEPTIONS;
    
    NSFileWrapper *wrapper = nil;

    DOMString attr = e->getAttribute(ATTR_SRC);
    if (!attr.isEmpty()) {
        NSURL *URL = completeURL(attr.string()).getNSURL();
        wrapper = [_bridge fileWrapperForURL:URL];
    }    
    if (!wrapper) {
        RenderImage *renderer = static_cast<RenderImage *>(e->renderer());
        NSImage *image = renderer->pixmap().image();
        NSData *tiffData = [image TIFFRepresentationUsingCompression:NSTIFFCompressionLZW factor:0.0];
        wrapper = [[NSFileWrapper alloc] initRegularFileWithContents:tiffData];
        [wrapper setPreferredFilename:@"image.tiff"];
        [wrapper autorelease];
    }

    return wrapper;
    
    KWQ_UNBLOCK_EXCEPTIONS;

    return nil;
}

static ElementImpl *listParent(ElementImpl *item)
{
    // Ick!  Avoid use of item->id() which confuses ObjC++.
    unsigned short _id = Node(item).elementId();
    
    while (_id != ID_UL && _id != ID_OL) {
        item = static_cast<ElementImpl *>(item->parentNode());
        if (!item)
            break;
        _id = Node(item).elementId();
    }
    return item;
}

static NodeImpl* isTextFirstInListItem(NodeImpl *e)
{
    if (Node(e).nodeType() != Node::TEXT_NODE)
        return 0;
    NodeImpl* par = e->parentNode();
    while (par) {
        if (par->firstChild() != e)
            return 0;
        if (Node(par).elementId() == ID_LI)
            return par;
        e = par;
        par = par->parentNode();
    }
    return 0;
}

// FIXME: This collosal function needs to be refactored into maintainable smaller bits.

#define BULLET_CHAR 0x2022
#define SQUARE_CHAR 0x25AA
#define CIRCLE_CHAR 0x25E6

NSAttributedString *KWQKHTMLPart::attributedString(NodeImpl *_start, int startOffset, NodeImpl *endNode, int endOffset)
{
    KWQ_BLOCK_EXCEPTIONS;

    NodeImpl * _startNode = _start;

    if (_startNode == nil) {
        return nil;
    }

    NSMutableAttributedString *result = [[[NSMutableAttributedString alloc] init] autorelease];

    bool hasNewLine = true;
    bool addedSpace = true;
    NSAttributedString *pendingStyledSpace = nil;
    bool hasParagraphBreak = true;
    const ElementImpl *linkStartNode = 0;
    unsigned linkStartLocation = 0;
    QPtrList<ElementImpl> listItems;
    QValueList<ListItemInfo> listItemLocations;
    float maxMarkerWidth = 0;
    
    Node n = _startNode;
    
    // If the first item is the entire text of a list item, use the list item node as the start of the 
    // selection, not the text node.  The user's intent was probably to select the list.
    if (n.nodeType() == Node::TEXT_NODE && startOffset == 0) {
        NodeImpl *startListNode = isTextFirstInListItem(_startNode);
        if (startListNode){
            _startNode = startListNode;
            n = _startNode;
        }
    }
    
    while (!n.isNull()) {
        RenderObject *renderer = n.handle()->renderer();
        if (renderer) {
            RenderStyle *style = renderer->style();
            NSFont *font = style->font().getNSFont();
            bool needSpace = pendingStyledSpace != nil;
            if (n.nodeType() == Node::TEXT_NODE) {
                if (hasNewLine) {
                    addedSpace = true;
                    hasNewLine = false;
                }
                QString text;
                QString str = n.nodeValue().string();
                int start = (n == _startNode) ? startOffset : -1;
                int end = (n == endNode) ? endOffset : -1;
                if (renderer->isText()) {
                    if (renderer->style()->whiteSpace() == PRE) {
                        if (needSpace && !addedSpace) {
                            if (text.isEmpty() && linkStartLocation == [result length]) {
                                ++linkStartLocation;
                            }
                            [result appendAttributedString:pendingStyledSpace];
                        }
                        int runStart = (start == -1) ? 0 : start;
                        int runEnd = (end == -1) ? str.length() : end;
                        text += str.mid(runStart, runEnd-runStart);
                        [pendingStyledSpace release];
                        pendingStyledSpace = nil;
                        addedSpace = str[runEnd-1].direction() == QChar::DirWS;
                    }
                    else {
                        RenderText* textObj = static_cast<RenderText*>(renderer);
                        InlineTextBoxArray runs = textObj->inlineTextBoxes();
                        if (runs.count() == 0 && str.length() > 0 && !addedSpace) {
                            // We have no runs, but we do have a length.  This means we must be
                            // whitespace that collapsed away at the end of a line.
                            text += " ";
                            addedSpace = true;
                        }
                        else {
                            addedSpace = false;
                            for (unsigned i = 0; i < runs.count(); i++) {
                                int runStart = (start == -1) ? runs[i]->m_start : start;
                                int runEnd = (end == -1) ? runs[i]->m_start + runs[i]->m_len : end;
                                runEnd = QMIN(runEnd, runs[i]->m_start + runs[i]->m_len);
                                if (runStart >= runs[i]->m_start &&
                                    runStart < runs[i]->m_start + runs[i]->m_len) {
                                    if (i == 0 && runs[0]->m_start == runStart && runStart > 0) {
                                        needSpace = true; // collapsed space at the start
                                    }
                                    if (needSpace && !addedSpace) {
                                        if (pendingStyledSpace != nil) {
                                            if (text.isEmpty() && linkStartLocation == [result length]) {
                                                ++linkStartLocation;
                                            }
                                            [result appendAttributedString:pendingStyledSpace];
                                        } else {
                                            text += ' ';
                                        }
                                    }
                                    QString runText = str.mid(runStart, runEnd - runStart);
                                    runText.replace('\n', ' ');
                                    text += runText;
                                    int nextRunStart = (i+1 < runs.count()) ? runs[i+1]->m_start : str.length(); // collapsed space between runs or at the end
                                    needSpace = nextRunStart > runEnd;
                                    [pendingStyledSpace release];
                                    pendingStyledSpace = nil;
                                    addedSpace = str[runEnd-1].direction() == QChar::DirWS;
                                    start = -1;
                                }
                                if (end != -1 && runEnd >= end)
                                    break;
                            }
                        }
                    }
                }
                
                text.replace('\\', renderer->backslashAsCurrencySymbol());
    
                if (text.length() > 0 || needSpace) {
                    NSMutableDictionary *attrs = [[NSMutableDictionary alloc] init];
                    [attrs setObject:font forKey:NSFontAttributeName];
                    if (style && style->color().isValid())
                        [attrs setObject:style->color().getNSColor() forKey:NSForegroundColorAttributeName];
                    if (style && style->backgroundColor().isValid())
                        [attrs setObject:style->backgroundColor().getNSColor() forKey:NSBackgroundColorAttributeName];

                    if (text.length() > 0) {
                        hasParagraphBreak = false;
                        NSAttributedString *partialString = [[NSAttributedString alloc] initWithString:text.getNSString() attributes:attrs];
                        [result appendAttributedString: partialString];                
                        [partialString release];
                    }

                    if (needSpace) {
                        [pendingStyledSpace release];
                        pendingStyledSpace = [[NSAttributedString alloc] initWithString:@" " attributes:attrs];
                    }

                    [attrs release];
                }
            } else {
                // This is our simple HTML -> ASCII transformation:
                QString text;
                unsigned short _id = n.elementId();
                switch(_id) {
                    case ID_A:
                        // Note the start of the <a> element.  We will add the NSLinkAttributeName
                        // attribute to the attributed string when navigating to the next sibling 
                        // of this node.
                        linkStartLocation = [result length];
                        linkStartNode = static_cast<ElementImpl*>(n.handle());
                        break;

                    case ID_BR:
                        text += "\n";
                        hasNewLine = true;
                        break;
    
                    case ID_LI:
                        {
                            QString listText;
                            ElementImpl *itemParent = listParent(static_cast<ElementImpl *>(n.handle()));
                            
                            if (!hasNewLine)
                                listText += '\n';
                            hasNewLine = true;
    
                            listItems.append(static_cast<ElementImpl*>(n.handle()));
                            ListItemInfo info;
                            info.start = [result length];
                            info.end = 0;
                            listItemLocations.append (info);
                            
                            listText += '\t';
                            if (itemParent){
                                khtml::RenderListItem *listRenderer = static_cast<khtml::RenderListItem*>(renderer);

                                maxMarkerWidth = MAX([font pointSize], maxMarkerWidth);
                                switch(listRenderer->style()->listStyleType()) {
                                    case khtml::DISC:
                                        listText += ((QChar)BULLET_CHAR);
                                        break;
                                    case khtml::CIRCLE:
                                        listText += ((QChar)CIRCLE_CHAR);
                                        break;
                                    case khtml::SQUARE:
                                        listText += ((QChar)SQUARE_CHAR);
                                        break;
                                    case khtml::LNONE:
                                        break;
                                    default:
                                        QString marker = listRenderer->markerStringValue();
                                        listText += marker;
                                        // Use AppKit metrics.  Will be rendered by AppKit.
                                        float markerWidth = [font widthOfString: marker.getNSString()];
                                        maxMarkerWidth = MAX(markerWidth, maxMarkerWidth);
                                }

                                listText += ' ';
                                listText += '\t';
    
                                NSMutableDictionary *attrs;
            
                                attrs = [[NSMutableDictionary alloc] init];
                                [attrs setObject:font forKey:NSFontAttributeName];
                                if (style && style->color().isValid())
                                    [attrs setObject:style->color().getNSColor() forKey:NSForegroundColorAttributeName];
                                if (style && style->backgroundColor().isValid())
                                    [attrs setObject:style->backgroundColor().getNSColor() forKey:NSBackgroundColorAttributeName];
            
                                NSAttributedString *partialString = [[NSAttributedString alloc] initWithString:listText.getNSString() attributes:attrs];
                                [attrs release];
                                [result appendAttributedString: partialString];                
                                [partialString release];
                            }
                        }
                        break;

                    case ID_OL:
                    case ID_UL:
                        if (!hasNewLine)
                            text += "\n";
                        hasNewLine = true;
                        break;

                    case ID_TD:
                    case ID_TH:
                    case ID_HR:
                    case ID_DD:
                    case ID_DL:
                    case ID_DT:
                    case ID_PRE:
                    case ID_BLOCKQUOTE:
                    case ID_DIV:
                        if (!hasNewLine)
                            text += '\n';
                        hasNewLine = true;
                        break;
                    case ID_P:
                    case ID_TR:
                    case ID_H1:
                    case ID_H2:
                    case ID_H3:
                    case ID_H4:
                    case ID_H5:
                    case ID_H6:
                        if (!hasNewLine)
                            text += '\n';
                        if (!hasParagraphBreak) {
                            text += '\n';
                            hasParagraphBreak = true;
                        }
                        hasNewLine = true;
                        break;
                        
                    case ID_IMG:
                        if (pendingStyledSpace != nil) {
                            if (linkStartLocation == [result length]) {
                                ++linkStartLocation;
                            }
                            [result appendAttributedString:pendingStyledSpace];
                            [pendingStyledSpace release];
                            pendingStyledSpace = nil;
                        }
                        NSFileWrapper *fileWrapper = fileWrapperForElement(static_cast<ElementImpl *>(n.handle()));
                        NSTextAttachment *attachment = [[NSTextAttachment alloc] initWithFileWrapper:fileWrapper];
                        NSAttributedString *iString = [NSAttributedString attributedStringWithAttachment:attachment];
                        [result appendAttributedString: iString];
                        [attachment release];
                        break;
                }
                NSAttributedString *partialString = [[NSAttributedString alloc] initWithString:text.getNSString()];
                [result appendAttributedString: partialString];
                [partialString release];
            }
        }

        if (n == endNode)
            break;

        Node next = n.firstChild();
        if (next.isNull()){
            next = n.nextSibling();
        }

        while (next.isNull() && !n.parentNode().isNull()) {
            QString text;
            n = n.parentNode();
            if (n == endNode)
                break;
            next = n.nextSibling();

            unsigned short _id = n.elementId();
            switch(_id) {
                case ID_A:
                    // End of a <a> element.  Create an attributed string NSLinkAttributeName attribute
                    // for the range of the link.  Note that we create the attributed string from the DOM, which
                    // will have corrected any illegally nested <a> elements.
                    if (linkStartNode && n.handle() == linkStartNode){
                        DOMString href = parseURL(linkStartNode->getAttribute(ATTR_HREF));
                        KURL kURL = KWQ(linkStartNode->getDocument()->part())->completeURL(href.string());
                        
                        NSURL *URL = kURL.getNSURL();
                        [result addAttribute:NSLinkAttributeName value:URL range:NSMakeRange(linkStartLocation, [result length]-linkStartLocation)];
                        linkStartNode = 0;
                    }
                    break;
                
                case ID_OL:
                case ID_UL:
                    if (!hasNewLine)
                        text += '\n';
                    hasNewLine = true;
                    break;

                case ID_LI:
                    {
                        int i, count = listItems.count();
                        for (i = 0; i < count; i++){
                            if (listItems.at(i) == n.handle()){
                                listItemLocations[i].end = [result length];
                                break;
                            }
                        }
                    }
                    if (!hasNewLine)
                        text += '\n';
                    hasNewLine = true;
                    break;

                case ID_TD:
                case ID_TH:
                case ID_HR:
                case ID_DD:
                case ID_DL:
                case ID_DT:
                case ID_PRE:
                case ID_BLOCKQUOTE:
                case ID_DIV:
                    if (!hasNewLine)
                        text += '\n';
                    hasNewLine = true;
                    break;
                case ID_P:
                case ID_TR:
                case ID_H1:
                case ID_H2:
                case ID_H3:
                case ID_H4:
                case ID_H5:
                case ID_H6:
                    if (!hasNewLine)
                        text += '\n';
                    // An extra newline is needed at the start, not the end, of these types of tags,
                    // so don't add another here.
                    hasNewLine = true;
                    break;
            }
            
            NSAttributedString *partialString = [[NSAttributedString alloc] initWithString:text.getNSString()];
            [result appendAttributedString:partialString];
            [partialString release];
        }

        n = next;
    }
    
    [pendingStyledSpace release];
    
    // Apply paragraph styles from outside in.  This ensures that nested lists correctly
    // override their parent's paragraph style.
    {
        unsigned i, count = listItems.count();
        ElementImpl *e;
        ListItemInfo info;

#ifdef POSITION_LIST
        NodeImpl *containingBlock;
        int containingBlockX, containingBlockY;
        
        // Determine the position of the outermost containing block.  All paragraph
        // styles and tabs should be relative to this position.  So, the horizontal position of 
        // each item in the list (in the resulting attributed string) will be relative to position 
        // of the outermost containing block.
        if (count > 0){
            containingBlock = _startNode;
            while (containingBlock->renderer()->isInline()){
                containingBlock = containingBlock->parentNode();
            }
            containingBlock->renderer()->absolutePosition(containingBlockX, containingBlockY);
        }
#endif
        
        for (i = 0; i < count; i++){
            e = listItems.at(i);
            info = listItemLocations[i];
            
            if (info.end < info.start)
                info.end = [result length];
                
            RenderObject *r = e->renderer();
            RenderStyle *style = r->style();

            int rx;
            NSFont *font = style->font().getNSFont();
            float pointSize = [font pointSize];

#ifdef POSITION_LIST
            int ry;
            r->absolutePosition(rx, ry);
            rx -= containingBlockX;
            
            // Ensure that the text is indented at least enough to allow for the markers.
            rx = MAX(rx, (int)maxMarkerWidth);
#else
            rx = (int)MAX(maxMarkerWidth, pointSize);
#endif

            // The bullet text will be right aligned at the first tab marker, followed
            // by a space, followed by the list item text.  The space is arbitrarily
            // picked as pointSize*2/3.  The space on the first line of the text item
            // is established by a left aligned tab, on subsequent lines it's established
            // by the head indent.
            NSMutableParagraphStyle *mps = [[NSMutableParagraphStyle alloc] init];
            [mps setFirstLineHeadIndent: 0];
            [mps setHeadIndent: rx];
            [mps setTabStops:[NSArray arrayWithObjects:
                        [[[NSTextTab alloc] initWithType:NSRightTabStopType location:rx-(pointSize*2/3)] autorelease],
                        [[[NSTextTab alloc] initWithType:NSLeftTabStopType location:rx] autorelease],
                        nil]];
            [result addAttribute:NSParagraphStyleAttributeName value:mps range:NSMakeRange(info.start, info.end-info.start)];
            [mps release];
        }
    }

    return result;

    KWQ_UNBLOCK_EXCEPTIONS;

    return nil;
}
#endif

QRect KWQKHTMLPart::selectionRect() const
{
    if(!xmlDocImpl()){
        return QRect();
    }

    RenderCanvas *root = static_cast<RenderCanvas *>(xmlDocImpl()->renderer());
    if (!root) {
        return QRect();

    }

    return root->selectionRect();
}

// returns NSRect because going through QRect would truncate any floats
QRect KWQKHTMLPart::visibleSelectionRect() const
{
    if (!d->m_view) {
        return QRect(-1,-1,-1,-1);
    }
#if 0
    NSView *documentView = d->m_view->getDocumentView();
    if (!documentView) {
        return NSZeroRect;
    }
    return NSIntersectionRect(selectionRect(), [documentView visibleRect]);     
#endif
    return QRect(0,0,0,0);
}

#if 0
NSImage *KWQKHTMLPart::imageFromRect(NSRect rect) const
{
    NSView *view = d->m_view->getDocumentView();
    if (!view) {
        return nil;
    }
    
    NSRect bounds = [view bounds];
    NSImage *resultImage = [[[NSImage alloc] initWithSize:rect.size] autorelease];
    
    KWQ_BLOCK_EXCEPTIONS;
    
    [resultImage setFlipped:YES];
    [resultImage lockFocus];
    
    [NSGraphicsContext saveGraphicsState];
    CGContextTranslateCTM((CGContext *)[[NSGraphicsContext currentContext] graphicsPort],
                          -(NSMinX(rect) - NSMinX(bounds)), -(NSMinY(rect) - NSMinY(bounds)));
    [view drawRect:rect];
    [NSGraphicsContext restoreGraphicsState];
    
    [resultImage unlockFocus];
    [resultImage setFlipped:NO];
    
    KWQ_UNBLOCK_EXCEPTIONS;
    
    return resultImage;
}

NSImage *KWQKHTMLPart::selectionImage() const
{
    _drawSelectionOnly = true;  // invoke special drawing mode
    NSImage *result = imageFromRect(visibleSelectionRect());
    _drawSelectionOnly = false;
    return result;
}

NSImage *KWQKHTMLPart::snapshotDragImage(DOM::Node node, NSRect *imageRect, NSRect *elementRect) const
{
    RenderObject *renderer = node.handle()->renderer();
    if (!renderer) {
        return nil;
    }
    
    renderer->updateDragState(true);    // mark dragged nodes (so they pick up the right CSS)
    d->m_doc->updateLayout();        // forces style recalc - needed since changing the drag state might
                                        // imply new styles, plus JS could have changed other things
    QRect topLevelRect;
    NSRect paintingRect = renderer->paintingRootRect(topLevelRect);

    _elementToDraw = node;              // invoke special sub-tree drawing mode
    NSImage *result = imageFromRect(paintingRect);
    renderer->updateDragState(false);
    _elementToDraw = 0;

    if (elementRect) {
        *elementRect = topLevelRect;
    }
    if (imageRect) {
        *imageRect = paintingRect;
    }
    return result;
}

NSFont *KWQKHTMLPart::fontForCurrentPosition() const
{
    if (d->m_selection.state() == Selection::NONE)
        return nil;
    
    Range range(d->m_selection.toRange());
    Position pos(range.startContainer().handle(), range.startOffset());
    ASSERT(pos.notEmpty());
    ElementImpl *elem = pos.element();
    if (d->m_typingStyle) {
        if (!xmlDocImpl())
            return nil;

        int exceptionCode = 0;
        ElementImpl *styleElement = xmlDocImpl()->createHTMLElement("SPAN", exceptionCode);
        ASSERT(exceptionCode == 0);
        
        styleElement->setAttribute(ATTR_STYLE, d->m_typingStyle->cssText().implementation(), exceptionCode);
        ASSERT(exceptionCode == 0);
        
        TextImpl *text = xmlDocImpl()->createEditingTextNode("");
        styleElement->appendChild(text, exceptionCode);
        ASSERT(exceptionCode == 0);

        elem->appendChild(styleElement, exceptionCode);
        ASSERT(exceptionCode == 0);
        
        RenderObject *renderer = styleElement->renderer();
        ASSERT(renderer);
        NSFont *result = renderer->style()->font().getNSFont();
        
        styleElement->removeChild(text, exceptionCode);
        ASSERT(exceptionCode == 0);

        elem->removeChild(styleElement, exceptionCode);
        ASSERT(exceptionCode == 0);
        
        return result;
    }
    else {
        RenderObject *renderer = elem->renderer();
        if (renderer)
            return renderer->style()->font().getNSFont();
    }
    return nil;
}
#endif

KWQWindowWidget *KWQKHTMLPart::topLevelWidget()
{
    return _windowWidget;
}

int KWQKHTMLPart::selectionStartOffset() const
{
    return d->m_selection.start().offset();
}

int KWQKHTMLPart::selectionEndOffset() const
{
    return d->m_selection.end().offset();
}

NodeImpl *KWQKHTMLPart::selectionStart() const
{
    return d->m_selection.start().node();
}

NodeImpl *KWQKHTMLPart::selectionEnd() const
{
    return d->m_selection.end().node();
}

void KWQKHTMLPart::setBridge(WebCoreBridge *p)
{
    if (_bridge != p) {
	delete _windowWidget;
	_windowWidget = 0;
    }
    _bridge = p;
    if  (_bridge)
	_windowWidget = new KWQWindowWidget(_bridge);
}

QString KWQKHTMLPart::overrideMediaType()
{
    return _bridge->overrideMediaType();
}

void KWQKHTMLPart::setMediaType(const QString &type)
{
    if (d->m_view) {
        d->m_view->setMediaType(type);
    }
}

void KWQKHTMLPart::setShowsFirstResponder(bool flag)
{
    if (flag != _showsFirstResponder) {
        _showsFirstResponder = flag;
        DocumentImpl *doc = xmlDocImpl();
        if (doc) {
            DOM::NodeImpl *node = doc->focusNode();
            if (node && node->renderer())
                node->renderer()->repaint();
        }
    }
}

QChar KWQKHTMLPart::backslashAsCurrencySymbol() const
{
    DocumentImpl *doc = xmlDocImpl();
    if (!doc) {
        return '\\';
    }
    Decoder *decoder = doc->decoder();
    if (!decoder) {
        return '\\';
    }
    const QTextCodec *codec = decoder->codec();
    if (!codec) {
        return '\\';
    }
    return codec->backslashAsCurrencySymbol();
}

QColor KWQKHTMLPart::bodyBackgroundColor(void) const
{
    DOM::HTMLDocumentImpl *doc = docImpl();
    
    if (doc){
        DOM::HTMLElementImpl *body = doc->body();
        QColor bgColor =  body->renderer()->style()->backgroundColor();
        
        if (bgColor.isValid())
            return bgColor;
    }
    return QColor();
}

WebCoreKeyboardUIMode KWQKHTMLPart::keyboardUIMode() const
{
    return _bridge->keyboardUIMode();
}


void KWQKHTMLPart::setName(const QString &name)
{
    QString n = name;

    KWQKHTMLPart *parent = KWQ(parentPart());

    if (parent && (name.isEmpty() || parent->frameExists(name))) {
	n = parent->requestFrameName();
    }

    KHTMLPart::setName(n);

    _bridge->didSetName(n.utf8());
}

void KWQKHTMLPart::didTellBridgeAboutLoad(const QString &urlString)
{
    urlsBridgeKnowsAbout.insert(urlString, (char *)1);
}


bool KWQKHTMLPart::haveToldBridgeAboutLoad(const QString &urlString)
{
    return urlsBridgeKnowsAbout.find(urlString) != 0;
}

void KWQKHTMLPart::clear()
{
    urlsBridgeKnowsAbout.clear();
    KHTMLPart::clear();
}

void KHTMLPart::print()
{
    KWQ(this)->_bridge->print();
}

#if 0
KJS::Bindings::Instance *KWQKHTMLPart::getAppletInstanceForView (NSView *aView)
{
    // Get a pointer to the actual Java applet instance.
    jobject applet = [_bridge pollForAppletInView:aView];
    
    if (applet)
        // Wrap the Java instance in a language neutral binding and hand
        // off ownership to the APPLET element.
        return KJS::Bindings::Instance::createBindingForLanguageInstance (KJS::Bindings::Instance::JavaLanguage, applet);
    
    return 0;
}

@interface NSObject (WebPlugIn)
- (id)objectForWebScript;
@end

KJS::Bindings::Instance *KWQKHTMLPart::getEmbedInstanceForView (NSView *aView)
{
    if ([aView respondsToSelector:@selector(objectForWebScript)]){
        id object = [aView objectForWebScript];
        if (object)
            return KJS::Bindings::Instance::createBindingForLanguageInstance (KJS::Bindings::Instance::ObjectiveCLanguage, object);
    }
    return 0;
}

void KWQKHTMLPart::addPluginRootObject(const KJS::Bindings::RootObject *root)
{
    rootObjects.append (root);
}

void KWQKHTMLPart::cleanupPluginRootObjects()
{
    KJS::Bindings::RootObject *root;
    while ((root = rootObjects.getLast())) {
        root->removeAllNativeReferences ();
        rootObjects.removeLast();
    }
}
#endif

void KWQKHTMLPart::registerCommandForUndo(const khtml::EditCommand &cmd)
{
    ASSERT(cmd.handle());
#if 0
    KWQEditCommand *kwq = [KWQEditCommand commandWithEditCommandImpl:cmd.handle()];
    [[_bridge undoManager] registerUndoWithTarget:_bridge selector:@selector(undoEditing:) object:kwq];
    _haveUndoRedoOperations = YES;
#endif
    LOG(NotYetImplemented,"KWIQ");

}

void KWQKHTMLPart::registerCommandForRedo(const khtml::EditCommand &cmd)
{
    ASSERT(cmd.handle());
#if 0
    KWQEditCommand *kwq = [KWQEditCommand commandWithEditCommandImpl:cmd.handle()];
    [[_bridge undoManager] registerUndoWithTarget:_bridge selector:@selector(redoEditing:) object:kwq];
    _haveUndoRedoOperations = YES;
#endif
    LOG(NotYetImplemented,"KWIQ");

}

void KWQKHTMLPart::clearUndoRedoOperations()
{
#if 0
    if (_haveUndoRedoOperations) {
	[[_bridge undoManager] removeAllActionsWithTarget:_bridge];
	_haveUndoRedoOperations = NO;
    }
#endif
}

bool KWQKHTMLPart::interceptEditingKeyEvent()
{
#if 0
    return [_bridge interceptEditingKeyEvent:_currentEvent];
#endif
    return false;
}

void KWQKHTMLPart::issueUndoCommand()
{
#if 0
    if (canUndo())
        [[_bridge undoManager] undo];
#endif
}

void KWQKHTMLPart::issueRedoCommand()
{
#if 0
    if (canRedo())
        [[_bridge undoManager] redo];
#endif
    
}

void KWQKHTMLPart::issueCutCommand()
{
#if 0
    [_bridge issueCutCommand];
#endif
    LOG(NotYetImplemented,"KWIQ");
}

void KWQKHTMLPart::issueCopyCommand()
{
#if 0
    [_bridge issueCopyCommand];
#endif
    LOG(NotYetImplemented,"KWIQ");
}

void KWQKHTMLPart::issuePasteCommand()
{
#if 0
    [_bridge issuePasteCommand];
#endif
    LOG(NotYetImplemented,"KWIQ");
}

bool KHTMLPart::canUndo() const
{
#if 0
    return [[KWQ(this)->_bridge undoManager] canUndo];
#endif
    LOG(NotYetImplemented,"KWIQ");
    return false;
}

bool KHTMLPart::canRedo() const
{
#if 0
    return [[KWQ(this)->_bridge undoManager] canRedo];
#endif
    LOG(NotYetImplemented,"KWIQ");
    return false;
}

void KWQKHTMLPart::respondToChangedSelection()
{
    _bridge->respondToChangedSelection();
}

void KWQKHTMLPart::respondToChangedContents()
{
    _bridge->respondToChangedContents();
}

bool KWQKHTMLPart::isContentEditable() const
{
#if 0
    return [_bridge isEditable];
#endif
    // LOG(NotYetImplemented,"KWIQ"); 
    //KWIQ: logging disabled due to lots of msgs
    return false;
}

bool KWQKHTMLPart::shouldBeginEditing(const Range &range) const
{
#if 0
    ASSERT(!range.isNull());
    return [_bridge shouldBeginEditing:[DOMRange _rangeWithImpl:range.handle()]];
#endif
    LOG(NotYetImplemented,"KWIQ");
    return false;
}

bool KWQKHTMLPart::shouldEndEditing(const Range &range) const
{
#if 0
    ASSERT(!range.isNull());
    return [_bridge shouldEndEditing:[DOMRange _rangeWithImpl:range.handle()]];
#endif
    LOG(NotYetImplemented,"KWIQ");
    return false;
}


// from WebCore-xxx/kwq/KWQKHTMLPart.cpp
bool KWQKHTMLPart::isFrameSet() const
{
    DocumentImpl *document = d->m_doc;
    if (!document || !document->isHTMLDocument())
        return false;
    DOM::NodeImpl *body = static_cast<DOM::HTMLDocumentImpl *>(document)->body();
    return body && body->renderer() && body->id() == ID_FRAMESET;
}
