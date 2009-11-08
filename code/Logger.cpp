#include <wx/wx.h>
#include <wx/wfstream.h>
#include <wx/datetime.h>

#include <wchar.h>

#include "Logger.h"

#include "wxLauncherSetup.h"

////// Logger
const wxString levels[] = {
	_T("FATAL"),
	_T("ERROR"),
	_T("WARN "),
	_T("MSG  "),
	_T("MSG1 "),
	_T("INFO "),
	_T("DEBUG"),
};
/** Constructor. */
Logger::Logger() {

	this->out = new wxFFileOutputStream(_T("wxLauncher.log"), _T("wb"));
	wxASSERT_MSG(out->IsOk(), _T("Log output file is not valid!"));
}

/** Destructor. */
Logger::~Logger() {
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
}
