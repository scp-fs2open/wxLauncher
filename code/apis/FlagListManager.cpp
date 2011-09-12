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

#include "apis/FlagListManager.h"

/** \class FlagListManager
 FlagListManager is used to notify controls that have registered with it
 that the flag list box's status has changed. */

FlagListEventHandlers FlagListManager::FlagListBoxDrawStatusChangedHandlers;

DEFINE_EVENT_TYPE(EVT_FLAG_LIST_BOX_DRAW_STATUS_CHANGED);

#include <wx/listimpl.cpp> // required magic incantation
WX_DEFINE_LIST(FlagListEventHandlers);

void FlagListManager::RegisterFlagListBoxDrawStatusChanged(wxEvtHandler *handler) {
	FlagListBoxDrawStatusChangedHandlers.Append(handler);
}

void FlagListManager::UnRegisterFlagListBoxDrawStatusChanged(wxEvtHandler *handler) {
	FlagListBoxDrawStatusChangedHandlers.DeleteObject(handler);
}

void FlagListManager::GenerateFlagListBoxDrawStatusChanged(bool isDrawOK) {
	wxCommandEvent event(EVT_FLAG_LIST_BOX_DRAW_STATUS_CHANGED, wxID_NONE);
	event.SetInt(isDrawOK);
	wxLogDebug(_T("Generating EVT_FLAG_LIST_BOX_DRAW_STATUS_CHANGED event"));
	for (FlagListEventHandlers::iterator iter = FlagListBoxDrawStatusChangedHandlers.begin(),
		 end = FlagListBoxDrawStatusChangedHandlers.end(); iter != end; ++iter) {
		wxEvtHandler* current = *iter;
		current->AddPendingEvent(event);
		wxLogDebug(_T(" Sent EVT_FLAG_LIST_BOX_DRAW_STATUS_CHANGED event to %p"), &(*iter));
	}
}
