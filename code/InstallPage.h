#ifndef INSTALLPAGE_H
#define INSTALLPAGE_H

#include <wx/wx.h>

class InstallPage: public wxPanel {
public:
	InstallPage(wxWindow* parent);

	void OnDownloadNewsCheck(wxCommandEvent& event);
	void OnUpdateNewsHelp(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif