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

#ifndef FSOEXECUTABLE_H
#define FSOEXECUTABLE_H

#include <wx/wx.h>
#include <wx/clntdata.h>
#include <wx/filename.h>

class FSOExecutable: public wxClientData {
public:
	enum Configuration {
		CONFIG_RELEASE,
		CONFIG_DEBUG,
		CONFIG_FASTDEBUG
	};

	virtual ~FSOExecutable();
	bool SupportsDirect3D();
	bool SupportsOpenAL();

	inline bool ExecutableNameEqualTo(const wxString& str) const;
	inline const wxString& GetExecutableName() const;

	static bool IsRootFolderValid(const wxFileName& path, bool quiet = false);
	static bool HasFSOExecutables(const wxFileName& path);

	static wxArrayString GetBinariesFromRootFolder(const wxFileName &path, bool quiet = false);
	static wxArrayString GetFredBinariesFromRootFolder(const wxFileName &path, bool quiet = false);
	static FSOExecutable GetBinaryVersion(wxString binaryname);
	static bool SmellsLikeGitCommitHash(const wxString& str);
	wxString GetVersionString() const;
protected:
	int major;
	int minor;
	int revision;
	bool inferno;
	int sse;
	bool _64bit;
	Configuration configuration;
	int build;
	long year;
	long month;
	bool antipodes;
	int antNumber; //!< antipodes number (such as 8)
	wxString string;
	wxString binaryname; //!< FS2 Open or FRED
	wxString executablename; //!< the actual name of the binary
	wxByte buildCaps;
private:
	FSOExecutable();
	static wxArrayString GetBinariesFromRootFolder(const wxFileName &path, const wxString &globPattern, bool quiet);
};

inline bool FSOExecutable::ExecutableNameEqualTo(const wxString& str) const {
	return this->executablename == str;
}

inline const wxString& FSOExecutable::GetExecutableName() const {
	return this->executablename;
}

#endif