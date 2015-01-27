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

#include "datastructures/ResolutionMap.h"

#include <wx/hashmap.h>

#include "controls/ModList.h"
#include "apis/EventHandlers.h"

LAUNCHER_DEFINE_EVENT_TYPE(EVT_RESOLUTION_MAP_CHANGED);

EventHandlers ResolutionMap::resolutionMapChangedHandlers;

PreferredResolutionMap ResolutionMap::prefResMap;

const ResolutionData* ResolutionMap::ResolutionRead(const wxString& shortname) {
	PreferredResolutionMap::const_iterator it = prefResMap.find(shortname);
	
	return (it == prefResMap.end()) ? NULL : &it->second;
}

void ResolutionMap::ResolutionWrite(const wxString& shortname, const ResolutionData& resData) {
	wxASSERT(!shortname.IsEmpty());
	wxASSERT(resData.IsValid());
	
	prefResMap[shortname] = resData;
	wxLogDebug(_T("Wrote resolution %dx%d for mod %s"),
		resData.width, resData.height, shortname.c_str());
	GenerateResolutionMapChanged();
}

bool ResolutionMap::HasEntryForActiveMod() {
	const ModItem* activeMod = ModList::GetActiveMod();
	wxCHECK_MSG(activeMod != NULL, false,
		_T("HasEntryForActiveMod: activeMod is NULL!"));
	
	return prefResMap.find(activeMod->shortname) != prefResMap.end();
}

void ResolutionMap::RegisterResolutionMapChanged(wxEvtHandler *handler) {
	wxASSERT_MSG(resolutionMapChangedHandlers.IndexOf(handler) == wxNOT_FOUND,
		wxString::Format(
			_T("RegisterResolutionMapChanged(): Handler at %p already registered."),
			handler));
	resolutionMapChangedHandlers.Append(handler);
}
void ResolutionMap::UnRegisterResolutionMapChanged(wxEvtHandler *handler) {
	wxASSERT_MSG(resolutionMapChangedHandlers.IndexOf(handler) != wxNOT_FOUND,
		wxString::Format(
			_T("UnRegisterResolutionMapChanged(): Handler at %p not registered."),
			handler));
	resolutionMapChangedHandlers.DeleteObject(handler);
}
void ResolutionMap::GenerateResolutionMapChanged() {
	wxCommandEvent event(EVT_RESOLUTION_MAP_CHANGED, wxID_NONE);
	wxLogDebug(_T("Generating EVT_RESOLUTION_MAP_CHANGED event"));
	EventHandlers::iterator iter = resolutionMapChangedHandlers.begin();
	while (iter != resolutionMapChangedHandlers.end()) {
		wxEvtHandler* current = *iter;
		current->AddPendingEvent(event);
		wxLogDebug(_T(" Sent EVT_RESOLUTION_MAP_CHANGED event to %p"), current);
		iter++;
	}
}
