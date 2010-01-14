#include "AdvSettingsPage.h"

#include "TCManager.h"
#include "ProfileManager.h"
#include "FlagList.h"
#include "ids.h"

#include <wx/html/htmlwin.h>
#include <windows.h>

#include "wxLauncherSetup.h" // Last include for memory debugging

AdvSettingsPage::AdvSettingsPage(wxWindow* parent, SkinSystem *skin): wxPanel(parent, wxID_ANY) {
	this->skin = skin;
	this->SetMinSize(wxSize(TAB_AREA_WIDTH, TAB_AREA_HEIGHT));

	TCManager::RegisterTCBinaryChanged(this);
	wxCommandEvent nullEvent;
	this->OnExeChanged(nullEvent);	
}

BEGIN_EVENT_TABLE(AdvSettingsPage, wxPanel)
EVT_COMMAND(wxID_NONE, EVT_TC_BINARY_CHANGED, AdvSettingsPage::OnExeChanged)
END_EVENT_TABLE()

void AdvSettingsPage::OnExeChanged(wxCommandEvent& WXUNUSED(event)) {
	if (this->GetSizer() != NULL) {
		this->GetSizer()->DeleteWindows();
	}

	FlagListBox* list = new FlagListBox(this, this->skin);
	wxHtmlWindow* description = new wxHtmlWindow(this);
	description->SetPage(_T("<p>Test Page</p>"));
	
	wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
	topSizer->Add(list, wxSizerFlags().Proportion(2).Expand());
	topSizer->Add(description, wxSizerFlags().Proportion(1).Expand());
	topSizer->SetMinSize(TAB_AREA_WIDTH, TAB_AREA_HEIGHT/2);


	wxStaticBitmap* idealIcon = new wxStaticBitmap(this, wxID_ANY, this->skin->GetIdealIcon());
	wxStaticText* idealLabel = new wxStaticText(this, wxID_ANY, _("= Recommended flag"));
	wxChoice* flagSetChoice = new wxChoice(this, ID_SELECT_FLAG_SET);

	wxBoxSizer* idealFlagsRowSizer = new wxBoxSizer(wxHORIZONTAL);
	idealFlagsRowSizer->Add(idealIcon);
	idealFlagsRowSizer->Add(idealLabel);
	idealFlagsRowSizer->AddStretchSpacer(1);
	idealFlagsRowSizer->Add(flagSetChoice, wxSizerFlags().Left());

	wxTextCtrl* flagsetNotes = new wxTextCtrl(this, ID_FLAG_SET_NOTES_TEXT);
	flagsetNotes->Disable();

	wxStaticText* customFlagsLabel = new wxStaticText(this, wxID_ANY, _("Custom flags:"));
	wxTextCtrl* customFlagsText = new wxTextCtrl(this, ID_CUSTOM_FLAGS_TEXT);
	wxBoxSizer* customFlagsSizer = new wxBoxSizer(wxHORIZONTAL);
	customFlagsSizer->Add(customFlagsLabel);
	customFlagsSizer->Add(customFlagsText);
	
	wxStaticText* commandLineLabel = new wxStaticText(this, wxID_ANY, _("Current commandline"));
	wxTextCtrl* commandLineText = new wxTextCtrl(this, ID_COMMAND_LINE_TEXT);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(topSizer);
	sizer->Add(idealFlagsRowSizer);//, wxSizerFlags().Expand());
	sizer->Add(flagsetNotes);//, wxSizerFlags().Expand());
	sizer->Add(customFlagsSizer);//, wxSizerFlags().Expand());
	sizer->Add(commandLineLabel);
	sizer->Add(commandLineText);//, wxSizerFlags().Expand());

	this->SetSizer(sizer);
	this->SetMaxSize(wxSize(TAB_AREA_WIDTH, TAB_AREA_HEIGHT));
	this->Layout();
}

	
	
