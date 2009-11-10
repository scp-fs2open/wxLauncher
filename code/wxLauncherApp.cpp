#include <wx/wx.h>
#include <wx/image.h>
#include "wxLauncherApp.h"
#include "MainWindow.h"
#include "Skin.h"
#include "Logger.h"
#include "version.h"
#include "ProfileManager.h"

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

	wxLogInfo(_T("Initing profiles..."));
	if ( !ProMan::Initialize() ) {
		wxLogFatalError(_T("ProfileManager failed to initialize. Aborting! See log file for more details."));
		return false;
	}

	wxLogInfo(_T("Initing Skin System..."));
	this->skin = new SkinSystem();

	wxLogInfo(_T("wxLauncher Starting up."));


	MainWindow* window = new MainWindow(skin);
	wxLogStatus(_T("MainWindow is complete"));
	window->Show(true);

	wxLogStatus(_T("Ready."));
	return true;
}

int wxLauncher::OnExit() {
	if (this->skin != NULL) {
		delete this->skin;
	}

	return 0;
}