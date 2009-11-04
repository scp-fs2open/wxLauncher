#ifndef WELCOMEPAGE_H
#define WELCOMEPAGE_H

#include <wx/wx.h>

#include "Skin.h"

class WelcomePage: public wxWindow {
public:
	WelcomePage(wxWindow* parent, SkinSystem* skin);
	void LinkClicked(wxHtmlLinkEvent& event);
	
private:
	/** The width of the items on the welcome tab. */
	const static int stuffWidth = 639;
	DECLARE_EVENT_TABLE();
};

#endif