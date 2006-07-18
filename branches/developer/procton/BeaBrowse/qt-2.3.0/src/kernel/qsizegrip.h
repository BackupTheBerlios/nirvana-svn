/****************************************************************************
** $Id: qsizegrip.h,v 1.1.1.1 2002/04/27 11:02:21 procton Exp $
**
** Definition of QSizeGrip class
**
** Created : 980316
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QSIZEGRIP_H
#define QSIZEGRIP_H

#ifndef QT_H
#include "qwidget.h"
#endif // QT_H

#ifndef QT_NO_SIZEGRIP

class Q_EXPORT QSizeGrip: public QWidget
{
    Q_OBJECT
public:
    QSizeGrip( QWidget *parent, const char* name=0 );
    ~QSizeGrip();

    QSize sizeHint() const;
    QSizePolicy sizePolicy() const;

protected:
    void paintEvent( QPaintEvent * );
    void mousePressEvent( QMouseEvent * );
    void mouseMoveEvent( QMouseEvent * );
private:
    QPoint p;
    QSize s;
    int d;
};

#endif //QT_NO_SIZEGRIP
#endif