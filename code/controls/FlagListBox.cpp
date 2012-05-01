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
#include "controls/FlagListBox.h"
//#include "apis/ProfileProxy.h" // TODO uncomment once proxy is ready
#include "tabs/AdvSettingsPage.h"
#include "global/ids.h"

#include "global/MemoryDebugging.h"

FlagListCheckBox::FlagListCheckBox(
	wxWindow* parent,
	const wxString& label,
	const wxString& flagString,
	int flagIndex)
: wxCheckBox(parent, wxID_ANY, label),
  flagString(flagString),
  flagIndex(flagIndex) {
	  wxASSERT(parent != NULL);
	  wxASSERT(!flagString.IsEmpty());
}

void FlagListCheckBox::OnClicked(wxCommandEvent &WXUNUSED(event)) {
	// FIXME the following line doesn't work yet because profile proxy isn't implemented
	//	ProfileProxy::GetProfileProxy()->SetFlag(this->flagString, this->flagIndex, this->IsChecked());
	wxLogDebug(_T("flag %s with index %d is now %s"),
		flagString.c_str(), flagIndex, this->IsChecked() ? _T("on") : _T("off"));
	
	// FIXME temp until the proxy is working
	wxCommandEvent fakeEvent;
	dynamic_cast<AdvSettingsPage*>(this->GetParent()->GetParent())->OnNeedUpdateCommandLine(fakeEvent);
}

FlagListCheckBoxItem::FlagListCheckBoxItem(const wxString& fsoCategory)
: fsoCategory(fsoCategory), checkBox(NULL), checkBoxSizer(NULL),
  shortDescription(wxEmptyString), flagString(wxEmptyString), flagIndex(-1),
  isRecommendedFlag(false) {
	  wxASSERT(!fsoCategory.IsEmpty());
}

FlagListCheckBoxItem::FlagListCheckBoxItem(
	FlagListCheckBox& checkBox, wxSizer& checkBoxSizer,
	const wxString& shortDescription, const wxString& flagString,
	const int flagIndex, const bool isRecommendedFlag)
: fsoCategory(wxEmptyString), checkBox(&checkBox), checkBoxSizer(&checkBoxSizer),
  shortDescription(shortDescription), flagString(flagString),
  flagIndex(flagIndex), isRecommendedFlag(isRecommendedFlag) {
	  // shortDescription can be empty
	  wxASSERT(!flagString.IsEmpty());
	  wxASSERT(flagIndex >= 0);
}

FlagListCheckBoxItem::~FlagListCheckBoxItem() {
	delete this->checkBoxSizer;
}

#include <wx/listimpl.cpp> // Magic Incantation
WX_DEFINE_LIST(FlagListCheckBoxItems);

DEFINE_EVENT_TYPE(EVT_FLAG_LIST_BOX_READY);

#include <wx/listimpl.cpp> // required magic incantation
WX_DEFINE_LIST(FlagListBoxReadyEventHandlers);

void FlagListBox::RegisterFlagListBoxReady(wxEvtHandler *handler) {
	this->flagListBoxReadyHandlers.Append(handler);
}

void FlagListBox::UnRegisterFlagListBoxReady(wxEvtHandler *handler) {
	this->flagListBoxReadyHandlers.DeleteObject(handler);
}

void FlagListBox::GenerateFlagListBoxReady() {
	wxASSERT_MSG(!this->isReadyEventGenerated,
		_T("GenerateFlagListBoxReady() was called a second time."));
	
	wxCommandEvent event(EVT_FLAG_LIST_BOX_READY, wxID_NONE);
	
	wxLogDebug(_T("Generating EVT_FLAG_LIST_BOX_READY event"));
	for (FlagListBoxReadyEventHandlers::iterator
		 iter = this->flagListBoxReadyHandlers.begin(),
		 end = this->flagListBoxReadyHandlers.end(); iter != end; ++iter) {
		wxEvtHandler* current = *iter;
		current->AddPendingEvent(event);
		wxLogDebug(_T(" Sent EVT_FLAG_LIST_BOX_READY event to %p"), current);
	}
	
	this->isReadyEventGenerated = true;
}

struct FlagInfo {
	wxString flag;
	wxString category;
	bool takesArg;
};
#include "datastructures/FlagInfo.cpp"

#define WIDTH_OF_CHECKBOX 16

// allows flag checkbox and text to be lined up, while avoiding
// visual collisions with flag category lines
const int ITEM_VERTICAL_OFFSET = 2; // in pixels
#if IS_LINUX
const int VERTICAL_OFFSET_MULTIPLIER = 2; // in pixels
#else
const int VERTICAL_OFFSET_MULTIPLIER = 1; // in pixels
#endif

FlagListBox::FlagListBox(wxWindow* parent, SkinSystem *skin)
: wxVListBox(parent,ID_FLAGLISTBOX),
  isReadyEventGenerated(false),
  isReady(false),
  flagData(NULL),
  areCheckBoxesGenerated(false) {
	wxASSERT(skin != NULL);
	this->skin = skin;
	
	this->errorText =
		new wxStaticText(this, wxID_ANY, wxEmptyString, wxDefaultPosition,
			wxDefaultSize, wxALIGN_CENTER);
}

void FlagListBox::AcceptFlagData(FlagFileData* flagData) {
	wxCHECK_RET(flagData != NULL, _T("AcceptFlagData(): flagData is null."));
	wxCHECK_RET(this->flagData == NULL,
		_T("AcceptFlagData(): flag list box given flag data twice."));
	
	this->flagData = flagData;
	FlagListBoxData* data = this->flagData->GenerateFlagListBoxData();
	wxCHECK_RET(data != NULL,
		_T("AcceptFlagData(): FlagFileData::GenerateFlagListBoxData() returned null."));
	this->GenerateCheckBoxes(*data);
	this->SetItemCount(flagData->GetItemCount());

	this->isReady = true;
	this->GenerateFlagListBoxReady();
}

void FlagListBox::GenerateCheckBoxes(const FlagListBoxData& data) {
	wxASSERT(!data.IsEmpty());
	wxASSERT_MSG(!this->areCheckBoxesGenerated,
		_T("Attempted to generate checkboxes a second time."));
	
	FlagListCheckBox* checkBox;
	wxSizer* checkBoxSizer;
	
	for (FlagListBoxData::const_iterator dataIter = data.begin();
		 dataIter != data.end(); ++dataIter) {
		
		FlagListBoxDataItem* item = *dataIter;
		
		if (!item->fsoCategory.IsEmpty()) {
			this->checkBoxes.Append(
				new FlagListCheckBoxItem(item->fsoCategory));
			continue;
		}
		
		checkBox =
			new FlagListCheckBox(
				this,
				wxEmptyString,
				item->flagString,
				item->GetFlagIndex());
		checkBox->Hide();
		
		checkBox->Connect(
			checkBox->GetId(),
			wxEVT_COMMAND_CHECKBOX_CLICKED,
			wxCommandEventHandler(FlagListCheckBox::OnClicked));
		
		checkBoxSizer = new wxBoxSizer(wxVERTICAL);
		checkBoxSizer->AddSpacer(ITEM_VERTICAL_OFFSET);
		checkBoxSizer->Add(checkBox);
		
		this->checkBoxes.Append(
			new FlagListCheckBoxItem(*checkBox, *checkBoxSizer,
				item->shortDescription, item->flagString, item->GetFlagIndex(),
				item->isRecommendedFlag));
	}
	
	this->areCheckBoxesGenerated = true;
}

FlagListBox::~FlagListBox() {
	FlagFileData* temp = this->flagData;
	this->flagData = NULL;
	delete temp;
	
	this->checkBoxes.Clear();
}

FlagListCheckBoxItem* FlagListBox::FindFlagAt(size_t n) const {
	wxCHECK_MSG(this->isReady, NULL,
		_T("FindFlagAt() called when flag list box is not ready"));
	wxCHECK_MSG(n >= 0 && n < this->checkBoxes.GetCount(), NULL,
		wxString::Format(_T("FindFlagAt() called with out-of-range value %lu"), n));
	
	return this->checkBoxes[n];
}

void FlagListBox::OnDrawItem(wxDC &dc, const wxRect &rect, size_t n) const {
#if IS_WIN32 // replace the ugly default font with one based on the system default
	wxFont font(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
	dc.SetFont(font);
#endif
	
	if (this->IsReady()) {
		this->errorText->Hide();
		FlagListCheckBoxItem* item = this->FindFlagAt(n);
		wxCHECK_RET(item != NULL, _T("Flag pointer is null"));
		
		if (item->GetCheckBox() != NULL) {
			if (item->IsRecommendedFlag()) {
				dc.DrawBitmap(this->skin->GetIdealIcon(), rect.x, rect.y);
			}
			
			item->GetCheckBox()->Show();
			item->GetCheckBoxSizer()->SetDimension(
				rect.x + SkinSystem::IdealIconWidth,
				rect.y,
				SkinSystem::IdealIconWidth,
				rect.height);
			
			if (item->GetShortDescription().IsEmpty()) {
				dc.DrawText(wxString(_T(" ")) + item->GetFlagString(),
					rect.x + SkinSystem::IdealIconWidth + WIDTH_OF_CHECKBOX,
					rect.y + (VERTICAL_OFFSET_MULTIPLIER*ITEM_VERTICAL_OFFSET));
			} else {
				dc.DrawText(wxString(_T(" ")) + item->GetShortDescription(),
					rect.x + SkinSystem::IdealIconWidth + WIDTH_OF_CHECKBOX,
					rect.y + (VERTICAL_OFFSET_MULTIPLIER*ITEM_VERTICAL_OFFSET));
			}
		} else { // draw a category
			wxASSERT(!item->GetFsoCategory().IsEmpty());
#if IS_WIN32
			font.SetWeight(wxFONTWEIGHT_BOLD);
			dc.SetTextForeground(*wxWHITE);
			dc.SetFont(font);
#endif
			dc.DrawText(wxString(_T(" ")) + item->GetFsoCategory(),
				rect.x + SkinSystem::IdealIconWidth + WIDTH_OF_CHECKBOX,
				rect.y + (VERTICAL_OFFSET_MULTIPLIER*ITEM_VERTICAL_OFFSET));
#if IS_WIN32
			dc.SetTextForeground(*wxBLACK);
#endif
		}
	} else {
		wxASSERT_MSG( n == 0, _T("FLAGLISTBOX: Trying to draw background n != 0") );
	}
}

wxCoord FlagListBox::OnMeasureItem(size_t n) const {
	if ( this->IsReady()) {
		return SkinSystem::IdealIconHeight;
	} else {
		return this->GetSize().y;
	}
}

void FlagListBox::OnDrawBackground(wxDC &dc, const wxRect &rect, size_t n) const {
	wxColour background = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
	
	if (this->isReady) {
		FlagListCheckBoxItem* item = FindFlagAt(n);
		if (item != NULL && item->GetFlagString().IsEmpty()) { // category header
			background = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
		}
	}
	
	dc.DestroyClippingRegion();
	wxBrush b(background);
	dc.SetPen(wxPen(background));
	dc.SetBrush(b);
	dc.SetBackground(b);
	dc.DrawRectangle(rect);
}

void FlagListBox::OnSize(wxSizeEvent &event) {
	wxVListBox::OnSize(event); // call parents onSize
	if (!this->IsReady()) {
		wxRect rect(0, 0, this->GetSize().x, this->GetSize().y);
		
		wxString msg = wxEmptyString;
		
		if (!FlagListManager::GetFlagListManager()->IsProcessingOK()) {
			msg = FlagListManager::GetFlagListManager()->GetStatusMessage();
		} else {
			msg = _("Waiting for extracted flag file data to be received.");
		}
		wxASSERT(!msg.IsEmpty());
		
		this->errorText->Show();
		this->errorText->SetLabel(msg);
		wxFont errorFont(16, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
		this->errorText->SetFont(errorFont);

		this->errorText->SetSize(rect, wxSIZE_FORCE);
		this->errorText->Wrap(rect.width);
		this->errorText->Center();
	}
}

void FlagListBox::OnDoubleClickFlag(wxCommandEvent &WXUNUSED(event)) {
	wxCHECK_RET(this->flagData != NULL,
		_T("OnDoubleClickFlag() called when flag data was null."));
	
	const wxString* webURL = this->flagData->GetWebURL(this->GetSelection());
	wxCHECK_RET(webURL != NULL,
		_T("GetWebURL() returned NULL, which shouldn't happen."));
	
	if (!webURL->IsEmpty()) {
		wxLaunchDefaultBrowser(*webURL);
	}
}

wxString FlagListBox::GenerateStringList() const {
	wxString flagList;
	
	wxCHECK_MSG(this->isReady, wxEmptyString,
		_T("GenerateStringList() called when flag list box is not ready."));
	
	FlagListCheckBoxItems::const_iterator it = this->checkBoxes.begin();
	while (it != this->checkBoxes.end()) {
		FlagListCheckBoxItem* item = *it;
		
		if (item->GetCheckBox() != NULL
			&& item->GetCheckBox()->IsChecked() 
			&& !item->GetFlagString().IsEmpty()) {
			
			if (!flagList.IsEmpty()) {
				flagList += _T(" ");
			}
			
			flagList += item->GetFlagString();
		}
		++it;
	}
	
	return flagList;
}

bool FlagListBox::SetFlag(const wxString& flagString, const bool state) {
	wxCHECK_MSG(this->isReady, false,
		_T("SetFlag() called when flag list box is not ready."));
	wxCHECK_MSG(!flagString.IsEmpty(), false,
		_T("SetFlag() called with empty flagString."));
	
	FlagListCheckBoxItems::iterator it = this->checkBoxes.begin();
	while (it != this->checkBoxes.end()) {
		FlagListCheckBoxItem* item = *it;
		
		if (!item->GetFlagString().IsEmpty()
			&& item->GetFlagString() == flagString) {
			item->GetCheckBox()->SetValue(state);
			return true;
		}
		++it;
	}
	return false;
}

BEGIN_EVENT_TABLE(FlagListBox, wxVListBox)
EVT_SIZE(FlagListBox::OnSize)
EVT_LISTBOX_DCLICK(ID_FLAGLISTBOX, FlagListBox::OnDoubleClickFlag)
END_EVENT_TABLE()

bool FlagListBox::SetFlagSet(const wxString& setToFind) {
	wxASSERT(!setToFind.IsEmpty());
	wxCHECK_MSG(this->flagData != NULL, false,
		_T("SetFlagSet() called when flagData was null."));
	
	const FlagSet* flagSet = this->flagData->GetFlagSet(setToFind); 
	
	if ( flagSet == NULL ) {
		return false;
	}

	wxArrayString::const_iterator disableIter =
		flagSet->flagsToDisable.begin();
	while ( disableIter != flagSet->flagsToDisable.end() ) {
		this->SetFlag(*disableIter, false);
		disableIter++;
	}
	wxArrayString::const_iterator enableIter =
		flagSet->flagsToEnable.begin();
	while ( enableIter != flagSet->flagsToEnable.end() ) {
		this->SetFlag(*enableIter, true);
		enableIter++;
	}
	return true;
}

void FlagListBox::GetFlagSets(wxArrayString& arr) const {
	wxASSERT(arr.IsEmpty());
	wxCHECK_RET(this->flagData != NULL,
		_T("GetFlagSets() called when flagData was null."));
	
	this->flagData->GetFlagSetNames(arr);
}

/** sets all flags off. */
void FlagListBox::ResetFlags() {
	wxCHECK_RET(this->IsReady(),
		_T("ResetFlags() called when flag list box isn't ready."));
	
	FlagListCheckBoxItems::iterator it = this->checkBoxes.begin();
	while (it != this->checkBoxes.end()) {
		FlagListCheckBoxItem* item = *it;
		if (item->GetCheckBox() != NULL) {
			item->GetCheckBox()->SetValue(false);
		}
		++it;
	}
}
