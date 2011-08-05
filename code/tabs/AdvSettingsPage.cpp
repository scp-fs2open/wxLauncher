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

#include "tabs/AdvSettingsPage.h"

#include "apis/TCManager.h"
#include "apis/ProfileManager.h"
#include "global/ids.h"

#include <wx/html/htmlwin.h>
#include <wx/tokenzr.h>

#include "global/MemoryDebugging.h" // Last include for memory debugging

AdvSettingsPage::AdvSettingsPage(wxWindow* parent, SkinSystem *skin): wxPanel(parent, wxID_ANY) {
	this->skin = skin;
	this->SetMinSize(wxSize(TAB_AREA_WIDTH, TAB_AREA_HEIGHT));

	TCManager::RegisterTCBinaryChanged(this);
	TCManager::RegisterTCSelectedModChanged(this);
	wxCommandEvent nullEvent;
	this->OnExeChanged(nullEvent);	
}

BEGIN_EVENT_TABLE(AdvSettingsPage, wxPanel)
EVT_COMMAND(wxID_NONE, EVT_TC_BINARY_CHANGED, AdvSettingsPage::OnExeChanged)
EVT_COMMAND(wxID_NONE, EVT_TC_SELECTED_MOD_CHANGED, AdvSettingsPage::OnNeedUpdateCommandLine)
EVT_COMMAND(wxID_NONE, EVT_FLAG_LIST_BOX_DRAW_STATUS_CHANGE, AdvSettingsPage::OnDrawStatusChange)
EVT_TEXT(ID_CUSTOM_FLAGS_TEXT, AdvSettingsPage::OnNeedUpdateCommandLine)
EVT_CHOICE(ID_SELECT_FLAG_SET, AdvSettingsPage::OnSelectFlagSet)
END_EVENT_TABLE()

void AdvSettingsPage::OnExeChanged(wxCommandEvent& event) {
	if (this->GetSizer() != NULL) {
		this->GetSizer()->DeleteWindows();
	}

	this->flagListBox = new FlagListBox(this, this->skin);

#if 0 // doesn't do anything
	wxHtmlWindow* description = new wxHtmlWindow(this);
	description->SetPage(_T("<p></p>"));
#endif

	wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
	topSizer->Add(this->flagListBox, wxSizerFlags().Proportion(1).Expand());
#if 0
	topSizer->Add(description, wxSizerFlags().Proportion(1).Expand());
#endif
	topSizer->SetMinSize(TAB_AREA_WIDTH, (7*TAB_AREA_HEIGHT)/12);

#if 0
	wxStaticBitmap* idealIcon = new wxStaticBitmap(this, wxID_ANY, this->skin->GetIdealIcon());
	wxStaticText* idealLabel = new wxStaticText(this, wxID_ANY, _("= Recommended flag"));
#endif
	wxStaticText* flagSetChoiceLabel = new wxStaticText(this, wxID_ANY, _T("Flag sets:"));
	wxChoice* flagSetChoice = new wxChoice(this, ID_SELECT_FLAG_SET);

	wxBoxSizer* idealFlagsRowSizer = new wxBoxSizer(wxHORIZONTAL);
#if 0
	idealFlagsRowSizer->Add(idealIcon);
	wxBoxSizer* idealLabelSizer = new wxBoxSizer(wxVERTICAL);
	idealLabelSizer->AddStretchSpacer(1);
	idealLabelSizer->Add(idealLabel);
	idealLabelSizer->AddStretchSpacer(1);
	idealFlagsRowSizer->Add(idealLabelSizer);
#endif
	idealFlagsRowSizer->AddStretchSpacer(1);
	idealFlagsRowSizer->Add(flagSetChoiceLabel, wxSizerFlags().Border(wxRIGHT, 5));
	idealFlagsRowSizer->Add(flagSetChoice, wxSizerFlags().Right());

#if 0 // doesn't do anything
	wxStaticBox* flagsetNotesBox = new wxStaticBox(this, wxID_ANY, _("Flag set notes"));
	wxTextCtrl* flagsetNotes = new wxTextCtrl(this, ID_FLAG_SET_NOTES_TEXT,
		wxEmptyString, wxDefaultPosition, wxSize(-1, TAB_AREA_HEIGHT/8),
		wxTE_MULTILINE|wxTE_READONLY);
	wxStaticBoxSizer* flagsetNotesSizer = new wxStaticBoxSizer(flagsetNotesBox, wxVERTICAL);
	flagsetNotesSizer->Add(flagsetNotes, wxSizerFlags().Expand());
#endif
	
	wxStaticBox* customFlagsBox = new wxStaticBox(this, wxID_ANY, _("Custom flags"));
	wxTextCtrl* customFlagsText = new wxTextCtrl(this, ID_CUSTOM_FLAGS_TEXT);
	wxStaticBoxSizer* customFlagsSizer = new wxStaticBoxSizer(customFlagsBox, wxVERTICAL);
	customFlagsSizer->Add(customFlagsText, wxSizerFlags().Expand());
	
	wxStaticBox* commandLineLabel = new wxStaticBox(this, wxID_ANY, _("Current command line"));
	wxTextCtrl* commandLineText = new wxTextCtrl(this, ID_COMMAND_LINE_TEXT,
		wxEmptyString, wxDefaultPosition, wxSize(-1, TAB_AREA_HEIGHT/5),
		wxTE_MULTILINE|wxTE_READONLY);
	wxStaticBoxSizer* commandLineSizer = new wxStaticBoxSizer(commandLineLabel, wxVERTICAL);
	commandLineSizer->Add(commandLineText, wxSizerFlags().Expand());

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(topSizer, wxSizerFlags().Expand());
	sizer->Add(idealFlagsRowSizer, wxSizerFlags().Expand());
#if 0
	sizer->Add(flagsetNotesSizer, wxSizerFlags().Expand());
#endif
	sizer->Add(customFlagsSizer, wxSizerFlags().Expand());
	sizer->Add(commandLineSizer, wxSizerFlags().Expand());

	this->SetSizer(sizer);
	this->SetMaxSize(wxSize(TAB_AREA_WIDTH, TAB_AREA_HEIGHT));
	this->Layout();

	this->OnDrawStatusChange(event);
}

void AdvSettingsPage::OnDrawStatusChange(wxCommandEvent &event) {
	wxTextCtrl* customFlagsText = dynamic_cast<wxTextCtrl*>(
		wxWindow::FindWindowById(ID_CUSTOM_FLAGS_TEXT, this));
	wxCHECK_RET( customFlagsText != NULL, _T("Cannot find custom flags box") );

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
	customFlagsText->ChangeValue(customFlags);
	this->OnNeedUpdateCommandLine(event);
}

void AdvSettingsPage::OnNeedUpdateCommandLine(wxCommandEvent &WXUNUSED(event)) {
	if ( !this->flagListBox->IsDrawOK() ) {
		// The control I need to update does not exist, do nothing
		return;
	}

	wxChoice *flagSetChoice = dynamic_cast<wxChoice*>(
		wxWindow::FindWindowById(ID_SELECT_FLAG_SET, this));
	wxCHECK_RET( flagSetChoice != NULL, _T("Unable to find the flagset choice control") );
	wxArrayString flagSetsArray;
	if (flagSetChoice->IsEmpty()) { // box should be appended just once
		this->flagListBox->GetFlagSets(flagSetsArray);

		// before populating the flag set choice box, let's remove the flag sets that don't make sense
		// and can thus potentially confuse users

		flagSetsArray.Remove(_T("Custom"));

		// workaround for removing "All features on", since the flag set names other than Custom seem
		// to end in some kind of weird unprintable char(s), which is then messing up string comparison.
		for (int i = flagSetsArray.GetCount() - 1; i >= 0; --i) {
			if (flagSetsArray[i].StartsWith(_T("All features on"))) {
				flagSetsArray.RemoveAt(i);
			}
		}
		
		flagSetChoice->Append(flagSetsArray);
	}

	wxTextCtrl* commandLine = dynamic_cast<wxTextCtrl*>(
		wxWindow::FindWindowById(ID_COMMAND_LINE_TEXT, this));
	wxCHECK_RET( commandLine != NULL, _T("Unable to find the text control") );
	wxTextCtrl* customFlags = dynamic_cast<wxTextCtrl*>(
		wxWindow::FindWindowById(ID_CUSTOM_FLAGS_TEXT, this));
	wxCHECK_RET( customFlags != NULL, _T("Unable to find the custom flag box"));

	wxString tcPath, exeName, modline;
	ProMan::GetProfileManager()->Get()->
		Read(PRO_CFG_TC_ROOT_FOLDER, &tcPath);
	ProMan::GetProfileManager()->Get()->
		Read(PRO_CFG_TC_CURRENT_BINARY, &exeName);
	ProMan::GetProfileManager()->Get()->
		Read(PRO_CFG_TC_CURRENT_MODLINE, &modline);

	wxString flagLine = wxString::Format(_T("%s %s"),
		this->flagListBox->GenerateStringList().c_str(),
		customFlags->GetValue().c_str());

	wxString cmdLine = wxString::Format(_T("%s%c%s -mod %s %s"),
		tcPath.c_str(), wxFileName::GetPathSeparator(), exeName.c_str(),
		modline.c_str(), flagLine.c_str());
	
	commandLine->ChangeValue(cmdLine);
	ProMan::GetProfileManager()->Get()->
		Write(PRO_CFG_TC_CURRENT_FLAG_LINE, flagLine);
}

void AdvSettingsPage::OnSelectFlagSet(wxCommandEvent &WXUNUSED(event)) {
	wxChoice* choice = dynamic_cast<wxChoice*>(
		wxWindow::FindWindowById(ID_SELECT_FLAG_SET, this));
	wxCHECK_RET( choice != NULL, _T("Unable to get flagSetChoice box"));

	wxString selectedSet = choice->GetString(choice->GetSelection());
	bool ret = this->flagListBox->SetFlagSet(selectedSet);
	if ( ret == false ) {
		wxLogError(_T("Unable to set flag set (%s). Set does not exist"), selectedSet.c_str());
	}
	wxCommandEvent event;
	this->OnNeedUpdateCommandLine(event);
}
	
