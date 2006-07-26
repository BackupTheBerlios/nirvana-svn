
#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#include <AppKit.h>
#include "KWQWidget.h"

class TestWindow: public BWindow {
public:
    TestWindow( BRect rect, const char *title )
	: BWindow(rect, title, B_TITLED_WINDOW, 0 )
    {
	view = new BView( Bounds(), "root_view", B_FOLLOW_ALL_SIDES, 0 );
	BRect rect = view->Bounds();
	widget = new QWidget(NULL, "root_widget", 0);
	widget->SetResizingMode( B_FOLLOW_ALL );
	widget->MoveTo( rect.LeftTop() );
	widget->ResizeTo( rect.Width(), rect.Height() );
	view->AddChild( widget );
	widget->show();	//does it work to have this here?
	AddChild( view );
    }
    ~TestWindow(){}
	
    virtual bool QuitRequested() {
	be_app->PostMessage( B_QUIT_REQUESTED );
	return true;
    }
	
    BView *RootView() { return view; }
    QWidget *RootWidget() { return widget; }

private:
    BView *view;
    QWidget *widget;

};

#endif
