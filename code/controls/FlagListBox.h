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

#ifndef FLAGLISTBOX_H
#define FLAGLISTBOX_H

#include <wx/wx.h>
#include <wx/vlbox.h>

#include "apis/EventHandlers.h"
#include "apis/FlagListManager.h"

class FlagListCheckBox: public wxCheckBox {
public:
	FlagListCheckBox(
		wxWindow* parent,
		const wxString& label,
		const wxString& flagString);
	void OnClicked(wxCommandEvent &event);
private:
	FlagListCheckBox();
	wxString flagString;
};

class FlagListCheckBoxItem {
public:
	FlagListCheckBoxItem(const wxString& fsoCategory);
	FlagListCheckBoxItem(FlagListCheckBox& checkBox, wxSizer& checkBoxSizer,
		const wxString& shortDescription, const wxString& flagString,
		bool isRecommendedFlag);
	~FlagListCheckBoxItem();
	const wxString& GetFsoCategory() const { return this->fsoCategory; }
	FlagListCheckBox* GetCheckBox() { return this->checkBox; }
	wxSizer* GetCheckBoxSizer() { return this->checkBoxSizer; }
	const wxString& GetShortDescription() const { return this->shortDescription; }
	const wxString& GetFlagString() const { return this->flagString; }
	bool IsRecommendedFlag() const { return this->isRecommendedFlag; }
private:
	FlagListCheckBoxItem();
	wxString fsoCategory;
	FlagListCheckBox* checkBox;
	wxSizer* checkBoxSizer;
	wxString shortDescription;
	wxString flagString;
	bool isRecommendedFlag;
};

WX_DECLARE_LIST(FlagListCheckBoxItem, FlagListCheckBoxItems);

/** Flag list box is ready for use. */
DECLARE_EVENT_TYPE(EVT_FLAG_LIST_BOX_READY, wxID_ANY);

class FlagListBox: public wxVListBox {
public:
	FlagListBox(wxWindow* parent);
	~FlagListBox();
	
	void RegisterFlagListBoxReady(wxEvtHandler *handler);
	void UnRegisterFlagListBoxReady(wxEvtHandler *handler);

	virtual void OnDrawItem(wxDC &dc, const wxRect &rect, size_t n) const;
	virtual void OnDrawBackground(wxDC &dc, const wxRect &rect, size_t n) const;
	virtual wxCoord OnMeasureItem(size_t n) const;

	void OnDoubleClickFlag(wxCommandEvent &event);
	
	/** Loads enabled flags from the proxy and checks the corresponding boxes. */
	void LoadEnabledFlags();
	
	/** Tries to find the flagSet specified and then set or unset all flags
	contained in the flag set, returns true on success, returns false
	iff it cannot find the flagset.  That is, will return true if none of
	the flags in the flag set are real flags. */
	bool SetFlagSet(const wxString& setToFind);
	
	void GetFlagSets(wxArrayString& arr) const;
	
	void AcceptFlagData(FlagFileData* flagData);
	
	bool IsReady() const { return this->isReady; }
	
	bool FlagsLoaded() const { return this->flagsLoaded; }

private:
	EventHandlers flagListBoxReadyHandlers;
	void GenerateFlagListBoxReady();
	bool isReadyEventGenerated;
	bool isReady;
	bool flagsLoaded;
	
	/** Tries to find flagString in the list of flags and set it to state.
	 returns true on successful set, returns false if cannot find flag.
	 Will update the proxy if updateProxy is true. */
	bool SetFlag(const wxString& flagString, bool state, bool updateProxy = false);
	
	FlagFileData* flagData;
	FlagListCheckBoxItems checkBoxes;
	void GenerateCheckBoxes(const FlagListBoxData& data);
	bool areCheckBoxesGenerated;

	FlagListCheckBoxItem* FindFlagAt(size_t n) const;

	DECLARE_EVENT_TABLE();

};

#endif
