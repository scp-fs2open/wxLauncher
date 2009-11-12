#include <wx/wx.h>
#include <wx/fileconf.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include <wx/dir.h>

#include "ProfileManager.h"
#include "wxLauncherApp.h"
#include "wxIDS.h"

#include "wxLauncherSetup.h"

ProMan* ProMan::proman = NULL;
bool ProMan::isInitialized = false;

#define GLOBAL_INI_FILE_NAME _T("global.ini")

///////////// Events

/** EVT_PROFILE_EVENT */
DEFINE_EVENT_TYPE(EVT_PROFILE_CHANGE);

DEFINE_EVENT_TYPE(EVT_CURRENT_PROFILE_CHANGED);

#include <wx/listimpl.cpp> // required magic incatation
WX_DEFINE_LIST(EventHandlers);


void ProMan::GenerateChangeEvent() {
	wxCommandEvent event(EVT_PROFILE_CHANGE, wxID_NONE);
	wxLogDebug(_T("Generating PRofile change event"));
	EventHandlers::iterator iter = this->eventHandlers.begin();
	do {
		wxEvtHandler* current = *iter;
		current->ProcessEvent(event);
		iter++;
		wxLogDebug(_T(" Sent Profile Change event"));
	} while (iter != this->eventHandlers.end());
}

void ProMan::GenerateCurrentProfileChangedEvent() {
	wxCommandEvent event(EVT_CURRENT_PROFILE_CHANGED, wxID_NONE);
	wxLogDebug(_T("Generating current profile changed event"));
	EventHandlers::iterator iter = this->eventHandlers.begin();
	while (iter != this->eventHandlers.end()) {
		wxEvtHandler* current = *iter;
		current->ProcessEvent(event);
		iter++;
		wxLogDebug(_T(" Sent current profile changed event"));
	} 
}

void ProMan::AddEventHandler(wxEvtHandler *handler) {
	this->eventHandlers.Append(handler);
}

void ProMan::RemoveEventHandler(wxEvtHandler *handler) {
	this->eventHandlers.DeleteObject(handler);
}
	

/** Sets up the profile manager. Must be called on program startup so that
it can intercept global wxWidgets configuation functions. 
\return true when setup was successful, false if proman is not
ready and the program should not continue. */
bool ProMan::Initialize() {
	wxConfigBase::DontCreateOnDemand();

	ProMan::proman = new ProMan();

	wxFileName file;
	file.Assign(GET_PROFILE_STORAGEFOLDER(), GLOBAL_INI_FILE_NAME);

	if ( !file.IsOk() ) {
		wxLogError(_T(" '%s' is not valid!"), file.GetFullPath());
		return false;
	}

	wxLogInfo(_T(" My profiles file is: %s"), file.GetFullPath());
	if ( !wxFileName::DirExists(file.GetPath())
		&& !wxFileName::Mkdir(file.GetPath(), wxPATH_MKDIR_FULL ) ) {
		wxLogError(_T(" Unable to make profile directory."));
		return false;
	}

	ProMan::proman->profileList = new wxFileConfig(wxFFileInputStream(file.GetFullPath(),
		(file.FileExists())?_T("rb"):_T("w+b")));

	// fetch all profiles.
	wxArrayString foundProfiles;
	wxDir::GetAllFiles(GET_PROFILE_STORAGEFOLDER(), &foundProfiles, _T("pro?????.ini"));

	wxLogInfo(_T(" Found %d profile(s)."), foundProfiles.Count());
	for( size_t i = 0; i < foundProfiles.Count(); i++) {
		wxLogDebug(_T("  Opening %s"), foundProfiles[i]);
		wxFFileInputStream instream(foundProfiles[i]);
		wxFileConfig *config = new wxFileConfig(instream);
		
		wxString name;
		config->Read(PRO_CFG_MAIN_NAME, &name, wxString::Format(_T("Profile %05d"), i));

		ProMan::proman->profiles[name] = config;
		wxLogDebug(_T("  Opened profile named: %s"), name);
	}

	wxString currentProfile;
	ProMan::proman->profileList->Read(
		GBL_CFG_MAIN_LASTPROFILE, &currentProfile, _T("Default"));
	
	wxLogDebug(_T(" Searching for profile: %s"), currentProfile);
	if ( ProMan::proman->profiles.find(currentProfile)
	== ProMan::proman->profiles.end() ) {
		// lastprofile does not exist
		wxLogDebug(_T(" lastprofile '%s' does not exist!"), currentProfile);
		if ( ProMan::proman->profiles.find(_T("Default"))
		== ProMan::proman->profiles.end() ) {
			// default profile also does not exist.
			// Means this is likely the first run this system
			// Create a default profile
			wxLogInfo(_T(" Default profile does not exist! Creating..."));
			ProMan::proman->CreateNewProfile(_T("Default"));
		}
		wxLogInfo(_T(" Resetting lastprofile to Default."));
		ProMan::proman->profileList->Write(GBL_CFG_MAIN_LASTPROFILE, _T("Default"));
		ProMan::proman->profileList->Save(wxFFileOutputStream(file.GetFullPath()));
		currentProfile = _T("Default");
	}

	wxLogDebug(_T(" Making '%s' the application profile"), currentProfile);
	if ( !ProMan::proman->SwitchTo(currentProfile) ) {
		wxLogError(_T("Unable to set current profile to '%s'"), currentProfile);
		return false;
	}

	ProMan::isInitialized = true;
	wxLogDebug(_T(" Profile Manager is setup"));
	return true;
}

/** clean up the memory that the manager is using. */
bool ProMan::DeInitialize() {
	if ( ProMan::isInitialized ) {
		ProMan::isInitialized = false;

		delete ProMan::proman;
		ProMan::proman = NULL;
		
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
	this->profileList = NULL;
	this->isAutoSaving = true;
	this->currentProfile = NULL;
}

/** Destructor. */
ProMan::~ProMan() {
	if ( this->profileList != NULL ) {
		if ( this->isAutoSaving ) {
			wxFileName file;
			file.Assign(GET_PROFILE_STORAGEFOLDER(), GLOBAL_INI_FILE_NAME);
			this->profileList->Save(wxFFileOutputStream(file.GetFullPath()));
		} else {
			wxLogWarning(_T("Profile Manager is being destroyed without saving changes."));
		}
		delete this->profileList;
	}

	this->SaveCurrentProfile();

	// don't leak the wxFileConfigs
	ProfileMap::iterator iter = this->profiles.begin();
	while ( iter != this->profiles.end() ) {
		delete iter->second;
		iter++;
	}
}
/** Creates a new profile including the directory for it to go in, the entry
in the profiles map. Returns true if creation was successful. */
bool ProMan::CreateNewProfile(wxString newName) {
	wxFileName profile;
	profile.Assign(
		GET_PROFILE_STORAGEFOLDER(),
		wxString::Format(_T("pro%05d.ini"), this->profiles.size()));

	wxASSERT_MSG( profile.IsOk(), _T("Profile filename is invalid"));

	if ( !wxFileName::DirExists(profile.GetPath())
		&& !wxFileName::Mkdir( profile.GetPath(), wxPATH_MKDIR_FULL) ) {
		wxLogWarning(_T("  Unable to create profile directory: %s"), profile.GetPath());
		return false;
	}

	wxFileConfig* config = new wxFileConfig(wxFFileInputStream(profile.GetFullPath(), _T("w+b")));
	config->Write(PRO_CFG_MAIN_NAME, newName);
	config->Write(PRO_CFG_MAIN_FILENAME, profile.GetFullName());
	config->Save(wxFFileOutputStream(profile.GetFullPath()));

	this->profiles[newName] = config;
	return true;
}

/** Returns the pointer to the currently selected profile. */
wxFileConfig* ProMan::Get() {
	if ( this->isInitialized ) {
		return this->currentProfile;
	} else {
		return NULL;
	}
}

/** Returns the pointer to the global profile. */
wxFileConfig* ProMan::Global() {
	return this->profileList;
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

/** Save the current profile to disk. Does not affect Global or anyother profile.
We assume all profiles other than the current app profile does not have any 
unsaved changes. So we only check the current profile. */
void ProMan::SaveCurrentProfile() {
	wxConfigBase* configbase = wxFileConfig::Get(false);
	if ( configbase == NULL ) {
		wxLogWarning(_T("There is no global fileconfig."));
		return;
	}
	wxFileConfig* config = dynamic_cast<wxFileConfig*>(configbase);
	if ( config != NULL ) {
		if ( this->isAutoSaving ) {
			wxString profilename;
			if ( !config->Read(PRO_CFG_MAIN_FILENAME, &profilename) ) {
				wxLogWarning(_T("Current Profile does not have a file name, and I am unable to auto save."));
			} else {
				wxFileName file;
				file.Assign(GET_PROFILE_STORAGEFOLDER(), profilename);
				wxASSERT( file.IsOk() );
				config->Save(wxFFileOutputStream(file.GetFullPath()));
				wxLogDebug(_T("Current config saved (%s)."), file.GetFullPath());
			}
		} else {
			wxLogWarning(_T("Current Profile Manager is being destroyed without saving changes."));
		}
	} else {
		wxLogWarning(_T("Configbase is not a wxFileConfig."));
	}
}

wxString ProMan::GetCurrentName() {
	return this->currentProfileName;
}

bool ProMan::SwitchTo(wxString name) {
	if ( this->profiles.find(name) == this->profiles.end() ) {
		return false;
	} else {
		this->currentProfileName = name;
		this->currentProfile = this->profiles.find(name)->second;
		wxFileConfig::Set(this->currentProfile);
		this->profileList->Write(GBL_CFG_MAIN_LASTPROFILE, name);
		this->GenerateCurrentProfileChangedEvent();
		return true;
	}
}

bool ProMan::CloneProfile(wxString originalName, wxString copyName) {
	wxLogDebug(_T("Cloning original profile (%s) to %s"), originalName, copyName);
	if ( !this->DoesProfileExist(originalName) ) {
		wxLogWarning(_("Original Profile '%s' does not exist!"), originalName);
		return false;
	}
	if ( this->DoesProfileExist(copyName) ) {
		wxLogWarning(_("Target profile '%s' already exists!"), copyName);
		return false;
	}
	if ( !this->CreateNewProfile(copyName) ) {
		return false;
	}

	wxFileConfig* config = this->profiles[copyName];
	wxCHECK_MSG( config != NULL, false, _T("Create returned true but did not create profile"));

	wxString str;
	long cookie;
	bool cont = config->GetFirstEntry(str, cookie);
	while ( cont ) {
		wxLogDebug(_T("  Got: %s"), str);

		cont = config->GetNextEntry(str, cookie);
	}
	this->GenerateChangeEvent();
	return true;
}

bool ProMan::DeleteProfile(wxString name) {
	wxLogDebug(_T("Deleting profile: %s"), name);
	if ( name == _T("Default") ) {
		wxLogWarning(_("Cannot delete Default profile."));
		return false;
	}
	if ( name == this->currentProfileName ) {
		wxLogInfo(_T("Deleting current profile. Switching current to 'Default' profile"));
		this->SwitchTo(_T("Default"));
	}
	if ( this->DoesProfileExist(name) ) {
		wxLogDebug(_T(" Profile exists"));
		wxFileConfig* config = this->profiles[name];

		wxString filename;
		if ( !config->Read(PRO_CFG_MAIN_FILENAME, &filename) ) {
			wxLogWarning(_T("Unable to get filename to delete %s"), name);
			return false;
		}

		wxFileName file;
		file.Assign(GET_PROFILE_STORAGEFOLDER(), filename);

		if ( file.FileExists() ) {
			wxLogDebug(_T(" Backing file exists"));
			if ( wxRemoveFile(file.GetFullPath()) ) {
				this->profiles.erase(this->profiles.find(name));
				delete config;
				
				wxLogMessage(_("Profile '%s' deleted."), name);
				this->GenerateChangeEvent();
				return true;
			} else {
				wxLogWarning(_("Unable to delete file for profile '%s'"), name);
			}
		} else {
			wxLogWarning(_("Backing file (%s) for profile '%s' does not exist"), file.GetFullPath(), name);
		}
	} else {
		wxLogWarning(_("Profile %s does not exist. Cannot delete."), name);
	}
	return false;
}

