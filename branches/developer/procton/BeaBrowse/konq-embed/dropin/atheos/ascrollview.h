#ifndef __F_ASCROLLVIEW_H__
#define __F_ASCROLLVIEW_H__

#include <qframe.h>
#include <qfontmetrics.h>

class QScrollViewPrivate;

//BScrollView??
class QScrollView : public QFrame
{
    Q_OBJECT
public:
    enum ScrollBarMode { Auto, AlwaysOff, AlwaysOn };
    
    QScrollView( QWidget* /*parent*/, const char* /*name*/, int /*flags*/ );
    ~QScrollView();

    virtual BPoint GetPreferredSize( bool bLargest ) const;
//    virtual void HandleMessage( BMessage* pcMessage );
//    virtual void WheelMoved( const BPoint& cDelta );
	virtual void ScrollTo( BPoint point );
    
    QWidget* viewport();
    


    void contentsToViewport(int x, int y, int& vx, int& vy);
    void viewportToContents(int vx, int vy, int& x, int& y);
    
    int	 contentsX();
    int	 contentsY();
    int  contentsWidth() const;
    int	 contentsHeight() const;
    
    int	visibleWidth();
    int	visibleHeight();
    
    virtual void setVScrollBarMode( ScrollBarMode );
    virtual void setHScrollBarMode( ScrollBarMode );

    void setStaticBackground( bool );

    bool focusNextPrevChild( bool /*next*/ );

    virtual void addChild( QWidget* /*child*/, int /*x*/=0, int /*y*/=0 );

    void enableClipper( bool );
    
    virtual void drawContents(QPainter*, int cx, int cy, int cw, int ch) = 0;
    void updateContents( int, int, int, int );
    void repaintContents( int x, int y, int w, int h, bool erase=true );

public:
//    virtual void Paint( const BRect& cUpdateRect );
    virtual void Draw( const BRect& cUpdateRect );

public slots:
    virtual void resizeContents( int w, int h );
    void scrollBy( int, int );
    virtual void setContentsPos( int x, int y );
    void ensureVisible( int x, int y );
    void ensureVisible(int x, int y, int xmargin, int ymargin);
    
private:
    void UpdateScrollBars();
    
    QScrollViewPrivate* d;
};

#endif // __F_ASCROLLVIEW_H__
