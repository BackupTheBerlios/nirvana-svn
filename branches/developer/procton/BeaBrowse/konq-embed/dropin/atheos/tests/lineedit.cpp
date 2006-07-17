#include <TWindow.h>
#include <OS.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qstring.h>
#include <qlistbox.h>
#include <qmultilineedit.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpushbutton.h>

class MWidget: public QWidget {
public:
	MWidget( QWidget *parent ): QWidget( parent, "MWidget" ) {}
	virtual void Draw( BRect aRect )
	{
		BRect r = Bounds();
		QPainter p( this );
		p.setPen( Qt::blue );
		p.drawLine( r.left, r.top, r.right, r.bottom );
	}

};

class MPixmapWidget: public QWidget {
public:
	MPixmapWidget( QWidget *parent ): QWidget( parent, "MWidget" )
	{
		mPix = new QPixmap( Bounds().Width(), Bounds().Height() );
	}
	~MPixmapWidget()
	{
		delete mPix;
	}
	virtual void Draw( BRect aRect )
	{
		BRect r = Bounds();
		QPainter p( mPix );
		p.setPen( Qt::red );
		p.drawLine( r.left, r.top, r.right, r.bottom );
		
		DrawBitmap( mPix->GetBitmap() );
	}
	virtual void FrameResized( float width, float height ) {
		mPix->resize( width, height );
	}

private:
	QPixmap *mPix;
};

void tmain() {

	TWindow *w = new TWindow( BRect(30,30,700,500), "test1" );

	QWidget *root = w->RootWidget();
	QCheckBox *check = new QCheckBox( root );
	QLineEdit *label = new QLineEdit( root );
	KComboBox *combo = new KComboBox( true, root );
	combo->insertItem( QString( "item1") );
	combo->insertItem( QString( "item2") );
	combo->insertItem( QString( "item3") );
	KListBox *list = new KListBox( root );
	list->insertItem( QString("item1" ) );
	list->insertItem( QString("item2" ) );
	list->insertItem( QString("item3" ) );
	QMultiLineEdit *multi = new QMultiLineEdit( root );
	MWidget *m = new MWidget ( root );
	MPixmapWidget *pix = new MPixmapWidget( root );
	QPushButton *button1 = new QPushButton( root );
	button1->setText( "Test 1" );
	QPushButton *button2 = new QPushButton("Test 2",root );
	
	
	w->Show();
}
