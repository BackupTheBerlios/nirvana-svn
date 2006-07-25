
#include <AppKit.h>
#include "KWQWidget.h"
#include "KWQCheckBox.h"
#include "TestWindow.h"

int main() {
    BApplication *app = new BApplication("application/x.vnd-Procton.ABrowseTest1");
    TestWindow *test = new TestWindow( BRect(30,30,700,500), "test1" );
    QWidget *widget = test->RootWidget();
    QCheckBox *check = new QCheckBox( widget ); 
    check->setText("Label");
    test->Show();
    app->Run();
    return 0;
}
