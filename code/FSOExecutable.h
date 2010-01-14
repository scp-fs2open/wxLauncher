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
	static wxArrayString GetBinariesFromRootFolder(wxFileName path);
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