#include <wx/wx.h>
#include <wx/gdicmn.h>
#include <wx/toolbook.h>
#include "wxIDS.h"
#include "MainWindow.h"
#include "WelcomePage.h"
#include "ModsPage.h"
#include "BasicSettingsPage.h"
#include "AdvSettingsPage.h"
#include "InstallPage.h"

#define MAINWINDOW_STYLE (wxBORDER_SUNKEN | wxBORDER_SIMPLE | wxSYSTEM_MENU\
	| wxCAPTION | wxCLOSE_BOX | wxCLIP_CHILDREN | wxMINIMIZE_BOX)

MainWindow::MainWindow() : wxFrame((wxFrame*)NULL, wxID_ANY,
								   _("wxLauncher for the FreeSpace Source Code Project"),
								   wxDefaultPosition,
								   wxSize(800, 600),
								   MAINWINDOW_STYLE)
{
	Centre();
	wxFont* windowFont = wxFont::New(12, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_NORMAL,
		wxFONTWEIGHT_BOLD, false);
	SetFont((*windowFont));

	// setup statusbar
	wxStatusBar* statusbar = CreateStatusBar(2);
	SetStatusText(_("Loading... Please Wait"),0);

	// setup tabs

	// Images used by wxImageList must be all the same dimentions
	wxImageList* images = new wxImageList(64,64);
	images->Add(wxBitmap(_T("welcome.bmp"), wxBITMAP_TYPE_BMP));
	images->Add(wxBitmap(_T("mods.bmp"), wxBITMAP_TYPE_BMP));
	images->Add(wxBitmap(_T("basic.bmp"), wxBITMAP_TYPE_BMP));
	images->Add(wxBitmap(_T("advanced.bmp"), wxBITMAP_TYPE_BMP));
	images->Add(wxBitmap(_T("install.bmp"), wxBITMAP_TYPE_BMP));

	this->mainTab = new wxToolbook(this, ID_MAINTAB, wxPoint(0,0), wxSize(800,600),	wxNB_LEFT);
	this->mainTab->AssignImageList(images);
	this->mainTab->AddPage(new WelcomePage(this->mainTab), _("Welcome"), true, ID_TAB_WELCOME_IMAGE);
	this->mainTab->AddPage(new ModsPage(this->mainTab), _("Mods"), false, ID_TAB_MOD_IMAGE);
	this->mainTab->AddPage(new BasicSettingsPage(this->mainTab), _("Basic Settings"), false, ID_TAB_BASIC_SETTINGS_IMAGE);
	this->mainTab->AddPage(new AdvSettingsPage(this->mainTab), _("Advanced Settings"), false, ID_TAB_ADV_SETTINGS_IMAGE);
	this->mainTab->AddPage(new InstallPage(this->mainTab), _("Install/Update"), false, ID_TAB_INSTALL_IMAGE);

	wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
	mainSizer->Add(this->mainTab);
	SetSizer(mainSizer, true);

	Layout();

}

MainWindow::~MainWindow() {
	//
}


