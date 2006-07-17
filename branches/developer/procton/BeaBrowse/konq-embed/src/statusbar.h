#ifndef __F_STATUSBAR_H__
#define __F_STATUSBAR_H__

#include <Box.h>
#include <Font.h>
#include <string>

class BStringView;
class StatusBar : public BBox
{
public:
	StatusBar( BRect cFrame, const std::string& cTitle );
	void SetStatus( const std::string& cText );

	void AttachedToWindow();
    
private:
	font_height m_sFontHeight;
	std::string m_cStatusStr;

	BStringView *mStatus;
	
};


#endif // __F_STATUSBAR_H__
