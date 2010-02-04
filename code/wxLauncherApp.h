#ifndef WXLAUNCHERAPP_H
#define WXLAUNCHERAPP_H
#include <wx/wx.h>
#include "Skin.h"

class wxLauncher: public wxApp {
public:
	virtual bool OnInit();
	virtual int OnExit();

	SkinSystem* skin;
};

DECLARE_APP(wxLauncher);

#endif