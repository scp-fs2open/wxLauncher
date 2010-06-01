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

#ifndef STATUSLOG_H
#define STATUSLOG_H

#include <wx/wx.h>
#include "global/ids.h"

/** Extends wxStatusBar.  Provides the a 4 section status bar: icon, main text,
progress bar, and small text for the progress bar.  This class will interface
this Logger to enable code to eaisly send messages to the statusbar. */
class StatusBar: public wxStatusBar {
public:
	StatusBar(wxWindow* parent);
	virtual ~StatusBar();

	void OnSize(wxSizeEvent& event);

	void SetMainStatusText(wxString msg, int icon=ID_SB_NO_CHANGE);
	void SetJobStatusText(int value, wxString msg=_T(""));
	void StartToolTipStatusText(wxString msg);
	void EndToolTipStatusText();

private:
	wxWindow* parent;
	wxBitmap icons[ID_SB_MAX_ID];
	bool showingToolTip;
	
	DECLARE_EVENT_TABLE();
};

#endif