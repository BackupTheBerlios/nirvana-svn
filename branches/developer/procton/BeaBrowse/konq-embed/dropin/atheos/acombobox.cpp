#include <stdio.h>

#include <qcombobox.h>
#include <Menu.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <Message.h>

#include <kdebug.h>

const int SELECTION_MSG = 'selM';

class KComboBox::Private
{
public:
	BMenuField *mMenuField;
};

// What could be a combobox?
KComboBox::KComboBox( bool, QWidget* pcParent ) : QWidget( pcParent, "combo_box" )
{
	kdDebug(10) << "KComboBox::" << __FUNCTION__ << "()" << endl;
	d = new Private;
	BMenu *menu = new BMenu( "" );
	menu->SetRadioMode( true );
	menu->SetLabelFromMarked( true );
	d->mMenuField = new BMenuField( Bounds(), "", "", menu, B_FOLLOW_ALL, B_WILL_DRAW | B_NAVIGABLE );
	d->mMenuField->SetDivider( 0 );
	AddChild( d->mMenuField );
}

int KComboBox::count() const
{
	kdDebug(10) << "KComboBox::" << __FUNCTION__ << "()" << endl;
	return( d->mMenuField->Menu()->CountItems() );
}

void KComboBox::clear()
{
	kdDebug(10) << "KComboBox::" << __FUNCTION__ << "()" << endl;
//	d->mMenuField->Clear();
	printf( "\tKComboBox is not implemented.\n" );
}

void KComboBox::insertItem( const QString &text, int index )
{
	kdDebug(10) << "KComboBox::" << __FUNCTION__ << "()" << endl;
	BMenuItem *item = new BMenuItem(text.ascii(), new BMessage( SELECTION_MSG ));
	if ( index == -1 ) {
		d->mMenuField->Menu()->AddItem( item );
	} else {
		d->mMenuField->Menu()->AddItem( item, index);
	}
	item->SetTarget( this );
}

void KComboBox::setCurrentItem( int index )
{
	kdDebug(10) << "KComboBox::" << __FUNCTION__ << "()" << endl;
	d->mMenuField->Menu()->ItemAt( index )->SetMarked( true );
}	

QSize KComboBox::sizeHint() const
{
	kdDebug(10) << "KComboBox::" << __FUNCTION__ << "()" << endl;
	float width, height;
	d->mMenuField->GetPreferredSize( &width, &height );
	return( QSize( int(width) + 1, int(height) + 1 ) );
}


void KComboBox::MessageReceived( BMessage* pcMessage )
{
	kdDebug(10) << "KComboBox::" << __FUNCTION__ << "()" << endl;
	switch( pcMessage->what )
	{
	case SELECTION_MSG:
	{
		int32 index(0);
		pcMessage->FindInt32( "index", &index );
		emit activated( index );
		break;
	}
	default:
		QWidget::MessageReceived( pcMessage );
		break;
	}
}

void 
KComboBox::AllAttached()
{
	kdDebug(10) << "KComboBox::" << __FUNCTION__ << "()" << endl;
	d->mMenuField->Menu()->SetTargetForItems( this );
}

#include "acombobox.moc"
