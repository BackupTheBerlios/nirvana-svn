#if 0
#ifndef __F_FILEDIALOG_H__
#define __F_FILEDIALOG_H__

#include <Window.h>
//#include <gui/layoutview.h>

#include <Message.h>
#include <Messenger.h>

class BButton;
class BTextView;
class BCheckBox;


//class FindDialogView : public os::LayoutView
class FindDialogView : public BView
{
public:
    FindDialogView( BRect cFrame, const BMessenger& cMsgTarget, const BMessage& cSearchMsg, const BMessage& cCloseMsg );
    ~FindDialogView();

    virtual void AllAttached();
    virtual void KeyDown( const char* pzRawString, int32 numBytes );
    virtual void MessageReceived( BMessage* pcMessage );
    
private:
    BTextView* m_pcSearchStr;
    BButton*	  m_pcSearchBut;
    BButton*   m_pcContinueBut;
    BButton*   m_pcCloseBut;
    BCheckBox* m_pcCaseSensCB;
    
    BMessenger m_cMsgTarget;
    BMessage   m_cSearchMsg;
    BMessage   m_cCloseMsg;
};


class FindDialog : public BWindow
{
public:
    FindDialog( const BMessenger& cMsgTarget, const BMessage& cSearchMsg, const BMessage& cCloseMsg );
    ~FindDialog();
private:
    FindDialogView* m_pcTopView;
};



#endif // __F_FILEDIALOG_H__

#endif //0
