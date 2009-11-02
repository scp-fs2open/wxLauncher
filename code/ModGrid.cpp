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
								directory markers."))
		);
		wxString shortname = tokens[tokens.GetCount() - 2];

		wxLogDebug(_T("   Mod short name is: %s"), shortname);

		(*(this->configFiles))[shortname] = config;
	}

	// create internal repesentation of the mod.ini's
}
/** the distructor.  Cleans up stuff. */
ModGridTable::~ModGridTable() {
	if ( this->configFiles != NULL ) {
		delete this->configFiles;
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

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(FlagSets);

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


#include <wx/listimpl.cpp>
WX_DEFINE_LIST(I18nData);

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
