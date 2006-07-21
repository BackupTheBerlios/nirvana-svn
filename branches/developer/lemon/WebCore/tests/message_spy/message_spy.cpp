
#include <Application.h>
#include <ScrollView.h>
#include <TextView.h>
#include <Screen.h>
#include <InterfaceKit.h>
#include <SupportKit.h>

#include <string.h>
#include <stdio.h>
#include <malloc.h>

const char *APP_SIGNATURE = "application/x-vnd.Be-MessageWorld";

const uint32 WINDOW_REGISTRY_ADD	= 'WRad';
const uint32 WINDOW_REGISTRY_SUB	= 'WRsb';
const uint32 WINDOW_REGISTRY_ADDED	= 'WRdd';

const uint32 MENU_FILE_NEW		= 'MFnw';
const uint32 MENU_FILE_OPEN		= 'MFop';
const uint32 MENU_FILE_CLOSE		= 'MFcl';
const uint32 MENU_FILE_SAVE		= 'MFsv';
const uint32 MENU_FILE_SAVEAS		= 'MFsa';
const uint32 MENU_FILE_PAGESETUP	= 'MFps';
const uint32 MENU_FILE_PRINT		= 'MFpr';
const uint32 MENU_FILE_QUIT		= 'MFqu';
const uint32 MENU_OPT_HELLO		= 'MOhl';

class HelloWindow : public BWindow {
	public:
		HelloWindow(BRect frame);
		~HelloWindow();
		virtual bool QuitRequested();
		virtual void MessageReceived(BMessage *message);
		virtual void DispatchMessage(BMessage *message, BHandler *target);
		BTextView *textview;
	
	private:
		void Register(bool need_id);
		void Unregister(void);
		
		BMenuBar *menubar;
};

HelloWindow::HelloWindow(BRect frame) :
    BWindow(frame, "Untitled ", B_TITLED_WINDOW, B_NOT_ZOOMABLE)
{
    BRect r = Bounds();
    
    menubar = new BMenuBar(r, "menu_bar");

    BMenu *menu;
    menu = new BMenu("File");
    menu->AddItem(new BMenuItem("New", new BMessage(MENU_FILE_NEW), 'N'));
    menu->AddItem(new BMenuItem("Open" B_UTF8_ELLIPSIS, new BMessage(MENU_FILE_OPEN), 'O'));
    menu->AddItem(new BMenuItem("Close", new BMessage(MENU_FILE_CLOSE), 'W'));
    menu->AddSeparatorItem();
    menu->AddItem(new BMenuItem("Save", new BMessage(MENU_FILE_SAVE), 'S'));
    menu->AddItem(new BMenuItem("Save as" B_UTF8_ELLIPSIS, new BMessage(MENU_FILE_SAVEAS)));
    menu->AddSeparatorItem();
    menu->AddItem(new BMenuItem("Page Setup" B_UTF8_ELLIPSIS, new BMessage(MENU_FILE_PAGESETUP)));
    menu->AddItem(new BMenuItem("Print" B_UTF8_ELLIPSIS, new BMessage(MENU_FILE_PRINT), 'P'));
    menu->AddSeparatorItem();
    menu->AddItem(new BMenuItem("Quit", new BMessage(MENU_FILE_QUIT), 'Q'));
    menubar->AddItem(menu);
    menu = new BMenu("Options");
    menu->AddItem(new BMenuItem("Hello Option", new BMessage(MENU_OPT_HELLO)));
    menubar->AddItem(menu);
    AddChild(menubar);
	
    // fix menubar & scrolls 
    r.top = menubar->Bounds().bottom+1;
    BRect fr = frame;
    r.right -= B_V_SCROLL_BAR_WIDTH ;
    r.bottom -= B_H_SCROLL_BAR_HEIGHT ; 
	
    // add textview
    textview = new BTextView(r, "textView", r, B_FOLLOW_ALL_SIDES, B_WILL_DRAW | B_NAVIGABLE);
    textview->SetStylable(true);   
    textview->MakeEditable(false);
	
    AddChild(new BScrollView("TextScroll", textview, B_FOLLOW_ALL_SIDES, 0, true, true));
	
    Register(true);
    Show();
}

HelloWindow::~HelloWindow() { Unregister(); }

void HelloWindow::MessageReceived(BMessage *message) {
    switch(message->what) {
	case WINDOW_REGISTRY_ADDED:
	    {
		char s[22];
	        int32 id = 0;
	        if (message->FindInt32("new_window_number", &id) == B_OK) {
		    sprintf(s, "Untitled %ld", id);	
		    SetTitle(s);
		}
	    }
	    break;
			
	case MENU_FILE_NEW:
	    {
		BRect r;
	        r = Frame();
	        r.OffsetBy(20,20);
	        new HelloWindow(r);
	    }
	    break;
	
	case MENU_FILE_CLOSE:
	    Quit();
	    break;
	
	case MENU_FILE_QUIT:
	    be_app->PostMessage(B_QUIT_REQUESTED);
	    break;
		
	case MENU_OPT_HELLO:
	    {
		BMenuItem *item;
		const char *s;
		bool mark;
				
		message->FindPointer("source", (void **) &item);
		if (item->IsMarked()) {
		    //s = STRING_GOODBYE;
		    mark = false;
		} else {
		    //s = STRING_HELLO;
		    mark = true;
		}
		
		item->SetMarked(mark);
		textview->Insert("Hello World\n");
		textview->Invalidate();
	    }
	    break;
		
	default:
	    BWindow::MessageReceived(message);
	    break;
    }
}

void HelloWindow::Register(bool need_id)
{
    BMessenger messenger(APP_SIGNATURE);
    BMessage message(WINDOW_REGISTRY_ADD);
    message.AddBool("need_id", need_id);
    messenger.SendMessage(&message, this);
}

void HelloWindow::Unregister(void)
{
    BMessenger messenger(APP_SIGNATURE);
    messenger.SendMessage(new BMessage(WINDOW_REGISTRY_SUB));
}

bool HelloWindow::QuitRequested() {
    return true;
}

class HelloApp : public BApplication {
    public:
	HelloApp();
	virtual void MessageReceived(BMessage *message);
	
    private:
	int32 window_count;
	int32 next_untitled_number;
};

HelloApp::HelloApp(): BApplication(APP_SIGNATURE) {
    BRect windowRect;
    windowRect.Set(50,50,349,399);
    window_count = 0;			// No windows yet
    next_untitled_number = 1;	// Next window is "Untitled 1"
    new HelloWindow(windowRect);
}

void HelloWindow::DispatchMessage(BMessage *message, BHandler *target)
{
    switch (message->what)
    {
	case B_MOUSE_DOWN:	    
	case B_KEY_DOWN:
	    {
		char keycode[] = "01234567\n";
		char buttonscode[] = "01234567\n";
		char *smods = (char *)calloc(1, 1024);
		uint32 mods = message->FindInt32("modifiers");
		uint32 key = message->FindInt32("key");
		uint32 buttons = message->FindInt32("buttons");
		if (mods & B_SHIFT_KEY) smods = strcat(smods, "SHIFT ");
		if (mods & B_CONTROL_KEY) smods = strcat(smods, "CONTROL ");
		if (mods & B_OPTION_KEY) smods = strcat(smods, "OPTION ");
		if (mods & B_COMMAND_KEY) smods = strcat(smods, "COMMAND ");
		if (message->what == B_KEY_DOWN) {
		    sprintf(keycode, "%08x\n", key);
		    smods = strcat(smods, "KEY ");
		    smods = strcat(smods, keycode);
		}
		if (message->what == B_MOUSE_DOWN) {
		    sprintf(buttonscode, "%08x\n", buttons);
		    smods = strcat(smods, "MOUSE ");
		    smods = strcat(smods, buttonscode);
		}
					
		HelloWindow::textview->Insert(smods);
		HelloWindow::textview->Invalidate();
		
		//BWindow::MessageReceived(message);
	    }
	    break;
    }
    BWindow::DispatchMessage(message, target);
}

void HelloApp::MessageReceived(BMessage *message) {
    switch(message->what) {
	case WINDOW_REGISTRY_ADD:
	    {
		bool need_id = false;
				
		if (message->FindBool("need_id", &need_id) == B_OK) {
		    if (need_id) {
			BMessage reply(WINDOW_REGISTRY_ADDED);
			reply.AddInt32("new_window_number", next_untitled_number);
			message->SendReply(&reply);
			next_untitled_number++;
		    }
		    window_count++;
		}
		break;
	    }

	case WINDOW_REGISTRY_SUB:
	    window_count--;
	    if (!window_count) {
		Quit();
	    }
	    break;
	    
	default:
	    BApplication::MessageReceived(message);
	    break;
    }
}

int main(void) {
    HelloApp theApp;		// The application object
    theApp.Run();
    return 0;
}
