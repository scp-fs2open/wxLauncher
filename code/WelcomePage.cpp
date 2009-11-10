#include <wx/wx.h>
#include <wx/html/htmlwin.h>
#include "WelcomePage.h"
#include "wxIDS.h"
#include "Skin.h"
#include "StatusBar.h"
#include "ProfileManager.h"

#include "wxLauncherSetup.h" // Last include for memory debugging

/** Class that manages the header image for the welcome tab. */
class HeaderBitmap: public wxPanel {
public:
	HeaderBitmap(wxWindow* parent, int width, SkinSystem* skin): wxPanel(parent, wxID_ANY) {
		this->bitmap = skin->GetBanner();
		wxASSERT_MSG(this->bitmap.IsOk(), _("Loaded bitmap is invalid."));

		wxASSERT_MSG(this->bitmap.GetWidth() <= width,
			(wxString::Format(_("Header bitmap is larger than %d pixels!"), width)));

		this->SetMinSize(wxSize(width, bitmap.GetHeight()));
	}
	virtual void OnPaint(wxPaintEvent& event) {
		wxPaintDC dc(this);
		dc.DrawBitmap(this->bitmap, (this->GetSize().GetWidth()/2) - (this->bitmap.GetWidth()/2), 0);
	}
private:
	wxBitmap bitmap;
	DECLARE_EVENT_TABLE();
};

BEGIN_EVENT_TABLE(HeaderBitmap, wxPanel)
	EVT_PAINT(HeaderBitmap::OnPaint)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(WelcomePage, wxWindow)
EVT_HTML_LINK_CLICKED(ID_SUMMARY_HTML_PANEL, WelcomePage::LinkClicked)
EVT_HTML_CELL_HOVER(ID_SUMMARY_HTML_PANEL, WelcomePage::LinkHover)
EVT_HTML_LINK_CLICKED(ID_HEADLINES_HTML_PANEL, WelcomePage::LinkClicked)
EVT_HTML_CELL_HOVER(ID_HEADLINES_HTML_PANEL, WelcomePage::LinkHover)
END_EVENT_TABLE()

WelcomePage::WelcomePage(wxWindow* parent, SkinSystem* skin): wxWindow(parent, wxID_ANY) {
	// member varirable init
	this->lastLinkInfo = NULL;
	ProMan* profile = ProMan::GetProfileManager();

	// language
	wxStaticText* launcherLanguageText = new wxStaticText(this, wxID_ANY, _("Launcher language:"));
	wxComboBox* launcherLanguageCombo = new wxComboBox(this, wxID_ANY, _("English (US)"));

	wxBoxSizer* languageSizer = new wxBoxSizer(wxHORIZONTAL);
	languageSizer->AddStretchSpacer(2);
	languageSizer->Add(launcherLanguageText);
	languageSizer->Add(launcherLanguageCombo);

	// header image
	HeaderBitmap* header = new HeaderBitmap(this, this->stuffWidth, skin);
	
	// Info
	wxStaticBox* generalBox = new wxStaticBox(this, wxID_ANY, _(""));
	wxHtmlWindow* general = new wxHtmlWindow(this, ID_SUMMARY_HTML_PANEL, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_NEVER );
	general->SetPage(_("<p><center><b><font size='3'>Welcome to  wxLauncher, your one-stop-shop for Freespace 2 and related content</font></b><br><br>\
						If you’re  new to the Freespace 2 universe, you might want to check out these links first:<br>\
						= <a href='http://www.hard-light.net/wiki/index.php/Main_Page'>FS2 Wiki</a>  \
						= <a href='http://www.hard-light.net/forums/'>FS2 Forum</a> \
						= <a href='http://en.wikipedia.org/wiki/FreeSpace_2'>Wikipedia FS2 page</a> \
						= <a href='http://scp.indiegames.us/mantis/main_page.php'> Reporting bugs</a> =<br><br>\
						Also, don’t  forget the help file, there is a nice 'Getting Started' tutorial there.<br>\
						</center></p>"));
	general->Connect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(WelcomePage::OnMouseOut));
	
	wxStaticBoxSizer* generalSizer = new wxStaticBoxSizer(generalBox, wxVERTICAL);
	generalSizer->SetMinSize(wxSize(this->stuffWidth, 200));
	generalSizer->Add(general, 1, wxEXPAND);

	// Profiles
	wxStaticBox* profileBox = new wxStaticBox(this, wxID_ANY, _("Profile"));
	wxComboBox* profileCombo = new wxComboBox(this, ID_PROFILE_COMBO,
		wxEmptyString,	// value,
		wxDefaultPosition,
		wxDefaultSize,
		0,	// number of choices
		0,	// choices
		wxCB_SORT);
	profileCombo->Append(profile->GetAllProfileNames());

	wxString lastselected;
	profile->Global()->Read(_T("/main/lastprofile"), &lastselected, _T("Default"));
	profileCombo->SetValue(lastselected);

	wxButton* newButton = new wxButton(this, ID_NEW_PROFILE, _("Clone"));
	wxButton* deleteButton = new wxButton(this, ID_DELETE_PROFILE, _("Delete"));
	wxButton* saveButton = new wxButton(this, ID_SAVE_PROFILE, _("Save"));
	wxCheckBox* saveDefaultCheck = new wxCheckBox(this, ID_SAVE_DEFAULT_CHECK, _("Always save default"));

	wxBoxSizer* profileButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
	profileButtonsSizer->Add(newButton);
	profileButtonsSizer->Add(deleteButton);
	profileButtonsSizer->Add(saveButton);
	profileButtonsSizer->AddStretchSpacer(1);
	profileButtonsSizer->Add(saveDefaultCheck, 0, wxALIGN_CENTER_VERTICAL);

	wxStaticBoxSizer* profileVerticalSizer = new wxStaticBoxSizer(profileBox, wxVERTICAL);
	profileVerticalSizer->Add(profileCombo, 0, wxALL | wxEXPAND, 4);
	profileVerticalSizer->Add(profileButtonsSizer, 0, wxALL | wxEXPAND, 4);
	profileVerticalSizer->SetMinSize(wxSize(this->stuffWidth, -1));

	// Latest headlines
	wxStaticBox* headlinesBox = new wxStaticBox(this, wxID_ANY, _("Latest headlines from the front"));
	wxHtmlWindow* headlinesView = new wxHtmlWindow(this, ID_HEADLINES_HTML_PANEL);
	headlinesView->SetPage(_("<ul>\
							 <li><a href='http://www.hard-light.net/forums/index.php?topic=65861.0'>Vidmaster finally released his FortuneHunters 2261 campaign.</a></li>\
							 <li><a href='http://www.hard-light.net/forums/index.php?topic=65667.0'>Komet has released a Japanese Localization Patch for FreeSpace 2!</a></li>\
							 <li><a href='http://www.hard-light.net/forums/index.php?topic=65671.0'>The 158th Banshee Squadron have released Exposition, the first episode of their \"Into the Night\" series.</a></li>\
							 <li><a href='http://www.hard-light.net/forums/index.php?topic=65671.0'>The 158th Banshee Squadron have released Exposition, the first episode of their \"Into the Night\" series.</a></li>\
							 </ul>"));
	headlinesView->Connect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(WelcomePage::OnMouseOut));

	wxStaticBoxSizer* headlines = new wxStaticBoxSizer(headlinesBox, wxVERTICAL);
	headlines->SetMinSize(wxSize(this->stuffWidth, 150));
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

void WelcomePage::LinkHover(wxHtmlCellEvent &event) {
	wxHtmlLinkInfo* info = event.GetCell()->GetLink(); // will be NULL if not a link.

	wxFrame *frame = dynamic_cast<wxFrame*>(this->GetParent()->GetParent());
	wxASSERT( frame != NULL );
	StatusBar *bar = dynamic_cast<StatusBar*>(frame->GetStatusBar());
	wxASSERT( bar != NULL );
	if (info == NULL) {
		if ( this->lastLinkInfo == NULL ) {
			// do nothing
		} else {
			bar->EndToolTipStatusText();
			this->lastLinkInfo = NULL;
		}
	} else  {
		if ( info == this->lastLinkInfo ) {
			// do nothing
		} else if ( this->lastLinkInfo == NULL ) {
			bar->StartToolTipStatusText(info->GetHref());
			this->lastLinkInfo = reinterpret_cast<void *>(info);
		} else {
			// still on a URL but we potentially changed the URL
			bar->EndToolTipStatusText();
			bar->StartToolTipStatusText(info->GetHref());
			this->lastLinkInfo = reinterpret_cast<void *>(info);
		}
	}
}

void WelcomePage::OnMouseOut(wxMouseEvent &event) {
	WXUNUSED(event);
	// clear url from status bar.
	wxFrame *frame = dynamic_cast<wxFrame*>(this->GetParent()->GetParent()->GetParent());
	wxASSERT( frame != NULL );
	StatusBar *bar = dynamic_cast<StatusBar*>(frame->GetStatusBar());
	wxASSERT( bar != NULL );
	bar->EndToolTipStatusText();
	this->lastLinkInfo = NULL;
}
