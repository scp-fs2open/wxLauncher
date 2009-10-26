#include "StatusLog.h"

StatusLog::StatusLog() {
	this->jobTable = new JobTable();

	this->gaugePanel = NULL;
	this->infoPanel = NULL;
	this->totalProgress = NULL;
	this->taskProgress = NULL;
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
#if 0
/** Called when the launcher is not doing anything. Vanishes the progress bar and
dims the log button. */
void BottomButtons::Done() {
	this->log->Disable();
}

/** Check the internal numbers and make the changes to the log button and the progress
bar. */
void BottomButtons::UpdateBars() {
	wxMessageBox(wxString::Format(_("%d"), joblist->size()));
	if ( joblist->size() == 0 ) {
		this->log->SetFont(*(this->disabledFont));
		this->log->SetForegroundColour(*wxLIGHT_GREY); // MODMOD: hard coded values
		this->bar->Hide();
	} else {
		this->log->SetFont(*(this->enabledFont));
		this->log->SetForegroundColour(*wxBLACK); // MODMOD: hard coded values
		this->bar->Show();
	}
}
#endif
StatusLog::StatusLogInfoPanel::StatusLogInfoPanel(wxWindow* parent): wxPanel(parent, ID_INFO_PANEL) {
	this->enabledFont = new wxFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
	wxASSERT_MSG( this->enabledFont->IsOk(), _("Enabled font is not valid"));
	this->disabledFont = new wxFont(*(this->enabledFont));
	this->disabledFont->SetStyle(wxFONTSTYLE_ITALIC);
	wxASSERT_MSG( this->disabledFont->IsOk(), _("Disabled font is not valid"));

	unsigned long black = 0x000000;
	unsigned long grey = 0x555555;
	this->enabledColour = new wxColour(black);
	wxASSERT_MSG( this->enabledColour->IsOk(), _("Enabled colour is not valid"));
	this->disabledColour = new wxColour(grey);
	wxASSERT_MSG( this->disabledColour->IsOk(), _("Disabled colour is not valid"));

	this->currentMessage = new wxString(_("Ready."));
}

BEGIN_EVENT_TABLE(StatusLog::StatusLogInfoPanel, wxPanel)
EVT_PAINT( StatusLog::StatusLogInfoPanel::OnPaint)
END_EVENT_TABLE()

void StatusLog::StatusLogInfoPanel::OnPaint(wxPaintEvent &event) {
	wxPaintDC dc(this);

	dc.DrawText(*(this->currentMessage), 0, 0);
}