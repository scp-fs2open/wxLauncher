#include <wx/wx.h>
#include <wx/grid.h>
#include <wx/dir.h>
#include <wx/fileconf.h>
#include <wx/wfstream.h>
#include <wx/tokenzr.h>
#include <wx/arrstr.h>
#include "ModGrid.h"
#include "wxIDS.h"

ModGrid::ModGrid(wxWindow *parent, wxSize &size) {
	this->Create(parent, ID_MODGRID, wxDefaultPosition, size );

	this->DisableCellEditControl();
	this->EnableEditing(false);	// Grid must be read only.
	this->EnableGridLines(false);	// no lines, we want to look like a listbox

	this->SetTable(new ModGridTable(), true, wxGridSelectRows);
}

///////////////////////////////////////////////////////////////////////////////
// ModGridTable

/** The constructor.  Scans the TC directory for the mods, parses the .ini's
and transforms that into its internal representation for ModGridTable to 
display. */
ModGridTable::ModGridTable(): wxGridTableBase() {
	this->semicolon = new wxChar(';');
	this->semicolon[1] = NULL;
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
	(*(this->configFiles))[_T("(No Mod)")] = new wxFileConfig();

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
		ModItem* item = new ModItem();
		wxLogDebug(_T(" %s"), shortname);

		readIniFileString(config, _T("/launcher/modname"), &(item->name));

		wxString *smallimagepath = NULL;
		readIniFileString(config, _T("/launcher/image255x112"), &smallimagepath);
		
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

			wxString *idealIconFile = NULL;
			readIniFileString(config, _T("/skin/idealicon"), &idealIconFile);

			wxString *fontName = NULL;
			readIniFileString(config, _T("/skin/font"), &fontName);
			wxString *fontSize = NULL;
			readIniFileString(config, _T("/skin/fontsize"), &fontSize);

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

		++iter;
	}

}
/** the distructor.  Cleans up stuff. */
ModGridTable::~ModGridTable() {
	if ( this->configFiles != NULL ) {
		delete this->configFiles;
	}
}
/** Function takes the keyvalue string to search for, and returns via location
the pointer to value. If the keyvalue is not found *location will remain NULL.*/
void ModGridTable::readIniFileString(ConfigHash::mapped_type config,
									 wxString keyvalue, wxString ** location) {
	if ( config->Exists(keyvalue) ) {
			*location = new wxString();
			config->Read(keyvalue, *location);
			if ( (*location)->EndsWith(this->semicolon) ) {
				(*location)->RemoveLast();
			}
		}
	wxLogDebug(_T("  %s:'%s'"), keyvalue, ((*location) == NULL) ? _T("Not Specified") : **location );
}

/** */
void ModGridTable::readFlagSet(ConfigHash::mapped_type config,
							   wxString keyprefix, FlagSetItem *set) {
	readIniFileString(config, wxString::Format(_T("%s/name"), keyprefix), &(set->name));
	readIniFileString(config, wxString::Format(_T("%s/flagset"), keyprefix), &(set->flagset));
	readIniFileString(config, wxString::Format(_T("%s/notes"), keyprefix), &(set->notes));
}

void ModGridTable::readTranslation(ConfigHash::mapped_type config, wxString langaugename, I18nItem **trans) {
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

int ModGridTable::GetNumberRows() {
	return 0;
}

int ModGridTable::GetNumberCols() {
	return 4;
}

bool ModGridTable::IsEmptyCell(int row, int col) {
	return true;
}

wxString ModGridTable::GetValue(int row, int col) {
	return wxString();
}

void ModGridTable::SetValue(int row, int col, const wxString& value) {
	;
}

wxString ModGridTable::GetTypeName(int row, int col) {
	return wxString();
}

bool ModGridTable::CanGetValueAs(int row, int col, const wxString& typeName) {
	return false;
}

bool ModGridTable::CanSetValueAs(int row, int col, const wxString& typeName) {
	return false;
}

long ModGridTable::GetValueAsLong(int row, int col) {
	return 0L;
}

double ModGridTable::GetValueAsDouble(int row, int col) {
	return 0.0;
}

bool ModGridTable::GetValueAsBool(int row, int col) {
	return false;
}

void ModGridTable::SetValueAsLong(int row, int col, long value) {
	;
}

void ModGridTable::SetValueAsDouble(int row, int col, double value) {
	;
}

void ModGridTable::SetValueAsBool(int row, int col, bool value) {
	;
}

void* ModGridTable::GetValueAsCustom(int row, int col, const wxString& typeName) {
	return NULL;
}

void ModGridTable::SetValueAsCustom(int row, int col, const wxString& typeName, void *value) {
	;
}

///////////////////////////////////////////////////////////////////////////////
// Flagsets
/** \struct FlagSetItem
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
/** \struct I18nItem
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
ModGridTable::ModItem::ModItem() {
	this->name = NULL;
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
}
