#include <wx/wx.h>
#include "ids.h"
#include "BottomButtons.h"
#include "TCManager.h"
#include "ProfileManager.h"

#include "wxLauncherSetup.h" // Last include for memory debugging


BottomButtons::BottomButtons(wxWindow* parent, wxPoint &pos, wxSize &size) : wxPanel(
	parent, wxID_ANY, pos, size) {
		this->SetMinSize(size);

		this->close = new wxButton(this, ID_CLOSE_BUTTON, _("Close"));
		this->help = new wxButton(this, ID_HELP_BUTTON, _("Help"));
		this->fred = new wxButton(this, ID_FRED_BUTTON, _("Fred"));
		this->update = new wxButton(this, ID_UPDATE_BUTTON, _("Update Available"));
		this->play = new wxButton(this, ID_PLAY_BUTTON, _("Play"));
		wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(this->close);
		sizer->Add(this->help);
		sizer->Add(this->update);
		sizer->Add(this->fred);
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
