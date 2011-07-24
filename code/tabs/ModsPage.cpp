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
#include <wx/settings.h>
#include "tabs/ModsPage.h"
#include "global/ids.h"
#include "controls/ModList.h"
#include "apis/SkinManager.h"
#include "apis/ProfileManager.h"
#include "apis/TCManager.h"
#include "generated/configure_launcher.h"


#include "global/MemoryDebugging.h" // Last include for memory debugging

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

	ProMan::GetProfileManager()->Get()
		->Read(PRO_CFG_TC_ROOT_FOLDER, &tcPath, wxEmptyString);

	wxSizer* currentSizer = this->GetSizer();
	if ( currentSizer != NULL ) {
		currentSizer->DeleteWindows();
	}
	if ( tcPath.IsEmpty()) {
		wxStaticText* noTC = new wxStaticText(this, wxID_ANY, _("To view a list of available MODs, you must first specify a Total Conversion root folder.\n You can do that on the Basic Settings page.\n\n(please check the help system for more info)"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
		wxFont messageFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
		noTC->SetFont(messageFont);
		noTC->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
		noTC->Wrap(TAB_AREA_WIDTH-50);

		wxFileName infoLocation(_T(RESOURCES_PATH), _T("info_big.png"));
		wxBitmap infoIcon(infoLocation.GetFullPath(), wxBITMAP_TYPE_ANY);
		wxASSERT(infoIcon.IsOk());
		wxStaticBitmap* infoImage = new wxStaticBitmap(this, wxID_ANY, infoIcon);

		wxBoxSizer* noTCSizer = new wxBoxSizer(wxVERTICAL);
		noTCSizer->AddStretchSpacer(1);
		noTCSizer->Add(infoImage,0, wxALL | wxCENTER);
		noTCSizer->AddSpacer(10);
		noTCSizer->Add(noTC, 0, wxEXPAND| wxALL | wxCENTER);
		noTCSizer->AddStretchSpacer(1);

		this->SetSizer(noTCSizer);
	} else if ( !wxFileName::DirExists(tcPath)  ) {
		wxStaticText* invalidTC = new wxStaticText(this, wxID_ANY,
			_("The currently specified Total Conversion root folder does not contain a valid Total Conversion.\nSelect a valid Total Conversion root folder on the Basic Settings page.\n\n(please check the help system for more info)"), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
		wxFont messageFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
		invalidTC->SetFont(messageFont);
		invalidTC->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
		invalidTC->Wrap(TAB_AREA_WIDTH-50);

		wxFileName warningLocation(_T(RESOURCES_PATH), _T("warning_big.png"));
		wxBitmap warningIcon(warningLocation.GetFullPath(), wxBITMAP_TYPE_ANY);
		wxASSERT(warningIcon.IsOk());
		wxStaticBitmap* warningImage = new wxStaticBitmap(this, wxID_ANY, warningIcon);

		wxBoxSizer* invalidTCSizer = new wxBoxSizer(wxVERTICAL);
		invalidTCSizer->AddStretchSpacer(1);
		invalidTCSizer->Add(warningImage,0, wxALL | wxCENTER);
		invalidTCSizer->AddSpacer(10);
		invalidTCSizer->Add(invalidTC, 0, wxEXPAND| wxALL | wxCENTER);
		invalidTCSizer->AddStretchSpacer(1);

		this->SetSizer(invalidTCSizer);
	} else {
#if !IS_APPLE
		wxStaticText* header = new wxStaticText(this, wxID_ANY,
			_("Installed MODs.  Click on Install/Update in the left to search, download, and install additional MODs and updates."), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
		header->Wrap(TAB_AREA_WIDTH);
#endif
		wxSize modGridSize(TAB_AREA_WIDTH, 500);
		ModList* modGrid = new ModList(this, modGridSize, skin, tcPath);
		modGrid->SetMinSize(modGridSize);

		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
#if !IS_APPLE
		sizer->Add(header);
#endif
		sizer->Add(modGrid, 1);

		this->SetSizer(sizer);
	}
	this->Layout();
}
