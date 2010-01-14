#ifndef MODLIST_H
#define MODLIST_H

#include <wx/wx.h>
#include <wx/vlbox.h>
#include <wx/fileconf.h>
#include <wx/arrstr.h>

#include "Skin.h"

class ConfigPair {
public:
	ConfigPair(wxString &shortname, wxFileConfig* config);
	~ConfigPair();
	wxString shortname;
	wxFileConfig* config;
};
WX_DECLARE_OBJARRAY(ConfigPair, ConfigArray);


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
	ModItem(SkinSystem* skin);
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

	void Draw(wxDC &dc, const wxRect &rect, bool selected, wxSizer *mainSizer, wxSizer *buttons, wxStaticBitmap* warn);

private:
	SkinSystem* skinSystem;


	class InfoText{
	public:
		InfoText(ModItem *myData);
		
		void Draw(wxDC &dc, const wxRect &rect);
	private:
		ModItem *myData;
	};
	
	InfoText *infoTextPanel;

	class ModImage{
	public:
		ModImage(ModItem *myData);

		void Draw(wxDC &dc, const wxRect &rect);
	private:
		ModItem *myData;
	};

	ModImage *modImagePanel;

	class ModName{
	public:
		ModName(ModItem *myData);

		void Draw(wxDC &dc, const wxRect &rect);
	private:
		ModItem *myData;
	};

	ModName* modNamePanel;
};

WX_DECLARE_OBJARRAY(ModItem, ModItemArray);


class ModList: public wxVListBox {
public:
	ModList(wxWindow *parent, wxSize& size, SkinSystem *skin, wxString tcPath);
	~ModList();

	// overrides for wxVListBox
	virtual void OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const;
	virtual void OnDrawSeparator(wxDC &dc, wxRect& rect, size_t n) const;
	virtual void OnDrawBackground(wxDC &dc, const wxRect &rect, size_t n) const;
	virtual wxCoord OnMeasureItem(size_t n) const;

	void OnSelectionChange(wxCommandEvent &event);
	void OnActivateMod(wxCommandEvent &event);
	void OnInfoMod(wxCommandEvent &event);

private:
	/** A hash map of the wxFileConfigs that represent the mod.ini files for
	each mod.  The key is the the mod's folder name which is used as the mod's
	internal name. */
	ConfigArray* configFiles;
	wxChar semicolon[2];
	SkinSystem* skinSystem;
	wxString stringNoMod;
	
	wxButton *infoButton, *activateButton;
	wxStaticBitmap *warnBitmap;
	wxBoxSizer *buttonSizer, *sizer;

	void readIniFileString(wxFileConfig* config,
		wxString keyvalue, wxString ** location);
	void readFlagSet(wxFileConfig* config,
		wxString keyprefix, FlagSetItem * set);
	void readTranslation(wxFileConfig* config,
		wxString langaugename, I18nItem ** trans);
	wxString excapeSpecials(wxString toexcape);

	ModItemArray* tableData;
	wxButton * testbutton;

	DECLARE_EVENT_TABLE();
};

#endif