#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#include <Application.h>
#include <Window.h>
#include <View.h>
#include <qwidget.h>

class TWindow: public BWindow {

public:
	TWindow( BRect rect, const char *title )
		:BWindow(rect, title, B_TITLED_WINDOW, 0 )
	{
		fRootView = new BView( Bounds(), "root_view", B_FOLLOW_ALL_SIDES, 0 );
		BRect rect = fRootView->Bounds();

		fRootWidget = new QWidget(NULL, NULL);
		fRootWidget->SetResizingMode( B_FOLLOW_ALL );
		fRootWidget->MoveTo( rect.LeftTop() );
		fRootWidget->ResizeTo( rect.Width(), rect.Height() );
		fRootView->AddChild( fRootWidget );
		fRootWidget->show();	//does it work to have this here?
		
		AddChild( fRootView );
	}
	~TWindow(){}
	
	virtual bool QuitRequested() {
		be_app->PostMessage( B_QUIT_REQUESTED );
		return true;
	}
	
	BView *RootView() { return fRootView; }
	
	QWidget *RootWidget() { return fRootWidget; }

private:
	BView *fRootView;
	QWidget *fRootWidget;

};

#endif

