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

/** Flag file processing status has changed.
 The event's int value indicates the FlagFileProcessingStatus. */
DECLARE_EVENT_TYPE(EVT_FLAG_FILE_PROCESSING_STATUS_CHANGED, wxID_ANY);

WX_DECLARE_LIST(wxEvtHandler, FlagFileProcessingEventHandlers);

class FlagListManager {
public:
	static bool Initialize();
	static void DeInitialize();
	static bool IsInitialized();
	static FlagListManager* GetFlagListManager();

	~FlagListManager();

	enum FlagFileProcessingStatus {
		FLAG_FILE_PROCESSING_OK = 0,
		FLAG_FILE_PROCESSING_WAITING,
		FLAG_FILE_PROCESSING_ERROR
	};

	void RegisterFlagFileProcessingStatusChanged(wxEvtHandler *handler);
	void UnRegisterFlagFileProcessingStatusChanged(wxEvtHandler *handler);
	void GenerateFlagFileProcessingStatusChanged(const FlagFileProcessingStatus& status);
private:
	FlagListManager();
	
	static FlagListManager* flagListManager;
	
	FlagFileProcessingEventHandlers flagFileProcessingStatusChangedHandlers;
};
#endif
