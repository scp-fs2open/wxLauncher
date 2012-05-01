/*
 Copyright (C) 2009-2012 wxLauncher Team
 
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

#include "datastructures/FlagFileData.h"

#include "global/MemoryDebugging.h"

int Flag::flagIndexCounter = 0;

Flag::Flag()
: flagIndex(flagIndexCounter++) {
}

#include <wx/listimpl.cpp> // Magic Incantation
WX_DEFINE_LIST(FlagList);

#include <wx/listimpl.cpp> // Magic Incantation
WX_DEFINE_LIST(FlagCategoryList);

FlagSet::FlagSet(wxString name)
: name(name) {
}

#include <wx/listimpl.cpp> // Magic Incantation
WX_DEFINE_LIST(FlagSetsList);

ProxyFlagDataItem::ProxyFlagDataItem(const wxString& flagString, int flagIndex)
: flagString(flagString), flagIndex(flagIndex) {
}

#include <wx/listimpl.cpp> // Magic Incantation
WX_DEFINE_LIST(ProxyFlagData);

FlagListBoxDataItem::FlagListBoxDataItem(const wxString& fsoCategory)
: fsoCategory(fsoCategory),
  shortDescription(wxEmptyString),
  flagString(wxEmptyString),
  isRecommendedFlag(false),
  flagIndex(-1) {
	wxASSERT(!fsoCategory.IsEmpty());
}

FlagListBoxDataItem::FlagListBoxDataItem(const wxString& shortDescription,
	const wxString& flagString, int flagIndex, bool isRecommendedFlag)
: fsoCategory(wxEmptyString),
  shortDescription(shortDescription),
  flagString(flagString),
  isRecommendedFlag(isRecommendedFlag),
  flagIndex(flagIndex) {
	// shortDescription can be empty
	wxASSERT(!flagString.IsEmpty());
	wxASSERT(flagIndex >= 0);
}

#include <wx/listimpl.cpp> // Magic Incantation
WX_DEFINE_LIST(FlagListBoxData);

FlagFileData::FlagFileData()
: isProxyDataGenerated(false),
  isFlagListBoxDataGenerated(false) {
}

FlagFileData::~FlagFileData() {
	for ( FlagCategoryList::iterator catIter = this->begin(); catIter != this->end(); catIter++ ) {
		FlagCategory* category = *catIter;
		for ( FlagList::iterator iter = category->flags.begin(); iter != category->flags.end(); iter++ ) {
			Flag *flag = *iter;
			delete flag;
		}
		category->flags.Clear();
		
		delete category;
	}
	this->allSupportedFlagsByCategory.Clear();
	
	FlagSetsList::iterator flagSetIter = this->flagSets.begin();
	while ( flagSetIter != this->flagSets.end() ) {
		delete *flagSetIter;
		flagSetIter++;
	}
	this->flagSets.clear();
}

void FlagFileData::AddEasyFlag(const wxString& easyFlag) {
	wxASSERT(!easyFlag.IsEmpty());
	wxASSERT_MSG(this->easyFlags.Index(easyFlag.c_str()) == wxNOT_FOUND,
		wxString::Format(_T("attempted to add easy flag '%s' a second time"), easyFlag.c_str()));
	
	this->easyFlags.Add(easyFlag);
}

void FlagFileData::AddFlag(Flag* flag) {
	wxASSERT(flag != NULL);
	
	FlagCategoryList::iterator iter;
	for (iter = this->begin(); iter != this->end(); iter++ ) {
		if ( flag->fsoCatagory == (*iter)->categoryName ) {
			break;
		}
	}
	if ( iter == this->end() ) {
		// did not find the category, so add it
		FlagCategory* flagCat = new FlagCategory();
		flagCat->categoryName = flag->fsoCatagory;
		
		Flag* headFlag = new Flag();
		headFlag->fsoCatagory = flag->fsoCatagory;
		headFlag->isRecomendedFlag = false;
		flagCat->flags.Append(headFlag);
		flagCat->flags.Append(flag);
		this->allSupportedFlagsByCategory.Append(flagCat);
	} else {
		(*iter)->flags.Append(flag);
	}
}

void FlagFileData::GenerateFlagSets() {
	wxASSERT_MSG(!this->easyFlags.IsEmpty(),
		_T("GenerateFlagSets() called when there are no easy flag categories."));
	// GenerateFlagSets should be run exactly once, at least until the new mod.ini support is working
	wxASSERT_MSG(this->flagSets.IsEmpty(),
		_T("GenerateFlagSets() called when there already are flag sets."));
	
	// \todo include the flag sets of the mod.inis as well
	
	// custom
	this->flagSets.Append(new FlagSet(_("Custom")));
	
	// the easy flags.
	wxUint32 counter = 0;
	wxArrayString::const_iterator easyIter = this->easyFlags.begin();
	while ( easyIter != this->easyFlags.end() ) {
		wxString easyFlag = *easyIter;
		
		if ( easyFlag.StartsWith(_T("Custom")) ) {
			// do nothing, we already have a custom
		} else {
			FlagSet* flagSet = new FlagSet(easyFlag);
			for (FlagCategoryList::const_iterator catIter = this->begin(); catIter != this->end(); catIter++) {
				for (FlagList::const_iterator flagIter = (*catIter)->flags.begin();
					 flagIter != (*catIter)->flags.end(); flagIter++) {
					
					Flag* flag = *flagIter;
					
					if ( !flag->flagString.IsEmpty()
						&& (flag->easyEnable & counter) > 0 ) {
						flagSet->flagsToEnable.Add(flag->flagString);
					}
					if ( !flag->flagString.IsEmpty()
						&& (flag->easyDisable & counter) > 0 ) {
						flagSet->flagsToDisable.Add(flag->flagString);
					}
				}
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
			easyIter = this->easyFlags.end();
			wxLogError(_T("FreeSpace 2 Open executable has more than 31 easy flag categories"));
		} else {
			easyIter++;
		}
	}
}

ProxyFlagData* FlagFileData::GenerateProxyFlagData() const {
	wxASSERT(!this->allSupportedFlagsByCategory.IsEmpty());
	wxASSERT_MSG(!this->isProxyDataGenerated,
		_T("Attempted to generate proxy data twice.")); // should never need to generate proxy data twice
	
	ProxyFlagData* proxyData = new ProxyFlagData();
	
	for (FlagCategoryList::const_iterator catIter = this->begin(); catIter != this->end(); catIter++) {
		for (FlagList::const_iterator flagIter = (*catIter)->flags.begin(); flagIter != (*catIter)->flags.end(); flagIter++) {
			Flag* flag = *flagIter;
			
			proxyData->Append(new ProxyFlagDataItem(flag->flagString, flag->GetFlagIndex()));
		}
	}
	
	// keep const in the function prototype to avoid corrupting data, but allow for making this one change
	const_cast<FlagFileData*>(this)->isProxyDataGenerated = true;
	
	return proxyData;
}

FlagListBoxData* FlagFileData::GenerateFlagListBoxData() const {
	wxASSERT(!this->allSupportedFlagsByCategory.IsEmpty());
	wxASSERT_MSG(!this->isFlagListBoxDataGenerated,
				 _T("Attempted to generate flag list box data twice."));
	
	FlagListBoxData* flagListBoxData = new FlagListBoxData();
	
	for (FlagCategoryList::const_iterator catIter = this->begin();
		 catIter != this->end();
		 catIter++) {
		for (FlagList::const_iterator flagIter = (*catIter)->flags.begin();
			 flagIter != (*catIter)->flags.end();
			 flagIter++) {
			Flag* flag = *flagIter;
			
			if (flag->flagString.IsEmpty()) {
				flagListBoxData->Append(
					new FlagListBoxDataItem(flag->fsoCatagory));
			} else {
				flagListBoxData->Append(
					new FlagListBoxDataItem(
						flag->shortDescription,
						flag->flagString,
						flag->GetFlagIndex(),
						flag->isRecomendedFlag));
			}
		}
	}
	
	// keep const in the function prototype to avoid corrupting data,
	// but allow for making this one change
	const_cast<FlagFileData*>(this)->isFlagListBoxDataGenerated = true;
	
	return flagListBoxData;
}

size_t FlagFileData::GetItemCount() const {
	size_t itemCount = 0;
	
	for (FlagCategoryList::const_iterator iter = this->begin(); iter != this->end(); iter++) {
		itemCount += (*iter)->flags.GetCount();
	}
	
	return itemCount;
}

const FlagSet* FlagFileData::GetFlagSet(const wxString& flagSetName) const {
	wxCHECK_MSG(!this->flagSets.IsEmpty(), NULL,
		wxString::Format(
			_T("Attempted to set flag set '%s' when there are no flag sets."),
			flagSetName.c_str()));
	// TODO once new mod.ini supported, may need to rethink this assert,
	//      and possibly also regenerate flag sets
	
	FlagSetsList::const_iterator flagSetsIter = this->flagSets.begin();
	while(flagSetsIter != this->flagSets.end()) {
		if ( (*flagSetsIter)->name.StartsWith(flagSetName) ) {
			return *flagSetsIter;
		}
		flagSetsIter++;
	}
	wxLogWarning(_T("GetFlagSet(): could not find set %s"),
		flagSetName.c_str());
	return NULL;
}

void FlagFileData::GetFlagSetNames(wxArrayString& arr) const {
	wxCHECK_RET(!this->flagSets.IsEmpty(),
		_T("Attempted to get flag sets when there are none."));
	// TODO once new mod.ini supported, may need to rethink this assert,
	//      and possibly also regenerate flag sets
	
	FlagSetsList::const_iterator flagSetsIter = this->flagSets.begin();
	while ( flagSetsIter != this->flagSets.end() ) {
		arr.Add((*flagSetsIter)->name);
		flagSetsIter++;
	}
}

const wxString* FlagFileData::GetWebURL(const int n) const {
	wxCHECK_MSG(n >= 0 && n < static_cast<int>(this->GetItemCount()),
		NULL,
		wxString::Format(_T("GetWebURL(): given invalid index %d"), n));
	
	int flagIndex = 0;
	
	FlagCategoryList::const_iterator category = this->begin();
	while (category != this->end()) {
		FlagList::const_iterator flag = (*category)->flags.begin();
		while( flag != (*category)->flags.end() ) {
			if (flagIndex == n) {
				return &((*flag)->webURL);
			}
			flag++;
			flagIndex++;
		}
		category++;
	}
	
	wxFAIL_MSG(_T("GetWebURL(): should never reach here!"));
	return NULL;
}
