#include <wx/wx.h>
#include "ModsPage.h"
#include "wxIDs.h"
#include "ModList.h"
#include "Skin.h"

#include "wxLauncherSetup.h" // Last include for memory debugging

ModsPage::ModsPage(wxWindow* parent, SkinSystem *skin): wxPanel(parent, wxID_ANY) {
	wxStaticText* header = new wxStaticText(this, wxID_ANY,
		_("Installed MODs.  Click on Install/Update in the left panel to\n\
		  search, download, and install additional MODs and updates."));

	ModList* modGrid = new ModList(this, wxSize(600, 400), skin);
	modGrid->SetMinSize(wxSize(600, 400));

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(header);
	sizer->Add(modGrid);

	this->SetSizer(sizer);
	this->Layout();

}