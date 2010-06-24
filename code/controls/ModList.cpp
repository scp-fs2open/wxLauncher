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
#include "controls/ModList.h"
#include "apis/ProfileManager.h"
#include "apis/TCManager.h"

#include "global/MemoryDebugging.h"

class ModInfoDialog: wxDialog {
public:
	ModInfoDialog(SkinSystem* skin, ModItem* item, wxWindow* parent);
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

	SkinSystem* skin;
	ModItem* item;
};


ConfigPair::ConfigPair(wxString &shortname, wxFileConfig *config)  {
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

ModList::ModList(wxWindow *parent, wxSize& size, SkinSystem *skin, wxString tcPath) {
	this->Create(parent, ID_MODLISTBOX, wxDefaultPosition, size, 
		wxLB_SINGLE | wxLB_ALWAYS_SB | wxBORDER);
	this->SetMargins(10, 10);

	this->skinSystem = skin;

	this->semicolon[0] = ';';
	this->semicolon[1] = '\0';

	this->stringNoMod = _("(No MOD)");

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

	wxLogDebug(_T("Inserting '(No MOD)'"));
	wxFileName tcmodini(tcPath, _T("mod.ini"));
	if ( tcmodini.IsOk() && tcmodini.FileExists() ) {
		wxFFileInputStream tcmodinistream(tcmodini.GetFullPath());
		this->configFiles->Add(new ConfigPair(stringNoMod, new wxFileConfig(tcmodinistream)));
		wxLogDebug(_T(" Found a mod.ini in the root TC folder. (%s)"), tcmodini.GetFullPath().c_str());

		// make sure that a mod.ini in the root TC folder is not apart of this set
		// because it will be addressed shortly and specificly
		int pos = foundInis.Index(tcmodini.GetFullPath());
		if ( pos != wxNOT_FOUND ) {
			foundInis.RemoveAt(pos);
		}

	} else {
		this->configFiles->Add(new ConfigPair(stringNoMod, new wxFileConfig()));
		wxLogDebug(_T(" Using defaults for TC."));
	}

	wxLogDebug(_T("Starting to parse mod.ini's..."));
	for (size_t i = 0; i < foundInis.Count(); i++) {
		wxLogDebug(_T("  Opening %s"), foundInis.Item(i).c_str());
		wxFFileInputStream stream(foundInis.Item(i));
		if ( stream.IsOk() ) {
			wxLogDebug(_T("   Opened ok"));
		} else {
			wxLogDebug(_T("   Open failed!"));
			continue;
		}

		wxFileConfig* config;

		// check if the stream is a UTF-8 File
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
		size_t size = buf->GetBufferSize();

		char* characterBuffer = new char[size+1];
		characterBuffer[size] = '\0';

		buf->Seek(0, wxFromStart);
		size_t read = buf->Read(reinterpret_cast<void*>(characterBuffer), size);
		if ( read != size ) {
			wxLogError(_T("read (%d) not equal to size (%d)"), read, size);
			delete[] characterBuffer;
			continue;
		}

		wxMBConv* conv;
		if ( isUTF8 ) {
			conv = new wxMBConvUTF8();
		} else {
			conv = new wxCSConv(wxFONTENCODING_ISO8859_1);
		}
		wxString stringBuffer(characterBuffer, *conv);
		delete conv;
		// A hack to insert a backslash into the stream so that when
		// wxFileConfig excapes the backslashes the one that is in 
		// the file is returned
		stringBuffer.Replace(_T("\\"), _T("\\\\"));
		wxStringInputStream finalBuffer(stringBuffer);

		config = new wxFileConfig(finalBuffer);
		delete[] characterBuffer;

		wxLogDebug(_T("   Mod fancy name is: %s"), config->Read(_T("/launcher/modname"), _T("Not specified")).c_str());

		// get the mod.ini's base directory
		// <something>/modfolder/mod.ini
		// <something>\modfolder\mod.ini
		wxArrayString tokens = wxStringTokenize(foundInis.Item(i), _T("\\/"),
			wxTOKEN_STRTOK); /* breakup on folder markers and never return an
							 empty string. */
		wxArrayString tcTokens = wxStringTokenize(tcPath, _T("\\/"), wxTOKEN_STRTOK);

		size_t j = tcTokens.GetCount();
		wxString shortname;
		while ( j < (tokens.GetCount() - 1) ) { // -1 to skip mod.ini
			if ( !shortname.IsEmpty() ) {
				shortname += _T("/");
			}
			shortname += tokens[j];
			j++;
		}

		wxLogDebug(_T("   Mod short name is: %s"), shortname.c_str());

		this->configFiles->Add(new ConfigPair(shortname, config));
	}

	// create internal repesentation of the mod.ini's
	wxLogDebug(_T("Transforming mod.ini's"));
	
	for(size_t i = 0; i < this->configFiles->size(); i++) {
		wxString shortname = this->configFiles->Item(i).shortname;
		wxFileConfig* config = this->configFiles->Item(i).config;
		ModItem* item = new ModItem(this->skinSystem);
		wxLogDebug(_T(" %s"), shortname.c_str());

		item->shortname = new wxString(shortname);

		readIniFileString(config, _T("/launcher/modname"), &(item->name));

		wxString *smallimagepath = NULL;
		readIniFileString(config, _T("/launcher/image255x112"), &smallimagepath);
		if ( smallimagepath != NULL ) {
			item->image = SkinSystem::VerifySmallImage(tcPath, shortname,
				*smallimagepath);
			delete smallimagepath;
		}
		
		readIniFileString(config, _T("/launcher/infotext"), &(item->infotext));

		readIniFileString(config, _T("/launcher/author"), &(item->author));

		readIniFileString(config, _T("/launcher/notes"), &(item->notes));

		config->Read(_T("/launcher/warn"), &(item->warn), false);

		readIniFileString(config, _T("/launcher/website"), &(item->website));
		readIniFileString(config, _T("/launcher/forum"), &(item->forum));
		readIniFileString(config, _T("/launcher/bugs"), &(item->bugs));
		readIniFileString(config, _T("/launcher/support"), &(item->support));

		readIniFileString(config, _T("/extremeforce/forcedflagson"), &(item->forcedon));
		readIniFileString(config, _T("/extremeforce/forcedflagsoff"), &(item->forcedoff));

		readIniFileString(config, _T("/multimod/primarylist"), &(item->primarylist));
		if ( config->Exists(_T("/multimod/secondrylist")) ) {
			wxLogInfo(_T("  DEPCRECIATION WARNING: Mod '%s' uses depreciated mod.ini parameter 'secondrylist'"),
				shortname.c_str());
		}
		readIniFileString(config, _T("/multimod/secondrylist"), &(item->secondarylist));
		readIniFileString(config, _T("/multimod/secondarylist"), &(item->secondarylist));

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
			wxLogDebug(_T("  Does Not Contain An idealflagset Section."));
		}

		// skin
		if ( config->Exists(_T("/skin")) ) {
			item->skin = new Skin();

			readIniFileString(config, _T("/skin/wtitle"), &(item->skin->windowTitle));
			
			wxString *windowIconFile = NULL;
			readIniFileString(config, _T("/skin/wicon"), &windowIconFile);
			if ( windowIconFile != NULL ) {
				item->skin->welcomeIcon = SkinSystem::VerifyWindowIcon(tcPath,
					shortname, *windowIconFile);
				delete windowIconFile;
			}

			readIniFileString(config, _T("/skin/welcometxt"), &(item->skin->welcomePageText));

			// read in file names of the icons
			wxString *welcomeIconFile = NULL,
				*modsIconFile = NULL,
				*basicIconFile = NULL,
				*advancedIconFile = NULL,
				*installIconFile = NULL;
			readIniFileString(config, _T("/skin/iconwelcome"), &welcomeIconFile);
			readIniFileString(config, _T("/skin/iconmods"), &modsIconFile);
			readIniFileString(config, _T("/skin/iconbasic"), &basicIconFile);
			readIniFileString(config, _T("/skin/iconadvanced"), &advancedIconFile);
			readIniFileString(config, _T("/skin/iconinstall"), &installIconFile);

			// Verify the icon will fit
			

			if ( welcomeIconFile != NULL ) {
				item->skin->welcomeIcon = SkinSystem::VerifyTabIcon(tcPath,
					shortname, *welcomeIconFile);
				delete welcomeIconFile;
			}
			if ( modsIconFile != NULL )	{
				item->skin->modsIcon = SkinSystem::VerifyTabIcon(tcPath,
					shortname, *modsIconFile);
				delete modsIconFile;
			}
			if ( basicIconFile != NULL ) {
				item->skin->basicIcon = SkinSystem::VerifyTabIcon(tcPath,
					shortname, *basicIconFile);
				delete basicIconFile;
			}
			if ( advancedIconFile != NULL ) {
				item->skin->advancedIcon = SkinSystem::VerifyTabIcon(tcPath,
					shortname, *advancedIconFile);
				delete advancedIconFile;
			}
			if ( installIconFile != NULL ) {
				item->skin->installIcon = SkinSystem::VerifyTabIcon(tcPath,
					shortname, *installIconFile);
				delete installIconFile;
			}

			wxString *idealIconFile = NULL;
			readIniFileString(config, _T("/skin/idealicon"), &idealIconFile);
			if ( idealIconFile != NULL ) {
				item->skin->idealIcon = SkinSystem::VerifyIdealIcon(tcPath,
					shortname, *idealIconFile);
				delete idealIconFile;
			}

			wxString *fontName = NULL;
			readIniFileString(config, _T("/skin/font"), &fontName);
			int fontSize = 0;
			config->Read(_T("/skin/fontsize"), &fontSize);
			if ( fontName != NULL ) {
				item->skin->baseFont = SkinSystem::VerifyFontChoice(tcPath,
					shortname, *fontName, fontSize);
				delete fontName;
			}

		} else {
			wxLogDebug(_T("  Does Not Contain An skin Section."));
		}

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

		this->tableData->Add(item);
	}

	this->SetItemCount(this->tableData->Count());

	// set currently select mod as selected or
	// set (No MOD) if none or previous does not exist
	wxString currentMod;
	ProMan::GetProfileManager()->Get()
		->Read(PRO_CFG_TC_CURRENT_MOD, &currentMod, _("(No MOD)"));

	{
		size_t i;
		for ( i = 0; i < this->tableData->size(); i++) {
			if ( *(this->tableData->Item(i).shortname) == currentMod ) {
				break;
			}
		}

		if ( i < this->tableData->size() ) {
			// found it
			this->SetSelection(i);
		} else {
			this->SetSelection(0);
		}
	}


	this->infoButton = 
		new wxButton(this, ID_MODLISTBOX_INFO_BUTTON, _("Info"));
	this->activateButton = 
		new wxButton(this, ID_MODLISTBOX_ACTIVATE_BUTTON, _("Activate"));
	this->warnBitmap =
		new wxStaticBitmap(this, wxID_ANY, this->skinSystem->GetWarningIcon());
	this->warnBitmap->SetToolTip(_("This mod requires your attention before playing it, please click Info for more details"));

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
/** Function takes the keyvalue string to search for, and returns via location
the pointer to value. If the keyvalue is not found *location will remain NULL.*/
void ModList::readIniFileString(wxFileConfig* config,
									 wxString keyvalue, wxString ** location) {
	if ( config->Exists(keyvalue) ) {
			*location = new wxString();
			config->Read(keyvalue, *location);
			if ( (*location)->EndsWith(this->semicolon) ) {
				(*location)->RemoveLast();
			}
	}
	wxLogDebug(_T("  %s:'%s'"), keyvalue.c_str(),
		((*location) == NULL) ? _T("Not Specified") : excapeSpecials(**location).c_str());

	if ( (*location) != NULL && (*location)->empty() ) {
		wxLogDebug(_T("  Nulled %s"), keyvalue.c_str());
		delete *location;
		*location = NULL;
	}
}

/** reexcape the newlines in the mod.ini values. */
wxString ModList::excapeSpecials(wxString toexcape) {
	wxString::iterator iter;
	for ( iter = toexcape.begin(); iter != toexcape.end(); iter++ ) {
		if ( *iter == wxChar('\n') ) {
			wxString::iterator end = iter;
			end++;
			toexcape.replace(iter, end, _T("\\n"));

			// have to start from the begining because we wrote to the string
			// in invalidated the iterator.
			iter = toexcape.begin();
		}
	}
	return toexcape;
}


/** */
void ModList::readFlagSet(wxFileConfig* config,
							   wxString keyprefix, FlagSetItem *set) {
	readIniFileString(config,
    wxString::Format(_T("%s/name"), keyprefix.c_str()),
    &(set->name));
	readIniFileString(config, 
    wxString::Format(_T("%s/flagset"), keyprefix.c_str()), 
    &(set->flagset));
	readIniFileString(config, 
    wxString::Format(_T("%s/notes"), keyprefix.c_str()), 
    &(set->notes));
}

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
	ProMan::GetProfileManager()->Get()
		->Read(PRO_CFG_TC_CURRENT_MOD, &activeMod, this->stringNoMod);
	wxBrush b;
	wxRect selectedRect(rect.x+2, rect.y+2, rect.width-4, rect.height-4);
	wxRect activeRect(selectedRect.x+3, selectedRect.y+3, selectedRect.width-7, selectedRect.height-7);

	if ( this->IsSelected(n) ) {
		b = wxBrush(highlighted, wxTRANSPARENT);
		dc.SetPen(wxPen(highlighted, 4));
	} else {
		b = wxBrush(background);
		dc.SetPen(wxPen(background, 4));
	}
	dc.SetBackground(b);
	dc.SetBrush(b);
	dc.DrawRoundedRectangle(selectedRect, 10.0);

	if ( activeMod == *(this->tableData->Item(n).shortname) ) {
		b = wxBrush(highlighted, wxSOLID);
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
		this->tableData->Item(event.GetInt()).shortname->c_str());
}

void ModList::OnActivateMod(wxCommandEvent &WXUNUSED(event)) {
	int selected = this->GetSelection();
	wxCHECK_RET(selected != wxNOT_FOUND, _T("Do not have a valid selection."));

	wxString modline;
	wxString* shortname = this->tableData->Item(selected).shortname;
	wxString* prependmods = this->tableData->Item(selected).primarylist;
	wxString* appendmods = this->tableData->Item(selected).secondarylist;

	if ( prependmods != NULL ) {
		wxStringTokenizer prependtokens(*prependmods, _T(", "), wxTOKEN_STRTOK); // no empty tokens
		while ( prependtokens.HasMoreTokens() ) {
			if ( !modline.IsEmpty() ) {
				modline += _T(",");
			}
			modline += prependtokens.GetNextToken();
		}
	}

	wxCHECK_RET( shortname != NULL, _T("Mod shortname is NULL!"));
	if ( !modline.IsEmpty() ) {
		modline += _T(",");
	}
	if ( selected != 0 ) {
		// put current mods name into the list unless it is (No MOD)
		modline += *shortname;
	}

	if ( appendmods != NULL ) {
		wxStringTokenizer appendtokens(*appendmods, _T(", "), wxTOKEN_STRTOK);
		while ( appendtokens.HasMoreTokens() ) {
			if ( !modline.IsEmpty() ) {
				modline += _T(",");
			}
			modline += appendtokens.GetNextToken();
		}
	}

	wxLogDebug(_T("New modline is %s"), modline.c_str());

	ProMan::GetProfileManager()->Get()
		->Write(PRO_CFG_TC_CURRENT_MODLINE, modline);
	ProMan::GetProfileManager()->Get()
		->Write(PRO_CFG_TC_CURRENT_MOD, *shortname);

	TCManager::GenerateTCSelectedModChanged();
	this->Refresh();
}

void ModList::OnInfoMod(wxCommandEvent &WXUNUSED(event)) {
	int selected = this->GetSelection();
	wxCHECK_RET(selected != wxNOT_FOUND, _T("Do not have a valid selection."));
	new ModInfoDialog(this->skinSystem, new ModItem(this->tableData->Item(selected)), this);
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
	this->name = NULL;
	this->flagset = NULL;
	this->notes = NULL;
}

/** Destructor.  Deletes any non NULL pointers that are contained in the
structure. */
FlagSetItem::~FlagSetItem() {
	if ( this->name != NULL ) {
		delete this->name;
	}
	if ( this->flagset != NULL ) {
		delete this->flagset;
	}
	if ( this->notes != NULL ) {
		delete this->notes;
	}
}

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(FlagSets);

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

//#include <wx/impl.cpp>
//WX_DEFINE_OBJARRAY(I18nData);

///////////////////////////////////////////////////////////////////////////////
// ModGridTable::ModItem
/** \struct ModGridTable::ModItem
Structure that holds all of the information for a single line in the mod table.
*/
/** Constructor.*/
ModItem::ModItem(SkinSystem* skin) {
	this->skinSystem = skin;

	this->name = NULL;
	this->shortname = NULL;
	this->image = NULL;
	this->infotext = NULL;
	this->author = NULL;
	this->notes = NULL;
	warn = false;
	this->website = NULL;
	this->forum = NULL;
	this->bugs = NULL;
	this->support = NULL;

	this->forcedon = NULL;
	this->forcedoff = NULL;

	this->primarylist = NULL;
	this->secondarylist = NULL;

	this->flagsets = NULL;
	this->skin = NULL;
	this->i18n = NULL;

	this->infoTextPanel = new InfoText(this);
	this->modImagePanel = new ModImage(this);
	this->modNamePanel = new ModName(this);

}

/** Destructor.  Deletes all memory pointed to by non NULL internal pointers. */
ModItem::~ModItem() {
	if (this->name != NULL) delete this->name;
	if (this->shortname != NULL) delete this->shortname;
	if (this->image != NULL) delete this->image;
	if (this->infotext != NULL) delete this->infotext;
	if (this->author != NULL) delete this->author;
	if (this->notes != NULL) delete this->notes;
	if (this->website != NULL) delete this->website;
	if (this->forum != NULL) delete this->forum;
	if (this->bugs != NULL) delete this->bugs;
	if (this->support != NULL) delete this->support;
	if (this->forcedon != NULL) delete this->forcedon;
	if (this->forcedoff != NULL) delete this->forcedoff;
	if (this->primarylist != NULL) delete this->primarylist;
	if (this->secondarylist != NULL) delete this->secondarylist;
	if (this->flagsets != NULL) delete this->flagsets;
	if (this->skin != NULL) delete this->skin;
	if (this->i18n != NULL) {
		I18nData::iterator i18niter = this->i18n->begin();
		while (i18niter != this->i18n->end()) {
			delete i18niter->second;
			i18niter++;
		}
		delete this->i18n;
	}
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
	// The 2 is to keep the text from touching the image.
	infotextrect.x = titlerect.width + imgrect.width + 2;
	infotextrect.width = rect.width - infotextrect.x;

	wxFont titlefont = this->skinSystem->GetFont();
	titlefont.SetWeight(wxFONTWEIGHT_BOLD);
	dc.SetFont(titlefont);
	this->modNamePanel->Draw(dc, titlerect);
	dc.SetFont(this->skinSystem->GetFont());
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

#include <wx/arrimpl.cpp>
WX_DEFINE_OBJARRAY(ArrayOfWords);

///////////////////////////////////////////
/** \class ModItem::InfoText
Extends wxPanel so that it can draw the info text to the correct size in the list
*/
/** Constructor. Sets up stuff. */
ModItem::InfoText::InfoText(ModItem *myData) {
	this->myData = myData;
}

void ModItem::InfoText::Draw(wxDC &dc, const wxRect &rect) {
	if ( this->myData->infotext != NULL ) {
		wxStringTokenizer tokens(*(this->myData->infotext));
		ArrayOfWords words;
		words.Alloc(tokens.CountTokens());

		do {
			wxString tok = tokens.GetNextToken();
			int x, y;
			dc.GetTextExtent(tok, &x, &y);

			Words* temp = new Words();
			temp->size = dc.GetTextExtent(tok);
			temp->word = tok;

			words.Add(temp);
		} while ( tokens.HasMoreTokens() );

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
				string.append(_T(" "));
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

	if ( this->myData->name != NULL ) {
		name = *this->myData->name;
	} else {
		name = *this->myData->shortname;
	}

	wxSize line = dc.GetTextExtent(name);

	if ( line.x > rect.width ) {
		// too wide need to wrap if possible.
		wxLogDebug(_T("Name is to long"));
	} else {
		dc.DrawText(name,
			rect.x + rect.width/2 - line.x/2,
			rect.y + rect.height/2 - line.y/2);
	}
}

///////////////////////////////////////////
/** \class ModItem::ModImage
Draws the Mod's image on the list or degrades smoothly.
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

ModInfoDialog::ModInfoDialog(SkinSystem* skin, ModItem* item, wxWindow* parent) {
	wxASSERT(skin != NULL);
	this->skin = skin;

	wxASSERT(item != NULL);
	this->item = item;

	wxASSERT(item->name != NULL || item->shortname != NULL);
	wxString modName = 
		wxString::Format(_T("%s"),
			(item->name == NULL)? item->shortname->c_str(): item->name->c_str());
	wxDialog::Create(parent, wxID_ANY, modName, wxDefaultPosition, wxDefaultSize, wxBORDER_RAISED | wxBORDER_DOUBLE );
	this->SetBackgroundColour(wxColour(_T("WHITE")));

	wxStaticText* titleBox = 
		new wxStaticText(this, wxID_ANY, modName, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);
	wxFont titleFont = titleBox->GetFont();
	titleFont.SetWeight(wxFONTWEIGHT_BOLD);
	titleFont.SetPointSize(14);
	titleBox->SetFont(titleFont);

	wxString tcPath;
	ProMan::GetProfileManager()->Get()->Read(PRO_CFG_TC_ROOT_FOLDER, &tcPath, wxEmptyString);
	wxString modFolderString = 
		wxString::Format(_T("%s%c%s"), tcPath.c_str(), wxFileName::GetPathSeparator(), item->shortname->c_str());
	wxStaticText* modFolderBox = 
		new wxStaticText(this, wxID_ANY, modFolderString, wxDefaultPosition, wxDefaultSize, wxALIGN_CENTRE);

	wxPanel* modImage = new ModInfoDialog::ImageDrawer(this);
	modImage->SetMaxSize(wxSize(SkinSystem::InfoWindowImageWidth, SkinSystem::InfoWindowImageHeight));

	wxHtmlWindow* info = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
	info->SetMinSize(wxSize(SkinSystem::InfoWindowImageWidth, 250));
	if ( item->infotext == NULL ) {
		info->SetPage(_T("<p>No information available.</p>"));
	} else {
		wxString infoText(*(item->infotext));
		infoText.Replace(_T("\n"), _T("<br />"));
		info->SetPage(infoText);
	}

	wxHtmlWindow* links = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN | wxHW_SCROLLBAR_NEVER );
	links->SetSize(SkinSystem::InfoWindowImageWidth, 40);
	links->SetPage(wxString::Format(_T("<center>%s%s%s%s</center>"),
		(item->website != NULL) ? 
			wxString::Format(_T("<a href='%s'>%s</a> :: "), item->website->c_str(), _("Website")).c_str():wxEmptyString,
		wxString::Format(_T("<a href='%s'>%s</a>"), (item->forum != NULL) ?
			item->forum->c_str():_("http://www.hard-light.net/forums/index.php?board=124.0"), _("Forum")).c_str(),
		(item->bugs != NULL) ?
			wxString::Format(_T(" :: <a href='%s'>%s</a>"), item->bugs->c_str(), _("Bugs")).c_str() : wxEmptyString,
		(item->support != NULL) ?
			wxString::Format(_T(" :: <a href='%s'>%s</a>"), item->support->c_str(), _("Support")).c_str() : wxEmptyString
		));
	links->Connect(wxEVT_COMMAND_HTML_LINK_CLICKED, wxHtmlLinkEventHandler(ModInfoDialog::OnLinkClicked));

	wxStaticBitmap* warning = NULL;
	wxHtmlWindow* notesText = NULL;

	if ( item->notes != NULL ) {
		if ( item->warn ) {
			warning = new wxStaticBitmap(this, wxID_ANY, skin->GetBigWarningIcon());
		}
		notesText = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SUNKEN);
		notesText->SetPage(*(item->notes));
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
