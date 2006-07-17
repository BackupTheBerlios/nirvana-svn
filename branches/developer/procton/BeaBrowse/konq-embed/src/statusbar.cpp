#include "statusbar.h"
#include <StringView.h>

StatusBar::StatusBar( BRect cFrame, const std::string& cTitle )
	:BBox( cFrame,  "status_view", B_FOLLOW_BOTTOM | B_FOLLOW_LEFT_RIGHT )
{
	mStatus = new BStringView( Bounds().InsetByCopy(2,2), "status_view", cTitle.c_str(), B_FOLLOW_ALL_SIDES );
}

void StatusBar::AttachedToWindow()
{
	AddChild( mStatus );
}

void StatusBar::SetStatus( const std::string& cText )
{
	if( LockLooper() ) {
		mStatus->SetText( cText.c_str() );
		UnlockLooper();
	}
}
