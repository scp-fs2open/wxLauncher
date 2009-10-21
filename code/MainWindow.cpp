
#include "wxIDS.h"
#include "MainWindow.h"

MainWindow::MainWindow() : wxFrame((wxFrame*)NULL, wxID_ANY, _("wxLauncher for the FreeSpace Source Code Project"),
		wxDefaultPosition, wxSize(800, 600), wxCLOSE_BOX)
{
	Centre();

	// setup statusbar
	wxStatusBar* statusbar = CreateStatusBar(2);
	SetStatusText(_("Loading... Please Wait"),0);

	// setup tabs

}

MainWindow::~MainWindow() {
	//
}


