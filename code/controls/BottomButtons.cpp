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
#include "global/ids.h"
#include "global/ProfileKeys.h"
#include "controls/BottomButtons.h"
#include "controls/ModList.h"
#include "datastructures/FSOExecutable.h"
#include "datastructures/ResolutionMap.h"
#include "apis/FREDManager.h"
#include "apis/TCManager.h"
#include "apis/ProfileManager.h"

#include "generated/configure_launcher.h" // needed to check whether OSX is used

#include "global/MemoryDebugging.h" // Last include for memory debugging


BottomButtons::BottomButtons(wxWindow* parent, wxPoint &pos, wxSize &size) : wxPanel(
	parent, wxID_ANY, pos, size) {
		this->SetMinSize(size);

		bool showFred;
		ProMan::GetProfileManager()->GlobalRead(GBL_CFG_OPT_CONFIG_FRED, &showFred, false);
#if 0
		this->close = new wxButton(this, ID_CLOSE_BUTTON, _("Close"));
#endif
		this->help = new wxButton(this, ID_HELP_BUTTON, _("Help"));
		this->about = new wxButton(this, ID_ABOUT_BUTTON, _("About"));
		this->fred = new wxButton(this, ID_FRED_BUTTON, _("FRED"));
		this->fred->Show(showFred);

#if 0
		this->update = new wxButton(this, ID_UPDATE_BUTTON, _("Update Available"));
		this->update->Hide();
#endif
		this->play = new wxButton(this, ID_PLAY_BUTTON, _("Play"));
		wxFont playButtonFont = this->play->GetFont();
		playButtonFont.SetWeight(wxFONTWEIGHT_BOLD);
		this->play->SetFont(playButtonFont);
		wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
#if 0
		sizer->Add(this->close);
#endif
		sizer->Add(this->help);
		sizer->Add(this->about);
		sizer->AddStretchSpacer(1);
#if 0
		sizer->Add(this->update, wxSizerFlags().ReserveSpaceEvenIfHidden());
		sizer->AddStretchSpacer(1);
#endif
		sizer->Add(this->fred, wxSizerFlags().ReserveSpaceEvenIfHidden());
		sizer->Add(this->play);

		this->SetSizerAndFit(sizer);
		this->Layout();

		wxLogDebug(_T("BottomButtons is at %p."), this);

		TCManager::RegisterTCBinaryChanged(this);
		TCManager::RegisterTCChanged(this);
		TCManager::RegisterTCActiveModChanged(this);
		TCManager::RegisterTCFredBinaryChanged(this);
		FREDManager::RegisterFREDEnabledChanged(this);
		ResolutionMap::RegisterResolutionMapChanged(this);
		wxCommandEvent nullEvent;
		this->OnTCChanges(nullEvent);
}

BEGIN_EVENT_TABLE(BottomButtons, wxPanel)
EVT_COMMAND(wxID_NONE, EVT_TC_CHANGED, BottomButtons::OnTCChanges)
EVT_COMMAND(wxID_NONE, EVT_TC_ACTIVE_MOD_CHANGED, BottomButtons::OnTCChanges)
EVT_COMMAND(wxID_NONE, EVT_TC_BINARY_CHANGED, BottomButtons::OnTCChanges)
EVT_COMMAND(wxID_NONE, EVT_TC_FRED_BINARY_CHANGED, BottomButtons::OnTCChanges)
EVT_COMMAND(wxID_NONE, EVT_FRED_ENABLED_CHANGED, BottomButtons::OnFREDEnabledChanged)
EVT_COMMAND(wxID_NONE, EVT_RESOLUTION_MAP_CHANGED, BottomButtons::OnTCChanges)
END_EVENT_TABLE()

/** remove the text after ".app" in the executable name.
 A no op on other platforms*/
#if IS_APPLE
wxString FixBinaryName(const wxString& binaryName) {
	wxString fixedBinaryName(binaryName);
	// the trailing / ensures that the .app indicates an extension
	int DotAppIndex = binaryName.Find(_T(".app/"));
	if (DotAppIndex != wxNOT_FOUND) {
		fixedBinaryName = binaryName.Left(DotAppIndex + 4); // 4 so that ".app" is retained
	}
	return fixedBinaryName;
}
#else
#define FixBinaryName(binaryName) binaryName
#endif

void BottomButtons::OnTCChanges(wxCommandEvent &WXUNUSED(event)) {
	wxString tc, binary, fredBinary;
	ProMan::GetProfileManager()->ProfileRead(PRO_CFG_TC_ROOT_FOLDER, &tc, wxEmptyString);
	ProMan::GetProfileManager()->ProfileRead(PRO_CFG_TC_CURRENT_BINARY, &binary, wxEmptyString);
	if ( tc.IsEmpty() || binary.IsEmpty() || ModList::GetActiveMod() == NULL || !ResolutionMap::HasEntryForActiveMod()) {
		this->play->Disable();
	} else if ( wxFileName(tc + wxFileName::GetPathSeparator() + binary).FileExists() ) {
		this->play->Enable();
	} else {
		wxLogWarning(_("Executable %s does not exist"), FixBinaryName(binary).c_str());
		this->play->Disable();
	}
	ProMan::GetProfileManager()->ProfileRead(PRO_CFG_TC_CURRENT_FRED, &fredBinary, wxEmptyString);
	if ( this->fred == NULL ) {
		// do nothing, no button to manipulate
	} else if ( tc.IsEmpty() || fredBinary.IsEmpty() || (!wxFileName::DirExists(tc)) ||
		(!FSOExecutable::HasFSOExecutables(wxFileName(tc, wxEmptyString))) ) {
		this->fred->Disable();
#if IS_APPLE
	} else if ( wxFileName(tc + wxFileName::GetPathSeparator() + fredBinary).FileExists() ) {
#else
	} else if ( wxFileName(tc, fredBinary).FileExists() ) {
#endif
		this->fred->Enable();
	} else {
		wxLogWarning(_("FRED executable %s does not exist"), FixBinaryName(fredBinary).c_str());
		this->fred->Disable();
	}
}

void BottomButtons::OnFREDEnabledChanged(wxCommandEvent &WXUNUSED(event)) {
	bool showFred;
	ProMan::GetProfileManager()->GlobalRead(GBL_CFG_OPT_CONFIG_FRED, &showFred, false);
	
	this->fred->Show(showFred);
}
