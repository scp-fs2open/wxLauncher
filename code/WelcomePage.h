#ifndef WELCOMEPAGE_H
#define WELCOMEPAGE_H

#include <wx/wx.h>

class WelcomePage: public wxWindow {
public:
	WelcomePage(wxWindow* parent);
	void LinkClicked(wxHtmlLinkEvent& event);
	
	DECLARE_EVENT_TABLE();
};

#endif