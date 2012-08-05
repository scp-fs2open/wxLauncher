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
	enum Flags
	{
		None = 0,
		NoUpdateLastProfile = 1 << 0,
		ProManFlagsMax
	};
	static bool Initialize(Flags flags = None);
	static bool DeInitialize();
	static ProMan* GetProfileManager();

	virtual ~ProMan();
	wxArrayString GetAllProfileNames();
	wxString GetCurrentName();

	bool GlobalExists(const wxChar* strName) const;
	bool GlobalExists(wxString& strName) const;
	
	bool GlobalRead(const wxString& key, bool* b) const;
	bool GlobalRead(const wxString& key, bool* b, bool defaultVal, bool writeBackIfAbsent = false);
	bool GlobalRead(const wxString& key, wxString* str) const;
	bool GlobalRead(const wxString& key, wxString* str, const wxString& defaultVal, bool writeBackIfAbsent = false);
	bool GlobalRead(const wxString& key, long* l, long defaultVal, bool writeBackIfAbsent = false);
	
	bool GlobalWrite(const wxString& key, const wxString& value);
	bool GlobalWrite(const wxString& key, const wxChar* value);
	bool GlobalWrite(const wxString& key, long value);
	bool GlobalWrite(const wxString& key, bool value);
	
	bool ProfileExists(const wxChar* strName) const;
	bool ProfileExists(wxString& strName) const;
	
	bool ProfileRead(const wxString& key, bool* b) const;
	bool ProfileRead(const wxString& key, bool* b, bool defaultVal, bool writeBackIfAbsent = false);
	bool ProfileRead(const wxString& key, wxString* str) const;
	bool ProfileRead(const wxString& key, wxString* str, const wxString& defaultVal, bool writeBackIfAbsent = false);
	bool ProfileRead(const wxString& key, long* l) const;
	bool ProfileRead(const wxString& key, long* l, long defaultVal, bool writeBackIfAbsent = false);
	
	bool ProfileWrite(const wxString& key, const wxString& value);
	bool ProfileWrite(const wxString& key, const wxChar* value);
	bool ProfileWrite(const wxString& key, long value);
	bool ProfileWrite(const wxString& key, bool value);
	
	bool ProfileDeleteEntry(const wxString& key, bool bDeleteGroupIfEmpty = true);
	
	enum SaveDialogContext {
		ON_PROFILE_SWITCH,
		ON_PROFILE_CREATE,
		ON_EXIT
	};

	static const wxString GetSaveDialogCaptionText(SaveDialogContext context,
		const wxString& profileName);
	static const wxString GetSaveDialogMessageText(SaveDialogContext context,
		const wxString& profileName);

	bool CreateProfile(const wxString& newProfileName, const wxString& cloneFromProfileName = wxEmptyString);
	bool DeleteProfile(wxString name);
	bool DoesProfileExist(wxString name);
	bool SwitchTo(wxString name);
	void SaveCurrentProfile(bool quiet = false);
	void RevertCurrentProfile();
	bool HasUnsavedChanges();
	inline bool NeedToPromptToSave() { return (!this->isAutoSaving) && this->HasUnsavedChanges(); }
	void SetAutoSave(bool value) { this->isAutoSaving = value; }

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

	RegistryCodes PushCurrentProfile(); //!< push current profile into registry

	static const wxString& DEFAULT_PROFILE_NAME;
private:
	static ProMan* proman;
	static bool isInitialized;
	static Flags flags;
	wxFileConfig* currentProfile;
	wxString currentProfileName;
	
	bool CreateNewProfile(wxString newName);
	static wxString GenerateNewProfileFileName();

	static RegistryCodes PushProfile(wxFileConfig *cfg); //!< push profile into registry
	static RegistryCodes PullProfile(wxFileConfig *cfg); //!< pull profile from registry

	static void CopyConfig(const wxConfigBase& src, wxConfigBase &dest, const bool includeMainGroup = true, const wxString path = _T("/"));
	static void CopyConfigEntry(const wxConfigBase& src, wxConfigBase& dest, const wxString path, const wxString entry);

	static void ClearConfig(wxConfigBase& cfg);
	
	static bool AreConfigsEqual(wxConfigBase& cfg1, wxConfigBase& cfg2);
	static bool IsConfigSubset(wxConfigBase& cfg1, wxConfigBase& cfg2, const wxString path = _T("/"));
	static bool AreEntriesEqual(const wxConfigBase& cfg1, const wxConfigBase& cfg2, const wxString path, const wxString entry);

	static void LogConfigContents(wxConfigBase& cfg, const wxString path = _T("/"), const bool includeWxWindows = false);
	static void TestConfigFunctions(wxConfigBase& src);
	
	ProMan();
	void SaveProfilesBeforeExiting();

	ProfileMap profiles; //!< The profiles. Indexed by Name;
	wxFileConfig* globalProfile;  //!< Global profile settings, like language, or proxy
	wxString privateCopyFilename; //!< Name of file used for private copy
	wxFileConfig* privateCopy; //!< Private copy, used in determining whether current profile has unsaved changes
	void ResetPrivateCopy();
	bool isAutoSaving; //!< Are we auto saving the profiles?
	void GenerateChangeEvent();
	void GenerateCurrentProfileChangedEvent();

	EventHandlers eventHandlers;
};

// These operators should be refactored into a common
// macro rather than copied for the next enum flag
inline ProMan::Flags operator|(ProMan::Flags a, ProMan::Flags b)
{
	return static_cast<ProMan::Flags>(
		static_cast<int>(a) | static_cast<int>(b)
		);
}

inline ProMan::Flags operator&(ProMan::Flags a, ProMan::Flags b)
{
	return static_cast<ProMan::Flags>(
		static_cast<int>(a) & static_cast<int>(b)
		);
}

#endif