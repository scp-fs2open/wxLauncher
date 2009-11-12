#include <wx/wx.h>
#include <wx/html/htmlwin.h>
#include <wx/valgen.h>
#include <wx/valtext.h>
#include "WelcomePage.h"
#include "wxIDS.h"
#include "Skin.h"
#include "StatusBar.h"
#include "ProfileManager.h"

#include "wxLauncherSetup.h" // Last include for memory debugging

class CloneProfileDialog: public wxDialog {
public:
	CloneProfileDialog(wxWindow* parent, wxString orignalname, wxString desinationName);
	wxString GetOriginalName();
	wxString GetTargetName();
private:
	wxString target;
	int fromNumber;
	wxChoice *cloneFrom;
};

class DeleteProfileDialog: public wxDialog {
public:
	DeleteProfileDialog(wxWindow* parent, wxString name);
};

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

EVT_COMMAND( wxID_NONE, EVT_PROFILE_CHANGE, WelcomePage::ProfileCountChanged)
EVT_COMMAND( wxID_NONE, EVT_CURRENT_PROFILE_CHANGED, WelcomePage::ProfileCountChanged)

// Profile controls
EVT_BUTTON(ID_NEW_PROFILE, WelcomePage::ProfileButtonClicked)
EVT_BUTTON(ID_DELETE_PROFILE, WelcomePage::ProfileButtonClicked)
EVT_BUTTON(ID_SAVE_PROFILE, WelcomePage::ProfileButtonClicked)
EVT_CHECKBOX(ID_SAVE_DEFAULT_CHECK, WelcomePage::SaveDefaultChecked)
EVT_COMBOBOX(ID_PROFILE_COMBO, WelcomePage::ProfileChanged)
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
	wxChoice* profileCombo = new wxChoice(this, ID_PROFILE_COMBO,
		wxDefaultPosition,
		wxDefaultSize,
		0,	// number of choices
		0,	// choices
		wxCB_SORT);
	profileCombo->Append(profile->GetAllProfileNames());
	profile->AddEventHandler(this);

	wxString lastselected;
	profile->Global()->Read(GBL_CFG_MAIN_LASTPROFILE, &lastselected, _T("Default"));
	profileCombo->SetStringSelection(lastselected);

	wxButton* newButton = new wxButton(this, ID_NEW_PROFILE, _("New"));
	wxButton* deleteButton = new wxButton(this, ID_DELETE_PROFILE, _("Delete"));
	wxButton* saveButton = new wxButton(this, ID_SAVE_PROFILE, _("Save"));
	wxCheckBox* saveDefaultCheck = new wxCheckBox(this, ID_SAVE_DEFAULT_CHECK, _("Always save default"));
	bool autosave;
	profile->Global()->Read(GBL_CFG_MAIN_AUTOSAVEPROFILES, &autosave, true);
	saveDefaultCheck->SetValue(autosave);

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

/** Calls the dialogs for cloning, saving or deleting profiles. */
void WelcomePage::ProfileButtonClicked(wxCommandEvent& event) {
	wxComboBox* combobox = dynamic_cast<wxComboBox*>(wxWindow::FindWindowById(ID_PROFILE_COMBO, this));
	ProMan *profile = ProMan::GetProfileManager();

	switch(event.GetId()) {
		case ID_NEW_PROFILE:
			cloneNewProfile(combobox, profile);
			break;

		case ID_DELETE_PROFILE:
			deleteProfile(combobox, profile);
			break;

		case ID_SAVE_PROFILE:
			profile->SaveCurrentProfile();
			break;
		default:
			wxCHECK_RET( false, _("Reached impossible location.\nHandler has been attached to a button that it cannot handle."));
	}
}

void WelcomePage::SaveDefaultChecked(wxCommandEvent& event) {
	wxButton* saveButton = dynamic_cast<wxButton*>(wxWindow::FindWindowById(ID_SAVE_PROFILE, this));
	wxCHECK_RET( saveButton != NULL , _("SaveDefaultChecked is unable to get the SaveButton") );

	ProMan* profile = ProMan::GetProfileManager();

	if ( event.IsChecked() ) {
		// I am to save all changes, so force save and disable the save button.
		saveButton->Disable();
		
		profile->Global()->Write(GBL_CFG_MAIN_AUTOSAVEPROFILES, true);
		profile->SaveCurrentProfile();
		wxLogStatus(_("Now autosaving profiles."));
	} else {
		saveButton->Enable();
		
		profile->Global()->Write(GBL_CFG_MAIN_AUTOSAVEPROFILES, false);
		wxLogStatus(_("No longer autosaving profiles."));
	}
}
void WelcomePage::ProfileChanged(wxCommandEvent& event) {
	wxComboBox* saveButton = dynamic_cast<wxComboBox*>(wxWindow::FindWindowById(ID_PROFILE_COMBO, this));
	wxString newProfile = saveButton->GetValue();
	ProMan* proman = ProMan::GetProfileManager();

	if ( proman->DoesProfileExist(newProfile) ) {
		if ( proman->NeedToPromptToSave() ) {
			int response = wxMessageBox(
				wxString::Format(
					_("There are unsaved changes to your profile '%s'.\n\nWould you like to save your changes?"),
					proman->GetCurrentName()),
				_("Save profile changes?"), wxYES_NO, this);

			if ( response == wxYES ) {
				proman->SaveCurrentProfile();
			}
		}
		if ( proman->SwitchTo(newProfile) ) {
			wxLogMessage(_T("Profile %s is now the active profile."), proman->GetCurrentName());
		} else {
			wxLogWarning(_T("Unable to switch to %s, staying on %s."), newProfile, proman->GetCurrentName());
		}
	} else {
		wxLogWarning(_T("Profile does not exist. Use Clone to create profile first"));
	}
}

void WelcomePage::cloneNewProfile(wxComboBox* combobox, ProMan* profile) {
	wxString originalName;
	wxString targetName;

	/* If the user has edited the combobox we will put the text that they 
	editied into the new name box in the dialog and then they would only have
	to choose the profile to choose from. The default from target would be
	the currently active profile.

	If the current value of the combobox is a valid profile name then we will
	use that as the profile to clone from, leaving the newname blank.
	*/
	wxLogDebug(_T("Combo's text box contains '%s'."), combobox->GetValue());
	if ( profile->DoesProfileExist(combobox->GetValue()) ) {
		// is a vaild profile
		originalName = combobox->GetValue();
		wxLogDebug(_T("  Which is an existing profile."));
	} else {
		targetName = combobox->GetValue();
		originalName = profile->GetCurrentName();
		wxLogDebug(_T("  Which is not an existing profile."));
	}

	CloneProfileDialog cloneDialog(this, originalName, targetName);

	if ( cloneDialog.ShowModal() == cloneDialog.GetAffirmativeId() ) {
		wxLogDebug(_T("User clicked clone"));
		if ( profile->CloneProfile(
			cloneDialog.GetOriginalName(),
			cloneDialog.GetTargetName()) ) {
				wxLogStatus(_("Cloned profile '%s' from '%s'"),
					cloneDialog.GetOriginalName(),
					cloneDialog.GetTargetName());
				profile->SwitchTo(cloneDialog.GetTargetName());
		} else {
				wxLogError(_("Unable to clone profile '%s' from '%s'. See log for details."),
					cloneDialog.GetOriginalName(),
					cloneDialog.GetTargetName());							
		}
	} else {
		wxLogStatus(_("Profile clone aborted"));
	}
}


void WelcomePage::deleteProfile(wxComboBox* combobox, ProMan* profile) {
	wxString nametodelete = combobox->GetValue();
	
	if ( profile->DoesProfileExist(nametodelete) ) {
		DeleteProfileDialog deleteDialog(this, nametodelete);
		
		if ( deleteDialog.ShowModal() == deleteDialog.GetAffirmativeId()) {
			if ( profile->DeleteProfile(nametodelete) ) {
				wxLogStatus(_("Deleted profile named '%s'."), nametodelete);
			} else {
				wxLogWarning(_("Unable to delete profile '%s', see log for more details."), nametodelete);
			}
		} else {
			wxLogStatus(_("Deletion of profile '%s' cancelled"), nametodelete);
		}
	} else {
		wxLogWarning(_T("Unable to delete non existant profile '%s'"), nametodelete);
	}
}

void WelcomePage::ProfileCountChanged(wxCommandEvent &event) {
	WXUNUSED(event);
	wxComboBox* combobox = dynamic_cast<wxComboBox*>(wxWindow::FindWindowById(ID_PROFILE_COMBO, this));
	ProMan *profile = ProMan::GetProfileManager();

	wxLogStatus(_T("ProfileCountChanged"));

	combobox->Clear();
	combobox->Append(profile->GetAllProfileNames());
	combobox->SetStringSelection(profile->GetCurrentName());
}


///////////////////////////////////////////////////////////////////////////////
///// DIALOGS ///
CloneProfileDialog::CloneProfileDialog(wxWindow* parent, wxString orignalName, wxString destName):
wxDialog(parent, ID_CLONE_PROFILE_DIALOG, _("New profile..."), wxDefaultPosition, wxDefaultSize) {
	this->target = destName;

	wxStaticText *newNameText = new wxStaticText(this, wxID_ANY, _("New Profile Name:"));
	wxTextCtrl *newName = new wxTextCtrl(this, wxID_ANY, this->target, wxDefaultPosition, wxSize(200,-1));
	
	wxSizer* nameSizer = new wxFlexGridSizer(2);
	nameSizer->Add(newNameText);
	nameSizer->Add(newName);

	wxStaticText *cloneFromText = new wxStaticText(this, wxID_ANY, _("Clone settings from:"));
	cloneFrom = new wxChoice(this, wxID_ANY);

	nameSizer->Add(cloneFromText);
	nameSizer->Add(cloneFrom);

	wxButton *createButton = new wxButton(this, wxID_OK, _("Clone"));
	wxButton *closeButton = new wxButton(this, wxID_CANCEL, _("Close"));

	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	buttonSizer->Add(createButton);
	buttonSizer->Add(closeButton);

	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(nameSizer, wxSizerFlags().Expand().Border(wxALL, 5));
	sizer->AddSpacer(15);
	sizer->Add(buttonSizer, wxSizerFlags().Right());

	this->SetSizer(sizer);

	newName->SetValidator(wxTextValidator(wxFILTER_NONE, &(this->target)));
	cloneFrom->SetValidator(wxGenericValidator(&(this->fromNumber)));

	cloneFrom->Append(ProMan::GetProfileManager()->GetAllProfileNames());
	cloneFrom->SetStringSelection(orignalName);

	this->Fit();
	this->Center();
	wxLogDebug(_T("Clone Profile Dialog Created"));
}

wxString CloneProfileDialog::GetTargetName() {
	return this->target;
}

wxString CloneProfileDialog::GetOriginalName() {
	return cloneFrom->GetStringSelection();
}

DeleteProfileDialog::DeleteProfileDialog(wxWindow* parent, wxString name):
wxDialog(parent, ID_DELETE_PROFILE_DIALOG, _("Delete profile..."), wxDefaultPosition, wxDefaultSize) {
	wxStaticText* text = new wxStaticText(this, wxID_ANY,
		wxString::Format(_("Are you sure you would like to delete profile %s"), name));

	wxButton *deleteButton = new wxButton(this, wxID_ANY, _("Delete"));
	wxButton *cancelButton = new wxButton(this, wxID_ANY, _("Cancel"));

	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	buttonSizer->Add(deleteButton);
	buttonSizer->Add(cancelButton);

	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(text);
	sizer->Add(buttonSizer, wxSizerFlags().Right());

	this->SetSizer(sizer);
	this->Layout();

	this->SetAffirmativeId(deleteButton->GetId());
	this->SetEscapeId(cancelButton->GetId());

	wxLogDebug(_T("Delete Profile Dialog created."));
}
	