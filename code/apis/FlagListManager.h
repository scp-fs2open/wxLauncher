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

#ifndef FLAGLISTMANAGER_H
#define FLAGLISTMANAGER_H

#include <wx/wx.h>

/** Flag list box's draw status has changed.
 The event's int value indicates the flag list box's FlagListBoxStatus. */
DECLARE_EVENT_TYPE(EVT_FLAG_LIST_BOX_DRAW_STATUS_CHANGED, wxID_ANY);

WX_DECLARE_LIST(wxEvtHandler, FlagListEventHandlers);

class FlagListManager {
public:
	static bool Initialize();
	static void DeInitialize();
	static bool IsInitialized();
	static FlagListManager* GetFlagListManager();

	~FlagListManager();

	enum FlagListBoxStatus {
		FLAGLISTBOX_OK = 0,
		FLAGLISTBOX_WAITING,
		FLAGLISTBOX_ERROR
	};

	void RegisterFlagListBoxDrawStatusChanged(wxEvtHandler *handler);
	void UnRegisterFlagListBoxDrawStatusChanged(wxEvtHandler *handler);
	void GenerateFlagListBoxDrawStatusChanged(const FlagListBoxStatus& status);
private:
	FlagListManager();
	
	static FlagListManager* flagListManager;
	
	FlagListEventHandlers flagListBoxDrawStatusChangedHandlers;
};
#endif
