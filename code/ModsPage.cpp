#include <wx/wx.h>
#include "ModsPage.h"
#include "wxIDs.h"
#include "ModList.h"
#include "Skin.h"

#include "wxLauncherSetup.h" // Last include for memory debugging

ModsPage::ModsPage(wxWindow* parent, SkinSystem *skin): wxPanel(parent, wxID_ANY) {
	wxStaticText* header = new wxStaticText(this, wxID_ANY,
		_("Installed MODs.  Click on Install/Update in the left panel to search, download, and install additional MODs and updates."));
	header->Wrap(TAB_AREA_WIDTH);

	ModList* modGrid = new ModList(this, wxSize(TAB_AREA_WIDTH, 400), skin);
	modGrid->SetMinSize(wxSize(TAB_AREA_WIDTH, 500));

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(header);
	sizer->Add(modGrid, 1);

	this->SetMinSize(wxSize(TAB_AREA_WIDTH, TAB_AREA_HEIGHT));
	this->SetSizer(sizer);
	this->Layout();

}