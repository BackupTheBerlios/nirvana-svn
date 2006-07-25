
#include <AppKit.h>
#include <SupportKit.h>
#include "KWQCheckBox.h"

// We empirically determined that check boxes have these dimensions.
// It would be better to get this info from AppKit somehow.

#define TOP_MARGIN 4
#define BOTTOM_MARGIN 3
#define LEFT_MARGIN 3
#define RIGHT_MARGIN 3

#define WIDTH 12
#define HEIGHT 12

#define BASELINE_MARGIN 4

enum {
    topMargin,
    bottomMargin,
    leftMargin,
    rightMargin,
    baselineFudgeFactor,
    dimWidth,
    dimHeight
};

const int SELECTION_MSG = 'selM'; 

QCheckBox::QCheckBox(QWidget * parent, const char * name, int f)
    : QButton(parent, name,f)
      ,m_stateChanged(this, SIGNAL(stateChanged(int)))

{
    BCheckBox *box = new BCheckBox(Bounds(), name, "",
	new BMessage(SELECTION_MSG),  B_FOLLOW_ALL, B_WILL_DRAW | B_NAVIGABLE );
    check_box = box;
    setBView(check_box);
    AddChild(check_box);
}

QSize QCheckBox::sizeHint()

{
    float width, height;
    check_box->GetPreferredSize( &width, &height );
    return( QSize( int(width) + 1, int(height) + 1 ) );   
}

void QCheckBox::setChecked(bool isChecked)
{    
    //gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(getGtkWidget()), isChecked );
    check_box->SetValue(isChecked);
}

bool QCheckBox::isChecked()
{
    //return gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(getGtkWidget()) );
    return check_box->Value();
}

void QCheckBox::clicked()
{
    // Note that it's important to give the stateChanged signal before
    // the clicked signal so that the corresponding JavaScript messages
    // go in the right order. A test for this at the time of this writing
    // was the languages radio buttons and check boxes at google.com prefs.
    //m_stateChanged.call(isChecked() ? 2 : 0);
    QButton::clicked();
}

int QCheckBox::baselinePosition(int aheight)
{
    return (int) ((15.0f/20.0f)*(float) aheight);
}

QRect QCheckBox::frameGeometry()
{
    return QButton::frameGeometry();
}
    
void QCheckBox::setFrameGeometry(const QRect &r)
{
    QButton::setFrameGeometry(r);
}    
	
void QCheckBox::MessageReceived(BMessage* message)
{
    switch (message->what) {
    case SELECTION_MSG:
	clicked();
	//stateChanged.call();//(check_box->Value());
	break;
    default:
	QButton::MessageReceived(message);
	break;
    }
}
