/*
Copyright (C) 2009-2010 wxLauncher Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <wx/wx.h>
#include <wx/html/htmlwin.h>
#include <wx/valgen.h>
#include <wx/valtext.h>
#include <wx/datetime.h>
#include <wx/sstream.h>
#include <wx/tokenzr.h>

#include "generated/configure_launcher.h"
#include "tabs/WelcomePage.h"
#include "controls/StatusBar.h"
#include "apis/HelpManager.h"

#include "global/MemoryDebugging.h" // Last include for memory debugging

#define TIME_BETWEEN_NEWS_UPDATES			wxTimeSpan::Day()

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
	virtual void OnPaint(wxPaintEvent& WXUNUSED(event)) {
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
EVT_CHOICE(ID_PROFILE_COMBO, WelcomePage::ProfileChanged)

EVT_CHECKBOX(ID_NET_DOWNLOAD_NEWS, WelcomePage::OnDownloadNewsCheck)

EVT_IDLE(WelcomePage::UpdateNews)
END_EVENT_TABLE()

WelcomePage::WelcomePage(wxWindow* parent, SkinSystem* skin): wxWindow(parent, wxID_ANY) {
	// member varirable init
	this->lastLinkInfo = NULL;
	ProMan* profile = ProMan::GetProfileManager();

	// language
	wxStaticText* launcherLanguageText = new wxStaticText(this, wxID_ANY, _("Launcher language:"));
	wxChoice* launcherLanguageCombo = new wxChoice(this, wxID_ANY);
	launcherLanguageCombo->Insert(_("English (US)"), 0);
	launcherLanguageCombo->SetSelection(0);
	launcherLanguageCombo->Disable();

	wxBoxSizer* languageSizer = new wxBoxSizer(wxHORIZONTAL);
	languageSizer->AddStretchSpacer(2);
	languageSizer->Add(launcherLanguageText);
	languageSizer->Add(launcherLanguageCombo);

	// header image
	HeaderBitmap* header = new HeaderBitmap(this, this->stuffWidth, skin);
	
	// Info
	wxStaticBox* generalBox = new wxStaticBox(this, wxID_ANY, _(""));
	wxHtmlWindow* general = new wxHtmlWindow(this, ID_SUMMARY_HTML_PANEL, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_NEVER );
	general->SetPage(skin->GetWelcomePageText());
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
	wxCheckBox* saveDefaultCheck = new wxCheckBox(this, ID_SAVE_DEFAULT_CHECK, _("Automatically save profile"));
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
	wxStaticBox* headlinesBox = new wxStaticBox(this, wxID_ANY, _("Latest highlights from the front"));
	wxHtmlWindow* headlinesView = new wxHtmlWindow(this, ID_HEADLINES_HTML_PANEL);
	headlinesView->SetPage(_T(""));
	headlinesView->Connect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(WelcomePage::OnMouseOut));
	wxCheckBox* updateNewsCheck = new wxCheckBox(this, ID_NET_DOWNLOAD_NEWS, _("Auto update highlights"));
	updateNewsCheck->SetToolTip(_("Check this to have the Launcher update the highlights on next run"));
	updateNewsCheck->SetValue(this->getOrPromptUpdateNews());
	this->needToUpdateNews = true;

	wxStaticBoxSizer* headlines = new wxStaticBoxSizer(headlinesBox, wxVERTICAL);
	headlines->SetMinSize(wxSize(this->stuffWidth, 150));
	headlines->Add(headlinesView, 
		wxSizerFlags().Expand().Center().Proportion(1));
	headlines->Add(updateNewsCheck, wxSizerFlags().Right());

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
	wxString rest;
	if (info.GetHref().StartsWith(_T("help://"), &rest)) {
		HelpManager::OpenHelpByString(rest);
	} else {
		wxLaunchDefaultBrowser(info.GetHref());
	}
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

void WelcomePage::OnMouseOut(wxMouseEvent &WXUNUSED(event)) {
	WXUNUSED(event);
	// clear url from status bar.
	wxFrame *frame = dynamic_cast<wxFrame*>(this->GetParent()->GetParent()->GetParent());
	wxASSERT( frame != NULL );
	StatusBar *bar = dynamic_cast<StatusBar*>(frame->GetStatusBar());
	wxASSERT( bar != NULL );
	bar->EndToolTipStatusText();
}

/** Calls the dialogs for cloning, saving or deleting profiles. */
void WelcomePage::ProfileButtonClicked(wxCommandEvent& event) {
	wxChoice* combobox = dynamic_cast<wxChoice*>(wxWindow::FindWindowById(ID_PROFILE_COMBO, this));
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
void WelcomePage::ProfileChanged(wxCommandEvent& WXUNUSED(event)) {
	wxChoice* saveButton = dynamic_cast<wxChoice*>(wxWindow::FindWindowById(ID_PROFILE_COMBO, this));
	wxString newProfile = saveButton->GetStringSelection();
	ProMan* proman = ProMan::GetProfileManager();

	if ( proman->DoesProfileExist(newProfile) ) {
		if ( proman->NeedToPromptToSave() ) {
			int response = wxMessageBox(
				wxString::Format(
					_("There are unsaved changes to your profile '%s'.\n\nWould you like to save your changes?"),
					proman->GetCurrentName().c_str()),
				_("Save profile changes?"), wxYES_NO, this);

			if ( response == wxYES ) {
				proman->SaveCurrentProfile();
			}
		}
		if ( proman->SwitchTo(newProfile) ) {
			wxLogMessage(_T("Profile %s is now the active profile."), proman->GetCurrentName().c_str());
		} else {
			wxLogWarning(_T("Unable to switch to %s, staying on %s."), newProfile.c_str(), proman->GetCurrentName().c_str());
		}
	} else {
		wxLogWarning(_T("Profile does not exist. Use Clone to create profile first"));
	}
}

void WelcomePage::cloneNewProfile(wxChoice* combobox, ProMan* profile) {
	wxString originalName;
	wxString targetName;

	/* If the user has edited the combobox we will put the text that they 
	editied into the new name box in the dialog and then they would only have
	to choose the profile to choose from. The default from target would be
	the currently active profile.

	If the current value of the combobox is a valid profile name then we will
	use that as the profile to clone from, leaving the newname blank.
	*/
	wxLogDebug(_T("Combo's text box contains '%s'."), combobox->GetStringSelection().c_str());
	if ( profile->DoesProfileExist(combobox->GetStringSelection()) ) {
		// is a vaild profile
		originalName = combobox->GetStringSelection();
		wxLogDebug(_T("  Which is an existing profile."));
	} else {
		targetName = combobox->GetStringSelection();
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
					cloneDialog.GetOriginalName().c_str(),
					cloneDialog.GetTargetName().c_str());
				profile->SwitchTo(cloneDialog.GetTargetName());
		} else {
				wxLogError(_("Unable to clone profile '%s' from '%s'. See log for details."),
					cloneDialog.GetOriginalName().c_str(),
					cloneDialog.GetTargetName().c_str());							
		}
	} else {
		wxLogStatus(_("Profile clone aborted"));
	}
}


void WelcomePage::deleteProfile(wxChoice* combobox, ProMan* profile) {
	wxString nametodelete = combobox->GetStringSelection();
	
	if ( profile->DoesProfileExist(nametodelete) ) {
		DeleteProfileDialog deleteDialog(this, nametodelete);
		
		if ( deleteDialog.ShowModal() == deleteDialog.GetAffirmativeId()) {
			if ( profile->DeleteProfile(nametodelete) ) {
				wxLogStatus(_("Deleted profile named '%s'."), nametodelete.c_str());
			} else {
				wxLogWarning(_("Unable to delete profile '%s', see log for more details."), nametodelete.c_str());
			}
		} else {
			wxLogStatus(_("Deletion of profile '%s' cancelled"), nametodelete.c_str());
		}
	} else {
		wxLogWarning(_T("Unable to delete non existant profile '%s'"), nametodelete.c_str());
	}
}

void WelcomePage::ProfileCountChanged(wxCommandEvent &WXUNUSED(event)) {
	wxChoice* combobox = dynamic_cast<wxChoice*>(wxWindow::FindWindowById(ID_PROFILE_COMBO, this));
	wxCHECK_RET(combobox != NULL, _T("can't find the profile combobox"));
	ProMan *profile = ProMan::GetProfileManager();

	combobox->Clear();
	combobox->Append(profile->GetAllProfileNames());
	combobox->SetStringSelection(profile->GetCurrentName());
}

void WelcomePage::UpdateNews(wxIdleEvent& WXUNUSED(event)) {
	if ( !this->needToUpdateNews ) {
		return;
	}
	this->needToUpdateNews = false;
	wxHtmlWindow* newsWindow = dynamic_cast<wxHtmlWindow*>(wxWindow::FindWindowById(ID_HEADLINES_HTML_PANEL, this));
	wxCHECK_RET(newsWindow != NULL, _T("Update news called, but can't find the news window"));

	ProMan* profile = ProMan::GetProfileManager();

	bool allowedToUpdateNews;
	if ( !profile->Global()->Read(GBL_CFG_NET_DOWNLOAD_NEWS, &allowedToUpdateNews)) {
		return;
	}
	if (allowedToUpdateNews) {
		wxString lastTimeString;
		profile->Global()->Read(GBL_CFG_NET_NEWS_LAST_TIME, &lastTimeString);
		wxDateTime lasttime;
		if ( (NULL != lasttime.ParseFormat(lastTimeString, NEWS_LAST_TIME_FORMAT) )
			&& (wxDateTime::Now() - lasttime < TIME_BETWEEN_NEWS_UPDATES) 
			&& (profile->Global()->Exists(GBL_CFG_NET_THE_NEWS)) ) {
			// post the news that we have on file for now
			wxString theNews;
			if ( profile->Global()->Read(GBL_CFG_NET_THE_NEWS, &theNews) ){ 
				newsWindow->SetPage(theNews);
			} else {
				wxLogFatalError(_T("%s does not exist but the exists function says it does"), GBL_CFG_NET_THE_NEWS);
			}
		} else {
			wxFileSystem filesystem;
			wxFSFile* news = filesystem.OpenFile(_("http://www.audiozone.ro/hl/"), wxFS_READ);
			if ( news == NULL ) {
				wxLogError(_("Error in retriving news"));
				return;
			}

			wxInputStream* theNews = news->GetStream();
			wxLogDebug(_T("news loaded from %s with type %s"), news->GetLocation().c_str(), news->GetMimeType().c_str());

			wxString newsData;
			wxStringOutputStream newsDataStream(&newsData);
			theNews->Read(newsDataStream);
			wxString formattedData(_T("<ul>"));
			wxStringTokenizer tok(newsData, _T("\t"), wxTOKEN_STRTOK);
			while(tok.HasMoreTokens()) {
				wxString title(tok.GetNextToken());
				wxCHECK_RET(tok.HasMoreTokens(), _T("news formatter has run out of tokens at wrong time"));
				wxString link(tok.GetNextToken());
				wxCHECK_RET(tok.HasMoreTokens(), _T("news formatter has run out of tokens at wrong time"));
				wxString imglink(tok.GetNextToken());
				
				formattedData += wxString::Format(_T("\n<li><a href='%s'>%s</a><!-- %s --></li>"), link.c_str(), title.c_str(), imglink.c_str());
			}
			formattedData += _T("\n</ul>");
			profile->Global()->Write(GBL_CFG_NET_THE_NEWS, formattedData);
			newsWindow->SetPage(formattedData);

			wxString currentTime(wxDateTime::Now().Format(NEWS_LAST_TIME_FORMAT));
			profile->Global()->Write(GBL_CFG_NET_NEWS_LAST_TIME, currentTime);
		}
	} else {
		newsWindow->SetPage(_("Auto news download disabled."));
	}
}

bool WelcomePage::getOrPromptUpdateNews() {
	bool updateNews;
	if (!ProMan::GetProfileManager()->Global()
		->Read(GBL_CFG_NET_DOWNLOAD_NEWS, &updateNews)) {
		wxDialog* updateNewsQuestion = 
			new wxDialog(NULL, wxID_ANY, 
			_("wxLauncher - Network Access Request"),
			wxDefaultPosition, wxDefaultSize, 
			wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP | wxDIALOG_NO_PARENT);
		updateNewsQuestion->SetBackgroundColour(
			wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
		wxFileName updateNewsQuestionIconLocation(
			_T(RESOURCES_PATH), _T("helpicon.png"));
		wxIcon updateNewsQuestionIcon(
			updateNewsQuestionIconLocation.GetFullPath(), wxBITMAP_TYPE_ANY);
		wxASSERT(updateNewsQuestionIcon.IsOk());

		updateNewsQuestion->SetIcon(updateNewsQuestionIcon);
		wxStaticText* updateNewsText1 = 
			new wxStaticText(updateNewsQuestion, wxID_ANY, 
				_("wxLauncher has the built-in capability of retrieving and displaying the Hard-Light.net highlights on the Welcome page of the launcher."),
				wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
		wxStaticText* updateNewsText2 = 
			new wxStaticText(updateNewsQuestion, wxID_ANY, 
				_("Would you like to allow wxLauncher to go online and retrieve the highlights automatically?"),
				wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
		wxStaticText* updateNewsText3 = 
			new wxStaticText(updateNewsQuestion, wxID_ANY,
				_("(this setting can can be changed anytime on the Install page, please click \"?\" for more info)"),
				wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);

		wxButton* allowNewsUpdate = 
			new wxButton(updateNewsQuestion, wxID_ANY, 
				_("Allow automatic Welcome page highlights update"));
		wxButton* denyNewsUpdate = 
			new wxButton(updateNewsQuestion, wxID_ANY,
				_("Do not update the highlights on the Welcome page"));
		updateNewsQuestion->SetAffirmativeId(allowNewsUpdate->GetId());
		updateNewsQuestion->SetEscapeId(denyNewsUpdate->GetId());
		wxButton* helpButton = new wxButton(updateNewsQuestion, wxID_ANY,
			_T("?"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);

		helpButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
			wxCommandEventHandler(WelcomePage::OnUpdateNewsHelp));
		
		wxBoxSizer* updateNewsSizer = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer* bodySizer= new wxBoxSizer(wxVERTICAL);
		wxBoxSizer* choiceSizer = new wxBoxSizer(wxHORIZONTAL);

		wxFileName questionMarkLocation(
			_T(RESOURCES_PATH), _T("questionmark.png"));
		wxBitmap questionMark(
			questionMarkLocation.GetFullPath(), wxBITMAP_TYPE_ANY);
		wxASSERT(questionMark.IsOk());
		wxStaticBitmap* questionImage = 
			new wxStaticBitmap(updateNewsQuestion, wxID_ANY, questionMark);
		
		choiceSizer->AddStretchSpacer(1);
		choiceSizer->Add(allowNewsUpdate);
		choiceSizer->AddSpacer(15);
		choiceSizer->Add(denyNewsUpdate);
		choiceSizer->AddSpacer(15);
		choiceSizer->Add(helpButton);
		choiceSizer->AddStretchSpacer(1);
		
		bodySizer->AddSpacer(10);
		bodySizer->Add(updateNewsText1, wxSizerFlags().Expand().Center());
		bodySizer->AddSpacer(5);
		bodySizer->Add(updateNewsText2, wxSizerFlags().Expand().Center());
		bodySizer->AddSpacer(5);
		bodySizer->Add(updateNewsText3, wxSizerFlags().Expand().Center());
		bodySizer->AddSpacer(10);
		bodySizer->Add(choiceSizer, wxSizerFlags().Expand().Center());
		bodySizer->AddSpacer(10);

		updateNewsSizer->Add(questionImage,0, wxALL | wxCENTER, 5);
		updateNewsSizer->Add(bodySizer, wxSizerFlags().Expand().Center());
		updateNewsSizer->AddSpacer(5);
		
		updateNewsQuestion->SetSizerAndFit(updateNewsSizer);
		updateNewsQuestion->Centre(wxBOTH | wxCENTRE_ON_SCREEN);

		if ( updateNewsQuestion->GetAffirmativeId() == updateNewsQuestion->ShowModal() ) {
			updateNews = true;
		} else {
			updateNews = false;
		}
		ProMan::GetProfileManager()->Global()->Write(GBL_CFG_NET_DOWNLOAD_NEWS, updateNews);
		
		updateNewsQuestion->Destroy();
	}
	return updateNews;
}

void WelcomePage::OnDownloadNewsCheck(wxCommandEvent& event) {
	wxCheckBox* checkbox = dynamic_cast<wxCheckBox*>(wxWindow::FindWindowById(event.GetId(), this));
	wxCHECK_RET( checkbox != NULL, _T("OnDownloadNewsCheck called by non checkbox"));

	ProMan::GetProfileManager()->Global()->Write(GBL_CFG_NET_DOWNLOAD_NEWS, checkbox->IsChecked());
}

void WelcomePage::OnUpdateNewsHelp(wxCommandEvent &WXUNUSED(event)) {
	HelpManager::OpenHelpById(ID_MORE_INFO_PRIVACY);
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
		wxString::Format(_("Are you sure you would like to delete profile %s"), name.c_str()));

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
