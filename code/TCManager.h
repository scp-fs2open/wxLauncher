#ifndef TCMANAGER_H
#define TCMANAGER_H

#include <wx/wx.h>
#include <wx/clntdata.h>

class FSOVersion: public wxClientData {
public:
	FSOVersion();
	virtual ~FSOVersion();
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
};

/** Selected TC has changed. */
DECLARE_EVENT_TYPE(EVT_TC_CHANGED, wxID_ANY);
/** Selected binary has changed. */
DECLARE_EVENT_TYPE(EVT_TC_BINARY_CHANGED, wxID_ANY);
/** Selected mod has changed. */
DECLARE_EVENT_TYPE(EVT_TC_SELECTED_MOD_CHANGED, wxID_ANY);

WX_DECLARE_LIST(wxEvtHandler, TCEventHandlers);

class TCManager: public wxEvtHandler {
public:
	TCManager();
	~TCManager();

	static void Initialize();
	static void DeInitialize();
	static bool IsInitialized();
	static TCManager* Get();
private:
	static TCManager *manager;

public:
	void CurrentProfileChanged(wxCommandEvent &event);
	static bool CheckRootFolder(wxFileName path);
	static wxArrayString GetBinariesFromRootFolder(wxFileName path);
	static FSOVersion GetBinaryVersion(wxString binaryname);
	static wxString MakeVersionStringFromVersion(FSOVersion version);
	// Events
public:
	static void RegisterTCChanged(wxEvtHandler *handler);
	static void UnRegisterTCChanged(wxEvtHandler *handler);
	static void RegisterTCBinaryChanged(wxEvtHandler *handler);
	static void UnRegisterTCBinaryChanged(wxEvtHandler *handler);
	static void RegisterTCSelectedModChanged(wxEvtHandler *handler);
	static void UnRegisterTCSelectedModChanged(wxEvtHandler *handler);
	static void GenerateTCChanged();
	static void GenerateTCBinaryChanged();
	static void GenerateTCSelectedModChanged();
private:
	static TCEventHandlers TCChangedHandlers, TCBinaryChangedHandlers, TCSelectedModChangedHandlers;
	DECLARE_EVENT_TABLE();
};
#endif