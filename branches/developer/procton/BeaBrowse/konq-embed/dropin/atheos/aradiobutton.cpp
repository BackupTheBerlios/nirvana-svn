

#include <qradiobutton.h>

#include <RadioButton.h>
#include <Message.h>

#include <kdebug.h>

const int SELECTION_MSG = 'selM';

class QRadioButton::Private
{
public:
    BRadioButton* m_pcRadioButton;    
};

QRadioButton::QRadioButton( QWidget* pcParent ) : QWidget( pcParent, "radiobutton" )
{
	kdDebug(10) << "QRadioButton::" << __FUNCTION__ << "()" << endl;
    d = new Private;
    d->m_pcRadioButton = new BRadioButton( Bounds(), "radiobutton", "", new BMessage( SELECTION_MSG ), B_FOLLOW_ALL );

    AddChild( d->m_pcRadioButton );
    d->m_pcRadioButton->SetTarget( this );
    
}

QRadioButton::~QRadioButton()
{
	kdDebug(10) << "QRadioButton::" << __FUNCTION__ << "()" << endl;
    delete d;
}

bool QRadioButton::isChecked() const
{
	kdDebug(10) << "QRadioButton::" << __FUNCTION__ << "()" << endl;
    return( d->m_pcRadioButton->Value() );
}

void QRadioButton::setChecked( bool check )
{
	kdDebug(10) << "QRadioButton::" << __FUNCTION__ << "()" << endl;
    d->m_pcRadioButton->SetValue( check );
}

QSize QRadioButton::sizeHint() const
{
	kdDebug(10) << "QRadioButton::" << __FUNCTION__ << "()" << endl;
	float width, height;
    d->m_pcRadioButton->GetPreferredSize( &width, &height );
    return( QSize( int(width) + 1, int(height) + 1 ) );
}

void QRadioButton::MessageReceived( BMessage* pcMessage )
{
	kdDebug(10) << "QRadioButton::" << __FUNCTION__ << "()" << endl;
    switch( pcMessage->what )
    {
	case SELECTION_MSG:
	    emit clicked();
	    break;
	default:
	    QWidget::MessageReceived( pcMessage );
	    break;
    }
}

#include "aradiobutton.moc"
