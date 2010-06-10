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
#include <wx/toolbook.h>
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
	| wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN)

MainWindow::MainWindow(SkinSystem* skin) {
	this->SetExtraStyle(wxFRAME_EX_CONTEXTHELP);
	this->Create((wxFrame*)NULL, wxID_ANY, skin->GetTitle(),
		wxDefaultPosition, wxSize(745, 550), MAINWINDOW_STYLE);

	this->FS2_pid = 0;

	this->SetFont(skin->GetFont());
	this->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));

	this->SetStatusBar(new StatusBar(this));

	// setup tabs

	// Images used by wxImageList must be all the same dimentions
	wxImageList* images = new wxImageList;
  images->Create(64,64);
	images->Add(skin->GetWelcomeIcon());
	images->Add(skin->GetModsIcon());
	images->Add(skin->GetBasicIcon());
	images->Add(skin->GetAdvancedIcon());
	images->Add(skin->GetInstallIcon());

	this->mainTab = new wxToolbook();
	this->mainTab->Create(this, ID_MAINTAB, wxPoint(0,0), wxSize(745,-1),	wxNB_LEFT);
	this->mainTab->AssignImageList(images);
	this->mainTab->AddPage(new WelcomePage(this->mainTab, skin), _("Welcome"), true, ID_TAB_WELCOME_IMAGE);
	this->mainTab->AddPage(new ModsPage(this->mainTab, skin), _("Mods"), false, ID_TAB_MOD_IMAGE);
	this->mainTab->AddPage(new BasicSettingsPage(this->mainTab), _("Basic Settings"), false, ID_TAB_BASIC_SETTINGS_IMAGE);
	this->mainTab->AddPage(new AdvSettingsPage(this->mainTab, skin), _("Advanced Settings"), false, ID_TAB_ADV_SETTINGS_IMAGE);
	this->mainTab->AddPage(new InstallPage(this->mainTab), _("Install/Update"), false, ID_TAB_INSTALL_IMAGE);

	wxPoint bbpoint(0, -1);
	wxSize bbsize(745, -1);
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
	EVT_BUTTON(ID_PLAY_BUTTON, MainWindow::OnStartFS)
	EVT_BUTTON(ID_ABOUT_BUTTON, MainWindow::OnAbout)
	EVT_HELP(wxID_ANY, MainWindow::OnContextHelp)
	EVT_END_PROCESS(ID_FS2_PROCESS, MainWindow::OnFS2Exited)
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
	wxMessageBox(_("Start Fred"));
}
void MainWindow::OnStartFS(wxCommandEvent& WXUNUSED(event)) {
	wxButton* play = dynamic_cast<wxButton*>(
		wxWindow::FindWindowById(ID_PLAY_BUTTON, this));
	wxCHECK_RET(play != NULL, _T("Unable to find play button"));
	play->SetLabel(_("Running"));
	play->Disable();
	
	ProMan* p = ProMan::GetProfileManager();
	wxString folder, binary;
	if ( !p->Get()->Read(PRO_CFG_TC_ROOT_FOLDER, &folder) ) {
		wxLogError(_T("TC folder for current profile is not set (%s)"), PRO_CFG_TC_ROOT_FOLDER);
		play->SetLabel(_("Play"));
		play->Enable();
		return;
	}
	if ( !p->Get()->Read(PRO_CFG_TC_CURRENT_BINARY, &binary) ) {
		wxLogError(_T("Binary to execute is not set (%s)"), PRO_CFG_TC_CURRENT_BINARY);
		play->SetLabel(_("Play"));
		play->Enable();
		return;
	}

	wxFileName path(folder, binary, wxPATH_NATIVE);
	if ( !path.FileExists() ) {
		wxLogError(_T("Binary %s does not exist"), path.GetFullName().c_str());
		play->SetLabel(_("Play"));
		play->Enable();
		return;
	}

	if ( ProMan::NoError != ProMan::PushProfile(p->Get()) ) {
		play->SetLabel(_("Play"));
		play->Enable();
		return;
	}

		wxString previousWorkingDir(::wxGetCwd());
	// hopefully this doesn't goof anything up
	if ( !::wxSetWorkingDirectory(folder) ) {
		wxLogError(_T("Unable to change working directory to %s"),
			folder.c_str());
		play->SetLabel(_T("Play"));
		play->Enable();
		return;
	}

	wxProcess* process = new wxProcess(this, ID_FS2_PROCESS);
	wxString formatString;
#if IS_APPLE
	formatString += _T("open ");
#endif
	formatString = _T("%s");
	wxString command(wxString::Format(formatString, path.GetFullPath().c_str()));
	long pid = ::wxExecute(command, wxEXEC_ASYNC, process);
	if ( pid == 0 ) {
		play->SetLabel(_("Play"));
		play->Enable();
		return;
	}
	this->FS2_pid = pid;
	wxLogInfo(_T("FreeSpace 2 Open is now running..."));

	if ( !::wxSetWorkingDirectory(previousWorkingDir) ) {
		wxLogError(_T("Unable to change back to working directory %s"),
			previousWorkingDir.c_str());
	}
}
void MainWindow::OnUpdate(wxCommandEvent& WXUNUSED(event)) {
	wxMessageBox(_("Update"));
}
void MainWindow::OnAbout(wxCommandEvent& WXUNUSED(event)) {
	wxMessageBox(_("About"));
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

	wxLogInfo(_T("FS2 Open exited with a status of %d"), exitCode);
	
	wxButton* play = dynamic_cast<wxButton*>(
		wxWindow::FindWindowById(ID_PLAY_BUTTON, this));
	wxCHECK_RET(play != NULL, _T("Unable to find play button"));
	play->SetLabel(_T("Play"));
	play->Enable();
	
}

