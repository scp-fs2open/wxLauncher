#include <wx/wx.h>
#include <wx/wfstream.h>
#include <wx/datetime.h>

#include <wchar.h>

#include "Logger.h"
#include "StatusBar.h"

#include "wxLauncherSetup.h"

////// Logger
const wxString levels[] = {
	_T("FATAL"),
	_T("ERROR"),
	_T("WARN "),
	_T("MSG  "),
	_T("STSBR"),
	_T("INFO "),
	_T("DEBUG"),
};
/** Constructor. */
Logger::Logger() {

	this->out = new wxFFileOutputStream(_T("wxLauncher.log"), _T("wb"));
	wxASSERT_MSG(out->IsOk(), _T("Log output file is not valid!"));

	this->statusBar = NULL;
}

/** Destructor. */
Logger::~Logger() {
	char exitmsg[] = "\nLog closed.\n";
	this->out->Write(exitmsg, sizeof(exitmsg));
	this->out->Close();
	delete this->out;
}

/** Overridden as per wxWidgets docs to implement a wxLog. */
void Logger::DoLog(wxLogLevel level, const wxChar *msg, time_t time) {
	wxString timestr = wxDateTime(time).Format(_T("%y%j%H%M%S"), wxDateTime::GMT0);
	wxString str = wxString::Format(_T("%s:%s:"), timestr, levels[level]);
	wxString buf(msg);
	out->Write(str.mb_str(wxConvUTF8), str.size());
	out->Write(buf.mb_str(wxConvUTF8), buf.size());
	out->Write("\n", 1);

	if ( this->statusBar != NULL ) {
		if ( level == 1 ) { // error
			this->statusBar->SetMainStatusText(buf, ID_SB_ERROR);
		} else if ( level == 2 ) { // warning
			this->statusBar->SetMainStatusText(buf, ID_SB_WARNING);
		} else if ( level == 3 || level == 4 ) { // message, statubar
			this->statusBar->SetMainStatusText(buf, ID_SB_OK);
		} else if ( level == 5 ) { // info
			this->statusBar->SetMainStatusText(buf, ID_SB_INFO);
		}
	}		
}

/** Stores the pointer the status bar that I am to send status messages to.
If a status bar is already set, function will do nothing to the old statusbar.
Logger does not take over managment of the statusbar passed in. */
void Logger::SetStatusBarTarget(StatusBar *bar) {
	this->statusBar = bar;
}

