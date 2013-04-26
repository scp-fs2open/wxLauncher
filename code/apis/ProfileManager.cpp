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

#include <algorithm>
#include <vector>

#include <wx/wx.h>
#include <wx/fileconf.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/dir.h>

#include "generated/configure_launcher.h"
#include "apis/ProfileManager.h"
#include "apis/PlatformProfileManager.h"
#include "wxLauncherApp.h"
#include "global/ProfileKeys.h"

#include "global/MemoryDebugging.h"

ProMan* ProMan::proman = NULL;
bool ProMan::isInitialized = false;
ProMan::Flags ProMan::flags;

const wxString& ProMan::DEFAULT_PROFILE_NAME = _T("Default");
#define GLOBAL_INI_FILE_NAME _T("global.ini")

///////////// Events

/** EVT_PROFILE_EVENT */
DEFINE_EVENT_TYPE(EVT_PROFILE_CHANGE);

DEFINE_EVENT_TYPE(EVT_CURRENT_PROFILE_CHANGED);

#include <wx/listimpl.cpp> // required magic incatation
WX_DEFINE_LIST(EventHandlers);


void ProMan::GenerateChangeEvent() {
	wxCommandEvent event(EVT_PROFILE_CHANGE, wxID_NONE);
	wxLogDebug(_T("Generating profile change event"));
	EventHandlers::iterator iter = this->eventHandlers.begin();
	while (iter != this->eventHandlers.end()) {
		wxEvtHandler* current = *iter;
		current->AddPendingEvent(event);
		wxLogDebug(_T(" Sent Profile Change event to %p"), current);
		iter++;
	}
}

void ProMan::GenerateCurrentProfileChangedEvent() {
	wxCommandEvent event(EVT_CURRENT_PROFILE_CHANGED, wxID_NONE);
	wxLogDebug(_T("Generating current profile changed event"));
	EventHandlers::iterator iter = this->eventHandlers.begin();
	while (iter != this->eventHandlers.end()) {
		wxEvtHandler* current = *iter;
		current->AddPendingEvent(event);
		wxLogDebug(_T(" Sent current profile changed event to %p"), current);
		iter++;
	} 
}

void ProMan::AddEventHandler(wxEvtHandler *handler) {
	wxASSERT_MSG(eventHandlers.IndexOf(handler) == wxNOT_FOUND,
		wxString::Format(
			_T("ProMan::AddEventHandler(): Handler at %p already registered."),
			handler));
	this->eventHandlers.Append(handler);
}

void ProMan::RemoveEventHandler(wxEvtHandler *handler) {
	wxASSERT_MSG(eventHandlers.IndexOf(handler) != wxNOT_FOUND,
		wxString::Format(
			_T("ProMan::RemoveEventHandler(): Handler at %p not registered."),
			handler));
	this->eventHandlers.DeleteObject(handler);
}

NewsData::NewsData(const wxString& theNews, const wxDateTime& lastDownloadNews)
: theNews(theNews), lastDownloadNews(lastDownloadNews) {
	wxASSERT(!theNews.IsEmpty());
	wxASSERT(lastDownloadNews.IsValid());
}

/** Load a profile from a fully quaified path.  Returns NULL on failure
or a pointer to a wxFileConfig that you must delete when done. */
wxFileConfig* LoadProfileFromFile(const wxFileName &file)
{
	wxFFileInputStream globalProfileInput(file.GetFullPath(),
		(file.FileExists())?_T("rb"):_T("w+b"));
	return new wxFileConfig(globalProfileInput);
}

/** Sets up the profile manager. Must be called on program startup so that
it can intercept global wxWidgets configuation functions. 
\return true when setup was successful, false if proman is not
ready and the program should not continue. */
bool ProMan::Initialize(Flags flags) {
	wxConfigBase::DontCreateOnDemand();

	ProMan::proman = new ProMan();
	ProMan::flags = flags;

	wxFileName file;
	file.Assign(GetProfileStorageFolder(), GLOBAL_INI_FILE_NAME);

	if ( !file.IsOk() ) {
		wxLogError(_T(" '%s' is not valid!"), file.GetFullPath().c_str());
		return false;
	}

	wxLogInfo(_T(" My profiles file is: %s"), file.GetFullPath().c_str());
	if ( !wxFileName::DirExists(file.GetPath())
		&& !wxFileName::Mkdir(file.GetPath(), 0700, wxPATH_MKDIR_FULL ) ) {
		wxLogError(_T(" Unable to make profile folder."));
		return false;
	}

	ProMan::proman->globalProfile = LoadProfileFromFile(file);
	ProMan::proman->LoadNewsMapFromGlobalProfile();

	// fetch all profiles.
	wxArrayString foundProfiles;
	wxDir::GetAllFiles(GetProfileStorageFolder(), &foundProfiles, _T("pro?????.ini"));

	wxLogInfo(_T(" Found %d profile(s)."), foundProfiles.Count());
	for( size_t i = 0; i < foundProfiles.Count(); i++) {
		wxLogDebug(_T("  Opening %s"), foundProfiles[i].c_str());
		wxFFileInputStream instream(foundProfiles[i]);
		wxFileConfig *config = new wxFileConfig(instream);
		
		wxString name;
		config->Read(PRO_CFG_MAIN_NAME, &name, wxString::Format(_T("Profile %05d"), i));

		ProMan::proman->profiles[name] = config;
		wxLogDebug(_T("  Opened profile named: %s"), name.c_str());
	}

	wxString currentProfile;
	ProMan::proman->globalProfile->Read(
		GBL_CFG_MAIN_LASTPROFILE, &currentProfile, ProMan::DEFAULT_PROFILE_NAME);
	
	wxLogDebug(_T(" Searching for profile: %s"), currentProfile.c_str());
	if ( ProMan::proman->profiles.find(currentProfile)
	== ProMan::proman->profiles.end() ) {
		// lastprofile does not exist
		wxLogDebug(_T(" lastprofile '%s' does not exist!"), currentProfile.c_str());
		if ( ProMan::proman->profiles.find(ProMan::DEFAULT_PROFILE_NAME)
		== ProMan::proman->profiles.end() ) {
			// default profile also does not exist.
			// Means this is likely the first run this system
			// Create a default profile
			wxLogInfo(_T(" Default profile does not exist! Creating..."));
			ProMan::proman->CreateNewProfile(ProMan::DEFAULT_PROFILE_NAME);
			// FIXME figure out how PullProfile is corrupting default profile (esp. on Windows)
//			wxLogInfo(_T(" Priming profile..."));
//			PullProfile(ProMan::proman->profiles[ProMan::DEFAULT_PROFILE_NAME]);
		}
		wxLogInfo(_T(" Resetting lastprofile to Default."));
		// Do not ignore updating last profile here because this is fixing bad data
		ProMan::proman->globalProfile->Write(GBL_CFG_MAIN_LASTPROFILE, ProMan::DEFAULT_PROFILE_NAME);
		wxFFileOutputStream globalProfileOutput(file.GetFullPath());
		ProMan::proman->globalProfile->Save(globalProfileOutput);
		currentProfile = ProMan::DEFAULT_PROFILE_NAME;
	}

	wxLogDebug(_T(" Making '%s' the application profile"), currentProfile.c_str());
	if ( !ProMan::proman->SwitchTo(currentProfile) ) {
		wxLogError(_T("Unable to set current profile to '%s'"), currentProfile.c_str());
		return false;
	}

	ProMan::isInitialized = true;
	wxLogDebug(_T(" Profile Manager is set up"));
	return true;
}

/** clean up the memory that the manager is using. */
bool ProMan::DeInitialize() {
	if ( ProMan::isInitialized ) {
		ProMan::isInitialized = false;

		ProMan::proman->SaveProfilesBeforeExiting();
		
		delete ProMan::proman;
		ProMan::proman = NULL;

		// Set the wxWidgets default FileConfig to null
		// so that it doesn't try to delete it again when
		// we shutdown
		wxFileConfig::Set((wxConfigBase*)NULL);

		return true;
	} else {
		return false;
	}
}


ProMan* ProMan::GetProfileManager() {
	if ( ProMan::isInitialized ) {
		return ProMan::proman;
	} else {
		return NULL;
	}
}

/** Private constructor.  Just makes instance variables safe.  Call Initialize()
to setup class, then call GetProfileManager() to get a pointer to the instance.
*/
ProMan::ProMan() {
	this->globalProfile = NULL;
	this->isAutoSaving = true;
	this->currentProfile = NULL;
	
	this->privateCopyFilename = wxFileName::CreateTempFileName(_T("wxLtest"));
	wxFFileInputStream instream(this->privateCopyFilename);
	this->privateCopy = new wxFileConfig(instream);
}

/** Destructor. */
ProMan::~ProMan() {
	// don't leak the wxFileConfigs
	ProfileMap::iterator iter = this->profiles.begin();
	while ( iter != this->profiles.end() ) {
		delete iter->second;
		iter++;
	}
	
	delete this->privateCopy;
	privateCopy = NULL;
	::wxRemoveFile(this->privateCopyFilename);
}

/** Saves changes to profiles according to autosave profiles checkbox. */
void ProMan::SaveProfilesBeforeExiting() {
	if ( this->globalProfile != NULL ) {
		wxLogInfo(_T("saving global profile before exiting."));
		SaveNewsMapToGlobalProfile();
		
		wxFileName file;
		file.Assign(GetProfileStorageFolder(), GLOBAL_INI_FILE_NAME);
		wxFFileOutputStream globalProfileOutput(file.GetFullPath());
		this->globalProfile->Save(globalProfileOutput);
		
		delete this->globalProfile;
	} else {
		wxLogWarning(_T("global profile is null, cannot save it"));
	}
	
	if (this->HasUnsavedChanges()) {
		if (this->isAutoSaving) {
			wxLogInfo(_T("autosaving profile %s before exiting"),
				this->GetCurrentName().c_str()),
			this->SaveCurrentProfile();
		} else {
			int response = wxMessageBox(
				GetSaveDialogMessageText(ProMan::ON_EXIT, this->GetCurrentName()),
				GetSaveDialogCaptionText(ProMan::ON_EXIT, this->GetCurrentName()),
				wxYES_NO);
			if ( response == wxYES ) {
				wxLogInfo(_T("saving profile %s before exiting"),
					this->GetCurrentName().c_str());
				this->SaveCurrentProfile();
			} else {
				wxLogWarning(_T("exiting without saving changes to profile %s"),
					this->GetCurrentName().c_str());
			}
		}
	} else {
		wxLogInfo(_T("Current profile %s has no unsaved changes. Exiting."),
			this->GetCurrentName().c_str());
	}
}

void ProMan::LoadNewsMapFromGlobalProfile() {
	wxASSERT(newsMap.empty());
	
	globalProfile->SetPath(GBL_CFG_NET_FOLDER);
	
	// inspired by CopyConfig()
	wxString groupName;
	long groupIndex;
	bool groupKeepGoing;
	
	wxString theNews;
	wxString lastDownloadNewsStr;
	wxDateTime lastDownloadNews;
	
	groupKeepGoing = globalProfile->GetFirstGroup(groupName, groupIndex);
	
	while (groupKeepGoing) {
		globalProfile->SetPath(groupName);
		
		if (globalProfile->Read(GBL_CFG_NET_THE_NEWS, theNews) &&
			(globalProfile->Read(GBL_CFG_NET_NEWS_LAST_TIME, lastDownloadNewsStr))) {
			if ((!theNews.IsEmpty()) &&
				(NULL != lastDownloadNews.ParseFormat(
					lastDownloadNewsStr, NEWS_LAST_TIME_FORMAT))) {
				newsMap[groupName] = NewsData(theNews, lastDownloadNews);
				
				wxLogDebug(_T("Created news map entry for source %s"),
					groupName.c_str());
			}
		}
		
		globalProfile->SetPath(_T(".."));
		
		groupKeepGoing = globalProfile->GetNextGroup(groupName, groupIndex);
	}
	
	globalProfile->SetPath(_T("/"));
}

void ProMan::SaveNewsMapToGlobalProfile() {
	globalProfile->SetPath(GBL_CFG_NET_FOLDER);
	
	for (NewsMap::const_iterator it = newsMap.begin(), end = newsMap.end();
		 it != end; ++it) {
		const wxString& newsSource = it->first;
		const NewsData& newsData = it->second;
		
		globalProfile->SetPath(newsSource);
		globalProfile->Write(GBL_CFG_NET_THE_NEWS, newsData.theNews);
		globalProfile->Write(GBL_CFG_NET_NEWS_LAST_TIME,
			newsData.lastDownloadNews.Format(NEWS_LAST_TIME_FORMAT));
		globalProfile->SetPath(_T(".."));
	}
	
	globalProfile->SetPath(_T("/"));
}

/** Resets the private copy so that it contains a copy
 of the current profile's contents. */
void ProMan::ResetPrivateCopy() {
	wxCHECK_RET(this->currentProfile != NULL, _T("ResetPrivateCopy called with null current profile!"));
	ClearConfig(*(this->privateCopy));
	CopyConfig(*(this->currentProfile), *(this->privateCopy));
}

/** Creates a new profile including the directory for it to go in, the entry
in the profiles map. Returns true if creation was successful. */
bool ProMan::CreateNewProfile(wxString newName) {
	wxFileName profile;
	profile.Assign(
		GetProfileStorageFolder(),
		this->GenerateNewProfileFileName());

	wxLogInfo(_T("New profile will be written to %s"), profile.GetFullPath().c_str());
	
	wxASSERT_MSG( profile.IsOk(), _T("Profile filename is invalid"));

	if ( !wxFileName::DirExists(profile.GetPath())
		&& !wxFileName::Mkdir( profile.GetPath(), wxPATH_MKDIR_FULL) ) {
		wxLogWarning(_T("  Unable to create profile folder: %s"), profile.GetPath().c_str());
		return false;
	}

	wxFFileInputStream configInput(profile.GetFullPath(), _T("w+b"));
	wxFileConfig* config = new wxFileConfig(configInput);
	config->Write(PRO_CFG_MAIN_NAME, newName);
	config->Write(PRO_CFG_MAIN_FILENAME, profile.GetFullName());
	wxFFileOutputStream configOutput(profile.GetFullPath());
	config->Save(configOutput);

	this->profiles[newName] = config;
	return true;
}

/** Generates a filename for a new profile, where the name is of the form
 pro#####.ini with ##### being the least 5-digit number not yet taken. */
wxString ProMan::GenerateNewProfileFileName() {
	wxArrayString profileFiles;
	wxDir::GetAllFiles(GetProfileStorageFolder(), &profileFiles, _T("pro*.ini"), wxDIR_FILES);
	
	long l;
	
	std::vector<long> proNums;
	
	for (int i = 0, n = profileFiles.GetCount(); i < n; ++i) {
		profileFiles[i] = profileFiles[i].AfterLast(_T('o')).BeforeLast(_T('.'));
		profileFiles[i].ToLong(&l);
		proNums.push_back(l);
	}
	
	// just in case they're not already in sorted order
	sort(proNums.begin(), proNums.end());
	
	proNums.insert(proNums.begin(), -1); // dummy value that allows for easily checking first element
	
	long proIndex = 0;
	
	int i;
	const int n = proNums.size();
	
	for (i = 1; i < n; ++i) {
		if (proNums[i] - proNums[i-1] > 1) { // if there's a gap in the numbering, try to fill it
			proIndex = proNums[i-1] + 1;
			break;
		}
	}
	
	if (i == n) { // there are no gaps, so increment from the last element
		proIndex = proNums[n-1] + 1;
	}

	wxLogDebug(_T("new profile number: %ld"), proIndex);
	
	wxASSERT(proIndex <= 99999); // the maximum possible index given a 5-digit number
	
	return wxString::Format(_T("pro%05d.ini"), static_cast<int>(proIndex));
}

// global profile access functions

/** Tests whether the key strName is in the global profile. */
bool ProMan::GlobalExists(wxString& strName) const {
	if (this->globalProfile == NULL) {
		wxLogWarning(_T("attempt to check existence of key %s in null global profile"),
			strName.c_str());
		return false;
	} else {
		return this->globalProfile->Exists(strName);
	}
}

/** Tests whether the key strName is in the global profile */
bool ProMan::GlobalExists(const wxChar* strName) const {
	if (this->globalProfile == NULL) {
		wxLogWarning(_T("attempt to check existence of key %s in null global profile"),
			strName);
		return false;
	} else {
		return this->globalProfile->Exists(strName);
	}
}

/** Reads a bool from the global profile. Returns true on success. */
bool ProMan::GlobalRead(const wxString& key, bool* b) const {
	if (this->globalProfile == NULL) {
		wxLogWarning(
			_T("attempt to read bool for key %s from null global profile"),
			key.c_str());
		return false;
	} else {
		return this->globalProfile->Read(key, b);
	}
}

/** Reads a bool from the global profile,
 using the default value if the key is not present.
 If the entry is not present in the global profile and writeBackIfAbsent is set to true,
 then the default value is written back to the global profile.
 Returns true on success. */
bool ProMan::GlobalRead(const wxString& key, bool* b, bool defaultVal, bool writeBackIfAbsent) {
	if (this->globalProfile == NULL) {
		wxLogWarning(
			_T("attempt to read bool for key %s with default value %s from null global profile"),
			key.c_str(), defaultVal ? _T("true") : _T("false"));
		return false;
	} else {
		bool readSuccess = this->globalProfile->Read(key, b, defaultVal);
		if (!readSuccess && writeBackIfAbsent) {
			wxLogDebug(_T("entry %s in global profile is absent. writing default value %s to it."),
				key.c_str(), defaultVal ? _T("true") : _T("false"));
			this->globalProfile->Write(key, defaultVal);
		}
		return readSuccess;
	}
}

/** Reads a string from the global profile. Returns true on success. */
bool ProMan::GlobalRead(const wxString& key, wxString* str) const {
	if (this->globalProfile == NULL) {
		wxLogWarning(
			_T("attempt to read string for key %s with from null global profile"),
			key.c_str());
		return false;
	} else {
		return this->globalProfile->Read(key, str);
	}
}

/** Reads a string from the global profile,
 using the default value if the key is not present.
 If the entry is not present in the global profile and writeBackIfAbsent is set to true,
 then the default value is written back to the global profile.
Returns true on success. */
bool ProMan::GlobalRead(const wxString& key, wxString* str, const wxString& defaultVal, bool writeBackIfAbsent) {
	if (this->globalProfile == NULL) {
		wxLogWarning(
			_T("attempt to read string for key %s with default value %s from null global profile"),
			key.c_str(), defaultVal.c_str());
		return false;
	} else {
		bool readSuccess = this->globalProfile->Read(key, str, defaultVal);
		if (!readSuccess && writeBackIfAbsent) {
			wxLogDebug(_T("entry %s in global profile is absent. writing default value %s to it."),
				key.c_str(), defaultVal.c_str());
			this->globalProfile->Write(key, defaultVal);
		}
		return readSuccess;
	}
}

/** Reads a long from the global profile,
 using the default value if the key is not present.
 If the entry is not present in the global profile and writeBackIfAbsent is set to true,
 then the default value is written back to the global profile.
 Returns true on success. */
bool ProMan::GlobalRead(const wxString& key, long* l, long defaultVal, bool writeBackIfAbsent) {
	if (this->globalProfile == NULL) {
		wxLogWarning(
			_T("attempt to read long for key %s with default value %ld from null global profile"),
			key.c_str(), defaultVal);
		return false;
	} else {
		bool readSuccess = this->globalProfile->Read(key, l, defaultVal);
		if (!readSuccess && writeBackIfAbsent) {
			wxLogDebug(_T("entry %s in global profile is absent. writing default value %ld to it."),
				key.c_str(), defaultVal);
			this->globalProfile->Write(key, defaultVal);
		}
		return readSuccess;
	}
}

/** Writes a string for the given key to the global profile.
 Returns true on success. */
bool ProMan::GlobalWrite(const wxString& key, const wxString& value) {
	if (this->globalProfile == NULL) {
		wxLogWarning(_T("attempt to write %s to %s in null global profile"),
			value.c_str(), key.c_str());
		return false;
	} else {
		return this->globalProfile->Write(key, value);
	}
}

/** Writes a string for the given key to the global profile.
 Returns true on success. */
bool ProMan::GlobalWrite(const wxString& key, const wxChar* value) {
	if (this->globalProfile == NULL) {
		wxLogWarning(_T("attempt to write %s to %s in null global profile"),
					 value, key.c_str());
		return false;
	} else {
		return this->globalProfile->Write(key, value);
	}
}

/** Writes a long for the given key to the global profile.
 Returns true on success. */
bool ProMan::GlobalWrite(const wxString& key, long value) {
	if (this->globalProfile == NULL) {
		wxLogWarning(_T("attempt to write %ld to %s in null global profile"),
			value, key.c_str());
		return false;
	} else {
		return this->globalProfile->Write(key, value);
	}
}

/** Writes a bool for the given key to the global profile.
 Returns true on success. */
bool ProMan::GlobalWrite(const wxString& key, bool value) {
	if (this->globalProfile == NULL) {
		wxLogWarning(_T("attempt to write %s to %s in null global profile"),
			value ? _T("true") : _T("false"), key.c_str());
		return false;
	} else {
		return this->globalProfile->Write(key, value);
	}
}

// current profile access functions

/** Tests whether the key strName is in the current profile. */
bool ProMan::ProfileExists(wxString& strName) const {
	if (this->currentProfile == NULL) {
		wxLogWarning(_T("attempt to check existence of key %s in null current profile"),
			strName.c_str());
		return false;
	} else {
		return this->currentProfile->Exists(strName);
	}
}

/** Tests whether the key strName is in the current profile. */
bool ProMan::ProfileExists(const wxChar* strName) const {
	if (this->currentProfile == NULL) {
		wxLogWarning(_T("attempt to check existence of key %s in null current profile"),
			strName);
		return false;
	} else {
		return this->currentProfile->Exists(strName);
	}
}

/** Reads a bool from the current profile. Returns true on success. */
bool ProMan::ProfileRead(const wxString& key, bool* b) const {
	if (this->currentProfile == NULL) {
		wxLogWarning(
			_T("attempt to read bool for key %s from null current profile"),
			key.c_str());
		return false;
	} else {
		return this->currentProfile->Read(key, b);
	}
}

/** Reads a bool from the current profile,
 using the default value if the key is not present.
 If the entry is not present in the current profile and writeBackIfAbsent is set to true,
 then the default value is written back to the current profile.
 Returns true on success. */
bool ProMan::ProfileRead(const wxString& key, bool* b, bool defaultVal, bool writeBackIfAbsent) {
	if (this->currentProfile == NULL) {
		wxLogWarning(
			_T("attempt to read bool for key %s with default value %s from null current profile"),
			key.c_str(), defaultVal ? _T("true") : _T("false"));
		return false;
	} else {
		bool readSuccess = this->currentProfile->Read(key, b, defaultVal);
		if (!readSuccess && writeBackIfAbsent) {
			wxLogDebug(_T("entry %s in current profile is absent. writing default value %s to it."),
				key.c_str(), defaultVal ? _T("true") : _T("false"));
			this->currentProfile->Write(key, defaultVal);
		}
		return readSuccess;
	}
}

/** Reads a string from the current profile. Returns true on success. */
bool ProMan::ProfileRead(const wxString& key, wxString* str) const {
	if (this->currentProfile == NULL) {
		wxLogWarning(
			_T("attempt to read string for key %s with from null current profile"),
			key.c_str());
		return false;
	} else {
		return this->currentProfile->Read(key, str);
	}
}

/** Reads a string from the current profile,
 using the default value if the key is not present.
 If the entry is not present in the current profile and writeBackIfAbsent is set to true,
 then the default value is written back to the current profile.
 Returns true on success. */
bool ProMan::ProfileRead(const wxString& key, wxString* str, const wxString& defaultVal, bool writeBackIfAbsent) {
	if (this->currentProfile == NULL) {
		wxLogWarning(
			_T("attempt to read string for key %s with default value %s from null current profile"),
			key.c_str(), defaultVal.c_str());
		return false;
	} else {
		bool readSuccess = this->currentProfile->Read(key, str, defaultVal);
		if (!readSuccess && writeBackIfAbsent) {
			wxLogDebug(_T("entry %s in current profile is absent. writing default value %s to it."),
				key.c_str(), defaultVal.c_str());
			this->currentProfile->Write(key, defaultVal);
		}
		return readSuccess;
	}
}

/** Reads a long from the current profile. Returns true on success. */
bool ProMan::ProfileRead(const wxString& key, long* l) const {
	if (this->currentProfile == NULL) {
		wxLogWarning(
			_T("attempt to read long for key %s from null current profile"),
			key.c_str());
		return false;
	} else {
		return this->currentProfile->Read(key, l);
	}
}

/** Reads a long from the current profile,
 using the default value if the key is not present.
 If the entry is not present in the current profile and writeBackIfAbsent is set to true,
 then the default value is written back to the current profile.
 Returns true on success. */
bool ProMan::ProfileRead(const wxString& key, long* l, long defaultVal, bool writeBackIfAbsent) {
	if (this->currentProfile == NULL) {
		wxLogWarning(
			_T("attempt to read long for key %s with default value %ld from null current profile"),
			key.c_str(), defaultVal);
		return false;
	} else {
		bool readSuccess = this->currentProfile->Read(key, l, defaultVal);
		if (!readSuccess && writeBackIfAbsent) {
			wxLogDebug(_T("entry %s in current profile is absent. writing default value %ld to it."),
				key.c_str(), defaultVal);
			this->currentProfile->Write(key, defaultVal);
		}
		return readSuccess;
	}
}

/** Writes a string for the given key to the current profile.
 Returns true on success. */
bool ProMan::ProfileWrite(const wxString& key, const wxString& value) {
	if (this->currentProfile == NULL) {
		wxLogWarning(_T("attempt to write %s to %s in null current profile"),
			value.c_str(), key.c_str());
		return false;
	} else {
		if (!this->currentProfile->Exists(key)) {
			wxLogDebug(_T("adding entry %s with value %s to current profile"),
				key.c_str(), value.c_str());
		} else {
			wxString oldValue;
			if (this->currentProfile->Read(key, &oldValue) && (value != oldValue)) {
				wxLogDebug(_T("replacing old value %s with value %s for current profile entry %s"),
					oldValue.c_str(), value.c_str(), key.c_str());
			}
		}
		return this->currentProfile->Write(key, value);
	}
}

/** Writes a string for the given key to the current profile.
 Returns true on success. */
bool ProMan::ProfileWrite(const wxString& key, const wxChar* value) {
	if (this->currentProfile == NULL) {
		wxLogWarning(_T("attempt to write %s to %s in null current profile"),
					 value, key.c_str());
		return false;
	} else {
		if (!this->currentProfile->Exists(key)) {
			wxLogDebug(_T("adding entry %s with value %s to current profile"),
					   key.c_str(), value);
		} else {
			wxString oldValue;
			if (this->currentProfile->Read(key, &oldValue) && (value != oldValue)) {
				wxLogDebug(_T("replacing old value %s with value %s for current profile entry %s"),
						   oldValue.c_str(), value, key.c_str());
			}
		}
		return this->currentProfile->Write(key, value);
	}
}

/** Writes a long for the given key to the current profile.
 Returns true on success. */
bool ProMan::ProfileWrite(const wxString& key, long value) {
	if (this->currentProfile == NULL) {
		wxLogWarning(_T("attempt to write %ld to %s in null current profile"),
			value, key.c_str());
		return false;
	} else {
		if (!this->currentProfile->Exists(key)) {
			wxLogDebug(_T("adding entry %s with value %ld to current profile"),
				key.c_str(), value);
		} else {
			long oldValue;
			if (this->currentProfile->Read(key, &oldValue) && (value != oldValue)) {
				wxLogDebug(_T("replacing old value %ld with value %ld for current profile entry %s"),
					oldValue, value, key.c_str());
			}
		}
		return this->currentProfile->Write(key, value);
	}
}

/** Writes a bool for the given key to the current profile.
 Returns true on success. */
bool ProMan::ProfileWrite(const wxString& key, bool value) {
	if (this->currentProfile == NULL) {
		wxLogWarning(_T("attempt to write %s to %s in null current profile"),
			value ? _T("true") : _T("false"), key.c_str());
		return false;
	} else {
		if (!this->currentProfile->Exists(key)) {
			wxLogDebug(_T("adding entry %s with value %s to current profile"),
				key.c_str(), value ? _T("true") : _T("false"));
		} else {
			bool oldValue;
			if (this->currentProfile->Read(key, &oldValue) && (value != oldValue)) {
				wxLogDebug(_T("replacing old value %s with value %s for current profile entry %s"),
					oldValue ? _T("true") : _T("false"),
					value ? _T("true") : _T("false"),
					key.c_str());
			}
		}
		
		return this->currentProfile->Write(key, value);
	}
}

/** Deletes an entry from the current profile,
 deleting the group if the entry was the only one in the group
 and the second parameter is true. */
bool ProMan::ProfileDeleteEntry(const wxString& key, bool bDeleteGroupIfEmpty) {
	if (this->currentProfile == NULL) {
		wxLogWarning(_T("attempt to delete entry %s in null current profile"),
			key.c_str());
		return false;
	} else {
		if (this-currentProfile->Exists(key)) {
			wxLogDebug(_T("deleting key %s in profile"),
				key.c_str());
		}
		return this->currentProfile->DeleteEntry(key, bDeleteGroupIfEmpty);
	}
}

const NewsData* ProMan::NewsRead(const wxString& newsSource) const {
	wxCHECK_MSG(!newsSource.IsEmpty(), NULL, _T("NewsRead: newsSource is empty!"));
	
	NewsMap::const_iterator it = newsMap.find(newsSource);
	
	if (it == newsMap.end()) {
		return NULL;
	} else {
		return &(it->second);
	}
}

void ProMan::NewsWrite(const wxString& newsSource, const NewsData& data) {
	wxCHECK_RET(!newsSource.IsEmpty(), _T("NewsWrite: newsSource is empty!"));
	wxCHECK_RET(data.IsValid(), _T("NewsWrite: data is not valid!"));
	
	newsMap[newsSource] = data;
}

/** Returns the text to use in the "save changes?" dialog's caption (window title) */
const wxString ProMan::GetSaveDialogCaptionText(ProMan::SaveDialogContext context,
												const wxString& profileName) {
	// currently, the same text is used on all platforms, although it doesn't have to be that way
	// NOTE: don't attempt to collapse the cases by removing break statements! it's asking for trouble
	switch (context) {
		case ON_PROFILE_SWITCH:
#if PROFILE_DEBUGGING
			wxLogDebug(_T("contents of private copy at save prompt on profile switch:"));
			LogConfigContents(*ProMan::proman->privateCopy);
			wxLogDebug(_T("contents of current profile at save prompt on profile switch:"));
			LogConfigContents(*ProMan::proman->currentProfile);
#endif
			return _T("Save changes to current profile?");
			break;

		case ON_PROFILE_CREATE:
#if PROFILE_DEBUGGING
			wxLogDebug(_T("contents of private copy at save prompt on profile create:"));
			LogConfigContents(*ProMan::proman->privateCopy);
			wxLogDebug(_T("contents of current profile at save prompt on profile create:"));
			LogConfigContents(*ProMan::proman->currentProfile);
#endif
			return _T("Save changes to current profile?");
			break;

		case ON_EXIT:
#if PROFILE_DEBUGGING
			wxLogDebug(_T("contents of private copy at save prompt on exit:"));
			LogConfigContents(*ProMan::proman->privateCopy);
			wxLogDebug(_T("contents of current profile at save prompt on exit:"));
			LogConfigContents(*ProMan::proman->currentProfile);
#endif
			return _T("Save changes to current profile?");
			break;

		default:
			wxCHECK_MSG(false, wxEmptyString, _T("ProMan::GetSaveDialogCaptionText: provided context is invalid"));
			break;
	}
}

/** Returns the text to use in the "save changes?" dialog's message (text area) */
const wxString ProMan::GetSaveDialogMessageText(ProMan::SaveDialogContext context,
												const wxString& profileName) {
	switch (context) {
		case ON_PROFILE_SWITCH:
			return wxString::Format(_T("Save changes to profile '%s' before switching profiles?"),
				profileName.c_str());
			break;
			
		case ON_PROFILE_CREATE:
			return wxString::Format(_T("Save changes to profile '%s' before creating a new profile?"),
				profileName.c_str());
			break;
			
		case ON_EXIT:
#if IS_WIN32
			return wxString::Format(_T("Save changes to profile '%s' before exiting?"),
				profileName.c_str());
#else
			return wxString::Format(_T("Save changes to profile '%s' before quitting?"),
				profileName.c_str());
#endif
			break;
			
		default:
			wxCHECK_MSG(false, wxEmptyString, _T("ProMan::GetSaveDialogMessageText: provided context is invalid"));
			break;
	}
}

/** Returns true if the named profile exists, false otherwise. */
bool ProMan::DoesProfileExist(wxString name) {
	/* Item exists if the returned value from find() does not equal 
	the value of .end().  As per the HashMap docs. */
	return (this->profiles.find(name) != this->profiles.end());
}

/** Returns an wxArrayString of all of the profile names. */
wxArrayString ProMan::GetAllProfileNames() {
	wxArrayString out(this->profiles.size());

	ProfileMap::iterator iter = this->profiles.begin();
	do {
		out.Add(iter->first);
		iter++;
	} while (iter != this->profiles.end());

	return out;
}

void SaveProfileToDisk(wxFileConfig* toSave, const wxString& name)
{
	wxString profileFilename;
	if ( !toSave->Read(PRO_CFG_MAIN_FILENAME, &profileFilename) ) {
		wxLogError(_T("Profile '%s' does not have a file name. Cannot save it."),
			name.c_str());
		// FIXME maybe make a new file and save the current profile there
	} else {
		wxFileName file;
		file.Assign(GetProfileStorageFolder(), profileFilename);
		wxASSERT( file.IsOk() );
		wxFFileOutputStream configOutput(file.GetFullPath());
		toSave->Save(configOutput);
		wxLogDebug(_T("Profile '%s' saved to '%s'"),
			name.c_str(), file.GetFullPath().c_str());
	}
}

/** Saves the current profile to disk, regardless of whether it has unsaved changes.
 Does not affect the global profile or any other profile. */
void ProMan::SaveCurrentProfile(bool quiet) {
	wxConfigBase* configbase = wxFileConfig::Get(false);
	if ( configbase == NULL ) {
		wxLogError(_T("There is no global file config."));
		return;
	}
	wxFileConfig* config = dynamic_cast<wxFileConfig*>(configbase);
	if ( config != NULL ) {
		SaveProfileToDisk(config, this->currentProfileName.c_str());
		this->ResetPrivateCopy();
		if (!quiet) {
			wxLogStatus(_T("Profile '%s' saved"), this->currentProfileName.c_str());				
		}
		wxLogDebug(_T("Current config%s saved."),
			quiet ? _T(" quietly") : wxEmptyString);
	} else {
		wxLogError(_T("Configbase is not a wxFileConfig."));
	}
}

/** Reverts any unsaved changes to the current profile. */
void ProMan::RevertCurrentProfile() {
	ClearConfig(*(this->currentProfile));
	CopyConfig(*(this->privateCopy), *(this->currentProfile));
}

bool ProMan::HasUnsavedChanges() {
	return !AreConfigsEqual(*(this->currentProfile), *(this->privateCopy));
}

wxString ProMan::GetCurrentName() {
	return this->currentProfileName;
}

/** Implements the literal switch to another profile.
Does not cause prompts and may destroy data if autosave is not on.
*/
bool ProMan::SwitchTo(wxString name) {
	if ( this->profiles.find(name) == this->profiles.end() ) {
		return false;
	} else {
		if (this->currentProfile != NULL && this->HasUnsavedChanges()) {
			if (this->isAutoSaving) {
				wxLogDebug(_T("Auto saving current profile %s before switching profiles"),
					this->currentProfileName.c_str());
				this->SaveCurrentProfile();
			} else {
				wxLogDebug(_T("Reverting unsaved changes to current profile %s before switching"),
					this->currentProfileName.c_str());
				this->RevertCurrentProfile();
			}
		}
		this->currentProfileName = name;
		this->currentProfile = this->profiles.find(name)->second;
		wxFileConfig::Set(this->currentProfile);
		if ( !(ProMan::flags & NoUpdateLastProfile) )
			this->globalProfile->Write(GBL_CFG_MAIN_LASTPROFILE, name);
		this->ResetPrivateCopy();
//		TestConfigFunctions(*this->currentProfile); // remove after testing on all platforms
		this->GenerateCurrentProfileChangedEvent();
		return true;
	}
}

/** Creates a profile from a fullyqualified path. */
bool ProMan::CreateProfile(const wxString& newProfileName, const wxFileName& sourceFile)
{
	wxLogDebug(_T("Importing profile in '%s' as '%s'"),
		sourceFile.GetFullPath().c_str(), newProfileName.c_str());
	wxFileConfig *sourceConfig = LoadProfileFromFile(sourceFile);

	return this->CreateProfile(newProfileName, sourceConfig);
}

/** Creates a profile named newProfileName from the contents of sourceConfig.
If sourceConfig is NULL then the newProfile is created blank. */
bool ProMan::CreateProfile(const wxString& newProfileName, const wxFileConfig *sourceConfig)
{
	if (this->DoesProfileExist(newProfileName)) {
		wxLogWarning(_("New profile '%s' already exists!"), newProfileName.c_str());
		return false;
	}

	if (!this->CreateNewProfile(newProfileName)) {
		wxLogWarning(_T("New profile creation failed."));
		return false;
	}

	if (sourceConfig != NULL)
	{
		/* We just created this profile it had better exist */
		wxFileConfig* newProfileConfig = this->profiles[newProfileName];
		wxCHECK_MSG(newProfileConfig != NULL, false, _T("Create returned true but did not create profile"));

#if PROFILE_DEBUGGING
		wxLogDebug(_T("contents of new profile '%s' before clone:"), newProfileName.c_str());
		LogConfigContents(*newProfileConfig);
#endif

		CopyConfig(*sourceConfig, *newProfileConfig, false);
		SaveProfileToDisk(newProfileConfig, newProfileName);

#if PROFILE_DEBUGGING
		wxLogDebug(_T("contents of new profile '%s' after clone:"), newProfileName.c_str());
		LogConfigContents(*newProfileConfig);
		wxLogDebug(_T("contents of cloned from profile '%s' after clone:"), cloneFromProfileName.c_str());
		LogConfigContents(*cloneFromProfileConfig);
#endif

	}

	this->GenerateChangeEvent();
	return true;
}

/** Creates a profile by name. If the second argument is non-empty and is the name
 of an existing profile, its settings/flags will be copied over to the new profile.
 Returns true on success. */
bool ProMan::CreateProfile(const wxString& newProfileName, const wxString& cloneFromProfileName) {
	wxFileConfig *cloneSource = NULL;

	if (cloneFromProfileName.IsEmpty())
	{
		wxLogDebug(_T("Creating blank profile '%s'"), newProfileName.c_str());
	}
	else
	{
		wxLogDebug(_T("Cloning original profile (%s) to '%s'"),
			cloneFromProfileName.c_str(), newProfileName.c_str());
		if (!this->DoesProfileExist(cloneFromProfileName))
		{
			wxLogWarning(_("Profile to clone from '%s' does not exist!"), cloneFromProfileName.c_str());
			return false;
		}
		cloneSource = this->profiles[cloneFromProfileName];
		wxCHECK_MSG( cloneSource != NULL, false,
			wxString::Format(_T("Cannot find profile '%s' from which to clone"),
				cloneFromProfileName.c_str()) );
	}

	return this->CreateProfile(newProfileName, cloneSource);
}

bool ProMan::DeleteProfile(wxString name) {
	wxLogDebug(_T("Deleting profile: %s"), name.c_str());
	if ( name == ProMan::DEFAULT_PROFILE_NAME ) {
		wxLogWarning(_("Cannot delete 'Default' profile."));
		return false;
	}
	if ( name == this->currentProfileName ) {
		wxLogInfo(
			_T("Deleting current profile. Switching to '%s' profile"),
			ProMan::DEFAULT_PROFILE_NAME.c_str());
		this->SwitchTo(ProMan::DEFAULT_PROFILE_NAME);
	}
	if ( this->DoesProfileExist(name) ) {
		wxLogDebug(_T(" Profile exists"));
		wxFileConfig* config = this->profiles[name];

		wxString filename;
		if ( !config->Read(PRO_CFG_MAIN_FILENAME, &filename) ) {
			wxLogWarning(_T("Unable to get filename to delete %s"), name.c_str());
			return false;
		}

		wxFileName file;
		file.Assign(GetProfileStorageFolder(), filename);

		if ( file.FileExists() ) {
			wxLogDebug(_T(" Backing file exists"));
			if ( wxRemoveFile(file.GetFullPath()) ) {
				this->profiles.erase(this->profiles.find(name));
				delete config;
				
				wxLogMessage(_("Profile '%s' deleted."), name.c_str());
				this->GenerateChangeEvent();
				return true;
			} else {
				wxLogWarning(_("Unable to delete file for profile '%s'"), name.c_str());
			}
		} else {
			wxLogWarning(_("Backing file (%s) for profile '%s' does not exist"), file.GetFullPath().c_str(), name.c_str());
		}
	} else {
		wxLogWarning(_("Profile %s does not exist. Cannot delete."), name.c_str());
	}
	return false;
}

// the config manipulation functions are adapted from CopyEntriesRecursive and CopyEntry
// from http://audacity.googlecode.com/svn/audacity-src/trunk/src/Prefs.cpp SVN r11245
/** copies the contents of one wxConfigBase to another wxConfigBase.
 The wxWindows group is not copied, if it exists. If includeMainGroup is true, then
 the contents of the "main" group (profile-specific information, such as name and filename)
 are copied as well, otherwise they are not copied. */
void ProMan::CopyConfig(const wxConfigBase& in_src,
						wxConfigBase& dest,
						const bool includeMainGroup,
						const wxString path) {
	wxString entryName;
	long entryIndex;
	bool entryKeepGoing;

	/* BUGNOTE - This is possibly not safe, but because the following code
	uses non const functions and restores what it does mutate this
	should be safe. */
	wxConfigBase& src = const_cast<wxConfigBase&>(in_src);
	
	entryKeepGoing = src.GetFirstEntry(entryName, entryIndex);
	while (entryKeepGoing) {
		CopyConfigEntry(src, dest, path, entryName);
		entryKeepGoing = src.GetNextEntry(entryName, entryIndex);
	}
	
	wxString groupName;
	long groupIndex;
	bool groupKeepGoing;
	
	groupKeepGoing = src.GetFirstGroup(groupName, groupIndex);
	while (groupKeepGoing) {
		if (groupName != _T("wxWindows") && (includeMainGroup || groupName != _T("main"))) {
			wxString subPath = path + groupName + _T("/");
			src.SetPath(subPath);
			CopyConfig(src, dest, includeMainGroup, subPath);
			src.SetPath(path);
		}
		groupKeepGoing = src.GetNextGroup(groupName, groupIndex);
	}
}

// assumed that cfg1's path is already at path
// and that cfg1 has an entry at /path/entry
/** Helper function for CopyConfig(). Copies a single config entry. */
void ProMan::CopyConfigEntry(const wxConfigBase& src,
							 wxConfigBase& dest,
							 const wxString path,
							 const wxString entry) {
	switch(src.GetEntryType(entry)) {
		case wxConfigBase::Type_Unknown:
		case wxConfigBase::Type_String: {
			wxString value = src.Read(entry, wxEmptyString);
			dest.Write(path + entry, value);
			break;
		}
		case wxConfigBase::Type_Boolean: {
			bool value = false;
			src.Read(entry, &value, value);
			dest.Write(path + entry, value);
			break;
		}
		case wxConfigBase::Type_Integer: {
			long value = false;
			src.Read(entry, &value, value);
			dest.Write(path + entry, value);
			break;
		}
		case wxConfigBase::Type_Float: {
			double value = false;
			src.Read(entry, &value, value);
			dest.Write(path + entry, value);
			break;
		}
	}
}

/** Clears the contents of the provided config. */
void ProMan::ClearConfig(wxConfigBase& cfg) {
	wxString entryName;
	long entryIndex;
	bool entryKeepGoing;
	
	wxArrayString entries, groups;
	
	entryKeepGoing = cfg.GetFirstEntry(entryName, entryIndex);
	while (entryKeepGoing) {
		entries.Add(entryName);
		entryKeepGoing = cfg.GetNextEntry(entryName, entryIndex);
	}
	
	wxString groupName;
	long groupIndex;
	bool groupKeepGoing;
	
	groupKeepGoing = cfg.GetFirstGroup(groupName, groupIndex);
	while (groupKeepGoing) {
		groups.Add(groupName);
		groupKeepGoing = cfg.GetNextGroup(groupName, groupIndex);
	}
	
	for (int i = 0, n = entries.GetCount(); i < n; ++i) {
		cfg.DeleteEntry(entries.Item(i));
	}
	
	for (int i = 0, n = groups.GetCount(); i < n; ++i) {
		cfg.DeleteGroup(groups.Item(i));
	}
}

/** Tests whether two configs are equal, where two configs are defined as 
 equal if they have the same entries and groups with the same values,
 except that the wxWindows group is ignored, if it exists. */
bool ProMan::AreConfigsEqual(wxConfigBase& cfg1, wxConfigBase& cfg2) {
	return IsConfigSubset(cfg1, cfg2) && IsConfigSubset(cfg2, cfg1);
}

/** Tests whether the first config is a subset of the second, where subset
 means that the first config's entries and groups appear in the second config
 in the same structure and with the same values, with the exception
 that the wxWindows group is ignored if it exists.*/
bool ProMan::IsConfigSubset(wxConfigBase& cfg1,
							 wxConfigBase& cfg2,
							 const wxString path) {
	wxString entryName;
	long entryIndex;
	bool entryKeepGoing;
	
	entryKeepGoing = cfg1.GetFirstEntry(entryName, entryIndex);
	while (entryKeepGoing) {
		if (!AreEntriesEqual(cfg1, cfg2, path, entryName)) {
			return false;
		}
		entryKeepGoing = cfg1.GetNextEntry(entryName, entryIndex);
	}
	
	wxString groupName;
	long groupIndex;
	bool groupKeepGoing;
	
	groupKeepGoing = cfg1.GetFirstGroup(groupName, groupIndex);
	while (groupKeepGoing) {
		if (groupName != _T("wxWindows")) {
			wxString subPath = path + groupName + _T("/");
			cfg1.SetPath(subPath);
			if (!IsConfigSubset(cfg1, cfg2, subPath)) {
				cfg1.SetPath(path); // fix the path before returning
				return false;
			}
			cfg1.SetPath(path);
		}
		groupKeepGoing = cfg1.GetNextGroup(groupName, groupIndex);
	}
	return true;
}

// assumed that cfg1's path is already at path
/** Helper function for AreConfigsEqual().
 Tests whether an entry exists in two configs and if so,
 whether the value stored at the entry in each is equal.*/
bool ProMan::AreEntriesEqual(const wxConfigBase& cfg1,
							 const wxConfigBase& cfg2,
							 const wxString path,
							 const wxString entry) {
	switch(cfg1.GetEntryType(entry)) {
		case wxConfigBase::Type_Unknown:
		case wxConfigBase::Type_String: {
			wxString value1, value2;
			if ((!cfg1.Read(entry, &value1)) || (!cfg2.Read(path + entry, &value2))) {
				return false;
			} else {
				return value1 == value2;
			}
			break;
		}
		case wxConfigBase::Type_Boolean: {
			bool value1, value2;
			if ((!cfg1.Read(entry, &value1)) || (!cfg2.Read(path + entry, &value2))) {
				return false;
			} else {
				return value1 == value2;
			}
			break;
		}
		case wxConfigBase::Type_Integer: {
			long value1, value2;
			if ((!cfg1.Read(entry, &value1)) || (!cfg2.Read(path + entry, &value2))) {
				return false;
			} else {
				return value1 == value2;
			}
			break;
		}
		case wxConfigBase::Type_Float: {
			double value1, value2;
			if ((!cfg1.Read(entry, &value1)) || (!cfg2.Read(path + entry, &value2))) {
				return false;
			} else {
				return value1 == value2;
			}
			break;
		}
		default:
			wxLogWarning(_T("unknown entry type %d"), cfg1.GetEntryType(entry));
			return false;
		break;
	}
	wxCHECK_MSG(false, false, _T("escaped switch statement in ProMan::AreEntriesEqual"));
}

/** Debugging function that prints contents of provided config to log. */
void ProMan::LogConfigContents(wxConfigBase& cfg, const wxString path, const bool includeWxWindows) {
	wxString entryName;
	long entryIndex;
	bool entryKeepGoing;
	
	entryKeepGoing = cfg.GetFirstEntry(entryName, entryIndex);
	while (entryKeepGoing) {
		wxLogDebug(_T("  %s%s = %s"),
			path.c_str(), entryName.c_str(), cfg.Read(entryName, _T("ENTRY_NOT_FOUND")).c_str());
		entryKeepGoing = cfg.GetNextEntry(entryName, entryIndex);
	}
	
	wxString groupName;
	long groupIndex;
	bool groupKeepGoing;
	
	groupKeepGoing = cfg.GetFirstGroup(groupName, groupIndex);
	while (groupKeepGoing) {
		if (includeWxWindows) {
			wxString subPath = path + groupName + _T("/");
			cfg.SetPath(subPath);
			LogConfigContents(cfg, subPath);
			cfg.SetPath(path);
		} else if (groupName != _T("wxWindows")) {
			wxString subPath = path + groupName + _T("/");
			cfg.SetPath(subPath);
			LogConfigContents(cfg, subPath);
			cfg.SetPath(path);
		}
		groupKeepGoing = cfg.GetNextGroup(groupName, groupIndex);
	}
}

/** Start of a test case for config manipulation functions */
void ProMan::TestConfigFunctions(wxConfigBase& src) {
	wxLogDebug(_T("contents of source config:"));
	LogConfigContents(src);

	wxString tempFileName = wxFileName::CreateTempFileName(_T("wxLtest"));
	wxLogDebug(_T("created temp file '%s'"), tempFileName.c_str());
	wxFFileInputStream instream(tempFileName);
	wxFileConfig* dest = new wxFileConfig(instream);
	
	wxLogDebug(_T("initial contents of dest config:"));
	LogConfigContents(*dest);
	
	wxLogDebug(_T("is src a subset of dest? %s"), IsConfigSubset(src, *dest) ? _T("true") : _T("false"));
	wxLogDebug(_T("is dest a subset of src? %s"), IsConfigSubset(*dest, src) ? _T("true") : _T("false"));
	wxLogDebug(_T("are configs src and dest equal? %s"), AreConfigsEqual(*dest, src) ? _T("true") : _T("false"));

	wxLogDebug(_T("copying source config to dest config"));
	CopyConfig(src, *dest);
	
	wxLogDebug(_T("contents of dest config after copying:"));
	LogConfigContents(*dest);
	
	wxLogDebug(_T("is src a subset of dest? %s"), IsConfigSubset(src, *dest) ? _T("true") : _T("false"));
	wxLogDebug(_T("is dest a subset of src? %s"), IsConfigSubset(*dest, src) ? _T("true") : _T("false"));
	wxLogDebug(_T("are configs src and dest equal? %s"), AreConfigsEqual(*dest, src) ? _T("true") : _T("false"));
	
	wxLogDebug(_T("deleting entry %s from dest"),
		PRO_CFG_LIGHTING_PRESET.c_str());
	dest->DeleteEntry(PRO_CFG_LIGHTING_PRESET, true);
	
	wxLogDebug(_T("contents of dest config after entry deletion:"));
	LogConfigContents(*dest);
	
	wxLogDebug(_T("is src a subset of dest? %s"), IsConfigSubset(src, *dest) ? _T("true") : _T("false"));
	wxLogDebug(_T("is dest a subset of src? %s"), IsConfigSubset(*dest, src) ? _T("true") : _T("false"));
	wxLogDebug(_T("are configs src and dest equal? %s"), AreConfigsEqual(*dest, src) ? _T("true") : _T("false"));
	
	wxLogDebug(_T("clearing dest config"));
	wxLogDebug(_T("before clearing, dest has %d entries and %d groups"),
		dest->GetNumberOfEntries(true), dest->GetNumberOfGroups(true));

	ClearConfig(*dest);
	
	wxLogDebug(_T("contents of dest config after clearing:"));
	LogConfigContents(*dest);
	
	wxLogDebug(_T("after clearing, dest has %d entries and %d groups"),
		dest->GetNumberOfEntries(true), dest->GetNumberOfGroups(true));
	
	wxLogDebug(_T("recopying src to dest"));
	
	CopyConfig(src, *dest);
	
	wxLogDebug(_T("contents of dest config after second copying:"));
	LogConfigContents(*dest);
	
	wxLogDebug(_T("is src a subset of dest? %s"), IsConfigSubset(src, *dest) ? _T("true") : _T("false"));
	wxLogDebug(_T("is dest a subset of src? %s"), IsConfigSubset(*dest, src) ? _T("true") : _T("false"));
	wxLogDebug(_T("are configs src and dest equal? %s"), AreConfigsEqual(*dest, src) ? _T("true") : _T("false"));
	
	const wxString FAKE_ENTRY = _T("/foo/bar");
	const wxString FAKE_VALUE = _T("blah");
	
	wxLogDebug(_T("adding entry %s to dest"), FAKE_ENTRY.c_str());
	dest->Write(FAKE_ENTRY, FAKE_VALUE);
	
	wxLogDebug(_T("contents of dest config after entry addition:"));
	LogConfigContents(*dest);
	
	wxLogDebug(_T("is src a subset of dest? %s"), IsConfigSubset(src, *dest) ? _T("true") : _T("false"));
	wxLogDebug(_T("is dest a subset of src? %s"), IsConfigSubset(*dest, src) ? _T("true") : _T("false"));
	wxLogDebug(_T("are configs src and dest equal? %s"), AreConfigsEqual(*dest, src) ? _T("true") : _T("false"));

	delete dest;
	
	wxLogDebug(_T("config test complete."));
}

/** Applies the current profile to the registry where 
 Freespace 2 can read it. */
ProMan::RegistryCodes ProMan::PushCurrentProfile() {
	if (this->currentProfile == NULL) {
		wxLogError(_T("PushCurrentProfile: attempt to push null current profile"));
		return ProMan::UnknownError;
	} else {
		return ProMan::PushProfile(this->currentProfile);
	}
}

/** Applies the passed wxFileConfig profile to the registry where 
Freespace 2 can read it. */
ProMan::RegistryCodes ProMan::PushProfile(wxFileConfig *cfg) {
	wxCHECK_MSG(cfg != NULL, ProMan::UnknownError, _T("ProMan::PushProfile given null wxFileConfig!"));
#if IS_WIN32
	// check if binary supports configfile
	return RegistryPushProfile(cfg);
#elif IS_LINUX || IS_APPLE
	return FilePushProfile(cfg);
#else
#error "One of IS_WIN32, IS_LINUX, IS_APPLE must evaluate to true"
#endif
}

/** Takes the settings in the registry and puts them into the wxFileConfig */
ProMan::RegistryCodes ProMan::PullProfile(wxFileConfig *cfg) {
	wxCHECK_MSG(cfg != NULL, ProMan::UnknownError, _T("ProMan::PullProfile given null wxFileConfig!"));
#if IS_WIN32
	// check if binary supports configfile
	return RegistryPullProfile(cfg);
#elif IS_LINUX || IS_APPLE
	return FilePullProfile(cfg);
#else
#error "One of IS_WIN32, IS_LINUX, IS_APPLE must evaluate to true"
#endif
}
