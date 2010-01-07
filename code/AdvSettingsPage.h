#ifndef ADVSETTINGSPAGE_H
#define ADVSETTINGSPAGE_H

#include <wx/wx.h>
#include "Skin.h"

class AdvSettingsPage: public wxPanel {
public:
	AdvSettingsPage(wxWindow* parent, SkinSystem* skin);

private:
	SkinSystem *skin;
	
public:
	void OnExeChanged(wxCommandEvent& event);

	DECLARE_EVENT_TABLE()
};

#endif