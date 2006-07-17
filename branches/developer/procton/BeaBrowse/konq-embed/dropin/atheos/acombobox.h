#ifndef __F_ACOMBOBOX_H__
#define __F_ACOMBOBOX_H__


#include <qwidget.h>

class KComboBox : public QWidget
{
    Q_OBJECT
public:
    KComboBox( bool, QWidget* pcParent );
    
    void	clear();
    int		count() const;
    void	insertItem( const QString &text, int index=-1 );

    virtual void	setCurrentItem( int index );

    virtual QSize sizeHint() const;
    
    virtual void MessageReceived( BMessage* pcMessage );
    virtual void AllAttached();

signals:
    void activated( int );
    
private:
    class Private;
    Private* d;
    
};

#endif // __F_ACOMBOBOX_H__
