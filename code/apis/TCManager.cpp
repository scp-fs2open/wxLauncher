/*
Copyright (C) 2009-2015 wxLauncher Team

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
	wxLogDebug(_T("TCManager is at %p."), this);
	ProMan::GetProfileManager()->AddEventHandler(this);
}
/** Destructor. Static class does nothing. */
TCManager::~TCManager() {
	ProMan::GetProfileManager()->RemoveEventHandler(this);
}

TCManager* TCManager::manager = NULL;
EventHandlers TCManager::TCChangedHandlers;
EventHandlers TCManager::TCBinaryChangedHandlers;
EventHandlers TCManager::TCActiveModChangedHandlers;
EventHandlers TCManager::TCFredBinaryChangedHandlers;

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

LAUNCHER_DEFINE_EVENT_TYPE(EVT_TC_CHANGED);
LAUNCHER_DEFINE_EVENT_TYPE(EVT_TC_BINARY_CHANGED);
LAUNCHER_DEFINE_EVENT_TYPE(EVT_TC_ACTIVE_MOD_CHANGED);
LAUNCHER_DEFINE_EVENT_TYPE(EVT_TC_FRED_BINARY_CHANGED);

void TCManager::RegisterTCBinaryChanged(wxEvtHandler *handler) {
	wxASSERT_MSG(TCBinaryChangedHandlers.IndexOf(handler) == wxNOT_FOUND,
		wxString::Format(
			_T("RegisterTCBinaryChanged(): Handler at %p already registered."),
			handler));
	TCBinaryChangedHandlers.Append(handler);
}
void TCManager::UnRegisterTCBinaryChanged(wxEvtHandler *handler) {
	wxASSERT_MSG(TCBinaryChangedHandlers.IndexOf(handler) != wxNOT_FOUND,
		wxString::Format(
			_T("UnRegisterTCBinaryChanged(): Handler at %p not registered."),
			handler));
	TCBinaryChangedHandlers.DeleteObject(handler);
}
void TCManager::RegisterTCChanged(wxEvtHandler *handler) {
	wxASSERT_MSG(TCChangedHandlers.IndexOf(handler) == wxNOT_FOUND,
		wxString::Format(
			_T("RegisterTCChanged(): Handler at %p already registered."),
			handler));
	TCChangedHandlers.Append(handler);
}
void TCManager::UnRegisterTCChanged(wxEvtHandler *handler) {
	wxASSERT_MSG(TCChangedHandlers.IndexOf(handler) != wxNOT_FOUND,
		wxString::Format(
			_T("UnRegisterTCChanged(): Handler at %p not registered."),
			handler));
	TCChangedHandlers.DeleteObject(handler);
}
void TCManager::RegisterTCActiveModChanged(wxEvtHandler *handler) {
	wxASSERT_MSG(TCActiveModChangedHandlers.IndexOf(handler) == wxNOT_FOUND,
		wxString::Format(
			_T("RegisterTCActiveModChanged(): Handler at %p already registered."),
			handler));
	TCActiveModChangedHandlers.Append(handler);
}
void TCManager::UnRegisterTCActiveModChanged(wxEvtHandler *handler) {
	wxASSERT_MSG(TCActiveModChangedHandlers.IndexOf(handler) != wxNOT_FOUND,
		wxString::Format(
			_T("UnRegisterTCActiveModChanged(): Handler at %p not registered."),
			handler));
	TCActiveModChangedHandlers.DeleteObject(handler);
}
void TCManager::RegisterTCFredBinaryChanged(wxEvtHandler *handler) {
	wxASSERT_MSG(TCFredBinaryChangedHandlers.IndexOf(handler) == wxNOT_FOUND,
		wxString::Format(
			_T("RegisterTCFredBinaryChanged(): Handler at %p already registered."),
			handler));
	TCFredBinaryChangedHandlers.Append(handler);
}
void TCManager::UnRegisterTCFredBinaryChanged(wxEvtHandler *handler) {
	wxASSERT_MSG(TCFredBinaryChangedHandlers.IndexOf(handler) != wxNOT_FOUND,
		wxString::Format(
			_T("UnRegisterTCFredBinaryChanged(): Handler at %p not registered."),
			handler));
	TCFredBinaryChangedHandlers.DeleteObject(handler);
}
void TCManager::GenerateTCChanged() {
	wxCommandEvent event(EVT_TC_CHANGED, wxID_NONE);
	wxLogDebug(_T("Generating EVT_TC_CHANGED event"));
	EventHandlers::iterator iter = TCChangedHandlers.begin();
	while (iter != TCChangedHandlers.end()) {
		wxEvtHandler* current = *iter;
		current->AddPendingEvent(event);
		wxLogDebug(_T(" Sent EVT_TC_CHANGED event to %p"), current);
		iter++;
	}
}
void TCManager::GenerateTCBinaryChanged() {
	wxCommandEvent event(EVT_TC_BINARY_CHANGED, wxID_NONE);
	wxLogDebug(_T("Generating EVT_TC_BINARY_CHANGED event"));
	EventHandlers::iterator iter = TCBinaryChangedHandlers.begin();
	while (iter != TCBinaryChangedHandlers.end()) {
		wxEvtHandler* current = *iter;
		current->AddPendingEvent(event);
		wxLogDebug(_T(" Sent EVT_TC_BINARY_CHANGED event to %p"), current);
		iter++;
	}
}
void TCManager::GenerateTCActiveModChanged() {
	wxCommandEvent event(EVT_TC_ACTIVE_MOD_CHANGED, wxID_NONE);
	wxLogDebug(_T("Generating EVT_TC_ACTIVE_MOD_CHANGED event"));
	EventHandlers::iterator iter = TCActiveModChangedHandlers.begin();
	while (iter != TCActiveModChangedHandlers.end()) {
		wxEvtHandler* current = *iter;
		current->AddPendingEvent(event);
		wxLogDebug(_T(" Sent EVT_TC_ACTIVE_MOD_CHANGED event to %p"), current);
		iter++;
	}
}
void TCManager::GenerateTCFredBinaryChanged() {
	wxCommandEvent event(EVT_TC_FRED_BINARY_CHANGED, wxID_NONE);
	wxLogDebug(_T("Generating EVT_TC_FRED_BINARY_CHANGED event"));
	EventHandlers::iterator iter = TCFredBinaryChangedHandlers.begin();
	while (iter != TCFredBinaryChangedHandlers.end()) {
		wxEvtHandler* current = *iter;
		current->AddPendingEvent(event);
		wxLogDebug(_T(" Sent EVT_TC_FRED_BINARY_CHANGED event to %p"), current);
		iter++;
	}
}

void TCManager::CurrentProfileChanged(wxCommandEvent &WXUNUSED(event)) {

	TCManager::GenerateTCChanged();
//	it's assumed that BasicSettingsPage::OnTCChanged() (which is called on an EVT_TC_CHANGED event)
//	calls TCManager::GenerateTCBinaryChanged() unconditionally, so no need to explicitly call it here
//	it's also assumed that BasicSettingsPage::OnTCChanged() calls TCManager::GenerateTCFredBinaryChanged()
//	unconditionally if FRED launching is enabled
}
