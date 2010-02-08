#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <wx/wx.h>
#include <wx/fileconf.h>

WX_DECLARE_STRING_HASH_MAP( wxFileConfig*, ProfileMap );
WX_DECLARE_LIST(wxEvtHandler, EventHandlers);

/** event is generated anytime the number of profiles in the manager change. */
DECLARE_EVENT_TYPE(EVT_PROFILE_CHANGE, -1);
/** Event is generated anytime the currently selected profile is changed. */
DECLARE_EVENT_TYPE(EVT_CURRENT_PROFILE_CHANGED, -1);

class ProMan {
public:
	static bool Initialize();
	static bool DeInitialize();
	static ProMan* GetProfileManager();
	static bool PrepareForAppShutdown();

	virtual ~ProMan();
	wxFileConfig* Get();
	wxFileConfig* Global();
	wxArrayString GetAllProfileNames();
	wxString GetCurrentName();

	bool CreateNewProfile(wxString newName);
	bool CloneProfile(wxString orignalName, wxString copyName);
	bool DeleteProfile(wxString name);
	bool DoesProfileExist(wxString name);
	bool SwitchTo(wxString name);
	void SaveCurrentProfile();
	inline bool NeedToPromptToSave() { return !this->isAutoSaving; };
	void SetAutoSave(bool value) { this->isAutoSaving = value; };

	void AddEventHandler(wxEvtHandler *handler);
	void RemoveEventHandler(wxEvtHandler *handler);

	enum RegistryCodes {
		InvalidParameters = -1,
		NoError = 0,
		AccessDenied,

		FileIsNotOK,
		InputFileDoesNotExist,
		UnknownError,
	};

	static RegistryCodes PushProfile(wxFileConfig *cfg); //!< push profile into registry
	static RegistryCodes PullProfile(wxFileConfig *cfg); //!< pull profile from registry


private:
	static ProMan* proman;
	static bool isInitialized;
	wxFileConfig* currentProfile;
	wxString currentProfileName;

	ProMan();
	ProfileMap profiles; //!< The profiles. Indexed by Name;
	wxFileConfig* profileList;  //!< Global profile settings, like language, or proxy
	bool isAutoSaving; //!< Are we auto saving the profiles?
	void GenerateChangeEvent();
	void GenerateCurrentProfileChangedEvent();

	EventHandlers eventHandlers;
};

#endif