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

#include "apis/FREDManager.h"
#include "apis/ProfileManager.h"
#include "global/ProfileKeys.h"

LAUNCHER_DEFINE_EVENT_TYPE(EVT_FRED_ENABLED_CHANGED);

EventHandlers FREDManager::FREDEnabledChangedHandlers;

void FREDManager::RegisterFREDEnabledChanged(wxEvtHandler *handler) {
	wxASSERT_MSG(FREDEnabledChangedHandlers.IndexOf(handler) == wxNOT_FOUND,
		wxString::Format(
			_T("RegisterFREDEnabledChanged(): Handler at %p already registered."),
			handler));
	FREDEnabledChangedHandlers.Append(handler);
}
void FREDManager::UnRegisterFREDEnabledChanged(wxEvtHandler *handler) {
	wxASSERT_MSG(FREDEnabledChangedHandlers.IndexOf(handler) != wxNOT_FOUND,
		wxString::Format(
			_T("UnRegisterFREDEnabledChanged(): Handler at %p not registered."),
			handler));
	FREDEnabledChangedHandlers.DeleteObject(handler);
}
void FREDManager::GenerateFREDEnabledChanged() {
	wxCommandEvent event(EVT_FRED_ENABLED_CHANGED, wxID_NONE);
	wxLogDebug(_T("Generating EVT_FRED_ENABLED_CHANGED event"));
	EventHandlers::iterator iter = FREDEnabledChangedHandlers.begin();
	while (iter != FREDEnabledChangedHandlers.end()) {
		wxEvtHandler* current = *iter;
		current->AddPendingEvent(event);
		wxLogDebug(_T(" Sent EVT_FRED_ENABLED_CHANGED event to %p"), current);
		iter++;
	}
}
