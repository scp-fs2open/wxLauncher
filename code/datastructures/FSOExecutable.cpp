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


bool FSOExecutable::IsRootFolderValid(const wxFileName& path, bool quiet) {
	if ( !path.IsOk() ) {
		if (!quiet) {
			wxLogError(_T(" New root folder %s is not OK"), path.GetFullPath().c_str());			
		}
		return false;
	}
	if ( path.GetPath().IsEmpty() ) {
		if (!quiet) {
			wxLogError(_T(" Root folder %s is empty"), path.GetFullPath().c_str());
		}
		return false;
	}
	return HasFSOExecutables(path);
}

bool FSOExecutable::HasFSOExecutables(const wxFileName& path) {
	wxCHECK_MSG(path.IsOk(), false,
		wxString::Format(_T("provided path %s to HasFSOExecutables is invalid"),
			path.GetFullPath().c_str()));
	
	return !FSOExecutable::GetBinariesFromRootFolder(path, true).IsEmpty(); 
}

#if IS_WIN32
#define EXECUTABLE_GLOB_PATTERN _T("fs2_*.exe")
#define FRED_EXECUTABLE_GLOB_PATTERN _T("fred2_*.exe")
#elif IS_LINUX
#define EXECUTABLE_GLOB_PATTERN _T("fs2_*")
#define FRED_EXECUTABLE_GLOB_PATTERN _T("fred2_*")
#elif IS_APPLE
#define EXECUTABLE_GLOB_PATTERN _T("fs2_*.app")
#define FRED_EXECUTABLE_GLOB_PATTERN _T("fred2_*.app")
#else
#error "One of IS_WIN32, IS_LINUX, IS_APPLE must evaluate to true"
#endif

// quiet is for when you just want to check whether there are FSO/FRED binaries
wxArrayString FSOExecutable::GetBinariesFromRootFolder(const wxFileName& path, bool quiet) {
	return FSOExecutable::GetBinariesFromRootFolder(path, EXECUTABLE_GLOB_PATTERN, quiet);
}

wxArrayString FSOExecutable::GetFredBinariesFromRootFolder(const wxFileName& path, bool quiet) {
	return FSOExecutable::GetBinariesFromRootFolder(path, FRED_EXECUTABLE_GLOB_PATTERN, quiet);
}

wxArrayString FSOExecutable::GetBinariesFromRootFolder(const wxFileName& path, const wxString& globPattern, bool quiet) {
	wxArrayString files;
	wxDir folder(path.GetPath());
	wxString filename;

#if IS_APPLE // Binaries are directories on OSX.
	bool cont = folder.GetFirst(&filename, globPattern, wxDIR_DIRS);
#else
	bool cont = folder.GetFirst(&filename, globPattern, wxDIR_FILES);
#endif

	while (cont) {
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

	// filter out launcher binaries (at least on OSX)
	for (int i = files.GetCount() - 1; i >= 0; --i) {
		if (files[i].Lower().Find(_T("launcher")) != wxNOT_FOUND) {
			files.RemoveAt(i);
		}
	}
	
#if IS_APPLE
	// find actual (Unix) executable inside .app bundle and call the path to it the "executable"
	for (wxArrayString::iterator it = files.begin(), end = files.end(); it != end; ++it) {
		wxString pathToBin = 
			wxDir::FindFirst(path.GetPath(wxPATH_GET_SEPARATOR) + *it + _T("/Contents/MacOS"),
							 _T("*"),
							 wxDIR_FILES);
		pathToBin.Replace(path.GetPath(wxPATH_GET_SEPARATOR), _T(""));
		*it = pathToBin;
	}
#endif
	
	if (!quiet) {
		wxString execType = globPattern.Lower().Find(_T("fred")) == wxNOT_FOUND ? _T("FS2") : _T("FRED2");
		wxLogInfo(_T(" Found %d %s Open executables in '%s'"),
			files.GetCount(), execType.c_str(), path.GetPath().c_str());
		
		for (size_t i = 0, n = files.GetCount(); i < n; ++i) {
			wxLogDebug(_T("Found executable: %s"), files.Item(i).c_str());
		}
	}

	return files;
}

FSOExecutable FSOExecutable::GetBinaryVersion(wxString binaryname) {
	wxLogDebug(_T("Making version struct for the executable '%s'"), binaryname.c_str());
	FSOExecutable ver;
	wxStringTokenizer tok(binaryname, _T("_.- ()[]/"));
	wxString first;
	ver.executablename = binaryname;

	if ( !tok.HasMoreTokens() ) {
		wxLogError(_T("Did not find initial 'fs2' token in executable '%s'"), binaryname.c_str());
		return ver;
	}
	first = tok.GetNextToken();
	if ( tok.HasMoreTokens() && (!first.CmpNoCase(_T("fred2")) || !first.CmpNoCase(_T("fs2"))) ) {
		wxString first1 = tok.GetNextToken();
		if ( !first1.CmpNoCase(_T("open")) ) {
			if ( !first.CmpNoCase(_T("fs2")) ) {
				ver.binaryname = _T("FreeSpace 2 Open");
			} else {
				ver.binaryname = _T("FRED2 Open");
			}
		} else {
			wxLogWarning(_T("was expecting 'open'; got %s in executable %s"), first1.c_str(), binaryname.c_str());
			return ver;
		}
	} else {
		wxLogWarning(_T("executable name '%s' too short"), binaryname.c_str());
		return ver;
	}

	while ( tok.HasMoreTokens() ) {
		wxString token = tok.GetNextToken();
		wxString temp;
		long tempVersion;

		if (token.IsEmpty()) { // can happen in OS X nightly debug builds
			// do nothing
		} else if ( !token.CmpNoCase(_T("exe")) ) {
			; // do nothing
#if IS_APPLE
		} else if ( !token.CmpNoCase(_T("app")) ) {
			break; // we've reached the end of the app name
#endif
		} else if ( token.ToLong(&tempVersion) && token.size() == 8 ) {
			// must be a date from a nightly build; just ignore it
		} else if ( token.ToLong(&tempVersion) && ver.major == 0 ) {
			// must be major version number
			if ( tempVersion < 1000 && tempVersion > 0 ) {
				ver.major = (int)tempVersion;
			} else {
				wxLogWarning(_T("major version number out of range (%ld) in executable %s"), tempVersion, binaryname.c_str());
			}
		} else if ( token.ToLong(&tempVersion) && ver.minor == 0 ) {
			// must be minor version number
			if ( tempVersion < 1000 && tempVersion > 0 ) {
				ver.minor = (int)tempVersion;
			} else {
				wxLogWarning(_T("minor version number out of range (%ld) in executable %s"), tempVersion, binaryname.c_str());
			}
		} else if ( token.ToLong(&tempVersion) && ver.revision == 0) {
			// must be revision version number
			if ( tempVersion < 1000 && tempVersion > 0 ) {
				ver.revision = (int)tempVersion;
			} else {
				wxLogWarning(_T("Revision version number out of range (%ld) in executable %s"), tempVersion, binaryname.c_str());
			}
		} else if ( !token.CmpNoCase(_T("d")) ) {
			ver.debug = true;
		} else if ( token.Lower().EndsWith(_T("d"), &temp) ) {
			if ( temp.ToLong(&tempVersion) ) {
				// is the revision version number
				if ( tempVersion < 1000 && tempVersion > 0 ) {
					ver.revision = (int)tempVersion;
					ver.debug = true;
				} else {
					wxLogWarning(_T("Revision version number out of range (%ld) in executable %s"), tempVersion, binaryname.c_str());
				}
			} else {
				wxLogWarning(_T("Token ending in 'd' is not a number (%s) in executable %s"), token.c_str(), binaryname.c_str());
			}
		} else if ( token.Lower().EndsWith(_T("r"), &temp) ) {
			if (temp.IsEmpty()) {
				// do nothing, the 'r' wasn't preceded by a number
			} else if ( temp.ToLong(&tempVersion) ) {
				// is the revision version number
				if ( tempVersion < 1000 && tempVersion > 0 ) {
					ver.revision = (int)tempVersion;
					ver.debug = false;
				} else {
					wxLogWarning(_T("Revision version number out of range (%ld) in executable %s"), tempVersion, binaryname.c_str());
				}
			} else {
				wxLogWarning(_T("Token ending in 'r' is not a number (%s) in executable %s"), token.c_str(), binaryname.c_str());
			}
		} else if ( token.Lower().StartsWith(_T("r"), &temp) && temp.ToLong(&tempVersion) ) {
			// must be a revision number from SirKnightly's builds
			if ( tempVersion > 0 ) {
				ver.build = (int)tempVersion;
			} else {
				wxLogWarning(_T("SirKnightly build number out of range (%ld) in executable %s"), tempVersion, binaryname.c_str());
			}
		} else if ( token.Lower().StartsWith(_T("ant")) && tok.HasMoreTokens() ) {
			ver.string = _T("ant");

			// in case the token is of the format, e.g., "Ant8"
			wxString tokenCopy(token.Lower());
			long antNumber;
			tokenCopy.Replace(_T("ant"), wxEmptyString);
			if (tokenCopy.ToLong(&antNumber) && ver.major == 0) {
				ver.major = antNumber;
			}
		} else if ( !token.CmpNoCase(_T("sse2")) ) {
			ver.sse = 2;
		} else if ( !token.CmpNoCase(_T("sse")) ) {
			ver.sse = 1;
		} else if ( !token.CmpNoCase(_T("inf")) ) {
			ver.inferno = true;
		} else if ( !token.CmpNoCase(_T("debug")) ){
			ver.debug = true;
		} else if ( !token.CmpNoCase(_T("inferno")) ) {
			ver.inferno = true;
		} else {
			if (!ver.string.IsEmpty()) {
				ver.string += _T(" ");
			}
			ver.string += token;
		}
	}
	if ( ver.string.StartsWith(_T("ant")) ) {
		// is an antipodes builds
		ver.build = 0; // do not display build number if it exists (mostly for OS X)
		ver.string = wxString::Format(_T("Antipodes%s"),
			(ver.major == 0) ? _T("") : wxString::Format(_T(" %d"), ver.major).c_str());
	}
	return ver;
}
/** Returns the version string to display to the user from a previously
parsed FSOVersion object.  The intention is to display all information that
is normally encoded into the executable's file name into a long string that
more user friendly. 

The resulting string looks something like this: \verbatim
FreeSpace 2 Open Antipodes 4 Debug Inferno SSE2
FreeSpace 2 Open 3.6.10 Release Inferno SSE
FRED2 Open 3.6.11 Debug
\endverbatim
*/
wxString FSOExecutable::GetVersionString() const {
	bool hasfullversion = (this->major != 0 && this->minor != 0 && this->revision != 0);
	return wxString::Format(_T("%s %s%s%s %s%s%s"),
		(this->binaryname.IsEmpty()) ? _T("Unknown") : this->binaryname.c_str(), // FreeSpace 2 Open
		(hasfullversion) ? wxString::Format(_T("%d.%d.%d"), this->major, this->minor, this->revision).c_str() : wxEmptyString,
		(this->build == 0) ? wxEmptyString : wxString::Format((hasfullversion) ? _T(" Build %d") : _T("Build %d"), this->build).c_str(),
		(this->string.IsEmpty() ) ? wxEmptyString : wxString::Format((hasfullversion) ? _T(" (%s)") : _T("%s"), this->string.c_str()).c_str(),
		(this->debug) ? _T("Debug") : _T("Release"),
		(this->inferno) ? _T(" Inferno") : wxEmptyString,
		(this->sse == 0) ? wxEmptyString : (this->sse == 1) ? _T(" SSE") : _T(" SSE2")
		);
}
