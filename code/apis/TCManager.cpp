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

#include "apis/TCManager.h"
#include "apis/ProfileManager.h"

#include "global/MemoryDebugging.h"

/** \class TCManager
TCManager is used to pickup CurrentProfileChanged events from ProMan and 
send the approriate messages to the controls that care.  It also sends 
out events when the binary or root folder get changed by user controls
so that other controls that are affected (like ModList) get notified in all
cases and will be able to update the information. */

/** Contructor. Static class does nothing. */
TCManager::TCManager() {
	ProMan::GetProfileManager()->AddEventHandler(this);
}
/** Destructor. Static class does nothing. */
TCManager::~TCManager() {
	ProMan::GetProfileManager()->RemoveEventHandler(this);
}

TCManager* TCManager::manager = NULL;
TCEventHandlers TCManager::TCChangedHandlers;
TCEventHandlers TCManager::TCBinaryChangedHandlers;
TCEventHandlers TCManager::TCSelectedModChangedHandlers;
TCEventHandlers TCManager::TCFredBinaryChangedHandlers;

void TCManager::Initialize() {
	if ( !IsInitialized() ) {
		manager = new TCManager();
	}
}

void TCManager::DeInitialize() {
	if ( IsInitialized()) {
		delete manager;
		manager = NULL;
	}
}

bool TCManager::IsInitialized() {
	return manager != NULL;
}

TCManager* TCManager::Get() {
	return manager;
}

///////////////////////////////////////////////////////////////////////////////
///// Events
BEGIN_EVENT_TABLE(TCManager, wxEvtHandler)
EVT_COMMAND(wxID_ANY, EVT_CURRENT_PROFILE_CHANGED, TCManager::CurrentProfileChanged)
END_EVENT_TABLE()

DEFINE_EVENT_TYPE(EVT_TC_CHANGED);
DEFINE_EVENT_TYPE(EVT_TC_BINARY_CHANGED);
DEFINE_EVENT_TYPE(EVT_TC_SELECTED_MOD_CHANGED);
DEFINE_EVENT_TYPE(EVT_TC_FRED_BINARY_CHANGED);

#include <wx/listimpl.cpp> // required magic incatation
WX_DEFINE_LIST(TCEventHandlers);

void TCManager::RegisterTCBinaryChanged(wxEvtHandler *handler) {
	TCBinaryChangedHandlers.Append(handler);
}
void TCManager::UnRegisterTCBinaryChanged(wxEvtHandler *handler) {
	TCBinaryChangedHandlers.DeleteObject(handler);
}
void TCManager::RegisterTCChanged(wxEvtHandler *handler) {
	TCChangedHandlers.Append(handler);
}
void TCManager::UnRegisterTCChanged(wxEvtHandler *handler) {
	TCChangedHandlers.DeleteObject(handler);
}
void TCManager::RegisterTCSelectedModChanged(wxEvtHandler *handler) {
	TCSelectedModChangedHandlers.Append(handler);
}
void TCManager::UnRegisterTCSelectedModChanged(wxEvtHandler *handler) {
	TCSelectedModChangedHandlers.DeleteObject(handler);
}
void TCManager::RegisterTCFredBinaryChanged(wxEvtHandler *handler) {
	TCFredBinaryChangedHandlers.Append(handler);
}
void TCManager::UnRegisterTCFredBinaryChanged(wxEvtHandler *handler) {
	TCFredBinaryChangedHandlers.DeleteObject(handler);
}
void TCManager::GenerateTCChanged() {
	wxCommandEvent event(EVT_TC_CHANGED, wxID_NONE);
	wxLogDebug(_T("Generating EVT_TC_CHANGED event"));
	TCEventHandlers::iterator iter = TCChangedHandlers.begin();
	while (iter != TCChangedHandlers.end()) {
		wxEvtHandler* current = *iter;
		current->ProcessEvent(*(event.Clone()));
		iter++;
		wxLogDebug(_T(" Sent event to %p"), &(*iter));
	}
}
void TCManager::GenerateTCBinaryChanged() {
	wxCommandEvent event(EVT_TC_BINARY_CHANGED, wxID_NONE);
	wxLogDebug(_T("Generating EVT_TC_BINARY_CHANGED event"));
	TCEventHandlers::iterator iter = TCBinaryChangedHandlers.begin();
	while (iter != TCBinaryChangedHandlers.end()) {
		wxEvtHandler* current = *iter;
		current->ProcessEvent(*(event.Clone()));
		iter++;
		wxLogDebug(_T(" Sent event to %p"), &(*iter));
	}
}
void TCManager::GenerateTCSelectedModChanged() {
	wxCommandEvent event(EVT_TC_SELECTED_MOD_CHANGED, wxID_NONE);
	wxLogDebug(_T("Generating EVT_TC_SELECTED_MOD_CHANGED event"));
	TCEventHandlers::iterator iter = TCSelectedModChangedHandlers.begin();
	while (iter != TCSelectedModChangedHandlers.end()) {
		wxEvtHandler* current = *iter;
		current->ProcessEvent(*(event.Clone()));
		iter++;
		wxLogDebug(_T(" Sent event to %p"), &(*iter));
	}
}
void TCManager::GenerateTCFredBinaryChanged() {
	wxCommandEvent event(EVT_TC_FRED_BINARY_CHANGED, wxID_NONE);
	wxLogDebug(_T("Generating EVT_TC_FRED_BINARY_CHANGED event"));
	TCEventHandlers::iterator iter = TCFredBinaryChangedHandlers.begin();
	while (iter != TCBinaryChangedHandlers.end()) {
		wxEvtHandler* current = *iter;
		current->ProcessEvent(*(event.Clone()));
		iter++;
		wxLogDebug(_T(" Sent event to %p"), &(*iter));
	}
}

void TCManager::CurrentProfileChanged(wxCommandEvent &WXUNUSED(event)) {

	TCManager::GenerateTCChanged();
	TCManager::GenerateTCBinaryChanged();
	TCManager::GenerateTCSelectedModChanged();
	TCManager::GenerateTCFredBinaryChanged();
}
