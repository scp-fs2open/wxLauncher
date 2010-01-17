#include <wx/wx.h>
#include "ids.h"
#include "ProfileManager.h"
#include "InstallPage.h"

#include "wxLauncherSetup.h" // Last include for memory debugging

InstallPage::InstallPage(wxWindow* parent): wxPanel(parent, wxID_ANY) {
	wxCheckBox* updateNewsCheck = new wxCheckBox(this, ID_NET_DOWNLOAD_NEWS, _("Auto download news for Welcome Page"));
	updateNewsCheck->SetToolTip(_("Launcher will update the news on next run"));
	bool updateNews;
	if (!ProMan::GetProfileManager()->Global()->Read(GBL_CFG_NET_DOWNLOAD_NEWS, &updateNews)) {
		wxDialog* updateNewsQuestion = new wxDialog(NULL, wxID_ANY, _("Network Access Question"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP | wxDIALOG_NO_PARENT);
		updateNewsQuestion->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
		wxStaticText* updateNewsText = new wxStaticText(updateNewsQuestion, wxID_ANY, _("This launcher can update the news on the front page automaticly.  This update will be done no more than once a day.\n(This setting can be changed at anytime on the Install page)"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
		//updateNewsText->Wrap(600);
		wxButton* allowNewsUpdate = new wxButton(updateNewsQuestion, wxID_ANY, _("Allow launcher to update the front page news"));
		wxButton* denyNewsUpdate = new wxButton(updateNewsQuestion, wxID_ANY, _("Do not update the news on the front page"));
		updateNewsQuestion->SetAffirmativeId(allowNewsUpdate->GetId());
		updateNewsQuestion->SetEscapeId(denyNewsUpdate->GetId());
		wxBoxSizer* updateNewsSizer = new wxBoxSizer(wxVERTICAL);
		wxBitmap questionMark(_T("questionmark.png"), wxBITMAP_TYPE_ANY);
		wxASSERT(questionMark.IsOk());
		wxStaticBitmap* questionImage = new wxStaticBitmap(updateNewsQuestion, wxID_ANY, questionMark);
		updateNewsSizer->Add(questionImage);
		updateNewsSizer->Add(updateNewsText);
		wxBoxSizer* choiceSizer = new wxBoxSizer(wxHORIZONTAL);
		choiceSizer->AddStretchSpacer(1);
		choiceSizer->Add(allowNewsUpdate);
		choiceSizer->AddSpacer(5);
		choiceSizer->Add(denyNewsUpdate);
		choiceSizer->AddStretchSpacer(1);
		updateNewsSizer->Add(choiceSizer, wxSizerFlags().Expand().Center());
		//updateNewsSizer->SetMinSize(wxSize(600,100));
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
	updateNewsCheck->SetValue(updateNews);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(updateNewsCheck);
	this->SetSizer(sizer);
	this->Layout();
}

BEGIN_EVENT_TABLE(InstallPage, wxPanel)
EVT_CHECKBOX(ID_NET_DOWNLOAD_NEWS, InstallPage::OnDownloadNewsCheck)
END_EVENT_TABLE()

void InstallPage::OnDownloadNewsCheck(wxCommandEvent& event) {
	wxCheckBox* checkbox = dynamic_cast<wxCheckBox*>(wxWindow::FindWindowById(event.GetId(), this));
	wxCHECK_RET( checkbox != NULL, _T("OnDownloadNewsCheck called by non checkbox"));

	ProMan::GetProfileManager()->Global()->Write(GBL_CFG_NET_DOWNLOAD_NEWS, checkbox->IsChecked());
}