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

#ifndef WXLAUNCHERAPP_H
#define WXLAUNCHERAPP_H
#include <wx/wx.h>
#include "apis/SkinManager.h"
#include "apis/ProfileManagerOperator.h"

class wxLauncher: public wxApp {
public:
	wxLauncher();
	virtual bool OnInit();
	virtual int OnExit();
	virtual int OnRun();

	virtual void OnInitCmdLine(wxCmdLineParser& parser);
	virtual bool OnCmdLineParsed(wxCmdLineParser& parser);

	wxString mFileOperand;
	wxString mProfileOperand;
	ProManOperator::profileOperator mProfileOperator;
	bool mKeepForSessionOnly;
	bool mShowGUI;
};

DECLARE_APP(wxLauncher);

#endif
