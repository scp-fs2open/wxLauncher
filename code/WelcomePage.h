#ifndef WELCOMEPAGE_H
#define WELCOMEPAGE_H

#include <wx/wx.h>

#include "Skin.h"
#include "wxIDS.h"

class WelcomePage: public wxWindow {
public:
	WelcomePage(wxWindow* parent, SkinSystem* skin);
	void LinkClicked(wxHtmlLinkEvent& event);
	
private:
	/** The width of the items on the welcome tab. */
	const static int stuffWidth = TAB_AREA_WIDTH;
	DECLARE_EVENT_TABLE();
};

#endif