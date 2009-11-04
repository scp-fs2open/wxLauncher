#include <wx/wx.h>
#include "AdvSettingsPage.h"

#include "wxLauncherSetup.h" // Last include for memory debugging

AdvSettingsPage::AdvSettingsPage(wxWindow* parent): wxWindow(parent, wxID_ANY) {
	// do nothing
	wxGauge* gauge = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition, wxSize(50, 100));
	gauge->SetValue(50);

	wxBoxSizer* boxSizer = new wxBoxSizer(wxVERTICAL);
	boxSizer->Add(gauge);

	SetSizer(boxSizer);
	Layout();
	
}