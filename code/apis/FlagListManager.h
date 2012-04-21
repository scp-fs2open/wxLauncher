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

/** Flag file processing status has changed.
 The event's int value indicates the FlagFileProcessingStatus, and
 its extraLong value indicates the item count for the flag list. */
DECLARE_EVENT_TYPE(EVT_FLAG_FILE_PROCESSING_STATUS_CHANGED, wxID_ANY);

WX_DECLARE_LIST(wxEvtHandler, FlagFileProcessingEventHandlers);

// FIXME temporarily using "My" prefix to avoid name clashes with items in FlagList
// TODO remove "My" prefix once refactoring is complete

class MyFlagListCheckBox: public wxCheckBox {
public:
	MyFlagListCheckBox(wxWindow* parent,
		const wxString& label,
		const wxString& flagString,
		int flagIndex);
	void OnClicked(wxCommandEvent &event);
private:
	wxString flagString;
	int flagIndex; // index is needed so that proxy can keep flags ordered in flag list order
};

class MyFlag {
public:
	MyFlag();
	wxString flagString;
	wxString shortDescription;
	wxString fsoCatagory;
	wxString webURL;
	bool isRecomendedFlag;
	wxUint32 easyEnable;
	wxUint32 easyDisable;
	MyFlagListCheckBox* checkbox;
	wxSizer* checkboxSizer;
	
	int GetFlagIndex() const { return this->flagIndex; }
private:
	int flagIndex; // private because the proxy depends on it being correct, so nothing should mess it up
	static int flagIndexCounter;
};

WX_DECLARE_LIST(MyFlag, MyFlagList);

/** Contains all of the flags in a category. */
class MyFlagCategory {
public:
	wxString categoryName;
	MyFlagList flags;
};

WX_DECLARE_LIST(MyFlagCategory, MyFlagCategoryList);

class MyFlagSet {
public:
	MyFlagSet(wxString name);
	wxString name;
	wxArrayString flagsToEnable;
	wxArrayString flagsToDisable;
};

WX_DECLARE_LIST(MyFlagSet, MyFlagSetsList);

/** Flag data needed by the profile proxy. */
class ProxyFlagDataItem {
public:
	ProxyFlagDataItem(const wxString& flagString, int flagIndex);
	const wxString& GetFlagString() const { return flagString; }
	int getFlagIndex() const { return flagIndex; }
private:
	wxString flagString;
	int flagIndex;
};

WX_DECLARE_LIST(ProxyFlagDataItem, ProxyFlagData);

/** The data extracted from the flag file. */
class FlagFileData {
public:
	FlagFileData();
	~FlagFileData();
	
	/** Adds the name of an "easy setup" flag set. */
	void AddEasyFlag(const wxString& easyFlag);
	
	void AddFlag(MyFlag* flag);
	
	/** Generates the "easy setup" flag sets.
	 This function requires that at least one "easy setup" name has been added.
	 Until support for the new mod.ini is added, this function should be called exactly once. */
	void GenerateFlagSets();
	
	/** Generates flag checkboxes for display. verticalOffset is the distance in pixels 
	 between the top of the checkbox and the top of the flag's entry in the FlagListBox entry. */
	void GenerateCheckBoxes(wxWindow* parent, const int verticalOffset);
	
	/** Creates a version of the data suitable for use by the profile proxy. */
	ProxyFlagData* GenerateProxyFlagData() const;
	
	/** Returns the total number of flags and flag category headers. */
	size_t GetItemCount() const;
	
	MyFlagCategoryList::iterator begin() { return this->allSupportedFlagsByCategory.begin(); }
	MyFlagCategoryList::const_iterator begin() const { return this->allSupportedFlagsByCategory.begin(); }

	MyFlagCategoryList::iterator end() { return this->allSupportedFlagsByCategory.end(); }
	MyFlagCategoryList::const_iterator end() const { return this->allSupportedFlagsByCategory.end(); }
private:
	wxArrayString easyFlags;
	MyFlagSetsList flagSets;
	MyFlagCategoryList allSupportedFlagsByCategory;
	bool isProxyDataGenerated;
};

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
