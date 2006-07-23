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

#ifndef QOBJECT_H_
#define QOBJECT_H_

#include <AppKit.h>

#include "KWQDef.h"
#include "KWQSignal.h"

#include "KWQNamespace.h"
#include "KWQString.h"
#include "KWQEvent.h"
#include "KWQStringList.h"
#include "KWQPtrList.h"

#define slots : public
#define SLOT(x) "SLOT:" #x
#define signals protected
#define SIGNAL(x) "SIGNAL:" #x
#define emit
#define Q_OBJECT
#define Q_PROPERTY(text)

class QEvent;
class QPaintDevice;
class QPaintDeviceMetrics;
class QWidget;
class QColor;
class QColorGroup;
class QPalette;
class QPainter;
class QRegion;
class QSize;
class QSizePolicy;
class QRect;
class QFont;
class QFontMetrics;
class QBrush;
class QBitmap;
class QMovie;
class QTimer;
class QImage;
class QVariant;

class KWQGuardedPtrBase;
class KWQSignal;
class KWQObjectTimerTarget;

#define QOBJECT_TYPE(x) this->set_class_type(QObjectTypes::x)
#define QOBJECT_IS_A(object, type) ((object)->is_a(QObjectTypes::type))

struct QObjectTypes {
    static const int QObject      = 1;
    //KParts::
    static const int Part         = 2;
    static const int ReadOnlyPart = 4;
    static const int Factory = 33554432; /* 2^25 */
    
    //khtml::
    static const int CachedObject = 32;
    static const int DocLoader    = 64;
    static const int Loader       = 128;
    static const int CachedImage  = 256;    	

    static const int RenderCheckBox     = 1024;
    static const int RenderFileButton   = 2048;
    static const int RenderFormElement  = 4096;
    static const int RenderLineEdit     = 8092;
    static const int RenderSelect       = 16384;
    static const int RenderTextArea     = 32768;
    static const int RenderWidget       = 65536;
    static const int RenderScrollMediator = 131072;
    static const int RenderRadioButton  = 134217728;
    static const int RenderButton       = 268435456;
  
    //KJS::
    static const int WindowQObject      = 524288;
    static const int XMLHttpRequestQObject = 1048576;
	
    //DOM
    static const int DocumentImpl          = 16777216;

    static const int QWidget               = 2097152;
    static const int QFrame                = 4194304;
    static const int QScrollView           = 8388608;

    static const int KHTMLPart    = 8;
    static const int KWQKHTMLPart = 16;
    static const int KHTMLView              = 262144;
    static const int Tokenizer    = 67108864;
    
    struct KParts {
	static const int Part         = QObjectTypes::Part;
	static const int ReadOnlyPart = QObjectTypes::ReadOnlyPart;
	static const int Factory      = QObjectTypes::Factory;
    };
    
    struct khtml {
	static const int CachedObject = QObjectTypes::CachedObject;
	static const int CachedImage  = QObjectTypes::CachedImage;
	static const int DocLoader    = QObjectTypes::DocLoader;
	static const int Loader       = QObjectTypes::Loader;
	static const int RenderCheckBox     = QObjectTypes::RenderCheckBox;
	static const int RenderFileButton   = QObjectTypes::RenderFileButton;
	static const int RenderFormElement  = QObjectTypes::RenderFormElement;
	static const int RenderLineEdit     = QObjectTypes::RenderLineEdit;
	static const int RenderSelect       = QObjectTypes::RenderSelect;
	static const int RenderTextArea     = QObjectTypes::RenderTextArea;
	static const int RenderWidget       = QObjectTypes::RenderWidget;
	static const int RenderScrollMediator = QObjectTypes::RenderScrollMediator;
        static const int RenderRadioButton = QObjectTypes::RenderRadioButton;
      static const int RenderButton = QObjectTypes::RenderButton;
    };
    
    struct KJS {
	static const int WindowQObject = QObjectTypes::WindowQObject;
	static const int XMLHttpRequestQObject = QObjectTypes::XMLHttpRequestQObject;
    };

    struct DOM {
	static const int DocumentImpl        = QObjectTypes::DocumentImpl;
    };

};


class QObject : public Qt {
public:
    QObject(QObject *parent = 0, const char *name = 0);
    virtual ~QObject();
    static BLooper *looper;

    static void connect(const QObject *sender, const char *signal, const QObject *receiver, const char *member);
    static void disconnect(const QObject *sender, const char *signal, const QObject *receiver, const char *member);
    static void clearPausedTimers (const void *key);
    static const QObject *sender() { return _sender; }
    static bool defersTimers() { return _defersTimers; }
    static void setDefersTimers(bool defers);

    virtual void timerEvent(QTimerEvent *);
    virtual bool eventFilter(QObject *object, QEvent *event) { return false; }
    virtual bool event(QEvent *);

    void connect(const QObject *sender, const char *signal, const char *member) const { connect(sender, signal, this, member); }
    bool inherits(const char *className) const;
    int startTimer(int);
    void killTimer(int);
    void killTimers();
    void pauseTimer(int _timerId, const void *key);
    void resumeTimers(const void *key, QObject *target);
    void installEventFilter(const QObject *o) { _eventFilterObject = o; }
    void removeEventFilter(const QObject *) { _eventFilterObject = 0; }
    const QObject *eventFilterObject() const { return _eventFilterObject; }
    void blockSignals(bool b) { _signalsBlocked = b; }
    bool is_a(int t) const { return _class_type & t; }

protected:
    /** circumvent Qt class info construct and C++ RTTI with a bitfield.
     *  use QOBJECT_TYPE() macro inside  QOBJECT -defined classes' constructors */
    void set_class_type(int t) { _class_type |= t; }
private:
    // no copying or assignment
    QObject(const QObject &);
    QObject &operator=(const QObject &);  
    KWQSignal *findSignal(const char *signalName) const;    
    QPtrList<QObject> _guardedPtrDummyList;
    mutable KWQSignal *_signalListHead;
    bool _signalsBlocked;
    static const QObject *_sender;
    KWQSignal _destroyed;
    const QObject *_eventFilterObject;
    static bool _defersTimers;
    QPtrList<KWQObjectTimerTarget> _timers;
    int _class_type;
    friend class KWQGuardedPtrBase;
    friend class KWQSignal;
    friend class KWQObjectSenderScope;
};

class KWQObjectSenderScope
{
public:
    KWQObjectSenderScope(const QObject *);
    ~KWQObjectSenderScope();

private:
    const QObject *_savedSender;
};

BLooper* getLooperForObjectTimers();

#endif
