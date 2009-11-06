#ifndef MODLIST_H
#define MODLIST_H

#include <wx/wx.h>
#include <wx/vlbox.h>
#include <wx/fileconf.h>
#include <wx/arrstr.h>

#include "Skin.h"

WX_DECLARE_HASH_MAP( wxString, wxFileConfig*, wxStringHash, wxStringEqual , ConfigHash);


class FlagSetItem {
public:
	FlagSetItem();
	~FlagSetItem();
	wxString* name;
	wxString* flagset;
	wxString* notes;
};

WX_DECLARE_OBJARRAY(FlagSetItem, FlagSets);

extern wxSortedArrayString SupportedLanguages;

class I18nItem {
public:
	I18nItem();
	~I18nItem();
	wxString* modname;
	wxString* infotext;
};

WX_DECLARE_STRING_HASH_MAP(I18nItem*, I18nData);


struct Words {
	wxString word;
	wxSize size;
};

WX_DECLARE_OBJARRAY(Words, ArrayOfWords);

class ModItem{
public:
	ModItem(wxWindow *parent, SkinSystem* skin);
	~ModItem();
	wxString* name;
	wxString* shortname;
	wxBitmap* image;
	wxString* infotext;
	wxString* author;
	wxString* notes;
	bool warn;
	wxString* website;
	wxString* forum;
	wxString* bugs;
	wxString* support;

	wxString* forcedon;
	wxString* forcedoff;

	wxString* primarylist;
	wxString* secondarylist;

	FlagSets* flagsets;	// set 0 is the ideal set.
	
	Skin* skin;

	I18nData* i18n;

	void Draw(wxDC &dc, const wxRect &rect) {
		this->infotextpanel->Draw(dc, rect);
	}

private:
	wxPanel *panel;
	wxButton *infoButton, *activateButton;
	SkinSystem* skinSystem;


	class InfoText : public wxPanel {
	public:
		InfoText(wxWindow *parent, ModItem *myData);
		
		virtual void OnDraw(wxPaintEvent &event);
		void Draw(wxDC &dc, const wxRect &rect);
	private:
		ModItem *myData;

		DECLARE_EVENT_TABLE();
	};
	
	InfoText *infotextpanel;

	class ModImage : public wxPanel {
	public:
		ModImage(wxWindow *parent, ModItem *myData);

		void OnDraw(wxPaintEvent &event);
	private:
		ModItem *myData;
	};
	class ModName : public wxPanel {
	public:
		ModName(wxWindow *parent, ModItem *myData);

		void OnDraw(wxPaintEvent &event);
	private:
		ModItem *myData;
	};
};

WX_DECLARE_OBJARRAY(ModItem, ModItemArray);


class ModList: public wxVListBox {
public:
	ModList(wxWindow *parent, wxSize& size, SkinSystem *skin);
	~ModList();

	// overrides for wxVListBox
	virtual void OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const;
	virtual void OnDrawSeparator(wxDC &dc, wxRect& rect, size_t n) const;
	virtual void OnDrawBackground(wxDC &dc, const wxRect &rect, size_t n) const;
	virtual wxCoord OnMeasureItem(size_t n) const;

	void OnSelectionChange(wxCommandEvent &event);

private:
	/** A hash map of the wxFileConfigs that represent the mod.ini files for
	each mod.  The key is the the mod's folder name which is used as the mod's
	internal name. */
	ConfigHash* configFiles;
	wxChar* semicolon;
	SkinSystem* skinSystem;

	void readIniFileString(ConfigHash::mapped_type config,
		wxString keyvalue, wxString ** location);
	void readFlagSet(ConfigHash::mapped_type config,
		wxString keyprefix, FlagSetItem * set);
	void readTranslation(ConfigHash::mapped_type config,
		wxString langaugename, I18nItem ** trans);

	ModItemArray* tableData;
	wxButton * testbutton;

	DECLARE_EVENT_TABLE();
};

#endif