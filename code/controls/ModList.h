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

#ifndef MODLIST_H
#define MODLIST_H

#include <wx/wx.h>
#include <wx/vlbox.h>
#include <wx/fileconf.h>
#include <wx/arrstr.h>

#include "apis/SkinManager.h"

#include "controls/LightingPresets.h"

class ConfigPair {
public:
	ConfigPair(const wxString &shortname, wxFileConfig* config);
	~ConfigPair();
	wxString shortname;
	wxFileConfig* config;
};
WX_DECLARE_OBJARRAY(ConfigPair, ConfigArray);


class FlagSetItem {
public:
	FlagSetItem();
	~FlagSetItem();
	wxString name;
	wxString flagset;
	wxString notes;
};

WX_DECLARE_OBJARRAY(FlagSetItem, FlagSets);

#ifdef MOD_TEXT_LOCALIZATION // mod text localization is not supported for now
extern wxSortedArrayString SupportedLanguages;

class I18nItem {
public:
	I18nItem();
	~I18nItem();
	wxString* modname;
	wxString* infotext;
};

WX_DECLARE_STRING_HASH_MAP(I18nItem*, I18nData);
#endif


class ModItem{
public:
	ModItem();
	~ModItem();
	wxString name;
	wxString shortname;
	wxBitmap image255x112;
	wxBitmap image182x80;
	wxString infotext;
	wxString author;
	wxString notes;
	bool warn;
	wxString website;
	wxString forum;
	wxString bugs;
	wxString support;
	
	long minhorizontalres;
	long minverticalres;

	wxString forcedon;
	wxString forcedoff;

	wxString primarylist;
	wxString secondarylist;
	
	wxString recommendedlightingname;
	Preset recommendedlightingpreset;

	FlagSets* flagsets;	// set 0 is the ideal set.

#ifdef MOD_TEXT_LOCALIZATION // mod text localization is not supported for now
	I18nData* i18n;
#endif

	void Draw(wxDC &dc, const wxRect &rect, bool selected, wxSizer *mainSizer, wxSizer *buttons, wxStaticBitmap* warn);

private:
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
	ModList(wxWindow *parent, wxSize& size, wxString tcPath);
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
	
	ModItemArray* tableData;
	
	Skin* TCSkin;
	
	wxButton *infoButton, *activateButton;
	wxStaticBitmap *warnBitmap;
	wxBoxSizer *buttonSizer, *sizer;
	
	/** Sets a bitmap of the ModList's TCSkin. */
	void SetSkinBitmap(
		const wxFileConfig& config,
		const wxString& modIniKey,
		const wxString& tcPath,
		const wxString& bitmapName,
		bool (Skin::* setFnPtr)(const wxBitmap&));

	void readIniFileString(const wxFileConfig* config,
		const wxString& key, wxString& location);
	void readFlagSet(wxFileConfig* config,
		wxString keyprefix, FlagSetItem * set);
#ifdef MOD_TEXT_LOCALIZATION // mod text localization is not supported for now
	void readTranslation(wxFileConfig* config,
		wxString langaugename, I18nItem ** trans);
#endif
	wxString escapeSpecials(const wxString& toEscape);
	
	bool ParseModIni(const wxString& modIniPath, const wxString& tcPath, bool isNoMod = false);
	void SetSelectedMod();
	static wxString GetShortName(const wxString& modIniPath, const wxString& tcPath);

	/** The active mod's prepend mods and append mods. */
	wxString prependmods, appendmods;
	
	/** Tests whether a mod is a prepend/append mod of the active mod. */
	bool isAPrependMod(const wxString& mod) const;
	bool isAnAppendMod(const wxString& mod) const;

	/** Tests whether a mod appears in the specified mod list. */
	static bool isADependency(const wxString& mod, const wxString& modlist);
	
	/** Tests whether a mod is a prepend/append mod of the selected mod. */
	bool isCurrentSelectionAPrependMod(const wxString &mod) const;
	bool isCurrentSelectionAnAppendMod(const wxString &mod) const;

	DECLARE_EVENT_TABLE();
};

#endif