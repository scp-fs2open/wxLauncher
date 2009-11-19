#ifndef BASICSETTINGSPAGE_H
#define BASICSETTINGSPAGE_H

#include <wx/wx.h>

class BasicSettingsPage : public wxPanel {
public:
	BasicSettingsPage(wxWindow* parent);
	~BasicSettingsPage();

	void OnSelectTC(wxCommandEvent &event);
	void OnTCChanged(wxCommandEvent &event);

	void OnSelectExecutable(wxCommandEvent &event);
	void OnSelectVideoResolution(wxCommandEvent &event);
	void OnSelectGraphicsAPI(wxCommandEvent &event);
	void OnSelectVideoDepth(wxCommandEvent &event);
	void OnSelectVideoAnistropic(wxCommandEvent &event);
	void OnSelectVideoAntiAlias(wxCommandEvent &event);
	void OnSelectVideoTextureFilter(wxCommandEvent &event);
	void OnSelectVideoGeneralSettings(wxCommandEvent &event);
	void OnToggleVideoUseLargeTexture(wxCommandEvent &event);
	void OnToggleVideoFixFontDistortion(wxCommandEvent &event);

	void OnSelectSpeechVoice(wxCommandEvent &event);
	void OnChangeSpeechVolume(wxCommandEvent &event);
	void OnPlaySpeechText(wxCommandEvent &event);
	void OnToggleSpeechInTechroom(wxCommandEvent &event);
	void OnToggleSpeechInBriefing(wxCommandEvent &event);
	void OnToggleSpeechInGame(wxCommandEvent &event);
	void OnGetMoreVoices(wxCommandEvent &event);

	void OnSelectNetworkType(wxCommandEvent &event);
	void OnSelectNetworkSpeed(wxCommandEvent &event);
	void OnChangePort(wxCommandEvent &event);
	void OnChangeIP(wxCommandEvent &event);

	void OnSelectOpenALDevice(wxCommandEvent &event);
	void OnDownloadOpenAL(wxCommandEvent& event);
	void OnDetectOpenAL(wxCommandEvent& event);

private:
	void FillExecutableDropBox(wxChoice* exeChoice, wxFileName path);
	void FillResolutionDropBox(wxChoice* exeChoice);
	void OpenNonSCPWebSite(wxString url);
	void SetupOpenALSection();
	DECLARE_EVENT_TABLE();

private:
	wxStaticText* soundDeviceText;
	wxChoice* soundDeviceCombo;
	wxStaticText* openALVersion;
	wxButton* downloadOpenALButton;
	wxButton* detectOpenALButton;
};

#endif