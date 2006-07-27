#include <AppKit.h>
#include <stdio.h>

#include "KWQWidget.h"
#include "KWQObject.h"
#include "KWQCheckBox.h"
#include "KWQSlot.h"
#include "KWQSignal.h"
#include "TestWindow.h"
#include "UserSlots.h"

class QSlotStorage : public QObject {
public:
    QSlotStorage() { }
    void onClick() {
	printf("QSlotStorage::onClicked()\n");
	fflush(stdout);
    }
};

int main() {
    BApplication *app = new BApplication("application/x.vnd-Lemon-Nirvana");
    TestWindow *test = new TestWindow( BRect(30,30,700,500), "test1" );
    QWidget *widget = test->RootWidget();
    QCheckBox *check = new QCheckBox( widget ); 
    QSlotStorage *storage = new QSlotStorage();
    
    QObject::connect(check, SIGNAL(clicked()), new QSlot(storage, SLOT(onClick())));
    KWQSignal *signal = check->findSignal(SIGNAL(clicked()));
    printf("%s\n",signal->_name);
    signal->call();
	
    check->setText("Label");
    check->move(100,100);

    test->Show();
    app->Run();
    return 0;
}
