#include <wx/wx.h>
#include <wx/image.h>
#include "wxLauncherApp.h"
#include "MainWindow.h"
#include "Skin.h"

#include "wxLauncherSetup.h" // Last include for memory debugging

IMPLEMENT_APP(wxLauncher);

bool wxLauncher::OnInit() {
	wxInitAllImageHandlers();

#if MSCRTMEMORY
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	wxLogInfo(_T("wxLauncher Starting up."));

	SkinSystem* skin = new SkinSystem();

	MainWindow* window = new MainWindow(skin);
	window->Show(true);

	return true;
}