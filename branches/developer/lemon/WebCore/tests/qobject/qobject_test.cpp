
#include <Application.h>
#include <ScrollView.h>
#include <TextView.h>
#include <Screen.h>
#include <InterfaceKit.h>
#include <SupportKit.h>

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
};

QObjectDerived::QObjectDerived(char *name) : _objectName(name) { }
void QObjectDerived::timerEvent(QTimerEvent *e) {
    printf("Timer %i generated event for %s object\n", e->timerId(), _objectName);
    fflush(stdout);
}

int main(void) {
    printf("QObject Test...\n");
    QObject *o1 = new QObjectDerived("1");
    QObject *o2 = new QObjectDerived("2");
    int i = o1->startTimer(1000);
    printf("Timer %i started\n", i);
    return 0;
}
