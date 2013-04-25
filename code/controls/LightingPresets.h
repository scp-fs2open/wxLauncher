/*
 Copyright (C) 2009-2011 wxLauncher Team
 
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

#ifndef LIGHTINGPRESETS_H
#define LIGHTINGPRESETS_H

#include <wx/wx.h>

class Preset {
public:
	Preset() { } // needed for wxHashMap, unfortunately
	Preset(const wxString& name, const int buttonId, const wxString& preset);
	const wxString& GetName() const { return this->name; }
	const int GetButtonId() const { return this->buttonId; }
	const wxString& GetFlagSet() const { return this->flagSet; }
	void SetFlagSet(const wxString& flagSet);
private:
	wxString name;
	int buttonId;
	wxString flagSet;
};

// from wxWidgets window IDs to Presets
WX_DECLARE_HASH_MAP(int, Preset, wxIntegerHash, wxIntegerEqual, PresetHashMap);

class LightingPresets: public wxPanel {
public:
	LightingPresets(wxWindow* parent);

	void OnSelectLightingPreset(wxCommandEvent &event);
	void OnCopyLightingPreset(wxCommandEvent &event);
	void OnActiveModChanged(wxCommandEvent &event);
	void OnProxyFlagDataReady(wxCommandEvent &event);

	static const wxString& PresetNameToPresetFlagSet(const wxString& presetName);

private:
	void Initialize();
	void Reset();
	
	static void InitializePresets();
	static int PresetNameToPresetButtonId(const wxString& presetName);
	static const wxString& PresetButtonIdToPresetName(int buttonId);

	static PresetHashMap presets;

	DECLARE_EVENT_TABLE()	
};

#endif
