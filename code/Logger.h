#ifndef LAUNCHERLOGGER_H
#define LAUNCHERLOGGER_H

#include <wx/wx.h>
#include <wx/wfstream.h>
#include "StatusBar.h"

class Logger: public wxLog {
public:
	Logger();
	virtual ~Logger();

	void SetStatusBarTarget(StatusBar *bar);

	virtual void DoLog(wxLogLevel level, const wxChar *msg, time_t time);
private:
	wxFFileOutputStream *out;
	StatusBar *statusBar;
};

#endif