#include <wx/wx.h>
#include "wxIDS.h"
#include "BottomButtons.h"

BottomButtons::BottomButtons(wxWindow* parent, wxPoint &pos, wxSize &size) : wxPanel(
	parent, wxID_ANY, pos, size) {

		wxButton* close = new wxButton(this, ID_CLOSE_BUTTON, _("Close"));
		wxButton* help = new wxButton(this, ID_HELP_BUTTON, _("Help"));
		wxButton* fred = new wxButton(this, ID_FRED_BUTTON, _("Fred"));

		wxButton* update = new wxButton(this, ID_UPDATE_BUTTON, _("Update Aviable"));
		wxButton* play = new wxButton(this, ID_PLAY_BUTTON, _("Play"));

		wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
		sizer->AddSpacer(150); /* HACKHACK: hard coded spacer to move the help
							   button from under the tab list. */
		sizer->Add(close);
		sizer->Add(help);
		sizer->AddStretchSpacer(10);
		sizer->Add(update);
		sizer->Add(fred);
		sizer->Add(play);

		this->SetSizer(sizer);
		this->Layout();
}