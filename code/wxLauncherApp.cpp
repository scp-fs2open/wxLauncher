#include <wx/wx.h>
#include <wx/image.h>
#include "wxLauncherApp.h"
#include "MainWindow.h"
#include "Skin.h"

IMPLEMENT_APP(wxLauncher);

bool wxLauncher::OnInit() {
	wxInitAllImageHandlers();

	wxLogInfo(_T("wxLauncher Starting up."));

	SkinSystem* skin = new SkinSystem();

	MainWindow* window = new MainWindow(skin);
	window->Show(true);

	return true;
}