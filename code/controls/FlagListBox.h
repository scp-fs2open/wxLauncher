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

#include "apis/FlagListManager.h"
#include "apis/SkinManager.h"

// temporary My prefix to avoid name clashes with FlagFileData
// TODO remove My prefix after changeover is complete
class MyFlagListCheckBox: public wxCheckBox {
public:
	MyFlagListCheckBox(
		wxWindow* parent,
		const wxString& label,
		const wxString& flagString,
		int flagIndex);
	void OnClicked(wxCommandEvent &event);
private:
	MyFlagListCheckBox();
	wxString flagString;
	int flagIndex;
};

class FlagListCheckBoxItem {
public:
	FlagListCheckBoxItem(const wxString& fsoCategory);
	FlagListCheckBoxItem(MyFlagListCheckBox& checkBox, wxSizer& checkBoxSizer,
		const wxString& shortDescription, const wxString& flagString,
		int flagIndex, bool isRecommendedFlag);
	~FlagListCheckBoxItem();
	const wxString& GetFsoCategory() const { return this->fsoCategory; }
	MyFlagListCheckBox* GetCheckBox() { return this->checkBox; }
	wxSizer* GetCheckBoxSizer() { return this->checkBoxSizer; }
	const wxString& GetShortDescription() const { return this->shortDescription; }
	const wxString& GetFlagString() const { return this->flagString; }
	int GetFlagIndex() const { return this->flagIndex; }
	bool IsRecommendedFlag() const { return this->isRecommendedFlag; }
private:
	FlagListCheckBoxItem();
	wxString fsoCategory;
	MyFlagListCheckBox* checkBox;
	wxSizer* checkBoxSizer;
	wxString shortDescription;
	wxString flagString;
	int flagIndex; // so proxy can keep flags ordered in flag list order
	bool isRecommendedFlag;
};

WX_DECLARE_LIST(FlagListCheckBoxItem, FlagListCheckBoxItems);

/** Flag list box is ready for use. */
DECLARE_EVENT_TYPE(EVT_FLAG_LIST_BOX_READY, wxID_ANY);

WX_DECLARE_LIST(wxEvtHandler, FlagListBoxReadyEventHandlers);

class FlagListBox: public wxVListBox {
public:
	FlagListBox(wxWindow* parent, SkinSystem* skin);
	~FlagListBox();
	
	void RegisterFlagListBoxReady(wxEvtHandler *handler);
	void UnRegisterFlagListBoxReady(wxEvtHandler *handler);

	// TODO remove during changeover. make sure OnDrawItem() and OnDrawBackground() stay virtual
	void MyOnDrawItem(wxDC &dc, const wxRect &rect, size_t n) const;
	virtual void OnDrawItem(wxDC &dc, const wxRect &rect, size_t n) const;
	void MyOnDrawBackground(wxDC &dc, const wxRect &rect, size_t n) const;
	virtual void OnDrawBackground(wxDC &dc, const wxRect &rect, size_t n) const;
	virtual wxCoord OnMeasureItem(size_t n) const;
	virtual void OnSize(wxSizeEvent &event);

	void OnDoubleClickFlag(wxCommandEvent &event);

	// TODO will eventually get removed once the proxy is in place, I think
	wxString GenerateStringList();
	wxString MyGenerateStringList() const;

	/** Tries to find flagString in the list of flags and set it to state,
	returns true on successful set, returns false if cannot find flag. */
	bool SetFlag(const wxString& flagString, bool state);
	bool MySetFlag(const wxString& flagString, bool state);
	/** Tries to find the flagSet specified and then set or unset all flags
	contained in the flag set, returns true on success, returns false
	iff it cannot find the flagset.  That is, will return true if none of
	the flags in the flag set are real flags. */
	bool SetFlagSet(const wxString& flagSet);
	void GetFlagSets(wxArrayString& arr) const;
	
	void ResetFlags();
	void MyResetFlags();
	
	void AcceptFlagData(FlagFileData* flagData);
	
	bool IsReady() const { return this->isReady; }

private:
	FlagListBoxReadyEventHandlers flagListBoxReadyHandlers;
	void GenerateFlagListBoxReady();
	bool isReadyEventGenerated;
	bool isReady;
	
	SkinSystem* skin;
	
	FlagFileData* flagData;
	FlagListCheckBoxItems checkBoxes;
	void GenerateCheckBoxes(const FlagListBoxData& data);
	bool areCheckBoxesGenerated;

	wxStaticText* errorText;

	void FindFlagAt(size_t n, Flag **flag, Flag ** catFlag) const;
	FlagListCheckBoxItem* MyFindFlagAt(size_t n) const;

	DECLARE_EVENT_TABLE();

};

#endif
