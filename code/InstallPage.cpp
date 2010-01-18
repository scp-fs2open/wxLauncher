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
		wxDialog* updateNewsQuestion = new wxDialog(NULL, wxID_ANY, _("wxLauncher - Network Access Request"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxSTAY_ON_TOP | wxDIALOG_NO_PARENT);
		updateNewsQuestion->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
		wxStaticText* updateNewsText1 = new wxStaticText(updateNewsQuestion, wxID_ANY, _("wxLauncher has the built-in capability of retrieving and displaying the Hard-Light.net highlights on the Welcome page of the launcher."), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
		wxStaticText* updateNewsText2 = new wxStaticText(updateNewsQuestion, wxID_ANY, _("Would you like to allow wxLauncher to go online and retrieve the highlights automatically? "), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
		wxStaticText* updateNewsText3 = new wxStaticText(updateNewsQuestion, wxID_ANY, _("(this setting can can be changed anytime on the Install page, please click \"?\" for more info)"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);

		wxButton* allowNewsUpdate = new wxButton(updateNewsQuestion, wxID_ANY, _("Allow automatic Welcome page highlights update"));
		wxButton* denyNewsUpdate = new wxButton(updateNewsQuestion, wxID_ANY, _("Do not update the highlights on the Welcome page"));
		updateNewsQuestion->SetAffirmativeId(allowNewsUpdate->GetId());
		updateNewsQuestion->SetEscapeId(denyNewsUpdate->GetId());
		
		wxBoxSizer* updateNewsSizer = new wxBoxSizer(wxHORIZONTAL);
		wxBoxSizer* bodySizer= new wxBoxSizer(wxVERTICAL);
		wxBoxSizer* choiceSizer = new wxBoxSizer(wxHORIZONTAL);

		wxBitmap questionMark(_T("questionmark.png"), wxBITMAP_TYPE_ANY);
		wxASSERT(questionMark.IsOk());
		wxStaticBitmap* questionImage = new wxStaticBitmap(updateNewsQuestion, wxID_ANY, questionMark);
		
		choiceSizer->AddStretchSpacer(1);
		choiceSizer->Add(allowNewsUpdate);
		choiceSizer->AddSpacer(15);
		choiceSizer->Add(denyNewsUpdate);
		choiceSizer->AddStretchSpacer(1);
		
		bodySizer->AddSpacer(10);
		bodySizer->Add(updateNewsText1, 0, wxEXPAND | wxCENTER);
		bodySizer->AddSpacer(5);
		bodySizer->Add(updateNewsText2, 0, wxEXPAND | wxCENTER);
		bodySizer->AddSpacer(5);
		bodySizer->Add(updateNewsText3, 0, wxEXPAND | wxCENTER);
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