#include "AdvSettingsPage.h"

#include "TCManager.h"
#include "ProfileManager.h"
#include "ids.h"

#include <wx/html/htmlwin.h>
#include <wx/tokenzr.h>

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
EVT_TEXT(ID_CUSTOM_FLAGS_TEXT, AdvSettingsPage::OnNeedUpdateCommandLine)
END_EVENT_TABLE()

void AdvSettingsPage::OnExeChanged(wxCommandEvent& event) {
	if (this->GetSizer() != NULL) {
		this->GetSizer()->DeleteWindows();
	}

	this->flagListBox = new FlagListBox(this, this->skin);
	wxHtmlWindow* description = new wxHtmlWindow(this);
	description->SetPage(_T("<p>Test Page</p>"));
	
	wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
	topSizer->Add(this->flagListBox, wxSizerFlags().Proportion(2).Expand());
	topSizer->Add(description, wxSizerFlags().Proportion(1).Expand());
	topSizer->SetMinSize(TAB_AREA_WIDTH, TAB_AREA_HEIGHT/2);


	wxStaticBitmap* idealIcon = new wxStaticBitmap(this, wxID_ANY, this->skin->GetIdealIcon());
	wxStaticText* idealLabel = new wxStaticText(this, wxID_ANY, _("= Recommended flag"));
	wxChoice* flagSetChoice = new wxChoice(this, ID_SELECT_FLAG_SET);

	wxBoxSizer* idealFlagsRowSizer = new wxBoxSizer(wxHORIZONTAL);
	idealFlagsRowSizer->Add(idealIcon);
	wxBoxSizer* idealLabelSizer = new wxBoxSizer(wxVERTICAL);
	idealLabelSizer->AddStretchSpacer(1);
	idealLabelSizer->Add(idealLabel);
	idealLabelSizer->AddStretchSpacer(1);
	idealFlagsRowSizer->Add(idealLabelSizer);
	idealFlagsRowSizer->AddStretchSpacer(1);
	idealFlagsRowSizer->Add(flagSetChoice, wxSizerFlags().Left());

	wxStaticBox* flagsetNotesBox = new wxStaticBox(this, wxID_ANY, _("Flag set notes"));
	wxTextCtrl* flagsetNotes = new wxTextCtrl(this, ID_FLAG_SET_NOTES_TEXT,
		wxEmptyString, wxDefaultPosition, wxSize(-1, TAB_AREA_HEIGHT/8),
		wxTE_MULTILINE|wxTE_READONLY);
	wxStaticBoxSizer* flagsetNotesSizer = new wxStaticBoxSizer(flagsetNotesBox, wxVERTICAL);
	flagsetNotesSizer->Add(flagsetNotes, wxSizerFlags().Expand());

	wxStaticBox* customFlagsBox = new wxStaticBox(this, wxID_ANY, _("Custom flags"));
	wxTextCtrl* customFlagsText = new wxTextCtrl(this, ID_CUSTOM_FLAGS_TEXT);
	wxStaticBoxSizer* customFlagsSizer = new wxStaticBoxSizer(customFlagsBox, wxVERTICAL);
	customFlagsSizer->Add(customFlagsText, wxSizerFlags().Expand());
	
	wxStaticBox* commandLineLabel = new wxStaticBox(this, wxID_ANY, _("Current commandline"));
	wxTextCtrl* commandLineText = new wxTextCtrl(this, ID_COMMAND_LINE_TEXT,
		wxEmptyString, wxDefaultPosition, wxSize(-1, TAB_AREA_HEIGHT/8),
		wxTE_MULTILINE|wxTE_READONLY);
	wxStaticBoxSizer* commandLineSizer = new wxStaticBoxSizer(commandLineLabel, wxVERTICAL);
	commandLineSizer->Add(commandLineText, wxSizerFlags().Expand());

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(topSizer);
	sizer->Add(idealFlagsRowSizer, wxSizerFlags().Expand());
	sizer->Add(flagsetNotesSizer, wxSizerFlags().Expand());
	sizer->Add(customFlagsSizer, wxSizerFlags().Expand());
	sizer->Add(commandLineSizer, wxSizerFlags().Expand());

	this->SetSizer(sizer);
	this->SetMaxSize(wxSize(TAB_AREA_WIDTH, TAB_AREA_HEIGHT));
	this->Layout();

	wxString cmdline, customFlags;
	ProMan::GetProfileManager()->Get()->
		Read(PRO_CFG_TC_CURRENT_FLAG_LINE, &cmdline);
	wxStringTokenizer tokenizer(cmdline, _T(" "));
	while(tokenizer.HasMoreTokens()) {
		wxString tok = tokenizer.GetNextToken();
		if ( this->flagListBox->SetFlag(tok, true) ) {
			continue;
		} else {
			if (!customFlags.IsEmpty()) {
				customFlags += _T(" ");
			}
			customFlags += tok;
		}
	}
	customFlagsText->SetLabel(customFlags);
	this->OnNeedUpdateCommandLine(event);
}

void AdvSettingsPage::OnNeedUpdateCommandLine(wxCommandEvent &WXUNUSED(event)) {
	wxTextCtrl* commandLine = dynamic_cast<wxTextCtrl*>(
		wxWindow::FindWindowById(ID_COMMAND_LINE_TEXT, this));
	wxCHECK_RET( commandLine != NULL, _T("Unable to find the text control") );
	wxTextCtrl* customFlags = dynamic_cast<wxTextCtrl*>(
		wxWindow::FindWindowById(ID_CUSTOM_FLAGS_TEXT, this));
	wxCHECK_RET( customFlags != NULL, _T("Unable to find the custom flag box"));

	wxString cmdLine = wxString::Format(_T("%s %s"),
		this->flagListBox->GenerateStringList().c_str(),
		customFlags->GetLabel().c_str());
	
	commandLine->SetLabel(cmdLine);
	ProMan::GetProfileManager()->Get()->
		Write(PRO_CFG_TC_CURRENT_FLAG_LINE, cmdLine);
}
	
	
