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

#include <wx/wx.h>
#include <wx/filename.h>
#include "apis/SkinManager.h"
#include "controls/StatusBar.h"
#include "global/ids.h"
#include "controls/Logger.h"
#include "generated/configure_launcher.h"

#include "global/MemoryDebugging.h" // Last include for memory debugging

enum FieldIDs {
	SB_FIELD_ICON = 0,
	SB_FIELD_MAINTEXT,
	SB_FIELD_PROGRESS_BAR,
	SB_FIELD_PROGRESS_TEXT,
	SB_FIELD_MAX,
};

#if IS_APPLE
const int ICON_FIELD_WIDTH = 18;
#else
const int ICON_FIELD_WIDTH = 25;
#endif

BEGIN_EVENT_TABLE(StatusBar, wxStatusBar)
EVT_SIZE(StatusBar::OnSize)
EVT_COMMAND(wxID_NONE, EVT_TC_SKIN_CHANGED, StatusBar::OnTCSkinChanged)
END_EVENT_TABLE()

StatusBar::StatusBar(wxWindow *parent)
		:wxStatusBar(parent) {
	this->parent = parent;
	this->showingToolTip = false;
	
	SkinSystem::RegisterTCSkinChanged(this);
	
	wxCommandEvent nullEvent;
	OnTCSkinChanged(nullEvent);

	// Just creating these now, will place them in the OnSize event handler
	new wxStaticBitmap(this, ID_STATUSBAR_STATUS_ICON, this->icons[ID_SB_OK]);
#if 0 // this progress bar doesn't need to be here
	new wxGauge(this, ID_STATUSBAR_PROGRESS_BAR, 100);
#endif
	// eliminate (by zeroing the width) the fields we're currently not using
	// FIXME and when we start using those fields, add the width back!
	int widths[] = { ICON_FIELD_WIDTH, -1, 0, 0 };

	wxASSERT_MSG( sizeof(widths)/sizeof(int) == SB_FIELD_MAX,
		wxString::Format(
			_T("Number of fields (%d) and number of widths (%d) do not match"),
			SB_FIELD_MAX, sizeof(widths)/sizeof(int)));

	this->SetFieldsCount(SB_FIELD_MAX, widths);

	this->SetStatusText(_T("Status bar created"), SB_FIELD_MAINTEXT);

	dynamic_cast<Logger*>(wxLog::GetActiveTarget())->SetStatusBarTarget(this);
}

StatusBar::~StatusBar() {
	// logger could be null if Logger already destroyed, so must check
	Logger* logger = dynamic_cast<Logger*>(wxLog::GetActiveTarget());
	if (logger) {
		logger->SetStatusBarTarget(NULL);
	}
}

void StatusBar::OnSize(wxSizeEvent& WXUNUSED(event)) {
	wxWindow* icon = dynamic_cast<wxWindow*>(wxWindow::FindWindowById(ID_STATUSBAR_STATUS_ICON, this));
	wxCHECK_RET( icon != NULL, _T("Cannot find status bar icon control"));
	
	wxRect iconrect;
	this->GetFieldRect(SB_FIELD_ICON, iconrect);
	// OS X-only hack to keep the icon from being clipped off
#if IS_APPLE
	iconrect.SetX(-3);
	iconrect.SetY(-2);
#endif
	icon->SetSize(iconrect);

#if 0 // this progress bar doesn't need to be here
	wxWindow* bar = dynamic_cast<wxWindow*>(wxWindow::FindWindowById(ID_STATUSBAR_PROGRESS_BAR, this));
	wxCHECK_RET( bar != NULL, _T("Cannot find status bar progress bar"));

	wxRect barrect;
	this->GetFieldRect(SB_FIELD_PROGRESS_BAR, barrect);
	bar->SetSize(barrect);
#endif
}

void StatusBar::OnTCSkinChanged(wxCommandEvent &WXUNUSED(event)) {
	this->icons[ID_SB_OK] = SkinSystem::GetSkinSystem()->GetOkIcon();
	
	this->icons[ID_SB_WARNING] = SkinSystem::GetSkinSystem()->GetWarningIcon();
	
	this->icons[ID_SB_ERROR] = SkinSystem::GetSkinSystem()->GetErrorIcon();
	
	this->icons[ID_SB_INFO] = SkinSystem::GetSkinSystem()->GetInfoIcon();
	
	for( int i = 0; i < ID_SB_MAX_ID; i++) { // Check that all icons are okay.
		if ( !this->icons[i].IsOk() ) {
			this->icons[i] = wxNullBitmap;
			wxLogWarning(_T("Icon ID: %d is not okay"), i);
		}
	}	
}

void StatusBar::SetMainStatusText(wxString msg, int icon) {
	this->SetStatusText(msg, SB_FIELD_MAINTEXT);
	if ( icon > ID_SB_NO_CHANGE && icon < ID_SB_MAX_ID ) {
		wxStaticBitmap* iconImage = dynamic_cast<wxStaticBitmap*>(wxWindow::FindWindowById(ID_STATUSBAR_STATUS_ICON, this));
  wxCHECK_RET( iconImage != NULL, _T("Cannot find status bar icon image"));

		iconImage->SetBitmap(this->icons[icon]);
		iconImage->Refresh();
	} else {
		if ( icon != ID_SB_NO_CHANGE ) {
			wxLogWarning(_T("SetMainStatusText was passed (%d) as an icon, which is out of range"), icon);
		}
	}
}

/** Causes the status bar to show the msg until EndToolTipStatusText() is
called.  When EndToolTipStatusText() is called the status text will be returned
to the original text. */
void StatusBar::StartToolTipStatusText(wxString msg) {
	if (!this->showingToolTip) {
		this->PushStatusText(msg, SB_FIELD_MAINTEXT);
		this->showingToolTip = true;
	}
}

/** Returns the statusbar text to the orginal message after a mouse over.
See StartToolTipStatusText() for more information. */
void StatusBar::EndToolTipStatusText() {
	if (this->showingToolTip) {
		this->PopStatusText(SB_FIELD_MAINTEXT);
		this->showingToolTip = false;
	}
}
