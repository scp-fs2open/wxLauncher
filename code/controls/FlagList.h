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

#ifndef FLAGLIST_H
#define FLAGLIST_H

#include <wx/wx.h>
#include <wx/vlbox.h>

#include "apis/SkinManager.h"

class Flag {
public:
	Flag();
	wxString flagString;
	wxString shortDescription;
	wxString fsoCatagory;
	wxString webURL;
	wxCheckBox *checkbox;
	wxSizer* checkboxSizer;
	bool isRecomendedFlag;
	wxUint32 easyEnable;
	wxUint32 easyDisable;
};

WX_DECLARE_LIST(Flag, FlagList);

/** Contains all of the flags in a category. */
class FlagCategory {
public:
	wxString categoryName;
	FlagList flags;
};

WX_DECLARE_LIST(FlagCategory, FlagCategoryList);

class FlagSet {
public:
	FlagSet(wxString Name);
	wxString Name;
	wxArrayString FlagsToEnable;
	wxArrayString FlagsToDisable;
};

WX_DECLARE_LIST(FlagSet, FlagSetsList);

class FlagListBox: public wxVListBox {
public:
	FlagListBox(wxWindow* parent, SkinSystem* skin);
	~FlagListBox();

	virtual void OnDrawItem(wxDC &dc, const wxRect &rect, size_t n) const;
	virtual void OnDrawBackground(wxDC &dc, const wxRect &rect, size_t n) const;
	virtual wxCoord OnMeasureItem(size_t n) const;
	virtual void OnSize(wxSizeEvent &event);

	void OnCheckCategoryBox(wxCommandEvent &event);
	wxString GenerateStringList();
	/** Tries to find flagString in the list of flags and set it to state,
	returns true on successful set, returns false if cannot find flag. */
	bool SetFlag(wxString flagString, bool state);
	/** Tries to find the flagSet specified and then set or unset all flags
	contained in the flag set, returns true on success, returns false
	iff it cannot find the flagset.  That is, will return true if none of
	the flags in the flag set are real flags. */
	bool SetFlagSet(wxString flagSet);
	wxArrayString& GetFlagSets(wxArrayString &arr);

	bool IsDrawOK();

private:
	SkinSystem* skin;
	enum DrawStatus {
		DRAW_OK = 0,
		MISSING_TC,
		MISSING_EXE,
		INVALID_BINARY,
		WAITING_FOR_FLAGFILE,
		FLAG_FILE_NOT_GENERATED,
		FLAG_FILE_NOT_VALID,
		FLAG_FILE_NOT_SUPPORTED,
		MAX_DRAWSTATUS
	};
	DrawStatus drawStatus; //!< are the draw methods allowed to draw.
	DrawStatus ParseFlagFile(wxFileName &flagfile);

	wxArrayString easyflags;
	FlagSetsList flagSets;
	void generateFlagSets();

	FlagCategoryList allSupportedFlagsByCategory;

	wxStaticText* errorText;

	void FindFlagAt(size_t n, Flag **flag, Flag ** catFlag) const;

	DECLARE_EVENT_TABLE();

};

#endif
