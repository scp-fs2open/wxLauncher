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

#ifndef FLAGFILEDATA_H
#define FLAGFILEDATA_H

class FlagListCheckBox: public wxCheckBox {
public:
	FlagListCheckBox(wxWindow* parent,
		const wxString& label,
		const wxString& flagString,
		int flagIndex);
	void OnClicked(wxCommandEvent &event);
private:
	wxString flagString;
	int flagIndex; // index is needed so that proxy can keep flags ordered in flag list order
};

class Flag {
public:
	Flag();
	wxString flagString;
	wxString shortDescription;
	wxString fsoCatagory;
	wxString webURL;
	bool isRecomendedFlag;
	wxUint32 easyEnable;
	wxUint32 easyDisable;
	FlagListCheckBox* checkbox;
	wxSizer* checkboxSizer;
	
	int GetFlagIndex() const { return this->flagIndex; }
private:
	int flagIndex; // private because the proxy depends on it being correct, so nothing should mess it up
	static int flagIndexCounter;
};

WX_DECLARE_LIST(Flag, FlagList);

/** Contains all of the flags in a category. */
class FlagCategory {
public:
	wxString categoryName;
	FlagList flags;
};

WX_DECLARE_LIST(FlagCategory, FlagCategoryList);

class FlagSet {
public:
	FlagSet(wxString name);
	wxString name;
	wxArrayString flagsToEnable;
	wxArrayString flagsToDisable;
};

WX_DECLARE_LIST(FlagSet, FlagSetsList);

/** Flag data needed by the profile proxy. */
class ProxyFlagDataItem {
public:
	ProxyFlagDataItem(const wxString& flagString, int flagIndex);
	const wxString& GetFlagString() const { return flagString; }
	int getFlagIndex() const { return flagIndex; }
private:
	wxString flagString;
	int flagIndex;
};

WX_DECLARE_LIST(ProxyFlagDataItem, ProxyFlagData);

/** Flag data needed by the flag list box. */
class FlagListBoxDataItem {
public:
	FlagListBoxDataItem(const wxString& fsoCategory);
	FlagListBoxDataItem(const wxString& shortDescription,
		const wxString& flagString, int flagIndex, bool isRecommendedFlag);
	wxString fsoCategory;
	wxString shortDescription;
	wxString flagString;
	bool isRecommendedFlag;
	int GetFlagIndex() const { return flagIndex; }
private:
	FlagListBoxDataItem();
	int flagIndex;
};

WX_DECLARE_LIST(FlagListBoxDataItem, FlagListBoxData);

/** The data extracted from the flag file. */
class FlagFileData {
public:
	FlagFileData();
	~FlagFileData();
	
	/** Adds the name of an "easy setup" flag set. */
	void AddEasyFlag(const wxString& easyFlag);
	
	void AddFlag(Flag* flag);
	
	/** Generates the "easy setup" flag sets.
	 This function requires that at least one "easy setup" name has been added.
	 Until support for the new mod.ini is added, this function should be called exactly once. */
	void GenerateFlagSets();
	
	/** Generates flag checkboxes for display. verticalOffset is the distance in pixels 
	 between the top of the checkbox and the top of the flag's entry in the FlagListBox entry. */
	void GenerateCheckBoxes(wxWindow* parent, const int verticalOffset);
	
	/** Creates a version of the data suitable for use by the profile proxy. */
	ProxyFlagData* GenerateProxyFlagData() const;
	
	/** Creates a version of the data suitable for use by the flag list box. */
	FlagListBoxData* GenerateFlagListBoxData() const;
	
	/** Returns the total number of flags and flag category headers. */
	size_t GetItemCount() const;
	
	/** Returns a FlagSet, given its name. Returns NULL if not found. */
	const FlagSet* GetFlagSet(const wxString& flagSetName) const;
	
	/** Stores the names of the flag sets in the passed-in array. */
	void GetFlagSetNames(wxArrayString& arr) const;
	
	/** Gets the nth flag's webURL (if it has one). */
	const wxString* GetWebURL(int n) const;
	
	// TODO move to private when no longer needed by non-member functions
	FlagCategoryList::iterator begin() { return this->allSupportedFlagsByCategory.begin(); }
	FlagCategoryList::const_iterator begin() const { return this->allSupportedFlagsByCategory.begin(); }
	
	FlagCategoryList::iterator end() { return this->allSupportedFlagsByCategory.end(); }
	FlagCategoryList::const_iterator end() const { return this->allSupportedFlagsByCategory.end(); }
	
private:
	wxArrayString easyFlags;
	FlagSetsList flagSets;
	FlagCategoryList allSupportedFlagsByCategory;
	bool isProxyDataGenerated;
	bool isFlagListBoxDataGenerated;
	bool areCheckBoxesGenerated;
};

#endif
