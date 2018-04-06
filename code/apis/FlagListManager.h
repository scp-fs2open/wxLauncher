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
#include "apis/EventHandlers.h"

#include <lib/json.hpp>

// for convenience
using json = nlohmann::json;

/** Flag file processing status has changed.
 The event's int value indicates the FlagFileProcessingStatus. */
LAUNCHER_DECLARE_EVENT_TYPE(EVT_FLAG_FILE_PROCESSING_STATUS_CHANGED);

WX_DECLARE_OBJARRAY(wxFileName, FlagFileArray);

class FlagListManager: public wxEvtHandler {
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
		FLAG_FILE_PROCESSING_RESET,
		FLAG_FILE_PROCESSING_ERROR
	};

	struct BuildCaps {
		bool openAL = false;
		bool noD3D = false;
		bool newSound = false;
		bool sdl = false;
	};

	struct Joystick {
		wxString name;
		wxString guid;
		bool is_haptic = false;
	};

	struct Resolution {
		int width = -1;
		int height = -1;

		bool operator==(const Resolution& rhs) const;
		bool operator!=(const Resolution& rhs) const;
		bool operator<(const Resolution& rhs) const;
		bool operator>(const Resolution& rhs) const;
		bool operator<=(const Resolution& rhs) const;
		bool operator>=(const Resolution& rhs) const;
	};

	void OnBinaryChanged(wxCommandEvent &event);
	
	static void RegisterFlagFileProcessingStatusChanged(wxEvtHandler *handler);
	static void UnRegisterFlagFileProcessingStatusChanged(wxEvtHandler *handler);
	
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
	
	/** Gets the build capabilities of the currently selected FSO executable.
	 Should only be called when processing succeeds. */
	BuildCaps GetBuildCaps() const;

	const std::vector<Joystick>& GetJoysticks() const;

	const std::vector<Resolution>& GetResolutions() const;

private:
	enum CapabilityFlags {
		BUILD_CAPS_OPENAL = 1 << 0,
		BUILD_CAPS_NO_D3D = 1 << 1,
		BUILD_CAPS_NEW_SND = 1 << 2,
		BUILD_CAPS_SDL = 1 << 3,
	};

	FlagListManager();
	void DeleteExistingData();
	
	static FlagListManager* flagListManager;
	
	static EventHandlers ffProcessingStatusChangedHandlers;
	
	static void GenerateFlagFileProcessingStatusChanged(const FlagFileProcessingStatus& status);
	
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

	void convertAndAddJsonFlag(const json& data);
	ProcessingStatus ParseJsonData(const std::string& data);
	
	void SetProcessingStatus(const ProcessingStatus& processingStatus);
	inline const ProcessingStatus& GetProcessingStatus() const { return this->processingStatus; }
	FlagFileProcessingStatus GetFlagFileProcessingStatus() const;
	
	FlagFileData* data;
	ProxyFlagData* proxyData;

	BuildCaps buildCaps;

	std::vector<Joystick> joysticks;

	std::vector<Resolution> resolutions;

	class FlagProcess: public wxProcess {
	public:
		FlagProcess(FlagFileArray flagFileLocations);
		virtual void OnTerminate(int pid, int status);
	private:
		FlagFileArray flagFileLocations;
	};
	
	DECLARE_EVENT_TABLE()
};
#endif
