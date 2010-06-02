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

#include "generated/configure_launcher.h"
#include "datastructures/FSOExecutable.h"
#include "global/ids.h"
#include <wx/dir.h>
#include <wx/tokenzr.h>

#include "global/MemoryDebugging.h"

enum BuildCapabilities_enum {
	SUPPORT_OPENAL			= 1<<0,
	NOT_SUPPORT_DIRECT3D	= 1<<1,
} BuildCapabilities;

FSOExecutable::FSOExecutable() {
	major = 0;
	minor = 0;
	revision = 0;
	inferno = false;
	sse = 0;
	debug = false;
	build = 0;
	buildCaps = 0;
}

FSOExecutable::~FSOExecutable() {
}

bool FSOExecutable::SupportsDirect3D() {
	return (buildCaps & NOT_SUPPORT_DIRECT3D) == 0;
}

bool FSOExecutable::SupportsOpenAL() {
	return (buildCaps & SUPPORT_OPENAL) > 0;
}


bool FSOExecutable::CheckRootFolder(wxFileName path) {
	if ( !path.IsOk() ) {
		wxLogError(_T(" New Root folder is not OK"));
		return false;
	}
	if ( path.GetPath().IsEmpty() ) {
		wxLogError(_T(" path FullName is not empty"));
		return false;
	}
	wxArrayString files = FSOExecutable::GetBinariesFromRootFolder(path);
	if ( files.Count() > 0 ) {
		return true;
	} else {
		return false;
	}
}

#if IS_WIN32
#define EXECUTABLE_GLOB_PATTERN _T("fs2_open_*.exe")
#elif IS_LINUX
#define EXECUTABLE_GLOB_PATTERN _T("fs2_open_*")
#elif IS_APPLE
#define EXECUTABLE_GLOB_PATTERN _T("fs2_open_*.app")
#else
#error "One of IS_WIN32, IS_LINUX, IS_APPLE must evaluate to true"
#endif

wxArrayString FSOExecutable::GetBinariesFromRootFolder(wxFileName path) {
	wxArrayString files;
	wxDir folder(path.GetPath());
	wxString filename;

	bool cont = folder.GetFirst(&filename, EXECUTABLE_GLOB_PATTERN, wxDIR_FILES);
	while (cont == true) {
#if IS_LINUX
		if ( !filename.EndsWith(_T(".exe"))
			&& !filename.EndsWith(_T(".app")) ) {
#endif
		files.Add(filename);
#if IS_LINUX
		}
#endif
		cont = folder.GetNext(&filename);
	}
	wxLogInfo(_T(" Found %d fs2_open executables in '%s'"), files.Count(), path.GetPath().c_str());
	return files;
}

FSOExecutable FSOExecutable::GetBinaryVersion(wxString binaryname) {
	wxLogDebug(_T("Making version struct for '%s'"), binaryname.c_str());
	FSOExecutable ver;
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
			wxLogWarning(_T("was expecting open; got %s"), first1.c_str());
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
		} else if ( token.StartsWith(_T("d")) && token.size() == 1 ) {
			ver.debug = true;
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
				wxLogWarning(_T("Token ending in 'd' is not a number (%s)"), token.c_str());
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
					wxLogWarning(_T("Revision version out of range (%d)"), version);
				}
			} else {
				wxLogWarning(_T("Token ending in 'r' is not a number (%s)"), token.c_str());
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
		} else {
			if (!ver.string.IsEmpty()) {
				ver.string += _T(" ");
			}
			ver.string += token;
		}
	}
	if ( ver.string.StartsWith(_T("ant")) ) {
		// is an antipodes builds
		ver.string = wxString::Format(_T("Antipodes%s"),
			(ver.revision == 0) ? _T("") : wxString::Format(_T(" #%d"), ver.revision).c_str());
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
wxString FSOExecutable::MakeVersionStringFromVersion(FSOExecutable ver) {
	bool hasfullversion = (ver.major != 0 && ver.minor != 0 && ver.revision != 0);
	return wxString::Format(_T("%s %s%s%s %s%s%s"),
		(ver.binaryname.IsEmpty()) ? _T("Unknown") : ver.binaryname.c_str(), // Freespace 2 Open
		(hasfullversion) ? wxString::Format(_T("%d.%d.%d"), ver.major, ver.minor, ver.revision).c_str() : wxEmptyString,
		(ver.build == 0) ? wxEmptyString : wxString::Format((hasfullversion) ? _T(" Build %d") : _T("Build %d"), ver.build).c_str(),
		(ver.string.IsEmpty() ) ? wxEmptyString : wxString::Format((hasfullversion) ? _T(" (%s)") : _T("%s"), ver.string.c_str()).c_str(),
		(ver.debug) ? _T("Debug") : _T("Release"),
		(ver.inferno) ? _T(" Inferno") : wxEmptyString,
		(ver.sse == 0) ? wxEmptyString : (ver.sse == 1) ? _T(" SSE") : _T(" SSE2")
		);
}
