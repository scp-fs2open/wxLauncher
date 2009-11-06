#include <wx/wx.h>
#include <wx/vlbox.h>
#include <wx/fileconf.h>
#include <wx/wfstream.h>
#include <wx/tokenzr.h>
#include <wx/arrstr.h>
#include <wx/filename.h>
#include <wx/dir.h>

#include "Skin.h"
#include "wxIDs.h"
#include "ModList.h"

#include "wxLauncherSetup.h"

ModList::ModList(wxWindow *parent, wxSize& size, SkinSystem *skin) {
	this->Create(parent, ID_MODLISTBOX, wxDefaultPosition, size, 
		wxLB_SINGLE | wxLB_ALWAYS_SB | wxBORDER);
	this->SetMargins(0, 0);

	this->skinSystem = skin;

	this->semicolon = new wxChar(';');
	this->semicolon[1] = NULL;

	this->tableData = new ModItemArray();
	// scan for mods in the current TCs directory
	wxArrayString foundInis;
	size_t numberfound = wxDir::GetAllFiles(_("."), &foundInis, _("mod.ini"));
	if ( foundInis.Count() > 0 ) {
		wxLogDebug(_T("I found %d .ini files:"), foundInis.Count());
	} else {
		wxLogDebug(_T("I did not find any .ini files."));
	}

	// parse mod.ini's in all of the directories that contain one
	this->configFiles = new ConfigHash();

	wxLogDebug(_T("Inserting '(No MOD)'"));
	wxFileName tcmodini(_T("mod.ini"));
	tcmodini.Normalize(wxPATH_NORM_ALL, _T("."));
	if ( tcmodini.IsOk() && tcmodini.FileExists() ) {
		wxFFileInputStream tcmodinistream(tcmodini.GetFullPath());
		(*(this->configFiles))[_T("(No Mod)")] = new wxFileConfig(tcmodinistream);
		wxLogDebug(_T(" Found a mod.ini in the root TC folder."));
	} else {
		(*(this->configFiles))[_T("(No Mod)")] = new wxFileConfig();
		wxLogDebug(_T(" Using defaults for TC."));
	}

	wxLogDebug(_T("Starting to opening mod.ini's..."));
	for (size_t i = 0; i < foundInis.Count(); i++) {
		wxLogDebug(_T("  Opening %s"), foundInis.Item(i));
		wxFFileInputStream stream(foundInis.Item(i));
		if ( stream.IsOk() ) {
			wxLogDebug(_T("   Opened ok"));
		} else {
			wxLogDebug(_T("   Open failed!"));
			continue;
		}
		wxFileConfig* config = new wxFileConfig(stream);

		wxLogDebug(_T("   Mod fancy name is: %s"), config->Read(_T("/launcher/modname"), _T("Not specified")));

		// get the mod.ini's base directory
		// <something>/modfolder/mod.ini
		// <something>\modfolder\mod.ini
		wxArrayString tokens = wxStringTokenize(foundInis.Item(i), _T("\\/"),
			wxTOKEN_STRTOK); /* breakup on folder markers and never return an
							 empty string. */

		wxASSERT_MSG( tokens.GetCount() >= 2,
			wxString::Format(_T("Path '%s' does not seems to have enough \
								directory markers."), foundInis.Item(i))
		);
		wxString shortname = tokens[tokens.GetCount() - 2];

		wxLogDebug(_T("   Mod short name is: %s"), shortname);

		(*(this->configFiles))[shortname] = config;
	}

	// create internal repesentation of the mod.ini's
	wxLogDebug(_T("Transforming mod.ini's"));
	ConfigHash::iterator iter = this->configFiles->begin();

	while ( iter != this->configFiles->end() ) {
		ConfigHash::key_type shortname = iter->first;
		ConfigHash::mapped_type config = iter->second;
		ModItem* item = new ModItem(this, this->skinSystem);
		wxLogDebug(_T(" %s"), shortname);

		item->shortname = new wxString(shortname);

		readIniFileString(config, _T("/launcher/modname"), &(item->name));

		wxString *smallimagepath = NULL;
		readIniFileString(config, _T("/launcher/image255x112"), &smallimagepath);
		if ( smallimagepath != NULL ) {
			item->image = SkinSystem::VerifySmallImage(_T("."), shortname,
				*smallimagepath);
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
				shortname);
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
			item->skin->welcomeIcon = SkinSystem::VerifyWindowIcon(_T("."),
				shortname, *windowIconFile);
			if ( windowIconFile != NULL ) {
				delete windowIconFile;
			}

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
			item->skin->welcomeIcon = SkinSystem::VerifyTabIcon(_T("."),
				shortname, *welcomeIconFile);
			item->skin->modsIcon = SkinSystem::VerifyTabIcon(_T("."),
				shortname, *modsIconFile);
			item->skin->basicIcon = SkinSystem::VerifyTabIcon(_T("."),
				shortname, *basicIconFile);
			item->skin->advancedIcon = SkinSystem::VerifyTabIcon(_T("."),
				shortname, *advancedIconFile);
			item->skin->installIcon = SkinSystem::VerifyTabIcon(_T("."),
				shortname, *installIconFile);

			if ( welcomeIconFile != NULL ) delete welcomeIconFile;
			if ( modsIconFile != NULL )	delete modsIconFile;
			if ( basicIconFile != NULL ) delete basicIconFile;
			if ( advancedIconFile != NULL ) delete advancedIconFile;
			if ( installIconFile != NULL ) delete installIconFile;

			wxString *idealIconFile = NULL;
			readIniFileString(config, _T("/skin/idealicon"), &idealIconFile);
			item->skin->idealIcon = SkinSystem::VerifyIdealIcon(_T("."),
				shortname, *idealIconFile);
			if ( idealIconFile != NULL ) delete idealIconFile;

			wxString *fontName = NULL;
			readIniFileString(config, _T("/skin/font"), &fontName);
			int fontSize = 0;
			config->Read(_T("/skin/fontsize"), &fontSize);
			item->skin->baseFont = SkinSystem::VerifyFontChoice(_T("."),
				shortname, *fontName, fontSize);
			if ( fontName != NULL ) delete fontName;

		} else {
			wxLogDebug(_T("  Does Not Contain An skin Section."));
		}

		// langauges
		for ( size_t i = 0;	i < SupportedLanguages.Count(); i++ ) {
			wxString section = wxString::Format(_T("/%s"), SupportedLanguages[i]);
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

		++iter;
	}

	this->SetItemCount(this->tableData->Count());

}

/** the distructor.  Cleans up stuff. */
ModList::~ModList() {
	if ( this->configFiles != NULL ) {
		delete this->configFiles;
	}
	if ( this->tableData != NULL ) {
		delete this->tableData;
	}
}
/** Function takes the keyvalue string to search for, and returns via location
the pointer to value. If the keyvalue is not found *location will remain NULL.*/
void ModList::readIniFileString(ConfigHash::mapped_type config,
									 wxString keyvalue, wxString ** location) {
	if ( config->Exists(keyvalue) ) {
			*location = new wxString();
			config->Read(keyvalue, *location);
			if ( (*location)->EndsWith(this->semicolon) ) {
				(*location)->RemoveLast();
			}
	}
	wxLogDebug(_T("  %s:'%s'"), keyvalue,
		((*location) == NULL) ? _T("Not Specified") : **location);

	if ( (*location) != NULL && (*location)->empty() ) {
		wxLogDebug(_T("  Nulled %s"), keyvalue);
		delete *location;
		*location = NULL;
	}
}

/** */
void ModList::readFlagSet(ConfigHash::mapped_type config,
							   wxString keyprefix, FlagSetItem *set) {
	readIniFileString(config, wxString::Format(_T("%s/name"), keyprefix), &(set->name));
	readIniFileString(config, wxString::Format(_T("%s/flagset"), keyprefix), &(set->flagset));
	readIniFileString(config, wxString::Format(_T("%s/notes"), keyprefix), &(set->notes));
}

void ModList::readTranslation(ConfigHash::mapped_type config, wxString langaugename, I18nItem **trans) {
	wxString section = wxString::Format(_T("/%s"), langaugename);
	if ( config->Exists(section) ) {
		*trans = new I18nItem();

		readIniFileString(config, wxString::Format(_T("%s/modname"), section),
			&((*trans)->infotext));
		readIniFileString(config, wxString::Format(_T("%s/infotext"), section),
			&((*trans)->infotext));

	} else {
		wxLogDebug( 
			wxString::Format(_T("  Section '%s' does not exist."), langaugename));
	}
}

void ModList::OnDrawItem(wxDC &dc, const wxRect &rect, size_t n) const {
	wxLogDebug(_T(" Draw %04d,%04d = %04d,%04d"), rect.x, rect.y, rect.width, rect.height);
	if ( n == 0 ) {
		// draw the header
		dc.DrawText(_("HEADER"), 5, 5);
		
	} else {
		this->tableData->Item(n).Draw(dc, rect);

	}
}

void ModList::OnDrawSeparator(wxDC &dc, wxRect& rect, size_t n) const {
	//dc.DrawLine(rect.x, rect.y, rect.x + rect.width, rect.y + rect.height);
}

void ModList::OnDrawBackground(wxDC &dc, const wxRect& rect, size_t n) const {
	wxLogDebug(_T(" Background %04d,%04d = %04d,%04d"), rect.x, rect.y, rect.width, rect.height);
	dc.DestroyClippingRegion();
	if ( this->IsSelected(n) ) {
		dc.SetBrush(*wxGREEN_BRUSH);
		dc.SetBackground(*wxGREEN_BRUSH);
		dc.DrawRoundedRectangle(rect, 5.0);
	} else {
		dc.SetBrush(*wxWHITE_BRUSH);
		dc.SetBackground(*wxWHITE_BRUSH);
		dc.DrawRectangle(rect);
	}
}

wxCoord ModList::OnMeasureItem(size_t n) const {
	return (n == 0) ? 50 : 125;
}

void ModList::OnSelectionChange(wxCommandEvent &event) {
	wxLogDebug(_T("Selection changed to %d."), event.GetInt());
}

BEGIN_EVENT_TABLE(ModList, wxVListBox)
EVT_LISTBOX(ID_MODLISTBOX, ModList::OnSelectionChange)
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
ModItem::ModItem(wxWindow *parent, SkinSystem* skin) {
	this->panel = new wxPanel(parent);
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

	this->infoButton = new wxButton(this->panel, wxID_ANY, _("Info"));
	this->activateButton = new wxButton(this->panel, wxID_ANY, _("Activate"));

	this->infotextpanel = new InfoText(this->panel, this);
	ModImage* modImagePanel = new ModImage(this->panel, this);
	ModName* modNamePanel = new ModName(this->panel, this);

	wxBoxSizer* optionsButtons = new wxBoxSizer(wxVERTICAL);
	optionsButtons->Add(this->infoButton);
	optionsButtons->Add(this->activateButton);

	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(modNamePanel);
	sizer->Add(modImagePanel);
	sizer->Add(this->infotextpanel);
	sizer->Add(optionsButtons);

	this->panel->SetSizer(sizer);
	this->panel->Layout();

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
	if (this->bugs != NULL) delete this->support;
	if (this->forcedon != NULL) delete this->forcedon;
	if (this->forcedoff != NULL) delete this->forcedoff;
	if (this->flagsets != NULL) delete this->flagsets;
	if (this->skin != NULL) delete this->skin;
	if (this->i18n != NULL) delete this->i18n;
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
ModItem::InfoText::InfoText(wxWindow *parent, ModItem *myData) {
	this->Create(parent);

	this->myData = myData;
}

/** Draws the currently set InfoText into a specific width.  If the text is too
long, will also truncate the text so that it is not too long visually. */
void ModItem::InfoText::OnDraw(wxPaintEvent &event) {
	WXUNUSED(event);

	wxPaintDC dc(this);

	Draw(dc, wxRect(0, 0, 125, 400));
}

void ModItem::InfoText::Draw(wxDC &dc, const wxRect &rect) {
	if ( this->myData->infotext != NULL ) {
		wxStringTokenizer tokens(*(this->myData->infotext));
		ArrayOfWords words;
		words.Alloc(tokens.CountTokens());
		this->SetFont(this->myData->skinSystem->GetFont());

		do {
			wxString tok = tokens.GetNextToken();
			int x, y;
			this->GetTextExtent(tok, &x, &y, NULL, NULL, this->myData->skinSystem->GetFontPointer());

			Words* temp = new Words();
			temp->size = dc.GetTextExtent(tok);
			temp->word = tok;

			words.Add(temp);
		} while ( tokens.HasMoreTokens() );

		const int maxwidth = 200;
		int currentx = rect.x, currenty = rect.y;

		size_t currentwidth  = 0;
		wxString string;
		for( size_t i = 0; i < words.Count(); i++) {
			if ( currentwidth + words[i].size.x > maxwidth ) {
				dc.DrawText(string, currentx, currenty);

				string.Empty();
				currentwidth = 0;

				currenty += words[i].size.y;
				if (currenty + words[i].size.y > rect.y + rect.height) {
					break;
				}
			} else {
				string.append(_T(" "));
			}
			string.append(words[i].word);
			currentwidth += words[i].size.x;
		}
		if ( !string.IsEmpty()) {
			dc.DrawText(string, currentx, currenty);
		}
	}
}

BEGIN_EVENT_TABLE(ModItem::InfoText, wxPanel)
EVT_PAINT(ModItem::InfoText::OnDraw)
END_EVENT_TABLE()

///////////////////////////////////////////
/** \class ModItem::ModName
Extends wxPanel so that it can draw the mod's name to the correct size in the list
or the mod's short name.
*/
/** Constructor. Sets up stuff. */
ModItem::ModName::ModName(wxWindow *parent, ModItem *myData) {
	this->Create(parent);

	this->myData = myData;
}


///////////////////////////////////////////
/** \class ModItem::ModImage
Extends wxPanel so that it can draw the Mod's image on the list or degrade smoothly.
*/
/** Constructor. Sets up stuff. */
ModItem::ModImage::ModImage(wxWindow *parent, ModItem *myData) {
	this->Create(parent);

	this->myData = myData;
}