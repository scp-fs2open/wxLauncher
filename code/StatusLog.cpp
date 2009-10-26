
#include "StatusLog.h"

#include <wx/wx.h>

StatusLog::StatusLog() {
	this->gaugePanel = NULL;
	this->infoPanel = NULL;
	this->totalProgress = NULL;
	this->taskProgress = NULL;

	this->taskProgressLock = new wxCriticalSection();
	this->messageLock = new wxCriticalSection();
}

wxPanel* StatusLog::GetGaugePanel(wxWindow* parent) {
	this->gaugePanel = new wxPanel(parent);
	this->taskProgress = new wxGauge(this->gaugePanel, ID_STATUS_TASK_PROGRESS, 100);
	this->totalProgress = new wxGauge(this->gaugePanel, ID_STATUS_TOTAL_PROGRESS, 100);
	this->totalProgress->Hide();

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(this->taskProgress, 0, wxEXPAND);
	sizer->Add(this->totalProgress,  0, wxEXPAND);

	this->gaugePanel->SetSizer(sizer);
	
	return (this->gaugePanel);
}

wxPanel* StatusLog::GetInfoPanel(wxWindow* parent) {
	this->infoPanel = new StatusLogInfoPanel(parent);

	return (this->infoPanel);
}

bool StatusLog::IsOk() {
	return ( (this->gaugePanel != NULL)
		&& (this->infoPanel != NULL)
		&& (this->totalProgress != NULL)
		&& (this->taskProgress != NULL)
		);
}

///////////////////////////////////////////////////////////////////////////////

StatusLog::StatusLogInfoPanel::StatusLogInfoPanel(wxWindow* parent): wxPanel(parent, ID_INFO_PANEL) {
	this->enabledFont = new wxFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
	wxASSERT_MSG( this->enabledFont->IsOk(), _("Enabled font is not valid"));
	this->disabledFont = new wxFont(*(this->enabledFont));
	this->disabledFont->SetStyle(wxFONTSTYLE_ITALIC);
	wxASSERT_MSG( this->disabledFont->IsOk(), _("Disabled font is not valid"));

	this->hoverFont = new wxFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
	this->hoverFont->SetStyle(wxFONTSTYLE_SLANT);
	wxASSERT_MSG( this->hoverFont->IsOk(), _("Hover font is not valid"));

	unsigned long black = 0x000000;
	unsigned long grey = 0x555555;
	this->enabledColour = new wxColour(black);
	wxASSERT_MSG( this->enabledColour->IsOk(), _("Enabled colour is not valid"));
	this->disabledColour = new wxColour(grey);
	wxASSERT_MSG( this->disabledColour->IsOk(), _("Disabled colour is not valid"));

	this->currentMessage = new wxString(_("Ready."));

	this->mouseHovering = false;
	this->mouseClicking = false;
}

void StatusLog::StatusLogInfoPanel::SetMessage(wxString& msg) {
	wxString* temp = this->currentMessage;

	this->currentMessage = new wxString(msg);

	delete temp;
}

BEGIN_EVENT_TABLE(StatusLog::StatusLogInfoPanel, wxPanel)
EVT_PAINT( StatusLog::StatusLogInfoPanel::OnPaint)
EVT_ENTER_WINDOW( StatusLog::StatusLogInfoPanel::MouseEnter)
EVT_LEAVE_WINDOW( StatusLog::StatusLogInfoPanel::MouseExit)
EVT_LEFT_UP( StatusLog::StatusLogInfoPanel::MouseUp)
EVT_LEFT_DOWN( StatusLog::StatusLogInfoPanel::MouseDown)
END_EVENT_TABLE()

void StatusLog::StatusLogInfoPanel::OnPaint(wxPaintEvent &event) {
	wxPaintDC dc(this);

	if ( this->mouseHovering ) {
		if ( this->mouseClicking ) {
			dc.SetFont(*(this->enabledFont));
		} else {
			dc.SetFont(*(this->hoverFont));
		}
	} else {
		dc.SetFont(*(this->enabledFont));
	}
	dc.DrawText(*(this->currentMessage), 0, 0);
}

void StatusLog::StatusLogInfoPanel::MouseEnter(wxMouseEvent &event) {
	this->mouseHovering = true;
	this->RefreshRect(wxRect(this->GetSize()));;
}

void StatusLog::StatusLogInfoPanel::MouseExit(wxMouseEvent &event) {
	this->mouseHovering = false;
	this->RefreshRect(wxRect(this->GetSize()));;
}

void StatusLog::StatusLogInfoPanel::MouseUp(wxMouseEvent &event) {
	if ( this->mouseClicking ) {
		this->OpenLogWindow();
	}
	this->mouseClicking = false;
	this->RefreshRect(wxRect(this->GetSize()));
}

void StatusLog::StatusLogInfoPanel::MouseDown(wxMouseEvent &event) {
	this->mouseClicking = true;
	this->RefreshRect(wxRect(this->GetSize()));
}

void StatusLog::StatusLogInfoPanel::OpenLogWindow() {
}