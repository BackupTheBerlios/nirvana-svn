
#include <qlistbox.h>
#include <Message.h>

#include <kdebug.h>
#include <stdio.h>

const int INVOKATION_MSG = 'invM';
const int SELECTION_MSG = 'selM';
class KListBox::Private
{
public:
	BListView* m_pcList;
};


QListBoxText::QListBoxText( const QString& cString )
{
	kdDebug(10) << "QListBoxText::" << __FUNCTION__ << "()" << endl;
	SetText( cString.utf8().data() );
}

KListBox::KListBox( QWidget* pcParent ) : QListBox( pcParent )
{
	kdDebug(10) << "KListBox::" << __FUNCTION__ << "()" << endl;
	d = new Private;
#warning Need a scrollview too.
	d->m_pcList = new BListView( Bounds(), "list_box", B_MULTIPLE_SELECTION_LIST, B_FOLLOW_ALL );
	d->m_pcList->SetInvocationMessage( new BMessage( INVOKATION_MSG ) );
	d->m_pcList->SetSelectionMessage( new BMessage( SELECTION_MSG ) );
	AddChild( d->m_pcList );
	d->m_pcList->SetTarget( this );
}

KListBox::~KListBox()
{
	kdDebug(10) << "KListBox::" << __FUNCTION__ << "()" << endl;
	delete d->m_pcList;
	delete d;
}

uint KListBox::count() const
{
	kdDebug(10) << "KListBox::" << __FUNCTION__ << "()" << endl;
	return( d->m_pcList->CountItems() );
}

void KListBox::clear()
{
	kdDebug(10) << "KListBox::" << __FUNCTION__ << "()" << endl;
	d->m_pcList->MakeEmpty();
}

void KListBox::insertItem( const QString &text, int index )
{
	kdDebug(10) << "KListBox::" << __FUNCTION__ << "()" << endl;
	if( index == -1 )
		d->m_pcList->AddItem( new QListBoxText( text ));
	else
		d->m_pcList->AddItem( new QListBoxText( text ), index );
}

void KListBox::insertItem( const QListBoxItem* pcItem, int index )
{
	kdDebug(10) << "KListBox::" << __FUNCTION__ << "()" << endl;
	printf( "\tWarning: %s not implemented\n", __FUNCTION__ );
	if( index == -1 )
		d->m_pcList->AddItem( const_cast<QListBoxItem *>(pcItem));
	else
		d->m_pcList->AddItem( const_cast<QListBoxItem *>(pcItem), index );
}

void KListBox::setSelectionMode( SelectionMode eMode )
{
	kdDebug(10) << "KListBox::" << __FUNCTION__ << "()" << endl;
	if( eMode == QListBox::Single ) 
		d->m_pcList->SetListType( B_SINGLE_SELECTION_LIST );
	 else if( eMode == QListBox::Multi )
		d->m_pcList->SetListType( B_MULTIPLE_SELECTION_LIST );
	 	
}

void KListBox::setSelected( int nIndex, bool bSelected )
{
	kdDebug(10) << "KListBox::" << __FUNCTION__ << "()" << endl;

	if( bSelected )
		d->m_pcList->Select( nIndex );
	else
		d->m_pcList->Deselect( nIndex );
}

void KListBox::setCurrentItem( int index )
{
	kdDebug(10) << "KListBox::" << __FUNCTION__ << "()" << endl;
	d->m_pcList->Select( index );
}

bool KListBox::isSelected( int nIndex ) const
{
	kdDebug(10) << "KListBox::" << __FUNCTION__ << "()" << endl;
	return( d->m_pcList->IsItemSelected( nIndex ) );
}

int KListBox::index( const QListBoxItem* pcItem ) const
{
	kdDebug(10) << "KListBox::" << __FUNCTION__ << "()" << endl;
	return( d->m_pcList->IndexOf( const_cast<QListBoxItem *>(pcItem) ) );
}

QSize KListBox::sizeHint() const
{
	kdDebug(10) << "KListBox::" << __FUNCTION__ << "()" << endl;
	float width, height;
	d->m_pcList->GetPreferredSize( &width, &height );
	return( QSize( int(width) + 1, int(height) + 1 ) );
}

void KListBox::MessageReceived( BMessage* pcMessage )
{
	kdDebug(10) << "KListBox::" << __FUNCTION__ << "()" << endl;
	switch( pcMessage->what ){	

	case INVOKATION_MSG:
	{
		kdDebug(10) << "\tINVOKATION_MSG" << endl;
		int32 selected; 
		for ( int i = 0 ; (selected = d->m_pcList->CurrentSelection( i ) ) >= 0; ++i ) {
//			emit activated( i /*d->m_pcList->GetFirstSelected()*/ );
			emit pressed( static_cast<QListBoxItem*>(d->m_pcList->ItemAt(selected)) );
		}
		break;
	}
	default:
		QWidget::MessageReceived( pcMessage );
		break;
	}
}

#include "alistbox.moc"
