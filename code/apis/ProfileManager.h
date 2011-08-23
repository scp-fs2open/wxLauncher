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
	wxArrayString GetAllProfileNames();
	wxString GetCurrentName();

	bool GlobalExists(const wxChar* strName) const;
	bool GlobalExists(wxString& strName) const;
	
	bool GlobalRead(const wxString& key, bool* b) const;
	bool GlobalRead(const wxString& key, bool* d, bool defaultVal) const;
	bool GlobalRead(const wxString& key, wxString* str) const;
	bool GlobalRead(const wxString& key, wxString* str, const wxString& defaultVal) const;
	bool GlobalRead(const wxString& key, long* l, long defaultVal) const;
	
	bool GlobalWrite(const wxString& key, const wxString& value);
	bool GlobalWrite(const wxString& key, long value);
	bool GlobalWrite(const wxString& key, bool value);
	
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
		SupportNotCompiledIn,
		UnknownError,
	};

	static RegistryCodes PushProfile(wxFileConfig *cfg); //!< push profile into registry
	static RegistryCodes PullProfile(wxFileConfig *cfg); //!< pull profile from registry

	static const wxString& DEFAULT_PROFILE_NAME;
private:
	static ProMan* proman;
	static bool isInitialized;
	wxFileConfig* currentProfile;
	wxString currentProfileName;

	ProMan();
	ProfileMap profiles; //!< The profiles. Indexed by Name;
	wxFileConfig* globalProfile;  //!< Global profile settings, like language, or proxy
	bool isAutoSaving; //!< Are we auto saving the profiles?
	void GenerateChangeEvent();
	void GenerateCurrentProfileChangedEvent();

	EventHandlers eventHandlers;
};

#endif