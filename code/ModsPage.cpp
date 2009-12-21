#include <wx/wx.h>
#include "ModsPage.h"
#include "ids.h"
#include "ModList.h"
#include "Skin.h"
#include "ProfileManager.h"
#include "TCManager.h"

#include "wxLauncherSetup.h" // Last include for memory debugging

ModsPage::ModsPage(wxWindow* parent, SkinSystem *skin): wxPanel(parent, wxID_ANY) {
	this->skin = skin;
	this->SetMinSize(wxSize(TAB_AREA_WIDTH, TAB_AREA_HEIGHT));

	TCManager::RegisterTCChanged(this);
	wxCommandEvent nullEvent;
	this->OnTCChanged(nullEvent);
}

BEGIN_EVENT_TABLE(ModsPage, wxPanel)
EVT_COMMAND(wxID_NONE, EVT_TC_CHANGED, ModsPage::OnTCChanged)
END_EVENT_TABLE()

void ModsPage::OnTCChanged(wxCommandEvent &WXUNUSED(event)) {
	wxString tcPath;

	ProMan::GetProfileManager()->Get()->Read(PRO_CFG_TC_ROOT_FOLDER, &tcPath, wxEmptyString);

	wxSizer* currentSizer = this->GetSizer();
	if ( currentSizer != NULL ) {
		currentSizer->DeleteWindows();
	}
	if ( tcPath.IsEmpty()) {
		wxStaticText* noTC = new wxStaticText(this, wxID_ANY,
			_("There is no path currently specified for the location of a Total Conversion.  Select a path on the Basic Settings tab."));
		wxFont messageFont(16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
		noTC->SetFont(messageFont);

		noTC->SetForegroundColour(wxTheColourDatabase->Find(_T("RED")));

		noTC->Wrap(TAB_AREA_WIDTH);
		
		wxBoxSizer* noTCSizer = new wxBoxSizer(wxVERTICAL);
		noTCSizer->Add(noTC, wxSizerFlags().Expand());

		this->SetSizer(noTCSizer);
	} else if ( !wxFileName::DirExists(tcPath)  ) {
		wxStaticText* invalidTC = new wxStaticText(this, wxID_ANY,
			_("The currently specified Total Conversion is not a valid Total Conversion.  Select a valid Total Conversion on the Basic Settings tab."));
		wxFont messageFont(16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
		invalidTC->SetFont(messageFont);

		invalidTC->SetForegroundColour(wxTheColourDatabase->Find(_T("RED")));

		invalidTC->Wrap(TAB_AREA_WIDTH);
		
		wxBoxSizer* invalidTCSizer = new wxBoxSizer(wxVERTICAL);
		invalidTCSizer->Add(invalidTC, wxSizerFlags().Expand());

		this->SetSizer(invalidTCSizer);
	} else {
		wxStaticText* header = new wxStaticText(this, wxID_ANY,
			_("Installed MODs.  Click on Install/Update in the left panel to search, download, and install additional MODs and updates."));
		header->Wrap(TAB_AREA_WIDTH);

		wxSize modGridSize(TAB_AREA_WIDTH, 500);
		ModList* modGrid = new ModList(this, modGridSize, skin, tcPath);
		modGrid->SetMinSize(modGridSize);

		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
		sizer->Add(header);
		sizer->Add(modGrid, 1);

		this->SetSizer(sizer);
	}
	this->Layout();
}