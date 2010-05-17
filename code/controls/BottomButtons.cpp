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

#include "global/MemoryDebugging.h" // Last include for memory debugging


BottomButtons::BottomButtons(wxWindow* parent, wxPoint &pos, wxSize &size) : wxPanel(
	parent, wxID_ANY, pos, size) {
		this->SetMinSize(size);

		this->close = new wxButton(this, ID_CLOSE_BUTTON, _("Close"));
		this->help = new wxButton(this, ID_HELP_BUTTON, _("Help"));
		this->fred = NULL; // new wxButton(this, ID_FRED_BUTTON, _("Fred"));
		this->update = new wxButton(this, ID_UPDATE_BUTTON, _("Update Available"));
		this->update->Hide();
		this->play = new wxButton(this, ID_PLAY_BUTTON, _("Play"));
		wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(this->close);
		sizer->Add(this->help);
		sizer->AddStretchSpacer(1);
		sizer->Add(this->update, wxSizerFlags().ReserveSpaceEvenIfHidden());
		sizer->AddStretchSpacer(1);
		//sizer->Add(this->fred);
		sizer->Add(this->play);

		this->SetSizer(sizer);
		this->Layout();

		TCManager::RegisterTCBinaryChanged(this);
		TCManager::RegisterTCChanged(this);
		wxCommandEvent nullEvent;
		this->OnTCChanges(nullEvent);
}

BEGIN_EVENT_TABLE(BottomButtons, wxPanel)
EVT_COMMAND(wxID_NONE, EVT_TC_CHANGED, BottomButtons::OnTCChanges)
EVT_COMMAND(wxID_NONE, EVT_TC_BINARY_CHANGED, BottomButtons::OnTCChanges)
END_EVENT_TABLE()

void BottomButtons::OnTCChanges(wxCommandEvent &WXUNUSED(event)) {
	wxString tc, binary;
	ProMan::GetProfileManager()->Get()->Read(PRO_CFG_TC_ROOT_FOLDER, &tc, wxEmptyString);
	ProMan::GetProfileManager()->Get()->Read(PRO_CFG_TC_CURRENT_BINARY, &binary, wxEmptyString);
	if ( tc.IsEmpty() || binary.IsEmpty() ) {
		this->play->Disable();
	} else if ( wxFileName(tc, binary).FileExists() ) {
		this->play->Enable();
	} else {
		wxLogWarning(_("Executable %s does not exist"), wxFileName(tc, binary).GetFullName().c_str());
		this->play->Disable();
	}
}
