/*
 * Copyright (C) 2001, 2002 Apple Computer, Inc.  All rights reserved.
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

#include "KWQDateTime.h"
#include "glib.h"
#include <time.h>
#include <string.h>

QTime::QTime()
    :_timer(0), _h(0),_m(0),_s(0),_ms(0)
{
}

QTime::QTime ( int h, int m, int s, int ms )
    :_timer(0), _h(h), _m(m), _s(s), _ms(ms)
{
};

QTime::QTime(time_t t)
    : _timer(0)
{
    struct tm *ts = localtime(&t);
    _ms = 0;
    _s = ts->tm_sec;
    _m = ts->tm_min;
    _h = ts->tm_hour;

}

int QTime::msec() const
{	
    return _ms;
}

int QTime::elapsed()
{
    if (!_timer) return 0;
    gdouble sec = g_timer_elapsed(_timer, NULL /*micro_secs*/);

    _s = (int) sec;
    _h = (int) (_s / (60*60));
    _m = (int) ((_s - (_h * (60*60)) ) / 60); 
    _s = _s % 60;
    _ms = (int) ((sec - ((gdouble) ((int)sec))) * 1000);

    return (int) (sec * 1000.0f);
}

void QTime::start()
{
    if (!_timer)	
	_timer = g_timer_new();
    else
	g_timer_start(_timer);
}

int QTime::restart()
{
    int t = elapsed();
    start();
    return t;
}

QTime::~QTime()
{
    if (_timer) {
	g_timer_stop(_timer);
	g_timer_destroy(_timer);
    }
}

QDate::QDate(int y, int m, int d)
{
    struct tm ts;
    memset(&ts, 0, sizeof(ts));
    ts.tm_mday = d;
    ts.tm_mon = m;
    ts.tm_year = y;
    ttime = mktime(&ts);
}

QDate::QDate(time_t t)
    : ttime(t)
{
}

QDateTime::QDateTime(const QDate &d, const QTime &t)
{
    if (!d.isValid()) 
	return;

    struct tm *otherts;
    struct tm ts;
    otherts = localtime(&(d.ttime));

    ts = *otherts;
    ts.tm_sec = 0;
    ts.tm_hour = 0;
    ts.tm_min = 0;
    ttime = mktime(&ts);

    ttime += (t._h*60*60) + (t._m*60) + (t._s);
}

int QDateTime::secsTo(const QDateTime &b) const
{
    // only use in KHTML is measuring from Unix Epoch
    return (int)::time(0);
}

QString QDateTime::toString(const QString &format) const 
{
    return QString(ctime(&ttime));
}

#ifdef _KWQ_IOSTREAM_
#include <iostream.h>
std::ostream &operator<<(std::ostream &o, const QDate &date)
{
    return o <<
        "QDate: [yy/mm/dd: " <<
        date.year <<
        '/' <<
        date.month <<
        '/' <<
        date.day <<
        ']';
}
/*
std::ostream &operator<<(std::ostream &o, const QTime &time)
{
    CFGregorianDate g = CFAbsoluteTimeGetGregorianDate(time.timeInSeconds, systemTimeZone());
    return o <<
        "QTime: [hh:mm:ss:ms = " <<
        (int)g.hour <<
        ':' <<
        (int)g.minute <<
        ':' <<
        (int)g.second <<
        ':' <<
        time.msec() <<
        ']';
}

std::ostream &operator<<(std::ostream &o, const QDateTime &dateTime)
{
   CFGregorianDate g = CFAbsoluteTimeGetGregorianDate(dateTime.dateInSeconds, systemTimeZone());
    return o <<
        "QDateTime: [yy/mm/dd hh:mm:ss:ms = " <<
        (int)g.year <<
        '/' <<
        (int)g.month <<
        '/' <<
        (int)g.day <<
        ' ' << 
        (int)g.hour <<
        ':' <<
        (int)g.minute <<
        ':' <<
        (int)g.second <<
        ':' <<
        ((int)(g.second * 1000) % 1000) <<
        ']';
}
*/
#endif // _KWQ_IOSTREAM_
