/*
 Copyright (C) 2012 wxLauncher Team
 
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

#include "apis/ProfileProxy.h"

#include "apis/CmdLineManager.h"
#include "apis/FlagListManager.h"
#include "apis/ProfileManager.h"

#include "controls/LightingPresets.h"

#include "global/ids.h"

#include <wx/tokenzr.h>

DEFINE_EVENT_TYPE(EVT_PROXY_RESET);
DEFINE_EVENT_TYPE(EVT_PROXY_FLAG_DATA_READY);

#include <wx/listimpl.cpp> // required magic incantation
WX_DEFINE_LIST(ProxyEventHandlers);

void ProfileProxy::RegisterProxyReset(wxEvtHandler *handler) {
	wxASSERT(ProfileProxy::IsInitialized());
	wxASSERT_MSG(this->resetEventHandlers.IndexOf(handler) == wxNOT_FOUND,
		wxString::Format(
			_T("RegisterProxyReset(): Handler at %p already registered."),
			handler));
	this->resetEventHandlers.Append(handler);
}

void ProfileProxy::UnRegisterProxyReset(wxEvtHandler *handler) {
	wxASSERT(ProfileProxy::IsInitialized());
	wxASSERT_MSG(this->resetEventHandlers.IndexOf(handler) != wxNOT_FOUND,
		wxString::Format(
			_T("UnRegisterProxyReset(): Handler at %p not registered."),
			handler));
	this->resetEventHandlers.DeleteObject(handler);
}

void ProfileProxy::RegisterProxyFlagDataReady(wxEvtHandler *handler) {
	wxASSERT(ProfileProxy::IsInitialized());
	wxASSERT_MSG(this->readyEventHandlers.IndexOf(handler) == wxNOT_FOUND,
		wxString::Format(
			_T("RegisterProxyFlagDataReady(): Handler at %p already registered."),
			handler));
	this->readyEventHandlers.Append(handler);
}

void ProfileProxy::UnRegisterProxyFlagDataReady(wxEvtHandler *handler) {
	wxASSERT(ProfileProxy::IsInitialized());
	wxASSERT_MSG(this->readyEventHandlers.IndexOf(handler) != wxNOT_FOUND,
		wxString::Format(
			_T("UnRegisterProxyFlagDataReady(): Handler at %p not registered."),
			handler));
	this->readyEventHandlers.DeleteObject(handler);
}

void ProfileProxy::GenerateProxyReset() {
	wxASSERT(ProfileProxy::IsInitialized());
	
	wxCommandEvent event(EVT_PROXY_RESET, wxID_NONE);
	
	wxLogDebug(_T("Generating EVT_PROXY_RESET event"));
	for (ProxyEventHandlers::iterator
		 iter = this->resetEventHandlers.begin(),
		 end = this->resetEventHandlers.end();
		 iter != end; ++iter) {
		wxEvtHandler* current = *iter;
		current->AddPendingEvent(event);
		wxLogDebug(_T(" Sent EVT_PROXY_RESET event to %p"), current);
	}
}

void ProfileProxy::GenerateProxyFlagDataReady() {
	wxASSERT(ProfileProxy::IsInitialized());
	
	wxCommandEvent event(EVT_PROXY_FLAG_DATA_READY, wxID_NONE);
	
	wxLogDebug(_T("Generating EVT_PROXY_FLAG_DATA_READY event"));
	for (ProxyEventHandlers::iterator
		 iter = this->readyEventHandlers.begin(),
		 end = this->readyEventHandlers.end();
		 iter != end; ++iter) {
		wxEvtHandler* current = *iter;
		current->AddPendingEvent(event);
		wxLogDebug(_T(" Sent EVT_PROXY_FLAG_DATA_READY event to %p"), current);
	}
}


ProfileProxy* ProfileProxy::proxy = NULL;

ProfileProxy* ProfileProxy::GetProxy() {
	wxCHECK_MSG(IsInitialized(), NULL,
		_T("GetProxy() called when proxy wasn't initialized."));
	
	return proxy;
}

bool ProfileProxy::Initialize() {
	wxASSERT(!IsInitialized());
	
	proxy = new ProfileProxy();
	return true;
}

void ProfileProxy::DeInitialize() {
	wxASSERT(IsInitialized());
	
	ProfileProxy* temp = proxy;
	proxy = NULL;
	delete temp;
}

bool ProfileProxy::IsInitialized() {
	return proxy != NULL;
}

ProfileProxy::ProfileProxy()
: isFlagDataReady(false) {
	FlagListManager::RegisterFlagFileProcessingStatusChanged(this);
}

ProfileProxy::~ProfileProxy() {
	FlagListManager::UnRegisterFlagFileProcessingStatusChanged(this);
}

BEGIN_EVENT_TABLE(ProfileProxy, wxEvtHandler)
EVT_COMMAND(wxID_NONE, EVT_FLAG_FILE_PROCESSING_STATUS_CHANGED,
	ProfileProxy::OnFlagFileProcessingStatusChanged)
END_EVENT_TABLE()


void ProfileProxy::OnFlagFileProcessingStatusChanged(wxCommandEvent &event) {
	wxASSERT(event.GetEventType() == EVT_FLAG_FILE_PROCESSING_STATUS_CHANGED);
	
	const FlagListManager::FlagFileProcessingStatus status =
		static_cast<FlagListManager::FlagFileProcessingStatus>(event.GetInt());
	
	if (status == FlagListManager::FLAG_FILE_PROCESSING_OK) {
		wxCHECK_RET(!this->IsFlagDataReady(),
			_T("ProfileProxy received FLAG_FILE_PROCESSING_OK when flag data is ready."));
		
		const ProxyFlagData* proxyData =
			FlagListManager::GetFlagListManager()->GetProxyFlagData();
		
		wxCHECK_RET(proxyData != NULL,
			_T("ProfileProxy: flag file processing succeeded but proxy data was NULL."));
		
		this->ProcessFlagData(*proxyData);

		this->ProcessFlagLine();
		
		this->isFlagDataReady = true;
		
		GenerateProxyFlagDataReady();
		
		delete proxyData;
	} else if (status == FlagListManager::FLAG_FILE_PROCESSING_RESET) {
		this->Reset();
		this->GenerateProxyReset();	
	} 
}

void ProfileProxy::SetFlag(const wxString& flag, const bool isChecked) {
	wxCHECK_RET(this->IsFlagDataReady(),
		_T("SetFlag() called when proxy flag data isn't ready."));
	wxCHECK_RET(this->flagMap.count(flag) > 0,
		wxString::Format(_T("SetFlag(): given unknown flag %s."), flag.c_str()));
	
	const int flagIndex = this->flagMap.find(flag)->second;
	
	wxASSERT_MSG(VerifyEnabledFlagsForFlag(flag, flagIndex).IsEmpty(),
		VerifyEnabledFlagsForFlag(flag, flagIndex));
	
	if (isChecked) {
		this->enabledFlags[flagIndex] = flag;
	} else {
		this->enabledFlags.erase(flagIndex);
	}
	
	this->WriteFlagLineToProfile();
	CmdLineManager::GenerateCmdLineChanged();
}

std::vector<wxString> ProfileProxy::GetEnabledFlags() const {
	wxCHECK_MSG(this->IsFlagDataReady(), std::vector<wxString>(),
		_T("GetEnabledFlags() called when proxy flag data isn't ready."));
	
	std::vector<wxString> flags;
	
	for (std::map<int,wxString>::const_iterator it = enabledFlags.begin(),
		 end = enabledFlags.end(); it != end; ++it) {
		flags.push_back(it->second);
	}
	
	return flags;
}

wxString ProfileProxy::GetEnabledFlagsString() const {
	wxCHECK_MSG(this->IsFlagDataReady(), wxEmptyString,
		_T("GetEnabledFlagsString() called when proxy flag data isn't ready."));
	
	std::vector<wxString> flags(this->GetEnabledFlags());
	
	wxString flagStr;
	
	for (std::vector<wxString>::const_iterator it = flags.begin(), end = flags.end();
		 it != end; ++it) {
		if (!flagStr.IsEmpty()) {
			flagStr += _T(" ");
		}
		flagStr += *it;		
	}
	
	return flagStr;	
}

// GenerateCustomFlagsChanged() is used to update the custom flags box
// GenerateCmdLineChanged() is used to update the current command line
void ProfileProxy::SetCustomFlags(const wxString& customFlags,
		const bool updateTextCtrl) {
	this->customFlags = customFlags;
	this->WriteFlagLineToProfile();
	if (updateTextCtrl) {
		CmdLineManager::GenerateCustomFlagsChanged();
	}
	CmdLineManager::GenerateCmdLineChanged();
}

bool ProfileProxy::HasLightingPreset() const {
	return ProMan::GetProfileManager()->ProfileExists(PRO_CFG_LIGHTING_PRESET);
}

wxString ProfileProxy::GetLightingPresetName() const {
	wxString presetName;
	ProMan::GetProfileManager()->ProfileRead(
		PRO_CFG_LIGHTING_PRESET, &presetName, wxEmptyString);
	return presetName;
}

void ProfileProxy::SetLightingPreset(const wxString& presetName) {
	ProMan::GetProfileManager()->ProfileWrite(PRO_CFG_LIGHTING_PRESET, presetName);
	CmdLineManager::GenerateCmdLineChanged();
}

void ProfileProxy::CopyPresetToCustomFlags() {
	wxCHECK_RET(this->IsFlagDataReady(),
		_T("CopyPresetToCustomFlags() called when proxy flag data isn't ready."));
	
	wxCHECK_RET(this->HasLightingPreset(),
		_T("CopyPresetToCustomFlags(): profile has no preset entry."));	
	
	wxString presetName(this->GetLightingPresetName());
	
	wxCHECK_RET(!presetName.IsEmpty(),
		_T("CopyPresetToCustomFlags(): preset name is empty."));
	
	const wxString& preset = LightingPresets::PresetNameToPresetString(presetName);
	
	if (!preset.IsEmpty()) {
		this->SetCustomFlags(preset + _T(" ") + this->GetCustomFlags());
	} else {
		wxLogError(
			_T("CopyPresetToCustomFlags(): retrieved preset was empty for name %s"),
				presetName.c_str());
	}
}

bool ProfileProxy::IsProfileInitialized() const {
	bool isProfileInitialized;
	ProMan::GetProfileManager()->ProfileRead(
		PRO_CFG_MAIN_INITIALIZED, &isProfileInitialized, false);
	return isProfileInitialized;
}

void ProfileProxy::FinishProfileInitialization() const {
	wxCHECK_RET(!this->IsProfileInitialized(),
		_T("FinishProfileInitialization(): profile already initialized."));
	
	wxASSERT(!ProMan::GetProfileManager()->ProfileExists(
		PRO_CFG_TC_CURRENT_FLAG_LINE));
	ProMan::GetProfileManager()->ProfileWrite(
		PRO_CFG_TC_CURRENT_FLAG_LINE, wxEmptyString);
	
	wxLogDebug(_T("Autosaving newly initialized profile '%s'."),
		ProMan::GetProfileManager()->GetCurrentName().c_str());
	ProMan::GetProfileManager()->ProfileWrite(PRO_CFG_MAIN_INITIALIZED, true);
	ProMan::GetProfileManager()->SaveCurrentProfile(true);
}

void ProfileProxy::WriteFlagLineToProfile() const {
	wxCHECK_RET(this->IsFlagDataReady(),
		_T("WriteFlagLineToProfile() called when proxy flag data isn't ready."));
	
	wxString newFlagLine(
		this->GetEnabledFlagsString() + _T(" ") + this->GetCustomFlags());
	ProMan::GetProfileManager()->ProfileWrite(
		PRO_CFG_TC_CURRENT_FLAG_LINE, newFlagLine);
}

void ProfileProxy::ProcessFlagData(const ProxyFlagData& data) {
	wxASSERT(!data.IsEmpty());
	
	for (ProxyFlagData::const_iterator it = data.begin(), end = data.end();
		 it != end; ++it) {
		const ProxyFlagDataItem& item = **it;
		
		wxASSERT_MSG(this->flagMap.count(item.GetFlagString()) == 0,
			wxString::Format(_T("ProcessFlagData(): attempted to add flag %s twice."),
				item.GetFlagString().c_str()));
		
		this->flagMap[item.GetFlagString()] = item.GetFlagIndex();
	}
}

void ProfileProxy::ProcessFlagLine() {
	wxASSERT(!this->flagMap.empty());
	wxASSERT(this->enabledFlags.empty());
	wxASSERT(this->customFlags.IsEmpty());
	wxASSERT(!this->IsFlagDataReady());
	
	wxString flagLine;
	
	ProMan::GetProfileManager()->ProfileRead(
		PRO_CFG_TC_CURRENT_FLAG_LINE, &flagLine, wxEmptyString);
	
	wxStringTokenizer tokenizer(flagLine, _T(" "));
	
	while(tokenizer.HasMoreTokens()) {
		wxString flag(tokenizer.GetNextToken());
	
		if (this->flagMap.count(flag) > 0) {
			int flagIndex = this->flagMap.find(flag)->second;
			this->enabledFlags[flagIndex] = flag;
		} else {
			if (!this->customFlags.IsEmpty()) {
				this->customFlags += _T(" ");
			}
			this->customFlags += flag;
		}
	}
}

std::vector<int> ProfileProxy::FindInEnabledFlags(const wxString& flag) const {
	std::vector<int> indices;
	
	for (std::map<int, wxString>::const_iterator it = this->enabledFlags.begin();
		 it != this->enabledFlags.end();
		 ++it) {
		
		const std::pair<int, wxString>& entry = *it;
		
		if (entry.second == flag) {
			indices.push_back(entry.first);
		}
	}
	
	return indices;
}

wxString ProfileProxy::VerifyEnabledFlagsForFlag(
		const wxString& flag, const int flagIndex) const {
	const std::vector<int> indicesFound(this->FindInEnabledFlags(flag));
	
	if (indicesFound.size() > 1) {
		wxString indicesStr;
		
		for (std::vector<int>::const_iterator it = indicesFound.begin() + 1;
			 it != indicesFound.end();
			 ++it) {
			if (!indicesStr.IsEmpty()) {
				indicesStr += _T(", ");
			}
			indicesStr += wxString::Format(_T("%d"), *it);
		}
		
		return wxString::Format(
			_T("Found flag %s at indices %s."), flag.c_str(), indicesStr.c_str());
	}
	
	if (indicesFound.size() == 1 && indicesFound[0] != flagIndex) {
		return wxString::Format(_T("Found flag %s at index %d instead of index %d."),
			flag.c_str(), indicesFound[0], flagIndex);
	}
	
	if ((this->enabledFlags.count(flagIndex) > 1) &&
		(this->enabledFlags.find(flagIndex)->second != flag)) {
		return wxString::Format(_T("Found flag %s at index %d instead of flag %s."),
			flag.c_str(),
			flagIndex,
			this->enabledFlags.find(flagIndex)->second.c_str());
	}
	
	return wxEmptyString; // no errors found
}

void ProfileProxy::Reset() {
	this->flagMap.clear();
	this->enabledFlags.clear();
	this->customFlags.Empty();
	this->isFlagDataReady = false;
}
