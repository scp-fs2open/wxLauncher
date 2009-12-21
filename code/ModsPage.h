#ifndef MODSPAGE_H
#define MODSPAGE_H

#include <wx/wx.h>
#include "Skin.h"

class ModsPage: public wxPanel {
public:
	ModsPage(wxWindow* parent, SkinSystem *skin);

	void OnTCChanged(wxCommandEvent &event);

private:
	SkinSystem* skin;
	DECLARE_EVENT_TABLE();
};

#endif