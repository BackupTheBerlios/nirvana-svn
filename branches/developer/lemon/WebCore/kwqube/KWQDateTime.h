/*
 * Copyright (C) 2003 Apple Computer, Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#ifndef QDATETIME_H_
#define QDATETIME_H_

#include "KWQDef.h"

#include "KWQString.h"

#include <time.h>
typedef struct _GTimer GTimer;


#ifdef _KWQ_IOSTREAM_
#include <iosfwd>
#endif

class QTime 
{
 public:
    QTime();
    QTime(int h, int m, int s = 0, int ms = 0);
    QTime(time_t t);
    ~QTime();

    int msec() const;
    
    void start();
    int elapsed() /*const*/; // FIXME KWIQ: const removed
    int restart();

protected:

private:
    GTimer* _timer;
	
    int _h, _m, _s, _ms;
    
    friend class QDateTime;
#ifdef _KWQ_IOSTREAM_
    friend std::ostream &operator<<( std::ostream &, const QTime & );
#endif
};

class QDate {
public:
    QDate(int year, int month, int day);
    QDate(time_t t);

    bool isValid() const { return ttime != 0; }
private:
    friend class QDateTime;

    time_t ttime;
#ifdef _KWQ_IOSTREAM_
    friend std::ostream &operator<<( std::ostream &, const QDate & );
#endif

};

class QDateTime {
 public:
    QDateTime(): ttime(0) {}
    QDateTime(time_t t): ttime(t) {};

    QDateTime(const QDate &, const QTime &);
    QTime time() { return QTime(ttime); }

    int secsTo(const QDateTime &) const;

    static QDateTime currentDateTime() {return QDateTime(::time(NULL));};

    bool isValid() const { return ttime != 0; }

    /** format not implemented*/
    QString toString(const QString &format) const;
private:
    
    time_t ttime;

#ifdef _KWQ_IOSTREAM_
    friend std::ostream &operator<<( std::ostream &, const QDateTime & );
#endif
};

#endif
