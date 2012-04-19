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
 that the flag file processing's status has changed. */

DEFINE_EVENT_TYPE(EVT_FLAG_FILE_PROCESSING_STATUS_CHANGED);

#include <wx/listimpl.cpp> // required magic incantation
WX_DEFINE_LIST(FlagFileProcessingEventHandlers);

FlagListManager* FlagListManager::flagListManager = NULL;

void FlagListManager::RegisterFlagFileProcessingStatusChanged(wxEvtHandler *handler) {
	this->flagFileProcessingStatusChangedHandlers.Append(handler);
}

void FlagListManager::UnRegisterFlagFileProcessingStatusChanged(wxEvtHandler *handler) {
	this->flagFileProcessingStatusChangedHandlers.DeleteObject(handler);
}

void FlagListManager::GenerateFlagFileProcessingStatusChanged(const FlagFileProcessingStatus& status) {
	wxCommandEvent event(EVT_FLAG_FILE_PROCESSING_STATUS_CHANGED, wxID_NONE);
	event.SetInt(status);

	wxLogDebug(_T("Generating EVT_FLAG_FILE_PROCESSING_STATUS_CHANGED event"));
	for (FlagFileProcessingEventHandlers::iterator
		 iter = this->flagFileProcessingStatusChangedHandlers.begin(),
		 end = this->flagFileProcessingStatusChangedHandlers.end(); iter != end; ++iter) {
		wxEvtHandler* current = *iter;
		current->AddPendingEvent(event);
		wxLogDebug(_T(" Sent EVT_FLAG_FILE_PROCESSING_STATUS_CHANGED event to %p"), &(*iter));
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
