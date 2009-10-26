#include "StatusLog.h"

#include <wx/wx.h>
#include <wx/listimpl.cpp>
WX_DEFINE_LIST(JobTable);

StatusLog::StatusLog() {
	this->jobTable = new JobTable();

	this->gaugePanel = NULL;
	this->infoPanel = NULL;
	this->totalProgress = NULL;
	this->taskProgress = NULL;

	this->taskProgressLock = new wxCriticalSection();
	this->jobTableLock = new wxCriticalSection();
	this->messageLock = new wxCriticalSection();
}

wxPanel* StatusLog::GetGaugePanel(wxWindow* parent) {
	this->gaugePanel = new wxPanel(parent);
	this->taskProgress = new wxGauge(this->gaugePanel, ID_STATUS_TASK_PROGRESS, 100);
	this->totalProgress = new wxGauge(this->gaugePanel, ID_STATUS_TOTAL_PROGRESS, 100);
	this->totalProgress->Hide();

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(this->taskProgress);
	sizer->Add(this->totalProgress);

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

///////////////////////////////////////////////////////////////////////////////
JobData::JobData(StatusLog *manager, int max, int min) {
	wxASSERT_MSG( manager != NULL, _("Passed StatusLog manager is NULL."));
	this->myManager = manager;

	wxASSERT_MSG( max > min, _("Passed max is less than min."));
	this->max = max;
	this->min = min;

	this->active = true;
	this->beingShown = false;

	this->value = min;
	this->message = NULL;
}

void JobData::Pulse() {
	wxASSERT_MSG( this->active, _("JobData is done but someone called."));
	wxCriticalSectionLocker lock(*(this->myManager->taskProgressLock));
	if ( this->beingShown ) {
		this->myManager->taskProgress->Pulse();
	}
	this->pulseState = true;
}

void JobData::SetMessage(wxString &msg) {
	wxASSERT_MSG( this->active, _("JobData is done but someone called."));
	wxCriticalSectionLocker lock(*(this->myManager->messageLock));
	if ( this->beingShown ) {
		this->myManager->infoPanel->SetMessage(msg);
	}
	if ( this->message != NULL ) {
		delete this->message;
	}
	if ( msg.empty() ) {
		this->message = NULL;
	} else {
		this->message = new wxString(msg);
	}
}

void JobData::SetRange(int max, int min) {
	wxASSERT_MSG( this->active, _("JobData is done but someone called."));
	wxASSERT_MSG( max > min, _("Max is less than or equal to min."));
	this->max = max;
	this->min = min;
}

void JobData::SetValue(int value) {
	wxASSERT_MSG( this->active, _("JobData is done but someone called."));
	wxASSERT_MSG( max >= value, _("Value is larger than max."));
	wxASSERT_MSG( min <= value, _("Value is smaller than min."));
	wxCriticalSectionLocker(*(this->myManager->taskProgressLock));
	if ( this->beingShown ) {
		this->myManager->taskProgress->SetValue(value);
	}
	this->pulseState = false;
	this->value = value;
}

void JobData::Done() {
	wxASSERT_MSG( this->active, _("Done has been called more than once."));
	this->active = false;
}