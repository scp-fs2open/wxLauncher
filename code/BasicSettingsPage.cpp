#include <wx/wx.h>
#include "BasicSettingsPage.h"
#include "wxIDS.h"

#include "wxLauncherSetup.h" // Last include for memory debugging

BasicSettingsPage::BasicSettingsPage(wxWindow* parent): wxPanel(parent, wxID_ANY) {
	// exe Selection
	wxStaticBox* exeBox = new wxStaticBox(this, wxID_ANY, _("Executable Selection"));

	wxStaticText* rootFolderText = new wxStaticText(this, wxID_ANY, _("FS2 Root Folder:"));
	wxTextCtrl* rootFolderBox = new wxTextCtrl(this, ID_EXE_ROOT_FOLDER_BOX, _T(""));
	wxButton* selectButton = new wxButton(this, ID_EXE_SELECT_ROOT_BUTTON, _T("Select"));

	wxStaticText* useExeText = new wxStaticText(this, wxID_ANY, _("Use this FS2_Open binary: "));
	wxChoice* useExeChoice = new wxChoice(this, ID_EXE_CHOICE_BOX);
	useExeChoice->Disable();

	wxBoxSizer* rootFolderSizer = new wxBoxSizer(wxHORIZONTAL);
	rootFolderSizer->Add(rootFolderText);
	rootFolderSizer->Add(rootFolderBox);
	rootFolderSizer->Add(selectButton);

	wxBoxSizer* selectExeSizer = new wxBoxSizer(wxHORIZONTAL);
	selectExeSizer->Add(useExeText);
	selectExeSizer->Add(useExeChoice);

	wxStaticBoxSizer* exeSizer = new wxStaticBoxSizer(exeBox, wxVERTICAL);
	exeSizer->Add(rootFolderSizer);
	exeSizer->Add(selectExeSizer);

	// Video Section
	wxStaticBox* videoBox = new wxStaticBox(this, wxID_ANY, _("Video"));

	wxStaticText* graphicsText = new wxStaticText(this, wxID_ANY, _("Graphics:"));
	wxComboBox* graphicsCombo = new wxComboBox(this, ID_GRAPHICS_COMBO, _("OpenGL"));

	wxStaticText* resolutionText = new wxStaticText(this, wxID_ANY, _("Resolution:"));
	wxComboBox* resolutionCombo = new wxComboBox(this, ID_RESOLUTION_COMBO, _("1024x768"));

	wxStaticText* depthText = new wxStaticText(this, wxID_ANY, _("Depth:"));
	wxComboBox* depthCombo = new wxComboBox(this, ID_DEPTH_COMBO, _("32 Bit"));

	wxStaticText* textureFilterText = new wxStaticText(this, wxID_ANY, _("Texture Filter:"));
	wxComboBox* textureFilterCombo = new wxComboBox(this, ID_TEXTURE_FILTER_COMBO, _("Trilinear"));

	wxStaticText* anisotropicText = new wxStaticText(this, wxID_ANY, _("Anisotropic:"));
	wxComboBox* anisotropicCombo = new wxComboBox(this, ID_ANISOTROPIC_COMBO, _("Off"));

	wxStaticText* aaText = new wxStaticText(this, wxID_ANY, _("Anti-Alias:"));
	wxComboBox* aaCombo = new wxComboBox(this, ID_AA_COMBO, _("Off"));

	wxStaticText* gsText = new wxStaticText(this, wxID_ANY, _("General settings (recommend: highest):"));
	wxComboBox* gsCombo = new wxComboBox(this, ID_GS_COMBO, _("Highest"));

	wxCheckBox* largeTextureCheck = new wxCheckBox(this, ID_LARGE_TEXTURE_CHECK, _("Use large textures:"));
	wxCheckBox* fontDistortion = new wxCheckBox(this, ID_FONT_DISTORTION_CHECK, _("Fix font distortion"));

	wxGridSizer* videoSizer1 = new wxFlexGridSizer(4); // Sizer for graphics, resolution, depth, etc
	videoSizer1->Add(graphicsText);
	videoSizer1->Add(graphicsCombo);
	videoSizer1->Add(resolutionText);
	videoSizer1->Add(resolutionCombo);
	videoSizer1->Add(depthText);
	videoSizer1->Add(depthCombo);
	videoSizer1->Add(textureFilterText);
	videoSizer1->Add(textureFilterCombo);
	videoSizer1->Add(anisotropicText);
	videoSizer1->Add(anisotropicCombo);
	videoSizer1->Add(aaText);
	videoSizer1->Add(aaCombo);

	wxBoxSizer* videoSizergs = new wxBoxSizer(wxHORIZONTAL);
	videoSizergs->Add(gsText);
	videoSizergs->Add(gsCombo);

	wxBoxSizer* videoSizer3 = new wxBoxSizer(wxHORIZONTAL);
	videoSizer3->Add(largeTextureCheck);
	videoSizer3->Add(fontDistortion);

	wxStaticBoxSizer* videoSizer = new wxStaticBoxSizer(videoBox, wxVERTICAL);
	videoSizer->SetMinSize(wxSize(300, -1));
	videoSizer->Add(videoSizer1);
	videoSizer->Add(videoSizergs);
	videoSizer->Add(videoSizer3);

	// Speech
	wxStaticBox* speechBox = new wxStaticBox(this, wxID_ANY, _("Speech"));

	wxTextCtrl* speechTestText = new wxTextCtrl(this, ID_SPEECH_TEST_TEXT,
		_("Press play to test this string"),
		wxDefaultPosition, wxDefaultSize,
		wxTE_MULTILINE);
	wxComboBox* speechVoiceCombo = new wxComboBox(this, ID_SPEECH_VOICE_COMBO, _("Microsoft Anna (United States)"));
	wxSlider* speechVoiceVolume = new wxSlider(this, ID_SPEECH_VOICE_VOLUME, 50, 0, 100);
	wxButton* speechPlayButton = new wxButton(this, ID_SPEECH_PLAY_BUTTON, _("Play String"));

	wxStaticText* speechUsingText = new wxStaticText(this, wxID_ANY, _("Use simulated speech:"));
	wxCheckBox* speechUsingTechroomCheck = new wxCheckBox(this, ID_SPEECH_USING_TECHROOM, _("Techroom"));
	wxCheckBox* speechUsingBriefingCheck = new wxCheckBox(this, ID_SPEECH_USING_BRIEFING, _("Briefings"));
	wxCheckBox* speechUsingIngameCheck = new wxCheckBox(this, ID_SPEECH_USING_INGAME, _("Ingame"));
	wxButton* speechMoreVoicedButton = new wxButton(this, ID_SPEECH_MORE_VOICES_BUTTON, _("Get More Voices"));

	wxBoxSizer* speechLeftSizer = new wxBoxSizer(wxVERTICAL);
	speechLeftSizer->Add(speechTestText, wxSizerFlags().Expand());
	speechLeftSizer->Add(speechVoiceCombo);
	speechLeftSizer->Add(speechVoiceVolume, wxSizerFlags().Expand());
	speechLeftSizer->Add(speechPlayButton, wxSizerFlags().Center());

	wxBoxSizer* speechRightSizer = new wxBoxSizer(wxVERTICAL);
	speechRightSizer->Add(speechUsingText);
	speechRightSizer->Add(speechUsingTechroomCheck);
	speechRightSizer->Add(speechUsingBriefingCheck);
	speechRightSizer->Add(speechUsingIngameCheck);
	speechRightSizer->Add(speechMoreVoicedButton);

	wxStaticBoxSizer* speechSizer = new wxStaticBoxSizer(speechBox, wxHORIZONTAL);
	speechSizer->Add(speechLeftSizer);
	speechSizer->Add(speechRightSizer);

	// Network
	wxStaticBox* networkBox = new wxStaticBox(this, wxID_ANY, _("Network"));

	wxComboBox* networkType = new wxComboBox(this, ID_NETWORK_TYPE, _("None"));
	wxComboBox* networkSpeed = new wxComboBox(this, ID_NETWORK_SPEED, _("None"));
	wxTextCtrl* networkPort = new wxTextCtrl(this, ID_NETWORK_PORT, _(""));
	wxTextCtrl* networkIP = new wxTextCtrl(this, ID_NETWORK_IP, _(""));

	wxGridSizer* networkInsideSizer = new wxFlexGridSizer(4);
	networkInsideSizer->Add(new wxStaticText(this, wxID_ANY, _("Connection type:")));
	networkInsideSizer->Add(networkType);
	networkInsideSizer->Add(new wxStaticText(this, wxID_ANY, _("Port:")));
	networkInsideSizer->Add(networkPort);
	networkInsideSizer->Add(new wxStaticText(this, wxID_ANY, _("Connection speed:")));
	networkInsideSizer->Add(networkSpeed);
	networkInsideSizer->Add(new wxStaticText(this, wxID_ANY, _("IP:")));
	networkInsideSizer->Add(networkIP);

	wxStaticBoxSizer* networkSizer = new wxStaticBoxSizer(networkBox, wxVERTICAL);
	networkSizer->Add(networkInsideSizer);

	// Audio
	wxStaticBox* audioBox = new wxStaticBox(this, wxID_ANY, _("Audio"));

	wxStaticText* soundDeviceText = new wxStaticText(this, wxID_ANY, _("Sound device:"));
	wxComboBox* soundDeviceCombo = new wxComboBox(this, ID_SELECT_SOUND_DEVICE, _("Generic device"));

	wxStaticText* openALVersion = new wxStaticText(this, wxID_ANY, _("OpenAL version v1.2.34"));
	openALVersion->Wrap(153); /* HACKHACK: hard coded width, using number of
							  pixels wide the text is on the prototype.*/
	wxButton* downloadOpenALButton = new wxButton(this, ID_DOWNLOAD_OPENAL, _("Download OpenAL"));
	wxButton* detectOpenALButton = new wxButton(this, ID_DETECT_OPENAL, _("Detect"));

	wxStaticBoxSizer* audioSizer = new wxStaticBoxSizer(audioBox, wxVERTICAL);
	audioSizer->Add(soundDeviceText);
	audioSizer->Add(soundDeviceCombo, wxSizerFlags().Expand());
	audioSizer->Add(openALVersion, wxSizerFlags().Center());
	audioSizer->Add(downloadOpenALButton, wxSizerFlags().Center());
	audioSizer->Add(detectOpenALButton, wxSizerFlags().Center());

	// Joystick
	wxStaticBox* joystickBox = new wxStaticBox(this, wxID_ANY, _("Joystick"));

	wxStaticText* selectedJoystickText = new wxStaticText(this, wxID_ANY, _("Selected joystick:"));
	wxComboBox* joystickSelected = new wxComboBox(this, ID_JOY_SELECTED, _("No joystick"));
	wxCheckBox* joystickForceFeedback = new wxCheckBox(this, ID_JOY_FORCE_FEEDBACK, _("Force feedback"));
	wxCheckBox* joystickDirectionalHit = new wxCheckBox(this, ID_JOY_DIRECTIONAL_HIT, _("Directional hit"));
	wxButton* joystickCalibrateButton = new wxButton(this, ID_JOY_CALIBRATE_BUTTON, _("Calibrate"));
	wxButton* joystickDetectButton = new wxButton(this, ID_JOY_DETECT_BUTTON, _("Detect"));

	wxBoxSizer* joyButtonSizer = new wxBoxSizer(wxHORIZONTAL);
	joyButtonSizer->Add(joystickCalibrateButton);
	joyButtonSizer->Add(joystickDetectButton);

	wxStaticBoxSizer* joystickSizer = new wxStaticBoxSizer(joystickBox, wxVERTICAL);
	joystickSizer->Add(selectedJoystickText);
	joystickSizer->Add(joystickSelected, wxSizerFlags().Expand());
	joystickSizer->Add(joystickForceFeedback);
	joystickSizer->Add(joystickDirectionalHit);
	joystickSizer->Add(joyButtonSizer);

	// Proxy
	wxStaticBox* proxyBox = new wxStaticBox(this, wxID_ANY, _("Proxy"));

	wxStaticText* proxyTypeText = new wxStaticText(this, wxID_ANY, _("Type:"));
	wxComboBox* proxyType = new wxComboBox(this, ID_PROXY_TYPE, _("None"));

	wxStaticText* proxyHttpText = new wxStaticText(this, wxID_ANY, _("HTTP Proxy:"));
	wxTextCtrl* proxyHttpServer = new wxTextCtrl(this, ID_PROXY_HTTP_SERVER, _(""));
	wxStaticText* proxyHttpPortText = new wxStaticText(this, wxID_ANY, _("Port:"));
	wxTextCtrl* proxyHttpPort = new wxTextCtrl(this, ID_PROXY_HTTP_PORT, _(""));

	wxStaticText* proxyAutoText = new wxStaticText(this, wxID_ANY, _("Auto Proxy Config URL:"));
	wxTextCtrl* proxyAuto = new wxTextCtrl(this, ID_PROXY_AUTO_URL, _(""));

	wxBoxSizer* proxyTypeSizer = new wxBoxSizer(wxHORIZONTAL);
	proxyTypeSizer->Add(proxyTypeText);
	proxyTypeSizer->Add(proxyType);

	wxBoxSizer* proxyPortSizer = new wxBoxSizer(wxHORIZONTAL);
	proxyPortSizer->Add(proxyHttpPortText);
	proxyPortSizer->Add(proxyHttpPort);

	wxStaticBoxSizer* proxySizer = new wxStaticBoxSizer(proxyBox, wxVERTICAL);
	proxySizer->Add(proxyTypeSizer);
	proxySizer->Add(proxyHttpText);
	proxySizer->Add(proxyHttpServer);
	proxySizer->Add(proxyPortSizer);
	proxySizer->Add(proxyAutoText);
	proxySizer->Add(proxyAuto, wxSizerFlags().Expand());

	// Final Layout
	wxBoxSizer* leftColumnSizer = new wxBoxSizer(wxVERTICAL);
	leftColumnSizer->Add(exeSizer, wxSizerFlags().Expand());
	leftColumnSizer->Add(videoSizer, wxSizerFlags().Expand());
	leftColumnSizer->Add(speechSizer, wxSizerFlags().Expand());
	leftColumnSizer->Add(networkSizer, wxSizerFlags().Expand());

	wxBoxSizer* rightColumnSizer = new wxBoxSizer(wxVERTICAL);
	rightColumnSizer->Add(audioSizer, wxSizerFlags().Expand());
	rightColumnSizer->Add(joystickSizer, wxSizerFlags().Expand());
	rightColumnSizer->Add(proxySizer, wxSizerFlags().Expand());

	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(leftColumnSizer);
	sizer->Add(rightColumnSizer);

	this->SetSizer(sizer);
	this->Layout();

}