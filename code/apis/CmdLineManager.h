/*
 Copyright (C) 2009-2011 wxLauncher Team
 
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

#ifndef CMDLINEMANAGER_H
#define CMDLINEMANAGER_H

#include <wx/wx.h>

/** Command line has changed. */
DECLARE_EVENT_TYPE(EVT_CMD_LINE_CHANGED, wxID_ANY);
/** Custom flags have changed. */
DECLARE_EVENT_TYPE(EVT_CUSTOM_FLAGS_CHANGED, wxID_ANY);

WX_DECLARE_LIST(wxEvtHandler, CmdLineEventHandlers);

class CmdLineManager {
private:
	CmdLineManager();
	~CmdLineManager();

public:
	static void RegisterCmdLineChanged(wxEvtHandler *handler);
	static void UnRegisterCmdLineChanged(wxEvtHandler *handler);
	static void RegisterCustomFlagsChanged(wxEvtHandler *handler);
	static void UnRegisterCustomFlagsChanged(wxEvtHandler *handler);
	static void GenerateCmdLineChanged();
	static void GenerateCustomFlagsChanged();
private:
	static CmdLineEventHandlers CmdLineChangedHandlers;
	static CmdLineEventHandlers CustomFlagsChangedHandlers;
};
#endif
