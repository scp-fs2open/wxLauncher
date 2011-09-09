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

#include "generated/configure_launcher.h"
#include "controls/FlagList.h"
#include "datastructures/FSOExecutable.h"
#include "tabs/AdvSettingsPage.h"
#include "apis/ProfileManager.h"
#include "global/ids.h"

#include "global/MemoryDebugging.h"

struct FlagInfo {
	wxString flag;
	wxString category;
	bool takesArg;
};
#include "datastructures/FlagInfo.cpp"

#define WIDTH_OF_CHECKBOX 16

Flag::Flag()
: checkbox(NULL), checkboxSizer(NULL) {
}

#include <wx/listimpl.cpp> // Magic Incantation
WX_DEFINE_LIST(FlagList);

#include <wx/listimpl.cpp> // Magic Incantation
WX_DEFINE_LIST(FlagCategoryList);

FlagSet::FlagSet(wxString Name) {
	this->Name = Name;
}

#include <wx/listimpl.cpp> // Magic Incantation
WX_DEFINE_LIST(FlagSetsList);

#include <wx/arrimpl.cpp> // Magic Incantation
WX_DEFINE_OBJARRAY(FlagFileArray);

// allows flag checkbox and text to be lined up, while avoiding
// visual collisions with flag category lines
const int ITEM_VERTICAL_OFFSET = 2; // in pixels
#if IS_LINUX
const int VERTICAL_OFFSET_MULTIPLIER = 2; // in pixels
#else
const int VERTICAL_OFFSET_MULTIPLIER = 1; // in pixels
#endif

FlagListBox::FlagListBox(wxWindow* parent, SkinSystem *skin)
:wxVListBox(parent,ID_FLAGLISTBOX) {
	wxASSERT(skin != NULL);
	this->skin = skin;
	this->drawStatus = DRAW_OK;
	this->errorText = new wxStaticText(this, wxID_ANY, wxEmptyString);

	this->Initialize();

	if ( this->drawStatus != DRAW_OK ) {
		this->SetItemCount(1);
	}
}

void FlagListBox::Initialize() {
	wxString tcPath, exeName;
	wxFileName exeFilename;

	wxLogDebug(_T("Initializing FlagList"));

	if ( !ProMan::GetProfileManager()->ProfileRead(PRO_CFG_TC_ROOT_FOLDER, &tcPath) ) {
		this->drawStatus = MISSING_TC;
		return;
	}

	if (!wxFileName::DirExists(tcPath)) {
		this->drawStatus = NONEXISTENT_TC;
		return;
	}
	
	if (!FSOExecutable::HasFSOExecutables(wxFileName(tcPath, wxEmptyString))) {
		this->drawStatus = INVALID_TC;
		return;
	}

	if ( !ProMan::GetProfileManager()->ProfileRead(PRO_CFG_TC_CURRENT_BINARY, &exeName)) {
		this->drawStatus = MISSING_EXE;
		return;
	}

#if IS_APPLE  // needed because on OSX exeName is a relative path from TC root dir
	exeFilename.Assign(tcPath + wxFileName::GetPathSeparator() + exeName);
#else
	exeFilename.Assign(tcPath, exeName);
#endif

	wxLogDebug(_T("exeName: ") + exeName);
	wxLogDebug(_T("exeFilename: ") + exeFilename.GetFullPath());
	
	if (!exeFilename.FileExists()) {
		this->drawStatus = INVALID_BINARY;
		return;
	}
	// Make sure that the directory that I am going to change to exists
	wxFileName tempExecutionLocation;
	tempExecutionLocation.AssignDir(GET_PROFILE_STORAGEFOLDER());
	tempExecutionLocation.AppendDir(_T("temp_flag_folder"));
	if ( !tempExecutionLocation.DirExists() 
		&& !tempExecutionLocation.Mkdir() ) {

		wxLogError(_T("Unable to create flag folder at %s"),
			tempExecutionLocation.GetFullPath().c_str());
		this->drawStatus = CANNOT_CREATE_FLAGFILE_FOLDER;
		return;
	}

	FlagFileArray flagFileLocations;
	flagFileLocations.Add(wxFileName(tcPath, _T("flags.lch")));
	flagFileLocations.Add(wxFileName(tempExecutionLocation.GetFullPath(), _T("flags.lch")));

	// remove potential flag files to eliminate any confusion.
	for( size_t i = 0; i < flagFileLocations.Count(); i++ ) {
		bool exists = flagFileLocations[i].FileExists();
		if (exists) {
			::wxRemoveFile(flagFileLocations[i].GetFullPath());
			wxLogDebug(_T(" Cleaned up %s ... %s"),
				flagFileLocations[i].GetFullPath().c_str(),
				(flagFileLocations[i].FileExists())? _T("Failed") : _T("Removed"));
		}
	}

	wxString previousWorkingDir(::wxGetCwd());
	// hopefully this doesn't goof anything up
	if ( !::wxSetWorkingDirectory(tempExecutionLocation.GetFullPath()) ) {
		wxLogError(_T("Unable to change working directory to %s"),
			tempExecutionLocation.GetFullPath().c_str());
		this->drawStatus = CANNOT_CHANGE_WORKING_FOLDER;
		return;
	}

	wxArrayString output;

	wxString commandline;
	// use "" to correct for spaces in path to exeFilename
	if (exeFilename.GetFullPath().Find(_T(" ")) != wxNOT_FOUND) {
		commandline = _T("\"") + exeFilename.GetFullPath() +  _T("\"") + _T(" -get_flags");
	} else {
		commandline = exeFilename.GetFullPath() + _T(" -get_flags");
	}

	wxLogDebug(_T(" Called FreeSpace 2 Open with command line '%s'."), commandline.c_str());
	FlagProcess *process = new FlagProcess(this, flagFileLocations);
	::wxExecute(commandline, wxEXEC_ASYNC, process);

	if ( !::wxSetWorkingDirectory(previousWorkingDir) ) {
		wxLogError(_T("Unable to change back to working directory %s"),
			previousWorkingDir.c_str());
		this->drawStatus = CANNOT_CHANGE_WORKING_FOLDER;
		return;
	}

	this->drawStatus = WAITING_FOR_FLAGFILE;
}

FlagListBox::DrawStatus FlagListBox::ParseFlagFile(wxFileName &flagfilename) {
	wxCHECK_MSG(flagfilename.FileExists(), FLAG_FILE_NOT_GENERATED,
		_T("The FreeSpace 2 Open executable did not generate a flag file."));

	wxFile flagfile(flagfilename.GetFullPath());
	wxLogDebug(_T("Reading flag file %s."), flagfilename.GetFullPath().c_str());
	// Flagfile requires that we use 32 bit little-endian numbers
	wxInt32 easy_flag_size, flag_size, num_easy_flags, num_flags;
	size_t bytesRead;

	bytesRead = flagfile.Read(&easy_flag_size, sizeof(easy_flag_size));
	if ( (size_t)wxInvalidOffset == bytesRead || bytesRead != sizeof(easy_flag_size) ) {
		wxLogError(_T(" Flag file is too short (failed to read easy_flag_size)"));
		return FLAG_FILE_NOT_VALID;
	}
	if ( easy_flag_size != 32 ) {
		wxLogError(_T("  Easy flag size (%d) is not supported"), easy_flag_size);
		return FLAG_FILE_NOT_SUPPORTED;
	}

	bytesRead = flagfile.Read(&flag_size, sizeof(flag_size));
	if ( (size_t)wxInvalidOffset == bytesRead || bytesRead != sizeof(flag_size) ) {
		wxLogError(_T(" Flag file is too short (failed to read flag_size)"));
		return FLAG_FILE_NOT_VALID;
	}
	if ( flag_size != 344 ) {
		wxLogError(_T(" Exe flag structure (%d) size is not supported"), flag_size);
		return FLAG_FILE_NOT_SUPPORTED;
	}

	bytesRead = flagfile.Read(&num_easy_flags, sizeof(num_easy_flags));
	if ( (size_t)wxInvalidOffset == bytesRead || bytesRead != sizeof(num_easy_flags) ) {
		wxLogError(_T(" Flag file is too short (failed to read num_easy_flags)"));
		return FLAG_FILE_NOT_VALID;
	}

	for ( int i = 0; i < num_easy_flags; i++ ) {
		char easy_flag[32];
		bytesRead = flagfile.Read(&easy_flag, sizeof(easy_flag));
		if ( (size_t)wxInvalidOffset == bytesRead || bytesRead != sizeof(easy_flag) ) {
			wxLogError(_T(" Flag file is too short, expected %d, got %d bytes (easy_flag)"), sizeof(easy_flag), bytesRead);
			return FLAG_FILE_NOT_VALID;
		}
		wxString flag(easy_flag, wxConvUTF8, strlen(easy_flag));
		this->easyflags.Add(flag);
	}

	bytesRead = flagfile.Read(&num_flags, sizeof(num_flags));
	if ( (size_t)wxInvalidOffset == bytesRead || bytesRead != sizeof(num_flags) ) {
		wxLogError(_T(" Flag file is too short (failed to read num_flags)"));
		return FLAG_FILE_NOT_VALID;
	}

	for ( int i = 0; i < num_flags; i++ ) {
		char flag_string[20];
		char description[40];
		wxInt32 fso_only, easy_on_flags, easy_off_flags;
		char easy_catagory[16], web_url[256];

		bytesRead = flagfile.Read(&flag_string, sizeof(flag_string));
		if ( (size_t)wxInvalidOffset == bytesRead || bytesRead != sizeof(flag_string) ) {
			wxLogError(_T(" Flag file is too short, expected %d, got %d bytes (flag_string)"), sizeof(flag_string), bytesRead);
			return FLAG_FILE_NOT_VALID;
		}

		bytesRead = flagfile.Read(&description, sizeof(description));
		if ( (size_t)wxInvalidOffset == bytesRead || bytesRead != sizeof(description) ) {
			wxLogError(_T(" Flag file is too short, expected %d, got %d bytes (description)"), sizeof(description), bytesRead);
			return FLAG_FILE_NOT_VALID;
		}

		bytesRead = flagfile.Read(&fso_only, sizeof(fso_only));
		if ( (size_t)wxInvalidOffset == bytesRead || bytesRead != sizeof(fso_only) ) {
			wxLogError(_T(" Flag file is too short, expected %d, got %d bytes (fso_only)"), sizeof(fso_only), bytesRead);
			return FLAG_FILE_NOT_VALID;
		}

		bytesRead = flagfile.Read(&easy_on_flags, sizeof(easy_on_flags));
		if ( (size_t)wxInvalidOffset == bytesRead || bytesRead != sizeof(easy_on_flags) ) {
			wxLogError(_T(" Flag file is too short, expected %d, got %d bytes (easy_on_flags)"), sizeof(easy_on_flags), bytesRead);
			return FLAG_FILE_NOT_VALID;
		}

		bytesRead = flagfile.Read(&easy_off_flags, sizeof(easy_off_flags));
		if ( (size_t)wxInvalidOffset == bytesRead || bytesRead != sizeof(easy_off_flags) ) {
			wxLogError(_T(" Flag file is too short, expected %d, got %d bytes (easy_off_flags)"), sizeof(easy_off_flags), bytesRead);
			return FLAG_FILE_NOT_VALID;
		}

		bytesRead = flagfile.Read(&easy_catagory, sizeof(easy_catagory));
		if ( (size_t)wxInvalidOffset == bytesRead || bytesRead != sizeof(easy_catagory) ) {
			wxLogError(_T(" Flag file is too short, expected %d, got %d bytes (easy_category)"), sizeof(easy_catagory), bytesRead);
			return FLAG_FILE_NOT_VALID;
		}

		bytesRead = flagfile.Read(&web_url, sizeof(web_url));
		if ( (size_t)wxInvalidOffset == bytesRead || bytesRead != sizeof(web_url) ) {
			wxLogError(_T(" Flag file is too short, expected %d, got %d bytes (web_url)"), sizeof(web_url), bytesRead);
			return FLAG_FILE_NOT_VALID;
		}

		flag_string[sizeof(flag_string)-1] = _T('\0');
		description[sizeof(description)-1] = _T('\0');
		easy_catagory[sizeof(easy_catagory)-1] = _T('\0');
		web_url[sizeof(web_url)-1] = _T('\0');

		Flag* flag = new Flag();
		flag->checkbox = new wxCheckBox(this, wxID_ANY, wxEmptyString);
		flag->checkbox->Hide();
		this->m_parent->Connect(flag->checkbox->GetId(),
			wxEVT_COMMAND_CHECKBOX_CLICKED,
			wxCommandEventHandler(AdvSettingsPage::OnNeedUpdateCommandLine));
		flag->checkboxSizer = new wxBoxSizer(wxVERTICAL);
		flag->checkboxSizer->AddSpacer(ITEM_VERTICAL_OFFSET);
		flag->checkboxSizer->Add(flag->checkbox);

		flag->isRecomendedFlag = false; // much better from a UI point of view than "true"
		flag->flagString = wxString(flag_string, wxConvUTF8, strlen(flag_string));
		flag->shortDescription = wxString(description, wxConvUTF8, strlen(description));
		flag->webURL = wxString(web_url, wxConvUTF8, strlen(web_url));
		flag->fsoCatagory = wxString(easy_catagory, wxConvUTF8, strlen(easy_catagory));

		flag->easyDisable = easy_off_flags;
		flag->easyEnable = easy_on_flags;

		FlagCategoryList::iterator iter = this->allSupportedFlagsByCategory.begin();
		while ( iter != this->allSupportedFlagsByCategory.end() ) {
			if ( flag->fsoCatagory == (*iter)->categoryName ) {
				break;
			}
			iter++;
		}
		if ( iter == this->allSupportedFlagsByCategory.end() ) {
			// did not find the category add it
			FlagCategory* flagCat = new FlagCategory();
			flagCat->categoryName = flag->fsoCatagory;

			Flag* headFlag = new Flag();
			headFlag->fsoCatagory = flag->fsoCatagory;
			headFlag->checkbox = NULL;
			headFlag->isRecomendedFlag = false;
			flagCat->flags.Append(headFlag);
			flagCat->flags.Append(flag);
			this->allSupportedFlagsByCategory.Append(flagCat);
		} else {
			(*iter)->flags.Append(flag);
		}
	}		

	wxLogDebug(_T(" easy_flag_size: %d, %d; flag_size: %d, %d; num_easy_flags: %d, %d; num_flags: %d, %d"),
		easy_flag_size, sizeof(easy_flag_size),
		flag_size, sizeof(flag_size),
		num_easy_flags, sizeof(num_easy_flags),
		num_flags, sizeof(num_flags));

	wxByte buildCaps;
	bytesRead = flagfile.Read(&buildCaps, sizeof(buildCaps));
	if ( (size_t)wxInvalidOffset == bytesRead ) {
		wxLogInfo(_T(" Old build that does not output its capabilities, must not support OpenAL"));
		buildCaps = 0;
	}

	return DRAW_OK;
}

FlagListBox::~FlagListBox() {
	FlagCategoryList::iterator catIter = this->allSupportedFlagsByCategory.begin();
	while ( catIter != this->allSupportedFlagsByCategory.end() ) {
		FlagCategory* category = *catIter;
		FlagList::iterator iter = category->flags.begin();
		while ( iter != category->flags.end() ) {
			Flag *flag = *iter;
			if ( flag->checkboxSizer != NULL ) {
				delete flag->checkboxSizer;
				flag->checkboxSizer = NULL;
			}
			delete flag;
			iter++;
		}
		category->flags.Clear();

		delete category;
		catIter++;
	}
	this->allSupportedFlagsByCategory.Clear();

	FlagSetsList::iterator flagSetIter = this->flagSets.begin();
	while ( flagSetIter != this->flagSets.end() ) {
		delete *flagSetIter;
		flagSetIter++;
	}
	this->flagSets.clear();
}

void FlagListBox::FindFlagAt(size_t n, Flag **flag, Flag ** catFlag) const {
	size_t index = n;
	FlagCategoryList::const_iterator iter =
		this->allSupportedFlagsByCategory.begin();
	while ( iter != this->allSupportedFlagsByCategory.end() ) {
		FlagCategory* cat = *iter;
		if ( cat->flags.GetCount() > index ) {
			(*flag) = cat->flags.Item(index)->GetData();
			if ( catFlag != NULL ) {
				// cat flag is first in the list
				(*catFlag) = cat->flags.Item(0)->GetData(); 
			}
			break;
		} else {
			index -= cat->flags.GetCount();
		}
		iter++;
	}
}

void FlagListBox::OnDrawItem(wxDC &dc, const wxRect &rect, size_t n) const {
#if IS_WIN32 // replace the ugly default font with one based on the system default
	wxFont font(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
	dc.SetFont(font);
#endif

	if ( this->drawStatus == DRAW_OK ) {
		this->errorText->Hide();
		Flag* item = NULL;
		this->FindFlagAt(n, &item, NULL);		
		wxCHECK_RET(item != NULL, _T("Flag pointer is null"));
	
		if ( item->isRecomendedFlag ) {
			dc.DrawBitmap(this->skin->GetIdealIcon(), rect.x, rect.y);
		}

		if (item->checkbox != NULL) {
			item->checkbox->Show();
			item->checkboxSizer->SetDimension(
				rect.x + SkinSystem::IdealIconWidth,
				rect.y,
				SkinSystem::IdealIconWidth,
				rect.height);
		}
		if ( item->flagString.IsEmpty() ) {
			// draw a category
#if IS_WIN32
			font.SetWeight(wxFONTWEIGHT_BOLD);
			dc.SetTextForeground(*wxWHITE);
			dc.SetFont(font);
#endif
			dc.DrawText(wxString(_T(" ")) + item->fsoCatagory,
						rect.x + SkinSystem::IdealIconWidth + WIDTH_OF_CHECKBOX,
				    rect.y + (VERTICAL_OFFSET_MULTIPLIER*ITEM_VERTICAL_OFFSET));
#if IS_WIN32
			dc.SetTextForeground(*wxBLACK);
#endif
		} else if ( item->shortDescription.IsEmpty() ) {
			dc.DrawText(wxString(_T(" ")) + item->flagString,
						rect.x + SkinSystem::IdealIconWidth + WIDTH_OF_CHECKBOX,
				    rect.y + (VERTICAL_OFFSET_MULTIPLIER*ITEM_VERTICAL_OFFSET));
		} else {
			dc.DrawText(wxString(_T(" ")) + item->shortDescription,
						rect.x + SkinSystem::IdealIconWidth + WIDTH_OF_CHECKBOX,
				    rect.y + (VERTICAL_OFFSET_MULTIPLIER*ITEM_VERTICAL_OFFSET));
		}
	} else {
		wxASSERT_MSG( n == 0, _T("FLAGLISTBOX: Trying to draw background n != 0") );
	}
}

wxCoord FlagListBox::OnMeasureItem(size_t n) const {
	if ( this->drawStatus == DRAW_OK ) {
		return SkinSystem::IdealIconHeight;
	} else {
		return this->GetSize().y;
	}
}

void FlagListBox::OnDrawBackground(wxDC &dc, const wxRect &rect, size_t n) const {
	Flag* item = NULL;
	this->FindFlagAt(n, &item, NULL);
	if ( item != NULL && item->flagString.IsEmpty() ) {
		dc.DestroyClippingRegion();
		wxColour background = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
		wxBrush b(background);
		dc.SetPen(wxPen(background));
		dc.SetBrush(b);
		dc.SetBackground(b);
		dc.DrawRectangle(rect);
	} else {
		dc.DestroyClippingRegion();
		wxColour background = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
		wxBrush b(background);
		dc.SetPen(wxPen(background));
		dc.SetBrush(b);
		dc.SetBackground(b);
		dc.DrawRectangle(rect);
	}
}

void FlagListBox::OnSize(wxSizeEvent &event) {
	wxVListBox::OnSize(event); // call parents onSize
	if ( this->drawStatus != DRAW_OK ) {
		wxRect rect(0, 0, this->GetSize().x, this->GetSize().y);

		wxString msg;
		switch(this->drawStatus) {
			case MISSING_TC:
				msg = _("No FreeSpace 2 installation or total conversion has been selected.\nSelect a FreeSpace 2 installation or a total conversion\non the Basic Settings page.");
				break;
			case NONEXISTENT_TC:
				msg = _("The selected FreeSpace 2 installation or total conversion root folder does not exist.\nSelect a different FreeSpace 2 installation or total conversion\non the Basic Settings page.");
				break;
			case INVALID_TC:
				msg = wxString(_("The selected FreeSpace 2 installation or total conversion root folder has no FreeSpace 2 Open executables.\n")) +
					_("Either add FS2 Open executables to the root folder and refresh the list of executables on the Basic Settings page, or select a different FreeSpace 2 installation or total conversion on the Basic Settings page.");
				break;
			case MISSING_EXE:
				msg = _("No FreeSpace 2 Open executable has been selected.\nSelect an executable on the Basic Settings page.");
				break;
			case INVALID_BINARY:
				msg = _("The selected FreeSpace 2 Open executable does not exist.\nSelect another on the Basic Settings page.");
				break;
			case WAITING_FOR_FLAGFILE:
				msg = _("Waiting for flag file to be produced and parsed.");
				break;
			case FLAG_FILE_NOT_GENERATED:
				msg = _("The executable did not generate a flag file.\nMake sure that the executable is a FreeSpace 2 Open executable.");
				break;
			case FLAG_FILE_NOT_VALID:
				msg = _("Generated flag file was not complete.\nPlease talk to a maintainer of this launcher, since you probably found a bug.");
				break;
			case FLAG_FILE_NOT_SUPPORTED:
				msg = _("Generated flag file is not supported.\nUpdate the launcher or talk to a maintainer of this launcher if you have the most recent version of the launcher.");
				break;
			default:
				msg = wxString::Format(
					_("Unknown error occurred while obtaining the flag file from the FreeSpace 2 Open executable (%d)"),
					this->drawStatus);
				break;
		}
		this->errorText->Show();
		this->errorText->SetLabel(msg);
		wxFont errorFont(16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
		this->errorText->SetFont(errorFont);

		this->errorText->SetSize(rect, wxSIZE_FORCE);
		this->errorText->Wrap(rect.width);
	}
}

void FlagListBox::OnDoubleClickFlag(wxCommandEvent &WXUNUSED(event)) {
	int selected = this->GetSelection();
	int flagIndex = 0;

	FlagCategoryList::const_iterator category =
	this->allSupportedFlagsByCategory.begin();
	while (category != this->allSupportedFlagsByCategory.end()) {
		FlagList::const_iterator flag = (*category)->flags.begin();
		while( flag != (*category)->flags.end() ) {
			if ( flagIndex == selected
				&& !(*flag)->webURL.IsEmpty() ) {
				wxLaunchDefaultBrowser((*flag)->webURL);
				return;
			}
			flag++;
			flagIndex++;
		}
		category++;
	}
}

void FlagListBox::OnCheckCategoryBox(wxCommandEvent &WXUNUSED(event)) {
	this->RefreshRect(this->GetRect(), true);
}

wxString FlagListBox::GenerateStringList() {
	wxString flagList;
	FlagCategoryList::const_iterator cat =
		this->allSupportedFlagsByCategory.begin();
	while ( cat != this->allSupportedFlagsByCategory.end() ) {
		FlagList::const_iterator flags =
			(*cat)->flags.begin();
		while ( flags != (*cat)->flags.end() ) {
			if ( (*flags)->checkbox != NULL
				&& (*flags)->checkbox->IsChecked() 
				&& !(*flags)->flagString.IsEmpty() ) {
					if ( !flagList.IsEmpty() ) {
						flagList += _T(" ");
					}
					flagList += (*flags)->flagString;
			}
			flags++;
		}
		cat++;
	}
	return flagList;
}

bool FlagListBox::SetFlag(wxString flagString, bool state) {
	FlagCategoryList::const_iterator category =
		this->allSupportedFlagsByCategory.begin();
	while (category != this->allSupportedFlagsByCategory.end()) {
		FlagList::const_iterator flag = (*category)->flags.begin();
		while( flag != (*category)->flags.end() ) {
			if ( !(*flag)->flagString.IsEmpty()
				&& (*flag)->flagString == flagString ) {
					(*flag)->checkbox->SetValue(state);
					return true;
			}
			flag++;
		}
		category++;
	}
	return false;
}

BEGIN_EVENT_TABLE(FlagListBox, wxVListBox)
EVT_SIZE(FlagListBox::OnSize)
EVT_LISTBOX_DCLICK(ID_FLAGLISTBOX, FlagListBox::OnDoubleClickFlag)
END_EVENT_TABLE()

bool FlagListBox::SetFlagSet(wxString setToFind) {
	if ( this->flagSets.GetCount() == 0 ) {
		this->generateFlagSets();
	}
	FlagSetsList::const_iterator flagSetsIter =
		this->flagSets.begin();
	FlagSet* sets = NULL; 
	while(flagSetsIter != this->flagSets.end()) {
		if ( (*flagSetsIter)->Name.StartsWith(setToFind) ) {
			sets = *flagSetsIter;
		}
		flagSetsIter++;
	}
	if ( sets == NULL ) {
		// never found the set
		return false;
	}

	wxArrayString::const_iterator disableIter =
		sets->FlagsToDisable.begin();
	while ( disableIter != sets->FlagsToDisable.end() ) {
		this->SetFlag(*disableIter, false);
		disableIter++;
	}
	wxArrayString::const_iterator enableIter =
		sets->FlagsToEnable.begin();
	while ( enableIter != sets->FlagsToEnable.end() ) {
		this->SetFlag(*enableIter, true);
		enableIter++;
	}
	return true;
}

void FlagListBox::generateFlagSets() {
	this->flagSets.clear();
	// \todo include the flag sets of the mod.inis as well

	// custom
	{
		FlagSet* flagSetCustom = new FlagSet(_("Custom"));
		this->flagSets.Append(flagSetCustom);
	}
	// the easy flags.
	wxUint32 counter = 0;
	wxArrayString::const_iterator easyIter =
		this->easyflags.begin();
	while ( easyIter != this->easyflags.end() ) {
		wxString easyFlag = *easyIter;

		if ( easyFlag.StartsWith(_T("Custom")) ) {
			// do nothing, we already have a custom
		} else {
			FlagSet* flagSet = new FlagSet(easyFlag);
			FlagCategoryList::const_iterator catIter =
				this->allSupportedFlagsByCategory.begin();

			while ( catIter != this->allSupportedFlagsByCategory.end() ) {
				FlagList::const_iterator flagIter =
					(*catIter)->flags.begin();

				while ( flagIter != (*catIter)->flags.end() ) {
					Flag* flag = *flagIter;

					if ( !flag->flagString.IsEmpty()
						&& (flag->easyEnable & counter) > 0 ) {
						flagSet->FlagsToEnable.Add(flag->flagString);
					}
					if ( !flag->flagString.IsEmpty()
						&& (flag->easyDisable & counter) > 0 ) {
						flagSet->FlagsToDisable.Add(flag->flagString);
					}
					flagIter++;
				}
				catIter++;
			}
			this->flagSets.Append(flagSet);
		}

		if (counter < 1) {
			counter = 2; // prime the counter so we can bitshift for the rest
		} else {
			counter = counter << 1;
		}
		if ( counter > (wxUint32)(1 << 31) ) {
			// we have checked 31 bits of counter, this is too many easy flag sets
			easyIter = this->easyflags.end();
			wxLogError(_T("FreeSpace 2 Open executable has more than 31 easy flag categories"));
		} else {
			easyIter++;
		}
	}
}

wxArrayString& FlagListBox::GetFlagSets(wxArrayString &arr) {
	if ( this->flagSets.size() == 0 ) {
		this->generateFlagSets();
	}
	FlagSetsList::const_iterator flagSetsIter =
		this->flagSets.begin();
	while ( flagSetsIter != this->flagSets.end() ) {
		arr.Add((*flagSetsIter)->Name);
		flagSetsIter++;
	}
	return arr;
}

/** sets all flags off. */
void FlagListBox::ResetFlags() {
	FlagCategoryList::const_iterator category =
	this->allSupportedFlagsByCategory.begin();
	while (category != this->allSupportedFlagsByCategory.end()) {
		FlagList::const_iterator flag = (*category)->flags.begin();
		while( flag != (*category)->flags.end() ) {
			if ( (*flag)->checkbox != NULL) {
				(*flag)->checkbox->SetValue(false);
			}
			flag++;
		}
		category++;
	}
}

/** returns true when the FlagList will draw the the actual list,
false when the FlagList is showing an error message. */
bool FlagListBox::IsDrawOK() {
	return (this->drawStatus == DRAW_OK);
}


FlagListBox::FlagProcess::FlagProcess(
							FlagListBox *target,
							FlagFileArray flagFileLocations)
							:
target(target), flagFileLocations(flagFileLocations) {
}

DEFINE_EVENT_TYPE(EVT_FLAG_LIST_BOX_DRAW_STATUS_CHANGE);

void FlagListBox::FlagProcess::OnTerminate(int pid, int status) {
	wxLogDebug(_T(" FreeSpace 2 Open returned %d when polled for the flags"), status);

	// Find the flag file
	wxFileName flagfile;
	for( size_t i = 0; i < flagFileLocations.Count(); i++ ) {
		bool exists = flagFileLocations[i].FileExists();
		if (exists) {
			flagfile = flagFileLocations[i];
			wxLogDebug(_T(" Searching for flag file at %s ... %s"),
				flagFileLocations[i].GetFullPath().c_str(),
				(flagFileLocations[i].FileExists())? _T("Located") : _T("Not Here"));
		}
	}


	if ( !flagfile.FileExists() ) {
		target->drawStatus = FLAG_FILE_NOT_GENERATED;
		wxLogError(_T(" FreeSpace 2 Open did not generate a flag file."));
		return;
	}

	target->drawStatus = target->ParseFlagFile(flagfile);
	if ( target->drawStatus == DRAW_OK ) {
		::wxRemoveFile(flagfile.GetFullPath());
		
		size_t itemCount = 0;
		FlagCategoryList::iterator iter =
			target->allSupportedFlagsByCategory.begin();
		while ( iter != target->allSupportedFlagsByCategory.end() ) {
			itemCount += (*iter)->flags.GetCount();
			iter++;
		}
		target->SetItemCount(itemCount);
	}

	wxCommandEvent evt(EVT_FLAG_LIST_BOX_DRAW_STATUS_CHANGE, wxID_NONE);
	this->target->AddPendingEvent(evt);

	delete this;
}
