#include <wx/wx.h>
#include <wx/html/htmlwin.h>
#include "WelcomePage.h"
#include "wxIDS.h"

class HeaderBitmap: public wxPanel {
public:
	HeaderBitmap(wxWindow* parent): wxPanel(parent, wxID_ANY) {
		this->bitmap = new wxBitmap(_("SCP Header.bmp"), wxBITMAP_TYPE_BMP);
		wxASSERT(bitmap->IsOk());
		
	}
	virtual void OnPaint(wxPaintEvent& event) {
		wxPaintDC dc(this);

		dc.DrawBitmap(*(this->bitmap), 0, 0);
	}
private:
	wxBitmap* bitmap;
	DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(HeaderBitmap, wxPanel)
	EVT_PAINT(HeaderBitmap::OnPaint)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(WelcomePage, wxWindow)
EVT_HTML_LINK_CLICKED(ID_SUMMARY_HTML_PANEL, WelcomePage::LinkClicked)
EVT_HTML_LINK_CLICKED(ID_HEADLINES_HTML_PANEL, WelcomePage::LinkClicked)
END_EVENT_TABLE()

WelcomePage::WelcomePage(wxWindow* parent): wxWindow(parent, wxID_ANY) {
	this->SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
	// language
	wxStaticText* launcherLanguageText = new wxStaticText(this, wxID_ANY, _("Launcher language:"));
	wxComboBox* launcherLanguageCombo = new wxComboBox(this, wxID_ANY, _("English (US)"));

	wxBoxSizer* languageSizer = new wxBoxSizer(wxHORIZONTAL);
	languageSizer->AddStretchSpacer(2);
	languageSizer->Add(launcherLanguageText);
	languageSizer->Add(launcherLanguageCombo);

	// header image
	HeaderBitmap* header = new HeaderBitmap(this);
	header->SetMinSize(wxSize(600, 100));
	
	// Info
	wxStaticBox* generalBox = new wxStaticBox(this, wxID_ANY, _(""));
	wxHtmlWindow* general = new wxHtmlWindow(this, ID_SUMMARY_HTML_PANEL, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_NEVER );
	general->SetPage(_("<p>Welcome to the FreeSpace Open Launcher, your one-stop-shop for Freespace 2 and releate content</p>\
					   <p>If you're new to FreeSpace, you might want to check these links first:\
					   <ul>\
					   <li><a href='http://www.hard-light.net/wiki/index.php/Main_Page'>Wiki</a></li>\
					   <li><a href='http://www.hard-light.net/forums/index.php/board,50.0.html'>Forums</a></li>\
					   <li><a href='http://en.wikipedia.org/wiki/FreeSpace_2'>Wikipedia FS2 Page</a></li></ul>\
					   </p>\
					   <p>Select a profile below and hit Play to start the game."));
	
	wxStaticBoxSizer* generalSizer = new wxStaticBoxSizer(generalBox, wxVERTICAL);
	generalSizer->SetMinSize(wxSize(600, 200));
	generalSizer->Add(general, 1, wxEXPAND);

	// Profiles
	wxStaticBox* profileBox = new wxStaticBox(this, wxID_ANY, _("Profile"));
	wxComboBox* profileCombo = new wxComboBox(this, ID_PROFILE_COMBO, _("Default"));
	wxButton* newButton = new wxButton(this, ID_NEW_PROFILE, _("New"));
	wxButton* deleteButton = new wxButton(this, ID_DELETE_PROFILE, _("Delete"));
	wxButton* saveButton = new wxButton(this, ID_SAVE_PROFILE, _("Save"));
	wxCheckBox* saveDefaultCheck = new wxCheckBox(this, ID_SAVE_DEFAULT_COMBO, _("Always save default"));
	wxBoxSizer* profileButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
	profileButtonsSizer->Add(newButton);
	profileButtonsSizer->Add(deleteButton);
	profileButtonsSizer->Add(saveButton);
	profileButtonsSizer->AddStretchSpacer(1);
	profileButtonsSizer->Add(saveDefaultCheck, 0, wxALIGN_CENTER_VERTICAL);

	wxStaticBoxSizer* profileVerticalSizer = new wxStaticBoxSizer(profileBox, wxVERTICAL);
	profileVerticalSizer->Add(profileCombo, 0, wxALL | wxEXPAND, 4);
	profileVerticalSizer->Add(profileButtonsSizer, 0, wxALL | wxEXPAND, 4);
	profileVerticalSizer->SetMinSize(wxSize(600, -1));

	// Latest headlines
	wxStaticBox* headlinesBox = new wxStaticBox(this, wxID_ANY, _("Latest headlines from the front"));
	wxHtmlWindow* headlinesView = new wxHtmlWindow(this, ID_HEADLINES_HTML_PANEL);
	headlinesView->SetPage(_("<ul>\
							 <li><a href='http://www.hard-light.net/forums/index.php?topic=65861.0'>Vidmaster finally released his FortuneHunters 2261 campaign.</a></li>\
							 <li><a href='http://www.hard-light.net/forums/index.php?topic=65667.0'>Komet has released a Japanese Localization Patch for FreeSpace 2!</a></li>\
							 <li><a href='http://www.hard-light.net/forums/index.php?topic=65671.0'>The 158th Banshee Squadron have released Exposition, the first episode of their \"Into the Night\" series.</a></li>\
							 <li><a href='http://www.hard-light.net/forums/index.php?topic=65671.0'>The 158th Banshee Squadron have released Exposition, the first episode of their \"Into the Night\" series.</a></li>\
							 </ul>"));

	wxStaticBoxSizer* headlines = new wxStaticBoxSizer(headlinesBox, wxVERTICAL);
	headlines->SetMinSize(wxSize(600, 150));
	headlines->Add(headlinesView, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL);

	// Final layout
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(languageSizer);
	sizer->Add(header);
	sizer->Add(generalSizer);
	sizer->Add(profileVerticalSizer);
	sizer->Add(headlines);

	this->SetSizer(sizer);
	this->Layout();
}

void WelcomePage::LinkClicked(wxHtmlLinkEvent &event) {
	wxHtmlLinkInfo info = event.GetLinkInfo();
	wxLaunchDefaultBrowser(info.GetHref());
}
