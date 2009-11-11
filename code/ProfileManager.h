#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <wx/wx.h>
#include <wx/fileconf.h>

WX_DECLARE_STRING_HASH_MAP( wxFileConfig*, ProfileMap );

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
	bool NeedToPromptToSave() { return this->isAutoSaving; };
	void SetAutoSave(bool value) { this->isAutoSaving = value; };

private:
	static ProMan* proman;
	static bool isInitialized;
	wxFileConfig* currentProfile;
	wxString currentProfileName;

	ProMan();
	ProfileMap profiles; //!< The profiles. Indexed by Name;
	wxFileConfig* profileList;  //!< Global profile settings, like language, or proxy
	bool isAutoSaving; //!< Are we auto saving the profiles?
};

#endif