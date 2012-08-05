/*
Copyright (C) 2009-2010 wxLauncher Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "generated/configure_launcher.h"
#include <wx/wx.h>
#include <wx/cmdline.h>
#include <wx/image.h>
#include <wx/filesys.h>
#include <wx/fs_arc.h>
#include <wx/fs_inet.h>
#include <wx/splash.h>

#if HAS_SDL == 1
#include "SDL.h"
#endif

// prevents SDL_main.h (included by SDL.h) from redefining main
#ifdef main
#undef main
#endif

#include "wxLauncherApp.h"
#include "MainWindow.h"
#include "apis/SkinManager.h"
#include "controls/Logger.h"
#include "global/version.h"
#include "apis/TCManager.h"
#include "apis/ProfileManager.h"
#include "apis/HelpManager.h"
#include "apis/FlagListManager.h"
#include "apis/ProfileProxy.h"

#include "global/MemoryDebugging.h" // Last include for memory debugging

IMPLEMENT_APP(wxLauncher);

const static wxCmdLineEntryDesc CmdLineOptions[] = {
	{wxCMD_LINE_SWITCH, NULL, _T("session-only"),
	_T("Do not remember the profile that is selected at exit")},
	{wxCMD_LINE_SWITCH, NULL, _T("add-profile"),
	_T("Add profile PROFILE from FILE. If PROFILE already exists ")
	_T("it will not be overwritten. *Operator*")},
	{wxCMD_LINE_SWITCH, NULL, _T("select-profile"),
	_T("Make PROFILE the that wxLauncher will use on next run. *Operator*")},
	{wxCMD_LINE_OPTION, NULL, _T("profile"),
	_T("The name of a profile to operate on. Operand PROFILE."),
	wxCMD_LINE_VAL_STRING, 0},
	{wxCMD_LINE_OPTION, NULL, _T("file"),
	_T("The path to a file to operate on. Operand FILE."),
	wxCMD_LINE_VAL_STRING, 0},
	{wxCMD_LINE_NONE},
};

void wxLauncher::OnInitCmdLine(wxCmdLineParser& parser)
{
	parser.SetDesc(CmdLineOptions);
	parser.SetSwitchChars(_T("-")); // always use -, even on windows

	wxApp::OnInitCmdLine(parser);
}

bool wxLauncher::OnCmdLineParsed(wxCmdLineParser& parser)
{
	if (!wxApp::OnCmdLineParsed(parser))
		return false;

	if (parser.Found(_T("session-only")))
	{
		mKeepForSessionOnly = true;
	}
	
	if (parser.Found(_T("add-profile")))
	{
		mProfileOperator = ProManOperator::add;
		if (!parser.Found(_T("profile"), &mProfileOperand))
		{
			wxLogError(_T("No profile specified to add"));
			return false;
		}
		if (!parser.Found(_T("file"), &mFileOperand))
		{
			wxLogError(_T("No file specified to add as profile"));
			return false;
		}
	}
	else if(parser.Found(_T("select-profile")))
	{
		mProfileOperator = ProManOperator::select;
		if (!parser.Found(_T("profile"), &mProfileOperand))
		{
			wxLogError(_T("No profile specified to select"));
			return false;
		}
	}

	return true;
}

wxLauncher::wxLauncher()
	:skin(NULL),
	mProfileOperator(ProManOperator::none),
	mKeepForSessionOnly(false),
	mShowGUI(false)
	// The strings init themselves sanely
{
}

/** Display the splash screen.

\param splashWindow Out. Point to splash window if created. NULL otherwise.
\param returns false if an error occurented while loading spalsh; a
false return value indicates that this function has already informed the user
of the failue.  Returns true otherwise.
*/
bool displaySplash(wxSplashScreen **splashWindow)
{
	wxBitmap splash;
	wxFileName splashFile(_T(RESOURCES_PATH),_T("SCP_Header.png"));
	if (splash.LoadFile(splashFile.GetFullPath(), wxBITMAP_TYPE_ANY)) {
#if NDEBUG
		(*splashWindow) = new wxSplashScreen(splash, wxSPLASH_CENTRE_ON_SCREEN, 0, NULL, wxID_ANY);
#else
		(*splashWindow) = NULL;//new wxSplashScreen(splash, wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_TIMEOUT, 1000, NULL, wxID_ANY);
#endif
		wxYield();
	} else {
		wxFileName expectedDir;
		if (wxFileName(_T(RESOURCES_PATH)).IsAbsolute()) {
			expectedDir = wxFileName(_T(RESOURCES_PATH));
		} else {
			expectedDir = wxFileName(::wxGetCwd(), _T(RESOURCES_PATH));	
		}
		wxLogFatalError(wxString::Format(
			_T("Unable to load splash image. ")
			_T("This normally means that you are running the Launcher from a folder")
			_T(" that the launcher does not know how to find the resource folder from.")
			_T("\n\nThe launcher is expecting (%s) to contain the resource images."),
			expectedDir.GetFullPath().c_str()).c_str());
		return false;
	}
	return true;
}

int wxLauncher::OnRun() {
	if (mProfileOperator == ProManOperator::none)
	{
		return wxApp::OnRun();
	}
	else
	{
		return ProManOperator::RunProfileOperator(mProfileOperator);
	}
}

bool wxLauncher::OnInit() {
	wxInitAllImageHandlers();

	// call base class OnInit so that cmdline stuff works.
	if (!wxApp::OnInit())
		return false; // base said abort so abort

#if MSCRTMEMORY
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
#if HAS_SDL == 1
	if ( 0 != SDL_InitSubSystem(SDL_INIT_VIDEO) ) {
	  wxLogFatalError(_T("SDL_InitSubSystem failed"));
	}
#endif
	// Little hack to deal with people starting the launcher from the bin folder
	if ( !wxFileName::DirExists(_T(RESOURCES_PATH)) ) {
		wxFileName resourceDir;
		resourceDir.AssignDir(::wxGetCwd());
		resourceDir.AppendDir(_T(".."));
		resourceDir.AppendDir(_T(RESOURCES_PATH));
		if ( resourceDir.DirExists() ) {
			wxFileName newWorkingDir(::wxGetCwd(), _T(".."));
			::wxSetWorkingDirectory(newWorkingDir.GetFullPath());
		}
	}

	wxSplashScreen* splashWindow = NULL;
	if (!displaySplash(&splashWindow))
		return false;

	wxLog::SetActiveTarget(new Logger());
	wxLogInfo(_T("wxLauncher Version %d.%d.%d"), MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION);
	wxLogInfo(_T("Build \"%s\" committed on (%s)"), HGVersion, HGDate);
	wxLogInfo(wxDateTime(time(NULL)).Format(_T("%c")));

	wxLogInfo(_T("Initializing profiles..."));
	ProMan::Flags promanFlags = ProMan::None;
	if (mKeepForSessionOnly)
		promanFlags = promanFlags | ProMan::NoUpdateLastProfile;
	if ( !ProMan::Initialize(promanFlags) ) {
		wxLogFatalError(_T("ProfileManager failed to initialize. Aborting! See log file for more details."));
		return false;
	}
	if (mProfileOperator != ProManOperator::none)
	{
		// We are not to create a GUI so we are done init now
		return true;
	}

	wxFileSystem::AddHandler(new wxArchiveFSHandler);
	wxFileSystem::AddHandler(new wxInternetFSHandler);

	wxLogInfo(_T("Initializing Skin System..."));
	this->skin = new SkinSystem();

	wxLogInfo(_T("Initializing HelpManager..."));
	HelpManager::Initialize();
	
	wxLogInfo(_T("Initializing FlagListManager..."));
	FlagListManager::Initialize();
	
	wxLogInfo(_T("Initializing ProfileProxy..."));
	ProfileProxy::Initialize();

	wxLogInfo(_T("wxLauncher starting up."));


	MainWindow* window = new MainWindow(skin);
	wxLogStatus(_T("MainWindow is complete"));
	window->Show(true);
#if NDEBUG // will autodelete when timout runs out in debug
	splashWindow->Show(false);
	splashWindow->Destroy();
#endif

	// must call TCManager::CurrentProfileChanged() manually on startup,
	// since initial profile switch takes place before TCManager has been initialized
	// calling it here to ensure that AdvSettingsPage is set up by the time events are triggered
	wxCommandEvent tcMgrInitEvent;
	TCManager::Get()->CurrentProfileChanged(tcMgrInitEvent);

	wxLogStatus(_T("Ready."));
	return true;
}

int wxLauncher::OnExit() {

	ProMan::DeInitialize();

	if (mProfileOperator == ProManOperator::none)
	{

		if (this->skin != NULL) {
			delete this->skin;
		}

		HelpManager::DeInitialize();
		ProfileProxy::DeInitialize();
		FlagListManager::DeInitialize();

#if HAS_SDL == 1
		SDL_Quit();
#endif

	}

	wxLogInfo(_T("wxLogger shutdown complete."));

	return wxApp::OnExit();
}
