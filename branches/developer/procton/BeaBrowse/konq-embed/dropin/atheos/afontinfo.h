#ifndef AFONTINFO_H
#define AFONTINFO_H

#include "qfont.h"

class QFontInfoInternal;

class QFontInfo
{
public:
    QFontInfo( const QFont & );
    ~QFontInfo();
    int pointSize() const;
    bool fixedPitch() const;
private:
    QFontInfo& operator=( const QFontInfo& );
    QFontInfoInternal* d;
};

#endif

