#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/toolbook.h>
#include <wx/process.h>
#include "Skin.h"

class MainWindow: public wxFrame {
public:
	MainWindow(SkinSystem* skin);
	~MainWindow();

	void OnQuit(wxCommandEvent& event);
	void OnHelp(wxCommandEvent& event);
	void OnStartFS(wxCommandEvent& event);
	void OnStartFred(wxCommandEvent& event);
	void OnUpdate(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnHelp(wxHelpEvent& event);
	void OnFS2Exited(wxProcessEvent& event);

private:
	wxToolbook* mainTab;
	long FS2_pid;

	DECLARE_EVENT_TABLE();
};

#endif