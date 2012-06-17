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

#include <wx/wx.h>
#include <wx/gdicmn.h>
#include <wx/imagpng.h>
#include <wx/imaglist.h>
#include <wx/html/htmlwin.h>
#include "global/ids.h"
#include "generated/configure_launcher.h"
#include "MainWindow.h"
#include "tabs/WelcomePage.h"
#include "tabs/ModsPage.h"
#include "tabs/BasicSettingsPage.h"
#include "tabs/AdvSettingsPage.h"
#include "tabs/InstallPage.h"
#include "controls/BottomButtons.h"
#include "apis/SkinManager.h"
#include "controls/Logger.h"
#include "controls/StatusBar.h"
#include "apis/HelpManager.h"

#include "global/MemoryDebugging.h" // Last include for memory debugging

#define MAINWINDOW_STYLE (wxBORDER_SUNKEN | wxSYSTEM_MENU\
	| wxCAPTION | wxCLOSE_BOX | wxMINIMIZE_BOX | wxCLIP_CHILDREN)

const int WINDOW_WIDTH = TAB_AREA_WIDTH;

MainWindow::MainWindow(SkinSystem* skin) {
	this->Create((wxFrame*)NULL, wxID_ANY, skin->GetTitle(),
		wxDefaultPosition, wxSize(WINDOW_WIDTH, 550), MAINWINDOW_STYLE);

	this->FS2_pid = 0;
	this->FRED2_pid = 0;

	this->SetFont(skin->GetFont());
	this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

	this->SetStatusBar(new StatusBar(this));

	// from http://stackoverflow.com/questions/6852413/
#if IS_WIN32 // set the titlebar/alt-tab icon
	this->SetIcon(wxICON(IDI_SMALL));
#elif IS_LINUX
	wxFileName iconFilename(_T(RESOURCES_PATH), _T("wxlauncher.ico"));
	wxIcon icon(iconFilename.GetFullPath(), wxBITMAP_TYPE_ICO);
	this->SetIcon(icon);
#endif

	// setup tabs

#if 0
	// Images used by wxImageList must be all the same dimentions
	wxImageList* images = new wxImageList;
  images->Create(64,64);
	images->Add(skin->GetWelcomeIcon());
	images->Add(skin->GetModsIcon());
	images->Add(skin->GetBasicIcon());
	images->Add(skin->GetAdvancedIcon());
	images->Add(skin->GetInstallIcon());
#endif

	this->mainTab = new wxNotebook();
	this->mainTab->Create(this, ID_MAINTAB, wxPoint(0,0), wxSize(WINDOW_WIDTH,-1), wxNB_TOP);
#if 0
	this->mainTab->AssignImageList(images);
#endif

	this->mainTab->AddPage(new WelcomePage(this->mainTab, skin), _("Welcome"), true);
	this->mainTab->AddPage(new ModsPage(this->mainTab, skin), _("Mods"), false);
	this->mainTab->AddPage(new BasicSettingsPage(this->mainTab), _("Basic Settings"), false);
	this->mainTab->AddPage(new AdvSettingsPage(this->mainTab, skin), _("Advanced Settings"), false);
#if 0
	this->mainTab->AddPage(new InstallPage(this->mainTab), _("Install/Update"), false);
#endif
	wxPoint bbpoint(0, -1);
	wxSize bbsize(WINDOW_WIDTH, -1);
	BottomButtons* bb = new BottomButtons(this, bbpoint, bbsize);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	sizer->Add(this->mainTab);
	sizer->Add(bb, wxSizerFlags().Expand());

	sizer->SetSizeHints(this);
	this->SetSizerAndFit(sizer);
	this->Layout();
	this->Center();
}

MainWindow::~MainWindow() {
	//
}

BEGIN_EVENT_TABLE(MainWindow, wxFrame)
	EVT_BUTTON(ID_CLOSE_BUTTON, MainWindow::OnQuit)
	EVT_BUTTON(ID_HELP_BUTTON, MainWindow::OnHelp)
	EVT_BUTTON(ID_FRED_BUTTON, MainWindow::OnStartFred)
	EVT_BUTTON(ID_UPDATE_BUTTON, MainWindow::OnUpdate)
	EVT_BUTTON(ID_PLAY_BUTTON, MainWindow::OnFSButton)
	EVT_BUTTON(ID_ABOUT_BUTTON, MainWindow::OnAbout)
	EVT_HELP(wxID_ANY, MainWindow::OnContextHelp)
	EVT_END_PROCESS(ID_FS2_PROCESS, MainWindow::OnFS2Exited)
	EVT_END_PROCESS(ID_FRED2_PROCESS, MainWindow::OnFRED2Exited)
END_EVENT_TABLE()

void MainWindow::OnQuit(wxCommandEvent& WXUNUSED(event)) {
	this->Destroy();
}
void MainWindow::OnHelp(wxCommandEvent& WXUNUSED(event)) {
	if (HelpManager::IsInitialized()) {
		HelpManager::OpenMainHelpPage();
	} else {
		wxLogWarning(_("Help Manager is not initialized"));
	}
}
void MainWindow::OnStartFred(wxCommandEvent& WXUNUSED(event)) {
	bool fredEnabled;
	ProMan::GetProfileManager()->GlobalRead(GBL_CFG_OPT_CONFIG_FRED, &fredEnabled, false);
	if ( fredEnabled ) {
		wxButton* fred = dynamic_cast<wxButton*>(
			wxWindow::FindWindowById(ID_FRED_BUTTON, this));
		wxCHECK_RET(fred != NULL, _T("Unable to find FRED button"));

		if (this->FRED2_pid == 0) {
			this->OnStart(fred, true);
		} else {
			this->OnKill(fred, true);
		}
	} else {
		wxLogError(_T("OnStartFred called while fredSupport is disabled!"));
	}
}
void MainWindow::OnFSButton(wxCommandEvent& WXUNUSED(event)) {
	wxButton* play = dynamic_cast<wxButton*>(
		wxWindow::FindWindowById(ID_PLAY_BUTTON, this));
	wxCHECK_RET(play != NULL, _T("Unable to find play button"));

	if (this->FS2_pid == 0) {
		this->OnStart(play);
	} else {
		this->OnKill(play);
	}
}

void MainWindow::OnStart(wxButton* button, bool startFred) {
	button->SetLabel(_("Starting"));
	button->Disable();

	const wxString defaultButtonValue((startFred)?_("FRED"):_("Play"));
	const wxString cfgBinaryPath((startFred)? PRO_CFG_TC_CURRENT_FRED : PRO_CFG_TC_CURRENT_BINARY);
	
	ProMan* p = ProMan::GetProfileManager();
	wxString folder, binary;
	if ( !p->ProfileRead(PRO_CFG_TC_ROOT_FOLDER, &folder) ) {
		wxLogError(_T("TC root folder for current profile is not set (%s)"), PRO_CFG_TC_ROOT_FOLDER);
		button->SetLabel(defaultButtonValue);
		button->Enable();
		return;
	}
	if ( !p->ProfileRead(cfgBinaryPath, &binary) ) {
		wxLogError(_T("No FreeSpace 2 Open executable has been selected (%s)"), cfgBinaryPath.c_str());
		button->SetLabel(defaultButtonValue);
		button->Enable();
		return;
	}

#if IS_APPLE
	wxFileName path(folder + wxFileName::GetPathSeparator() + binary, wxPATH_NATIVE);
#else
	wxFileName path(folder, binary, wxPATH_NATIVE);
#endif

	if ( !path.FileExists() ) {
		wxLogError(_T("Executable %s does not exist"), path.GetFullName().c_str());
		button->SetLabel(defaultButtonValue);
		button->Enable();
		return;
	}

	if ( ProMan::NoError != ProMan::GetProfileManager()->PushCurrentProfile() ) {
		button->SetLabel(defaultButtonValue);
		button->Enable();
		return;
	}

		wxString previousWorkingDir(::wxGetCwd());
	// hopefully this doesn't goof anything up
	if ( !::wxSetWorkingDirectory(folder) ) {
		wxLogError(_T("Unable to change working directory to %s"),
			folder.c_str());
		button->SetLabel(defaultButtonValue);
		button->Enable();
		return;
	}

	if ( startFred ) {
		this->process = new wxProcess(this, ID_FRED2_PROCESS);
	} else {
		this->process = new wxProcess(this, ID_FS2_PROCESS);
	}

	wxString command;
	// the "" correct for spaces in the path
	if (path.GetFullPath().Find(_T(" ")) != wxNOT_FOUND) {
		command = _T("\"") + path.GetFullPath() + _T("\"");
	} else {
		command = path.GetFullPath();
	}

	wxLogDebug(_T("Starting a process using '%s'"), command.c_str());
	long pid = ::wxExecute(command, wxEXEC_ASYNC, this->process);
	if ( pid == 0 ) {
		button->SetLabel(defaultButtonValue);
		button->Enable();
		return;
	}
	if ( startFred ) {
		this->FRED2_pid = pid;
		wxLogInfo(_T("FRED2 Open is now running..."));
	} else {
		this->FS2_pid = pid;
		wxLogInfo(_T("FreeSpace 2 Open is now running..."));
	}

	if ( !::wxSetWorkingDirectory(previousWorkingDir) ) {
		wxLogError(_T("Unable to change back to working directory %s"),
			previousWorkingDir.c_str());
	}

	button->SetLabel(_T("Kill"));
	button->Enable();
}

void MainWindow::OnKill(wxButton* button, bool killFred) {
	button->SetLabel(_T("Stopping"));
	button->Disable();

	int ret = ::wxKill((killFred)?this->FRED2_pid:this->FS2_pid, wxSIGKILL);
	if ( ret != wxKILL_OK ) {
		wxLogError(_T("Got KillError %d"), ret);
		wxLogError(_T("Failed to kill %s process!"), killFred?_T("FRED2 Open"):_T("FreeSpace 2 Open"));
	}
}

void MainWindow::OnUpdate(wxCommandEvent& WXUNUSED(event)) {
	wxMessageBox(_T("Update"));
}
void MainWindow::OnAbout(wxCommandEvent& WXUNUSED(event)) {
	wxMessageBox(_T("About"));
}

void MainWindow::OnContextHelp(wxHelpEvent& event) {
	HelpManager::OpenHelpById((WindowIDS)event.GetId());
}

void MainWindow::OnFS2Exited(wxProcessEvent &event) {
	if ( this->FS2_pid == 0 ) {
		wxLogError(_T("OnFS2Exited called before there is a process running"));
		return;
	}

	int exitCode = event.GetExitCode();

	wxLogInfo(_T("FreeSpace 2 Open exited with a status of %d"), exitCode);

	delete this->process;
	this->process = NULL;
	this->FS2_pid = 0;
	
	wxButton* play = dynamic_cast<wxButton*>(
		wxWindow::FindWindowById(ID_PLAY_BUTTON, this));
	wxCHECK_RET(play != NULL, _T("Unable to find play button"));
	play->SetLabel(_T("Play"));
	play->Enable();	
}

void MainWindow::OnFRED2Exited(wxProcessEvent &event) {
	if ( this->FRED2_pid == 0 ) {
		wxLogError(_T("OnFRED2Exited called before there is a process running"));
		return;
	}

	int exitCode = event.GetExitCode();

	wxLogInfo(_T("FRED2 Open exited with a status of %d"), exitCode);

	delete this->process;
	this->process = NULL;
	this->FRED2_pid = 0;
	
	wxButton* fred = dynamic_cast<wxButton*>(
		wxWindow::FindWindowById(ID_FRED_BUTTON, this));
	wxCHECK_RET(fred != NULL, _T("Unable to find FRED button"));
	fred->SetLabel(_T("FRED"));
	fred->Enable();
}
