#include <wx/wx.h>
#include "StatusBar.h"
#include "wxIDS.h"
#include "logger.h"

#include "wxLauncherSetup.h" // Last include for memory debugging

enum FieldIDs {
	SB_FIELD_ICON = 0,
	SB_FIELD_MAINTEXT,
	SB_FIELD_PROGRESS_BAR,
	SB_FIELD_PROGRESS_TEXT,
	SB_FIELD_MAX,
};

BEGIN_EVENT_TABLE(StatusBar, wxStatusBar)
EVT_SIZE(StatusBar::OnSize)
END_EVENT_TABLE()

StatusBar::StatusBar(wxWindow *parent)
		:wxStatusBar(parent) {
	this->parent = parent;
	this->showingToolTip = false;

	this->icons[ID_SB_OK] = wxBitmap(_T("ok.png"), wxBITMAP_TYPE_ANY);
	this->icons[ID_SB_WARNING] = wxBitmap(_T("warning.png"), wxBITMAP_TYPE_ANY);
	this->icons[ID_SB_ERROR] = wxBitmap(_T("error.png"), wxBITMAP_TYPE_ANY);
	this->icons[ID_SB_INFO] = wxBitmap(_T("information.png"), wxBITMAP_TYPE_ANY);

	for( int i = 0; i < ID_SB_MAX_ID; i++) { // Check that all icons are okay.
		if ( !this->icons[i].IsOk() ) {
			this->icons[i] = wxNullBitmap;
			wxLogWarning(_T("Icon ID: %d is not okay"), i);
		}
	}

	// Just creating these now, will place them in the OnSize event handler
	new wxStaticBitmap(this, ID_STATUSBAR_STATUS_ICON, this->icons[ID_SB_OK]);
	new wxGauge(this, ID_STATUSBAR_PROGRESS_BAR, 100);

	int widths[] = { 25, -1, 100, 200 };

	wxASSERT_MSG( sizeof(widths)/sizeof(int) == SB_FIELD_MAX,
		wxString::Format(
			_T("Number of fields (%d) and number of widths (%d) do not match"),
			SB_FIELD_MAX, sizeof(widths)/sizeof(int)));

	this->SetFieldsCount(SB_FIELD_MAX, widths);

	this->SetStatusText(_T("Status bar created"), SB_FIELD_MAINTEXT);

	dynamic_cast<Logger*>(wxLog::GetActiveTarget())->SetStatusBarTarget(this);
}

StatusBar::~StatusBar() {
	dynamic_cast<Logger*>(wxLog::GetActiveTarget())->SetStatusBarTarget(NULL);
}

void StatusBar::OnSize(wxSizeEvent& WXUNUSED(event)) {
	wxWindow* icon = this->GetWindowChild(ID_STATUSBAR_STATUS_ICON);

	wxASSERT( icon != NULL );
	
	wxRect iconrect;
	this->GetFieldRect(SB_FIELD_ICON, iconrect);
	icon->SetSize(iconrect);

	wxWindow* bar = this->GetWindowChild(ID_STATUSBAR_PROGRESS_BAR);

	wxASSERT( bar != NULL );

	wxRect barrect;
	this->GetFieldRect(SB_FIELD_PROGRESS_BAR, barrect);
	bar->SetSize(barrect);

}

void StatusBar::SetMainStatusText(wxString msg, int icon) {
	this->SetStatusText(msg, SB_FIELD_MAINTEXT);
	if ( icon > ID_SB_NO_CHANGE && icon < ID_SB_MAX_ID ) {
		wxStaticBitmap* iconImage = dynamic_cast<wxStaticBitmap*>(this->GetWindowChild(ID_STATUSBAR_STATUS_ICON));

		iconImage->SetBitmap(this->icons[icon]);
		iconImage->Refresh();
	} else {
		if ( icon != ID_SB_NO_CHANGE ) {
			wxLogWarning(_T("SetMainStatusText was passed (%d) as an icon, which is out of range"), icon);
		}
	}
}

/** Causes the status bar to show the msg until EndToolTipStatusText() is
called.  When EndToolTipStatusText() is called the status text will be returned
to the original text. */
void StatusBar::StartToolTipStatusText(wxString msg) {
	if (!this->showingToolTip) {
		this->PushStatusText(msg, SB_FIELD_MAINTEXT);
		this->showingToolTip = true;
	}
}

/** Returns the statusbar text to the orginal message after a mouse over.
See StartToolTipStatusText() for more information. */
void StatusBar::EndToolTipStatusText() {
	if (this->showingToolTip) {
		this->PopStatusText(SB_FIELD_MAINTEXT);
		this->showingToolTip = false;
	}
}
