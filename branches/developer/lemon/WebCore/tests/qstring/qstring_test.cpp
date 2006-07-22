
#include <Application.h>
#include <ScrollView.h>
#include <TextView.h>
#include <Screen.h>
#include <InterfaceKit.h>
#include <SupportKit.h>

#include "KWQString.h"

#include <string.h>
#include <stdio.h>
#include <malloc.h>


int main(void) {
    printf("QString test\n");
    char *string = "\0xD1\0x8Bs\0";
    QString s = QString::fromUtf8(string);
    printf("strlen - %i\n", s.length());
    return 0;
}
