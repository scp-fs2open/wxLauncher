#include <wx/wx.h>
#include "wxIDS.h"
#include "BottomButtons.h"

BottomButtons::BottomButtons(wxWindow* parent, wxPoint &pos, wxSize &size) : wxPanel(
	parent, wxID_ANY, pos, size) {
		joblist = new JobTable();
		this->SetMinSize(size);

		// MODMOD: hard coded values
		this->enabledFont = new wxFont(12, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
		this->disabledFont = new wxFont(12, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_LIGHT);
		this->SetFont(wxFont(12, wxFONTFAMILY_DECORATIVE, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL));

		this->close = new wxButton(this, ID_CLOSE_BUTTON, _("Close"));
		this->help = new wxButton(this, ID_HELP_BUTTON, _("Help"));
		this->fred = new wxButton(this, ID_FRED_BUTTON, _("Fred"));
		this->log = new wxButton(this, ID_ABOUT_BUTTON, _("Ready."), wxDefaultPosition, wxDefaultSize, wxNO_BORDER);

		this->update = new wxButton(this, ID_UPDATE_BUTTON, _("Update Available"));
		this->play = new wxButton(this, ID_PLAY_BUTTON, _("Play"));

		this->bar = new wxGauge(this, wxID_ANY, 100);
		this->bar->SetMinSize(wxSize(150,-1));

		wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
		sizer->Add(bar); /* HACKHACK: hard coded spacer to move the help
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

/** Called when the launcher is not doing anything. Vanishes the progress bar and
dims the log button. */
void BottomButtons::Done() {
	this->log->Disable();
}

/** Check the internal numbers and make the changes to the log button and the progress
bar. */
void BottomButtons::UpdateBars() {
	wxMessageBox(wxString::Format(_("%d"), joblist->size()));
	if ( joblist->size() == 0 ) {
		this->log->SetFont(*(this->disabledFont));
		this->log->SetForegroundColour(*wxLIGHT_GREY); // MODMOD: hard coded values
		this->bar->Hide();
	} else {
		this->log->SetFont(*(this->enabledFont));
		this->log->SetForegroundColour(*wxBLACK); // MODMOD: hard coded values
		this->bar->Show();
	}
}