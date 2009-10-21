
#include "wxLauncherApp.h"
#include "MainWindow.h"

IMPLEMENT_APP(wxLauncher);

bool wxLauncher::OnInit() {
	MainWindow* window = new MainWindow();
	window->ShowFullScreen(true);

	return true;
}