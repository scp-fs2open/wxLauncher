#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/toolbook.h>
#include "Skin.h"

class MainWindow: public wxFrame {
public:
	MainWindow(SkinSystem* skin);
	~MainWindow();

	void OnQuit(wxCommandEvent& WXUNUSED(event));
	void OnHelp(wxCommandEvent& WXUNUSED(event));
	void OnStartFS(wxCommandEvent& WXUNUSED(event));
	void OnStartFred(wxCommandEvent& WXUNUSED(event));
	void OnUpdate(wxCommandEvent& WXUNUSED(event));
	void OnAbout(wxCommandEvent& WXUNUSED(event));

private:
	wxToolbook* mainTab;

	DECLARE_EVENT_TABLE();
};

#endif