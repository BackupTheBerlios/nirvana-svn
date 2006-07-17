#ifndef AFONTMETRICS_H
#define AFONTMETRICS_H

#include <qrect.h>
#include <qstring.h>

class QFont;
class QString;

class RFont;


class QFontMetricsInternal;

class QFontMetrics
{
public:
    QFontMetrics();
    QFontMetrics( const QFont& );
    QFontMetrics( const QFontMetrics & );
    ~QFontMetrics();
    QFontMetrics& operator=( const QFontMetrics& cOther);
    
    void setFont( const QFont& );
    int height() const;
    int width( const QString &, int len = -1 ) const;
    int width( QChar ) const;
    int width( char c ) const;

    int ascent() const;
    int descent() const;
    
    int rightBearing(QChar) const;
    
    QRect boundingRect( const QString &, int len = -1 ) const;
    QRect boundingRect( QChar ) const;
private:
    friend class QPainter;
    QFontMetrics( RFont* pcFont );
    QFontMetricsInternal* d;
};

#endif

