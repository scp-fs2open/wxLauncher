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
};

WX_DECLARE_LIST(Flag, FlagList);

/** Contains all of the flags in a category. */
class FlagCategory {
public:
	wxString categoryName;
	FlagList flags;
};

WX_DECLARE_LIST(FlagCategory, FlagCategoryList);

class FlagListBox: public wxVListBox {
public:
	FlagListBox(wxWindow* parent, SkinSystem* skin);
	~FlagListBox();

	virtual void OnDrawItem(wxDC &dc, const wxRect &rect, size_t n) const;
	virtual void OnDrawBackground(wxDC &dc, const wxRect &rect, size_t n) const;
	virtual wxCoord OnMeasureItem(size_t n) const;
	virtual void OnSize(wxSizeEvent &event);

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

	FlagCategoryList allSupportedFlagsByCategory;

	wxStaticText* errorText;

	void FindFlagAt(size_t n, Flag **flag, Flag ** catFlag) const;

	DECLARE_EVENT_TABLE();

};

#endif