#ifndef STATUSLOG_H
#define STATUSLOG_H

#include <wx/wx.h>

enum StatusLogIDS: int {
	ID_STATUS_TOTAL_PROGRESS = 1,
	ID_STATUS_TASK_PROGRESS,
	ID_INFO_PANEL,
};

class StatusLog;

class JobData {
public:
	/** Pulses the progress bar for indeterminate jobs. */
	void Pulse();
	/** Set the minimum and maximum values that will be passed with SetValue(). */
	void SetRange(int max, int min=0);
	/** Set the position in the range set by SetRange(). */
	void SetValue(int value);
	/** Set the statusbar message. Method copies the message that is passed in. */
	void SetMessage(wxString& msg);
	/** Mark job as done so that it is not included in the job list. */
	void Done();

private:
	friend StatusLog;
	JobData(StatusLog* manager, int max=100, int min=0);
	StatusLog* myManager;
	int max, min, value;
	bool active;	//!< !active jobData's will be reaped on StatusLog's next pass
	bool beingShown;//!< when true, this job data is the one that is being shown to the user
	bool pulseState;//!< when true job was last pulsed for its update.
	wxString* message;
};

WX_DECLARE_LIST( JobData, JobTable);

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
		void MouseEnter(wxMouseEvent &event);
		void MouseExit(wxMouseEvent &event);
		void MouseUp(wxMouseEvent &event);
		void MouseDown(wxMouseEvent &event);

		void OpenLogWindow();

	protected:
		void SetMessage(wxString &msg);
	private:
		friend JobData;
		wxString* currentMessage;
		wxFont *enabledFont, *disabledFont, *hoverFont;
		wxColour *enabledColour, *disabledColour;
		bool mouseHovering, mouseClicking;

		DECLARE_EVENT_TABLE();
	};

private:
	friend JobData;
	wxPanel *gaugePanel;
	StatusLogInfoPanel *infoPanel;
	wxGauge *totalProgress, *taskProgress;

	wxCriticalSection *taskProgressLock;	//!< lock held when making changes to the progress bar.
	wxCriticalSection *jobTableLock;		//!< lock held when making changes to the jobTable.
	wxCriticalSection *messageLock;			//!< lock held when changing the currentMessage.

	JobTable *jobTable;

	
};

#endif