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
#include "datastructures/FSOExecutable.h"
#include "generated/configure_launcher.h"


#include "global/MemoryDebugging.h" // Last include for memory debugging

ModsPage::ModsPage(wxWindow* parent, SkinSystem *skin): wxPanel(parent, wxID_ANY) {
	this->skin = skin;

	wxLogDebug(_T("ModsPage is at %p."), this);

	TCManager::RegisterTCChanged(this);
	wxCommandEvent nullEvent;
	this->OnTCChanged(nullEvent);
}

BEGIN_EVENT_TABLE(ModsPage, wxPanel)
EVT_COMMAND(wxID_NONE, EVT_TC_CHANGED, ModsPage::OnTCChanged)
END_EVENT_TABLE()

void ModsPage::OnTCChanged(wxCommandEvent &WXUNUSED(event)) {
	wxString tcPath;

	ProMan::GetProfileManager()->ProfileRead(PRO_CFG_TC_ROOT_FOLDER, &tcPath, wxEmptyString);

	wxSizer* currentSizer = this->GetSizer();
	if ( currentSizer != NULL ) {
		currentSizer->Clear(true);
	}
	if ( tcPath.IsEmpty()) {
		wxStaticText* noTC = new wxStaticText(this, wxID_ANY, _("To view a list of available mods, you must first select the root folder\nof a FreeSpace 2 installation or a total conversion\non the Basic Settings page."),
											  wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
		wxFont messageFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
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
		noTCSizer->Add(noTC, 0, wxALL | wxCENTER);
		noTCSizer->AddStretchSpacer(1);

#if IS_LINUX
		this->SetSizerAndFit(noTCSizer);
#else
		this->SetSizer(noTCSizer);
#endif
	} else if ( !wxFileName::DirExists(tcPath)  ) {
		wxStaticText* nonexistentTC = new wxStaticText(this, wxID_ANY,
			_("The selected root folder does not exist.\nSelect a valid root folder for a FreeSpace 2 installation\nor a total conversion on the Basic Settings page."),
			wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
		wxFont messageFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
		nonexistentTC->SetFont(messageFont);
		nonexistentTC->SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
		nonexistentTC->Wrap(TAB_AREA_WIDTH-50);

		wxFileName warningLocation(_T(RESOURCES_PATH), _T("warning_big.png"));
		wxBitmap warningIcon(warningLocation.GetFullPath(), wxBITMAP_TYPE_ANY);
		wxASSERT(warningIcon.IsOk());
		wxStaticBitmap* warningImage = new wxStaticBitmap(this, wxID_ANY, warningIcon);

		wxBoxSizer* nonexistentTCSizer = new wxBoxSizer(wxVERTICAL);
		nonexistentTCSizer->AddStretchSpacer(1);
		nonexistentTCSizer->Add(warningImage,0, wxALL | wxCENTER);
		nonexistentTCSizer->AddSpacer(10);
		nonexistentTCSizer->Add(nonexistentTC, 0, wxALL | wxCENTER);
		nonexistentTCSizer->AddStretchSpacer(1);

#if IS_LINUX
		this->SetSizerAndFit(nonexistentTCSizer);
#else
		this->SetSizer(nonexistentTCSizer);
#endif
	} else if ( !FSOExecutable::HasFSOExecutables(wxFileName(tcPath, wxEmptyString)) ) {
		wxStaticText* invalidTC = new wxStaticText(this, wxID_ANY,
			_("The selected root folder\ndoes not contain any FreeSpace 2 Open executables.\n\nOn the Basic Settings page, either select a different root folder,\nor add FS2 Open executables to the selected root folder\nand press the Refresh button."),
			wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
		wxFont messageFont(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL);
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
		invalidTCSizer->Add(invalidTC, 0, wxALL | wxCENTER);
		invalidTCSizer->AddStretchSpacer(1);

#if IS_LINUX
		this->SetSizerAndFit(invalidTCSizer);
#else
		this->SetSizer(invalidTCSizer);
#endif
	} else {
#if 0
		wxStaticText* header = new wxStaticText(this, wxID_ANY,
			_("Installed mods.  Click on Install/Update in the left to search, download, and install additional mods and updates."), wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
		header->Wrap(TAB_AREA_WIDTH);
#endif
		wxSize modGridSize(TAB_AREA_WIDTH - 20, TAB_AREA_HEIGHT); // FIXME for left and right borders of 5 pixels each -- but why does it have to be 20?
		ModList* modGrid = new ModList(this, modGridSize, skin, tcPath);
		modGrid->SetMinSize(modGridSize);

		wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
#if 0
		sizer->Add(header);
#endif
		sizer->Add(modGrid, wxSizerFlags().Center().Border(wxALL,5));

		this->SetMaxSize(wxSize(TAB_AREA_WIDTH, TAB_AREA_HEIGHT));
		this->SetSizer(sizer);
	}
	this->Layout();
}
