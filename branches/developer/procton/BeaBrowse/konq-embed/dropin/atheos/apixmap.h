#ifndef APIXMAP_H
#define APIXMAP_H

#include <qpaintdevice.h>
#include <qnamespace.h>
#include <qsize.h>
#include <qrect.h>

class QBitmap;
class QPixmapPrivate;

class BBitmap;
class BView;


class QPixmap : public QPaintDevice, public Qt
{
public:
    QPixmap();
    QPixmap( const QPixmap& cOther );
    QPixmap( int, int );
    QPixmap( BBitmap* pcBitmap );
    ~QPixmap();
    QPixmap& operator=(const QPixmap&);

    virtual int	 metric( int id ) const;
    
    QSize size() const;
    QRect rect() const;
    int width() const;
    int height() const;
    void resize( int, int );

    bool isNull() const;

    QBitmap* mask() const;

    void SetIsTransparent( bool bTrans );
    bool IsTransparent() const;
    void ExpandValidRect( const QRect& cRect ) const;
    QRect GetValidRect() const;

    BBitmap* GetBitmap() const;
private:
    friend QPainter;
    BView* GetView() const;
    QPixmapPrivate* d;
};

#endif

