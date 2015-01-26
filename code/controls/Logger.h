/*
Copyright (C) 2009-2010 wxLauncher Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#ifndef LAUNCHERLOGGER_H
#define LAUNCHERLOGGER_H

#include <wx/wx.h>
#include <wx/wfstream.h>
#include "controls/StatusBar.h"

class Logger: public wxLog {
public:
	Logger();
	virtual ~Logger();

	void SetStatusBarTarget(StatusBar *bar);

	/* Compatiblity with 2.8.x */
#if wxVERSION_NUMBER > 20899
	virtual void DoLogRecord(
		wxLogLevel level,
		const wxString& msg,
		const wxLogRecordInfo& info);
#else
	virtual void DoLog(
		wxLogLevel level,
		const wxChar *msg,
		time_t time);
#endif
	
	virtual void Flush();
private:
	wxFFileOutputStream *out;
	StatusBar *statusBar;
	wxFFile *outFile;
};

#endif
