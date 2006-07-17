
#include <qcheckbox.h>

#include <CheckBox.h>
#include <Message.h>

#include <kdebug.h>

const int SELECTION_MSG = 'selM';

class QCheckBox::Private
{
public:
	BCheckBox* m_pcCheckBox;
};

QCheckBox::QCheckBox( QWidget* pcParent ) : QWidget( pcParent, "checkbox" )
{
	kdDebug(10) << "QCheckBox::" << __FUNCTION__ << "()" << endl;
	d = new Private;
	d->m_pcCheckBox = new BCheckBox( Bounds(), "radiobutton", "", new BMessage( SELECTION_MSG ), B_FOLLOW_ALL, B_WILL_DRAW | B_NAVIGABLE );

	AddChild( d->m_pcCheckBox );
	d->m_pcCheckBox->SetTarget( this );	//Hmm... should this not be in attached to window?
}

bool QCheckBox::isChecked() const
{
	kdDebug(10) << "QCheckBox::" << __FUNCTION__ << "()" << endl;
	return( d->m_pcCheckBox->Value() );
}

void QCheckBox::setChecked( bool check )
{
	kdDebug(10) << "QCheckBox::" << __FUNCTION__ << "()" << endl;
	d->m_pcCheckBox->SetValue( check );
}

QSize QCheckBox::sizeHint() const
{
	kdDebug(10) << "QCheckBox::" << __FUNCTION__ << "()" << endl;
	float width, height;
	d->m_pcCheckBox->GetPreferredSize( &width, &height );
	return( QSize( int(width) + 1, int(height) + 1 ) );
}


void QCheckBox::MessageReceived( BMessage* pcMessage )
{
	kdDebug(10) << "QCheckBox::" << __FUNCTION__ << "()" << endl;
	switch( pcMessage->what )
	{
	case SELECTION_MSG:
		emit clicked();
		emit stateChanged( (d->m_pcCheckBox->Value()));
		break;
	default:
		QWidget::MessageReceived( pcMessage );
		break;
	}
}

void 
QCheckBox::AttachedToWindow()
{
	kdDebug(10) << "QCheckBox::" << __FUNCTION__ << "()" << endl;
}

void 
QCheckBox::AllAttached()
{
	kdDebug(10) << "QCheckBox::" << __FUNCTION__ << "()" << endl;
}

#include "acheckbox.moc"
