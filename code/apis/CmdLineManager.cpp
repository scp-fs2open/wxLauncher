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

#include "apis/CmdLineManager.h"

/** \class CmdLineManager
 CmdLineManager is used to notify controls that have registered with it
 that the command line or custom flags have changed. */

CmdLineEventHandlers CmdLineManager::CmdLineChangedHandlers;
CmdLineEventHandlers CmdLineManager::CustomFlagsChangedHandlers;

DEFINE_EVENT_TYPE(EVT_CMD_LINE_CHANGED);
DEFINE_EVENT_TYPE(EVT_CUSTOM_FLAGS_CHANGED);

#include <wx/listimpl.cpp> // required magic incantation
WX_DEFINE_LIST(CmdLineEventHandlers);

void CmdLineManager::RegisterCmdLineChanged(wxEvtHandler *handler) {
	wxASSERT_MSG(CmdLineChangedHandlers.IndexOf(handler) == wxNOT_FOUND,
		wxString::Format(
			_T("RegisterCmdLineChanged(): Handler at %p already registered."),
			handler));
	CmdLineChangedHandlers.Append(handler);
}
void CmdLineManager::RegisterCustomFlagsChanged(wxEvtHandler *handler) {
	wxASSERT_MSG(CustomFlagsChangedHandlers.IndexOf(handler) == wxNOT_FOUND,
		wxString::Format(
			_T("RegisterCustomFlagsChanged(): Handler at %p already registered."),
			handler));
	CustomFlagsChangedHandlers.Append(handler);
}
void CmdLineManager::UnRegisterCmdLineChanged(wxEvtHandler *handler) {
	wxASSERT_MSG(CmdLineChangedHandlers.IndexOf(handler) != wxNOT_FOUND,
		wxString::Format(
			_T("UnRegisterCmdLineChanged(): Handler at %p not registered."),
			handler));
	CmdLineChangedHandlers.DeleteObject(handler);
}
void CmdLineManager::UnRegisterCustomFlagsChanged(wxEvtHandler *handler) {
	wxASSERT_MSG(CustomFlagsChangedHandlers.IndexOf(handler) != wxNOT_FOUND,
		wxString::Format(
			_T("UnRegisterCustomFlagsChanged(): Handler at %p not registered."),
			handler));
	CustomFlagsChangedHandlers.DeleteObject(handler);
}

void CmdLineManager::GenerateCmdLineChanged() {
	wxCommandEvent event(EVT_CMD_LINE_CHANGED, wxID_NONE);
	wxLogDebug(_T("Generating EVT_CMD_LINE_CHANGED event"));
	for (CmdLineEventHandlers::iterator iter = CmdLineChangedHandlers.begin(),
		 end = CmdLineChangedHandlers.end(); iter != end; ++iter) {
		wxEvtHandler* current = *iter;
		current->AddPendingEvent(event);
		wxLogDebug(_T(" Sent EVT_CMD_LINE_CHANGED event to %p"), current);
	}
}

void CmdLineManager::GenerateCustomFlagsChanged() {
	wxCommandEvent event(EVT_CUSTOM_FLAGS_CHANGED, wxID_NONE);
	wxLogDebug(_T("Generating EVT_CUSTOM_FLAGS_CHANGED event"));
	for (CmdLineEventHandlers::iterator iter = CustomFlagsChangedHandlers.begin(),
		 end = CustomFlagsChangedHandlers.end(); iter != end; ++iter) {
		wxEvtHandler* current = *iter;
		current->AddPendingEvent(event);
		wxLogDebug(_T(" Sent EVT_CUSTOM_FLAGS_CHANGED event to %p"), current);
	}
}
