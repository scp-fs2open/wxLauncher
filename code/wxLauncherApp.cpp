
#include "wxLauncherApp.h"
#include "MainWindow.h"

IMPLEMENT_APP(wxLauncher);

bool wxLauncher::OnInit() {
	wxBitmap::InitStandardHandlers();

	MainWindow* window = new MainWindow();
	window->Show(true);

	return true;
}