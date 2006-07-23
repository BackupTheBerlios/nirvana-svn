
#include <Application.h>
#include <AppKit.h>
#include <ScrollView.h>
#include <TextView.h>
#include <Screen.h>
#include <InterfaceKit.h>
#include <SupportKit.h>
#include <OS.h>

#include "KWQObject.h"
#include "KWQEvent.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

class QObjectDerived : public QObject {
    char* const _objectName;
public:
    QObjectDerived(char *name);
    virtual void timerEvent(QTimerEvent *e);
    int i;
};

QObjectDerived::QObjectDerived(char *name) : _objectName(name) { i = 0; }
void QObjectDerived::timerEvent(QTimerEvent *e) {
    printf("Timer %i generates event for object %s\n", e->timerId(), _objectName);
    fflush(stdout);
    if (i == 3) killTimer(2);
    i++;
}

int main(void) {

    printf("QObject test...\n");

    printf("timers...\n");
    QObject *o1 = new QObjectDerived("1");
    QObject *o2 = new QObjectDerived("2");
    int i;
    long long int ival = 1000000;
    i = o1->startTimer(ival); printf("Timer %i started\n", i);
    i = o1->startTimer(ival); printf("Timer %i started\n", i);
    i = o2->startTimer(ival); printf("Timer %i started\n", i);
    i = o2->startTimer(ival); printf("Timer %i started\n", i);
    printf("Press any key to start looper\n");    
    getchar();
    status_t error;
    wait_for_thread(getLooperForObjectTimers()->Run(), &error);

    return 0;
}
