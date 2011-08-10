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

#include "generated/configure_launcher.h"

#include "tabs/AdvSettingsPage.h"

#include "apis/TCManager.h"
#include "apis/ProfileManager.h"
#include "controls/ModList.h" // for code needed in rendering command line text
#include "global/ids.h"

#include <wx/html/htmlwin.h>
#include <wx/tokenzr.h>

#include "global/MemoryDebugging.h" // Last include for memory debugging

AdvSettingsPage::AdvSettingsPage(wxWindow* parent, SkinSystem *skin): wxPanel(parent, wxID_ANY) {
	this->skin = skin;

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
		wxEmptyString, wxDefaultPosition, wxDefaultSize,
		wxTE_MULTILINE|wxTE_READONLY);
	wxStaticBoxSizer* commandLineSizer = new wxStaticBoxSizer(commandLineLabel, wxVERTICAL);
	commandLineSizer->Add(commandLineText, wxSizerFlags().Expand().Proportion(1));

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(topSizer, wxSizerFlags().Expand().Proportion(5).Border(wxLEFT|wxRIGHT|wxTOP, 5));
	sizer->Add(idealFlagsRowSizer, wxSizerFlags().Expand().Border(wxLEFT|wxRIGHT|wxTOP, 5));
#if 0
	sizer->Add(flagsetNotesSizer, wxSizerFlags().Expand().Border(wxLEFT|wxRIGHT, 5));
#endif
	sizer->Add(customFlagsSizer, wxSizerFlags().Expand().Border(wxLEFT|wxRIGHT|wxBOTTOM, 5));
	sizer->Add(commandLineSizer, wxSizerFlags().Expand().Proportion(2).Border(wxLEFT|wxRIGHT|wxBOTTOM, 5));

	this->SetSizer(sizer);
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

		wxClientDC dc(this);
		wxArrayString flagSets = flagSetChoice->GetStrings();
		wxFont font(this->GetFont());
		int maxStringWidth = 0;
		int x, y;

		for (int i = 0, n = flagSets.GetCount(); i < n; ++i) {
			dc.GetTextExtent(flagSets[i], &x, &y, NULL, NULL, &font);

			if (x > maxStringWidth) {
				maxStringWidth = x;
			}
		}

		flagSetChoice->SetMinSize(wxSize(maxStringWidth + 40, // 40 to include drop down box control
			flagSetChoice->GetSize().GetHeight()));
		this->Layout();
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

	commandLine->ChangeValue(FormatCommandLineString(cmdLine,
													 commandLine->GetSize().GetWidth() - 20)); // 20 for scrollbar
	ProMan::GetProfileManager()->Get()->
		Write(PRO_CFG_TC_CURRENT_FLAG_LINE, flagLine);
}

// Adapted from ModList.cpp - FIXME there should really be just one copy
void ASPFillArrayOfWordsFromTokens(wxStringTokenizer &tokens,
								   wxDC& dc,
								   wxFont *testFont,
								   ArrayOfWords *words)
{
	while ( tokens.HasMoreTokens() ) {
		wxString tok = tokens.GetNextToken();
#if IS_APPLE
		if (tok.Lower() == _T("(debug)")) { // left over from tokenizing executable in debug .app
			tok = _T("");
		}
		// remove the text after ".app" in the FSO executable name
		int DotAppIndex = tok.Find(_T(".app/")); // the trailing / ensures that the .app indicates an extension
		if (DotAppIndex != wxNOT_FOUND) {
			tok = tok.Mid(0, DotAppIndex + 4); // 4 so that ".app" is retained
		}
#endif
		int x, y;
		dc.GetTextExtent(tok, &x, &y, NULL, NULL, testFont);

		Words* temp = new Words();
		temp->size.SetWidth(x);
		temp->size.SetHeight(y);
		temp->word = tok;

		words->Add(temp);
	}
}

wxString AdvSettingsPage::FormatCommandLineString(const wxString& origCmdLine,
												  const int textAreaWidth) {
	// inspired by ModItem::InfoText::Draw()
	wxStringTokenizer tokens(origCmdLine);
	ArrayOfWords words;
	words.Alloc(tokens.CountTokens());
	
	wxClientDC dc(this);
	wxFont font(this->GetFont());

	ASPFillArrayOfWordsFromTokens(tokens, dc, &font, &words);

	const int spaceWidth = dc.GetTextExtent(_T(" ")).GetWidth();
	
	int currentWidth = 0;
	
	wxString formattedCmdLine;
	wxString currentLine;

	bool spaceAdded = false;

	for (size_t i = 0, n = words.Count(); i < n; i++) {
		if (words[i].word.IsEmpty()) { // skip over remnants of words eliminated in tokens-to-words conversion
			continue;
		} else if (currentWidth + words[i].size.x + spaceWidth > textAreaWidth) {
			// prevents trailing newline in cmdLineString (nitpicky, I know)
			formattedCmdLine += wxString::Format(formattedCmdLine.IsEmpty() ? _T("%s") : _T("\n%s"),
												 currentLine.c_str());
			currentLine.Empty();
			currentWidth = 0;
		} else {
			if (!currentLine.IsEmpty()) {
				currentLine.append(_T(" "));
				spaceAdded = true;
			}
		}
		currentLine.append(words[i].word);
		currentWidth += words[i].size.x + (spaceAdded ? spaceWidth : 0);
		spaceAdded = false;
	}
	if (!currentLine.IsEmpty()) {
		formattedCmdLine += wxString::Format(formattedCmdLine.IsEmpty() ? _T("%s") : _T("\n%s"),
											 currentLine.c_str());
	}

	return formattedCmdLine;
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
	
