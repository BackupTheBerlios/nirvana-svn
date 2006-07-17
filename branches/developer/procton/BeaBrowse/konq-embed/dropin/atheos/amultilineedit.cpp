
#include <qmultilineedit.h>

#include <TextView.h>
#include <Message.h>
#include <Messenger.h>

#include <kdebug.h>
#include <stdio.h>

const int TEXT_MODIFIED = 'txtM';

class MTextView: public BTextView {
public:
	MTextView( BRect frame, const char *name, BRect textframe, uint32 resizingmode, uint32 flags)
		:BTextView( frame, name, textframe, resizingmode, flags)
		,mTarget(NULL){}
	
	~MTextView() { delete mTarget; }
	
	virtual void MessageReceived( BMessage *aMsg ) {
		kdDebug(10) << "MTextView::" << __FUNCTION__ << "()" << endl;
		switch( aMsg->what ) {
		case B_CUT:
		case B_PASTE:
			if( mTarget ) {
				mTarget->SendMessage( TEXT_MODIFIED );
			}
			break;
		}
		BTextView::MessageReceived( aMsg );	
	}
	
	virtual void KeyDown( const char *bytes, int32 numBytes) {
		kdDebug(10) << "MTextView::" << __FUNCTION__ << "()" << endl;
		if( mTarget ) {
			mTarget->SendMessage( TEXT_MODIFIED );
		}
		BTextView::KeyDown( bytes, numBytes );
	}
	
	void SetTarget( BHandler *aTarget ) { mTarget = new BMessenger(aTarget); }
	
private:
	BMessenger *mTarget;
};

class QMultiLineEdit::Private
{
public:
	MTextView* m_pcTextView;
};

QMultiLineEdit::QMultiLineEdit( QWidget* pcParent ) : QWidget( pcParent, "text_view_cont" )
{
	kdDebug(10) << "QMultiLineEdit::" << __FUNCTION__ << "()" << endl;
	d = new Private;
#warning May need a ScrollView.
	d->m_pcTextView = new MTextView( Bounds(), "text_view", Bounds(), B_FOLLOW_ALL, B_WILL_DRAW | B_NAVIGABLE | B_FRAME_EVENTS | B_PULSE_NEEDED );
	d->m_pcTextView->MakeResizable( true );
	
	AddChild( d->m_pcTextView );
}

QMultiLineEdit::~QMultiLineEdit()
{
	kdDebug(10) << "QMultiLineEdit::" << __FUNCTION__ << "()" << endl;
	delete d;
}

void QMultiLineEdit::selectAll()
{
	kdDebug(10) << "QMultiLineEdit::" << __FUNCTION__ << "()" << endl;
	d->m_pcTextView->SelectAll();
}

bool QMultiLineEdit::hasMarkedText() const
{
	kdDebug(10) << "QMultiLineEdit::" << __FUNCTION__ << "()" << endl;
	int32 start, end;
	d->m_pcTextView->GetSelection(&start, &end);
	return( start != end );
}

void QMultiLineEdit::getCursorPosition( int *line, int *col ) const
{
	kdDebug(10) << "QMultiLineEdit::" << __FUNCTION__ << "()" << endl;
	int32 start, end;
	d->m_pcTextView->GetSelection(&start, &end);
	
	*line = d->m_pcTextView->CurrentLine();
	*col = start & *line;
}

void QMultiLineEdit::setCursorPosition( int line, int col, bool mark )
{
	kdDebug(10) << "QMultiLineEdit::" << __FUNCTION__ << "()" << endl;
	int32 start, end;
	d->m_pcTextView->GetSelection(&start, &end);
	
	if( mark ) {
		end = d->m_pcTextView->OffsetAt( line );
		end += col;
	} else {
		start = d->m_pcTextView->OffsetAt( line );
		start += col;
		end = start;
	}

	d->m_pcTextView->Select(start, end);
}

void QMultiLineEdit::setReadOnly( bool bFlag )
{
	kdDebug(10) << "QMultiLineEdit::" << __FUNCTION__ << "()" << endl;
	d->m_pcTextView->MakeEditable( !bFlag );
}

bool QMultiLineEdit::isReadOnly() const
{
	kdDebug(10) << "QMultiLineEdit::" << __FUNCTION__ << "()" << endl;
	return( !d->m_pcTextView->IsEditable() );
}

QSize QMultiLineEdit::sizeHint() const
{
	kdDebug(10) << "QMultiLineEdit::" << __FUNCTION__ << "()" << endl;
	float width, height;
	d->m_pcTextView->GetPreferredSize( &width, &height );
	return( QSize( int(width) + 1, int(height) + 1 ) );
}

void QMultiLineEdit::SetMinPreferredSize( int nWidthChars, int nHeightChars )
{
	kdDebug(10) << "QMultiLineEdit::" << __FUNCTION__ << "()" << endl;
	printf( "\tWarning: %s not implemented\n", __FUNCTION__ );
#warning SetMinPreferredSize not implemented
}

void QMultiLineEdit::setText( const QString& cText )
{
	kdDebug(10) << "QMultiLineEdit::" << __FUNCTION__ << "()" << endl;
	d->m_pcTextView->SetText( cText.utf8().data(), cText.utf8().length() );
}

QString QMultiLineEdit::text() const
{
	kdDebug(10) << "QMultiLineEdit::" << __FUNCTION__ << "()" << endl;
	return( QString::fromUtf8( d->m_pcTextView->Text() ) );
}

QString QMultiLineEdit::textLine( int line ) const
{
	kdDebug(10) << "QMultiLineEdit::" << __FUNCTION__ << "()" << endl;
	int32 off1, off2, len;
	off1 = d->m_pcTextView->OffsetAt( line );
	off2 = d->m_pcTextView->OffsetAt( line+1 );
	len = off2-off1;
	if( len <= 0 )
		return QString::null;
	char buf[len+1];
	d->m_pcTextView->GetText(off1, len, buf );
	return QString( buf );
}

int QMultiLineEdit::numLines() const
{
	kdDebug(10) << "QMultiLineEdit::" << __FUNCTION__ << "()" << endl;
	return( d->m_pcTextView->CountLines() );
}

void QMultiLineEdit::MessageReceived( BMessage* pcMessage )
{
	kdDebug(10) << "QMultiLineEdit::" << __FUNCTION__ << "()" << endl;
	switch( pcMessage->what )
	{
	case TEXT_MODIFIED:
	{
		emit textChanged();
		break;
	}
	default:
		QWidget::MessageReceived( pcMessage );
		break;
	}
}

void 
QMultiLineEdit::AllAttached()
{
	d->m_pcTextView->SetTarget( this );	
}

#include "amultilineedit.moc"
