#ifndef STATUSLOG_H
#define STATUSLOG_H

#include <wx/wx.h>
#include "global/ids.h"

/** Extends wxStatusBar.  Provides the a 4 section status bar: icon, main text,
progress bar, and small text for the progress bar.  This class will interface
this Logger to enable code to eaisly send messages to the statusbar. */
class StatusBar: public wxStatusBar {
public:
	StatusBar(wxWindow* parent);
	virtual ~StatusBar();

	void OnSize(wxSizeEvent& event);

	void SetMainStatusText(wxString msg, int icon=ID_SB_NO_CHANGE);
	void SetJobStatusText(int value, wxString msg=_T(""));
	void StartToolTipStatusText(wxString msg);
	void EndToolTipStatusText();

private:
	wxWindow* parent;
	wxBitmap icons[ID_SB_MAX_ID];
	bool showingToolTip;
	
	DECLARE_EVENT_TABLE();
};

#endif