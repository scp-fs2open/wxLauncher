/*
 Copyright (C) 2012-2015 wxLauncher Team
 
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

#ifndef PROFILEPROXY_H
#define PROFILEPROXY_H

#include <wx/wx.h>
#include <wx/event.h>

#include <map>
#include <vector>

#include "datastructures/FlagFileData.h"
#include "apis/EventHandlers.h"

/* ProfileProxy - a high-level API for the data in the current profile.
   Other classes should use the ProfileProxy instead of the ProfileManager
   whenever possible. */

/** Indicates the proxy has reset itself. */
DECLARE_EVENT_TYPE(EVT_PROXY_RESET, wxID_ANY);

/** Indicates the proxy has successfully processed flag data from the flag file
 and profile. */
DECLARE_EVENT_TYPE(EVT_PROXY_FLAG_DATA_READY, wxID_ANY);

WX_DECLARE_STRING_HASH_MAP(int, FlagStringToIndexMap);

class ProfileProxy: public wxEvtHandler {
public:
	static ProfileProxy* GetProxy();
	static bool Initialize();
	static void DeInitialize();
	static bool IsInitialized();

	void RegisterProxyReset(wxEvtHandler *handler);
	void UnRegisterProxyReset(wxEvtHandler *handler);
	
	void RegisterProxyFlagDataReady(wxEvtHandler *handler);
	void UnRegisterProxyFlagDataReady(wxEvtHandler *handler);
	
	virtual ~ProfileProxy();

	void OnFlagFileProcessingStatusChanged(wxCommandEvent &event);
	
	/** Indicates whether the proxy has processed flag data for the current
	 profile's FSO binary and flag line. */
	bool IsFlagDataReady() const { return this->isFlagDataReady; }
	
	/** Sets a flag from the flag list. */
	void SetFlag(const wxString& flag, bool isChecked);
	
	/** Gets the enabled flag list flags as individual flag strings. */
	std::vector<wxString> GetEnabledFlags() const;
	
	/** Gets the enabled flag list flags as a single string. */
	wxString GetEnabledFlagsString() const;
	
	/** Gets the custom flags. */
	const wxString& GetCustomFlags() const { return this->customFlags; }
	
	/** Sets the custom flags. */
	void SetCustomFlags(const wxString& customFlags, bool updateTextCtrl = true);
	
	/** Indicates whether the current profile has a lighting preset entry. */
	bool HasLightingPreset() const;
	
	/** Gets the name of the selected lighting preset.
	   Returns an empty string if no lighting preset is in the profile
	 or if the preset name stored in the profile is empty. */
	wxString GetLightingPresetName() const;
	
	/** Sets the current lighting preset. */
	void SetLightingPreset(const wxString& presetName);
	
	/** Prepends the current lighting preset to custom flags
	 The profile must have a non-empty lighting preset name.*/
	void CopyPresetToCustomFlags();
	
	/** Returns whether the current profile has been initialized. */
	bool IsProfileInitialized() const;
	
	/** Finishes initialization of the current profile.
	 The profile must not already be initialized. */
	void FinishProfileInitialization() const;
	
private:
	static ProfileProxy* proxy;
	ProfileProxy();
	
	EventHandlers resetEventHandlers;
	EventHandlers readyEventHandlers;

	void GenerateProxyReset();
	void GenerateProxyFlagDataReady();
	
	/** Writes both flag list flags and custom flags to profile. */
	void WriteFlagLineToProfile() const;
	
	/** Processes data extracted from the flag file. */
	void ProcessFlagData(const ProxyFlagData& data);
	
	/** Process the current profile's flag line,
	 using the data extracted from the flag file. */
	void ProcessFlagLine();
	
	/** Returns all indices (if any) where flag is found in enabledFlags. */
	std::vector<int> FindInEnabledFlags(const wxString& flag) const;
	
	/** Verifies that the flag is either not in enabledFlags or at its flagIndex.
	 Returns an empty string on success, an error message otherwise. */
	wxString VerifyEnabledFlagsForFlag(const wxString& flag, int flagIndex) const;
	
	void Reset();
	
	std::map<int, wxString> enabledFlags; // ordered, flagIndex to flagString
	
	FlagStringToIndexMap flagMap;
	
	wxString customFlags;
	
	bool isFlagDataReady;
	
	DECLARE_EVENT_TABLE()
};

#endif
