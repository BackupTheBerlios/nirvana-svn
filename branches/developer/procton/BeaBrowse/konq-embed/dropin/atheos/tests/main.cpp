#include <Application.h>

extern void tmain();
int main() {

	BApplication app("application/x.vnd-Procton.ABrowseTest1");
	
	tmain();	
	app.Run();
	return 0;
}
