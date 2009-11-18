#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/tokenzr.h>

#include "TCManager.h"
#include "ProfileManager.h"

#include "wxLauncherSetup.h"

FSOVersion::FSOVersion() {
	major = 0;
	minor = 0;
	revision = 0;
	inferno = false;
	sse = 0;
	debug = false;
	build = 0;
}

FSOVersion::~FSOVersion() {
}

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

bool TCManager::CheckRootFolder(wxFileName path) {
	if ( !path.IsOk() ) {
		wxLogError(_T(" New Root folder is not OK"));
		return false;
	}
	if ( path.GetPath().IsEmpty() ) {
		wxLogError(_T(" path FullName is not empty"));
		return false;
	}
	wxArrayString files = TCManager::GetBinariesFromRootFolder(path);
	if ( files.Count() > 0 ) {
		return true;
	} else {
		return false;
	}
}

wxArrayString TCManager::GetBinariesFromRootFolder(wxFileName path) {
	wxArrayString files;
	wxDir::GetAllFiles(path.GetPath(), &files, _T("fs2_open_*.exe"), wxDIR_FILES);
	wxLogInfo(_T(" Found %d fs2_open executables in '%s'"), files.Count(), path.GetPath());
	return files;
}

FSOVersion TCManager::GetBinaryVersion(wxString binaryname) {
	wxLogDebug(_T("Making version struct for '%s'"), binaryname);
	FSOVersion ver;
	wxStringTokenizer tok(binaryname, _T("_.-"));
	wxString first;
	ver.executablename = binaryname;

	if ( !tok.HasMoreTokens() ) {
		wxLogError(_T("Have not gotten fs2"));
		return ver;
	}
	first = tok.GetNextToken();
	if ( tok.HasMoreTokens() && (first == _T("fred2") || first == _T("fs2")) ) {
		wxString first1 = tok.GetNextToken();
		if ( first1 == _T("open") ) {
			if ( first = _T("fs2") ) {
				ver.binaryname = _T("FreeSpace 2 Open");
			} else {
				ver.binaryname = _T("Fred 2 Open");
			}
		} else {
			wxLogWarning(_T("was expecting open; got %s"), first1);
			return ver;
		}
	} else {
		wxLogWarning(_T("name too short"));
		return ver;
	}

	while ( tok.HasMoreTokens() ) {
		wxString token = tok.GetNextToken();
		token.MakeLower();
		wxString temp;

		if ( token.StartsWith(_T("exe")) ) {
			; // do nothing
		} else if ( token.IsNumber() && ver.major == 0 ) {
			// must be major version number
			long version = 0;
			bool ok = token.ToLong(&version);
			if ( ok && version < 1000 && version > 0 ) {
				ver.major = (int)version;
			} else {
				wxLogWarning(_T("major out of range (%d)"), version);
			}
		} else if ( token.IsNumber() && ver.minor == 0 ) {
			// must be minor version number
			long version = 0;
			bool ok = token.ToLong(&version);
			if ( ok && version < 1000 && version > 0 ) {
				ver.minor = (int)version;
			} else {
				wxLogWarning(_T("minor out of range (%d)"), version);
			}
		} else if ( token.IsNumber() && ver.revision == 0) {
			// must be revision version number
			long version = 0;
			bool ok = token.ToLong(&version);
			if ( ok && version < 1000 && version > 0 ) {
				ver.revision = (int)version;
			} else {
				wxLogWarning(_T("Revision version out of range (%d)"), version);
			}
		} else if ( token.EndsWith(_T("d"), &temp) ) {
			if ( temp.IsNumber() ) {
				// is the revision version number
				long version = 0;
				bool ok = temp.ToLong(&version);
				if ( ok && version < 1000 && version > 0 ) {
					ver.revision = (int)version;
					ver.debug = true;
				} else {
					wxLogWarning(_T("Revisiond version out of range (%d)"), version);
				}
			} else {
				wxLogWarning(_T("Token ending in 'd' is not a number (%s)"), token);
			}
		} else if ( token.EndsWith(_T("r"), &temp) ) {
			if ( temp.IsNumber() ) {
				// is the revision version number
				long version = 0;
				bool ok = temp.ToLong(&version);
				if ( ok && version < 1000 && version > 0 ) {
					ver.revision = (int)version;
					ver.debug = false;
				} else {
					wxLogWarning(_T("Revisionr version out of range (%d)"), version);
				}
			} else {
				wxLogWarning(_T("Token ending in 'r' is not a number (%s)"), token);
			}
		} else if ( token.IsNumber() && token.size() == 8 ) {
			// must be a date from SirKnightly's builds
			// just ignore it the date
		} else if ( token.StartsWith(_T("r"), &temp) && temp.IsNumber() ) {
			// must be a revision number from SirKnightly's builds
			long version = 0;
			bool ok = temp.ToLong(&version);
			if ( ok && version > 0 ) {
				ver.build = (int)version;
			} else {
				wxLogWarning(_T("SirKightly build number out of range (%d)"), version);
			}
		} else if ( token.StartsWith(_T("ant")) && tok.HasMoreTokens() ) {
			ver.string = _T("ant");
		} else if ( token.StartsWith(_T("sse2")) ) {
			ver.sse = 2;
		} else if ( token.StartsWith(_T("sse")) ) {
			ver.sse = 1;
		} else if ( token.StartsWith(_T("inf")) ) {
			ver.inferno = true;
		} else if ( token.StartsWith(_T("debug")) ){
			ver.debug = true;
		} else if ( token.EndsWith(_T("t"), &temp) && temp.size() == 8 ) {
			ver.string += token;
		} else {
			wxLogWarning(_T(" Got token I don't understand (%s)"), token);
		}
	}
	if ( ver.string.StartsWith(_T("ant")) ) {
		// is an antipodes builds
		ver.string = wxString::Format(_T("Antipodes%s"),
			(ver.revision == 0) ? _T("") : wxString::Format(_T(" #%d"), ver.revision));
	}
	return ver;
}
/** Returns the version string to display to the user from a previously
parsed FSOVersion object.  The intention is to display all information that
is normally encoded into the executable's file name into a long string that
more user friendly. 

The resulting string looks something like this: \verbatim
Freespace 2 Open Antipodes #4 Debug Inferno SSE2
Freespace 2 Open 3.6.10 Release Inferno SSE
Fred 2 Open 3.6.11 Debug
\endverbatim
*/
wxString TCManager::MakeVersionStringFromVersion(FSOVersion ver) {
	bool hasfullversion = (ver.major != 0 && ver.minor != 0 && ver.revision != 0);
	return wxString::Format(_T("%s %s%s%s %s%s%s"),
		(ver.binaryname.IsEmpty()) ? _T("Unknown") : ver.binaryname, // Freespace 2 Open
		(hasfullversion) ? wxString::Format(_T("%d.%d.%d"), ver.major, ver.minor, ver.revision) : wxEmptyString,
		(ver.build == 0) ? wxEmptyString : wxString::Format((hasfullversion) ? _T(" Build %d") : _T("Build %d"), ver.build),
		(ver.string.IsEmpty() ) ? wxEmptyString : wxString::Format((hasfullversion) ? _T(" (%s)") : _T("%s"), ver.string),
		(ver.debug) ? _T("Debug") : _T("Release"),
		(ver.inferno) ? _T(" Inferno") : wxEmptyString,
		(ver.sse == 0) ? wxEmptyString : (ver.sse == 1) ? _T(" SSE") : _T(" SSE2")
		);
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
		wxLogDebug(_T(" Sent event"));
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
		wxLogDebug(_T(" Sent event"));
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
		wxLogDebug(_T(" Sent event"));
	}
}

void TCManager::CurrentProfileChanged(wxCommandEvent &WXUNUSED(event)) {

	TCManager::GenerateTCChanged();
	TCManager::GenerateTCBinaryChanged();
	TCManager::GenerateTCSelectedModChanged();
}