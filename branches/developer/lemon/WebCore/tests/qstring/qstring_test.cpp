
#include <Application.h>
#include <ScrollView.h>
#include <TextView.h>
#include <Screen.h>
#include <InterfaceKit.h>
#include <SupportKit.h>

#include "KWQString.h"
#include "KWQAssertions.h"

#include <string.h>
#include <stdio.h>
#include <malloc.h>

int main(void) {
    printf("QString test\n");
    char *string = "\xD1\x8B";
    QString s = QString::fromUtf8(string);
    ASSERT(s.length() == 1);
    printf("\nstrlen - %i\n", s.length());
    fflush(stdout);
    string = "\x8B\xD1";
    s =  QString::fromUtf8(string);
    ASSERT(s.length() == 0);
    printf("\nstrlen - %i\n", s.length());
    fflush(stdout);
    return 0;
}
