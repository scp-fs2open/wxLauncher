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

#ifndef WELCOMEPAGE_H
#define WELCOMEPAGE_H

#include <wx/wx.h>

#include "apis/SkinManager.h"
#include "global/ids.h"
#include "apis/ProfileManager.h"

class WelcomePage: public wxPanel {
public:
	WelcomePage(wxWindow* parent, SkinSystem* skin);
	void LinkClicked(wxHtmlLinkEvent& event);
	void LinkHover(wxHtmlCellEvent& event);
	void OnMouseOut(wxMouseEvent& event);
	void ProfileButtonClicked(wxCommandEvent& event);
	void SaveDefaultChecked(wxCommandEvent& event);
	void ProfileChanged(wxCommandEvent& event);
	void ProfileCountChanged(wxCommandEvent& event);
	void UpdateNews(wxIdleEvent& event);
	void OnDownloadNewsCheck(wxCommandEvent& event);
	void OnUpdateNewsHelp(wxCommandEvent& event);
	
private:
	/** The width of the items on the welcome tab. */
	const static int stuffWidth = TAB_AREA_WIDTH;
	void *lastLinkInfo;

	void cloneNewProfile(wxChoice* combobox, ProMan* profile);
	void deleteProfile(wxChoice* combobox, ProMan* profile);

	bool needToUpdateNews;
	bool getOrPromptUpdateNews();

	DECLARE_EVENT_TABLE();
};

#endif