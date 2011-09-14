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
	CloneProfileDialog(wxWindow* parent);
	const wxString GetSourceProfileName();
	const wxString& GetNewProfileName();
	bool UseProfileCloning() const { return useProfileCloning; }
	void OnUpdateText(wxCommandEvent& event);
	void OnPressEnterKey(wxCommandEvent& event);
	void OnClickCloneCheckbox(wxCommandEvent& event);
private:
	bool useProfileCloning;
	wxString newProfileName;
	wxButton *createButton;
	wxChoice *cloneFrom;
	
	bool NewNameIsValid();
	
	DECLARE_EVENT_TABLE();
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

BEGIN_EVENT_TABLE(WelcomePage, wxPanel)
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
EVT_CHECKBOX(ID_SAVE_DEFAULT_CHECK, WelcomePage::AutoSaveProfilesChecked)
EVT_CHOICE(ID_PROFILE_COMBO, WelcomePage::ProfileChanged)

EVT_CHECKBOX(ID_NET_DOWNLOAD_NEWS, WelcomePage::OnDownloadNewsCheck)

EVT_IDLE(WelcomePage::UpdateNews)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(CloneProfileDialog, wxDialog)
EVT_TEXT(ID_CLONE_PROFILE_NEWNAME, CloneProfileDialog::OnUpdateText)
EVT_TEXT_ENTER(ID_CLONE_PROFILE_NEWNAME, CloneProfileDialog::OnPressEnterKey)
EVT_CHECKBOX(ID_CLONE_PROFILE_CHECKBOX, CloneProfileDialog::OnClickCloneCheckbox)
END_EVENT_TABLE()

WelcomePage::WelcomePage(wxWindow* parent, SkinSystem* skin): wxPanel(parent, wxID_ANY) {
	// member varirable init
	this->lastLinkInfo = NULL;
	ProMan* proman = ProMan::GetProfileManager();

#if 0
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
#endif
	// header image
	HeaderBitmap* header = new HeaderBitmap(this, skin->GetBanner().GetWidth(), skin);
	
	// Info
	wxStaticBox* generalBox = new wxStaticBox(this, wxID_ANY, _(""));
	wxHtmlWindow* general = new wxHtmlWindow(this, ID_SUMMARY_HTML_PANEL, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_NEVER );
	general->SetPage(skin->GetWelcomePageText());
	general->Connect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(WelcomePage::OnMouseOut));
	
	wxStaticBoxSizer* generalSizer = new wxStaticBoxSizer(generalBox, wxVERTICAL);
	generalSizer->SetMinSize(wxSize(-1, 175));
	generalSizer->Add(general, wxSizerFlags().Expand().Proportion(1));

	// Profiles
	wxStaticBox* profileBox = new wxStaticBox(this, wxID_ANY, _("Profile"));
	wxChoice* profileCombo = new wxChoice(this, ID_PROFILE_COMBO,
		wxDefaultPosition,
		wxDefaultSize,
		0,	// number of choices
		0,	// choices
		wxCB_SORT);
	profileCombo->Append(proman->GetAllProfileNames());
	proman->AddEventHandler(this);

	wxString lastselected;
	proman->GlobalRead(GBL_CFG_MAIN_LASTPROFILE, &lastselected, ProMan::DEFAULT_PROFILE_NAME);
	profileCombo->SetStringSelection(lastselected);

	wxButton* newButton = new wxButton(this, ID_NEW_PROFILE, _("New"));
	wxButton* deleteButton = new wxButton(this, ID_DELETE_PROFILE, _("Delete"));
	wxButton* saveButton = new wxButton(this, ID_SAVE_PROFILE, _("Save"));

	wxCheckBox* autoSaveProfilesCheck = new wxCheckBox(this, ID_SAVE_DEFAULT_CHECK, _("Automatically save profiles"));
	bool autosave;
	proman->GlobalRead(GBL_CFG_MAIN_AUTOSAVEPROFILES, &autosave, true, true);
	autoSaveProfilesCheck->SetValue(autosave);

	wxCommandEvent autoSaveEvent(wxEVT_COMMAND_CHECKBOX_CLICKED, ID_SAVE_DEFAULT_CHECK);
	autoSaveEvent.SetInt(autosave);
	this->AutoSaveProfilesChecked(autoSaveEvent);

	wxBoxSizer* profileButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
	profileButtonsSizer->Add(newButton);
	profileButtonsSizer->Add(deleteButton);
	profileButtonsSizer->Add(saveButton);
	profileButtonsSizer->AddStretchSpacer(1);
	profileButtonsSizer->Add(autoSaveProfilesCheck, 0, wxALIGN_CENTER_VERTICAL);

	wxStaticBoxSizer* profileVerticalSizer = new wxStaticBoxSizer(profileBox, wxVERTICAL);
	profileVerticalSizer->Add(profileCombo, 0, wxALL | wxEXPAND, 4);
	profileVerticalSizer->Add(profileButtonsSizer, 0, wxALL | wxEXPAND, 4);

	// Latest headlines
	wxStaticBox* headlinesBox = new wxStaticBox(this, wxID_ANY, _("Latest highlights from the front"));
	wxHtmlWindow* headlinesView = new wxHtmlWindow(this, ID_HEADLINES_HTML_PANEL);
	headlinesView->SetPage(_T(""));
	headlinesView->Connect(wxEVT_LEAVE_WINDOW, wxMouseEventHandler(WelcomePage::OnMouseOut));
	wxCheckBox* updateNewsCheck = new wxCheckBox(this, ID_NET_DOWNLOAD_NEWS, _("Automatically retrieve highlights at startup"));
	updateNewsCheck->SetToolTip(_("Check this to have the launcher retrieve the Hard Light Productions highlights the next time it runs"));
	updateNewsCheck->SetValue(this->getOrPromptUpdateNews());
	this->needToUpdateNews = true;

	wxStaticBoxSizer* headlines = new wxStaticBoxSizer(headlinesBox, wxVERTICAL);
	headlines->Add(headlinesView, 
		wxSizerFlags().Expand().Center().Proportion(1));
	headlines->Add(updateNewsCheck, wxSizerFlags().Right().Border(wxTOP,5));

	// Final layout
	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
#if 0
	sizer->Add(languageSizer);
#endif
	sizer->Add(header, wxSizerFlags().Proportion(0).Expand().Center().Border(wxTOP, 5));
	sizer->Add(generalSizer, wxSizerFlags().Proportion(0).Expand().Border(wxLEFT|wxRIGHT|wxBOTTOM, 5));
	sizer->Add(profileVerticalSizer, wxSizerFlags().Proportion(0).Expand().Border(wxLEFT|wxRIGHT|wxBOTTOM, 5));
	sizer->Add(headlines, wxSizerFlags().Expand().Proportion(1).Border(wxLEFT|wxRIGHT|wxBOTTOM, 5));

#if !IS_APPLE // needed for sizer to expand properly on Windows and Linux for some reason
	sizer->SetMinSize(wxSize(TAB_AREA_WIDTH-10, TAB_AREA_HEIGHT-5));
#endif

	this->SetSizerAndFit(sizer);
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
	wxChoice* profileCombo = dynamic_cast<wxChoice*>(wxWindow::FindWindowById(ID_PROFILE_COMBO, this));
	ProMan *proman = ProMan::GetProfileManager();

	switch(event.GetId()) {
		case ID_NEW_PROFILE:
			cloneNewProfile(profileCombo, proman);
			break;

		case ID_DELETE_PROFILE:
			deleteProfile(profileCombo, proman);
			break;

		case ID_SAVE_PROFILE:
			proman->SaveCurrentProfile();
			break;
		default:
			wxCHECK_RET( false, _("Reached impossible location.\nHandler has been attached to a button that it cannot handle."));
	}
}

void WelcomePage::AutoSaveProfilesChecked(wxCommandEvent& event) {
	ProMan* proman = ProMan::GetProfileManager();

	if ( event.IsChecked() ) {
		proman->SetAutoSave(true);
		proman->GlobalWrite(GBL_CFG_MAIN_AUTOSAVEPROFILES, true);
		proman->SaveCurrentProfile();
		wxLogStatus(_("Now autosaving profiles."));
	} else {
		proman->SetAutoSave(false);
		proman->GlobalWrite(GBL_CFG_MAIN_AUTOSAVEPROFILES, false);
		wxLogStatus(_("No longer autosaving profiles."));
	}
}
void WelcomePage::ProfileChanged(wxCommandEvent& WXUNUSED(event)) {
	wxChoice* profileCombo = dynamic_cast<wxChoice*>(wxWindow::FindWindowById(ID_PROFILE_COMBO, this));
	wxString newProfile = profileCombo->GetStringSelection();
	ProMan* proman = ProMan::GetProfileManager();

	if (newProfile == proman->GetCurrentName()) {
		// user selected the current profile, so no need to switch, just return
		return;
	}

	if ( proman->DoesProfileExist(newProfile) ) {
		if ( proman->NeedToPromptToSave() ) {
			int response = wxMessageBox(
				ProMan::GetSaveDialogMessageText(ProMan::ON_PROFILE_SWITCH, proman->GetCurrentName()),
				ProMan::GetSaveDialogCaptionText(ProMan::ON_PROFILE_SWITCH, proman->GetCurrentName()),
				wxYES_NO|wxCANCEL, this);
			if ( response == wxYES ) {
				wxLogDebug(_T("saving profile %s before switching to profile %s"),
					proman->GetCurrentName().c_str(), newProfile.c_str());
				proman->SaveCurrentProfile();
			} else if ( response == wxCANCEL ) {
				wxLogInfo(_T("Cancelled switch to profile %s."), newProfile.c_str());
				profileCombo->SetSelection(profileCombo->FindString(proman->GetCurrentName()));
				return;
			} else {
				wxLogWarning(_T("switching from profile %s to profile %s without saving changes"),
					proman->GetCurrentName().c_str(), newProfile.c_str());
			}
		}
		if ( proman->SwitchTo(newProfile) ) {
			wxLogMessage(_T("Profile %s is now the active profile."), proman->GetCurrentName().c_str());
		} else {
			wxLogError(_T("Unable to switch to %s, staying on %s."), newProfile.c_str(), proman->GetCurrentName().c_str());
		}
	} else {
		wxLogError(_T("Profile does not exist. Use Clone to create profile first"));
	}
}

void WelcomePage::cloneNewProfile(wxChoice* profileCombo, ProMan* proman) {
	if ( proman->NeedToPromptToSave() ) {
		int response = wxMessageBox(
			ProMan::GetSaveDialogMessageText(ProMan::ON_PROFILE_CREATE, proman->GetCurrentName()),
			ProMan::GetSaveDialogCaptionText(ProMan::ON_PROFILE_CREATE, proman->GetCurrentName()),
			wxYES_NO|wxCANCEL, this);
		if ( response == wxYES ) {
			wxLogDebug(_T("Saving current profile %s before opening create profile dialog."),
				proman->GetCurrentName().c_str());
			proman->SaveCurrentProfile();
		} else if ( response == wxCANCEL ) {
			wxLogInfo(_T("Cancelled opening create profile dialog."));
			return;
		} else {
			wxLogWarning(_T("Reverting unsaved changes to current profile %s before opening create profile dialog"),
				proman->GetCurrentName().c_str());
			proman->RevertCurrentProfile();
		}
	}
	
	CloneProfileDialog cloneDialog(this);

	if ( cloneDialog.ShowModal() == cloneDialog.GetAffirmativeId() ) {
		wxLogDebug(_T("User clicked %s"), cloneDialog.UseProfileCloning() ? _T("clone") : _T("create")); 
		if ( proman->CreateProfile(
				cloneDialog.GetNewProfileName(),
				cloneDialog.GetSourceProfileName()) ) {
			if (cloneDialog.UseProfileCloning()) {
				wxLogStatus(_("Cloned profile '%s' from '%s'"),
					cloneDialog.GetNewProfileName().c_str(),
					cloneDialog.GetSourceProfileName().c_str());
			} else {
				wxLogStatus(_("Created profile '%s'"),
					cloneDialog.GetNewProfileName().c_str());
			}
				proman->SwitchTo(cloneDialog.GetNewProfileName());
		} else { // profile creation was unsuccessful
			if (cloneDialog.UseProfileCloning()) {
				wxLogError(_("Unable to clone profile '%s' from '%s'. See log for details."),
					cloneDialog.GetNewProfileName().c_str(),
					cloneDialog.GetSourceProfileName().c_str());
			} else {
				wxLogError(_("Unable to create profile '%s'. See log for details."),
					cloneDialog.GetNewProfileName().c_str());	
			}
		}
	} else {
		wxLogStatus(_("Profile creation aborted"));
	}
}


void WelcomePage::deleteProfile(wxChoice* profileCombo, ProMan* proman) {
	wxString nametodelete = profileCombo->GetStringSelection();
	
	if ( proman->DoesProfileExist(nametodelete) ) {
		if (nametodelete == ProMan::DEFAULT_PROFILE_NAME) {
			wxLogWarning(_T("The default profile cannot be deleted."));
			return;
		}
		
		DeleteProfileDialog deleteDialog(this, nametodelete);
		
		if ( deleteDialog.ShowModal() == deleteDialog.GetAffirmativeId()) {
			if ( proman->DeleteProfile(nametodelete) ) {
				wxLogStatus(_("Deleted profile named '%s'."), nametodelete.c_str());
			} else {
				wxLogWarning(_("Unable to delete profile '%s', see log for more details."), nametodelete.c_str());
			}
		} else {
			wxLogStatus(_("Deletion of profile '%s' cancelled"), nametodelete.c_str());
		}
	} else {
		wxLogWarning(_T("Unable to delete non-existent profile '%s'"), nametodelete.c_str());
	}
}

void WelcomePage::ProfileCountChanged(wxCommandEvent &WXUNUSED(event)) {
	wxChoice* profileCombo = dynamic_cast<wxChoice*>(wxWindow::FindWindowById(ID_PROFILE_COMBO, this));
	wxCHECK_RET(profileCombo != NULL, _T("can't find the profile combobox"));
	ProMan *proman = ProMan::GetProfileManager();

	profileCombo->Clear();
	profileCombo->Append(proman->GetAllProfileNames());
	profileCombo->SetStringSelection(proman->GetCurrentName());
}

void WelcomePage::UpdateNews(wxIdleEvent& WXUNUSED(event)) {
	if ( !this->needToUpdateNews ) {
		return;
	}
	this->needToUpdateNews = false;
	wxHtmlWindow* newsWindow = dynamic_cast<wxHtmlWindow*>(wxWindow::FindWindowById(ID_HEADLINES_HTML_PANEL, this));
	wxCHECK_RET(newsWindow != NULL, _T("Update highlights called, but can't find the highlights window"));

	ProMan* proman = ProMan::GetProfileManager();

	bool allowedToUpdateNews;
	if ( !proman->GlobalRead(GBL_CFG_NET_DOWNLOAD_NEWS, &allowedToUpdateNews)) {
		return;
	}
	if (allowedToUpdateNews) {
		wxString lastTimeString;
		proman->GlobalRead(GBL_CFG_NET_NEWS_LAST_TIME, &lastTimeString);
		wxDateTime lasttime;
		if ( (NULL != lasttime.ParseFormat(lastTimeString, NEWS_LAST_TIME_FORMAT) )
			&& (wxDateTime::Now() - lasttime < TIME_BETWEEN_NEWS_UPDATES) 
			&& (proman->GlobalExists(GBL_CFG_NET_THE_NEWS)) ) {
			// post the news that we have on file for now
			wxString theNews;
			if ( proman->GlobalRead(GBL_CFG_NET_THE_NEWS, &theNews) ){ 
				newsWindow->SetPage(theNews);
			} else {
				wxLogFatalError(_T("%s does not exist but the exists function says it does"), GBL_CFG_NET_THE_NEWS);
			}
		} else {
			wxFileSystem filesystem;
			wxFSFile* news = filesystem.OpenFile(_("http://www.audiozone.ro/hl/"), wxFS_READ);
			if ( news == NULL ) {
				wxLogError(_("Error in retrieving highlights"));
				return;
			}

			wxInputStream* theNews = news->GetStream();
			wxLogDebug(_T("highlights loaded from %s with type %s"), news->GetLocation().c_str(), news->GetMimeType().c_str());

			wxString newsData;
			wxStringOutputStream newsDataStream(&newsData);
			theNews->Read(newsDataStream);
			wxString formattedData(_T("<ul>"));
			wxStringTokenizer tok(newsData, _T("\t"), wxTOKEN_STRTOK);
			while(tok.HasMoreTokens()) {
				wxString title(tok.GetNextToken());
				wxCHECK_RET(tok.HasMoreTokens(), _T("highlights formatter has run out of tokens at wrong time"));
				wxString link(tok.GetNextToken());
				wxCHECK_RET(tok.HasMoreTokens(), _T("highlights formatter has run out of tokens at wrong time"));
				wxString imglink(tok.GetNextToken());
				
				formattedData += wxString::Format(_T("\n<li><a href='%s'>%s</a><!-- %s --></li>"), link.c_str(), title.c_str(), imglink.c_str());
			}
			formattedData += _T("\n</ul>");
			proman->GlobalWrite(GBL_CFG_NET_THE_NEWS, formattedData);
			newsWindow->SetPage(formattedData);

			wxString currentTime(wxDateTime::Now().Format(NEWS_LAST_TIME_FORMAT));
			proman->GlobalWrite(GBL_CFG_NET_NEWS_LAST_TIME, currentTime);
		}
	} else {
		newsWindow->SetPage(_("Automatic highlights retrieval disabled."));
	}
}

bool WelcomePage::getOrPromptUpdateNews() {
	bool updateNews;
	if (!ProMan::GetProfileManager()->GlobalRead(GBL_CFG_NET_DOWNLOAD_NEWS, &updateNews)) {
		wxDialog* updateNewsQuestion = 
			new wxDialog(NULL, wxID_ANY, 
			_("wxLauncher - network access request"),
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
				_("Should wxLauncher automatically retrieve the highlights from Hard Light Productions?"),
				wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
		
		wxStaticText* updateNewsText2 = 
			new wxStaticText(updateNewsQuestion, wxID_ANY,
				_("You can change this setting on the Welcome page."),
				wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);

		wxButton* allowNewsUpdate = 
			new wxButton(updateNewsQuestion, wxID_ANY, 
				_("Yes"));
		wxButton* denyNewsUpdate = 
			new wxButton(updateNewsQuestion, wxID_ANY,
				_("No"));
		updateNewsQuestion->SetAffirmativeId(allowNewsUpdate->GetId());
		updateNewsQuestion->SetEscapeId(denyNewsUpdate->GetId());
#if 0
		wxButton* helpButton = new wxButton(updateNewsQuestion, wxID_ANY,
			_T("?"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);

		helpButton->Connect(wxEVT_COMMAND_BUTTON_CLICKED,
			wxCommandEventHandler(WelcomePage::OnUpdateNewsHelp));
#endif

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

		choiceSizer->Add(allowNewsUpdate, wxSizerFlags().Border(wxRIGHT, 5));
		choiceSizer->Add(denyNewsUpdate);
#if 0
		choiceSizer->Add(helpButton, wxSizerFlags().Border(wxLEFT, 15));
#endif

		bodySizer->Add(updateNewsText1, wxSizerFlags().Center());
		bodySizer->Add(updateNewsText2, wxSizerFlags().Center().Border(wxBOTTOM, 10));
		bodySizer->Add(choiceSizer, wxSizerFlags().Center());

		updateNewsSizer->Add(questionImage, wxSizerFlags().Center().Border(wxTOP|wxLEFT|wxBOTTOM, 5));
		updateNewsSizer->Add(bodySizer, wxSizerFlags().Center().Border(wxALL, 5));
		
		updateNewsQuestion->SetSizerAndFit(updateNewsSizer);
		updateNewsQuestion->Centre(wxBOTH | wxCENTRE_ON_SCREEN);

		if ( updateNewsQuestion->GetAffirmativeId() == updateNewsQuestion->ShowModal() ) {
			updateNews = true;
		} else {
			updateNews = false;
		}
		ProMan::GetProfileManager()->GlobalWrite(GBL_CFG_NET_DOWNLOAD_NEWS, updateNews);
		
		updateNewsQuestion->Destroy();
	}
	return updateNews;
}

void WelcomePage::OnDownloadNewsCheck(wxCommandEvent& event) {
	wxCheckBox* checkbox = dynamic_cast<wxCheckBox*>(wxWindow::FindWindowById(event.GetId(), this));
	wxCHECK_RET( checkbox != NULL, _T("OnDownloadNewsCheck called by non checkbox"));

	ProMan::GetProfileManager()->GlobalWrite(GBL_CFG_NET_DOWNLOAD_NEWS, checkbox->IsChecked());
}

void WelcomePage::OnUpdateNewsHelp(wxCommandEvent &WXUNUSED(event)) {
	HelpManager::OpenHelpById(ID_MORE_INFO_PRIVACY);
}


///////////////////////////////////////////////////////////////////////////////
///// DIALOGS ///
CloneProfileDialog::CloneProfileDialog(wxWindow* parent):
wxDialog(parent, ID_CLONE_PROFILE_DIALOG, _("Create new profile"), wxDefaultPosition, wxDefaultSize) {
	wxStaticText *newNameText = new wxStaticText(this, wxID_ANY, _("New profile name:"));
	wxTextCtrl *newName = new wxTextCtrl(this, ID_CLONE_PROFILE_NEWNAME, wxEmptyString,
										 wxDefaultPosition, wxSize(200,-1), wxTE_PROCESS_ENTER);
	
	wxSizer* nameSizer = new wxFlexGridSizer(3);
	nameSizer->AddStretchSpacer(1);
	nameSizer->Add(newNameText, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);
	nameSizer->Add(newName, wxSizerFlags().Expand());

	wxCheckBox* cloneFromCheckbox = new wxCheckBox(this, ID_CLONE_PROFILE_CHECKBOX, wxEmptyString);
	wxStaticText* cloneFromText = new wxStaticText(this, wxID_ANY, _("Clone settings from:"));
	cloneFrom = new wxChoice(this, wxID_ANY);

	nameSizer->Add(cloneFromCheckbox, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);
	nameSizer->Add(cloneFromText, 0, wxALIGN_CENTER_VERTICAL|wxRIGHT, 5);
	nameSizer->Add(cloneFrom, wxSizerFlags().Expand());
	
	this->createButton = new wxButton(this, wxID_OK, _("Create"));
	wxCommandEvent initDialogEvent;
	CloneProfileDialog::OnUpdateText(initDialogEvent);
	wxButton *closeButton = new wxButton(this, wxID_CANCEL, _("Cancel"));

	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	buttonSizer->Add(createButton, wxSizerFlags().Border(wxRIGHT, 5));
	buttonSizer->Add(closeButton, wxSizerFlags());

	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(nameSizer, wxSizerFlags().Expand().Border(wxALL, 5));
	sizer->Add(buttonSizer, wxSizerFlags().Right().Border(wxALL, 5));

	newName->SetValidator(wxTextValidator(wxFILTER_NONE, &(this->newProfileName)));

	cloneFrom->Append(ProMan::GetProfileManager()->GetAllProfileNames());
	cloneFrom->SetStringSelection(ProMan::GetProfileManager()->GetCurrentName());

	wxCommandEvent initEvent(wxEVT_COMMAND_CHECKBOX_CLICKED, ID_CLONE_PROFILE_CHECKBOX);
	initEvent.SetInt(0); // will initialize checkbox to unchecked
	this->OnClickCloneCheckbox(initEvent);
	
	this->SetSizerAndFit(sizer);
	this->Layout();
	this->Center();
	wxLogDebug(_T("Clone Profile Dialog Created"));
}

const wxString& CloneProfileDialog::GetNewProfileName() {
	return this->newProfileName.Trim(true).Trim(false); // strip trailing/leading whitespace
}

const wxString CloneProfileDialog::GetSourceProfileName() {
	// can't return wxString& since cloneFrom returns a temporary var
	return this->UseProfileCloning() ? cloneFrom->GetStringSelection() : wxString(wxEmptyString);
}

void CloneProfileDialog::OnUpdateText(wxCommandEvent& event) {
	this->createButton->Enable(this->NewNameIsValid());
}

void CloneProfileDialog::OnPressEnterKey(wxCommandEvent& event) {
	if (this->NewNameIsValid()) {
		wxCommandEvent simulatedClickEvent(wxEVT_COMMAND_BUTTON_CLICKED, wxID_OK);
		this->createButton->Command(simulatedClickEvent);
	}
}

void CloneProfileDialog::OnClickCloneCheckbox(wxCommandEvent& event) {
	if (event.IsChecked()) {
		this->useProfileCloning = true;
		this->createButton->SetLabel(_("Clone"));
		this->cloneFrom->Enable();
		this->Refresh();
		this->Update();
	} else {
		this->useProfileCloning = false;
		this->createButton->SetLabel(_("Create"));		
		this->cloneFrom->Disable();
		this->Refresh();
		this->Update();
	}
}

// a valid name is non-empty and does not consist purely of whitespace
bool CloneProfileDialog::NewNameIsValid() {
	wxTextCtrl* newName = dynamic_cast<wxTextCtrl*>(wxWindow::FindWindowById(ID_CLONE_PROFILE_NEWNAME, this));
	wxCHECK_MSG(newName != NULL, false, _T("can't find the clone profile new name text ctrl"));
	return !(newName->GetValue().Trim().IsEmpty());
}

DeleteProfileDialog::DeleteProfileDialog(wxWindow* parent, wxString name):
wxDialog(parent, ID_DELETE_PROFILE_DIALOG, _("Delete profile?"), wxDefaultPosition, wxDefaultSize) {
	wxStaticText* text = new wxStaticText(this, wxID_ANY,
		wxString::Format(_("Are you sure you want to delete profile '%s'?"), name.c_str()));

	wxButton *deleteButton = new wxButton(this, wxID_ANY, _("Delete"));
	wxButton *cancelButton = new wxButton(this, wxID_ANY, _("Cancel"));

	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	buttonSizer->Add(deleteButton, wxSizerFlags().Border(wxRIGHT, 5));
	buttonSizer->Add(cancelButton, wxSizerFlags());

	wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(text, wxSizerFlags().Expand().Border(wxALL, 5));
	sizer->Add(buttonSizer, wxSizerFlags().Right().Border(wxALL, 5));

	this->SetSizerAndFit(sizer);
	this->Layout();
	this->Center();

	this->SetAffirmativeId(deleteButton->GetId());
	this->SetEscapeId(cancelButton->GetId());

	wxLogDebug(_T("Delete Profile Dialog created."));
}
