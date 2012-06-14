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

#include <algorithm>

#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/choicebk.h>
#include <wx/gbsizer.h>
#include <wx/hyperlink.h>

#include "generated/configure_launcher.h"

#if HAS_SDL == 1
#include "SDL.h"
#endif

#include "tabs/BasicSettingsPage.h"
#include "global/ids.h"
#include "apis/FlagListManager.h"
#include "apis/ProfileManager.h"
#include "apis/TCManager.h"
#include "apis/SpeechManager.h"
#include "apis/OpenALManager.h"
#include "apis/JoystickManager.h"
#include "apis/HelpManager.h"
#include "datastructures/FSOExecutable.h"

#include "global/MemoryDebugging.h" // Last include for memory debugging

/** A mechanism for allowing a network settings option's description (GUI label)
 to differ from its corresponding registry value. */
class NetworkSettingsOption {
public:
	NetworkSettingsOption(const wxString& regValue, const wxString& guiDesc);
	const wxString& GetRegistryValue() const { return this->regValue; }
	const wxString& GetDescription() const { return this->guiDesc; }
private:
	NetworkSettingsOption();
	wxString regValue;
	wxString guiDesc;
};

typedef std::vector<NetworkSettingsOption> NetworkSettingsOptions;
NetworkSettingsOptions networkTypeOptions;
NetworkSettingsOptions networkSpeedOptions;

NetworkSettingsOption::NetworkSettingsOption(
	const wxString& regValue,
	const wxString& guiDesc)
: regValue(regValue), guiDesc(guiDesc) {
	wxASSERT(!regValue.IsEmpty());
	wxASSERT(!guiDesc.IsEmpty());
}

void InitializeNetworkOptions() {
	wxASSERT(networkTypeOptions.empty());
	wxASSERT(networkSpeedOptions.empty());
	
	networkTypeOptions.push_back(NetworkSettingsOption(_T("None"), _T("None")));
	networkTypeOptions.push_back(NetworkSettingsOption(_T("Dialup"), _T("Dialup")));
	networkTypeOptions.push_back(NetworkSettingsOption(_T("LAN"), _T("Broadband/LAN")));
	
	networkSpeedOptions.push_back(NetworkSettingsOption(_T("None"), _T("None")));
	networkSpeedOptions.push_back(NetworkSettingsOption(_T("Slow"), _T("28k modem")));
	networkSpeedOptions.push_back(NetworkSettingsOption(_T("56K"), _T("56k modem")));
	networkSpeedOptions.push_back(NetworkSettingsOption(_T("ISDN"), _T("ISDN")));
	networkSpeedOptions.push_back(NetworkSettingsOption(_T("Cable"), _T("DSL")));
	networkSpeedOptions.push_back(NetworkSettingsOption(_T("Fast"), _T("Cable/LAN")));
}

int FindOptionIndexWithRegistryValue(
		const NetworkSettingsOptions &options,
		const wxString& regValue) {
	wxCHECK_MSG(!options.empty(), -1,
		_T("FindOptionIndexGivenRegistryValue(): passed in options is empty."));
	wxCHECK_MSG(!regValue.IsEmpty(), -1,
		_T("FindOptionIndexGivenRegistryValue(): passed in registry value is empty."));
	
	for (int i = 0, n = options.size(); i < n; ++i) {
		if (options[i].GetRegistryValue() == regValue) {
			return i;			
		}
	}
	
	return -1; // not found
}

/** The index in the basic settings page's sizer where the settings sizer is located. */
const size_t SETTINGS_SIZER_INDEX = 1;

class ProxyChoice: public wxChoicebook {
public:
	ProxyChoice(wxWindow *parent, wxWindowID id);
	virtual ~ProxyChoice();

	void OnChangeServer(wxCommandEvent &event);
	void OnChangePort(wxCommandEvent &event);
	void OnProxyTypeChange(wxChoicebookEvent &event);

private:


	DECLARE_EVENT_TABLE();
};

class ExeChoice: public wxChoice {
public:
	ExeChoice(wxWindow * parent, wxWindowID id) : wxChoice(parent, id) {}
	bool FindAndSetSelectionWithClientData(wxString item) {
		size_t number = this->GetStrings().size();
		for( size_t i = 0; i < number; i++ ) {
			FSOExecutable* data = dynamic_cast<FSOExecutable*>(this->GetClientObject(i));
			wxCHECK2_MSG( data != NULL, continue, _T("Client data is not a FSOVersion pointer"));
			if ( data->GetExecutableName() == item ) {
				this->SetSelection(i);
				return true;
			}
		}
		return false;
	}
};

BasicSettingsPage::BasicSettingsPage(wxWindow* parent): wxPanel(parent, wxID_ANY) {
	wxLogDebug(_T("BasicSettingsPage is at %p."), this);

	if (networkTypeOptions.empty() || networkSpeedOptions.empty()) {
		InitializeNetworkOptions();
	}
	
	TCManager::Initialize();
	TCManager::RegisterTCChanged(this);
	TCManager::RegisterTCBinaryChanged(this);
	TCManager::RegisterTCFredBinaryChanged(this);
	ProMan::GetProfileManager()->AddEventHandler(this);
	wxCommandEvent event(this->GetId());
	this->ProfileChanged(event);
}

void BasicSettingsPage::ProfileChanged(wxCommandEvent &WXUNUSED(event)) {
	if (this->GetSizer() != NULL) {
		this->GetSizer()->Clear(true);
	}

	ProMan* proman = ProMan::GetProfileManager();
	// exe Selection
	bool fredEnabled;
	proman->GlobalRead(GBL_CFG_OPT_CONFIG_FRED, &fredEnabled, false);

	wxStaticBox* exeBox = new wxStaticBox(this, wxID_ANY, _("FreeSpace 2 or total conversion root folder and executable"));

	wxStaticText* rootFolderText = new wxStaticText(this, ID_EXE_ROOT_FOLDER_BOX_TEXT, _("FS2 or TC root folder:"));
	wxTextCtrl* rootFolderBox = new wxTextCtrl(this, ID_EXE_ROOT_FOLDER_BOX);
	wxButton* selectButton = new wxButton(this, ID_EXE_SELECT_ROOT_BUTTON, _T("Browse..."));

	rootFolderBox->SetEditable(false);

	wxStaticText* useExeText = new wxStaticText(this, wxID_ANY, _("FS2 Open executable:"));
	ExeChoice* useExeChoice = new ExeChoice(this, ID_EXE_CHOICE_BOX);
	wxButton* exeChoiceRefreshButton = new wxButton(this, ID_EXE_CHOICE_REFRESH_BUTTON, _("Refresh"));

	wxStaticText* useFredText = NULL;
	ExeChoice* useFredChoice = NULL;
	wxButton* exeFredChoiceRefreshButton = NULL;
	if ( fredEnabled ) {
		useFredText = new wxStaticText(this, wxID_ANY, _("FRED2 Open executable:"));
		useFredChoice = new ExeChoice(this, ID_EXE_FRED_CHOICE_BOX);
		exeFredChoiceRefreshButton = new wxButton(this, ID_EXE_FRED_CHOICE_REFRESH_BUTTON, _("Refresh"));
	}

	
	// new sizer layout that should line things up nicely
	// inspired by the thread http://markmail.org/message/rlgv6y6xbw5dkvyy#query:+page:1+mid:5cqagz2jbygwqt2x+state:results
	// or "RE: [wxPython-users] wx.FlexGridSizer..." Mar 31, 2005 in com.googlegroups.wxpython-users
	// this idea could also work on, say, the video box, if you needed, for Windows
	wxFlexGridSizer* exeInsideSizer;
	if (useFredText != NULL && useFredChoice != NULL) {
		exeInsideSizer = new wxFlexGridSizer(3,3,0,0);
	} else {
		exeInsideSizer = new wxFlexGridSizer(2,3,0,0);
	}
	exeInsideSizer->AddGrowableCol(1);

	exeInsideSizer->Add(rootFolderText, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);
	exeInsideSizer->Add(rootFolderBox, wxSizerFlags().Proportion(1).Expand());
	exeInsideSizer->Add(selectButton, wxSizerFlags().Expand().Border(wxLEFT, 5));
	exeInsideSizer->Add(useExeText, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);
	exeInsideSizer->Add(useExeChoice, wxSizerFlags().Proportion(1).Expand());
	exeInsideSizer->Add(exeChoiceRefreshButton, wxSizerFlags().Expand().Border(wxLEFT, 5));

	if ((useFredText != NULL) && (useFredChoice != NULL) && (exeFredChoiceRefreshButton != NULL)) {
		exeInsideSizer->Add(useFredText, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);
		exeInsideSizer->Add(useFredChoice, wxSizerFlags().Proportion(1).Expand());
		exeInsideSizer->Add(exeFredChoiceRefreshButton, wxSizerFlags().Expand().Border(wxLEFT, 5));
	}

	wxStaticBoxSizer* exeSizer = new wxStaticBoxSizer(exeBox, wxHORIZONTAL);
	exeSizer->Add(exeInsideSizer, wxSizerFlags().Proportion(1).Expand().Border(wxLEFT|wxRIGHT|wxBOTTOM, 5));

	// Video Section
	wxStaticBox* videoBox = new wxStaticBox(this, ID_VIDEO_STATIC_BOX, _("Video"));

	wxStaticText* resolutionText = 
		new wxStaticText(this, wxID_ANY, _("Resolution:"));
	wxChoice* resolutionCombo = new wxChoice(this, ID_RESOLUTION_COMBO);
	this->FillResolutionDropBox(resolutionCombo);
	long width, height;
	bool hasResWidth = proman->ProfileRead(PRO_CFG_VIDEO_RESOLUTION_WIDTH, &width);
	bool hasResHeight = proman->ProfileRead(PRO_CFG_VIDEO_RESOLUTION_HEIGHT, &height);

	if ((!hasResWidth) || (!hasResHeight)) {
		wxLogDebug(_T("initializing resolution to maximum supported resolution"));
		int maxResIndex = GetMaxSupportedResolution(*resolutionCombo, width, height);
		wxCHECK_RET(maxResIndex != wxNOT_FOUND, _T("could not get max supported resolution"));
		resolutionCombo->SetSelection(maxResIndex);
	}

	const wxString resString = wxString::Format(CFG_RES_FORMAT_STRING,
		static_cast<int>(width), static_cast<int>(height));
	bool resFound = resolutionCombo->SetStringSelection(resString);

	if (!resFound) {
		wxLogWarning(_T("resolution %s not found, resetting to maximum supported resolution"),
			resString.c_str());
		int maxResIndex = GetMaxSupportedResolution(*resolutionCombo, width, height);
		wxCHECK_RET(maxResIndex != wxNOT_FOUND, _T("could not get max supported resolution"));
		resolutionCombo->SetSelection(maxResIndex);
	}

	if ((!hasResWidth) || (!hasResHeight) || (!resFound)) {
		proman->ProfileWrite(PRO_CFG_VIDEO_RESOLUTION_WIDTH, width);
		proman->ProfileWrite(PRO_CFG_VIDEO_RESOLUTION_HEIGHT, height);
	}

	wxStaticText* depthText = 
		new wxStaticText(this, wxID_ANY, _("Depth:"));
	wxChoice* depthCombo = new wxChoice(this, ID_DEPTH_COMBO);
	long bitDepth;
	depthCombo->Append(_("16-bit"));
	depthCombo->Append(_("32-bit"));
	proman->ProfileRead(PRO_CFG_VIDEO_BIT_DEPTH, &bitDepth, DEFAULT_VIDEO_BIT_DEPTH, true);
	depthCombo->SetSelection((bitDepth == 16) ? 0 : 1);

	wxStaticText* textureFilterText = 
		new wxStaticText(this, wxID_ANY, _("Texture filter:"));
	wxChoice* textureFilterCombo = new wxChoice(this, ID_TEXTURE_FILTER_COMBO);
	wxString filter;
	textureFilterCombo->Append(_("Bilinear"));
	textureFilterCombo->Append(_("Trilinear"));
	proman->ProfileRead(PRO_CFG_VIDEO_TEXTURE_FILTER, &filter, DEFAULT_VIDEO_TEXTURE_FILTER, true);
	// FIXME shouldn't need case folding. comparison should be case-sensitive:
	//       either Bilinear or Trilinear.
	//       although now we've created legacy texture filter values. hmm.
	filter.MakeLower();
	textureFilterCombo->SetSelection( (filter == _T("bilinear")) ? 0 : 1);

#if !IS_WIN32 // AF/AA don't yet work on Windows
	wxStaticText* anisotropicText = 
		new wxStaticText(this, wxID_ANY, _("Anisotropic:"));
	wxChoice* anisotropicCombo = new wxChoice(this, ID_ANISOTROPIC_COMBO);
	long anisotropic;
	anisotropicCombo->Append(_("Off"));
	anisotropicCombo->Append(_T(" 1x"));
	anisotropicCombo->Append(_T(" 2x"));
	anisotropicCombo->Append(_T(" 4x"));
	anisotropicCombo->Append(_T(" 8x"));
	anisotropicCombo->Append(_T("16x"));
	proman->ProfileRead(PRO_CFG_VIDEO_ANISOTROPIC, &anisotropic, DEFAULT_VIDEO_ANISOTROPIC, true);
	switch(anisotropic) {
		case 0:
			anisotropic = 0;
			break;
		case 1:
			anisotropic = 1;
			break;
		case 2:
			anisotropic = 2;
			break;
		case 4:
			anisotropic = 3;
			break;
		case 8:
			anisotropic = 4;
			break;
		case 16:
			anisotropic = 5;
			break;
		default:
			wxLogWarning(_T("invalid anisotropic factor %ld, setting to 0"),
				anisotropic);
			proman->ProfileWrite(PRO_CFG_VIDEO_ANISOTROPIC, static_cast<long>(0));
			anisotropic = 0;
	}
	anisotropicCombo->SetSelection(anisotropic);


	wxStaticText* aaText = new wxStaticText(this, wxID_ANY, _("Anti-aliasing:"));
	wxChoice* aaCombo = new wxChoice(this, ID_AA_COMBO);
	long antialias;
	aaCombo->Append(_("Off"));
	aaCombo->Append(_T(" 2x"));
	aaCombo->Append(_T(" 4x"));
	aaCombo->Append(_T(" 8x"));
	aaCombo->Append(_T("16x"));
	proman->ProfileRead(PRO_CFG_VIDEO_ANTI_ALIAS, &antialias, DEFAULT_VIDEO_ANTI_ALIAS, true);
	switch(antialias) {
		case 0:
			antialias = 0;
			break;
		case 2:
			antialias = 1;
			break;
		case 4:
			antialias = 2;
			break;
		case 8:
			antialias = 3;
			break;
		case 16:
			antialias = 4;
			break;
		default:
			wxLogWarning(_T("invalid anti-aliasing factor %ld, setting to 0"),
				antialias);
			proman->ProfileWrite(PRO_CFG_VIDEO_ANTI_ALIAS, static_cast<long>(0));
			antialias = 0;
	}
	aaCombo->SetSelection(antialias);
#endif

	// Sizer for graphics, resolution, depth, etc
	wxGridSizer* videoSizerL = new wxFlexGridSizer(2);
	videoSizerL->Add(resolutionText, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);
	videoSizerL->Add(resolutionCombo, wxSizerFlags().Expand());
	videoSizerL->Add(depthText, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);
	videoSizerL->Add(depthCombo, wxSizerFlags().Expand());

	wxGridSizer* videoSizerR = new wxFlexGridSizer(2);
	videoSizerR->Add(textureFilterText, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);
	videoSizerR->Add(textureFilterCombo, wxSizerFlags().Expand());
#if !IS_WIN32 // AF/AA don't yet work on Windows
	videoSizerR->Add(anisotropicText, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);
	videoSizerR->Add(anisotropicCombo, wxSizerFlags().Expand());
	videoSizerR->Add(aaText, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);
	videoSizerR->Add(aaCombo, wxSizerFlags().Expand());
#endif

	wxStaticBoxSizer* videoSizer = new wxStaticBoxSizer(videoBox, wxHORIZONTAL);
#if IS_WIN32
	videoSizer->Add(videoSizerL, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);
	videoSizer->Add(videoSizerR, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT, 5);
#else
	videoSizer->Add(videoSizerL, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxBOTTOM, 5);
	videoSizer->AddStretchSpacer(5);
	videoSizer->Add(videoSizerR, wxSizerFlags().Expand().Border(wxBOTTOM, 5));
	videoSizer->AddStretchSpacer(5);
#endif

#if IS_WIN32
	// Speech
	wxStaticBox* speechBox = new wxStaticBox(this, wxID_ANY, _("Speech"));
	wxTextCtrl* speechTestText = new wxTextCtrl(this, ID_SPEECH_TEST_TEXT,
		_("Press play to test this string."),
		wxDefaultPosition, wxDefaultSize,
		wxTE_MULTILINE);
	wxChoice* speechVoiceCombo = new wxChoice(this, ID_SPEECH_VOICE_COMBO);
	wxStaticText* speechVolumeLabel = new wxStaticText(this, wxID_ANY, _T("Volume"));
	wxSlider* speechVoiceVolume = 
		new wxSlider(this, ID_SPEECH_VOICE_VOLUME, 50, 0, 100);
	wxButton* speechPlayButton = 
		new wxButton(this, ID_SPEECH_PLAY_BUTTON, _("Play"));
	wxStaticText* speechUseInText = 
		new wxStaticText(this, wxID_ANY, _("Use simulated speech in:"));
	wxCheckBox* speechInTechroomCheck = 
		new wxCheckBox(this, ID_SPEECH_IN_TECHROOM, _("Tech room"));
	wxCheckBox* speechInBriefingCheck = 
		new wxCheckBox(this, ID_SPEECH_IN_BRIEFING, _("Briefings"));
	wxCheckBox* speechInGameCheck = 
		new wxCheckBox(this, ID_SPEECH_IN_GAME, _("In-game"));
	wxCheckBox* speechInMultiCheck=
		new wxCheckBox(this, ID_SPEECH_IN_MULTI, _("Multiplayer"));

	wxButton* speechMoreVoicesButton = 
		new wxButton(this, ID_SPEECH_MORE_VOICES_BUTTON, _("Get more voices"));

	wxGridBagSizer* speechLeftSizer = new wxGridBagSizer();
	speechLeftSizer->Add(speechVoiceCombo, wxGBPosition(0,0), wxGBSpan(1,1), wxEXPAND|wxRIGHT, 10);
	speechLeftSizer->Add(speechMoreVoicesButton, wxGBPosition(0,2), wxGBSpan(1,1), wxEXPAND);
	speechLeftSizer->Add(speechTestText, wxGBPosition(1,0), wxGBSpan(2,3), wxEXPAND|wxTOP|wxBOTTOM, 5);
	speechLeftSizer->Add(speechPlayButton, wxGBPosition(3,0), wxGBSpan(1,1), wxALIGN_CENTER_HORIZONTAL|wxRIGHT, 10);
	speechLeftSizer->Add(speechVolumeLabel, wxGBPosition(3,1), wxGBSpan(1,1), wxEXPAND|wxALIGN_CENTER_VERTICAL|wxALIGN_RIGHT|wxTOP, 5);
	speechLeftSizer->Add(speechVoiceVolume, wxGBPosition(3,2), wxGBSpan(1,1), wxEXPAND);

	wxBoxSizer* speechRightSizer = new wxBoxSizer(wxVERTICAL);
	speechRightSizer->Add(speechUseInText, wxSizerFlags().Expand().Border(wxBOTTOM, 5));
	speechRightSizer->Add(speechInTechroomCheck, wxSizerFlags().Expand().Border(wxBOTTOM, 5));
	speechRightSizer->Add(speechInBriefingCheck, wxSizerFlags().Expand().Border(wxBOTTOM, 5));
	speechRightSizer->Add(speechInGameCheck, wxSizerFlags().Expand().Border(wxBOTTOM, 5));
	speechRightSizer->Add(speechInMultiCheck, wxSizerFlags().Expand());

	wxStaticBoxSizer* speechSizer = new wxStaticBoxSizer(speechBox, wxHORIZONTAL);
	speechSizer->Add(speechLeftSizer, wxSizerFlags().Expand().Border(wxLEFT|wxBOTTOM, 5));
	speechSizer->AddStretchSpacer(3);
	speechSizer->Add(speechRightSizer, wxSizerFlags().Expand().Border(wxBOTTOM, 5));
	speechSizer->AddStretchSpacer(2);

	if ( SpeechMan::WasBuiltIn() && SpeechMan::Initialize() ) {

		speechVoiceCombo->Append(SpeechMan::EnumVoices());

		// FIXME consolidate this code and similar code in AdvSettingsPage.cpp
		wxClientDC dc(this);
		wxArrayString voices = speechVoiceCombo->GetStrings();
		wxFont font(this->GetFont());
		int maxStringWidth = 0;
		int x, y;

		for (int i = 0, n = voices.GetCount(); i < n; ++i) {
			dc.GetTextExtent(voices[i], &x, &y, NULL, NULL, &font);

			if (x > maxStringWidth) {
				maxStringWidth = x;
			}
		}

		speechVoiceCombo->SetMinSize(wxSize(maxStringWidth + 40, // 40 to include drop down box control
			speechVoiceCombo->GetSize().GetHeight()));
		this->Layout();

		long speechVoice;
		int speechSystemVoice = SpeechMan::GetVoice();
		if ( speechSystemVoice < 0 ) {
			wxLogWarning(_T("Had problem retrieving the system voice, using voice %d"),
				DEFAULT_SPEECH_VOICE);
			speechSystemVoice = DEFAULT_SPEECH_VOICE;
		}
		// set the voice to what is in the profile, if not set in profile use
		// system settings
		proman->ProfileRead(PRO_CFG_SPEECH_VOICE, &speechVoice, speechSystemVoice, true);
		// there should not be more than MAX_INT voices installed on a system so
		// the cast of an unsigned int to a signed int should not result in a 
		// loss of data.
		if ( speechVoice >= static_cast<int>(speechVoiceCombo->GetCount()) ) {
			wxLogWarning(_T("Profile speech voice index out of range,")
				_T(" setting to system default"));
			speechVoice = speechSystemVoice;
		}
		speechVoiceCombo->SetSelection(speechVoice);

		long speechVolume;
		int speechSystemVolume = SpeechMan::GetVolume();
		if (speechSystemVolume < 0) {
			wxLogWarning(_T("Had problem in retrieving the system speech volume,")
				_T(" setting to %d"), DEFAULT_SPEECH_VOLUME);
			speechSystemVolume = DEFAULT_SPEECH_VOLUME;
		}
		proman->ProfileRead(PRO_CFG_SPEECH_VOLUME, &speechVolume, speechSystemVolume, true);
		if ( speechVolume < 0 || speechVolume > 100 ) {
			wxLogWarning(_T("Speech Volume recorded in profile is out of range,")
				_T(" resetting to %d"), DEFAULT_SPEECH_VOLUME);
			speechVolume = DEFAULT_SPEECH_VOLUME;
		}
		speechVoiceVolume->SetValue(speechVolume);


		bool speechInTechroom;
		proman->ProfileRead(
			PRO_CFG_SPEECH_IN_TECHROOM, &speechInTechroom, DEFAULT_SPEECH_IN_TECHROOM, true);
		speechInTechroomCheck->SetValue(speechInTechroom);

		bool speechInBriefings;
		proman->ProfileRead(
			PRO_CFG_SPEECH_IN_BRIEFINGS, &speechInBriefings, DEFAULT_SPEECH_IN_BRIEFINGS, true);
		speechInBriefingCheck->SetValue(speechInBriefings);

		bool speechInGame;
		proman->ProfileRead(
			PRO_CFG_SPEECH_IN_GAME, &speechInGame, DEFAULT_SPEECH_IN_GAME, true);
		speechInGameCheck->SetValue(speechInGame);

		bool speechInMulti;
		proman->ProfileRead(
			PRO_CFG_SPEECH_IN_MULTI, &speechInMulti, DEFAULT_SPEECH_IN_MULTI, true);
		speechInMultiCheck->SetValue(speechInMulti);
	} else {
		speechBox->Disable();
		speechTestText->Disable();
		speechVoiceCombo->Disable();
		speechVoiceVolume->Disable();
		speechPlayButton->Disable();
		speechUseInText->Disable();
		speechInTechroomCheck->Disable();
		speechInBriefingCheck->Disable();
		speechInGameCheck->Disable();
		speechInMultiCheck->Disable();
		speechMoreVoicesButton->Disable();
	}
#endif

	// Network
	wxStaticBox* networkBox = new wxStaticBox(this, wxID_ANY, _("Network"));

	wxChoice* networkType = new wxChoice(this, ID_NETWORK_TYPE);
	for (NetworkSettingsOptions::const_iterator
		 it = networkTypeOptions.begin(),
		 end = networkTypeOptions.end();
		 it != end; ++it) {
		networkType->Append(it->GetDescription());
	}
	
	wxString type;
	proman->ProfileRead(PRO_CFG_NETWORK_TYPE, &type,
		DEFAULT_NETWORK_TYPE, true);
	
	int networkTypeSelection =
		FindOptionIndexWithRegistryValue(networkTypeOptions, type);
	if (networkTypeSelection < 0) {
		wxLogError(
			_T("Registry value '%s' was not found in list of options. Using default '%s'."),
				type.c_str(), DEFAULT_NETWORK_TYPE.c_str());		
	}
	
	networkTypeSelection =
		FindOptionIndexWithRegistryValue(networkTypeOptions, DEFAULT_NETWORK_TYPE);
	if (networkTypeSelection < 0) {
		wxLogError(
			_T("Default value '%s' was not found in list of type speed options. Using first entry '%s'."),
				DEFAULT_NETWORK_TYPE.c_str(),
				networkTypeOptions[0].GetRegistryValue().c_str());
		networkTypeSelection = 0;
	}

	networkType->SetSelection(networkTypeSelection);
	
	wxChoice* networkSpeed = new wxChoice(this, ID_NETWORK_SPEED);
	for (NetworkSettingsOptions::const_iterator
		 it = networkSpeedOptions.begin(),
		 end = networkSpeedOptions.end();
		 it != end; ++it) {
		networkSpeed->Append(it->GetDescription());
	}
	
	wxString speed;
	proman->ProfileRead(PRO_CFG_NETWORK_SPEED, &speed,
		DEFAULT_NETWORK_SPEED, true);
	
	int networkSpeedSelection =
		FindOptionIndexWithRegistryValue(networkSpeedOptions, speed);
	if (networkSpeedSelection < 0) {
		wxLogError(
			_T("Registry value '%s' was not found in list of speed options. Using default '%s'."),
				speed.c_str(), DEFAULT_NETWORK_SPEED.c_str());		
	}
	
	networkSpeedSelection =
		FindOptionIndexWithRegistryValue(networkSpeedOptions, DEFAULT_NETWORK_SPEED);
	if (networkSpeedSelection < 0) {
		wxLogError(
			_T("Default value '%s' was not found in list of speed options. Using first entry '%s'."),
				DEFAULT_NETWORK_SPEED.c_str(),
				networkSpeedOptions[0].GetRegistryValue().c_str());
		networkSpeedSelection = 0;
	}
	
	networkSpeed->SetSelection(networkSpeedSelection);

	wxTextCtrl* networkPort = 
		new wxTextCtrl(this, ID_NETWORK_PORT, wxEmptyString);
	long port;
	proman->ProfileRead(PRO_CFG_NETWORK_PORT, &port, DEFAULT_NETWORK_PORT, true);
	if (port != DEFAULT_NETWORK_PORT) {
		networkPort->ChangeValue(wxString::Format(_T("%ld"), port));
	}
	networkPort->SetValidator(wxTextValidator(wxFILTER_NUMERIC));
	networkPort->SetMaxLength(5);

	wxTextCtrl* networkIP = new wxTextCtrl(this, ID_NETWORK_IP, wxEmptyString);
	wxString ip;
	proman->ProfileRead(PRO_CFG_NETWORK_IP, &ip, DEFAULT_NETWORK_IP, true);
	networkIP->ChangeValue(ip);
	networkIP->SetMaxLength(15); // for ###.###.###.###
	
	wxGridSizer* networkInsideSizerL = new wxFlexGridSizer(2);
	networkInsideSizerL->Add(new wxStaticText(this, wxID_ANY, _("Connection type:")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);
	networkInsideSizerL->Add(networkType, wxSizerFlags().Expand());
	networkInsideSizerL->Add(new wxStaticText(this, wxID_ANY, _("Connection speed:")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);
	networkInsideSizerL->Add(networkSpeed, wxSizerFlags().Expand());
	
	wxGridSizer* networkInsideSizerR = new wxFlexGridSizer(2);
	networkInsideSizerR->Add(new wxStaticText(this, wxID_ANY, _("Force local port:")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);
	networkInsideSizerR->Add(networkPort, wxSizerFlags().Expand());
	networkInsideSizerR->Add(new wxStaticText(this, wxID_ANY, _("Force IP address:")), 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);
	networkInsideSizerR->Add(networkIP, wxSizerFlags().Expand());
	
	wxStaticBoxSizer* networkSizer = new wxStaticBoxSizer(networkBox, wxHORIZONTAL);
	networkSizer->Add(networkInsideSizerL, wxSizerFlags().Expand().Border(wxLEFT|wxBOTTOM, 5));
	networkSizer->AddStretchSpacer(5);
	networkSizer->Add(networkInsideSizerR, wxSizerFlags().Expand().Border(wxBOTTOM, 5));
	networkSizer->AddStretchSpacer(5);

	// Audio
	wxStaticBox* audioBox = new wxStaticBox(this, wxID_ANY, _("Audio"));

	this->soundDeviceText = new wxStaticText(this, wxID_ANY, _("Sound device:"));
	this->soundDeviceCombo = new wxChoice(this, ID_SELECT_SOUND_DEVICE);
	this->openALVersion = new wxStaticText(this, wxID_ANY, wxEmptyString);

	this->downloadOpenALButton = new wxButton(this, ID_DOWNLOAD_OPENAL, _("Download OpenAL"));
	this->detectOpenALButton = new wxButton(this, ID_DETECT_OPENAL, _("Detect OpenAL"));

	this->audioNonButtonsSizer = new wxBoxSizer(wxVERTICAL);
	this->audioNonButtonsSizer->Add(soundDeviceText, wxSizerFlags().Border(wxBOTTOM, 5));
	this->audioNonButtonsSizer->Add(soundDeviceCombo,
					wxSizerFlags().Expand());
	this->audioNonButtonsSizer->Add(openALVersion,
					wxSizerFlags().Center().Border(wxTOP, 5));
	
	this->audioButtonsSizer = new wxBoxSizer(wxVERTICAL);
	this->audioButtonsSizer->Add(downloadOpenALButton, wxSizerFlags().Expand());
	this->audioButtonsSizer->Add(detectOpenALButton, wxSizerFlags().Expand());
	
	this->audioSizer = new wxStaticBoxSizer(audioBox, wxHORIZONTAL);
	this->audioSizer->Add(audioNonButtonsSizer, wxSizerFlags().Proportion(1).Expand().Border(wxLEFT|wxRIGHT|wxBOTTOM, 5));
	this->audioSizer->Add(audioButtonsSizer, 0, wxALIGN_CENTER_VERTICAL|wxLEFT|wxRIGHT|wxBOTTOM, 5);

	// fill in controls
	this->SetupOpenALSection();

	// Joystick
	wxStaticBox* joystickBox = new wxStaticBox(this, wxID_ANY, _("Joystick"));

	this->joystickSelected = new wxChoice(this, ID_JOY_SELECTED);
#if IS_WIN32
	this->joystickForceFeedback = new wxCheckBox(this, ID_JOY_FORCE_FEEDBACK, _("Force feedback"));
	this->joystickDirectionalHit = new wxCheckBox(this, ID_JOY_DIRECTIONAL_HIT, _("Directional hit"));
	this->joystickCalibrateButton = new wxButton(this, ID_JOY_CALIBRATE_BUTTON, _("Calibrate"));
	this->joystickDetectButton = new wxButton(this, ID_JOY_DETECT_BUTTON, _("Detect"));
#else
	// FIXME get Detect button working on Linux/OS X
	wxStaticText* detectJoystickText = new wxStaticText(this, wxID_ANY,
														_("Restart launcher to re-detect joysticks."),
														wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
#endif

	this->SetupJoystickSection();

	wxBoxSizer* joystickDetectionSizer = new wxBoxSizer(wxVERTICAL);
	joystickDetectionSizer->Add(joystickSelected, wxSizerFlags().Proportion(1).Expand().Border(wxBOTTOM, 5));
#if IS_WIN32
	joystickDetectionSizer->Add(joystickDetectButton, wxSizerFlags().Right());
#else
	joystickDetectionSizer->Add(detectJoystickText, wxSizerFlags().Center());
#endif

#if IS_WIN32
	wxBoxSizer* joystickExtrasSizer = new wxBoxSizer(wxVERTICAL);
	joystickExtrasSizer->Add(joystickForceFeedback, wxSizerFlags().Expand().Border(wxBOTTOM, 5));
	joystickExtrasSizer->Add(joystickDirectionalHit, wxSizerFlags().Expand().Border(wxBOTTOM, 5));
	joystickExtrasSizer->Add(joystickCalibrateButton, wxSizerFlags().Expand());
#endif

	wxStaticBoxSizer* joystickSizer = new wxStaticBoxSizer(joystickBox, wxHORIZONTAL);
#if IS_WIN32
	joystickSizer->Add(joystickDetectionSizer, 1, wxALIGN_BOTTOM|wxLEFT|wxRIGHT|wxBOTTOM, 5);
	joystickSizer->Add(joystickExtrasSizer, wxSizerFlags().Expand().Border(wxLEFT|wxRIGHT|wxBOTTOM, 5));
#else
	joystickSizer->Add(joystickDetectionSizer, wxSizerFlags().Expand().Proportion(1).Border(wxLEFT|wxRIGHT|wxBOTTOM, 5));
#endif

	// Proxy
	// sorry, but there won't be space for the proxy on any platform
#if 0
	wxStaticBox* proxyBox = new wxStaticBox(this, wxID_ANY, _("Proxy"));

	wxChoicebook* proxyChoice = new ProxyChoice(this, ID_PROXY_TYPE);

	wxStaticBoxSizer* proxySizer = new wxStaticBoxSizer(proxyBox, wxVERTICAL);
	proxySizer->Add(proxyChoice, wxSizerFlags().Expand().Border(wxLEFT|wxRIGHT, 5));
#endif

	// Final Layout
	wxBoxSizer* settingsSizer = new wxBoxSizer(wxVERTICAL);

#if IS_WIN32
	wxBoxSizer* videoAudioSizer = new wxBoxSizer(wxHORIZONTAL);
	videoAudioSizer->Add(videoSizer, wxSizerFlags().Expand().Border(wxRIGHT, 10));
	videoAudioSizer->Add(audioSizer, wxSizerFlags().Proportion(1).Expand());
	settingsSizer->Add(videoAudioSizer, wxSizerFlags().Expand().Border(wxBOTTOM, 5));
	settingsSizer->Add(speechSizer, wxSizerFlags().Expand().Border(wxBOTTOM, 5));
	settingsSizer->Add(joystickSizer, wxSizerFlags().Expand().Border(wxBOTTOM, 5));
	settingsSizer->Add(networkSizer, wxSizerFlags().Expand());
#else
	settingsSizer->Add(videoSizer, wxSizerFlags().Expand().Border(wxBOTTOM, 5));
	settingsSizer->Add(audioSizer, wxSizerFlags().Expand().Border(wxBOTTOM, 5));
	settingsSizer->Add(joystickSizer, wxSizerFlags().Expand().Border(wxBOTTOM, 5));
	settingsSizer->Add(networkSizer, wxSizerFlags().Expand());
#endif

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->SetMinSize(wxSize(TAB_AREA_WIDTH-5, -1)); // 5 being for the border
	sizer->Add(exeSizer, wxSizerFlags().Expand().Border(wxALL, 5));
	sizer->Add(settingsSizer, wxSizerFlags().Expand().Border(wxLEFT|wxRIGHT|wxBOTTOM, 5));

	this->SetSizer(sizer);
	this->Layout();
}

BasicSettingsPage::~BasicSettingsPage() {
	TCManager::DeInitialize();
	if ( SpeechMan::IsInitialized() ) {
		SpeechMan::DeInitialize();
	}
	JoyMan::DeInitialize();
	OpenALMan::DeInitialize();
}

/// Event Handling
BEGIN_EVENT_TABLE(BasicSettingsPage, wxPanel)
EVT_BUTTON(ID_EXE_SELECT_ROOT_BUTTON, BasicSettingsPage::OnSelectTC)
EVT_CHOICE(ID_EXE_CHOICE_BOX, BasicSettingsPage::OnSelectExecutable)
EVT_BUTTON(ID_EXE_CHOICE_REFRESH_BUTTON, BasicSettingsPage::OnPressExecutableChoiceRefreshButton)
EVT_CHOICE(ID_EXE_FRED_CHOICE_BOX, BasicSettingsPage::OnSelectFredExecutable)
EVT_BUTTON(ID_EXE_FRED_CHOICE_REFRESH_BUTTON, BasicSettingsPage::OnPressFredExecutableChoiceRefreshButton)
EVT_COMMAND(wxID_NONE, EVT_TC_CHANGED, BasicSettingsPage::OnTCChanged)
EVT_COMMAND(wxID_NONE, EVT_TC_BINARY_CHANGED, BasicSettingsPage::OnCurrentBinaryChanged)
EVT_COMMAND(wxID_NONE, EVT_TC_FRED_BINARY_CHANGED, BasicSettingsPage::OnCurrentFredBinaryChanged)

// Video controls
EVT_CHOICE(ID_RESOLUTION_COMBO, BasicSettingsPage::OnSelectVideoResolution)
EVT_CHOICE(ID_DEPTH_COMBO, BasicSettingsPage::OnSelectVideoDepth)
EVT_CHOICE(ID_TEXTURE_FILTER_COMBO, BasicSettingsPage::OnSelectVideoTextureFilter)
EVT_CHOICE(ID_ANISOTROPIC_COMBO, BasicSettingsPage::OnSelectVideoAnisotropic)
EVT_CHOICE(ID_AA_COMBO, BasicSettingsPage::OnSelectVideoAntiAlias)

// Speech Controls
EVT_CHOICE(ID_SPEECH_VOICE_COMBO, BasicSettingsPage::OnSelectSpeechVoice)
EVT_SLIDER(ID_SPEECH_VOICE_VOLUME, BasicSettingsPage::OnChangeSpeechVolume)
EVT_BUTTON(ID_SPEECH_PLAY_BUTTON, BasicSettingsPage::OnPlaySpeechText)
EVT_CHECKBOX(ID_SPEECH_IN_TECHROOM, BasicSettingsPage::OnToggleSpeechInTechroom)
EVT_CHECKBOX(ID_SPEECH_IN_BRIEFING, BasicSettingsPage::OnToggleSpeechInBriefing)
EVT_CHECKBOX(ID_SPEECH_IN_GAME, BasicSettingsPage::OnToggleSpeechInGame)
EVT_CHECKBOX(ID_SPEECH_IN_MULTI, BasicSettingsPage::OnToggleSpeechInMulti)
EVT_BUTTON(ID_SPEECH_MORE_VOICES_BUTTON, BasicSettingsPage::OnGetMoreVoices)

// Network
EVT_CHOICE(ID_NETWORK_TYPE, BasicSettingsPage::OnSelectNetworkType)
EVT_CHOICE(ID_NETWORK_SPEED, BasicSettingsPage::OnSelectNetworkSpeed)
EVT_TEXT(ID_NETWORK_PORT, BasicSettingsPage::OnChangePort)
EVT_TEXT(ID_NETWORK_IP, BasicSettingsPage::OnChangeIP)

// OpenAL
EVT_CHOICE(ID_SELECT_SOUND_DEVICE, BasicSettingsPage::OnSelectSoundDevice)
EVT_CHOICE(ID_SELECT_CAPTURE_DEVICE, BasicSettingsPage::OnSelectCaptureDevice)
EVT_CHECKBOX(ID_ENABLE_EFX, BasicSettingsPage::OnToggleEnableEFX)
EVT_TEXT(ID_AUDIO_SAMPLE_RATE, BasicSettingsPage::OnChangeSampleRate)
EVT_BUTTON(ID_DOWNLOAD_OPENAL, BasicSettingsPage::OnDownloadOpenAL)
EVT_BUTTON(ID_DETECT_OPENAL, BasicSettingsPage::OnDetectOpenAL)

// Joystick
EVT_CHOICE(ID_JOY_SELECTED, BasicSettingsPage::OnSelectJoystick)
EVT_CHECKBOX(ID_JOY_FORCE_FEEDBACK, BasicSettingsPage::OnCheckForceFeedback)
EVT_CHECKBOX(ID_JOY_DIRECTIONAL_HIT, BasicSettingsPage::OnCheckDirectionalHit)
EVT_BUTTON(ID_JOY_CALIBRATE_BUTTON, BasicSettingsPage::OnCalibrateJoystick)
EVT_BUTTON(ID_JOY_DETECT_BUTTON, BasicSettingsPage::OnDetectJoystick)

// Profile
EVT_COMMAND(wxID_NONE, EVT_CURRENT_PROFILE_CHANGED, BasicSettingsPage::ProfileChanged)

END_EVENT_TABLE()

void BasicSettingsPage::OnSelectTC(wxCommandEvent &WXUNUSED(event)) {
	wxString directory;
	ProMan* proman = ProMan::GetProfileManager();
	proman->ProfileRead(PRO_CFG_TC_ROOT_FOLDER, &directory, wxEmptyString);
	wxDirDialog filechooser(this, _T("Choose the root folder of a FreeSpace 2 installation or a total conversion"),
		directory, wxDD_DEFAULT_STYLE|wxDD_DIR_MUST_EXIST);

	wxString chosenDirectory;
	wxFileName path;
	while (true) {
		if ( wxID_CANCEL == filechooser.ShowModal() ) {
			return;
		}
		chosenDirectory = filechooser.GetPath();
		if ( chosenDirectory == directory ) {
			wxLogInfo(_T("The TC root folder selection was not changed."));
			return; // User canceled, bail out.
		}
		path.SetPath(chosenDirectory);
		if ( !path.IsOk() ) {
			wxLogWarning(_T("Folder is not valid"));
			continue;
		} else if ( FSOExecutable::IsRootFolderValid(path) ) {
			break;
		} else {
			wxLogWarning(_T("Folder does not have any supported executables"));
		}
	}
	wxLogDebug(_T("User chose '%s' as the TC root folder"), path.GetPath().c_str());
	proman->ProfileWrite(PRO_CFG_TC_ROOT_FOLDER, path.GetPath());
	TCManager::GenerateTCChanged();
}

/** Handles TCChanged events from TCManager.
//FIXME rewrite OnTCChanged() documentation once revisions are complete
Currently function only changes the executable dropbox control (clearing, and 
filling in the executables that are in the new TC folder) and removes the
currently select executable from the active profile only if the executable
specified in the profile does not exist in the TC.

\note clearing the selected executable disables the play button.
\note Emits a EVT_TC_BINARY_CHANGED in any case.*/
void BasicSettingsPage::OnTCChanged(wxCommandEvent &WXUNUSED(event)) {
	ExeChoice* exeChoice = dynamic_cast<ExeChoice*>(
		wxWindow::FindWindowById(ID_EXE_CHOICE_BOX, this));
	wxCHECK_RET( exeChoice != NULL, 
		_T("Cannot find executable choice control"));

	wxButton* exeChoiceRefreshButton = dynamic_cast<wxButton*>(
		wxWindow::FindWindowById(ID_EXE_CHOICE_REFRESH_BUTTON, this));
	wxCHECK_RET( exeChoiceRefreshButton != NULL,
		_T("Cannot find executable choice refresh button"));
	
	bool fredEnabled;
	ProMan::GetProfileManager()->GlobalRead(GBL_CFG_OPT_CONFIG_FRED, &fredEnabled, false);

	ExeChoice* fredChoice = NULL;
	wxButton* fredChoiceRefreshButton = NULL;
	if (fredEnabled) {
		fredChoice = dynamic_cast<ExeChoice*>(
			wxWindow::FindWindowById(ID_EXE_FRED_CHOICE_BOX, this));
		wxCHECK_RET( fredChoice != NULL, 
			_T("Cannot find FRED executable choice control"));
		
		fredChoiceRefreshButton = dynamic_cast<wxButton*>(
			wxWindow::FindWindowById(ID_EXE_FRED_CHOICE_REFRESH_BUTTON, this));
		wxCHECK_RET( fredChoiceRefreshButton != NULL,
			_T("Cannot find FRED executable choice refresh button"));
	}

	wxTextCtrl* tcFolder = dynamic_cast<wxTextCtrl*>(
		wxWindow::FindWindowById(ID_EXE_ROOT_FOLDER_BOX, this));
	wxCHECK_RET( tcFolder != NULL, 
		_T("Cannot find Text Control to show folder in."));

	wxString tcPath, binaryName, fredBinaryName;
	exeChoice->Clear();
	if (fredEnabled) {
		fredChoice->Clear();
	}

	if ( ProMan::GetProfileManager()->ProfileRead(PRO_CFG_TC_ROOT_FOLDER, &tcPath) ) {
		wxLogInfo(_T("The current FS2/TC root folder is %s"), tcPath.c_str());
		tcFolder->ChangeValue(tcPath);

		// note that disabling the controls is necessary if we reached this code from the
		// "refresh list of FSO execs" button being pressed
		if (!wxFileName::DirExists(tcPath)) {
			this->isTcRootFolderValid = false;
			this->DisableExecutableChoiceControls(NONEXISTENT_TC_ROOT_FOLDER);
		} else if (!FSOExecutable::HasFSOExecutables(wxFileName(tcPath, wxEmptyString))) {
			this->isTcRootFolderValid = false;
			this->DisableExecutableChoiceControls(INVALID_TC_ROOT_FOLDER);
		} else { // the root folder is valid
			this->isTcRootFolderValid = true;

			this->FillFSOExecutableDropBox(exeChoice, wxFileName(tcPath, wxEmptyString));
			exeChoice->Enable();
			exeChoiceRefreshButton->Enable();

			if (fredEnabled) {
				this->FillFredExecutableDropBox(fredChoice, wxFileName(tcPath, wxEmptyString));
				fredChoice->Enable();
				fredChoiceRefreshButton->Enable();
			}

			// set selection to profile entry for current binary if there is one, noting if selected binary can't be found
			bool hasBinary = ProMan::GetProfileManager()->ProfileRead(PRO_CFG_TC_CURRENT_BINARY, &binaryName);
			if ( hasBinary && !exeChoice->FindAndSetSelectionWithClientData(binaryName) ) {
				// no need for a warning, since classes handling the EVT_TC_BINARY_CHANGED will issue warnings
				wxLogDebug(_T("BasicSettingsPage::OnTCChanged(): couldn't find selected FSO executable %s in list of executables"),
					binaryName.c_str());
			}

			if (fredEnabled) {
				// set selection to profile entry for current FRED binary if there is one,
				// noting if the FRED binary can't be found
				bool hasFredBinary =
					ProMan::GetProfileManager()->ProfileRead(PRO_CFG_TC_CURRENT_FRED, &fredBinaryName);
				if ( hasFredBinary && !fredChoice->FindAndSetSelectionWithClientData(fredBinaryName) ) {
					// no need for a warning, since classes handling the EVT_TC_FRED_BINARY_CHANGED will issue warnings
					wxLogDebug(_T("BasicSettingsPage::OnTCChanged(): couldn't find selected FRED executable %s in list of executables"),
						fredBinaryName.c_str());
				}
			}
		}
		wxLogDebug(_T("The current root folder is %s."), this->isTcRootFolderValid ? _T("valid") : _T("invalid"));
	} else {
		wxLogDebug(_T("The current profile has no entry for root folder."));
		this->isTcRootFolderValid = false;
		this->DisableExecutableChoiceControls(MISSING_TC_ROOT_FOLDER);
	}
	this->GetSizer()->Layout();

	// TCManager::CurrentProfileChanged() (which calls TCManager::GenerateTCChanged())
	// assumes that TCManager::GenerateTCBinaryChanged() is called here unconditionally
	TCManager::GenerateTCBinaryChanged();
	// TCManager::CurrentProfileChanged() also assumes that TCManager::GenerateTCFredBinaryChanged()
	// is called here unconditionally if FRED launching is enabled
	if (fredEnabled) {
		TCManager::GenerateTCFredBinaryChanged();
	}
}

/** Puts the pretty description of all of the executables in the TCs folder
into the Executable DropBox.  This function does nothing else to the choice
control, not even clearing the drop box (call the Clear function if you don't
want the old items to stay. */
void BasicSettingsPage::FillFSOExecutableDropBox(wxChoice* exeChoice, wxFileName path) {
	BasicSettingsPage::FillExecutableDropBox(exeChoice, FSOExecutable::GetBinariesFromRootFolder(path));
}

void BasicSettingsPage::FillFredExecutableDropBox(wxChoice* exeChoice, wxFileName path) {
	BasicSettingsPage::FillExecutableDropBox(exeChoice, FSOExecutable::GetFredBinariesFromRootFolder(path));
}

bool compareExecutables(FSOExecutable exe1, FSOExecutable exe2) {
	return exe1.GetVersionString().CmpNoCase(exe2.GetVersionString()) < 0;
}

void BasicSettingsPage::FillExecutableDropBox(wxChoice* exeChoice, wxArrayString exes) {
	std::vector<FSOExecutable> fsoExes;
	
	wxArrayString::iterator iter = exes.begin();
	while ( iter != exes.end() ) {
		wxFileName path(*iter);
#if IS_APPLE // need complete path through app bundle to executable
		FSOExecutable ver = FSOExecutable::GetBinaryVersion(path.GetFullPath());
#else
		FSOExecutable ver = FSOExecutable::GetBinaryVersion(path.GetFullName());
#endif
		
		fsoExes.push_back(ver);
		iter++;
	}
	
	sort(fsoExes.begin(), fsoExes.end(), compareExecutables);
	
	for (std::vector<FSOExecutable>::const_iterator
		 it = fsoExes.begin(), end = fsoExes.end();
		 it != end; ++it) {
		exeChoice->Append(it->GetVersionString(), new FSOExecutable(*it));
	}
}

void BasicSettingsPage::OnSelectExecutable(wxCommandEvent &WXUNUSED(event)) {
	ExeChoice* choice = dynamic_cast<ExeChoice*>(
		wxWindow::FindWindowById(ID_EXE_CHOICE_BOX, this));
	wxCHECK_RET( choice != NULL, 
		_T("OnSelectExecutable: cannot find FS2 Open choice drop down box"));

	FSOExecutable* ver = dynamic_cast<FSOExecutable*>(
		choice->GetClientObject(choice->GetSelection()));
	wxCHECK_RET( ver != NULL,
		_T("OnSelectExecutable: choice does not have FSOVersion data"));
	wxLogDebug(_T("Have selected ver for %s"), ver->GetExecutableName().c_str());

	ProMan::GetProfileManager()->ProfileWrite(PRO_CFG_TC_CURRENT_BINARY, ver->GetExecutableName());
	TCManager::GenerateTCBinaryChanged();
}

void BasicSettingsPage::OnPressExecutableChoiceRefreshButton(wxCommandEvent &WXUNUSED(event)) {
	ExeChoice* exeChoice = dynamic_cast<ExeChoice*>(
		wxWindow::FindWindowById(ID_EXE_CHOICE_BOX, this));
	wxCHECK_RET( exeChoice != NULL, 
		_T("Cannot find executable choice control"));

	wxString tcPath, binaryName;
	wxCHECK_RET(ProMan::GetProfileManager()->ProfileRead(PRO_CFG_TC_ROOT_FOLDER, &tcPath),
		_T("OnPressExecutableChoiceRefreshButton: FS2/TC root folder entry not found"));
	
	if (!wxFileName::DirExists(tcPath)) {
		// warning not needed, since it will be issued by DisableExecutableChoiceControls
		wxLogDebug(_T("OnPressExecutableChoiceRefreshButton: root folder '%s' no longer exists"),
			tcPath.c_str());
		TCManager::GenerateTCChanged();
		return;
	}
	
	exeChoice->Clear();

	this->FillFSOExecutableDropBox(exeChoice, wxFileName(tcPath, wxEmptyString));

	if (exeChoice->IsEmpty()) {
		// current root folder has no FSO executables. update GUI if there were FSO executables before
		if (this->isTcRootFolderValid) {
			wxLogWarning(_T("after refreshing list of FSO executables, none were found"));
			TCManager::GenerateTCChanged();
		}
	} else if (!this->isTcRootFolderValid) {
		wxLogInfo(_T("after refreshing list of FSO executables, some have now been found"));
		TCManager::GenerateTCChanged();
	} else {
		// set selection to profile entry for current binary if there is one,
		// noting if selected binary can't be found and could be found before or vice versa
		if (ProMan::GetProfileManager()->ProfileRead(PRO_CFG_TC_CURRENT_BINARY, &binaryName)) {
			bool exeFound = exeChoice->FindAndSetSelectionWithClientData(binaryName);
			if (!exeFound && this->isCurrentBinaryValid) {
				wxLogDebug(_T("OnPressExecutableChoiceRefresh: couldn't find selected FSO executable %s in list of executables"),
					binaryName.c_str());
				TCManager::GenerateTCBinaryChanged();
			} else if (exeFound && !this->isCurrentBinaryValid) {
				wxLogDebug(_T("OnPressExecutableChoiceRefresh: found selected FSO executable %s in list after previously unable to do so"),
					binaryName.c_str());
				TCManager::GenerateTCBinaryChanged();				
			}
		}
	}
}

void BasicSettingsPage::OnSelectFredExecutable(wxCommandEvent &WXUNUSED(event)) {
	ExeChoice* choice = dynamic_cast<ExeChoice*>(
		wxWindow::FindWindowById(ID_EXE_FRED_CHOICE_BOX, this));
	wxCHECK_RET( choice != NULL, 
		_T("OnSelectExecutable: cannot find FRED choice drop down box"));

	FSOExecutable* ver = dynamic_cast<FSOExecutable*>(
		choice->GetClientObject(choice->GetSelection()));
	wxCHECK_RET( ver != NULL,
		_T("OnSelectExecutable: choice does not have FSOVersion data"));
	wxLogDebug(_T("Have selected ver for %s"), ver->GetExecutableName().c_str());

	ProMan::GetProfileManager()->ProfileWrite(PRO_CFG_TC_CURRENT_FRED, ver->GetExecutableName());
	TCManager::GenerateTCFredBinaryChanged();
}

void BasicSettingsPage::OnPressFredExecutableChoiceRefreshButton(wxCommandEvent &WXUNUSED(event)) {
	bool fredEnabled;
	ProMan::GetProfileManager()->GlobalRead(GBL_CFG_OPT_CONFIG_FRED, &fredEnabled, false);
	
	wxCHECK_RET(fredEnabled,
		_T("OnPressFredExecutableChoiceRefreshButton called when fredEnabled is false"));

	ExeChoice* fredChoice = dynamic_cast<ExeChoice*>(
		wxWindow::FindWindowById(ID_EXE_FRED_CHOICE_BOX, this));
	wxCHECK_RET( fredChoice != NULL, 
		_T("Cannot find FRED executable choice control"));

	wxString tcPath, fredBinaryName;
	wxCHECK_RET(ProMan::GetProfileManager()->ProfileRead(PRO_CFG_TC_ROOT_FOLDER, &tcPath),
		_T("OnPressFredExecutableChoiceRefreshButton: FS2/TC root folder entry not found"));
	
	if (!wxFileName::DirExists(tcPath)) {
		// warning not needed, since it will be issued by DisableExecutableChoiceControls
		wxLogDebug(_T("OnPressExecutableChoiceRefreshButton: root folder '%s' no longer exists"),
			tcPath.c_str());
		TCManager::GenerateTCChanged();
		return;
	}
	
	fredChoice->Clear();

	this->FillFredExecutableDropBox(fredChoice, wxFileName(tcPath, wxEmptyString));

	// set selection to profile entry for current FRED binary if there is one,
	// noting if selected FRED binary can't be found and could be found before or vice versa
	if (ProMan::GetProfileManager()->ProfileRead(PRO_CFG_TC_CURRENT_FRED, &fredBinaryName)) {
		bool fredExeFound = fredChoice->FindAndSetSelectionWithClientData(fredBinaryName);
		if (!fredExeFound && this-isCurrentFredBinaryValid) {
			wxLogDebug(_T("OnPressFredExecutableChoiceRefresh: couldn't find selected FRED exec %s in list of executables"),
				fredBinaryName.c_str());
			TCManager::GenerateTCFredBinaryChanged();
		} else if (fredExeFound && !this->isCurrentFredBinaryValid) {
			wxLogDebug(
				_T("OnPressFredExecutableChoiceRefresh: found selected FRED exec %s in list after previously unable to do so"),
				fredBinaryName.c_str());
			TCManager::GenerateTCFredBinaryChanged();
		}
	}
}

/** Disables the executable choice and refresh button controls, such as would occur
 if the currently loaded TC root folder doesn't exist or has no FSO executables. */
void BasicSettingsPage::DisableExecutableChoiceControls(const ReasonForExecutableDisabling reason) {

	wxString tcFolderPath;
	bool hasTcPath = ProMan::GetProfileManager()->ProfileRead(PRO_CFG_TC_ROOT_FOLDER, &tcFolderPath);
	wxCHECK_RET(hasTcPath || (reason == MISSING_TC_ROOT_FOLDER),
		_T("DisableChoiceExecutableControls: profile has no root folder entry, but reason is not a missing root folder"));
	wxCHECK_RET((!hasTcPath) || (reason != MISSING_TC_ROOT_FOLDER),
		_T("DisableChoiceExecutableControls: reason is a missing root folder, but profile has a root folder entry"));
	
	switch (reason) {
		case MISSING_TC_ROOT_FOLDER:
			// no user-facing message needed, since this is not an error state
			wxLogDebug(_T("disabling executable controls, since root folder entry is missing"));
			break;
		case NONEXISTENT_TC_ROOT_FOLDER:
			wxLogWarning(_("Selected root folder does not exist."));
			break;
		case INVALID_TC_ROOT_FOLDER:
			wxLogWarning(_("Selected root folder has no FS2 Open executables"));
			break;
		default:
			wxCHECK_RET(false, _T("DisableExecutableChoiceControls called with invalid reason"));
			break;
	}
	
	ExeChoice *exeChoice = dynamic_cast<ExeChoice*>(
		wxWindow::FindWindowById(ID_EXE_CHOICE_BOX, this));
	wxCHECK_RET( exeChoice != NULL, 
		_T("Cannot find executable choice control"));
	
	wxButton* exeChoiceRefreshButton = dynamic_cast<wxButton*>(
		wxWindow::FindWindowById(ID_EXE_CHOICE_REFRESH_BUTTON, this));
	wxCHECK_RET( exeChoiceRefreshButton != NULL,
		_T("Cannot find executable choice refresh button"));
	
	exeChoice->Clear();
	exeChoice->Disable();
	if (reason == NONEXISTENT_TC_ROOT_FOLDER || reason == MISSING_TC_ROOT_FOLDER) {
		exeChoiceRefreshButton->Disable();		
	}
	
	bool fredEnabled;
	ProMan::GetProfileManager()->GlobalRead(GBL_CFG_OPT_CONFIG_FRED, &fredEnabled, false);
	
	if (fredEnabled) {
		ExeChoice* fredChoice = dynamic_cast<ExeChoice*>(
			wxWindow::FindWindowById(ID_EXE_FRED_CHOICE_BOX, this));
		wxCHECK_RET( fredChoice != NULL, 
			_T("Cannot find FRED executable choice control"));
		
		wxButton* fredChoiceRefreshButton = dynamic_cast<wxButton*>(
			wxWindow::FindWindowById(ID_EXE_FRED_CHOICE_REFRESH_BUTTON, this));
		wxCHECK_RET( fredChoiceRefreshButton != NULL,
			_T("Cannot find FRED executable choice refresh button"));
		
		fredChoice->Clear();
		fredChoice->Disable();
		fredChoiceRefreshButton->Disable();
	}
}

/** Updates the status of whether the currently selected FSO binary is valid. */
void BasicSettingsPage::OnCurrentBinaryChanged(wxCommandEvent& event) {
	wxString tcPath, binaryName;

	if (!ProMan::GetProfileManager()->ProfileRead(PRO_CFG_TC_ROOT_FOLDER, &tcPath)) {
		this->isCurrentBinaryValid = false;
		this->ShowSettings(this->isCurrentBinaryValid);
		return;
	}
	
	if ((!FSOExecutable::IsRootFolderValid(wxFileName(tcPath, wxEmptyString), true)) && this->isTcRootFolderValid) {
		this->isCurrentBinaryValid = false;
		this->ShowSettings(this->isCurrentBinaryValid);
		TCManager::GenerateTCChanged();
		return;
	}
	
	bool hasBinary = ProMan::GetProfileManager()->ProfileRead(PRO_CFG_TC_CURRENT_BINARY, &binaryName);
	this->isCurrentBinaryValid =
		this->isTcRootFolderValid && hasBinary && wxFileName::FileExists(tcPath + wxFileName::GetPathSeparator() + binaryName);

	if (hasBinary) {
		wxLogDebug(_T("The current profile's listed FSO executable '%s' is %s."),
			binaryName.c_str(),
			this->isCurrentBinaryValid ? _T("valid") : _T("invalid"));	
	} else {
		wxLogDebug(_T("The current profile has no FSO executable listed."));
	}
	
	this->ShowSettings(this->isCurrentBinaryValid);
}

/** Updates the status of whether the currently selected FRED binary is valid. */
void BasicSettingsPage::OnCurrentFredBinaryChanged(wxCommandEvent& event) {
	bool fredEnabled;
	ProMan::GetProfileManager()->GlobalRead(GBL_CFG_OPT_CONFIG_FRED, &fredEnabled, false);
	wxCHECK_RET(fredEnabled, _T("OnCurrentFredBinaryChanged called while fredEnabled is false"));

	wxString tcPath, fredBinaryName;

	if (!ProMan::GetProfileManager()->ProfileRead(PRO_CFG_TC_ROOT_FOLDER, &tcPath)) {
		this->isCurrentFredBinaryValid = false;
		return;
	}
	
	if ((!FSOExecutable::IsRootFolderValid(wxFileName(tcPath, wxEmptyString), true)) && this->isTcRootFolderValid) {
		TCManager::GenerateTCChanged();
		return;
	}

	bool hasFredBinary = ProMan::GetProfileManager()->ProfileRead(PRO_CFG_TC_CURRENT_FRED, &fredBinaryName);
	this->isCurrentFredBinaryValid =
		this->isTcRootFolderValid && hasFredBinary && wxFileName::FileExists(tcPath + wxFileName::GetPathSeparator() + fredBinaryName);

	if (hasFredBinary) {
		wxLogDebug(_T("The current profile's listed FRED executable '%s' is %s."),
			fredBinaryName.c_str(),
			this->isCurrentFredBinaryValid ? _T("valid") : _T("invalid"));
	} else {
		wxLogDebug(_T("The current profile has no FRED executable listed."));					
	}
}

/** Hides or shows the settings. */
void BasicSettingsPage::ShowSettings(const bool showSettings) {
	if (showSettings) {
		if (!this->GetSizer()->GetItem(SETTINGS_SIZER_INDEX)->IsShown()) {
			this->GetSizer()->Show(SETTINGS_SIZER_INDEX);
			this->Layout();
		}
	} else { // using else rather than else-if to make the code easier to read (indentation-wise)
		if (this->GetSizer()->GetItem(SETTINGS_SIZER_INDEX)->IsShown()) {
			this->GetSizer()->Hide(SETTINGS_SIZER_INDEX);
			this->Layout();	
		}
	}
}

class Resolution: public wxClientData {
public:
	Resolution(const int width, const int height, const bool isHeader) {
		wxASSERT_MSG(width > 0, wxString::Format(_T("Resolution: provided width %d is invalid"), width));
		wxASSERT_MSG(height > 0, wxString::Format(_T("Resolution: provided height %d is invalid"), height));
		this->width = width;
		this->height = height;
		this->isHeader = isHeader;
		resString = isHeader ? wxString::Format(_T("--- %d:%d ---"), width, height)
			: wxString::Format(CFG_RES_FORMAT_STRING, width, height);
	}
	virtual ~Resolution() {}
	const int GetWidth() const { return this->width; }
	const int GetHeight() const { return this->height; }
	const bool IsHeader() const { return this->isHeader; }
	const wxString &GetResString() const { return this->resString; }
	bool IsSameResolution(const int otherWidth, const int otherHeight) const {
		return ((this->width == otherWidth) &&
				(this->height == otherHeight));
	}

private:
	int width;
	int height;
	bool isHeader;
	wxString resString;
};

WX_DEFINE_ARRAY_PTR(Resolution *, ResolutionArray);

// sort by aspect ratio, then by size, in descending order
int CompareResolutions(Resolution **resp1, Resolution **resp2) {

	wxASSERT_MSG(resp1 != NULL, _T("CompareResolutions: provided resp1 is NULL"));
	wxASSERT_MSG((*resp1) != NULL, _T("CompareResolutions: resp1 points to NULL"));
	wxASSERT_MSG(resp2 != NULL, _T("CompareResolutions: provided resp2 is NULL"));
	wxASSERT_MSG((*resp2) != NULL, _T("CompareResolutions: resp2 points to NULL"));
	
	// compare the two width/height ratios by cross-multiplying and comparing the results
	const int width1 = (*resp1)->GetWidth();
	const int height1 = (*resp1)->GetHeight();
	const int width2 = (*resp2)->GetWidth();
	const int height2 = (*resp2)->GetHeight();
	
	wxASSERT_MSG(width1 > 0, wxString::Format(_T("CompareResolutions: width1 %d is invalid"), width1));
	wxASSERT_MSG(height1 > 0, wxString::Format(_T("CompareResolutions: height1 %d is invalid"), height1));
	wxASSERT_MSG(width2 > 0, wxString::Format(_T("CompareResolutions: width2 %d is invalid"), width2));
	wxASSERT_MSG(height2 > 0, wxString::Format(_T("CompareResolutions: height2 %d is invalid"), height2));
	
	const int value1 = width1 * height2;
	const int value2 = width2 * height1;
	int result;
	// first compare aspect ratio
	if (value1 < value2) {
		result = -1;
	} else if (value1 > value2) {
		result = 1;
	} else { // then compare size if aspect ratios are equal
		if (width1 < width2) {
			result = -1;
		} else if (width1 > width2) {
			result = 1;
		} else {
			result = 0;
		}
	}

	return (-1) * result; // for reverse comparison (descending order)
}

// brought to you by http://en.wikipedia.org/wiki/Euclidean_algorithm#Implementations
int ComputeGCD(int a, int b) {
	wxASSERT_MSG(a > 0, wxString::Format(_T("ComputeGCD(a=%d, b=%d): a must be positive"), a, b));
	wxASSERT_MSG(b > 0, wxString::Format(_T("ComputeGCD(a=%d, b=%d): b must be positive"), a, b));

//	part of the original algorithm, but irrelevant, since a and b are positive
//	if (a == 0) {
//		return b;
//	}
	while (b != 0) {
		if (a > b) {
			a = a - b;
		} else {
			b = b - a;
		}
	}
	return a;
}

// Assumed resolutions has been sorted using CompareResolutions
void AddHeaders(ResolutionArray &resolutions) {
	wxASSERT_MSG(!resolutions.IsEmpty(), _T("AddHeaders: provided ResolutionArray is empty"));

	ResolutionArray headers;
	wxArrayInt headerIndexes;

	int lastAspectWidth = -1;
	int lastAspectHeight = -1;
	int width;
	int height;
	int gcd;
	
	// find aspect ratios and determine where the aspect ratio headers should be inserted
	for (int i = 0, n = resolutions.GetCount(); i < n; ++i) {
		width = resolutions.Item(i)->GetWidth();
		height = resolutions.Item(i)->GetHeight();
		gcd = ComputeGCD(width, height);
		width /= gcd;
		height /= gcd;
		
		// special exception: 8:5 should be 16:10
		if ((width == 8) && (height == 5)) {
			width *= 2;
			height *= 2;
		}
		
		if ((width != lastAspectWidth) || (height != lastAspectHeight)) {
			wxLogDebug(_T(" found aspect ratio %d:%d"), width, height);
			headers.Add(new Resolution(width, height, true));
			headerIndexes.Add(i);
			lastAspectWidth = width;
			lastAspectHeight = height;
		}
	}
	
	// now insert the headers into resolutions
	// must insert in reverse to avoid messing up insertion indexes
	for (int i = headerIndexes.GetCount() - 1; i >= 0; --i) {
		resolutions.Insert(headers.Item(i), headerIndexes.Item(i));
	}
}

void BasicSettingsPage::FillResolutionDropBox(wxChoice *resChoice) {
	
	ResolutionArray resolutions;
	
	// first, detect supported resolutions
#ifdef WIN32
	DEVMODE deviceMode;
	DWORD modeCounter = 0;
	BOOL result;

	wxLogDebug(_T("Enumerating graphics modes"));

	do {
		memset(&deviceMode, 0, sizeof(DEVMODE));
		deviceMode.dmSize = sizeof(DEVMODE);

		result = EnumDisplaySettings(NULL, modeCounter, &deviceMode);

		if ( result == TRUE ) {
			wxLogDebug(_T(" %dx%d %d bit %d hertz (%d)"),
				deviceMode.dmPelsWidth,
				deviceMode.dmPelsHeight,
				deviceMode.dmBitsPerPel,
				deviceMode.dmDisplayFrequency,
				deviceMode.dmDisplayFlags);
			// check to see if the resolution has already been added
			// FIXME is there a reasonable way to not make populating resolutions O(n^2)?
			bool resExists = false;
			for (ResolutionArray::iterator it = resolutions.begin(), end = resolutions.end();
				it != end; ++it) {
				if ((*it)->IsSameResolution(deviceMode.dmPelsWidth, deviceMode.dmPelsHeight)) {
					resExists = true;
					break; // no need to keep looking
				}
			}

			if ( !resExists ) {
				resolutions.Add(new Resolution(deviceMode.dmPelsWidth, deviceMode.dmPelsHeight, false));
			}
		}
		modeCounter++;
	} while ( result == TRUE );
#elif HAS_SDL == 1
	wxLogDebug(_T("Enumerating graphics modes with SDL"));
	SDL_Rect** modes;
	modes = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_HWSURFACE);
	
	if ( modes == (SDL_Rect**)NULL ) {
	  wxLogWarning(_T("Unable to retrieve any video modes"));
	} else if ( modes == (SDL_Rect**)(-1) ) {
	  wxLogWarning(_T("All resolutions are available.  If you get this message please report it to the developers as they do not think this response is actually possible"));
	} else {
	  wxLogDebug(_T("Found the following video modes:"));
	  
	  for(int i = 0; modes[i]; i++) {
	    wxLogDebug(_T(" %d x %d"), modes[i]->w, modes[i]->h);
		// FIXME why is "check to see if resolution has already been added" not used here?
		resolutions.Add(new Resolution(modes[i]->w, modes[i]->h, false));
	  }
	}
#else
#error "BasicSettingsPage::FillResolutionDropBox not implemented because not on windows and SDL is not implemented"
#endif
	// then arrange the resolutions for drop down box and insert them
	resolutions.Sort(CompareResolutions);
	AddHeaders(resolutions);
	for (ResolutionArray::iterator it = resolutions.begin(), end = resolutions.end();
		 it != end; ++it) {
		resChoice->Append((*it)->GetResString(), *it);
	}
}

/** Takes a resolution drop down box, finds the maximum resolution, updates the provided
    width and height variables, and returns the index at which the maximum resolution
    was found, returning wxNOT_FOUND on error. */
int BasicSettingsPage::GetMaxSupportedResolution(const wxChoice& resChoice, long& width, long& height) {
	int maxResIndex = -1;
	int maxResProduct = 0;
	Resolution* res;

	for (unsigned int i = 0; i < resChoice.GetCount(); ++i) {
		res = dynamic_cast<Resolution*>(resChoice.GetClientObject(i));
		wxCHECK_MSG(res != NULL, wxNOT_FOUND,
			wxString::Format(_T("choice does not have Resolution object at index %d"), i));
		// FIXME could be clever and only read the highest resolution in each aspect ratio group
		if (!res->IsHeader()) {
			int resProduct = res->GetWidth() * res->GetHeight();
			if (resProduct > maxResProduct) {
				maxResIndex = i;
				maxResProduct = resProduct;
			}
		}
	}
	
	wxCHECK_MSG(maxResIndex > -1, wxNOT_FOUND, _T("maximum Resolution was not found"));
	
	res = dynamic_cast<Resolution*>(resChoice.GetClientObject(maxResIndex));
	wxCHECK_MSG(res != NULL, wxNOT_FOUND, _T("Choice is missing max Resolution object"));
	width = res->GetWidth();
	height = res->GetHeight();
	
	wxLogDebug(_T("Found max resolution of %s at index %d"),
		res->GetResString().c_str(), maxResIndex);
	return maxResIndex;
}

void BasicSettingsPage::OnSelectVideoResolution(wxCommandEvent &WXUNUSED(event)) {
	wxChoice* choice = dynamic_cast<wxChoice*>(
		wxWindow::FindWindowById(ID_RESOLUTION_COMBO, this));
	wxCHECK_RET( choice != NULL, _T("Unable to find resolution combo"));

	Resolution* res = dynamic_cast<Resolution*>(
		choice->GetClientObject(choice->GetSelection()));
	wxCHECK_RET( res != NULL, _T("Choice does not have Resolution objects"));

	if (res->IsHeader()) { // then advance to first resolution in the header's list
		choice->SetSelection(choice->GetSelection() + 1);
	}
	
	res = dynamic_cast<Resolution*>(choice->GetClientObject(choice->GetSelection()));
	wxCHECK_RET( res != NULL, _T("after adjusting selection from header, Choice does not have Resolution objects"));
	
	ProMan::GetProfileManager()->ProfileWrite(
		PRO_CFG_VIDEO_RESOLUTION_WIDTH,
		static_cast<long>(res->GetWidth()));
	ProMan::GetProfileManager()->ProfileWrite(
		PRO_CFG_VIDEO_RESOLUTION_HEIGHT,
		static_cast<long>(res->GetHeight()));
}

void BasicSettingsPage::OnSelectVideoDepth(wxCommandEvent &WXUNUSED(event)) {
	wxChoice* depth = dynamic_cast<wxChoice*>(
		wxWindow::FindWindowById(ID_DEPTH_COMBO, this));
	wxCHECK_RET( depth != NULL, _T("Unable to find depth choice box"));

	ProMan::GetProfileManager()->ProfileWrite(
		PRO_CFG_VIDEO_BIT_DEPTH,
		(depth->GetSelection() == 0) ? static_cast<long>(16) : static_cast<long>(32));
}

void BasicSettingsPage::OnSelectVideoTextureFilter(wxCommandEvent &WXUNUSED(event)) {
	wxChoice* tex = dynamic_cast<wxChoice*>(
		wxWindow::FindWindowById(ID_TEXTURE_FILTER_COMBO, this));
	wxCHECK_RET( tex != NULL, _T("Unable to find texture filter choice"));

	ProMan::GetProfileManager()->ProfileWrite(
		PRO_CFG_VIDEO_TEXTURE_FILTER,
		(tex->GetSelection() == 0) ? _T("Bilinear") : _T("Trilinear"));
}

void BasicSettingsPage::OnSelectVideoAnisotropic(wxCommandEvent &WXUNUSED(event)) {
	wxChoice* as = dynamic_cast<wxChoice*>(
		wxWindow::FindWindowById(ID_ANISOTROPIC_COMBO, this));
	wxCHECK_RET( as != NULL, _T("Unable to find anisotropic choice"));

	ProMan::GetProfileManager()->ProfileWrite(
		PRO_CFG_VIDEO_ANISOTROPIC,
		(as->GetSelection() == 0) ? static_cast<long>(0) : static_cast<long>(1 << (as->GetSelection()-1)));
}

void BasicSettingsPage::OnSelectVideoAntiAlias(wxCommandEvent &WXUNUSED(event)) {
	wxChoice* aa = dynamic_cast<wxChoice*>(
		wxWindow::FindWindowById(ID_AA_COMBO, this));
	wxCHECK_RET( aa != NULL, _T("Unable to find anti-alias choice"));

	ProMan::GetProfileManager()->ProfileWrite(
		PRO_CFG_VIDEO_ANTI_ALIAS,
		(aa->GetSelection() == 0) ? static_cast<long>(0) : static_cast<long>(1 << (aa->GetSelection())));

}

void BasicSettingsPage::OnSelectSpeechVoice(wxCommandEvent &WXUNUSED(event)) {
	wxCHECK_RET( SpeechMan::WasBuiltIn(), _T("Speech was not compiled in."));
	wxChoice* voice = dynamic_cast<wxChoice*>(
		wxWindow::FindWindowById(ID_SPEECH_VOICE_COMBO, this));
	wxCHECK_RET( voice != NULL, _T("Unable to find voice choice box"));

	int v = voice->GetSelection();
	
	SpeechMan::SetVoice(v);

	ProMan::GetProfileManager()->ProfileWrite(PRO_CFG_SPEECH_VOICE, static_cast<long>(v));
}

void BasicSettingsPage::OnChangeSpeechVolume(wxCommandEvent &WXUNUSED(event)) {
	wxCHECK_RET( SpeechMan::WasBuiltIn(), _T("Speech was not compiled in."));
	wxSlider* volume = dynamic_cast<wxSlider*>(
		wxWindow::FindWindowById(ID_SPEECH_VOICE_VOLUME, this));
	wxCHECK_RET( volume != NULL, _T("Unable to find speech volume slider"));

	int v = volume->GetValue();

	SpeechMan::SetVolume(v);

	ProMan::GetProfileManager()->ProfileWrite(PRO_CFG_SPEECH_VOLUME, static_cast<long>(v));
}

void BasicSettingsPage::OnPlaySpeechText(wxCommandEvent &WXUNUSED(event)) {
	wxCHECK_RET( SpeechMan::WasBuiltIn(), _T("Speech was not compiled in."));
	wxTextCtrl *text = dynamic_cast<wxTextCtrl*>(
		wxWindow::FindWindowById(ID_SPEECH_TEST_TEXT, this));
	wxCHECK_RET( text != NULL, _T("Unable to find text control to get play text"));

	wxString str = text->GetValue();

	SpeechMan::Speak(str);
}

void BasicSettingsPage::OnToggleSpeechInTechroom(wxCommandEvent &event) {
	wxCHECK_RET( SpeechMan::WasBuiltIn(), _T("Speech was not compiled in."));
	bool checked = event.IsChecked();

	ProMan::GetProfileManager()->ProfileWrite(PRO_CFG_SPEECH_IN_TECHROOM, checked);
}

void BasicSettingsPage::OnToggleSpeechInBriefing(wxCommandEvent &event) {
	wxCHECK_RET( SpeechMan::WasBuiltIn(), _T("Speech was not compiled in."));
	bool checked = event.IsChecked();

	ProMan::GetProfileManager()->ProfileWrite(PRO_CFG_SPEECH_IN_BRIEFINGS, checked);
}

void BasicSettingsPage::OnToggleSpeechInGame(wxCommandEvent &event) {
	wxCHECK_RET( SpeechMan::WasBuiltIn(), _T("Speech was not compiled in."));
	bool checked = event.IsChecked();

	ProMan::GetProfileManager()->ProfileWrite(PRO_CFG_SPEECH_IN_GAME, checked);
}

void BasicSettingsPage::OnToggleSpeechInMulti(wxCommandEvent &event) {
	wxCHECK_RET( SpeechMan::WasBuiltIn(), _T("Speech was not compiled in."));
	bool checked = event.IsChecked();

	ProMan::GetProfileManager()->ProfileWrite(PRO_CFG_SPEECH_IN_MULTI, checked);
}

void BasicSettingsPage::OnGetMoreVoices(wxCommandEvent &WXUNUSED(event)) {
	HelpManager::OpenHelpById(ID_SPEECH_MORE_VOICES_BUTTON);
}

void BasicSettingsPage::OnChangeIP(wxCommandEvent &event) {
	wxTextCtrl* ip = dynamic_cast<wxTextCtrl*>(
		wxWindow::FindWindowById(event.GetId(), this));
	wxCHECK_RET(ip != NULL, _T("Unable to find IP Text Control"));

	wxString string(ip->GetValue());

	ProMan::GetProfileManager()->ProfileWrite(PRO_CFG_NETWORK_IP, string);
}

void BasicSettingsPage::OnChangePort(wxCommandEvent &event) {
	wxTextCtrl* port = dynamic_cast<wxTextCtrl*>(
		wxWindow::FindWindowById(event.GetId(), this));
	wxCHECK_RET(port != NULL, _T("Unable to find Port Text Control"));

	if (port->IsEmpty()) {
		wxLogDebug(_T("Port field is blank, writing 0 to profile"));
		ProMan::GetProfileManager()->ProfileWrite(PRO_CFG_NETWORK_PORT, static_cast<long>(0));
		return;
	}
	
	long portNumber;
	if ( port->GetValue().ToLong(&portNumber) ) {
		if ( portNumber < 0 ) {
			wxLogInfo(_T("Port number must be greater than or equal to 0"));
		} else if ( portNumber > 65535 ) {
			wxLogInfo(_T("Port number must be less than 65536"));
		} else {
			ProMan::GetProfileManager()->ProfileWrite(PRO_CFG_NETWORK_PORT, portNumber);
		}
	} else {
		wxLogWarning(_T("Port number is not a number"));
	}
}

void BasicSettingsPage::OnSelectNetworkSpeed(wxCommandEvent &event) {
	wxChoice* networkSpeed = dynamic_cast<wxChoice*>(
		wxWindow::FindWindowById(event.GetId(), this));
	wxCHECK_RET(networkSpeed != NULL, _T("Unable to find Network speed choice"));

	ProMan::GetProfileManager()->ProfileWrite(PRO_CFG_NETWORK_SPEED,
		networkSpeedOptions[networkSpeed->GetSelection()].GetRegistryValue());
}

void BasicSettingsPage::OnSelectNetworkType(wxCommandEvent &event) {
	wxChoice* networkType = dynamic_cast<wxChoice*>(
		wxWindow::FindWindowById(event.GetId(), this));
	wxCHECK_RET(networkType != NULL, _T("Unable to find Network type choice"));

	ProMan::GetProfileManager()->ProfileWrite(PRO_CFG_NETWORK_TYPE,
		networkTypeOptions[networkType->GetSelection()].GetRegistryValue());
}

void BasicSettingsPage::OnSelectSoundDevice(wxCommandEvent &event) {
	wxChoice* openaldevice = dynamic_cast<wxChoice*>(
		wxWindow::FindWindowById(event.GetId(), this));
	wxCHECK_RET(openaldevice != NULL, _T("Unable to find OpenAL Device choice"));

	ProMan::GetProfileManager()->ProfileWrite(PRO_CFG_OPENAL_DEVICE, openaldevice->GetStringSelection());
}

void BasicSettingsPage::OnSelectCaptureDevice(wxCommandEvent &event) {
	wxCHECK_RET(OpenALMan::BuildHasNewSoundCode(),
		_T("Selected FSO build doesn't have new sound code."));
	
	wxChoice* captureDevice = dynamic_cast<wxChoice*>(
		wxWindow::FindWindowById(event.GetId(), this));
	wxCHECK_RET(captureDevice != NULL, _T("Unable to find capture device choice"));
	
	ProMan::GetProfileManager()->ProfileWrite(
		PRO_CFG_OPENAL_CAPTURE_DEVICE, captureDevice->GetStringSelection());
}

void BasicSettingsPage::OnToggleEnableEFX(wxCommandEvent &event) {
	wxCHECK_RET(OpenALMan::BuildHasNewSoundCode(),
		_T("Selected FSO build doesn't have new sound code."));
	
	ProMan::GetProfileManager()->ProfileWrite(PRO_CFG_OPENAL_EFX, event.IsChecked());
}

void BasicSettingsPage::OnChangeSampleRate(wxCommandEvent &event) {
	wxCHECK_RET(OpenALMan::BuildHasNewSoundCode(),
		_T("Selected FSO build doesn't have new sound code."));
	
	wxTextCtrl* sampleRateBox = dynamic_cast<wxTextCtrl*>(
		wxWindow::FindWindowById(event.GetId(), this));
	wxCHECK_RET(sampleRateBox != NULL, _T("Unable to find sample rate text control"));
	
	if (sampleRateBox->IsEmpty()) {
		wxLogDebug(_T("Sample rate field is blank, writing 0 to profile"));
		
		ProMan::GetProfileManager()->ProfileWrite(
			PRO_CFG_OPENAL_SAMPLE_RATE, static_cast<long>(0));
		return;
	}
	
	long sampleRate;
	if ( sampleRateBox->GetValue().ToLong(&sampleRate) ) {
		if ( sampleRate <= 0 ) {
			wxLogInfo(_T("Sample rate must be greater than 0"));
		} else if ( sampleRate > 44100 ) {
			wxLogInfo(_T("Sample rate must be at most 44100"));
		} else {
			ProMan::GetProfileManager()->ProfileWrite(
				PRO_CFG_OPENAL_SAMPLE_RATE, sampleRate);
		}
	} else {
		wxLogWarning(_T("Sample rate is not a number"));
	}
}

void BasicSettingsPage::OnDownloadOpenAL(wxCommandEvent &WXUNUSED(event)) {
	this->OpenNonSCPWebSite(_T("http://connect.creativelabs.com/openal/Downloads/Forms/AllItems.aspx"));
}

void BasicSettingsPage::OnDetectOpenAL(wxCommandEvent& WXUNUSED(event)) {
	if ( !OpenALMan::IsInitialized() ) {
		this->SetupOpenALSection();
	}
}

void BasicSettingsPage::SetupOpenALSection() {
	if ( !OpenALMan::WasCompliedIn() ) {
		wxLogWarning(_T("Launcher was not compiled to support OpenAL"));
		if (this->openALVersion != NULL) {
			this->openALVersion->SetLabel(_("Launcher was not compiled to support OpenAL"));
		}
		this->soundDeviceText->Disable();
		this->soundDeviceCombo->Disable();
		this->downloadOpenALButton->Disable();
		this->detectOpenALButton->Disable();
	} else if ( !OpenALMan::Initialize() ) {
		wxLogError(_T("Unable to initialize OpenAL"));
		if (this->openALVersion != NULL) {
			this->openALVersion->SetLabel(_("Unable to initialize OpenAL"));			
		}
		this->soundDeviceText->Disable();
		this->soundDeviceCombo->Disable();
		this->detectOpenALButton->SetLabel(_("Redetect OpenAL"));
		this->downloadOpenALButton->Enable();
	} else {
		// have working openal
		this->soundDeviceCombo->Append(OpenALMan::GetAvailiableDevices());
		wxASSERT_MSG(soundDeviceCombo->GetCount() > 0, _T("sound device combo box is empty!"));
		wxString openaldevice;
		if ( ProMan::GetProfileManager()->ProfileRead(PRO_CFG_OPENAL_DEVICE, &openaldevice) ) {
			soundDeviceCombo->SetStringSelection(openaldevice);
		} else {
			wxString defaultSoundDevice(OpenALMan::SystemDefaultDevice());
			wxLogDebug(_T("Reported default sound device: %s"), defaultSoundDevice.c_str());
			if (!defaultSoundDevice.IsEmpty() &&
				(soundDeviceCombo->FindString(defaultSoundDevice) != wxNOT_FOUND)) {
				soundDeviceCombo->SetStringSelection(defaultSoundDevice);	
			} else { // just pick the first one on the list as the default
				soundDeviceCombo->SetSelection(0);
			}
		}
		// update current profile if necessary
		ProMan* proman = ProMan::GetProfileManager();
		wxString openALDevice;
		if (!proman->ProfileRead(PRO_CFG_OPENAL_DEVICE, &openALDevice) ||
			(openALDevice != this->soundDeviceCombo->GetStringSelection())) {
			wxLogDebug(_T("updating OpenAL sound device profile entry to \"%s\""),
				soundDeviceCombo->GetStringSelection().c_str());
			proman->ProfileWrite(PRO_CFG_OPENAL_DEVICE, this->soundDeviceCombo->GetStringSelection());
		}
		
		this->soundDeviceText->Enable();
		this->soundDeviceCombo->Enable();
		if (this->openALVersion != NULL) {
			this->audioNonButtonsSizer->Detach(this->openALVersion);
			this->openALVersion->Hide();
			delete this->openALVersion;
			this->openALVersion = NULL;
		}
		
		wxLogInfo(OpenALMan::GetCurrentVersion());
		
		this->downloadOpenALButton->Disable();
		this->downloadOpenALButton->Hide();
		this->detectOpenALButton->Disable();
		this->detectOpenALButton->Hide();
		
		this->audioSizer->Remove(this->audioButtonsSizer);
		this->audioSizer->Detach(this->audioNonButtonsSizer);
		this->audioSizer->Add(this->audioNonButtonsSizer,
			wxSizerFlags().Proportion(1).Expand().Border(wxLEFT|wxRIGHT|wxBOTTOM, 5));
		this->Layout();
	}
}

void BasicSettingsPage::OpenNonSCPWebSite(wxString url) {
	::wxLaunchDefaultBrowser(url);
}

/** Client data for the Joystick Choice box. Stores the joysticks
Windows ID so that we can pass it back correctly to the engine. */
class JoyNumber: public wxClientData {
public:
	JoyNumber(unsigned int i) {
		this->number = i;
	}
	int GetNumber() {
		return static_cast<int>(this->number);
	}
private:
	unsigned int number;
};

void BasicSettingsPage::SetupJoystickSection() {
	this->joystickSelected->Clear();
	if ( !JoyMan::WasCompiledIn() ) {
		this->joystickSelected->Disable();
		this->joystickSelected->Append(_("No Launcher Support"));
#if IS_WIN32
		this->joystickForceFeedback->Disable();
		this->joystickDirectionalHit->Disable();
		this->joystickCalibrateButton->Disable();
		this->joystickDetectButton->Disable();
#endif
	} else if ( !JoyMan::Initialize() ) {
		this->joystickSelected->Disable();
		this->joystickSelected->Append(_("Initialize Failed"));
#if IS_WIN32
		this->joystickForceFeedback->Disable();
		this->joystickDirectionalHit->Disable();
		this->joystickCalibrateButton->Disable();
		this->joystickDetectButton->Enable();
#endif
	} else {
		this->joystickSelected
			->Append(_("No Joystick"), new JoyNumber(JOYMAN_INVALID_JOYSTICK));
		for ( unsigned int i = 0; i < JoyMan::NumberOfJoysticks(); i++ ) {
			if ( JoyMan::IsJoystickPluggedIn(i) ) {
				this->joystickSelected
					->Append(JoyMan::JoystickName(i), new JoyNumber(i));
			}
		}

		if ( JoyMan::NumberOfPluggedInJoysticks() == 0 ) {
			this->joystickSelected->SetSelection(0);
			this->joystickSelected->Disable();
#if IS_WIN32
			this->joystickForceFeedback->Disable();
			this->joystickDirectionalHit->Disable();
			this->joystickCalibrateButton->Disable();
			this->joystickDetectButton->Enable();
#endif
		} else {
			long profileJoystick;
			unsigned int i;
			this->joystickSelected->Enable();
#if IS_WIN32
			this->joystickDetectButton->Enable();
#endif
			ProMan::GetProfileManager()->
				ProfileRead(PRO_CFG_JOYSTICK_ID,
					&profileJoystick,
					DEFAULT_JOYSTICK_ID,
					true);
			// set current joystick
			for ( i = 0; i < this->joystickSelected->GetCount(); i++ ) {
				JoyNumber* data = dynamic_cast<JoyNumber*>(
					this->joystickSelected->GetClientObject(i));
				wxCHECK2_MSG( data != NULL, continue,
					_T("JoyNumber is not the clientObject in joystickSelected"));

				if ( profileJoystick == data->GetNumber() ) {
					this->joystickSelected->SetSelection(i);
					this->SetupControlsForJoystick(i);
					return; // All joystick controls are now setup
				}
			}
			// Getting here means that the joystick is no longer installed
			// or is not plugged in
			if ( JoyMan::IsJoystickPluggedIn(profileJoystick) ) {
				wxLogWarning(_T("Last selected joystick is not plugged in"));
			} else {
				wxLogWarning(_T("Last selected joystick does not seem to be installed"));
			}
			// set to no joystick (the first selection)
			this->joystickSelected->SetSelection(0);
			this->SetupControlsForJoystick(0);
		}
	}
}

void BasicSettingsPage::SetupControlsForJoystick(unsigned int i) {
	JoyNumber* joynumber = dynamic_cast<JoyNumber*>(
		this->joystickSelected->GetClientObject(i));
	wxCHECK_RET( joynumber != NULL,
		_T("JoyNumber is not joystickSelected's client data"));

#if IS_WIN32 // calibration and force feedback don't work on OS X or Linux at the moment
	if ( JoyMan::HasCalibrateTool(joynumber->GetNumber()) ) {
		this->joystickCalibrateButton->Enable();
	} else {
		this->joystickCalibrateButton->Disable();
	}

	if ( JoyMan::SupportsForceFeedback(joynumber->GetNumber()) ) {
		bool ff, direct;
		ProMan::GetProfileManager()->ProfileRead(
			PRO_CFG_JOYSTICK_DIRECTIONAL, &direct, DEFAULT_JOYSTICK_DIRECTIONAL, true);
		ProMan::GetProfileManager()->ProfileRead(
			PRO_CFG_JOYSTICK_FORCE_FEEDBACK, &ff, DEFAULT_JOYSTICK_FORCE_FEEDBACK, true);
		this->joystickDirectionalHit->SetValue(direct);
		this->joystickForceFeedback->SetValue(ff);

		this->joystickDirectionalHit->Enable();
		this->joystickForceFeedback->Enable();
	} else {
		this->joystickDirectionalHit->Disable();
		this->joystickForceFeedback->Disable();
	}
#endif

	ProMan::GetProfileManager()->ProfileWrite(PRO_CFG_JOYSTICK_ID, static_cast<long>(joynumber->GetNumber()));
}

void BasicSettingsPage::OnSelectJoystick(
	wxCommandEvent &WXUNUSED(event)) {
	this->SetupControlsForJoystick(
		this->joystickSelected->GetSelection());
}

void BasicSettingsPage::OnCheckForceFeedback(
	wxCommandEvent &event) {
		ProMan::GetProfileManager()->ProfileWrite(PRO_CFG_JOYSTICK_FORCE_FEEDBACK, event.IsChecked());
}

void BasicSettingsPage::OnCheckDirectionalHit(
	wxCommandEvent &event) {
		ProMan::GetProfileManager()->ProfileWrite(PRO_CFG_JOYSTICK_DIRECTIONAL, event.IsChecked());
}

void BasicSettingsPage::OnCalibrateJoystick(
	wxCommandEvent &WXUNUSED(event)) {
		JoyNumber *data = dynamic_cast<JoyNumber*>(
			this->joystickSelected->GetClientObject(
			this->joystickSelected->GetSelection()));
		wxCHECK_RET( data != NULL,
			_T("joystickSelected does not have JoyNumber as clientdata"));

		JoyMan::LaunchCalibrateTool(data->GetNumber());
}

void BasicSettingsPage::OnDetectJoystick(wxCommandEvent &WXUNUSED(event)) {
		if ( JoyMan::DeInitialize() ) {
			this->SetupJoystickSection();
		}
}

//////////// ProxyChoice
ProxyChoice::ProxyChoice(wxWindow *parent, wxWindowID id)
:wxChoicebook(parent, id) {
	wxString type; // NOTE: not adding writeBackIfAbsent to GlobalRead calls, since proxy box not in GUI
	ProMan::GetProfileManager()->GlobalRead(GBL_CFG_PROXY_TYPE, &type, _T("none"));

	wxPanel* noneProxyPanel = new wxPanel(this);
	this->AddPage(noneProxyPanel, _("None"));

	/// Manual Proxy
	wxPanel* manualProxyPanel = new wxPanel(this);
	wxStaticText* proxyHttpText = new wxStaticText(manualProxyPanel, wxID_ANY, _("HTTP Proxy:"));
	wxString server;
	ProMan::GetProfileManager()->GlobalRead(GBL_CFG_PROXY_SERVER, &server, _T(""));
	wxTextCtrl* proxyHttpServer = new wxTextCtrl(manualProxyPanel, ID_PROXY_HTTP_SERVER, server);
	wxStaticText* proxyHttpPortText = new wxStaticText(manualProxyPanel, wxID_ANY, _("Port:"));

	long port;
	ProMan::GetProfileManager()->GlobalRead(GBL_CFG_PROXY_PORT, &port, 0);
	wxTextCtrl* proxyHttpPort = new wxTextCtrl(manualProxyPanel, ID_PROXY_HTTP_PORT, wxString::Format(_T("%ld"), port));

	wxBoxSizer* manualProxyPortSizer = new wxBoxSizer(wxHORIZONTAL);
	manualProxyPortSizer->Add(proxyHttpPortText, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);
	manualProxyPortSizer->Add(proxyHttpPort, wxSizerFlags().Proportion(1));

	wxBoxSizer* manualProxySizer = new wxBoxSizer(wxVERTICAL);
	manualProxySizer->Add(proxyHttpText);
	manualProxySizer->Add(proxyHttpServer, wxSizerFlags().Expand());
	manualProxySizer->Add(manualProxyPortSizer, wxSizerFlags().Expand());
	manualProxySizer->SetContainingWindow(this);
	manualProxyPanel->SetSizer(manualProxySizer);

	this->AddPage(manualProxyPanel, _T("Manual"));

	if ( type == _T("manual") ) {
		this->SetSelection(1);
	} else {
		this->SetSelection(0);
	}
}

ProxyChoice::~ProxyChoice() {
}

BEGIN_EVENT_TABLE(ProxyChoice, wxChoicebook)
EVT_TEXT(ID_PROXY_HTTP_SERVER, ProxyChoice::OnChangeServer)
EVT_TEXT(ID_PROXY_HTTP_PORT, ProxyChoice::OnChangePort)
EVT_CHOICEBOOK_PAGE_CHANGED(ID_PROXY_TYPE, ProxyChoice::OnProxyTypeChange)
END_EVENT_TABLE()

void ProxyChoice::OnChangeServer(wxCommandEvent &event) {
	wxString str = event.GetString();

	if ( str == wxEmptyString ) {
		// do nothing
	} else {
		ProMan::GetProfileManager()->GlobalWrite(GBL_CFG_PROXY_SERVER, str);
	}
}

void ProxyChoice::OnChangePort(wxCommandEvent &event) {
	int port = event.GetInt();

	ProMan::GetProfileManager()->GlobalWrite(GBL_CFG_PROXY_PORT, static_cast<long>(port));
}

void ProxyChoice::OnProxyTypeChange(wxChoicebookEvent &event) {
	int page = event.GetSelection();
	wxString str;

	switch (page) {
		case 0:
			str = _T("none");
			break;
		case 1:
			str = _T("manual");
			break;
		default:
			wxFAIL_MSG(wxString::Format(_T("Proxy type changed to invalid id %d"), page));
			return;
	}

	ProMan::GetProfileManager()->GlobalWrite(GBL_CFG_PROXY_TYPE, str);
}
