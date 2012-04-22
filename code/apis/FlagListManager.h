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
#include <wx/filename.h>
#include <wx/process.h>

#include "datastructures/FlagFileData.h"

/** Flag file processing status has changed.
 The event's int value indicates the FlagFileProcessingStatus, and
 its extraLong value indicates the item count for the flag list. */
DECLARE_EVENT_TYPE(EVT_FLAG_FILE_PROCESSING_STATUS_CHANGED, wxID_ANY);

WX_DECLARE_LIST(wxEvtHandler, FlagFileProcessingEventHandlers);

WX_DECLARE_OBJARRAY(wxFileName, MyFlagFileArray);

class FlagListManager {
public:
	static bool Initialize();
	static void DeInitialize();
	static bool IsInitialized();
	static FlagListManager* GetFlagListManager();

	~FlagListManager();

	/** FlagFileProcessingStatus is a high-level description of the status of flag file processing. */
	enum FlagFileProcessingStatus {
		FLAG_FILE_PROCESSING_OK = 0,
		FLAG_FILE_PROCESSING_WAITING,
		FLAG_FILE_PROCESSING_ERROR
	};

	void RegisterFlagFileProcessingStatusChanged(wxEvtHandler *handler);
	void UnRegisterFlagFileProcessingStatusChanged(wxEvtHandler *handler);
	
	// TODO move this function to private once the refactoring is complete
	void GenerateFlagFileProcessingStatusChanged(const FlagFileProcessingStatus& status);
	
	void BeginFlagFileProcessing();
	
	/** Returns true when the flag file processing has succeeded, false otherwise. */
	inline bool IsProcessingOK() const { return (this->processingStatus == PROCESSING_OK); }
	
	/** Returns the message to display when processing has not (yet) succeded.
	 This function should not called when processing has succeeded. */
	wxString GetStatusMessage() const;
	
	/** Returns the extracted data from the flag file.
	 Should only be called when processing succeeds and only once per flag file processed. */
	FlagFileData* GetFlagFileData();
	
	/** Returns the extracted data in a form suitable for use by the profile proxy.
	 Should only be called when processing succeeds and only once per flag file processed. */
	ProxyFlagData* GetProxyFlagData();

private:
	FlagListManager();
	
	static FlagListManager* flagListManager;
	
	FlagFileProcessingEventHandlers flagFileProcessingStatusChangedHandlers;
	
	/** ProcessingStatus is a low-level description of the status of flag file processing. */
	enum ProcessingStatus {
		PROCESSING_OK = 0,
		INITIAL_STATUS,
		MISSING_TC,
		NONEXISTENT_TC,
		INVALID_TC,
		MISSING_EXE,
		INVALID_BINARY,
		WAITING_FOR_FLAG_FILE,
		FLAG_FILE_NOT_GENERATED,
		FLAG_FILE_NOT_VALID,
		FLAG_FILE_NOT_SUPPORTED,
		CANNOT_CREATE_FLAGFILE_FOLDER,
		CANNOT_CHANGE_WORKING_FOLDER,
		MAX_PROCESSINGSTATUS
	};
	ProcessingStatus processingStatus; //!< has processing succeeded
	ProcessingStatus ParseFlagFile(const wxFileName& flagfile);
	
	void SetProcessingStatus(const ProcessingStatus& processingStatus);
	inline const ProcessingStatus& GetProcessingStatus() const { return this->processingStatus; }
	FlagFileProcessingStatus GetFlagFileProcessingStatus() const;
	
	FlagFileData* data;
	ProxyFlagData* proxyData;
	
	class FlagProcess: public wxProcess {
	public:
		FlagProcess(MyFlagFileArray flagFileLocations);
		virtual void OnTerminate(int pid, int status);
	private:
		MyFlagFileArray flagFileLocations;
	};
};
#endif
