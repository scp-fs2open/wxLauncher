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

DEFINE_EVENT_TYPE(EVT_FLAG_LIST_BOX_DRAW_STATUS_CHANGED);

#include <wx/listimpl.cpp> // required magic incantation
WX_DEFINE_LIST(FlagListEventHandlers);

FlagListManager* FlagListManager::flagListManager = NULL;

void FlagListManager::RegisterFlagListBoxDrawStatusChanged(wxEvtHandler *handler) {
	this->flagListBoxDrawStatusChangedHandlers.Append(handler);
}

void FlagListManager::UnRegisterFlagListBoxDrawStatusChanged(wxEvtHandler *handler) {
	this->flagListBoxDrawStatusChangedHandlers.DeleteObject(handler);
}

void FlagListManager::GenerateFlagListBoxDrawStatusChanged(const FlagListBoxStatus& status) {
	wxCommandEvent event(EVT_FLAG_LIST_BOX_DRAW_STATUS_CHANGED, wxID_NONE);
	event.SetInt(status);

	wxLogDebug(_T("Generating EVT_FLAG_LIST_BOX_DRAW_STATUS_CHANGED event"));
	for (FlagListEventHandlers::iterator iter = this->flagListBoxDrawStatusChangedHandlers.begin(),
		 end = this->flagListBoxDrawStatusChangedHandlers.end(); iter != end; ++iter) {
		wxEvtHandler* current = *iter;
		current->AddPendingEvent(event);
		wxLogDebug(_T(" Sent EVT_FLAG_LIST_BOX_DRAW_STATUS_CHANGED event to %p"), &(*iter));
	}
}

bool FlagListManager::Initialize() {
	wxASSERT(!FlagListManager::IsInitialized());
	
	FlagListManager::flagListManager = new FlagListManager();
	return true;
}

void FlagListManager::DeInitialize() {
	wxASSERT(FlagListManager::IsInitialized());
	
	FlagListManager* temp = FlagListManager::flagListManager;
	FlagListManager::flagListManager = NULL;
	delete temp;
}

bool FlagListManager::IsInitialized() {
	return (FlagListManager::flagListManager != NULL); 
}

FlagListManager* FlagListManager::GetFlagListManager() {
	wxCHECK_MSG(FlagListManager::IsInitialized(),
		NULL,
		_T("Attempt to get flag list manager when it has not been initialized."));
	
	return FlagListManager::flagListManager;
}

FlagListManager::FlagListManager() {
	// nothing for now
}

FlagListManager::~FlagListManager() {
	// nothing for now
}
