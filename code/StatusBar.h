#ifndef STATUSLOG_H
#define STATUSLOG_H

#include <wx/wx.h>
#include "wxIDS.h"

/** Extends wxStatusBar.  Provides the a 4 section status bar: icon, main text,
progress bar, and small text for the progress bar.  This class will interface
this Logger to enable code to eaisly send messages to the statusbar. */
class StatusBar: public wxStatusBar {
public:
	StatusBar(wxWindow* parent);
	virtual ~StatusBar();

	void OnSize(wxSizeEvent& event);

private:
	wxWindow* parent;
	wxBitmap icons[ID_SB_MAX_ID];
	
	DECLARE_EVENT_TABLE();
};

#endif