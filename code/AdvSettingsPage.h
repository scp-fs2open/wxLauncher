#ifndef ADVSETTINGSPAGE_H
#define ADVSETTINGSPAGE_H

#include <wx/wx.h>
#include "Skin.h"
#include "FlagList.h"

class AdvSettingsPage: public wxPanel {
public:
	AdvSettingsPage(wxWindow* parent, SkinSystem* skin);

	void OnCheckFlag(wxCommandEvent &event);

private:
	SkinSystem *skin;
	FlagListBox* flagListBox;
	
public:
	void OnExeChanged(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif