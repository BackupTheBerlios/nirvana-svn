
#include <qlineedit.h>

#include <TextControl.h>
#include <Message.h>

#include <kdebug.h>
#include <stdio.h>

const int INVOKATION_ID = 'invI';
const int MODIFICATION_ID = 'modI';

class QLineEdit::Private
{
public:
	BTextControl* m_pcTextControl;
};

QLineEdit::QLineEdit( QWidget* pcParent ) : QWidget( pcParent, "text_view_cont" )
{
	kdDebug(10) << "QLineEdit::" << __FUNCTION__ << "()" << endl;
	d = new Private;
	d->m_pcTextControl = new BTextControl( Bounds(), "text_view", "", "", new BMessage( INVOKATION_ID ), B_FOLLOW_ALL, B_WILL_DRAW | B_NAVIGABLE );
	d->m_pcTextControl->SetModificationMessage( new BMessage( MODIFICATION_ID ) );
	d->m_pcTextControl->SetDivider( 0 );
	
	AddChild( d->m_pcTextControl );
	d->m_pcTextControl->SetTarget( this );
}

QLineEdit::~QLineEdit()
{
	kdDebug(10) << "QLineEdit::" << __FUNCTION__ << "()" << endl;
	delete d;
}

bool QLineEdit::frame() const
{
	kdDebug(10) << "QLineEdit::" << __FUNCTION__ << "()" << endl;
	return( true );
}

void QLineEdit::selectAll()
{
	kdDebug(10) << "QLineEdit::" << __FUNCTION__ << "()" << endl;
	d->m_pcTextControl->TextView()->SelectAll();
}

void QLineEdit::setMaxLength( int nLength )
{
	kdDebug(10) << "QLineEdit::" << __FUNCTION__ << "()" << endl;
	d->m_pcTextControl->TextView()->SetMaxBytes( nLength );
}

void QLineEdit::setReadOnly( bool bFlag )
{
	kdDebug(10) << "QLineEdit::" << __FUNCTION__ << "()" << endl;
	d->m_pcTextControl->SetEnabled( !bFlag );
}

void QLineEdit::setText( const QString& cText )
{
	kdDebug(10) << "QLineEdit::" << __FUNCTION__ << "()" << endl;
	d->m_pcTextControl->SetText( cText.utf8().data() );
}

bool QLineEdit::isReadOnly() const
{
	kdDebug(10) << "QLineEdit::" << __FUNCTION__ << "()" << endl;
	return( !d->m_pcTextControl->IsEnabled() );
}

void QLineEdit::setEchoMode( EchoMode eMode )
{
	kdDebug(10) << "QLineEdit::" << __FUNCTION__ << "()" << endl;
	d->m_pcTextControl->SetEnabled( eMode != Normal );
}

QSize QLineEdit::sizeHint() const
{
	kdDebug(10) << "QLineEdit::" << __FUNCTION__ << "()" << endl;
	float width, height;
	d->m_pcTextControl->GetPreferredSize( &width, &height );
	return( QSize( int(width) + 1, int(height) + 1 ) );
}

void QLineEdit::SetMinPreferredSize( int nWidthChars )
{
	kdDebug(10) << "QLineEdit::" << __FUNCTION__ << "()" << endl;
	printf( "\tWarning: %s not implemented\n", __FUNCTION__ );
#if 0
	d->m_pcTextControl->SetMinPreferredSize( nWidthChars, 1 );
	d->m_pcTextControl->SetMaxPreferredSize( nWidthChars, 1 );
#endif
#warning SetMinPreferredSize not implemented
}

void QLineEdit::AllAttached()
{
	kdDebug(10) << "QLineEdit::" << __FUNCTION__ << "()" << endl;
	QWidget::AllAttached();
}

void QLineEdit::MessageReceived( BMessage* pcMessage )
{
	kdDebug(10) << "QLineEdit::" << __FUNCTION__ << "()" << endl;
	switch( pcMessage->what )
	{
	case INVOKATION_ID:
	{
		emit returnPressed();
		break;
	}
	case MODIFICATION_ID:
		emit textChanged( QString::fromUtf8( d->m_pcTextControl->Text() ) );
//		emit textChanged( QString::fromLatin1( d->m_pcTextControl->Text() ) );
		break;
	default:
		QWidget::MessageReceived( pcMessage );
		break;
	}
}

#include "alineedit.moc"
