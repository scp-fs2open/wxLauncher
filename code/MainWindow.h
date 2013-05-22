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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/process.h>

class MainWindow: public wxFrame {
public:
	MainWindow();
	~MainWindow();

	void OnQuit(wxCommandEvent& event);
	void OnHelp(wxCommandEvent& event);
	void OnFSButton(wxCommandEvent& event);
	void OnStart(wxButton* button, bool startFred=false);
	void OnKill(wxButton* button, bool killFred=false);
	void OnStartFred(wxCommandEvent& event);
	void OnUpdate(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnContextHelp(wxHelpEvent& event);
	void OnFS2Exited(wxProcessEvent& event);
	void OnFRED2Exited(wxProcessEvent& event);
	void OnTCSkinChanged(wxCommandEvent& event);
	
	/** F3 toggles FRED launching. */
	void OnF3Pressed(wxCommandEvent& event);

private:
	wxProcess* process;
	wxNotebook* mainTab;
	long FS2_pid, FRED2_pid;

	DECLARE_EVENT_TABLE();
};

#endif
