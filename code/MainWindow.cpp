#include <wx/wx.h>
#include <wx/gdicmn.h>
#include <wx/toolbook.h>
#include <wx/imagpng.h>
#include <wx/html/htmlwin.h>
#include "wxIDS.h"
#include "MainWindow.h"
#include "WelcomePage.h"
#include "ModsPage.h"
#include "BasicSettingsPage.h"
#include "AdvSettingsPage.h"
#include "InstallPage.h"
#include "BottomButtons.h"
#include "Skin.h"

#include "wxLauncherSetup.h" // Last include for memory debugging

#define MAINWINDOW_STYLE (wxBORDER_SUNKEN | wxBORDER_SIMPLE | wxSYSTEM_MENU\
	| wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN | wxMINIMIZE_BOX)

MainWindow::MainWindow(SkinSystem* skin) {
	this->Create((wxFrame*)NULL, wxID_ANY, skin->GetTitle(),
		wxDefaultPosition, wxSize(800, 600), MAINWINDOW_STYLE);

	this->SetFont(skin->GetFont());

	// setup tabs

	// Images used by wxImageList must be all the same dimentions
	wxImageList* images = new wxImageList(64,64);
	images->Add(skin->GetWelcomeIcon());
	images->Add(skin->GetModsIcon());
	images->Add(skin->GetBasicIcon());
	images->Add(skin->GetAdvancedIcon());
	images->Add(skin->GetInstallIcon());

	this->mainTab = new wxToolbook();
	this->SetBackgroundStyle(wxBG_STYLE_COLOUR);
	this->SetBackgroundColour(*wxWHITE);
	this->mainTab->Create(this, ID_MAINTAB, wxPoint(0,0), wxSize(800,-1),	wxNB_LEFT);
	this->mainTab->AssignImageList(images);
	this->mainTab->AddPage(new WelcomePage(this->mainTab, skin), _("Welcome"), true, ID_TAB_WELCOME_IMAGE);
	this->mainTab->AddPage(new ModsPage(this->mainTab, skin), _("Mods"), false, ID_TAB_MOD_IMAGE);
	this->mainTab->AddPage(new BasicSettingsPage(this->mainTab), _("Basic Settings"), false, ID_TAB_BASIC_SETTINGS_IMAGE);
	this->mainTab->AddPage(new AdvSettingsPage(this->mainTab), _("Advanced Settings"), false, ID_TAB_ADV_SETTINGS_IMAGE);
	this->mainTab->AddPage(new InstallPage(this->mainTab), _("Install/Update"), false, ID_TAB_INSTALL_IMAGE);

	BottomButtons* bb = new BottomButtons(this, wxPoint(0, -1), wxSize(800, -1));

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
END_EVENT_TABLE()

void MainWindow::OnQuit(wxCommandEvent& event) {
	wxMessageBox(_("Quit"));
}
void MainWindow::OnHelp(wxCommandEvent& event) {
	wxMessageBox(_("Help"));
}
void MainWindow::OnStartFred(wxCommandEvent& event) {
	wxMessageBox(_("Start Fred"));
}
void MainWindow::OnStartFS(wxCommandEvent& event) {
	wxMessageBox(_("Start FS"));
}
void MainWindow::OnUpdate(wxCommandEvent& event) {
	wxMessageBox(_("Update"));
}
void MainWindow::OnAbout(wxCommandEvent& event) {
	wxMessageBox(_("About"));
}


