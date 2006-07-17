#if 0
#ifndef __F_TOOLBAR_H__
#define __F_TOOLBAR_H__

#include <Message.h>
#include <Messenger.h>
#include <View.h>
#include <Bitmap.h>

#include <vector>

class ToolButton
{
public:
    ToolButton( const std::string& cImagePath, const BMessage& cMsg );
    ~ToolButton();

    
    
private:
    friend class ToolBar;
    
    BBitmap* m_pcImage;
    BMessage m_cMessage;
    bool	m_bIsEnabled;
};

class ToolBar : public BView
{
public:
    ToolBar( BRect cFrame, const std::string& cName );

//    virtual void	Paint( const BRect& cUpdateRect );
//    virtual void	MouseMove( const BPoint& cNewPos, int nCode, uint32 nButtons, BMessage* pcData );
//    virtual void	MouseDown( const BPoint& cPosition, uint32 nButtons );
//    virtual void	MouseUp( const BPoint& cPosition, uint32 nButtons, BMessage* pcData );
//    virtual os::Point	GetPreferredSize( bool bLargest ) const;    
    virtual void	Draw( BRect cUpdateRect );
    virtual void	MouseMoved( BPoint cNewPos, int nCode, const BMessage* pcData );
    virtual void	MouseDown( BPoint cPosition );
    virtual void	MouseUp( BPoint cPosition );
    virtual void	GetPreferredSize( float *width, float *height ) const;

    void SetTarget( const BMessenger& cTarget );    
    void EnableButton( int nIndex, bool bEnabled );
    void AddButton( const std::string& cImagePath, const BMessage& cMsg );

private:
    BRect GetButtonRect( int nIndex, bool bFull );

    BMessenger		m_cTarget;
    std::vector<ToolButton*>	m_cButtons;
    int				m_nSelectedButton;
};



#endif // __F_TOOLBAR_H__
#endif //0 
