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

#ifndef QWIDGET_H_
#define QWIDGET_H_
//#include <gdk/gdk.h>

#include <AppKit.h>

#include "KWQObject.h"
#include "KWQPaintDevice.h"
#include "KWQPainter.h"
#include "KWQPointArray.h"
#include "KWQSize.h"
#include "KWQPalette.h"
#include "KWQFont.h"
#include "KWQCursor.h"
#include "KWQEvent.h"
#include "KWQStyle.h"


//typedef struct _GtkWidget GtkWidget;
//typedef struct _PangoFontDescription PangoFontDescription;

BFont* createPangoFontDescription(const QFont* font) ;
    
class QWidgetPrivate;

class QWidget : public BView, public QObject, public QPaintDevice {
public:
  
    enum WidgetFlags {
	WResizeNoErase = 1 << 0,
	WRepaintNoErase = 1 << 1,
	WPaintUnclipped = 1 << 2,
    };
    
    enum FocusPolicy {
        NoFocus = 0,
        TabFocus = 0x1,
        ClickFocus = 0x2,
        StrongFocus = 0x3,
        WheelFocus = 0x7
    };

    enum BackgroundMode {
	NoBackground
    };

    QWidget(QWidget * parent = 0, const char * name = 0, int f = 0);
    //QWidget(GtkWidget*);

    virtual ~QWidget();
    virtual QSize sizeHint();
    virtual void setEnabled(bool);
    virtual bool isEnabled();
    //virtual void paint(QPainter *, const QRect &);
    virtual QRect frameGeometry();
    virtual void setFrameGeometry(const QRect &);
    virtual int baselinePosition(int height); // relative to the top of the widget
    virtual QPoint mapFromGlobal(const QPoint &);
    virtual bool hasFocus();
    virtual void setFocus();
    virtual bool checksDescendantsForFocus();
    virtual FocusPolicy focusPolicy();
    virtual void setFocusProxy(QWidget *) {};
    virtual void setPalette(const QPalette &);
    virtual void setFont(const QFont &);
    //virtual void setWritingDirection(QPainter::TextDirection direction);
    virtual void setBView(BView* c);
    virtual BView* getBView() { return _widget; }
    
    void setActiveWindow();
    void setAutoMask(bool) { }
    void setMouseTracking(bool) { }
    long winId();
    int x();
    int y();
    int width();
    int height();
    QSize size();
    virtual void resize(int,int);
    void resize(const QSize &);
    QPoint pos();
    void move(int, int);
    void move(const QPoint &);
    void clearFocus();
    void setFocusPolicy(FocusPolicy) {};
    const QPalette& palette();
    void unsetPalette();
    QStyle &style();
    void setStyle(QStyle *);
    QFont font();
    void constPolish();
    bool isVisible();
    void setCursor(const QCursor &);
    QCursor cursor();
    void unsetCursor();
    virtual bool focusNextPrevChild(bool);
    bool hasMouseTracking();
    void show();
    void hide();
    void setBackgroundMode(BackgroundMode) { }
    void setAcceptDrops(bool) { }
    void setIsSelected(bool) { }

    void showEvent(QShowEvent *) { }
    void hideEvent(QHideEvent *) { }
    void wheelEvent(QWheelEvent *) { }
    void keyPressEvent(QKeyEvent *) { }
    void keyReleaseEvent(QKeyEvent *) { }
    void focusInEvent(QFocusEvent *) { }
    void focusOutEvent(QFocusEvent *) { }

    //void setGtkWidget(GtkWidget* widget);
    //GtkWidget* getGtkWidget() const { return _widget;}
    //BView* getBView();
    
    void displayRect(int x, int y, int w, int h);
    void lockDrawingFocus();
    void unlockDrawingFocus();
    void enableFlushDrawing();
    void disableFlushDrawing();
    void setDrawingAlpha(float alpha);
    void sendConsumedMouseUp();
    
private:
    QWidgetPrivate *data;
    BView* _widget; // for BControl
    
};

#endif
