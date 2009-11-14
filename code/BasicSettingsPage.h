#ifndef BASICSETTINGSPAGE_H
#define BASICSETTINGSPAGE_H

#include <wx/wx.h>

class BasicSettingsPage : public wxPanel {
public:
	BasicSettingsPage(wxWindow* parent);
	~BasicSettingsPage();

	void OnSelectTC(wxCommandEvent &event);
	void OnTCChanged(wxCommandEvent &event);

	void OnSelectExecutable(wxCommandEvent &event);
	void OnSelectResolution(wxCommandEvent &event);

private:
	void FillExecutableDropBox(wxChoice* exeChoice, wxFileName path);
	void FillResolutionDropBox(wxChoice* exeChoice);
	DECLARE_EVENT_TABLE();
};

#endif