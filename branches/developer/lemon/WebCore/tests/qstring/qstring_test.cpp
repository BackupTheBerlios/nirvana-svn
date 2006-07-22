
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
    printf("QString Test...\n");

    char *string = "\xD1\x8B"; // cyrillic symbol
    QString s = QString::fromUtf8(string);
    ASSERT(s.length() == 1);
    printf("\nstrlen - %i\n", s.length());
    fflush(stdout);

    string = "\x8B\xD1"; // incorrect symbol
    s =  QString::fromUtf8(string);
    ASSERT(s.length() == 0);
    printf("\nstrlen - %i\n", s.length());
    fflush(stdout);

    string = "\xE3\x81\x8D"; // japanese symbol
    s =  QString::fromUtf8(string);
    ASSERT(s.length() == 1);
    printf("\nstrlen - %i\n", s.length());
    fflush(stdout);

    string = "lucky"; // latin string
    s =  QString::fromUtf8(string);
    ASSERT(s.length() == 5);
    printf("\nstrlen - %i\n", s.length());
    fflush(stdout);

    return 0;
}
