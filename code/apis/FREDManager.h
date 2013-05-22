/*
 Copyright (C) 2009-2013 wxLauncher Team
 
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

/** Manages the status of whether FRED launching is enabled. */

#ifndef FRED_MANAGER_H
#define FRED_MANAGER_H

#include <wx/event.h>

/** FRED enabled status has changed. */
DECLARE_EVENT_TYPE(EVT_FRED_ENABLED_CHANGED, wxID_ANY);

WX_DECLARE_LIST(wxEvtHandler, FREDEnabledEventHandlers);

class FREDManager {
public:
	static void RegisterFREDEnabledChanged(wxEvtHandler *handler);
	static void UnRegisterFREDEnabledChanged(wxEvtHandler *handler);
	static void GenerateFREDEnabledChanged();
private:
	FREDManager(); // prevents instantiation
	
	static FREDEnabledEventHandlers FREDEnabledChangedHandlers;
};

#endif
