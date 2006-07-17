#if 0
#ifndef __F_DOWNLOADDIALOG_H__
#define __F_DOWNLOADDIALOG_H__

#include <Window.h>
//#include <gui/layoutview.h>
#include <View.h>

#include <Message.h>
#include <Messenger.h>

class BButton;
class BStringView;
class BProgressBar;
//class FileRequester;
class BFilePanel;

class StatusView;

//class DownloadRequesterView : public os::LayoutView
class DownloadRequesterView : public BView
{
public:
    DownloadRequesterView( BRect      cFrame,
			   const std::string&   cURL,
			   const std::string&   cPreferredName,
			   const std::string&   cType,
			   off_t	        nContentSize,
			   const BMessenger& cMsgTarget,
			   const BMessage&   cOkMsg,
			   const BMessage&   cCancelMessage);
    ~DownloadRequesterView();

    virtual void MessageRreceived( BMessage* pcMessage );
    virtual void AllAttached();
    
private:
    static std::string s_cDownloadPath;

    BButton*	m_pcOkBut;
    BButton* m_pcCancelBut;

    BMessenger m_cMsgTarget;
    BMessage	  m_cOkMsg;
    BMessage   m_cCancelMsg;
    BMessage*  m_pcTermMsg;
    
    std::string	       m_cPreferredName;
    BFilePanel* m_pcFilePanel;
};


class DownloadRequester : public BWindow
{
public:
    DownloadRequester( BRect      cFrame,
		       const std::string&   cTitle,
		       const std::string&   cURL,
		       const std::string&   cPreferredName,
		       const std::string&   cType,
		       off_t		    nContentSize,
		       const BMessenger& cMsgTarget,
		       const BMessage&   cOkMsg,
		       const BMessage&   cCancelMessage );
    ~DownloadRequester();
private:
    DownloadRequesterView* m_pcTopView;
};


//class DownloadProgressView : public os::LayoutView
class DownloadProgressView : public BView
{
public:
    DownloadProgressView( BRect cFrame,
			  const std::string& cURL,
			  const std::string& cPath,
			  off_t nTotalSize,
			  bigtime_t nStartTime,
			  const BMessenger& cMsgTarget,
			  const BMessage& cCancelMessage );
    ~DownloadProgressView();
    virtual void MessageReceived( BMessage* pcMessage );
    virtual void AllAttached();
    
    void UpdateProgress( off_t nBytesReceived );

    void ClearTermMsg() { m_pcTermMsg = NULL; }
private:
    BProgressBar* m_pcProgressBar;
//    BStatusView*	     m_pcStatusView;
    BButton*	     m_pcCancelBut;

    
    bigtime_t	  m_nStartTime;
    off_t 	  m_nTotalSize;
    
    BMessenger m_cMsgTarget;
    BMessage   m_cCancelMsg;
    BMessage*  m_pcTermMsg;
};


class DownloadProgress : public BWindow
{
public:
    DownloadProgress( BRect cFrame,
		      const std::string& cTitle,
		      const std::string& cURL,
		      const std::string& cPath,
		      off_t nTotalSize,
		      bigtime_t nStartTime,
		      const BMessenger& cMsgTarget,
		      const BMessage& cCancelMessage );
    ~DownloadProgress();

    void UpdateProgress( off_t nBytesReceived );
    void Terminate();
private:
    DownloadProgressView* m_pcTopView;
    std::string		  m_cTitle;
    off_t		  m_nTotalSize;
};



#endif // __F_DOWNLOADDIALOG_H__
#endif //0
