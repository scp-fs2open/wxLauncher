#include <wx/wx.h>
#include <wx/filename.h>

#include "BasicSettingsPage.h"
#include "wxIDS.h"
#include "ProfileManager.h"
#include "TCManager.h"

#include "wxLauncherSetup.h" // Last include for memory debugging

class ExeChoice: public wxChoice {
public:
	ExeChoice(wxWindow * parent, wxWindowID id) : wxChoice(parent, id) {}
	void FindAndSetSelectionWithClientData(wxString item) {
		size_t number = this->GetStrings().size();
		for( size_t i = 0; i < number; i++ ) {
			FSOVersion* data = dynamic_cast<FSOVersion*>(this->GetClientObject(i));
			wxCHECK2_MSG( data != NULL, continue, _T("Client data is not a FSOVersion pointer"));
			if ( data->executablename == item ) {
				this->SetSelection(i);
				return;
			}
		}
	}
};
		

BasicSettingsPage::BasicSettingsPage(wxWindow* parent): wxPanel(parent, wxID_ANY) {
	ProMan* proman = ProMan::GetProfileManager();
	TCManager::Initialize();
	// exe Selection
	wxString tcfolder, binary;
	bool hastcfolder = proman->Get()->Read(PRO_CFG_TC_ROOT_FOLDER, &tcfolder, _T(""));
	bool hasbinary = proman->Get()->Read(PRO_CFG_TC_CURRENT_BINARY, &binary, _T(""));
	
	wxStaticBox* exeBox = new wxStaticBox(this, wxID_ANY, _("Executable Selection"));

	wxStaticText* rootFolderText = new wxStaticText(this, wxID_ANY, _("FS2 Root Folder:"));
	wxTextCtrl* rootFolderBox = new wxTextCtrl(this, ID_EXE_ROOT_FOLDER_BOX, tcfolder);
	wxButton* selectButton = new wxButton(this, ID_EXE_SELECT_ROOT_BUTTON, _T("Select"));

	wxStaticText* useExeText = new wxStaticText(this, wxID_ANY, _("Use this FS2_Open binary: "));
	ExeChoice* useExeChoice = new ExeChoice(this, ID_EXE_CHOICE_BOX);
	if ( hastcfolder ) {
		this->FillExecutableDropBox(useExeChoice, wxFileName(tcfolder, wxEmptyString));
		useExeChoice->FindAndSetSelectionWithClientData(binary);
	} else {
		useExeChoice->Disable();
	}
	TCManager::RegisterTCChanged(this);

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
	wxStaticBox* videoBox = new wxStaticBox(this, ID_VIDEO_STATIC_BOX, _("Video"));

	wxStaticText* graphicsText = 
		new wxStaticText(this, wxID_ANY, _("Graphics:"));
	wxChoice* graphicsCombo = new wxChoice(this, ID_GRAPHICS_COMBO);
	graphicsCombo->Insert(_T("OpenGL"), 0);
	wxString graphicsAPI;
	proman->Get()->Read(PRO_CFG_VIDEO_API, &graphicsAPI, _T("OpenGL"));
	graphicsCombo->SetStringSelection(graphicsAPI);

	wxStaticText* resolutionText = 
		new wxStaticText(this, wxID_ANY, _("Resolution:"));
	wxChoice* resolutionCombo = new wxChoice(this, ID_RESOLUTION_COMBO);
	this->FillResolutionDropBox(resolutionCombo);
	int width, height;
	proman->Get()->Read(PRO_CFG_VIDEO_RESOLUTION_WIDTH, &width, 800);
	proman->Get()->Read(PRO_CFG_VIDEO_RESOLUTION_HEIGHT, &height, 600);
	resolutionCombo->SetStringSelection(
		wxString::Format(CFG_RES_FORMAT_STRING, width, height));

	wxStaticText* depthText = 
		new wxStaticText(this, wxID_ANY, _("Depth:"));
	wxChoice* depthCombo = new wxChoice(this, ID_DEPTH_COMBO);
	int bitDepth;
	depthCombo->Append(_("16 bit"));
	depthCombo->Append(_("32 bit"));
	proman->Get()->Read(PRO_CFG_VIDEO_BIT_DEPTH, &bitDepth, 16);
	depthCombo->SetSelection((bitDepth == 16) ? 0 : 1);

	wxStaticText* textureFilterText = 
		new wxStaticText(this, wxID_ANY, _("Texture Filter:"));
	wxChoice* textureFilterCombo = new wxChoice(this, ID_TEXTURE_FILTER_COMBO);
	wxString filter;
	textureFilterCombo->Append(_("Bilinear"));
	textureFilterCombo->Append(_("Trilinear"));
	proman->Get()->Read(PRO_CFG_VIDEO_TEXTURE_FILTER, &filter, _T("bilinear"));
	filter.MakeLower();
	textureFilterCombo->SetSelection( (filter == _T("bilinear")) ? 0 : 1);

	wxStaticText* anisotropicText = 
		new wxStaticText(this, wxID_ANY, _("Anisotropic:"));
	wxChoice* anisotropicCombo = new wxChoice(this, ID_ANISOTROPIC_COMBO);
	int anisotropic;
	anisotropicCombo->Append(_("Off"));
	anisotropicCombo->Append(_T(" 1x"));
	anisotropicCombo->Append(_T(" 2x"));
	anisotropicCombo->Append(_T(" 4x"));
	anisotropicCombo->Append(_T(" 8x"));
	anisotropicCombo->Append(_T("16x"));
	proman->Get()->Read(PRO_CFG_VIDEO_ANISOTROPIC, &anisotropic, 0);
	switch(anisotropic) {
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
			anisotropic = 0;
	}
	anisotropicCombo->SetSelection(anisotropic);


	wxStaticText* aaText = new wxStaticText(this, wxID_ANY, _("Anti-Alias:"));
	wxChoice* aaCombo = new wxChoice(this, ID_AA_COMBO);
	int antialias;
	aaCombo->Append(_("Off"));
	aaCombo->Append(_T(" 1x"));
	aaCombo->Append(_T(" 2x"));
	aaCombo->Append(_T(" 4x"));
	aaCombo->Append(_T(" 8x"));
	aaCombo->Append(_T("16x"));
	proman->Get()->Read(PRO_CFG_VIDEO_ANISOTROPIC, &antialias, 0);
	switch(antialias) {
		case 1:
			antialias = 1;
			break;
		case 2:
			antialias = 2;
			break;
		case 4:
			antialias = 3;
			break;
		case 8:
			antialias = 4;
			break;
		case 16:
			antialias = 5;
			break;
		default:
			antialias = 0;
	}
	aaCombo->SetSelection(antialias);

	wxStaticText* gsText = 
		new wxStaticText(this, wxID_ANY, _("General settings (recommend: highest):"));
	wxChoice* gsCombo = new wxChoice(this, ID_GS_COMBO);
	gsCombo->Append(_("1. Lowest"));
	gsCombo->Append(_("2. Low"));
	gsCombo->Append(_("3. High"));
	gsCombo->Append(_("4. Highest"));
	int gamespeed;
	proman->Get()->Read(PRO_CFG_VIDEO_GENERAL_SETTINGS, &gamespeed, 3);
	gsCombo->SetSelection(gamespeed);

	wxCheckBox* largeTextureCheck = 
		new wxCheckBox(this, ID_LARGE_TEXTURE_CHECK, _("Use large textures"));
	bool largeTextures;
	proman->Get()->Read(PRO_CFG_VIDEO_USE_LARGE_TEXTURES, &largeTextures, false);
	largeTextureCheck->SetValue(largeTextures);
	wxCheckBox* fontDistortion = 
		new wxCheckBox(this, ID_FONT_DISTORTION_CHECK, _("Fix font distortion"));
	bool fixFont;
	proman->Get()->Read(PRO_CFG_VIDEO_USE_LARGE_TEXTURES, &fixFont, false);
	fontDistortion->SetValue(fixFont);
	
	// Sizer for graphics, resolution, depth, etc
	wxGridSizer* videoSizer1 = new wxFlexGridSizer(4); 
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

BasicSettingsPage::~BasicSettingsPage() {
	TCManager::DeInitialize();
}

/// Event Handling
BEGIN_EVENT_TABLE(BasicSettingsPage, wxPanel)
EVT_BUTTON(ID_EXE_SELECT_ROOT_BUTTON, BasicSettingsPage::OnSelectTC)
EVT_CHOICE(ID_EXE_CHOICE_BOX, BasicSettingsPage::OnSelectExecutable)
EVT_COMMAND( wxID_NONE, EVT_TC_CHANGED, BasicSettingsPage::OnTCChanged)

// Video controls
EVT_CHOICE(ID_GRAPHICS_COMBO, BasicSettingsPage::OnSelectGraphicsAPI)
EVT_CHOICE(ID_RESOLUTION_COMBO, BasicSettingsPage::OnSelectVideoResolution)
EVT_CHOICE(ID_DEPTH_COMBO, BasicSettingsPage::OnSelectVideoDepth)
EVT_CHOICE(ID_TEXTURE_FILTER_COMBO, BasicSettingsPage::OnSelectVideoTextureFilter)
EVT_CHOICE(ID_ANISOTROPIC_COMBO, BasicSettingsPage::OnSelectVideoAnistropic)
EVT_CHOICE(ID_AA_COMBO, BasicSettingsPage::OnSelectVideoAntiAlias)
EVT_CHOICE(ID_GS_COMBO, BasicSettingsPage::OnSelectVideoGeneralSettings)
EVT_CHECKBOX(ID_LARGE_TEXTURE_CHECK, BasicSettingsPage::OnToggleVideoUseLargeTexture)
EVT_CHECKBOX(ID_FONT_DISTORTION_CHECK, BasicSettingsPage::OnToggleVideoFixFontDistortion)

END_EVENT_TABLE()

void BasicSettingsPage::OnSelectTC(wxCommandEvent &event) {
	WXUNUSED(event);
	wxString directory;
	ProMan* proman = ProMan::GetProfileManager();
	proman->Get()->Read(PRO_CFG_TC_ROOT_FOLDER, &directory, wxEmptyString);
	wxDirDialog filechooser(this, _T("Please choose the base directory of the Total Conversion"),
		directory, wxDD_DEFAULT_STYLE|wxDD_DIR_MUST_EXIST);

	wxString chosenDirectory;
	wxFileName path;
	while (true) {
		if ( wxID_CANCEL == filechooser.ShowModal() ) {
			return;
		}
		chosenDirectory = filechooser.GetPath();
		if ( chosenDirectory == directory ) {
			wxLogInfo(_T("The same was not changed."));
			return; // User canceled, bail out.
		}
		path.SetPath(chosenDirectory);
		if ( !path.IsOk() ) {
			wxLogWarning(_T("Directory is not valid"));
			continue;
		} else if ( TCManager::CheckRootFolder(path) ) {
			break;
		} else {
			wxLogWarning(_T("Directory does not have supported executables in it"));
		}
	}
	wxLogDebug(_T("User choose '%s' as the TC directory"), path.GetPath());
	proman->Get()->Write(PRO_CFG_TC_ROOT_FOLDER, path.GetPath());
	TCManager::GenerateTCChanged();
}

void BasicSettingsPage::OnTCChanged(wxCommandEvent &event) {
	WXUNUSED(event);

	wxChoice *exeChoice = dynamic_cast<wxChoice*>(
		wxWindow::FindWindowById(ID_EXE_CHOICE_BOX, this));
	wxCHECK_RET( exeChoice != NULL, 
		_T("Cannot find executable choice control"));

	wxTextCtrl* tcFolder = dynamic_cast<wxTextCtrl*>(
		wxWindow::FindWindowById(ID_EXE_ROOT_FOLDER_BOX, this));
	wxCHECK_RET( tcFolder != NULL, 
		_T("Cannot find Text Control to show folder in."));

	wxString tcPath;
	exeChoice->Clear();
	ProMan::GetProfileManager()
		->Get()
			->DeleteEntry(PRO_CFG_TC_CURRENT_BINARY);

	if ( ProMan::GetProfileManager()->Get()
			->Read(PRO_CFG_TC_ROOT_FOLDER, &tcPath) ) {
		tcFolder->SetValue(tcPath);

		this->FillExecutableDropBox(exeChoice, wxFileName(tcPath, wxEmptyString));
	}
	this->GetSizer()->Layout();
	TCManager::GenerateTCBinaryChanged();
}

void BasicSettingsPage::FillExecutableDropBox(wxChoice* exeChoice, wxFileName path) {
	wxArrayString exes = TCManager::GetBinariesFromRootFolder(path);
	wxArrayString::iterator iter = exes.begin();
	while ( iter != exes.end() ) {
		wxFileName path(*iter);
		FSOVersion ver = TCManager::GetBinaryVersion(path.GetFullName());
		exeChoice->Insert(TCManager::MakeVersionStringFromVersion(ver), 0, new FSOVersion(ver));
		iter++;
	}
}

void BasicSettingsPage::OnSelectExecutable(wxCommandEvent &event) {
	ExeChoice* choice = dynamic_cast<ExeChoice*>(
		wxWindow::FindWindowById(ID_EXE_CHOICE_BOX, this));
	wxCHECK_RET( choice != NULL, 
		_T("OnSelectExecutable: cannot find choice drop box"));

	FSOVersion* ver = dynamic_cast<FSOVersion*>(
		choice->GetClientObject(choice->GetSelection()));
	wxCHECK_RET( ver != NULL,
		_T("OnSelectExecutable: choice does not have FSOVersion data"));
	wxLogDebug(_T("Have selected ver for %s"), ver->executablename);

	ProMan::GetProfileManager()->Get()
		->Write(PRO_CFG_TC_CURRENT_BINARY, ver->executablename);
}

void BasicSettingsPage::OnSelectGraphicsAPI(wxCommandEvent &event) {
	WXUNUSED(event);
	wxChoice* api = dynamic_cast<wxChoice*>(
		wxWindow::FindWindowById(ID_GRAPHICS_COMBO, this));
	wxCHECK_RET( api != NULL, _T("Cannot find graphics api choice box"));

	wxStaticBox* box = dynamic_cast<wxStaticBox*>(
		wxWindow::FindWindowById(ID_VIDEO_STATIC_BOX, this));
	wxCHECK_RET( box != NULL, _T("Cannot find static box for video settings"));
	
	ProMan::GetProfileManager()->Get()
		->Write(PRO_CFG_VIDEO_API, api->GetStringSelection());
	box->SetLabel(wxString::Format(_("Video (%s)"), api->GetStringSelection()));
}

class Resolution: public wxClientData {
public:
	Resolution(int height, int width) {
		this->height = height;
		this->width = width;
	}
	virtual ~Resolution() {}
	int GetHeight() { return this->height; }
	int GetWidth() { return this->width; }
private:
	int height;
	int width;
};

void BasicSettingsPage::FillResolutionDropBox(wxChoice *exeChoice) {
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
			wxString resolution = wxString::Format(
				CFG_RES_FORMAT_STRING, deviceMode.dmPelsWidth, deviceMode.dmPelsHeight);
			// check to see if the resolution has already been added.
			wxArrayString strings = exeChoice->GetStrings();
			wxArrayString::iterator iter = strings.begin();
			bool exists = false;
			while ( iter != strings.end() ) {
				if ( *iter == resolution ) {
					exists = true;
				}
				iter++;
			}
			if ( !exists ) {
				exeChoice->Insert(
					resolution,
					0,
					new Resolution(deviceMode.dmPelsHeight, deviceMode.dmPelsWidth));
			}
		}
		modeCounter++;
	} while ( result == TRUE );
}

void BasicSettingsPage::OnSelectVideoResolution(wxCommandEvent &event) {
	WXUNUSED(event);
	wxChoice* choice = dynamic_cast<wxChoice*>(
		wxWindow::FindWindowById(ID_RESOLUTION_COMBO, this));
	wxCHECK_RET( choice != NULL, _T("Unable to find resolution combo"));

	Resolution* res = dynamic_cast<Resolution*>(
		choice->GetClientObject(choice->GetSelection()));
	wxCHECK_RET( res != NULL, _T("Choice does not have Resolution objects"));

	ProMan::GetProfileManager()->Get()
		->Write(PRO_CFG_VIDEO_RESOLUTION_WIDTH, res->GetWidth());
	ProMan::GetProfileManager()->Get()
		->Write(PRO_CFG_VIDEO_RESOLUTION_HEIGHT, res->GetHeight());
}

void BasicSettingsPage::OnSelectVideoDepth(wxCommandEvent &event) {
	WXUNUSED(event);
	wxChoice* depth = dynamic_cast<wxChoice*>(
		wxWindow::FindWindowById(ID_RESOLUTION_COMBO, this));
	wxCHECK_RET( depth != NULL, _T("Unable to find depth choice box"));

	ProMan::GetProfileManager()->Get()
		->Write(PRO_CFG_VIDEO_BIT_DEPTH,
		(depth->GetSelection() == 0) ? 16 : 32);
}

void BasicSettingsPage::OnSelectVideoTextureFilter(wxCommandEvent &event) {
	WXUNUSED(event);
	wxChoice* tex = dynamic_cast<wxChoice*>(
		wxWindow::FindWindowById(ID_TEXTURE_FILTER_COMBO, this));
	wxCHECK_RET( tex != NULL, _T("Unable to find texture filter choice"));

	ProMan::GetProfileManager()->Get()
		->Write(PRO_CFG_VIDEO_TEXTURE_FILTER,
		(tex->GetSelection() == 0) ? _T("Bilinear") : _T("Trilinear"));
}

void BasicSettingsPage::OnSelectVideoAnistropic(wxCommandEvent &event) {
	WXUNUSED(event);
	wxChoice* as = dynamic_cast<wxChoice*>(
		wxWindow::FindWindowById(ID_ANISOTROPIC_COMBO, this));
	wxCHECK_RET( as != NULL, _T("Unable to find anisotropic choice"));

	ProMan::GetProfileManager()->Get()
		->Write(PRO_CFG_VIDEO_ANISOTROPIC,
		(as->GetSelection() == 0) ? 0 : 1 << as->GetSelection());
}

void BasicSettingsPage::OnSelectVideoAntiAlias(wxCommandEvent &event) {
	WXUNUSED(event);
	wxChoice* aa = dynamic_cast<wxChoice*>(
		wxWindow::FindWindowById(ID_AA_COMBO, this));
	wxCHECK_RET( aa != NULL, _T("Unable to find anti-alias choice"));

	ProMan::GetProfileManager()->Get()
		->Write(PRO_CFG_VIDEO_ANTI_ALIAS, (aa->GetSelection() == 0) ? 0 : 1 << aa->GetSelection());

}

void BasicSettingsPage::OnSelectVideoGeneralSettings(wxCommandEvent &event) {
	WXUNUSED(event);
	wxChoice* gs = dynamic_cast<wxChoice*>(
		wxWindow::FindWindowById(ID_GS_COMBO, this));
	wxCHECK_RET( gs != NULL, _T("Unable to find general settings choice"));

	ProMan::GetProfileManager()->Get()
		->Write(PRO_CFG_VIDEO_GENERAL_SETTINGS, gs->GetSelection());
}

void BasicSettingsPage::OnToggleVideoUseLargeTexture(wxCommandEvent &event) {
	WXUNUSED(event);
	wxCheckBox* large = dynamic_cast<wxCheckBox*>(
		wxWindow::FindWindowById(ID_LARGE_TEXTURE_CHECK));
	wxCHECK_RET( large != NULL, _T("Unable to find large texture checkbox"));

	ProMan::GetProfileManager()->Get()
		->Write(PRO_CFG_VIDEO_USE_LARGE_TEXTURES,  large->IsChecked());
}

void BasicSettingsPage::OnToggleVideoFixFontDistortion(wxCommandEvent &event) {
	WXUNUSED(event);
	wxCheckBox* font = dynamic_cast<wxCheckBox*>(
		wxWindow::FindWindowById(ID_FONT_DISTORTION_CHECK));
	wxCHECK_RET( font != NULL, _T("Unable to find font distortion checkbox"));

	ProMan::GetProfileManager()->Get()
		->Write(PRO_CFG_VIDEO_FIX_FONT_DISTORTION, font->IsChecked());
}