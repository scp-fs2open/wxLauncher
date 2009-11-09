#include <wx/wx.h>
#include "StatusBar.h"
#include "wxIDS.h"

#include "wxLauncherSetup.h" // Last include for memory debugging

StatusBar::StatusBar(wxWindow *parent)
		:wxStatusBar(parent) {
	this->parent = parent;

	this->icons[ID_SB_OK] = wxBitmap(_T("ok.png"), wxBITMAP_TYPE_ANY);
	this->icons[ID_SB_WARNING] = wxBitmap(_T("warning.png"), wxBITMAP_TYPE_ANY);
	this->icons[ID_SB_ERROR] = wxBitmap(_T("error.png"), wxBITMAP_TYPE_ANY);

	for( int i = 0; i < ID_SB_MAX_ID; i++) { // Check that all icons are okay.
		if ( !this->icons[i].IsOk() ) {
			this->icons[i] = wxNullBitmap;
			wxLogWarning(_T("Icon ID: %d is not okay"), i);
		}
	}

	wxStaticBitmap statusIcon(this, ID_STATUSBAR_STATUS_ICON, this->icons[ID_SB_OK]);
	wxGauge(this, ID_STATUSBAR_PROGRESS_BAR, 100);

	int widths[] = { 25, -1, 100, 200 };

	this->SetFieldsCount(4, widths);
}

StatusBar::~StatusBar() {
}