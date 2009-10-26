#ifndef STATUSLOG_H
#define STATUSLOG_H

#include <wx/wx.h>

enum StatusLogIDS: int {
	ID_STATUS_TOTAL_PROGRESS = 1,
	ID_STATUS_TASK_PROGRESS,
	ID_INFO_PANEL,
};

class JobData {
};

WX_DECLARE_HASH_MAP( int, JobData*, wxIntegerHash, wxIntegerEqual, JobTable);

class StatusLog {
public:
	StatusLog();

	wxPanel* GetGaugePanel(wxWindow* parent);
	wxPanel* GetInfoPanel(wxWindow* parent);
	bool IsOk();

protected:
	class StatusLogInfoPanel: public wxPanel {
	public:
		StatusLogInfoPanel(wxWindow* parent);

		void OnPaint(wxPaintEvent &event);

	protected:
		void SetMessage(wxString &msg);
	private:
		wxString* currentMessage;
		wxFont *enabledFont, *disabledFont;
		wxColour *enabledColour, *disabledColour;

		DECLARE_EVENT_TABLE();
	};

private:
	wxPanel *gaugePanel;
	StatusLogInfoPanel *infoPanel;
	wxGauge *totalProgress, *taskProgress;

	JobTable *jobTable;

	
};

#endif