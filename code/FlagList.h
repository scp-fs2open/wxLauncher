#ifndef FLAGLIST_H
#define FLAGLIST_H

#include <wx/wx.h>
#include <wx/vlbox.h>

#include "Skin.h"

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
