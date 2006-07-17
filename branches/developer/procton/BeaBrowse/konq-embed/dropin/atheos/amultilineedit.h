#ifndef __F_AMULTILINEEDIT_H__
#define __F_AMULTILINEEDIT_H__

#include <qwidget.h>


class QMultiLineEdit : public QWidget
{
	Q_OBJECT
public:
	QMultiLineEdit( QWidget* pcParent );
	~QMultiLineEdit();
	
	virtual void	   setText( const QString &);
	void	   selectAll();

	QString text() const;

	QString textLine( int line ) const;
	int numLines() const;
	
	bool hasMarkedText() const;
	void setReadOnly( bool );
	bool isReadOnly() const;
	
	virtual void setCursorPosition( int line, int col, bool mark = FALSE );
	void getCursorPosition( int *line, int *col ) const;

	virtual QSize sizeHint() const;

	void   SetMinPreferredSize( int nWidthChars, int nHeightChars );
	virtual void MessageReceived( BMessage* pcMessage );
	virtual void AllAttached();
signals:
	void textChanged();
	
private:
	class Private;
	Private* d;
	
};

#endif // __F_AMULTILINEEDIT_H__
