#include <wx/wx.h>
#include "wxIDS.h"
#include "BottomButtons.h"
#include "StatusLog.h"

BottomButtons::BottomButtons(wxWindow* parent, wxPoint &pos, wxSize &size) : wxPanel(
	parent, wxID_ANY, pos, size) {

		wxButton* close = new wxButton(this, ID_CLOSE_BUTTON, _("Close"));
		wxButton* help = new wxButton(this, ID_HELP_BUTTON, _("Help"));
		wxButton* fred = new wxButton(this, ID_FRED_BUTTON, _("Fred"));
		wxButton* update = new wxButton(this, ID_UPDATE_BUTTON, _("Update Available"));
		wxButton* play = new wxButton(this, ID_PLAY_BUTTON, _("Play"));

		StatusLog* status = new StatusLog();
		wxPanel* gaugePanel = status->GetGaugePanel(this);
		wxPanel* infoPanel = status->GetInfoPanel(this);

		wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(gaugePanel);
		gaugePanel->SetMinSize(wxSize(150, -1));/* HACKHACK: hard coded spacer to move the help
							   button from under the tab list. */
		sizer->Add(close, wxSizerFlags().Center());
		sizer->Add(help, wxSizerFlags().Center());
		sizer->Add(infoPanel, wxSizerFlags().Proportion(10).Expand());
		sizer->Add(update, wxSizerFlags().Center().ReserveSpaceEvenIfHidden().Left());
		sizer->Add(fred, wxSizerFlags().Center().Left());
		sizer->Add(play, wxSizerFlags().Center().Left());

		this->SetSizer(sizer);
		this->Layout();
}