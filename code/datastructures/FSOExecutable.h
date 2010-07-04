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
	virtual ~FSOExecutable();
	bool SupportsDirect3D();
	bool SupportsOpenAL();

	inline bool ExecutableNameEqualTo(const wxString& str) const;
	inline const wxString& GetExecutableName() const;

	static bool CheckRootFolder(wxFileName path);
	static wxArrayString GetBinariesFromRootFolder(const wxFileName &path);
	static wxArrayString GetBinariesFromRootFolder(const wxFileName &path, const wxString &globPattern);
	static wxArrayString GetFredBinariesFromRootFolder(const wxFileName &path);
	static FSOExecutable GetBinaryVersion(wxString binaryname);
	static wxString MakeVersionStringFromVersion(FSOExecutable version);
	wxString MakeVersionStringFromVersion();
protected:
	int major;
	int minor;
	int revision;
	bool inferno;
	int sse;
	bool debug;
	int build;
	wxString string;
	wxString binaryname; //!< fs2_open or fred
	wxString executablename; //!< the actual name of the binary
	wxByte buildCaps;
private:
	FSOExecutable();
};

inline bool FSOExecutable::ExecutableNameEqualTo(const wxString& str) const {
	return this->executablename == str;
}

inline const wxString& FSOExecutable::GetExecutableName() const {
	return this->executablename;
}

#endif