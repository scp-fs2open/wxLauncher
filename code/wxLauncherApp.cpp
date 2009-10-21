#include <wx/wx.h>
#include <wx/image.h>
#include "wxLauncherApp.h"
#include "MainWindow.h"

IMPLEMENT_APP(wxLauncher);

bool wxLauncher::OnInit() {
	wxInitAllImageHandlers();

	MainWindow* window = new MainWindow();
	window->Show(true);

	return true;
}