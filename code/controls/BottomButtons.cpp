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
#include "controls/BottomButtons.h"
#include "apis/TCManager.h"
#include "apis/ProfileManager.h"

#include "generated/configure_launcher.h" // needed to check whether OSX is used

#include "global/MemoryDebugging.h" // Last include for memory debugging


BottomButtons::BottomButtons(wxWindow* parent, wxPoint &pos, wxSize &size) : wxPanel(
	parent, wxID_ANY, pos, size) {
		this->SetMinSize(size);

		bool showFred;
		ProMan::GetProfileManager()->Global()->Read(GBL_CFG_OPT_CONFIG_FRED, &showFred, false);

		this->close = new wxButton(this, ID_CLOSE_BUTTON, _("Close"));
		this->help = new wxButton(this, ID_HELP_BUTTON, _("Help"));
		if ( showFred ) {
			this->fred = new wxButton(this, ID_FRED_BUTTON, _("Fred"));
		} else {
			this->fred = NULL;
		}
		this->update = new wxButton(this, ID_UPDATE_BUTTON, _("Update Available"));
		this->update->Hide();
		this->play = new wxButton(this, ID_PLAY_BUTTON, _("Play"));
		wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(this->close);
		sizer->Add(this->help);
		sizer->AddStretchSpacer(1);
		sizer->Add(this->update, wxSizerFlags().ReserveSpaceEvenIfHidden());
		sizer->AddStretchSpacer(1);
		if ( this->fred != NULL ) {
			sizer->Add(this->fred);
		}
		sizer->Add(this->play);

		this->SetSizer(sizer);
		this->Layout();

		TCManager::RegisterTCBinaryChanged(this);
		TCManager::RegisterTCChanged(this);
		TCManager::RegisterTCFredBinaryChanged(this);
		wxCommandEvent nullEvent;
		this->OnTCChanges(nullEvent);
}

BEGIN_EVENT_TABLE(BottomButtons, wxPanel)
EVT_COMMAND(wxID_NONE, EVT_TC_CHANGED, BottomButtons::OnTCChanges)
EVT_COMMAND(wxID_NONE, EVT_TC_BINARY_CHANGED, BottomButtons::OnTCChanges)
EVT_COMMAND(wxID_NONE, EVT_TC_FRED_BINARY_CHANGED, BottomButtons::OnTCChanges)
END_EVENT_TABLE()

void BottomButtons::OnTCChanges(wxCommandEvent &WXUNUSED(event)) {
	wxString tc, binary, fredBinary;
	ProMan::GetProfileManager()->Get()->Read(PRO_CFG_TC_ROOT_FOLDER, &tc, wxEmptyString);
	ProMan::GetProfileManager()->Get()->Read(PRO_CFG_TC_CURRENT_BINARY, &binary, wxEmptyString);
	if ( tc.IsEmpty() || binary.IsEmpty() ) {
		this->play->Disable();
#if IS_APPLE
	} else if ( wxFileName(tc + wxFileName::GetPathSeparator() + binary).FileExists() ) {
#else
	} else if ( wxFileName(tc, binary).FileExists() ) {
#endif
		this->play->Enable();
	} else {
#if IS_APPLE
		wxLogWarning(_("Executable %s does not exist"),
					 wxFileName(tc + wxFileName::GetPathSeparator() + binary).GetFullName().c_str());
#else
		wxLogWarning(_("Executable %s does not exist"), wxFileName(tc, binary).GetFullName().c_str());
#endif
		this->play->Disable();
	}
	ProMan::GetProfileManager()->Get()->Read(PRO_CFG_TC_CURRENT_FRED, &fredBinary, wxEmptyString);
	if ( this->fred == NULL ) {
		// do nothing, no button to manipulate
	} else if ( tc.IsEmpty() || fredBinary.IsEmpty() ) {
		this->fred->Disable();
	} else if ( wxFileName(tc, fredBinary).FileExists() ) {
		this->fred->Enable();
	} else {
		wxLogWarning(_("Fred binary %s does not exist"), wxFileName(tc, fredBinary).GetFullName().c_str());
		this->fred->Disable();
	}
}
