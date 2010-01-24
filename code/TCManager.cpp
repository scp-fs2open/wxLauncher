#include "TCManager.h"
#include "ProfileManager.h"

#include "wxLauncherSetup.h"

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
EVT_COMMAND(wxID_ANY, EVT_PROFILE_CHANGE, TCManager::CurrentProfileChanged)
END_EVENT_TABLE()

DEFINE_EVENT_TYPE(EVT_TC_CHANGED);
DEFINE_EVENT_TYPE(EVT_TC_BINARY_CHANGED);
DEFINE_EVENT_TYPE(EVT_TC_SELECTED_MOD_CHANGED);

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
void TCManager::GenerateTCChanged() {
	wxCommandEvent event(EVT_TC_CHANGED, wxID_NONE);
	wxLogDebug(_T("Generating EVT_TC_CHANGED event"));
	TCEventHandlers::iterator iter = TCChangedHandlers.begin();
	while (iter != TCChangedHandlers.end()) {
		wxEvtHandler* current = *iter;
		current->ProcessEvent(event);
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
		current->ProcessEvent(event);
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
		current->ProcessEvent(event);
		iter++;
		wxLogDebug(_T(" Sent event to %p"), &(*iter));
	}
}

void TCManager::CurrentProfileChanged(wxCommandEvent &WXUNUSED(event)) {

	TCManager::GenerateTCChanged();
	TCManager::GenerateTCBinaryChanged();
	TCManager::GenerateTCSelectedModChanged();
}
