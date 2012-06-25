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

#include "apis/CmdLineManager.h"
#include "apis/FlagListManager.h"
#include "apis/ProfileProxy.h"
#include "apis/TCManager.h"
#include "apis/ProfileManager.h"
#include "controls/LightingPresets.h"
#include "controls/ModList.h" // for code needed in rendering command line text
#include "global/ids.h"

#include <wx/html/htmlwin.h>
#include <wx/tokenzr.h>

#include "global/MemoryDebugging.h" // Last include for memory debugging

const size_t TOP_SIZER_INDEX = 0;
const size_t TOP_LEFT_SIZER_INDEX = 0;
const size_t WIKI_LINK_SIZER_INDEX = 1;
const size_t TOP_RIGHT_SIZER_INDEX = 1;
const size_t BOTTOM_SIZER_INDEX = 1;

AdvSettingsPage::AdvSettingsPage(wxWindow* parent, SkinSystem *skin): wxPanel(parent, wxID_ANY), flagListBox(NULL) {
	wxASSERT(skin != NULL);
	this->skin = skin;

	this->errorText =
		new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
			wxDefaultSize, wxALIGN_CENTER);
	
	wxLogDebug(_T("AdvSettingsPage is at %p."), this);

	CmdLineManager::RegisterCmdLineChanged(this);
	CmdLineManager::RegisterCustomFlagsChanged(this);
	FlagListManager::GetFlagListManager()->RegisterFlagFileProcessingStatusChanged(this);
	TCManager::RegisterTCSelectedModChanged(this);
	ProfileProxy::GetProxy()->RegisterProxyReset(this);
	ProfileProxy::GetProxy()->RegisterProxyFlagDataReady(this);
}

BEGIN_EVENT_TABLE(AdvSettingsPage, wxPanel)
EVT_COMMAND(wxID_NONE, EVT_PROXY_RESET, AdvSettingsPage::OnExeChanged)
EVT_COMMAND(wxID_NONE, EVT_PROXY_FLAG_DATA_READY, AdvSettingsPage::OnProxyFlagDataReady)
EVT_COMMAND(wxID_NONE, EVT_TC_SELECTED_MOD_CHANGED, AdvSettingsPage::OnNeedUpdateCommandLine)
EVT_COMMAND(wxID_NONE, EVT_FLAG_FILE_PROCESSING_STATUS_CHANGED, AdvSettingsPage::OnFlagFileProcessingStatusChanged)
EVT_COMMAND(wxID_NONE, EVT_CMD_LINE_CHANGED, AdvSettingsPage::OnNeedUpdateCommandLine)
EVT_COMMAND(wxID_NONE, EVT_CUSTOM_FLAGS_CHANGED, AdvSettingsPage::OnNeedUpdateCustomFlags)
EVT_COMMAND(wxID_NONE, EVT_FLAG_LIST_BOX_READY, AdvSettingsPage::OnFlagListBoxReady)
EVT_TEXT(ID_CUSTOM_FLAGS_TEXT, AdvSettingsPage::OnCustomFlagsBoxChanged)
EVT_CHOICE(ID_SELECT_FLAG_SET, AdvSettingsPage::OnSelectFlagSet)
END_EVENT_TABLE()

// FIXME HACK for now, hard-code flag list box height (sigh)
#if IS_WIN32
const int FLAG_LIST_BOX_HEIGHT = 410;
#elif IS_LINUX
const int FLAG_LIST_BOX_HEIGHT = 363;
#elif IS_APPLE
const int FLAG_LIST_BOX_HEIGHT = 363;
#else
#error "One of IS_WIN32, IS_LINUX, IS_APPLE must evaluate to true"
#endif

void AdvSettingsPage::OnExeChanged(wxCommandEvent& event) {
	if (this->GetSizer() != NULL) {
		this->GetSizer()->Clear(true);
	}

	// top left components
	this->flagListBox = new FlagListBox(this, this->skin);
	this->flagListBox->RegisterFlagListBoxReady(this);
	FlagListManager::GetFlagListManager()->BeginFlagFileProcessing();

#if 0 // doesn't do anything
	wxHtmlWindow* description = new wxHtmlWindow(this);
	description->SetPage(_T("<p></p>"));
#endif


#if IS_WIN32
	wxStaticText* wikiLinkText1 = new wxStaticText(this, wxID_ANY,
		_T("Double-click on a flag for its online documentation, if available."));
#else
	wxStaticText* wikiLinkText1 = new wxStaticText(this, wxID_ANY,
		_T("Double-click on a flag"));
	wxStaticText* wikiLinkText2 = new wxStaticText(this, wxID_ANY,
		_T("for its online documentation, if available."));
#endif
	
	wxBoxSizer* wikiLinkSizer = new wxBoxSizer(wxVERTICAL);
	wikiLinkSizer->Add(wikiLinkText1, 0, wxALIGN_CENTER_HORIZONTAL);
#if !IS_WIN32
	wikiLinkSizer->Add(wikiLinkText2, 0, wxALIGN_CENTER_HORIZONTAL);
#endif

	wxBoxSizer* topLeftSizer = new wxBoxSizer(wxVERTICAL);
	topLeftSizer->Add(this->flagListBox, wxSizerFlags().Proportion(1).Expand());
	topLeftSizer->Add(wikiLinkSizer, 0, wxALIGN_CENTER_HORIZONTAL|wxTOP, 5);

	// top right components
	LightingPresets* lightingPresets = new LightingPresets(this);
	
	wxBoxSizer* lightingPresetsSizer = new wxBoxSizer(wxVERTICAL);
	lightingPresetsSizer->Add(lightingPresets, wxSizerFlags().Proportion(1).Expand());

	wxStaticText* flagSetChoiceLabel = new wxStaticText(this, wxID_ANY, _T("Flag sets:"));
	wxChoice* flagSetChoice = new wxChoice(this, ID_SELECT_FLAG_SET);
	
	wxBoxSizer* topRightSizer = new wxBoxSizer(wxVERTICAL);
	topRightSizer->Add(lightingPresetsSizer, wxSizerFlags().Proportion(1).Border(wxBOTTOM, 5));
	topRightSizer->Add(flagSetChoiceLabel, wxSizerFlags().Left().Border(wxBOTTOM, 5));
	topRightSizer->Add(flagSetChoice, wxSizerFlags().Expand());
	
	// putting the top sizer together
	wxBoxSizer* topSizer = new wxBoxSizer(wxHORIZONTAL);
	topSizer->Add(topLeftSizer, wxSizerFlags().Proportion(1).Expand());
	topSizer->Add(topRightSizer, wxSizerFlags().Expand().Border(wxLEFT, 5));
#if 0
	topSizer->Add(description, wxSizerFlags().Proportion(1).Expand());
#endif

#if 0 // related to functionality that isn't working yet
	wxStaticBitmap* idealIcon = new wxStaticBitmap(this, wxID_ANY, this->skin->GetIdealIcon());
	wxStaticText* idealLabel = new wxStaticText(this, wxID_ANY, _("= Recommended flag"));

	wxBoxSizer* idealFlagsRowSizer = new wxBoxSizer(wxHORIZONTAL);
	idealFlagsRowSizer->Add(idealIcon);
	wxBoxSizer* idealLabelSizer = new wxBoxSizer(wxVERTICAL);
	idealLabelSizer->AddStretchSpacer(1);
	idealLabelSizer->Add(idealLabel);
	idealLabelSizer->AddStretchSpacer(1);
	idealFlagsRowSizer->Add(idealLabelSizer);
#endif

#if 0 // doesn't do anything
	wxStaticBox* flagsetNotesBox = new wxStaticBox(this, wxID_ANY, _("Flag set notes"));
	wxTextCtrl* flagsetNotes = new wxTextCtrl(this, ID_FLAG_SET_NOTES_TEXT,
		wxEmptyString, wxDefaultPosition, wxSize(-1, TAB_AREA_HEIGHT/8),
		wxTE_MULTILINE|wxTE_READONLY);
	wxStaticBoxSizer* flagsetNotesSizer = new wxStaticBoxSizer(flagsetNotesBox, wxVERTICAL);
	flagsetNotesSizer->Add(flagsetNotes, wxSizerFlags().Expand());
#endif
	
	wxStaticText* customFlagsTextLabel = new wxStaticText(this, wxID_ANY, _("Custom flags:"));
	wxTextCtrl* customFlagsText = new wxTextCtrl(this, ID_CUSTOM_FLAGS_TEXT);
	
	wxStaticText* commandLineTextLabel = new wxStaticText(this, wxID_ANY, _("Current command line:"));
	wxTextCtrl* commandLineText = new wxTextCtrl(this, ID_COMMAND_LINE_TEXT,
		wxEmptyString, wxDefaultPosition, wxDefaultSize,
		wxTE_MULTILINE|wxTE_READONLY);

	wxBoxSizer* bottomSizer = new wxBoxSizer(wxVERTICAL);
	bottomSizer->Add(customFlagsTextLabel, wxSizerFlags().Left().Border(wxTOP|wxBOTTOM, 5));
	bottomSizer->Add(customFlagsText, wxSizerFlags().Expand().Border(wxBOTTOM, 10));
	bottomSizer->Add(commandLineTextLabel, wxSizerFlags().Left().Border(wxBOTTOM, 5));
	bottomSizer->Add(commandLineText, wxSizerFlags().Proportion(1).Expand());

	// final layout
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(topSizer, wxSizerFlags().Expand().Border(wxALL, 5));
#if 0
	sizer->Add(idealFlagsRowSizer, wxSizerFlags().Expand().Border(wxALL, 5));
	sizer->Add(flagsetNotesSizer, wxSizerFlags().Expand().Border(wxLEFT|wxRIGHT|wxBOTTOM, 5));
#endif
	sizer->Add(bottomSizer, wxSizerFlags().Expand().Proportion(1).Border(wxALL, 5));

	this->SetSizer(sizer);
	this->Layout();

	if (!ProfileProxy::GetProxy()->IsProfileInitialized()) {
		ProfileProxy::GetProxy()->FinishProfileInitialization();
	}
}

void AdvSettingsPage::OnFlagFileProcessingStatusChanged(wxCommandEvent &event) {
	wxASSERT(event.GetEventType() == EVT_FLAG_FILE_PROCESSING_STATUS_CHANGED);
	
	const FlagListManager::FlagFileProcessingStatus status =
		static_cast<FlagListManager::FlagFileProcessingStatus>(event.GetInt());
	
	// Ignore an event with reset status, since it's meant for the proxy
	if (status == FlagListManager::FLAG_FILE_PROCESSING_RESET) {
		return;
	}
	
	this->UpdateComponents();
	
	if (status == FlagListManager::FLAG_FILE_PROCESSING_OK) {
		FlagFileData* flagData = FlagListManager::GetFlagListManager()->GetFlagFileData();
		wxCHECK_RET(flagData != NULL,
			_T("Flag file processing succeeded but could not retrieve extracted data."));
		this->flagListBox->AcceptFlagData(flagData);
	} else {
		this->UpdateErrorText();
	}
}

void AdvSettingsPage::OnNeedUpdateCustomFlags(wxCommandEvent &event) {
	wxASSERT((this->flagListBox != NULL) && this->flagListBox->IsReady());
	wxASSERT(ProfileProxy::GetProxy()->IsFlagDataReady());
	
	wxTextCtrl* customFlagsText = dynamic_cast<wxTextCtrl*>(
		wxWindow::FindWindowById(ID_CUSTOM_FLAGS_TEXT, this));
	wxCHECK_RET(customFlagsText != NULL,
		_T("Unable to find the custom flags text ctrl"));
	
	customFlagsText->ChangeValue(ProfileProxy::GetProxy()->GetCustomFlags());
}

void AdvSettingsPage::OnFlagListBoxReady(wxCommandEvent &WXUNUSED(event)) {
	wxASSERT((this->flagListBox != NULL) && this->flagListBox->IsReady());

	if (ProfileProxy::GetProxy()->IsFlagDataReady()
		 && !this->flagListBox->FlagsLoaded()) {
		this->flagListBox->LoadEnabledFlags();
		CmdLineManager::GenerateCustomFlagsChanged();
		CmdLineManager::GenerateCmdLineChanged();
	}
	
	this->UpdateComponents();
	this->UpdateFlagSetsBox();
}

void AdvSettingsPage::OnProxyFlagDataReady(wxCommandEvent& event) {
	wxASSERT((this->flagListBox != NULL) &&
		ProfileProxy::GetProxy()->IsFlagDataReady());
	
	if (this->flagListBox->IsReady() && !this->flagListBox->FlagsLoaded()) {
		this->flagListBox->LoadEnabledFlags();
		CmdLineManager::GenerateCustomFlagsChanged();
		CmdLineManager::GenerateCmdLineChanged();
	}	
}

void AdvSettingsPage::UpdateComponents() {
	wxSizer* topSizer = this->GetSizer()->GetItem(TOP_SIZER_INDEX)->GetSizer();
	wxCHECK_RET(topSizer != NULL, _T("cannot find the top sizer"));
	
	wxSizer* topLeftSizer = topSizer->GetItem(TOP_LEFT_SIZER_INDEX)->GetSizer();
	wxCHECK_RET(topLeftSizer != NULL, _T("cannot find the top left sizer"));
	
	wxCHECK_RET(this->flagListBox != NULL,
		_T("UpdateComponents() called when flagListBox was null."));
	
	if (this->flagListBox->IsReady()) {
		topSizer->Show(TOP_RIGHT_SIZER_INDEX);
		topLeftSizer->Show(WIKI_LINK_SIZER_INDEX);
		this->GetSizer()->Show(BOTTOM_SIZER_INDEX);
		this->flagListBox->Show();
		this->errorText->Hide();
		this->Layout();
	} else {
		topSizer->Hide(TOP_RIGHT_SIZER_INDEX);
		topLeftSizer->Hide(WIKI_LINK_SIZER_INDEX);
		this->GetSizer()->Hide(BOTTOM_SIZER_INDEX);
		this->flagListBox->Hide();
		this->errorText->Show();
		this->Layout();
	}
}

void AdvSettingsPage::UpdateErrorText() {
	wxCHECK_RET(!FlagListManager::GetFlagListManager()->IsProcessingOK(),
		_T("UpdateErrorText() called when processing succeeded."));
	
	wxRect rect(0, 0, this->GetSize().x, this->GetSize().y);
	
	wxString msg = FlagListManager::GetFlagListManager()->GetStatusMessage();

	wxASSERT(!msg.IsEmpty());
	
	this->errorText->SetLabel(msg);
	
	wxFont errorFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
	this->errorText->SetFont(errorFont);
	
	this->errorText->SetSize(rect, wxSIZE_FORCE);
	this->errorText->Wrap(rect.width - 225); // to match mods page
	this->errorText->Center();
}

void AdvSettingsPage::OnCustomFlagsBoxChanged(wxCommandEvent &WXUNUSED(event)) {
	wxASSERT(this->flagListBox != NULL && this->flagListBox->IsReady());
	wxASSERT(ProfileProxy::GetProxy()->IsFlagDataReady());
	
	wxTextCtrl* customFlagsText = dynamic_cast<wxTextCtrl*>(
		wxWindow::FindWindowById(ID_CUSTOM_FLAGS_TEXT, this));
	wxCHECK_RET(customFlagsText != NULL,
		_T("Unable to find the custom flags text ctrl"));
	
	ProfileProxy::GetProxy()->SetCustomFlags(customFlagsText->GetValue(), false);
}

void AdvSettingsPage::UpdateFlagSetsBox() {
	wxASSERT(this->flagListBox != NULL);
	wxASSERT(this->flagListBox->IsReady());
	
	wxChoice *flagSetChoice = dynamic_cast<wxChoice*>(
		wxWindow::FindWindowById(ID_SELECT_FLAG_SET, this));
	wxCHECK_RET(flagSetChoice != NULL,
		_T("Unable to find the flagset choice control"));
	
	// TODO rethink the following assertion when new mod.ini is supported
	// the assertion also assumes that the flag sets box is never reused
	wxASSERT(flagSetChoice->IsEmpty()); // shouldn't add sets more than once
	
	wxArrayString flagSetsArray;
	this->flagListBox->GetFlagSets(flagSetsArray);
	
	// before populating the flag set choice box, let's remove the flag sets
	// that don't make sense and can thus potentially confuse users
	
	flagSetsArray.Remove(_T("Custom"));
	flagSetsArray.Remove(_T("All features on"));
	
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
	
	flagSetChoice->SetMinSize(
		wxSize(maxStringWidth + 40, // 40 to include drop down box control
			flagSetChoice->GetSize().GetHeight()));
	this->Layout();
}

void AdvSettingsPage::OnNeedUpdateCommandLine(wxCommandEvent &WXUNUSED(event)) {
	if ( (this->flagListBox == NULL) || !this->flagListBox->IsReady() ) {
		// The control I need to update does not exist, do nothing
		return;
	}

	wxTextCtrl* commandLine = dynamic_cast<wxTextCtrl*>(
		wxWindow::FindWindowById(ID_COMMAND_LINE_TEXT, this));
	wxCHECK_RET( commandLine != NULL, _T("Unable to find the command line view text control") );

	wxString tcPath, exeName, modline;
	wxCHECK_RET(
		ProMan::GetProfileManager()->ProfileRead(
			PRO_CFG_TC_ROOT_FOLDER, &tcPath),
		_T("Could not find profile entry for root folder."));
	wxCHECK_RET(
		ProMan::GetProfileManager()->ProfileRead(
			PRO_CFG_TC_CURRENT_BINARY, &exeName),
		_T("Could not find profile entry for FSO binary."));
	wxCHECK_RET(
		ProMan::GetProfileManager()->ProfileRead(
			PRO_CFG_TC_CURRENT_MODLINE, &modline),
		_T("Could not find profile entry for mod line."));
	
	wxString flagFileFlags(ProfileProxy::GetProxy()->GetEnabledFlagsString());

	wxString lightingPresetString;
	if (ProfileProxy::GetProxy()->HasLightingPreset()) {
		lightingPresetString = LightingPresets::PresetNameToPresetString(
			ProfileProxy::GetProxy()->GetLightingPresetName());
	}
	
	wxString customFlags(ProfileProxy::GetProxy()->GetCustomFlags());
	
	wxString cmdLine =
		wxString::Format(_T("%s%c%s%s%s%s%s"),
						 tcPath.c_str(),
						 wxFileName::GetPathSeparator(),
						 exeName.c_str(),
						 (modline.IsEmpty() ? wxEmptyString :
							wxString::Format(_T(" -mod %s"), modline.c_str()).c_str()),
						 (flagFileFlags.IsEmpty() ? wxEmptyString :
							wxString::Format(_T(" %s"), flagFileFlags.c_str()).c_str()),
						 (lightingPresetString.IsEmpty() ? wxEmptyString :
							wxString::Format(_T(" %s"), lightingPresetString.c_str()).c_str()),
						 (customFlags.IsEmpty() ? wxEmptyString :
						  wxString::Format(_T(" %s"), customFlags.c_str()).c_str()));

	commandLine->ChangeValue(FormatCommandLineString(cmdLine,
		commandLine->GetSize().GetWidth() - 30)); // 30 for scrollbar
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
}
	
