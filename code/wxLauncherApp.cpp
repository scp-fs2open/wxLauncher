#include <wx/wx.h>
#include <wx/image.h>
#include "wxLauncherApp.h"
#include "MainWindow.h"
#include "Skin.h"
#include "Logger.h"
#include "version.h"

#include "wxLauncherSetup.h" // Last include for memory debugging

IMPLEMENT_APP(wxLauncher);

bool wxLauncher::OnInit() {
	wxInitAllImageHandlers();

#if MSCRTMEMORY
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	
	wxLog::SetActiveTarget(new Logger());
	wxLogInfo(_T("wxLauncher Version %d.%d"), MAJOR_VERSION, MINOR_VERSION);
	wxLogInfo(_T("Build \"%s\" committed on (%s)"), HGVersion, HGDate);
	wxLogInfo(wxDateTime(time(NULL)).Format(_T("%c")));

	wxLogInfo(_T("wxLauncher Starting up."));

	this->skin = new SkinSystem();

	MainWindow* window = new MainWindow(skin);
	window->Show(true);

	return true;
}

int wxLauncher::OnExit() {
	if (this->skin != NULL) {
		delete this->skin;
	}

	return 0;
}