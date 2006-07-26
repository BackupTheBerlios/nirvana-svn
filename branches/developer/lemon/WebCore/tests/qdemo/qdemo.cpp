#include <AppKit.h>
#include <stdio.h>

#include "KWQWidget.h"
#include "KWQObject.h"
#include "KWQCheckBox.h"
#include "KWQSlot.h"
#include "KWQSignal.h"
#include "TestWindow.h"

class QSlotStorage : public QObject {
public:
    QSlotStorage() {
    }
    void onClick() {
	printf("QSlotStorage::onClicked()\n");
	fflush(stdout);
    }
};

enum FunctionNumberEx {
    onNull = 10000,
    onClick
};

class QSlot : public KWQSlot {
public:
    QSlot(QObject *object, const char *member); 
    virtual void call();
    virtual char *version() const { return "QSlot"; }
};

QSlot::QSlot(QObject *object, const char *member)//: KWQSlot(object, member)
{
    #define CASE(function, parameters, type) \
        if (KWQNamesMatch(member, "SLOT:" #function #parameters)) { \
        ASSERT( QOBJECT_IS_A(object, type) ); \
        m_function = function; \
        } else

    CASE(onClick, (), QSlotStorage)
    {}
	
    #undef CASE
    
    m_object = object;
}

void QSlot::call() {

    printf("QSlot::call()\n");
    fflush(stdout);

    if (m_object.isNull()) {
        return;
    }
	    
    #define CASE(member, type, function) \
	case member: \
	    static_cast<type *>(m_object.pointer())->function(); \
	    break;
						
    switch (m_function) {
        CASE(onClick, QSlotStorage, onClick)
    }
    
    #undef CASE

    KWQSlot::call();
}


int main() {
    BApplication *app = new BApplication("application/x.vnd-Procton.ABrowseTest1");
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
