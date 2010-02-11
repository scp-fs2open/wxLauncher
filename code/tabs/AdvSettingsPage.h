#ifndef ADVSETTINGSPAGE_H
#define ADVSETTINGSPAGE_H

#include <wx/wx.h>
#include "apis/SkinManager.h"
#include "controls/FlagList.h"

class AdvSettingsPage: public wxPanel {
public:
	AdvSettingsPage(wxWindow* parent, SkinSystem* skin);

	void OnNeedUpdateCommandLine(wxCommandEvent &event);

private:
	SkinSystem *skin;
	FlagListBox* flagListBox;
	
public:
	void OnExeChanged(wxCommandEvent& event);
	void OnSelectFlagSet(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif