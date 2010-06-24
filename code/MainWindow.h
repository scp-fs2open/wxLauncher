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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/toolbook.h>
#include <wx/process.h>
#include "apis/SkinManager.h"

class MainWindow: public wxFrame {
public:
	MainWindow(SkinSystem* skin);
	~MainWindow();

	void OnQuit(wxCommandEvent& event);
	void OnHelp(wxCommandEvent& event);
	void OnFSButton(wxCommandEvent& event);
	void OnStartFS(wxButton* play);
	void OnKillFS(wxButton* play);
	void OnStartFred(wxCommandEvent& event);
	void OnUpdate(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);
	void OnContextHelp(wxHelpEvent& event);
	void OnFS2Exited(wxProcessEvent& event);

private:
	wxProcess* process;
	wxToolbook* mainTab;
	long FS2_pid;

	DECLARE_EVENT_TABLE();
};

#endif
