/*
    Copyright (c) 2006 Maxim Sokhatsky
*/

#ifndef KWQSIGNAL_H
#define KWQSIGNAL_H

#include "KWQSlot.h"
#include "KWQValueList.h"

class KURL;
class KWIQResponse;

class KWQSignal {
public:
    KWQSignal(QObject *, const char *name);
    ~KWQSignal();
    void connect(const KWQSlot &);
    void disconnect(const KWQSlot &);
    void call(); // should be "emit"; can't be due to define in qobject.h
    void call(bool);
    void call(int);
    void call(const QString &);
#ifndef SANDBOX
    void call(KIO::Job *) const;
    void call(khtml::DocLoader *, khtml::CachedObject *) const;
    void call(KIO::Job *, const char *data, int size) const;
    void call(KIO::Job *, const KURL &) const;
    void call(KIO::Job *, KWIQResponse *) const;
#endif
private:
    // forbid copying and assignment
    KWQSignal(const KWQSignal &);
    KWQSignal &operator=(const KWQSignal &);
    QObject *_object;
    KWQSignal *_next;
    const char *_name;
    QValueList<KWQSlot> _slots;
    friend class QObject;
};

#endif
