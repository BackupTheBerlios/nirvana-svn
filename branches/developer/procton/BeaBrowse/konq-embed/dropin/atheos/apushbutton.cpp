
#include <qpushbutton.h>

#include <Button.h>
#include <Message.h>

#include <kdebug.h>

enum { ID_SELECTED };

class QPushButton::Private
{
public:
	BButton* m_pcButton;
};




QPushButton::QPushButton( QWidget* pcParent ) : QWidget( pcParent, "push_button" )
{
	kdDebug(10) << "QPushButton::" << __FUNCTION__ << "1()" << endl;
	d = new Private;
	d->m_pcButton = new BButton( Bounds(), "push_button", "Button", new BMessage( ID_SELECTED ), B_FOLLOW_ALL, B_WILL_DRAW | B_NAVIGABLE );
	d->m_pcButton->SetTarget( this );
	AddChild( d->m_pcButton );
}

QPushButton::QPushButton( const QString &text, QWidget* pcParent, const char* /*name*/ ) : QWidget( pcParent, "push_button" )
{
	kdDebug(10) << "QPushButton::" << __FUNCTION__ << "2()" << endl;
	d = new Private;
	d->m_pcButton = new BButton( Bounds(), "push_button", text.ascii(), new BMessage( ID_SELECTED ), B_FOLLOW_ALL, B_WILL_DRAW | B_NAVIGABLE); 
	d->m_pcButton->SetTarget( this );
	AddChild( d->m_pcButton );
}

QPushButton::~QPushButton()
{
	kdDebug(10) << "QPushButton::" << __FUNCTION__ << "()" << endl;
	delete d;
}

void QPushButton::AllAttached()
{
	kdDebug(10) << "QPushButton::" << __FUNCTION__ << "()" << endl;
	d->m_pcButton->SetTarget( this );
	QWidget::AllAttached();
}

void QPushButton::MessageReceived(BMessage *pcMessage)
{
	kdDebug(10) << "QPushButton::" << __FUNCTION__ << "()" << endl;
	switch( pcMessage->what )
	{
	case ID_SELECTED:
		kdDebug(10) << "\tID_SELECTED" << endl;		
		emit clicked();
		break;
	default:
		QWidget::MessageReceived( pcMessage );
		break;
	}
}

void QPushButton::setText( const QString& cString )
{
	kdDebug(10) << "QPushButton::" << __FUNCTION__ << "()" << endl;
	d->m_pcButton->SetLabel( cString.ascii() );
}

QSize QPushButton::sizeHint() const
{
	kdDebug(10) << "QPushButton::" << __FUNCTION__ << "()" << endl;
	float width, height;
	d->m_pcButton->GetPreferredSize( &width, &height );
	return( QSize( int(width) + 1, int(height) + 1 ) );
}

#include "apushbutton.moc"
