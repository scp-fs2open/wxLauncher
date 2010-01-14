#ifndef TCMANAGER_H
#define TCMANAGER_H

#include <wx/wx.h>
#include <wx/clntdata.h>
#include <wx/filename.h>

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