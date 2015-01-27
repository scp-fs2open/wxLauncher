/*
 Copyright (C) 2009-2013 wxLauncher Team
 
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

#ifndef RESOLUTION_MAP_H
#define RESOLUTION_MAP_H

#include <wx/event.h>
#include <wx/list.h>

#include "global/ModDefaults.h"
#include "apis/EventHandlers.h"

/** Maps a mod shortname to its user-preferred resolution. */
// TODO FIXME: Once the refresh button for the mod list is added,
// then this map will need to be cleared whenever the user presses the button.

/** ResolutionMap has changed. */
LAUNCHER_DECLARE_EVENT_TYPE(EVT_RESOLUTION_MAP_CHANGED);

struct ResolutionData {
	ResolutionData() { } // required for wxHashMap, unfortunately
	ResolutionData(long width, long height) : width(width), height(height) { }
	bool IsValid() const {
		return (width >= DEFAULT_MOD_RESOLUTION_MIN_HORIZONTAL_RES) &&
			(height >= DEFAULT_MOD_RESOLUTION_MIN_VERTICAL_RES);
	}
	long width;
	long height;
};

WX_DECLARE_STRING_HASH_MAP(
	ResolutionData,
	PreferredResolutionMap);

class ResolutionMap {
public:
	static const ResolutionData* ResolutionRead(const wxString& shortname);
	static void ResolutionWrite(const wxString& shortname, const ResolutionData& resData);
	static bool HasEntryForActiveMod();
	
	static void RegisterResolutionMapChanged(wxEvtHandler *handler);
	static void UnRegisterResolutionMapChanged(wxEvtHandler *handler);
private:
	static void GenerateResolutionMapChanged();
	static EventHandlers resolutionMapChangedHandlers;
	static PreferredResolutionMap prefResMap;
};

#endif
