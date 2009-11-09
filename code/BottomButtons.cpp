#include <wx/wx.h>
#include "wxIDS.h"
#include "BottomButtons.h"

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
}
