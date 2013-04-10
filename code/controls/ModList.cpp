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

#include <algorithm>
#include <vector>

#include <wx/wx.h>
#include <wx/vlbox.h>
#include <wx/fileconf.h>
#include <wx/wfstream.h>
#include <wx/sstream.h>
#include <wx/mstream.h>
#include <wx/tokenzr.h>
#include <wx/arrstr.h>
#include <wx/filename.h>
#include <wx/dir.h>
#include <wx/html/htmlwin.h>

#include "apis/SkinManager.h"
#include "global/ids.h"
#include "global/ProfileKeys.h"
#include "global/ModDefaults.h"
#include "global/Utils.h"
#include "controls/ModList.h"
#include "apis/ProfileManager.h"
#include "apis/TCManager.h"

#include "global/MemoryDebugging.h"

using TextUtils::Words;
using TextUtils::ArrayOfWords;

const wxString NO_MOD(_("(No mod)"));

class ModInfoDialog: wxDialog {
public:
	ModInfoDialog(ModItem* item, wxWindow* parent);
	void OnLinkClicked(wxHtmlLinkEvent &event);

private:
	class ImageDrawer: public wxPanel {
	public:
		ImageDrawer(ModInfoDialog* parent);

		void OnDraw(wxPaintEvent &event);

	private:
		ModInfoDialog* parent;

		DECLARE_EVENT_TABLE();
	};
	friend class ImageDrawer;

	ModItem* item;
};


ConfigPair::ConfigPair(const wxString &shortname, wxFileConfig *config)  {
	this->shortname = shortname;
	this->config = config;
}

ConfigPair::~ConfigPair() {
	if ( this->config != NULL ) {
		delete this->config;
	}
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ConfigArray);

bool CompareModItems(ModItem* item1, ModItem* item2) {
	wxASSERT(item1 != NULL);
	wxASSERT(item2 != NULL);
	
	wxString item1Name(
		(!item1->name.IsEmpty()) ? item1->name : item1->shortname);
	
	wxString item2Name(
		(!item2->name.IsEmpty()) ? item2->name : item2->shortname);

	// ignore a leading "the" for comparison purposes
	wxString temp;
	
	if (item1Name.Lower().StartsWith(_T("the"), &temp)) {
		item1Name = temp.Trim(false);
	}
	
	if (item2Name.Lower().StartsWith(_T("the"), &temp)) {
		item2Name = temp.Trim(false);
	}
	
	// (No mod) must come before all other mods
	if (!item1->shortname.Cmp(NO_MOD)) {
		return true;
	} else if (!item2->shortname.Cmp(NO_MOD)) {
		return false;
	} else {
		return item1Name.CmpNoCase(item2Name) < 0;
	}
}

ModList::ModList(wxWindow *parent, wxSize& size, wxString tcPath) {
	this->Create(parent, ID_MODLISTBOX, wxDefaultPosition, size, 
		wxLB_SINGLE | wxLB_ALWAYS_SB | wxBORDER);
	this->SetMargins(10, 10);

	std::vector<ModItem*> modsTemp; // for use in presorting

	this->tableData = new ModItemArray();
	// scan for mods in the current TCs directory
	wxArrayString foundInis;
	wxDir::GetAllFiles(tcPath, &foundInis, _("mod.ini"));
	if ( foundInis.Count() > 0 ) {
		wxLogDebug(_T("I found %d .ini files:"), foundInis.Count());
	} else {
		wxLogDebug(_T("I did not find any .ini files."));
	}

	// parse mod.ini's in all of the directories that contain one
	this->configFiles = new ConfigArray();

	wxLogDebug(_T("Inserting '(No mod)'"));
	wxFileName tcmodini(tcPath, _T("mod.ini"));
	if ( tcmodini.IsOk() && tcmodini.FileExists() ) {
		wxLogDebug(_T(" Found a mod.ini in the root TC folder. (%s)"), tcmodini.GetFullPath().c_str());

		if (!ParseModIni(tcmodini.GetFullPath(), tcPath, true)) {
			wxLogError(_T(" Error parsing mod.ini in the root TC folder. (%s)"),
				tcmodini.GetFullPath().c_str());
		}

		// make sure that a mod.ini in the root TC folder is not apart of this set
		// because it will be addressed shortly and specificly
		int pos = foundInis.Index(tcmodini.GetFullPath());
		if ( pos != wxNOT_FOUND ) {
			foundInis.RemoveAt(pos);
		}

	} else {
		this->configFiles->Add(new ConfigPair(NO_MOD, new wxFileConfig()));
		wxLogDebug(_T(" Using defaults for TC."));
	}

	wxLogDebug(_T("Starting to parse mod.ini's..."));
	for (size_t i = 0; i < foundInis.Count(); i++) {
		wxLogDebug(_T("  Parsing %s"), foundInis.Item(i).c_str());

		if (!ParseModIni(foundInis.Item(i), tcPath)) {
			wxLogError(_T("  Parsing %s failed."), foundInis.Item(i).c_str());
		}
	}

	// create internal repesentation of the mod.ini's
	wxLogDebug(_T("Transforming mod.ini's"));
	
	for(size_t i = 0; i < this->configFiles->size(); i++) {
		wxString shortname = this->configFiles->Item(i).shortname;
		wxFileConfig* config = this->configFiles->Item(i).config;
		ModItem* item = new ModItem();
		wxLogDebug(_T(" %s"), shortname.c_str());

		item->shortname = shortname;

		readIniFileString(config, _T("/launcher/modname"), item->name);

		// TODO allow TC authors to specify a specific directory to load this image from,
		// in case the TC author doesn't want the TC root folder to be cluttered
		// with the images that they specify for the skin
		wxString smallimagepath;
		readIniFileString(config, _T("/launcher/image255x112"), smallimagepath);
		if ( !smallimagepath.IsEmpty() ) {
			if (shortname == NO_MOD) {
				item->image = SkinSystem::VerifySmallImage(tcPath, wxEmptyString,
					smallimagepath);
			} else {
				item->image = SkinSystem::VerifySmallImage(tcPath, shortname,
					smallimagepath);
			}
		}
		
		readIniFileString(config, _T("/launcher/infotext"), item->infotext);

		readIniFileString(config, _T("/launcher/author"), item->author);

		readIniFileString(config, _T("/launcher/notes"), item->notes);

		config->Read(_T("/launcher/warn"), &(item->warn), false);

		readIniFileString(config, _T("/launcher/website"), item->website);
		readIniFileString(config, _T("/launcher/forum"), item->forum);
		readIniFileString(config, _T("/launcher/bugs"), item->bugs);
		readIniFileString(config, _T("/launcher/support"), item->support);

		readIniFileString(config, _T("/extremeforce/forcedflagson"), item->forcedon);
		readIniFileString(config, _T("/extremeforce/forcedflagsoff"), item->forcedoff);

		readIniFileString(config, _T("/multimod/primarylist"), item->primarylist);
		// Log the warning for any mod authors, specifically for those who indicate
		// that they are mod authors by their having FRED launching enabled
		bool fredEnabled;
		ProMan::GetProfileManager()->GlobalRead(GBL_CFG_OPT_CONFIG_FRED, &fredEnabled, false);
		
		if ( config->Exists(_T("/multimod/secondrylist")) && fredEnabled) {
			wxLogInfo(_T("  DEPRECATION WARNING: Mod '%s' uses deprecated mod.ini parameter 'secondrylist'"),
				shortname.c_str());
		}
		readIniFileString(config, _T("/multimod/secondrylist"), item->secondarylist);
		readIniFileString(config, _T("/multimod/secondarylist"), item->secondarylist);

		// flag sets
		if ( config->Exists(_T("/flagsetideal")) ) {
			item->flagsets = new FlagSets();

			FlagSetItem* flagset = new FlagSetItem();

			readFlagSet(config, _T("/flagsetideal"), flagset);

			item->flagsets->Add(flagset);

			unsigned int counter = 1;
			bool done = false;
			do {
				wxString sectionname = wxString::Format(_T("/flagset%d"), counter);
				if ( config->Exists( sectionname )) {
					FlagSetItem* numberedflagset = new FlagSetItem();

					readFlagSet(config, sectionname, numberedflagset);
					
					item->flagsets->Add(numberedflagset);
				} else {
					done = true;
				}
				counter++;
			} while ( !done );
		} else {
#if 0 // preprocessing out until this functionality is complete
			wxLogDebug(_T("  Does Not Contain An idealflagset Section."));
#endif
		}

		// skin
		// TODO: If only TCs have skins and not mods, maybe rethink this.
		// For example, does every mod item need a skin?
		if ( config->Exists(_T("/skin")) ) {
			item->skin = new Skin();

			wxString windowTitle;
			readIniFileString(config, _T("/skin/wtitle"), windowTitle);
			if ( !windowTitle.IsEmpty() ) {
				item->skin->SetWindowTitle(windowTitle);
			}

			wxString windowIconFile;
			readIniFileString(config, _T("/skin/wicon"), windowIconFile);
			if ( !windowIconFile.IsEmpty() ) {
				// TODO: let SetWindowIcon() do the image validation
				wxIcon* windowIcon =
					SkinSystem::VerifyWindowIcon(tcPath, shortname, windowIconFile);
				if (windowIcon != NULL) {
					item->skin->SetWindowIcon(*windowIcon);
					delete windowIcon;
				}
			}

			wxString welcomeText;
			readIniFileString(config, _T("/skin/welcometxt"), welcomeText);
			if ( !welcomeText.IsEmpty() ) {
				item->skin->SetWelcomeText(welcomeText);
			}

			wxString idealIconFile;
			readIniFileString(config, _T("/skin/idealicon"), idealIconFile);
			if ( !idealIconFile.IsEmpty() ) {
				// TODO: let SetIdealIcon() do the image validation
				wxBitmap* idealIcon =
					SkinSystem::VerifyIdealIcon(tcPath, shortname, idealIconFile);
				if (idealIcon != NULL) {
					item->skin->SetIdealIcon(*idealIcon);
					delete idealIcon;
				}
			}

		} else {
#if 0 // preprocessing out until this functionality is complete
			wxLogDebug(_T("  Does Not Contain An skin Section."));
#endif
		}

#ifdef MOD_TEXT_LOCALIZATION // mod text localization is not supported for now
		// langauges
		for ( size_t i = 0;	i < SupportedLanguages.Count(); i++ ) {
			wxString section = wxString::Format(_T("/%s"), SupportedLanguages[i].c_str());
			if ( config->Exists(section) ) {
				if ( item->i18n == NULL ) {
					item->i18n = new I18nData();
				}
			}
			I18nItem *temp = NULL;

			readTranslation(config, SupportedLanguages[i], &temp);
			
			if ( temp != NULL ) {
				(*(item->i18n))[SupportedLanguages[i]] = temp;
			}
		}
#endif

		modsTemp.push_back(item);
	}
	
	std::sort(modsTemp.begin(), modsTemp.end(), CompareModItems);
	
	for (std::vector<ModItem*>::const_iterator it = modsTemp.begin();
		 it != modsTemp.end(); ++it) {
		this->tableData->Add(*it);
	}

	this->SetItemCount(this->tableData->Count());

	SetSelectedMod();


	this->infoButton = 
		new wxButton(this, ID_MODLISTBOX_INFO_BUTTON, _("Info"));
	this->activateButton = 
		new wxButton(this, ID_MODLISTBOX_ACTIVATE_BUTTON, _("Activate"));
	this->warnBitmap =
		new wxStaticBitmap(this, wxID_ANY, SkinSystem::GetSkinSystem()->GetWarningIcon());
	this->warnBitmap->SetToolTip(_("This mod requires your attention before playing it. Click Info for more details."));

	this->buttonSizer = new wxBoxSizer(wxVERTICAL);
	this->buttonSizer->AddStretchSpacer(2);
	this->buttonSizer->Add(this->activateButton, wxSizerFlags().Expand().ReserveSpaceEvenIfHidden());
	this->buttonSizer->AddStretchSpacer(1);
	this->buttonSizer->Add(this->infoButton, wxSizerFlags().Expand().ReserveSpaceEvenIfHidden());
	this->buttonSizer->AddStretchSpacer(2);

	wxSizer* warningSizer = new wxBoxSizer(wxVERTICAL);
	warningSizer->AddStretchSpacer(1);
	warningSizer->Add(this->warnBitmap, wxSizerFlags().ReserveSpaceEvenIfHidden());
	warningSizer->AddStretchSpacer(1);

	this->sizer = new wxBoxSizer(wxHORIZONTAL);
	this->sizer->AddStretchSpacer(1);
	this->sizer->Add(warningSizer, wxSizerFlags().Expand().ReserveSpaceEvenIfHidden());
	this->sizer->AddStretchSpacer(1);
	this->sizer->Add(this->buttonSizer, wxSizerFlags().Expand().ReserveSpaceEvenIfHidden());
	this->sizer->AddStretchSpacer(2);
	this->buttonSizer->Show(false);
	this->warnBitmap->Show(false);


}

/** the dtor.  Cleans up stuff. */
ModList::~ModList() {
	if ( this->configFiles != NULL ) {
		delete this->configFiles;
	}
	if ( this->tableData != NULL ) {
		delete this->tableData;
	}
	if ( this->sizer != NULL ) {
		delete this->sizer;
	}
}

/** Takes the key to search for and sets location to key's value.
    If the key is not found, location is unchanged. */
void ModList::readIniFileString(const wxFileConfig* config,
		const wxString& key, wxString& location) {
	wxASSERT(config != NULL);
	wxASSERT(location.IsEmpty());

	if ( config->HasEntry(key) ) {
		config->Read(key, &location);
		if ( location.EndsWith(_T(";")) ) {
			location.RemoveLast();
		}
	}

	wxLogDebug(_T("  %s:'%s'"), key.c_str(),
		location.IsEmpty() ? _T("Not Specified") : escapeSpecials(location).c_str());
}

/** re-escape the newlines in the mod.ini values. */
wxString ModList::escapeSpecials(const wxString& toEscape) {
	wxString toEscapeTemp(toEscape);

	wxString::iterator iter = toEscapeTemp.begin();

	while (iter != toEscapeTemp.end() ) {
		if ( *iter == wxChar('\n') ) {
			wxString::iterator end = iter;
			end++;
			toEscapeTemp.replace(iter, end, _T("\\n"));

			// have to start over because we wrote to the string,
			// which invalidated the iterator.
			iter = toEscapeTemp.begin();
		} else {
			++iter;
		}
	}

	return toEscapeTemp;
}


/** */
void ModList::readFlagSet(wxFileConfig* config,
							   wxString keyprefix, FlagSetItem *set) {
	readIniFileString(config,
    wxString::Format(_T("%s/name"), keyprefix.c_str()),
    set->name);
	readIniFileString(config, 
    wxString::Format(_T("%s/flagset"), keyprefix.c_str()), 
    set->flagset);
	readIniFileString(config, 
    wxString::Format(_T("%s/notes"), keyprefix.c_str()), 
    set->notes);
}

#ifdef MOD_TEXT_LOCALIZATION // mod text localization is not supported for now
void ModList::readTranslation(wxFileConfig* config, wxString langaugename, I18nItem **trans) {
	wxString section = wxString::Format(_T("/%s"), langaugename.c_str());
	if ( config->Exists(section) ) {
		*trans = new I18nItem();

		readIniFileString(config, 
      wxString::Format(_T("%s/modname"), section.c_str()),
			&((*trans)->modname));
		readIniFileString(config, 
      wxString::Format(_T("%s/infotext"), section.c_str()),
			&((*trans)->infotext));

	} else {
		wxLogDebug( 
			wxString::Format(_T("  Section '%s' does not exist."), langaugename.c_str()));
	}
}
#endif

/** Parses the specified mod.ini file and adds it to configFiles.
    Returns true on success, false otherwise. */
bool ModList::ParseModIni(const wxString& modIniPath, const wxString& tcPath, const bool isNoMod) {
	wxFFileInputStream stream(modIniPath);

	if ( stream.IsOk() ) {
		wxLogDebug(_T("   Opened ok"));
	} else {
		wxLogError(_T("   Open failed!"));
		return false;
	}

	// check if the stream is a UTF-8 File (has a BOM)
	char header[3];
	stream.Read(reinterpret_cast<void*>(&header), sizeof(header));
	stream.SeekI(0);
	
	bool isUTF8 = false;
	if ( header[0] == '\357' && header[1] == '\273' && header[2] == '\277' ) {
		// is a UTF-8 file
		isUTF8 = true;
	}

	wxMemoryOutputStream tempStream;
	tempStream.Write(stream);

	wxStreamBuffer* buf = tempStream.GetOutputStreamBuffer();
	const size_t size = buf->GetBufferSize();

	char* characterBuffer = new char[size+1];
	characterBuffer[size] = '\0';

	buf->Seek(0, wxFromStart);

	// don't try to read in buffer when there is nothing to read.
	size_t read = (size == 0) ? 0 : buf->Read(reinterpret_cast<void*>(characterBuffer), size);
	if ( read != size ) {
		wxLogError(_T("read (%d) not equal to size (%d)"), read, size);
		delete[] characterBuffer;
		return false;
	}

	const wxMBConv* conv = NULL;
	if ( isUTF8 ) {
		conv = &wxConvUTF8;
	} else {
		conv = &wxConvISO8859_1;
	}

	wxString stringBuffer(characterBuffer, *conv);

	// A hack to insert a backslash into the stream so that when
	// wxFileConfig escapes the backslashes, the one that is in 
	// the file is returned
	stringBuffer.Replace(_T("\\"), _T("\\\\"));
	wxStringInputStream finalBuffer(stringBuffer);

	wxFileConfig* config = new wxFileConfig(finalBuffer);
	delete[] characterBuffer;

	wxString shortname(isNoMod ? NO_MOD : GetShortName(modIniPath, tcPath));
	
	if (!isNoMod) {
		wxLogDebug(_T("   Mod fancy name is: %s"),
			config->Read(_T("/launcher/modname"), _T("Not specified")).c_str());

		wxLogDebug(_T("   Mod short name is: %s"), shortname.c_str());
	} 

	this->configFiles->Add(new ConfigPair(shortname, config));

	return true;
}

/** Set currently select mod as selected
    or set (No mod) if none or previous does not exist. */
void ModList::SetSelectedMod() {
	wxString currentMod;
	ProMan::GetProfileManager()->ProfileRead(
		PRO_CFG_TC_CURRENT_MOD, &currentMod, NO_MOD);
	
	size_t i;
	for ( i = 0; i < this->tableData->size(); ++i ) {
		if ( this->tableData->Item(i).shortname == currentMod ) {
			break;
		}
	}
	
	if ( i < this->tableData->size() ) {
		// found it
		this->SetSelection(i);
	} else {
		this->SetSelection(0);
	}
	
	wxCommandEvent activateModEvent;
	this->OnActivateMod(activateModEvent);
}

/** get the mod.ini's short name (base directory) */
/** <something>/modfolder/mod.ini
    <something>\modfolder\mod.ini */
wxString ModList::GetShortName(const wxString& modIniPath, const wxString& tcPath) {
	wxArrayString tokens = wxStringTokenize(modIniPath, _T("\\/"),
		wxTOKEN_STRTOK); /* breakup on folder markers and never return an
						 empty string. */
	wxArrayString tcTokens = wxStringTokenize(tcPath, _T("\\/"), wxTOKEN_STRTOK);

	wxString shortname;

	 // "tokens.GetCount() - 1" is to skip "mod.ini" at end
	for ( size_t j = tcTokens.GetCount(); j < (tokens.GetCount() - 1); ++j ) {
		if ( !shortname.IsEmpty() ) {
			shortname += _T("/");
		}
		shortname += tokens[j];
	}
	
	return shortname;
}

void ModList::OnDrawItem(wxDC &dc, const wxRect &rect, size_t n) const {
	wxLogDebug(_T(" Draw %04d,%04d = %04d,%04d"), rect.x, rect.y, rect.width, rect.height);
	this->tableData->Item(n).Draw(dc, rect, this->IsSelected(n), this->sizer, this->buttonSizer, this->warnBitmap);
}

void ModList::OnDrawSeparator(wxDC &WXUNUSED(dc), wxRect& WXUNUSED(rect), size_t WXUNUSED(n)) const {
	//dc.DrawLine(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
}

void ModList::OnDrawBackground(wxDC &dc, const wxRect& rect, size_t n) const {
	wxLogDebug(_T(" Background %04d,%04d = %04d,%04d"), rect.x, rect.y, rect.width, rect.height);
	dc.DestroyClippingRegion();
	wxColour highlighted = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
	wxColour background = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
	wxString activeMod;
	ProMan::GetProfileManager()->ProfileRead(PRO_CFG_TC_CURRENT_MOD, &activeMod, NO_MOD, true);
	wxBrush b;
	wxRect selectedRect(rect.x+2, rect.y+2, rect.width-4, rect.height-4);
	wxRect activeRect(selectedRect.x+3, selectedRect.y+3, selectedRect.width-7, selectedRect.height-7);

	if ( this->IsSelected(n) ) {
		b = wxBrush(highlighted, wxTRANSPARENT);
		dc.SetPen(wxPen(highlighted, 4));
	} else if ( this->isCurrentSelectionAnAppendMod(this->tableData->Item(n).shortname) ) {
		b = wxBrush(highlighted, wxBDIAGONAL_HATCH);
		dc.SetPen(wxPen(highlighted, 1));
	} else if ( this->isCurrentSelectionAPrependMod(this->tableData->Item(n).shortname) ) {
		b = wxBrush(highlighted, wxFDIAGONAL_HATCH);
		dc.SetPen(wxPen(highlighted, 1));
	} else {
		b = wxBrush(background);
		dc.SetPen(wxPen(background, 4));
	}
	dc.SetBackground(b);
	dc.SetBrush(b);
	dc.DrawRoundedRectangle(selectedRect, 10.0);

	if ( activeMod == this->tableData->Item(n).shortname ) {
		b = wxBrush(highlighted, wxSOLID);
		dc.SetPen(wxPen(highlighted, 1));
	} else if ( this->isAnAppendMod(this->tableData->Item(n).shortname) ) {
		b = wxBrush(highlighted, wxBDIAGONAL_HATCH);
		dc.SetPen(wxPen(highlighted, 1));
	} else if ( this->isAPrependMod(this->tableData->Item(n).shortname) ) {
		b = wxBrush(highlighted, wxFDIAGONAL_HATCH);
		dc.SetPen(wxPen(highlighted, 1));
	} else {
		b = wxBrush(background, wxSOLID);
		dc.SetPen(wxPen(background, 1));
	}
	dc.SetBackground(b);
	dc.SetBrush(b);
	dc.DrawRoundedRectangle(activeRect, 10.0);
}

wxCoord ModList::OnMeasureItem(size_t WXUNUSED(n)) const {
	return 80;
}

void ModList::OnSelectionChange(wxCommandEvent &event) {
	wxLogDebug(_T("Selection changed to %d (%s)."),
		event.GetInt(),
		this->tableData->Item(event.GetInt()).shortname.c_str());
	this->Refresh();
}

void ModList::OnActivateMod(wxCommandEvent &WXUNUSED(event)) {
	int selected = this->GetSelection();
	wxCHECK_RET(selected != wxNOT_FOUND, _T("Do not have a valid selection."));

	wxString modline;
	const wxString& shortname(this->tableData->Item(selected).shortname);

	if ( !this->GetPrependMods().IsEmpty() ) {
		wxStringTokenizer prependtokens(this->GetPrependMods(), _T(","), wxTOKEN_STRTOK); // no empty tokens
		while ( prependtokens.HasMoreTokens() ) {
			if ( !modline.IsEmpty() ) {
				modline += _T(",");
			}
			modline += prependtokens.GetNextToken().Trim(true).Trim(false);
		}
	}

	wxCHECK_RET( !shortname.IsEmpty(), _T("Mod shortname is empty!"));
	if ( !modline.IsEmpty() ) {
		modline += _T(",");
	}
	if ( selected != 0 ) {
		// put current mods name into the list unless it is (No mod)
		modline += shortname;
	}

	if ( !this->GetAppendMods().IsEmpty() ) {
		wxStringTokenizer appendtokens(this->GetAppendMods(), _T(","), wxTOKEN_STRTOK);
		while ( appendtokens.HasMoreTokens() ) {
			if ( !modline.IsEmpty() ) {
				modline += _T(",");
			}
			modline += appendtokens.GetNextToken().Trim(true).Trim(false);
		}
	}

	wxLogDebug(_T("New modline is %s"), modline.c_str());

	ProMan::GetProfileManager()->ProfileWrite(PRO_CFG_TC_CURRENT_MODLINE, modline);
	ProMan::GetProfileManager()->ProfileWrite(PRO_CFG_TC_CURRENT_MOD, shortname);

	TCManager::GenerateTCSelectedModChanged();
	this->Refresh();
}

void ModList::OnInfoMod(wxCommandEvent &WXUNUSED(event)) {
	int selected = this->GetSelection();
	wxCHECK_RET(selected != wxNOT_FOUND, _T("Do not have a valid selection."));
	new ModInfoDialog(new ModItem(this->tableData->Item(selected)), this);
}

// comparison is case-insensitive, and mod names containing spaces are preserved
bool ModList::isADependency(const wxString &mod, const wxString& modlist) {
	wxCHECK_MSG(!mod.IsEmpty(), false, _T("isADependency() called with empty mod!"));
	wxCHECK_MSG(!modlist.IsEmpty(), false, _T("isADependency() called with empty modlist!"));
	
	wxString normalizedModName(mod);
	normalizedModName.Trim(true).Trim(false).MakeLower();

	wxStringTokenizer tokens(modlist, _T(","), wxTOKEN_STRTOK);
	while ( tokens.HasMoreTokens() ) {
		if ( tokens.GetNextToken().Trim(true).Trim(false).Lower() == normalizedModName ) {
			return true;
		}
	}
	return false;
}

const wxString& ModList::GetPrependMods() const {
	const int selection = this->GetSelection();
	
	wxASSERT_MSG(selection != wxNOT_FOUND,
		_T("GetPrependMods() called with no mod selected!"));
	
	return this->tableData->Item(selection).primarylist;
}

const wxString& ModList::GetAppendMods() const {
	const int selection = this->GetSelection();
	
	wxASSERT_MSG(selection != wxNOT_FOUND,
		_T("GetAppendMods() called with no mod selected!"));
	
	return this->tableData->Item(selection).secondarylist;
}

bool ModList::isAnAppendMod(const wxString &mod) const {
	wxCHECK_MSG(this->GetSelection() != wxNOT_FOUND, false,
		_T("isAnAppendMod() called with no mod selected!"));
	if ( this->GetAppendMods().IsEmpty() ) return false; // no append mods, mod cannot be one
	return ModList::isADependency(mod, this->GetAppendMods());
}

bool ModList::isAPrependMod(const wxString &mod) const {
	wxCHECK_MSG(this->GetSelection() != wxNOT_FOUND, false,
		_T("isAPrependMod() called with no mod selected!"));
	if ( this->GetPrependMods().IsEmpty() ) return false;	// no prepend mods, mod cannot be one
	return ModList::isADependency(mod, this->GetPrependMods());
}

bool ModList::isCurrentSelectionAnAppendMod(const wxString &mod) const {
	int selection = this->GetSelection();
	if ( selection == wxNOT_FOUND
		|| this->tableData->Item(selection).secondarylist.IsEmpty()) {
			return false;
	}
	return ModList::isADependency(mod, this->tableData->Item(selection).secondarylist);
}

bool ModList::isCurrentSelectionAPrependMod(const wxString &mod) const {
	int selection = this->GetSelection();
	if ( selection == wxNOT_FOUND
		|| this->tableData->Item(selection).primarylist.IsEmpty()) {
		return false;
	}
	return ModList::isADependency(mod, this->tableData->Item(selection).primarylist);
}


BEGIN_EVENT_TABLE(ModList, wxVListBox)
EVT_LISTBOX(ID_MODLISTBOX, ModList::OnSelectionChange)
EVT_BUTTON(ID_MODLISTBOX_ACTIVATE_BUTTON, ModList::OnActivateMod)
EVT_BUTTON(ID_MODLISTBOX_INFO_BUTTON, ModList::OnInfoMod)
END_EVENT_TABLE()

///////////////////////////////////////////////////////////////////////////////
// Flagsets
/** \class FlagSetItem
Structure used to store the name, notes, and the flagset itself.  Pointer's that
are not NULL when structure is destroyed will be deleted by the structure. */
/** Constructor. Only makes sure that the members are nulled. Does nothing
else.*/
FlagSetItem::FlagSetItem() {
}

/** Destructor.  Deletes any non NULL pointers that are contained in the
structure. */
FlagSetItem::~FlagSetItem() {
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(FlagSets);

#ifdef MOD_TEXT_LOCALIZATION // mod text localization is not supported for now
///////////////////////////////////////////////////////////////////////////////
// I18nData
/** \class I18nItem
Structure to store the translation of the supported translatable strings
(modname and infotext).  Structure will delete the non-null pointers it
contains at the structures time of destruction. */
/** Constructor.  Only NULL's the member variables. */
I18nItem::I18nItem() {
	this->modname = NULL;
	this->infotext = NULL;
}

/** Destructor. Deletes any non-null member variables. */
I18nItem::~I18nItem() {
	if ( this->modname ) {
		delete this->modname;
	}
	if ( this->infotext ) {
		delete this->infotext;
	}
}
/** The supported langauges. */
const wxString __SupportedLanguages[] = {
	_T("French"),
	_T("Romanian"),
	_T("German"),
	_T("Polish"),
};
/** Languages that are supported by the launcher and thus will search for
translations for in mod.ini's. */
wxSortedArrayString SupportedLanguages = wxArrayString(sizeof(__SupportedLanguages)/sizeof(wxString), __SupportedLanguages);
#endif

//#include <wx/impl.cpp>
//WX_DEFINE_OBJARRAY(I18nData);

///////////////////////////////////////////////////////////////////////////////
// ModGridTable::ModItem
/** \struct ModGridTable::ModItem
Structure that holds all of the information for a single line in the mod table.
*/
/** Constructor.*/
ModItem::ModItem() {
	this->image = NULL;
	warn = false;

	this->flagsets = NULL;
	this->skin = NULL;
#ifdef MOD_TEXT_LOCALIZATION // mod text localization is not supported for now
	this->i18n = NULL;
#endif

	this->infoTextPanel = new InfoText(this);
	this->modImagePanel = new ModImage(this);
	this->modNamePanel = new ModName(this);

}

/** Destructor.  Deletes all memory pointed to by non NULL internal pointers. */
ModItem::~ModItem() {
	if (this->image != NULL) delete this->image;
	if (this->flagsets != NULL) delete this->flagsets;
	if (this->skin != NULL) delete this->skin;
#ifdef MOD_TEXT_LOCALIZATION // mod text localization is not supported for now
	if (this->i18n != NULL) {
		I18nData::iterator i18niter = this->i18n->begin();
		while (i18niter != this->i18n->end()) {
			delete i18niter->second;
			i18niter++;
		}
		delete this->i18n;
	}
#endif
	if (this->infoTextPanel != NULL) delete this->infoTextPanel;
	if (this->modImagePanel != NULL) delete this->modImagePanel;
	if (this->modNamePanel != NULL) delete this->modNamePanel;
}

void ModItem::Draw(wxDC &dc, const wxRect &rect, bool selected, wxSizer* mainSizer, wxSizer* buttons, wxStaticBitmap* warn) {
	wxRect titlerect = rect;
	titlerect.width = 150;

	wxRect imgrect = rect;
	imgrect.width = SkinSystem::ModsListImageWidth;
	imgrect.x = titlerect.width;

	wxRect infotextrect = rect;
	// The 5 is to keep the text away from the image.
	infotextrect.x = titlerect.width + imgrect.width + 5;
	infotextrect.width = rect.width - infotextrect.x;

	wxFont titlefont = SkinSystem::GetSkinSystem()->GetFont();
	titlefont.SetPointSize(titlefont.GetPointSize() + 2);
	titlefont.SetWeight(wxFONTWEIGHT_BOLD);
	dc.SetFont(titlefont);
	this->modNamePanel->Draw(dc, titlerect);
	dc.SetFont(SkinSystem::GetSkinSystem()->GetFont());
	this->modImagePanel->Draw(dc, imgrect);

	if ( selected ) { /* If I am selected do not have info panel draw because 
					  I am going to put the buttons over the info text. */
		buttons->Show(true);
		if ( this->warn) {
			warn->Show(true);
		} else if ( !this->warn) {
			warn->Show(false);
		}
		mainSizer->SetDimension(infotextrect.x, infotextrect.y,
			infotextrect.width, infotextrect.height);
	} else {
		this->infoTextPanel->Draw(dc, infotextrect);
	}
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ModItemArray);

///////////////////////////////////////////
/** \class ModItem::InfoText
Extends wxPanel so that it can draw the info text to the correct size in the list
*/
/** Constructor. Sets up stuff. */
ModItem::InfoText::InfoText(ModItem *myData) {
	this->myData = myData;
}

void ModItem::InfoText::Draw(wxDC &dc, const wxRect &rect) {
	if ( !this->myData->infotext.IsEmpty() ) {
		wxStringTokenizer tokens(this->myData->infotext);
		ArrayOfWords words;
		words.Alloc(tokens.CountTokens());

		FillArrayOfWordsFromTokens(tokens, dc, NULL, words);

		const int maxwidth = rect.width;
		int currentx = rect.x, currenty = rect.y;

		wxSize spaceSize = dc.GetTextExtent(_T(" "));
		int currentwidth  = 0;
		wxString string;
		for( size_t i = 0; i < words.Count(); i++) {
			if ( currentwidth + words[i].size.x + spaceSize.x > maxwidth ) {
				dc.DrawText(string, currentx, currenty);

				string.Empty();
				currentwidth = 0;

				currenty += words[i].size.y;
				if (currenty + words[i].size.y> rect.y + rect.height) {
					break;
				}
			} else {
				if (!(string.IsEmpty())) { // prevent leading space in info text
					string.append(_T(" "));	
				}
			}
			string.append(words[i].word);
			currentwidth += words[i].size.x + spaceSize.x;
		}
		if ( !string.IsEmpty()) {
			dc.DrawText(string, currentx, currenty);
		}
	}
}

///////////////////////////////////////////
/** \class ModItem::ModName
Extends wxPanel so that it can draw the mod's name to the correct size in the list
or the mod's short name.
*/
/** Constructor. Sets up stuff. */
ModItem::ModName::ModName(ModItem *myData) {
	this->myData = myData;
}

void ModItem::ModName::Draw(wxDC &dc, const wxRect &rect) {
	wxString name;

	if ( !this->myData->name.IsEmpty() ) {
		name = this->myData->name;
	} else {
		name = this->myData->shortname;
	}

	wxCoord width, height;
	wxFont testFont(dc.GetFont());	/* font to use to compensate for
									GetTextExtent's inabliity to handle
									bold font.*/
	testFont.SetPointSize(testFont.GetPointSize() + 2);
	dc.GetMultiLineTextExtent(name, &width, &height, NULL, &testFont);

	if ( width > rect.width ) {
		// too wide need to wrap if possible.
		ArrayOfWords titleWords;
		wxStringTokenizer tokens(name);
		ArrayOfWords words;
		words.Alloc(tokens.CountTokens());

		FillArrayOfWordsFromTokens(tokens, dc, &testFont, words);

		const int maxwidth = rect.width;
		int currentx = rect.x, currenty = rect.y;

		wxCoord spaceX, spaceY;
		dc.GetTextExtent(_T(" "), &spaceX, &spaceY, NULL, NULL, &testFont);
		int currentwidth  = 0;
		wxString string;
		for( size_t i = 0; i < words.Count(); i++) {
			if ( currentwidth + words[i].size.x + spaceX > maxwidth ) {
				wxCoord tempX, tempY;
				dc.GetTextExtent(string, &tempX, &tempY, NULL, NULL, &testFont);

				Words* temp = new Words();
				temp->size.SetWidth(tempX);
				temp->size.SetHeight(tempY);
				temp->word = string;
				titleWords.Add(temp);

				string.Empty();
				currentwidth = 0;

				currenty += words[i].size.y;
				if (currenty + words[i].size.y > rect.y + rect.height) {
					break;
				}
			} else if ( string.size() > 0 ) {
				string.append(_T(" "));
			}
			string.append(words[i].word);
			currentwidth += words[i].size.x + spaceX;
		}
		wxCoord tempX, tempY;
		dc.GetTextExtent(string, &tempX, &tempY, NULL, NULL, &testFont);

		Words* temp = new Words();
		temp->size.SetWidth(tempX);
		temp->size.SetHeight(tempY);
		temp->word = string;
		titleWords.Add(temp);

		// draw the words properly centered

		// Find the hight of all of the lines of text
		int totalHeight = 0;
		for( size_t i = 0; i < titleWords.Count(); i++ ) {
			totalHeight += titleWords[i].size.y;
		}

		int currentHeightOffset = 0;
		for( size_t i = 0; i < titleWords.Count(); i++ ) {
			dc.DrawText(titleWords[i].word,
				rect.x + rect.width/2 - titleWords[i].size.x/2,
				rect.y + rect.height/2 - titleWords[i].size.y/2 + currentHeightOffset - totalHeight/2);
			currentHeightOffset += titleWords[i].size.y;
		}

	} else {
		dc.DrawText(name,
			rect.x + rect.width/2 - width/2,
			rect.y + rect.height/2 - height/2);
	}
}

///////////////////////////////////////////
/** \class ModItem::ModImage
Draws the Mod's image on the list or degrades smoothly to drawing the text
"NO IMAGE".
*/
/** Constructor. Sets up stuff. */
ModItem::ModImage::ModImage(ModItem *myData) {
	this->myData = myData;
}

void ModItem::ModImage::Draw(wxDC &dc, const wxRect &rect) {
	if ( this->myData->image != NULL ) {
		dc.DrawBitmap(SkinSystem::MakeModsListImage(*this->myData->image), rect.x, rect.y);
	} else {
		dc.DrawRectangle(rect);
		wxPen pen(dc.GetPen());
		pen.SetWidth(1);
		dc.SetPen(pen);
		wxString noimg = _("NO IMAGE");
		wxSize size = dc.GetTextExtent(noimg);
		dc.DrawText(noimg, rect.x + rect.width/2 - size.x/2, rect.y + rect.height/2 - size.y/2);
	}
}

ModInfoDialog::ModInfoDialog(ModItem* item, wxWindow* parent) {
	wxASSERT(item != NULL);
	this->item = item;

	wxASSERT(!item->name.IsEmpty() || !item->shortname.IsEmpty());
	wxString modName = 
		wxString::Format(_T("%s"),
			item->name.IsEmpty() ? item->shortname.c_str(): item->name.c_str());
	wxDialog::Create(parent, wxID_ANY, modName, wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED | wxBORDER_DOUBLE );
	this->SetBackgroundColour(wxColour(_T("WHITE")));

	wxStaticText* titleBox = 
		new wxStaticText(this, wxID_ANY, modName, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
	wxFont titleFont = titleBox->GetFont();
	titleFont.SetWeight(wxFONTWEIGHT_BOLD);
	titleFont.SetPointSize(14);
	titleBox->SetFont(titleFont);

	wxString tcPath;
	ProMan::GetProfileManager()->ProfileRead(PRO_CFG_TC_ROOT_FOLDER, &tcPath, wxEmptyString);
	wxString modFolderString =
		wxString::Format(_T("%s%s"),
			tcPath.c_str(),
			(item->shortname == NO_MOD) ? wxEmptyString :
				(wxString(wxFileName::GetPathSeparator()) + item->shortname).c_str());
	wxStaticText* modFolderBox = 
		new wxStaticText(this, wxID_ANY, modFolderString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);

	wxPanel* modImage = new ModInfoDialog::ImageDrawer(this);
	modImage->SetMaxSize(wxSize(SkinSystem::InfoWindowImageWidth, SkinSystem::InfoWindowImageHeight));

	wxHtmlWindow* info = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
	info->SetMinSize(wxSize(SkinSystem::InfoWindowImageWidth, 250));
	if ( item->infotext.IsEmpty() ) {
		info->SetPage(DEFAULT_MOD_LAUNCHER_INFO_TEXT);
	} else {
		wxString infoText(item->infotext);
		infoText.Replace(_T("\n"), _T("<br />"));
		info->SetPage(infoText);
	}

	wxHtmlWindow* links = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN | wxHW_SCROLLBAR_NEVER );
	links->SetSize(SkinSystem::InfoWindowImageWidth, 40);
	links->SetPage(wxString::Format(_T("<center>%s%s%s%s</center>"),
		(!item->website.IsEmpty()) ? 
			wxString::Format(_T("<a href='%s'>%s</a> :: "), item->website.c_str(), _("Website")).c_str():wxEmptyString,
		wxString::Format(_T("<a href='%s'>%s</a>"), (!item->forum.IsEmpty()) ?
			item->forum.c_str():_("http://www.hard-light.net/forums/index.php?board=124.0"), _("Forum")).c_str(),
		(!item->bugs.IsEmpty()) ?
			wxString::Format(_T(" :: <a href='%s'>%s</a>"), item->bugs.c_str(), _("Bugs")).c_str() : wxEmptyString,
		(!item->support.IsEmpty()) ?
			wxString::Format(_T(" :: <a href='%s'>%s</a>"), item->support.c_str(), _("Support")).c_str() : wxEmptyString
		));
	links->Connect(wxEVT_COMMAND_HTML_LINK_CLICKED, wxHtmlLinkEventHandler(ModInfoDialog::OnLinkClicked));

	wxStaticBitmap* warning = NULL;
	wxHtmlWindow* notesText = NULL;

	if ( !item->notes.IsEmpty() ) {
		if ( item->warn ) {
			warning = new wxStaticBitmap(this, wxID_ANY, SkinSystem::GetSkinSystem()->GetBigWarningIcon());
		}
		notesText = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
		notesText->SetPage(item->notes);
		notesText->SetMinSize(wxSize(200, 64));
	}

	wxButton* close = new wxButton(this, wxID_ANY, _("Close"));
	this->SetEscapeId(close->GetId());

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	sizer->Add(titleBox, wxSizerFlags().Centre());
	sizer->Add(modFolderBox, wxSizerFlags().Centre());
	sizer->Add(modImage, wxSizerFlags().Centre());
	sizer->Add(info, wxSizerFlags().Centre());
	sizer->Add(links, wxSizerFlags().Centre());
	if ( notesText != NULL ) {
		if ( warning != NULL ) {
			wxSizer* linkSizer = new wxBoxSizer(wxHORIZONTAL);
			linkSizer->Add(warning);
			linkSizer->Add(notesText);
			sizer->Add(linkSizer, wxSizerFlags().Expand().Centre());
		} else {
			sizer->Add(notesText, wxSizerFlags().Expand().Centre());
		}
	}
	sizer->Add(close, wxSizerFlags().Centre());
	this->SetSizerAndFit(sizer);
	this->Layout();
	this->CentreOnParent();
	this->ShowModal();
}

void ModInfoDialog::OnLinkClicked(wxHtmlLinkEvent &event) {
	wxHtmlLinkInfo info = event.GetLinkInfo();
	wxString rest;
	wxLaunchDefaultBrowser(info.GetHref());
}

ModInfoDialog::ImageDrawer::ImageDrawer(ModInfoDialog* parent):
wxPanel(parent) {
	this->parent = parent;

	if (parent->item->image == NULL) {
		this->SetSize(SkinSystem::InfoWindowImageWidth, SkinSystem::InfoWindowImageHeight);
	} else {
		this->SetSize(
			parent->item->image->GetWidth(),
			parent->item->image->GetHeight());
	}
	this->GetEventHandler()->Connect(wxEVT_PAINT, wxPaintEventHandler(ModInfoDialog::ImageDrawer::OnDraw));
}

void ModInfoDialog::ImageDrawer::OnDraw(wxPaintEvent &WXUNUSED(event)) {
	wxPaintDC dc(this);
	if ( parent->item->image != NULL ) {
		dc.DrawBitmap(*(parent->item->image), 0, 0);
	} else {
		wxCoord textWidth, textHeight;
		dc.GetTextExtent(_("NO IMAGE"), &textWidth, &textHeight);

		wxCoord drawLocationX = SkinSystem::InfoWindowImageWidth/2 - textWidth/2;
		wxCoord drawLocationY = SkinSystem::InfoWindowImageHeight/2 - textHeight/2;
		
		dc.DrawText(_("NO IMAGE"), drawLocationX, drawLocationY);
	}
}

BEGIN_EVENT_TABLE(ModInfoDialog::ImageDrawer, wxPanel)
END_EVENT_TABLE()
