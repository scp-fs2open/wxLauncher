#include <wx/wx.h>
#include "wxIDS.h"
#include "BottomButtons.h"
#include "StatusLog.h"

BottomButtons::BottomButtons(wxWindow* parent, wxPoint &pos, wxSize &size) : wxPanel(
	parent, wxID_ANY, pos, size) {
		this->SetMinSize(size);

		this->close = new wxButton(this, ID_CLOSE_BUTTON, _("Close"));
		this->help = new wxButton(this, ID_HELP_BUTTON, _("Help"));
		this->fred = new wxButton(this, ID_FRED_BUTTON, _("Fred"));
		this->update = new wxButton(this, ID_UPDATE_BUTTON, _("Update Available"));
		this->play = new wxButton(this, ID_PLAY_BUTTON, _("Play"));
		StatusLog* statusLog = new StatusLog();
		this->log = statusLog->GetInfoPanel(this);
		this->gaugePanel = statusLog->GetGaugePanel(this);
		wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(this->gaugePanel); /* HACKHACK: hard coded spacer to move the help
		gaugePanel->SetMinSize(wxSize(150, -1));/* HACKHACK: hard coded spacer to move the help
							   button from under the tab list. */
		sizer->Add(this->close);
		sizer->Add(this->help);
		sizer->Add(this->log,2);
		sizer->Add(this->update);
		sizer->Add(this->fred);
		sizer->Add(this->play);

		this->SetSizer(sizer);
		this->Layout();
}
