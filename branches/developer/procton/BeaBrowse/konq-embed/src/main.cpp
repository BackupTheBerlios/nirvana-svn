#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <OS.h>

#include <Application.h>
#include <Roster.h>
#include <Message.h>
#include <Messenger.h>
#include <MessageRunner.h>
#include <Clipboard.h>
#include <Window.h>
#include <View.h>
//#include <Frameview.h>
#include <Button.h>
#include <TextControl.h>
#include <String.h>
#include <List.h>

#include <khtml_part.h>
#include <khtmlview.h>
#include <misc/loader.h>

#include <klibloader.h>
#include <ecma/kjs_proxy.h>
#include <kiconloader.h>

#include <string>
#include <list>

#include <Menu.h>
//#include <gui/requesters.h>
#include <Path.h>
#include <File.h>
#include <Directory.h>
//#include <storage/tempfile.h>

#include <Autolock.h>
#include <atheos/amutex.h>
//#include <Locker.h>
//#include <Autolock.h>

#include "toolbar.h"
#include "statusbar.h"
#include "searchdialog.h"
#include "downloaddialog.h"

//using namespace os;

static BPoint get_window_pos()
{
	static BPoint cLastPos( 100, 100 );
	cLastPos.x += 30;
	cLastPos.y += 30;
	if ( cLastPos.x > 500 ) {
		cLastPos.x = 100;
	}
	if ( cLastPos.y > 500 ) {
		cLastPos.y = 100;
	}
	return( cLastPos );
}

const int32 ID_URL_CHANGED = 'IDuc';
const int32 ID_OPEN_URL = 'IDou';
const int32 ID_GOHOME = 'IDgh';
const int32 ID_NEXT_URL = 'IDnu';
const int32 ID_PREV_URL = 'IDpu';
const int32 ID_RELOAD = 'IDRe';
const int32 ID_FIND = 'IDFi';
const int32 ID_REPEAT_FIND = 'IDrf';
const int32 ID_DO_SEARCH = 'IDds';
const int32 ID_ACTIVATE_URLEDIT = 'IDae';
const int32 ID_SEARCH_CLOSED = 'IDsc';
const int32 ID_START_DOWNLOAD = 'IDsd';
const int32 ID_CANCEL_DOWNLOAD = 'IDcd';
const int32 ID_NEW_WINDOW = 'IDnw';
const int32 ID_OPEN_LINK = 'IDol';
const int32 ID_SAVE_LINK = 'IDsl';
const int32 ID_COPY_LINK_LOCATION = 'IDcl';
const int32 ID_COPY = 'IDcp';
const int32 ID_VIEW_DOC_SRC = 'IDvd';
const int32 ID_VIEW_FRAME_SRC = 'IDvf';

enum { BI_BACK, BI_FORWARD, BI_GOHOME, BI_RELOAD, BI_FIND, BI_COPY };

// ### from ecma/kjs_proxy.cpp and khtml_factory.cpp
extern "C"
{
	KJSProxy *kjs_html_init( KHTMLPart * );
	void *init_libkhtml();
}

// ### from khtml_part.cpp
extern int kjs_lib_count;


const char* g_pzDefaultURL = "file:/Applications/ABrowse/docs/startup.html";

#if 0
struct HistoryEntry
{
	BMessage m_cState;
	std::string m_cURL;
};

struct DownloadNode
{
	DownloadNode() {
	m_psDownloadStream = NULL;
	m_pcRequester	  = NULL;
	m_pcProgressDlg	= NULL;
	m_nContentSize	 = -1;
	m_pcTmpFile		= NULL;
	m_pcFile		   = NULL;
	m_nBytesReceived   = 0;
	m_bDone			= false;
	m_bCanceled		= false;
	}
	
	KParts::DownloadStream* m_psDownloadStream;
	DownloadRequester*		m_pcRequester;
	DownloadProgress*		m_pcProgressDlg;
	std::string			m_cURL;
	std::string			m_cPreferredName;
	off_t			m_nContentSize;
	bigtime_t			m_nStartTime;
	std::string				m_cPath;
//	os::TempFile*		   m_pcTmpFile;
	BFile*		   m_pcTmpFile;
	BFile*				m_pcFile;
	int					m_nBytesReceived;
	bool			m_bDone;
	bool				m_bCanceled;
};
#endif

class TopView : public QWidget
{
public:
	TopView() : QWidget( NULL, "top_view" ) {}
	
	virtual void KeyDown(const char* pzRawString, int32 numBytes )
	{
		BMessage *curMsg = Window()->CurrentMessage();
		int32 key(0), modifiers(0);
		
		curMsg->FindInt32( "key", &key );
		curMsg->FindInt32( "modifiers", &modifiers );
		
	#warning These shortcuts should change to be more BeOS like.
		if ( modifiers & B_CONTROL_KEY ) {
			switch( pzRawString[0] )
			{
			case B_TAB:
				Window()->PostMessage( ID_ACTIVATE_URLEDIT, Window() );
				break;
			case 'r':
				Window()->PostMessage( ID_RELOAD, Window() );
				break;			
			case 'f':
				Window()->PostMessage( ID_FIND, Window() );
				break;
			case B_INSERT:
			case 'c':
				Window()->PostMessage( ID_COPY, Window() );
				break;
			case 'n':
				Window()->PostMessage( ID_NEW_WINDOW, Window() );
				break;
			default:
				QWidget::KeyDown( pzRawString, numBytes );
				break;
			}
		} else {
			switch( pzRawString[0] )
			{
			case B_BACKSPACE:
				Window()->PostMessage( ID_PREV_URL, Window() );
				break;			
			case B_FUNCTION_KEY:
				switch( key )
				{
//				case 4:	// F3
				case B_F3_KEY:	// F3
					Window()->PostMessage( ID_REPEAT_FIND, Window() );
					break;
				default:
					QWidget::KeyDown( pzRawString, numBytes );
					break;
				}
				break;
			default:
				QWidget::KeyDown( pzRawString, numBytes );
				break;
			
			}
		}
	}
	
};

class BrowserWindow : public BWindow, public KParts::BrowserCallback
{
public:
//	BrowserWindow( BRect cFrame, std::vector<HistoryEntry*>* pcHistory, bool bLoadPos );
	BrowserWindow( BRect cFrame, bool bLoadPos );
	~BrowserWindow();

	void OpenURL( const std::string& cURL, const KParts::URLArgs& cArgs );
	
  // From BWindow:
	virtual void DispatchMessage( BMessage* aMsg, BHandler* aHandler ) {
//		BAutolock al( GlobalMutex::GetMutex() );

		BWindow::DispatchMessage( aMsg, aHandler );

	}
	virtual void MessageReceived( BMessage* pcMsg );
	
	bool	QuitRequested();

// From BrowserCallback:
	virtual void EnableAction( const char* /*name*/, bool /*enabled*/ ) {}
	virtual void OpenURLRequest( const std::string &url, const KParts::URLArgs& args );
	virtual void OpenURLNotify();
	virtual void Completed();
	virtual void SetLocationBarURL( const std::string &url );
	virtual void CreateNewWindow( const std::string& url, const KParts::URLArgs& args );
	virtual KParts::ReadOnlyPart* CreateNewWindow( const std::string& url, const KParts::URLArgs& args,
						   const KParts::WindowArgs& windowArgs );
	virtual void PopupMenu( KParts::BrowserExtension* pcSource, const std::string& cURL );
	virtual void LoadingProgress( int percent );
	virtual void SpeedProgress( int bytesPerSecond );
	virtual void InfoMessage( const std::string& cString );
	virtual void SetWindowCaption( KParts::ReadOnlyPart* pcPart, const std::string &caption );
	virtual void SetStatusBarText( const std::string &text );
	virtual void SelectionInfo( const std::string& text );
	
	virtual bool BeginDownload( KParts::DownloadStream* psStream );
	virtual bool HandleDownloadData( KParts::DownloadStream* psStream, const void* pData, int nLen );
	virtual void DownloadFinished( KParts::DownloadStream* psStream, int nError );

	  // New members:
	KHTMLPart*	GetPart() const { return( m_pcHTMLPart ); }
	void SetPart( KHTMLPart *aPart ) {
		m_pcHTMLPart = aPart;
		m_pcHTMLPart->browserExtension()->SetCallback( this );
	}
	void SetStatusBar( StatusBar *aStatusBar ) { m_pcStatusBar = aStatusBar; }
	void SetURLView( BTextControl *aURLView ) { m_pcURLView = aURLView; }
	
private:
	static vint32 s_nWndCount;

	KHTMLPart* m_pcHTMLPart;

	StatusBar* m_pcStatusBar;
	BTextControl*  m_pcURLView;
	BMenu*  m_pcPopupMenu;
	
};


//------Root View--------

class RootView: public BView {

public:
	RootView( BRect frame );
	
	void AttachedToWindow();

private:

	BTextControl *mURLView;
	StatusBar *mStatusBar;
	QWidget *mTopView;

};


//------- The App class -----
const int TIMER_TICK = 'TmTi';

class	MyApp : public BApplication
{
public:
	MyApp( const char* pzName );
	~MyApp();

	// From BHandler:
	virtual void	MessageReceived( BMessage* pcMsg );

	virtual void TimerTick( int /*nID*/ ) {
//		BAutolock al( GlobalMutex::GetMutex() );
		
		khtml::Loader::Run();

	}
	
private:

	BMessageRunner *mTimer;
	BList *mWindows;

};

//----------------------------------------------------------------------------
// NAME:
// DESC:
// NOTE:
// SEE ALSO:
//----------------------------------------------------------------------------

MyApp::MyApp( const char* pzName ) : BApplication( pzName )
{
//	signal( SIGCHLD, SIG_IGN );

//	BAutolock al( GlobalMutex::GetMutex() );
//	SetMutex( GlobalMutex::GetMutex() );
	mWindows = new BList();
	
	kjs_lib_count++; // ### hack, prevent khtmlpart from deleting the js klibrary object

	KLibLoader::self()->library( "kjs_html" )->registerSymbol( "kjs_html_init", (void *)&kjs_html_init );
	KLibLoader::self()->library( "libkhtml" )->registerSymbol( "init_libkhtml", (void *)&init_libkhtml );

//	BrowserWindow* pcWindow = new BrowserWindow( BRect( 150, 200, 1000, 800 ), NULL, true );
	BrowserWindow* pcWindow = new BrowserWindow( BRect( 150, 200, 1000, 800 ), true );

	KParts::URLArgs cArgs;

	app_info ai;
	be_app->GetAppInfo( &ai );
	BEntry e( &ai.ref );
	BPath path;
	e.GetParent( &e );
	e.GetPath( &path );
	
	std::string cDefaultPath = path.Path();
//	Directory cDir( open_image_file( IMGFILE_BIN_DIR ) );
//	cDir.GetPath( &cDefaultPath );
	cDefaultPath = std::string("file:") + cDefaultPath + "/docs/startup.html";
//	pcWindow->OpenURL( cDefaultPath, cArgs );

	pcWindow->Show();
//	pcWindow->MakeFocus();

	BMessage msg( TIMER_TICK );
//	mTimer = new BMessageRunner( BMessenger( this ), &msg, 100000, 1 );
	mTimer = new BMessageRunner( BMessenger( this ), &msg, 100000 );
//	AddTimer( this, 1, 100000, false );

//	GlobalMutex::Unlock();

}

//----------------------------------------------------------------------------
// NAME:
// DESC:
// NOTE:
// SEE ALSO:
//----------------------------------------------------------------------------

MyApp::~MyApp()
{
	delete mTimer;
	delete mWindows;
}

//----------------------------------------------------------------------------
// NAME:
// DESC:
// NOTE:
// SEE ALSO:
//----------------------------------------------------------------------------

void MyApp::MessageReceived( BMessage* pcMsg )
{

	switch( pcMsg->what )
	{
	case TIMER_TICK:
		TimerTick( 0 );
//		mTimer->SetCount( 1 );
	default:
		BApplication::MessageReceived( pcMsg );
		return;
	}
}



RootView::RootView(BRect frame)
	:BView( frame, "root_view", B_FOLLOW_ALL, B_WILL_DRAW )
	,mURLView( NULL )
	,mStatusBar( NULL )
	,mTopView( NULL )
{
	BRect rect = frame;
	rect.bottom = rect.top + 20;
	mURLView = new BTextControl( rect, "url_view", "URL:", "", new BMessage( ID_URL_CHANGED ), B_FOLLOW_TOP | B_FOLLOW_LEFT_RIGHT, B_WILL_DRAW ); ///*g_pzDefaultURL*/ );


	rect = frame;
	rect.top = rect.bottom - 20;
	mStatusBar = new StatusBar( rect, "status_bar" );

	rect = frame;
	rect.top = mURLView->Frame().bottom + 1;
	rect.bottom = mStatusBar->Frame().top - 1;

	mTopView = new TopView; 									/* QWidget *mTopView */
	mTopView->SetResizingMode( B_FOLLOW_ALL );
	mTopView->MoveTo( rect.LeftTop() );
	mTopView->ResizeTo( rect.Width(), rect.Height() );

}


void RootView::AttachedToWindow()
{
	SetViewColor( 216,216,216 );
	mURLView->SetDivider( 40 );
	mTopView->show();
	
	AddChild( mURLView );
	AddChild( mTopView );
	AddChild( mStatusBar );

	KHTMLPart *htmlPart = new KHTMLPart( mTopView, "khtmlpart" );

	KHTMLView* pcView = htmlPart->view();
	
	BRect rect = mTopView->Bounds();
	pcView->SetResizingMode( B_FOLLOW_ALL );
	pcView->MoveTo( rect.LeftTop() );
	pcView->ResizeTo( rect.Width(), rect.Height() );
//	pcView->MakeFocus( true );
	pcView->SetTarget( BMessenger( Window() ) );
	pcView->show();
	
	BrowserWindow *bw = (BrowserWindow *)Window();
	bw->SetPart( htmlPart );
	bw->SetStatusBar( mStatusBar );
	bw->SetURLView( mURLView );
	
}

vint32 BrowserWindow::s_nWndCount = 0;

//----------------------------------------------------------------------------
// NAME:
// DESC:
// NOTE:
// SEE ALSO:
//----------------------------------------------------------------------------

//BrowserWindow::BrowserWindow( BRect cFrame, std::vector<HistoryEntry*>* pcHistory, bool bLoadPos ) :
BrowserWindow::BrowserWindow( BRect cFrame, bool /*bLoadPos*/ )
	:BWindow( cFrame, "ABrowse", B_TITLED_WINDOW,  0 )
	,m_pcHTMLPart( NULL )
	,m_pcStatusBar( NULL )
	,m_pcURLView( NULL )
{
	atomic_add( &s_nWndCount, 1 );

//	BAutolock al( GlobalMutex::GetMutex() );
//	GlobalMutex::Lock();
	RootView *pcRoot =  new RootView( Bounds() );

	AddChild( pcRoot );

//	AddChild( pcRoot );

//	AddTimer( this, 1, 100000, false );
//	GlobalMutex::Unlock();

//--------------------------------------------------------------------------
#if 0
	LayoutView* pcFrame = new LayoutView( cWndBounds, "frame_view" );

	
	VLayoutNode* pcRoot = new VLayoutNode( "root" );

	FrameView* pcNavBarFrame = new FrameView( Rect(0,0,0,0), "nav_bar_frame", "" );
	HLayoutNode* pcNavBar = new HLayoutNode( "nav_bar" );


	m_pcURLView = new TextView( Rect(0,0,0,0), "url_view", ""/*g_pzDefaultURL*/ );

	m_pcURLView->SetEventMask( os::TextView::EI_ENTER_PRESSED | os::TextView::EI_ESC_PRESSED );
	m_pcURLView->SetMultiLine( false );
	m_pcURLView->SetMessage( new Message( ID_URL_CHANGED ) );

	m_pcToolBar   = new ToolBar( Rect(0,0,0,0), "tool_bar" );
	m_pcStatusBar = new StatusBar( Rect(), "status_bar" );

	
	m_pcToolBar->AddButton( "hi16-action-back.png", os::Message( ID_PREV_URL ) );
	m_pcToolBar->AddButton( "hi16-action-forward.png", os::Message( ID_NEXT_URL ) );
	m_pcToolBar->AddButton( "hi16-action-gohome.png", os::Message( ID_GOHOME ) );
	m_pcToolBar->AddButton( "hi16-action-reload.png", os::Message( ID_RELOAD ) );
	m_pcToolBar->AddButton( "hi16-action-find.png", os::Message( ID_FIND ) );
	m_pcToolBar->AddButton( "hi16-action-editcopy.png", os::Message( ID_COPY ) );

	pcNavBar->AddChild( m_pcToolBar )->SetBorders( Rect( 4, 2, 10, 2 ) );
	pcNavBar->AddChild( m_pcURLView );

	pcNavBarFrame->SetRoot( pcNavBar );
	
	pcRoot->AddChild( pcNavBarFrame );
	
	
	QWidget* pcTopView = new TopView;
	pcTopView->show();
	
	pcTopView->SetResizeMask( CF_FOLLOW_ALL );
	pcTopView->SetFrame( cWndBounds.Bounds() );
	pcRoot->AddChild( pcTopView );
	pcRoot->AddChild( m_pcStatusBar );
	pcFrame->SetRoot( pcRoot );

   
	m_pcHTMLPart = new KHTMLPart( pcTopView, "khtmlpart" ); // ### frame name
	m_pcHTMLPart->browserExtension()->SetCallback( this );

	AddChild( pcFrame );
	
	KHTMLView* pcView = m_pcHTMLPart->view();

	pcView->SetResizeMask( CF_FOLLOW_ALL );

	pcView->SetFrame( pcTopView->GetBounds() );

	pcView->MakeFocus();

	pcView->SetMsgTarget( Messenger( this ) );

	pcView->show();
	
	m_pcURLView->SetTarget( this );
	m_pcToolBar->SetTarget( Messenger( this ) );
	
	m_pcToolBar->EnableButton( BI_BACK, false );
	m_pcToolBar->EnableButton( BI_FORWARD, false );
	m_pcToolBar->EnableButton( BI_COPY, false );
	
	char*	pzHome = getenv( "HOME" );

	if ( bLoadPos && NULL != pzHome ) {
		FILE*	hFile;
		char	zPath[ 256 ];
				
		strcpy( zPath, pzHome );
		strcat( zPath, "/config/abrowse.cfg" );
	
		hFile = fopen( zPath, "rb" );
	
		if ( NULL != hFile ) {
			Rect cNewFrame;
					
			fread( &cNewFrame, sizeof( cNewFrame ), 1, hFile );
			fclose( hFile );
			SetFrame( cNewFrame );
		}
	}
	if ( pcHistory != NULL ) {
		for ( uint i = 0 ; i < pcHistory->size() ; ++i ) {
			HistoryEntry* pcEntry = new HistoryEntry;
	//		*pcEntry = *(*pcHistory)[i];
			pcEntry->m_cURL = (*pcHistory)[i]->m_cURL;
			pcEntry->m_cState = (*pcHistory)[i]->m_cState;
			m_cHistory.push_back( pcEntry );
		}
		m_nCurHistoryPos = m_cHistory.size() - 1;
		m_pcToolBar->EnableButton( BI_BACK, true );
	}
//	AddTimer( this, 1, 100000, false );
	GlobalMutex::PopLooper();
//	GlobalMutex::Unlock();
#endif
}

//----------------------------------------------------------------------------
// NAME:
// DESC:
// NOTE:
// SEE ALSO:
//----------------------------------------------------------------------------

BrowserWindow::~BrowserWindow()
{
	atomic_add( &s_nWndCount, -1 );

//	Lock();
//	BAutolock al( GlobalMutex::GetMutex() );
	m_pcHTMLPart->closeURL();
	delete m_pcHTMLPart;
//	Unlock();

}
#if 0
HistoryEntry* BrowserWindow::HistoryCurrent()
{
	if ( m_nCurHistoryPos >= 0 && m_nCurHistoryPos < int(m_cHistory.size()) ) {
		return( m_cHistory[m_nCurHistoryPos] );
	} else {
		return( NULL );
	}
}

void BrowserWindow::UpdateHistory()
{	
	HistoryEntry* pcCurrent = HistoryCurrent();
	if ( pcCurrent == NULL ) {
		pcCurrent = new HistoryEntry;
		m_nCurHistoryPos = m_cHistory.size();
		m_cHistory.push_back( pcCurrent );
	} else {
		pcCurrent->m_cState.MakeEmpty();
	}
	GlobalMutex::PushLooper( this );
	m_pcHTMLPart->browserExtension()->saveState( &pcCurrent->m_cState );
	pcCurrent->m_cURL = m_pcHTMLPart->url().prettyURL().utf8().data(); // m_pcURLView->GetBuffer()[0];
	GlobalMutex::PopLooper();
}


void BrowserWindow::AddHistoryEntry()
{
	if ( m_cHistory.empty() == false &&
		m_nCurHistoryPos == int(m_cHistory.size()) &&
		m_cHistory[m_nCurHistoryPos-1]->m_cURL == m_pcHTMLPart->url().prettyURL().utf8().data() /*m_pcURLView->GetBuffer()[0].c_str()*/ )
	{
		return;
	}
	m_nCurHistoryPos++;
	for ( uint i = m_nCurHistoryPos ; i < m_cHistory.size() ; ++i ) {
		delete m_cHistory[i];
	}
	if ( m_nCurHistoryPos > 0 ) {
		m_cHistory.resize( m_nCurHistoryPos );
	}
	HistoryEntry* pcEntry = new HistoryEntry;
	m_cHistory.push_back( pcEntry );
	m_pcToolBar->EnableButton( BI_FORWARD, false );
	m_pcToolBar->EnableButton( BI_BACK, true );
}

void BrowserWindow::GoHistory( int nStep )
{

//	if ( nStep != 0 ) {
//	UpdateHistory();
//	}
	m_nCurHistoryPos += nStep;
	HistoryEntry* pcCurrent = HistoryCurrent();
	if ( pcCurrent != NULL ) {
		GlobalMutex::PushLooper( this );
		m_pcHTMLPart->browserExtension()->restoreState( pcCurrent->m_cState );
		m_pcURLView->Set( pcCurrent->m_cURL.c_str(), false );
		GlobalMutex::PopLooper();
	}
	if ( m_nCurHistoryPos == 0 ) {
	m_pcToolBar->EnableButton( BI_BACK, false );
	} else {
	m_pcToolBar->EnableButton( BI_BACK, true );
	}
	if ( m_nCurHistoryPos >= int(m_cHistory.size()) - 1 ) {
	m_pcToolBar->EnableButton( BI_FORWARD, false );
	} else {
	m_pcToolBar->EnableButton( BI_FORWARD, true );
	}
}
#endif

void dump_message( BMessage* pcMsg )
{
	pcMsg->PrintToStream();
#if 0
	int nCount = pcMsg->CountNames();

	for ( int i = 0 ; i < nCount ; ++i ) {
	int nType;
	int nCount;
	
	pcMsg->GetNameInfo( pcMsg->GetName( i ).c_str(), &nType, &nCount );
	switch( nType )
	{
		case T_STRING:
		{
		std::string cStr;
		pcMsg->FindString( pcMsg->GetName( i ).c_str(), &cStr );
		printf( "%s: '%s'\n", pcMsg->GetName( i ).c_str(), cStr.c_str() );
		break;
		}
		default:
		printf( "%s: (Type=%d)\n", pcMsg->GetName( i ).c_str(), nType );
		break;
		
	}
	}
#endif
}

void BrowserWindow::OpenURL( const std::string& cURL, const KParts::URLArgs& cArgs )
{
//	GlobalMutex::Lock();
	
//	m_pcURLView->Set( cURL.c_str(), false );
	m_pcURLView->SetText( cURL.c_str() );
	m_pcHTMLPart->browserExtension()->setURLArgs( cArgs );

	m_pcHTMLPart->openURL( cURL.c_str() );
//	GlobalMutex::Unlock();
	
}

void BrowserWindow::OpenURLRequest( const std::string &cURL, const KParts::URLArgs& cURLArgs )
{
//	UpdateHistory();
//	AddHistoryEntry();
	OpenURL( cURL, cURLArgs );
//	UpdateHistory();
}

void BrowserWindow::OpenURLNotify()
{
#if 0
	UpdateHistory();
#endif
}

void BrowserWindow::Completed()
{
#if 0
	UpdateHistory();
#endif
}

void BrowserWindow::SetLocationBarURL( const std::string &/*cURL*/ )
{
//	m_pcURLView->Set( m_pcHTMLPart->url().prettyURL().utf8().data(), false );
	m_pcURLView->SetText( m_pcHTMLPart->url().prettyURL().utf8().data() );
//??	m_pcURLView->SetText( cURL.c_str() );
//	m_pcURLView->Set( cURL.c_str(), false );
}

void BrowserWindow::CreateNewWindow( const std::string& cURL, const KParts::URLArgs& cURLArgs )
{
	BRect cFrame( 0, 0, 799, 599 );

	cFrame.OffsetBy( get_window_pos() );

	BrowserWindow* pcNewWindow = new BrowserWindow( cFrame, false );
	if ( cURL.empty() == false ) {
		pcNewWindow->OpenURL( cURL, cURLArgs );
	}

	pcNewWindow->Show();
}

KParts::ReadOnlyPart* BrowserWindow::CreateNewWindow( const std::string& cURL, const KParts::URLArgs& cURLArgs, const KParts::WindowArgs& sWndArgs )
{
	BRect cFrame( 0, 0, 799, 599 );

	BPoint cPos;
	if ( sWndArgs.x == -1 || sWndArgs.y == -1 ) {
		cPos = get_window_pos();
	}
	if ( sWndArgs.x != -1 ) {
		cPos.x = sWndArgs.x;
	}
	if ( sWndArgs.y != -1 ) {
		cPos.y  = sWndArgs.y;
	}
	if ( sWndArgs.width != -1 ) {
		cFrame.right   = cFrame.left + sWndArgs.width - 1;
	}
	if ( sWndArgs.height != -1 ) {
		cFrame.bottom  = cFrame.top + sWndArgs.height - 1;
	}
	cFrame.OffsetBy( cPos );
		
	BrowserWindow* pcNewWindow = new BrowserWindow( cFrame, false );
	if ( cURL.empty() == false ) {
		pcNewWindow->OpenURL( cURL, cURLArgs );

	}

	pcNewWindow->Show();
	return( pcNewWindow->GetPart() );
}

void BrowserWindow::PopupMenu( KParts::BrowserExtension* /*pcSource*/, const std::string& cURL )
{
#if 0
	m_pcPopupMenu	 = new os::Menu( Rect( 0, 0, 10, 10 ), "popup", ITEMS_IN_COLUMN );
//	Menu* pcSpeedMenu = new os::Menu( Rect( 0, 0, 10, 10 ), "Speed", ITEMS_IN_COLUMN );
  
//	m_pcPopupMenu->AddItem( pcSpeedMenu );

	os::Message* pcMsg;
	m_pcPopupMenu->AddItem( new os::MenuItem( "New window", new os::Message( ID_NEW_WINDOW ) ) );
	m_pcPopupMenu->AddItem( new os::MenuItem( "Reload", new os::Message( ID_RELOAD ) ) );
	
	if ( cURL.empty() == false ) {
	m_pcPopupMenu->AddItem( new os::MenuSeparator() );
	pcMsg = new os::Message( ID_OPEN_LINK );
	pcMsg->AddString( "url", cURL );
	m_pcPopupMenu->AddItem( new os::MenuItem( "Open link in new window", pcMsg ) );

	
	pcMsg = new os::Message( ID_SAVE_LINK );
	pcMsg->AddString( "url", cURL );
	m_pcPopupMenu->AddItem( new os::MenuItem( "Save link as...", pcMsg ) );
	
	pcMsg = new os::Message( ID_COPY_LINK_LOCATION );
	pcMsg->AddString( "url", cURL );
	m_pcPopupMenu->AddItem( new os::MenuItem( "Copy link location", pcMsg ) );
	}
//	m_pcPopupMenu->AddItem( new os::MenuItem( "View document source", new os::Message( ID_VIEW_DOC_SRC ) ) );
//	pcMsg = new os::Message( ID_VIEW_FRAME_SRC );
//	pcMsg->AddPointer( "browser", pcSource );
//	m_pcPopupMenu->AddItem( new os::MenuItem( "View frame source",	new os::Message( ID_VIEW_FRAME_SRC ) ) );
	m_pcPopupMenu->AddItem( new os::MenuSeparator() );
	m_pcPopupMenu->AddItem( new os::MenuItem( "Quit", new os::Message( M_QUIT ) ) );

	m_pcPopupMenu->SetTargetForItems( this );

	os::Point cMousePos;
	m_pcStatusBar->GetMouse( &cMousePos, NULL );
	m_pcPopupMenu->Open( m_pcStatusBar->ConvertToScreen( cMousePos ) );	
#endif

}

void BrowserWindow::LoadingProgress( int /*percent*/ )
{
}
void BrowserWindow::SpeedProgress( int /*bytesPerSecond*/ )
{
}
void BrowserWindow::InfoMessage( const std::string& cString )
{
	m_pcStatusBar->SetStatus( cString );
}

void BrowserWindow::SetWindowCaption( KParts::ReadOnlyPart* pcPart, const std::string& cCaption )
{
	if ( pcPart == m_pcHTMLPart ) {
		BString str = cCaption.c_str();
		str += " - ABrowse";
		SetTitle( str.String() );
	}
}

void BrowserWindow::SetStatusBarText( const std::string& cString )
{
	m_pcStatusBar->SetStatus( cString );
}


void BrowserWindow::SelectionInfo( const std::string& cText )
{
#if 0
	m_cSelectedText = cText;
	m_pcToolBar->EnableButton( BI_COPY, m_cSelectedText.empty() == false );
#endif
}
	
bool BrowserWindow::BeginDownload( KParts::DownloadStream* psStream )
{
	return false;
#if 0
	DownloadNode* psNode = new DownloadNode;

	psNode->m_nStartTime	 = get_system_time();
	psNode->m_cURL		   = psStream->m_cURL;
	psNode->m_cPreferredName = psStream->m_cPreferredName;
	psNode->m_nContentSize   = psStream->m_nContentSize;
	
	try {
	psNode->m_pcTmpFile = new os::TempFile( "ab-" );
	} catch( ... ) {
	printf( "Error: BrowserWindow::BeginDownload() failed to create temporary file\n" );
	return( false );
	}

	Message cOkMsg( ID_START_DOWNLOAD );
	Message cCancelMsg( ID_CANCEL_DOWNLOAD );

	cOkMsg.AddPointer( "node", psNode );
	cCancelMsg.AddPointer( "node", psNode );
	
	psNode->m_psDownloadStream = psStream;
	psNode->m_pcRequester = new DownloadRequester( Rect( 250, 200, 599, 300 ),
						   "File Download",
						   psStream->m_cURL,
						   psStream->m_cPreferredName,
						   psStream->m_cMimeType,
						   psStream->m_nContentSize,
						   Messenger( this ),
						   cOkMsg,
						   cCancelMsg );
	psNode->m_pcRequester->Show();
	psStream->m_pUserData = psNode;
	return( true );
#endif
}

bool BrowserWindow::HandleDownloadData( KParts::DownloadStream* psStream, const void* pData, int nLen )
{
	return false;
#if 0
	DownloadNode* psNode = static_cast<DownloadNode*>(psStream->m_pUserData);
	if ( psNode == NULL || psNode->m_bCanceled || (psNode->m_pcTmpFile == NULL && psNode->m_pcFile == NULL) ) {
	return( false );
	}
	int nBytesWritten;
	
	if ( psNode->m_pcFile != NULL ) {
	nBytesWritten = psNode->m_pcFile->Write( pData, nLen );
	} else {
	nBytesWritten = psNode->m_pcTmpFile->Write( pData, nLen );
	}
	if ( nBytesWritten != nLen ) {
	(new os::Alert( "Error: Failed to create file!",
			os::String().Format( "Failed to write %df bytes to\n%s\n\nError: %s\n", psNode->m_cPath.c_str(), strerror(errno) ), 0,
			"Sorry!", NULL ))->Go(NULL);
	psNode->m_bCanceled = true;

	if ( psNode->m_pcProgressDlg != NULL ) {
		psNode->m_pcProgressDlg->Terminate();
		psNode->m_pcProgressDlg = NULL;
	}
	if ( psNode->m_pcRequester == NULL ) {
		delete psNode->m_pcTmpFile;
		delete psNode->m_pcFile;
		if ( psNode->m_cPath.empty() == false ) {
		unlink( psNode->m_cPath.c_str() );
		}
		delete psNode;
	} else {
		psNode->m_bDone = true;
	}
	
	} else {
	psNode->m_nBytesReceived += nLen;
	if ( psNode->m_pcProgressDlg != NULL ) {
		psNode->m_pcProgressDlg->UpdateProgress( psNode->m_nBytesReceived );
	}
	}
	return( true );
#endif
}

void BrowserWindow::DownloadFinished( KParts::DownloadStream* psStream, int nError )
{
#if 0
	DownloadNode* psNode = static_cast<DownloadNode*>(psStream->m_pUserData);
	if ( psNode == NULL || psNode->m_bCanceled || (psNode->m_pcTmpFile == NULL && psNode->m_pcFile == NULL) ) {
	return;
	}

	if ( psNode->m_pcProgressDlg != NULL ) {
	psNode->m_pcProgressDlg->Terminate();
	psNode->m_pcProgressDlg = NULL;
	}
	if ( psNode->m_pcRequester == NULL ) {
	delete psNode->m_pcTmpFile;
	delete psNode->m_pcFile;
	delete psNode;
	} else {
	psNode->m_bDone = true;
	}
#endif
}

void BrowserWindow::MessageReceived( BMessage* pcMsg )
{
	switch( pcMsg->what )
	{
	case ID_URL_CHANGED:
	{

		std::string cURL = m_pcURLView->Text();

		for ( uint i = 0 ; i <= cURL.size() ; ++i ) {
			if ( i == cURL.size() || isalnum( cURL[i] ) == false ) {
				if ( i == cURL.size() || cURL[i] != ':' ) {
					std::string cTmp = cURL;
					cURL = "http://";
					for ( uint j = 0 ; j < cTmp.size() ; ++j ) {
						if ( cTmp[j] != '/' ) {
							cURL.insert( cURL.end(), cTmp.begin() + j, cTmp.end() );
							break;
						}
					}
				}
				break;
			}
		}
		m_pcHTMLPart->view()->MakeFocus();
		OpenURL( cURL, KParts::URLArgs() );
//----------------------------------------------
#if 0
		uint32 nEvents = 0;
		pcMsg->FindInt( "events", &nEvents );
		
		if ( nEvents & os::TextView::EI_ESC_PRESSED ) {
			m_pcURLView->Set( m_pcHTMLPart->url().prettyURL().utf8().data() );
			m_pcHTMLPart->view()->MakeFocus();
		break;
		}
		if ( nEvents & os::TextView::EI_ENTER_PRESSED ) {
		std::string cURL = m_pcURLView->GetBuffer()[0];

		for ( uint i = 0 ; i <= cURL.size() ; ++i ) {
			if ( i == cURL.size() || isalnum( cURL[i] ) == false ) {
				if ( i == cURL.size() || cURL[i] != ':' ) {
					std::string cTmp = cURL;
					cURL = "http://";
					for ( uint j = 0 ; j < cTmp.size() ; ++j ) {
						if ( cTmp[j] != '/' ) {
							cURL.insert( cURL.end(), cTmp.begin() + j, cTmp.end() );
							break;
						}
					}
				}
				break;
			}
		}
		UpdateHistory();
		AddHistoryEntry();
		m_pcHTMLPart->view()->MakeFocus();
		OpenURL( cURL, KParts::URLArgs() );
		UpdateHistory();
		}
#endif
		break;
	}
	case ID_ACTIVATE_URLEDIT:
		m_pcURLView->MakeFocus( true );
#if 0
		m_pcURLView->SetCursor( -1, 0 );
#endif
		m_pcURLView->TextView()->SelectAll();
		break;
	case ID_RELOAD:
	{
		KParts::URLArgs cArgs;
		cArgs.reload = true;
		cArgs.xOffset = m_pcHTMLPart->browserExtension()->xOffset();
		cArgs.yOffset = m_pcHTMLPart->browserExtension()->yOffset();
#if 0
		OpenURL( m_pcURLView->GetBuffer()[0], cArgs );
#endif
		OpenURL( m_pcURLView->Text(), cArgs );
		break;
	}
	case ID_NEW_WINDOW:
	{
		BrowserWindow* pcNewWindow = new BrowserWindow( Frame().OffsetByCopy( 30.0f, 30.0f ), false );
		pcNewWindow->Show();
#if 0
		UpdateHistory();
		std::vector<HistoryEntry*> cHistory = m_cHistory;
		cHistory.resize( m_nCurHistoryPos + 1 );
		BrowserWindow* pcNewWindow = new BrowserWindow( GetFrame() + os::Point( 30.0f, 30.0f ), &cHistory, false );
		pcNewWindow->GoHistory(0);
		pcNewWindow->Show( true );
		pcNewWindow->MakeFocus( true );	
#endif
		break;
	}
	case ID_OPEN_LINK:
	{
		BString cURL;
		pcMsg->FindString( "url", &cURL );
		BrowserWindow* pcNewWindow = new BrowserWindow( Frame().OffsetByCopy( 30.0f, 30.0f ), false  );
		pcNewWindow->OpenURL( cURL.String(), KParts::URLArgs() );
		pcNewWindow->Show();
#if 0
		std::string cURL;
		pcMsg->FindString( "url", &cURL );
		std::vector<HistoryEntry*> cHistory = m_cHistory;
		cHistory.resize( m_nCurHistoryPos + 1 );
		BrowserWindow* pcNewWindow = new BrowserWindow( GetFrame() + os::Point( 30.0f, 30.0f ), &cHistory, false );
		pcNewWindow->OpenURL( cURL, KParts::URLArgs() );
		pcNewWindow->AddHistoryEntry();
		pcNewWindow->Show( true );
		pcNewWindow->MakeFocus( true );	
#endif
		break;
	}
	case ID_SAVE_LINK:
	{
		BString cURL;
		pcMsg->FindString( "url", &cURL );

		m_pcHTMLPart->browserExtension()->SaveURL( cURL.String(), "Save link as..." );
		break;
	}
	case ID_COPY_LINK_LOCATION:
	{
#if 0
		std::string cURL;
		pcMsg->FindString( "url", &cURL );

		os::Clipboard cClipboard;
		cClipboard.Lock();
		cClipboard.Clear();
		os::Message* pcData = cClipboard.GetData();
		pcData->AddString( "text/plain", cURL );
		cClipboard.Commit();
		cClipboard.Unlock();
#endif
		break;
	}
	case ID_COPY:
	{
#if 0
		if ( m_cSelectedText.empty() ) {
			break;
		}
		os::Clipboard cClipboard;
		cClipboard.Lock();
		cClipboard.Clear();
		os::Message* pcData = cClipboard.GetData();
		pcData->AddString( "text/plain", m_cSelectedText );
		cClipboard.Commit();
		cClipboard.Unlock();
#endif
		break;
	}
	case ID_FIND:
#if 0
		if ( m_pcSearchDialog == NULL ) {
		m_pcHTMLPart->findTextBegin();
		m_pcSearchDialog = new FindDialog( os::Messenger(this), os::Message(ID_DO_SEARCH), os::Message(ID_SEARCH_CLOSED) );
		m_pcSearchDialog->Show();
		m_pcSearchDialog->MakeFocus();
		}
#endif
		break;
	case ID_DO_SEARCH:
	{
#if 0
		bool bFromTop = false;
		pcMsg->FindBool( "from_top", &bFromTop );
		pcMsg->FindBool( "case_sensitive", &m_bSearchCaseSensitive );
		pcMsg->FindString( "string", &m_cLastSearchStr );

		KHTMLView* pcActiveView = dynamic_cast<KHTMLView*>(GetFocusChild());
		
		KHTMLPart* pcPart = m_pcHTMLPart;
		if ( pcActiveView != NULL ) {
		pcPart = pcActiveView->part();
		}
		if ( bFromTop ) {
		pcPart->findTextBegin();
		}
		pcPart->findTextNext( m_cLastSearchStr.c_str(), true, m_bSearchCaseSensitive );
		break;
#endif
	}
	case ID_REPEAT_FIND:
#if 0
		if ( m_cLastSearchStr.empty() == false ) {
		KHTMLView* pcActiveView = dynamic_cast<KHTMLView*>(GetFocusChild());
		
		KHTMLPart* pcPart = m_pcHTMLPart;
		if ( pcActiveView != NULL ) {
			pcPart = pcActiveView->part();
		}		
		pcPart->findTextNext( m_cLastSearchStr.c_str(), true, m_bSearchCaseSensitive );
		} else if ( m_pcSearchDialog == NULL ) {
		m_pcHTMLPart->findTextBegin();
		m_pcSearchDialog = new FindDialog( os::Messenger(this), os::Message(ID_DO_SEARCH), os::Message(ID_SEARCH_CLOSED) );
		m_pcSearchDialog->Show();
		m_pcSearchDialog->MakeFocus();
		}
		break;
	case ID_SEARCH_CLOSED:
		m_pcSearchDialog = NULL;
		break;
#endif
	case ID_GOHOME:
		OpenURL( "http://www.openbeos.org/", KParts::URLArgs() );
#if 0
		UpdateHistory();
		AddHistoryEntry();
		OpenURL( "http://www.atheos.cx/", KParts::URLArgs() );
		UpdateHistory();
#endif
		break;
#if 0
	case ID_PREV_URL:
		GoHistory( -1 );
		break;
	case ID_NEXT_URL:
		GoHistory( 1 );
		break;
#endif
	case ID_START_DOWNLOAD:
	{
#if 0
		DownloadNode* psNode = NULL;
		std::string   cPath;
		pcMsg->FindPointer( "node", (void**) &psNode );
		pcMsg->FindString( "path", &cPath );

		
		if ( psNode == NULL ) {
		break; // Should never happen
		}
		psNode->m_cPath = cPath;
		const std::string cDstDir = os::Path( cPath.c_str() ).GetDir();

		try {
		os::FSNode cDirNode( cDstDir );
		if ( cDirNode.GetDev() == psNode->m_pcTmpFile->GetDev() ) {
			rename( psNode->m_pcTmpFile->GetPath().c_str(), cPath.c_str() );
			psNode->m_pcTmpFile->Detatch(); // Make sure the destructor don't do anything silly.
		} else {
			psNode->m_pcFile = new os::File( cPath, O_WRONLY | O_CREAT );
			psNode->m_pcTmpFile->Seek( 0, SEEK_SET );
			char anBuffer[32*1024];
			for (;;) {
			int nLen = psNode->m_pcTmpFile->Read( anBuffer, sizeof(anBuffer) );
			if ( nLen < 0 ) {
				throw os::errno_exception( "Failed to copy temporary file" );
			}
			if ( psNode->m_pcFile->Write( anBuffer, nLen ) != nLen ) {
				throw os::errno_exception( "Failed to copy temporary file" );
			}
			if ( nLen < int(sizeof(anBuffer)) ) {
				break;
			}
			}
			delete psNode->m_pcTmpFile;
			psNode->m_pcTmpFile = NULL;
		}
		} catch(...) {
		(new os::Alert( "Error: Failed to create file!",
				os::String().Format( "Failed to create '%s'", cPath.c_str() ), 0,
				"Sorry!", NULL ))->Go(NULL);
		}
		
		psNode->m_pcRequester = NULL;

		if ( psNode->m_bDone == false ) {
		Message cCancelMsg( ID_CANCEL_DOWNLOAD );
		cCancelMsg.AddPointer( "node", psNode );
		
		psNode->m_pcProgressDlg = new DownloadProgress( os::Rect( 100, 100, 449, 349 ), os::String().Format( "Download: %s", cPath.c_str() ),
								psNode->m_cURL,
								cPath,
								psNode->m_nContentSize,
								psNode->m_nStartTime,
								os::Messenger( this ),
								cCancelMsg );

		psNode->m_pcProgressDlg->Show();
		} else {
		delete psNode->m_pcTmpFile;
		delete psNode->m_pcFile;
		delete psNode;
		}
#endif
		break;
	}
	case ID_CANCEL_DOWNLOAD:
	{
#if 0
		DownloadNode* psNode = NULL;
		pcMsg->FindPointer( "node", (void**) &psNode );
		
		if ( psNode == NULL ) {
		break; // Should never happen
		}
		if ( psNode->m_pcProgressDlg != NULL ) {
		psNode->m_pcProgressDlg->Terminate();
		psNode->m_pcProgressDlg = NULL;
		}
		if ( psNode->m_cPath.empty() == false ) {
		unlink( psNode->m_cPath.c_str() );
		}
		if ( psNode->m_bDone == false ) {
		psNode->m_pcRequester = NULL;
		psNode->m_bCanceled = true;
		delete psNode->m_pcTmpFile;
		psNode->m_pcTmpFile = NULL;
		delete psNode->m_pcFile;
		psNode->m_pcFile = NULL;
		} else {
		delete psNode->m_pcTmpFile;
		delete psNode->m_pcFile;
		delete psNode;		
		}
#endif
		break;
	}
	default:
		BWindow::MessageReceived( pcMsg );
		break;
	}
}

//----------------------------------------------------------------------------
// NAME:
// DESC:
// NOTE:
// SEE ALSO:
//----------------------------------------------------------------------------

bool BrowserWindow::QuitRequested( void )
{
	if ( s_nWndCount == 1 ) {
		printf("Window closed\n" );
#warning Does not work!
		be_app_messenger.SendMessage( B_QUIT_REQUESTED );
	}
#if 0
	if ( m_pcSearchDialog != NULL ) {
		if ( m_pcSearchDialog->SafeLock() >= 0 ) {
			m_pcSearchDialog->PostMessage( M_QUIT );
			m_pcSearchDialog->Unlock();
			m_pcSearchDialog = NULL;
		}
	}
	if ( s_nWndCount == 1 ) {
		if ( m_bSaveSize ) {
			char*	pzHome = getenv( "HOME" );
	
			if ( NULL != pzHome ) {
				FILE*	hFile;
				char	zPath[ 256 ];
					
				strcpy( zPath, pzHome );
				strcat( zPath, "/config/abrowse.cfg" );
		
				hFile = fopen( zPath, "wb" );
		
				if ( hFile != NULL ) {
					Rect cFrame = GetFrame();
					fwrite( &cFrame, sizeof( cFrame ), 1, hFile );
					fclose( hFile );
				}
			}
		}
		Application::GetInstance()->PostMessage( M_QUIT );
	}
#endif
	return( true );
}






void print_msg( QtMsgType, const char * /*pzMsg*/ )
{
//	printf( "QWarning: %s\n", pzMsg );
}


int main()
{
	MyApp* pcMyApp;

	qInstallMsgHandler( print_msg );

	pcMyApp = new MyApp( "application/x-vnd.Procton-beabrowse" );

	pcMyApp->Run();
	return( 0 ); 
}
