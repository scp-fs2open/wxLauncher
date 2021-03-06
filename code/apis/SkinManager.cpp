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

#include "apis/SkinManager.h"
#include "global/SkinDefaults.h"
#include <wx/filename.h>
#include "generated/configure_launcher.h"

#include "global/MemoryDebugging.h" // Last include for memory debugging

LAUNCHER_DEFINE_EVENT_TYPE(EVT_TC_SKIN_CHANGED);

EventHandlers SkinSystem::TCSkinChangedHandlers;

void SkinSystem::RegisterTCSkinChanged(wxEvtHandler *handler) {
	wxASSERT(SkinSystem::IsInitialized());
	wxASSERT_MSG(TCSkinChangedHandlers.IndexOf(handler) == wxNOT_FOUND,
		wxString::Format(
			_T("RegisterTCSkinChanged(): Handler at %p already registered."),
			handler));
	SkinSystem::TCSkinChangedHandlers.Append(handler);
}

void SkinSystem::UnRegisterTCSkinChanged(wxEvtHandler *handler) {
	wxASSERT(SkinSystem::IsInitialized());
	wxASSERT_MSG(TCSkinChangedHandlers.IndexOf(handler) != wxNOT_FOUND,
		wxString::Format(
			_T("UnRegisterTCSkinChanged(): Handler at %p not registered."),
			handler));
	SkinSystem::TCSkinChangedHandlers.DeleteObject(handler);
}

void SkinSystem::GenerateTCSkinChanged() {
	wxASSERT(SkinSystem::IsInitialized());
	
	wxCommandEvent event(EVT_TC_SKIN_CHANGED, wxID_NONE);
	
	wxLogDebug(_T("Generating EVT_TC_SKIN_CHANGED event"));
	for (EventHandlers::iterator
		 iter = SkinSystem::TCSkinChangedHandlers.begin(),
		 end = SkinSystem::TCSkinChangedHandlers.end();
		 iter != end; ++iter) {
		wxEvtHandler* current = *iter;
		current->AddPendingEvent(event);
		wxLogDebug(_T(" Sent EVT_TC_SKIN_CHANGED event to %p"), current);
	}
}


Skin::Skin()
: newsSource(NULL) {
}

bool Skin::SetWindowTitle(const wxString& windowTitle) {
	if (windowTitle.IsEmpty()) {
		wxLogWarning(_T("Provided window title is empty."));
		return false;
	} else {
		this->windowTitle = windowTitle;
		return true;
	}
}

bool Skin::SetWindowIcon(const wxIcon& windowIcon) {
	if (!windowIcon.IsOk()) {
		wxLogWarning(_T("Provided window icon is not valid."));
		return false;
	} else {
		this->windowIcon = windowIcon;
		return true;
	}
}

bool Skin::SetBanner(const wxBitmap& banner) {
	if (!banner.IsOk()) {
		wxLogWarning(_T("Provided banner is not valid."));
		return false;
	} else if (banner.GetWidth() > SkinSystem::BannerMaxWidth) {
		wxLogWarning(_T("Provided banner is too wide (%d vs. %d pixels)."),
			banner.GetWidth(), SkinSystem::BannerMaxWidth);
		return false;
	} else if (banner.GetHeight() != SkinSystem::BannerHeight) {
		wxLogWarning(_T("Provided banner is incorrect height (%d vs. %d pixels)."),
			banner.GetHeight(), SkinSystem::BannerHeight);
		return false;
	} else {
		this->banner = banner;
		return true;
	}
}

bool Skin::SetWelcomeText(const wxString& welcomeText) {
	if (welcomeText.IsEmpty()) {
		wxLogWarning(_T("Provided welcome text is empty."));
		return false;
	} else {
		this->welcomeText = welcomeText;
		return true;
	}
}

bool Skin::SetModImage(const wxBitmap& modImage) {
	if (!modImage.IsOk()) {
		wxLogWarning(_T("Provided mod image is not valid."));
		return false;
	} else if ((modImage.GetWidth() > SkinSystem::ModInfoDialogImageWidth) ||
			   (modImage.GetHeight() > SkinSystem::ModInfoDialogImageHeight)) {
		wxLogDebug(_T("Provided mod image size %dx%d is larger than expected size %dx%d. Resizing."),
			modImage.GetWidth(), modImage.GetHeight(),
			SkinSystem::ModInfoDialogImageWidth, SkinSystem::ModInfoDialogImageHeight);
		
		wxImage tempModImage(modImage.ConvertToImage());
		wxImage scaledTempModImage(
			tempModImage.Scale(
				SkinSystem::ModListImageWidth,
				SkinSystem::ModListImageHeight,
				wxIMAGE_QUALITY_HIGH));
		
		wxBitmap newModImage(scaledTempModImage);
		wxASSERT(newModImage.GetWidth() == SkinSystem::ModListImageWidth);
		wxASSERT(newModImage.GetHeight() == SkinSystem::ModListImageHeight);
		
		this->modImage = wxBitmap(newModImage);
		return true;
	} else if ((modImage.GetWidth() < SkinSystem::ModInfoDialogImageWidth) ||
			   (modImage.GetHeight() < SkinSystem::ModInfoDialogImageHeight)) {
		wxLogDebug(_T("Provided mod image size %dx%d is smaller than expected size %dx%d. Using as is."),
			modImage.GetWidth(), modImage.GetHeight(),
			SkinSystem::ModInfoDialogImageWidth, SkinSystem::ModInfoDialogImageHeight);
		return true;
	} else {
		this->modImage = modImage;
		return true;
	}
}

bool Skin::SetSmallModImage(const wxBitmap& smallModImage) {
	if (!smallModImage.IsOk()) {
		wxLogWarning(_T("Provided small mod image is not valid."));
		return false;
	} else if ((smallModImage.GetWidth() != SkinSystem::ModListImageWidth) ||
			   (smallModImage.GetHeight() != SkinSystem::ModListImageHeight)) {
		wxLogWarning(_T("Provided small mod image size %dx%d is not expected size %dx%d."),
			smallModImage.GetWidth(), smallModImage.GetHeight(),
			SkinSystem::ModListImageWidth, SkinSystem::ModListImageHeight);
		return false;
	} else {
		this->smallModImage = smallModImage;
		return true;
	}
}

bool Skin::SetOkIcon(const wxBitmap& okIcon) {
	if (!okIcon.IsOk()) {
		wxLogWarning(_T("Provided ok icon is not valid."));
		return false;
	} else if (!CheckStatusBarIconDimensions(okIcon)) {
		wxLogWarning(_T("Provided ok icon size is wrong."));
		return false;
	} else {
		this->okIcon = okIcon;
		return true;
	}
}

bool Skin::SetWarningIcon(const wxBitmap& warningIcon) {
	if (!warningIcon.IsOk()) {
		wxLogWarning(_T("Provided warning icon is not valid."));
		return false;
	} else if (!CheckStatusBarIconDimensions(warningIcon)) {
		wxLogWarning(_T("Provided warning icon size is wrong."));
		return false;
	} else {
		this->warningIcon = warningIcon;
		return true;
	}
}

bool Skin::SetBigWarningIcon(const wxBitmap& bigWarningIcon) {
	if (!bigWarningIcon.IsOk()) {
		wxLogWarning(_T("Provided big warning icon is not valid."));
		return false;
	} else if (!CheckBigIconDimensions(bigWarningIcon)) {
		wxLogWarning(_T("Provided big warning icon size is wrong."));
		return false;
	} else {
		this->bigWarningIcon = bigWarningIcon;
		return true;
	}
}

bool Skin::SetErrorIcon(const wxBitmap& errorIcon) {
	if (!errorIcon.IsOk()) {
		wxLogWarning(_T("Provided error icon is not valid."));
		return false;
	} else if (!CheckStatusBarIconDimensions(errorIcon)) {
		wxLogWarning(_T("Provided error icon size is wrong."));
		return false;
	} else {
		this->errorIcon = errorIcon;
		return true;
	}
}

bool Skin::SetInfoIcon(const wxBitmap& infoIcon) {
	if (!infoIcon.IsOk()) {
		wxLogWarning(_T("Provided info icon is not valid."));
		return false;
	} else if (!CheckStatusBarIconDimensions(infoIcon)) {
		wxLogWarning(_T("Provided info icon size is wrong."));
		return false;
	} else {
		this->infoIcon = infoIcon;
		return true;
	}
}

bool Skin::SetBigInfoIcon(const wxBitmap& bigInfoIcon) {
	if (!bigInfoIcon.IsOk()) {
		wxLogWarning(_T("Provided big info icon is not valid."));
		return false;
	} else if (!CheckBigIconDimensions(bigInfoIcon)) {
		wxLogWarning(_T("Provided big info icon size is wrong."));
		return false;
	} else {
		this->bigInfoIcon = bigInfoIcon;
		return true;
	}
}

bool Skin::SetHelpIcon(const wxBitmap& helpIcon) {
	if (!helpIcon.IsOk()) {
		wxLogWarning(_T("Provided help icon is not valid."));
		return false;
	} else if ((helpIcon.GetWidth() != SkinSystem::HelpIconWidth) ||
			   (helpIcon.GetHeight() != SkinSystem::HelpIconHeight)) {
		wxLogWarning(_T("Provided help icon size %dx%d is not expected size %dx%d."),
			helpIcon.GetWidth(), helpIcon.GetHeight(),
			SkinSystem::HelpIconWidth, SkinSystem::HelpIconHeight);
		return false;
	} else {
		this->helpIcon = helpIcon;
		return true;
	}
}

bool Skin::SetBigHelpIcon(const wxBitmap& bigHelpIcon) {
	if (!bigHelpIcon.IsOk()) {
		wxLogWarning(_T("Provided help icon is not valid."));
		return false;
	} else if (!CheckBigIconDimensions(bigHelpIcon)) {
		wxLogWarning(_T("Provided help icon size is wrong."));
		return false;
	} else {
		this->bigHelpIcon = bigHelpIcon;
		return true;
	}
}

bool Skin::SetIdealIcon(const wxBitmap& idealIcon) {
	if (!idealIcon.IsOk()) {
		wxLogWarning(_T("Provided ideal icon is not valid."));
		return false;
	} else if ((idealIcon.GetWidth() != SkinSystem::IdealIconWidth) ||
			   (idealIcon.GetHeight() != SkinSystem::IdealIconHeight)) {
		wxLogWarning(_T("Provided ideal icon size %dx%d is not expected size %dx%d."),
			idealIcon.GetWidth(), idealIcon.GetHeight(),
			SkinSystem::IdealIconWidth, SkinSystem::IdealIconHeight);
		return false;
	} else {
		this->idealIcon = idealIcon;
		return true;
	}
}

bool Skin::SetNewsSource(const NewsSource* newsSource) {
	if (newsSource == NULL) {
		wxLogWarning(_T("Provided news source is NULL."));
		return false;
	} else {
		this->newsSource = newsSource;
		return true;
	}
}

bool Skin::CheckStatusBarIconDimensions(const wxBitmap& icon) {
	const bool result =
		(icon.GetWidth() == SkinSystem::StatusBarIconWidth) &&
		(icon.GetHeight() == SkinSystem::StatusBarIconHeight);
	if (!result) {
		wxLogWarning(_T("Expected status bar icon size is %dx%d but icon size is %dx%d"),
			SkinSystem::StatusBarIconWidth, SkinSystem::StatusBarIconHeight,
			icon.GetWidth(), icon.GetHeight());
	}
	return result;
}

bool Skin::CheckBigIconDimensions(const wxBitmap& icon) {
	const bool result =
		(icon.GetWidth() == SkinSystem::BigIconWidth) &&
		(icon.GetHeight() == SkinSystem::BigIconHeight);
	if (!result) {
		wxLogWarning(_T("Expected big icon size is %dx%d but icon size is %dx%d"),
			SkinSystem::BigIconWidth, SkinSystem::BigIconHeight,
			icon.GetWidth(), icon.GetHeight());
	}
	return result;
}


SkinSystem* SkinSystem::skinSystem = NULL;

bool SkinSystem::Initialize() {
	wxASSERT(!SkinSystem::IsInitialized());
	
	SkinSystem::skinSystem = new SkinSystem();
	return true;
}

void SkinSystem::DeInitialize() {
	wxASSERT(SkinSystem::IsInitialized());
	
	if (!SkinSystem::TCSkinChangedHandlers.IsEmpty()) {
		wxLogDebug(_T("SkinSystem::DeInitialize(): contents of handler list:"));
		for (EventHandlers::const_iterator
			 iter = SkinSystem::TCSkinChangedHandlers.begin(),
			 end = SkinSystem::TCSkinChangedHandlers.end();
			 iter != end; ++iter) {
			wxLogDebug(_T(" handler at %p"), *iter);
		}
		SkinSystem::TCSkinChangedHandlers.Clear();
	}
	
	SkinSystem* temp = SkinSystem::skinSystem;
	SkinSystem::skinSystem = NULL;
	delete temp;
}

bool SkinSystem::IsInitialized() {
	return (SkinSystem::skinSystem != NULL); 
}

SkinSystem* SkinSystem::GetSkinSystem() {
	wxCHECK_MSG(SkinSystem::IsInitialized(),
		NULL,
		_T("Attempt to get skin system when it has not been initialized."));
	
	return SkinSystem::skinSystem;
}

SkinSystem::SkinSystem()
: TCSkin(NULL),
  font(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)),
  messageFont(14, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL) {

	InitializeDefaultSkin();
}

SkinSystem::~SkinSystem() {
	if (this->TCSkin != NULL) delete this->TCSkin;
}

void SkinSystem::InitializeDefaultSkin() {
	// launcher can't function if any of the default skin is missing/invalid
	bool success = false;
	
	success = this->defaultSkin.SetWindowTitle(DEFAULT_SKIN_WINDOW_TITLE);
	if (!success) {
		wxLogFatalError(_T("Setting default window title '%s' failed"),
			DEFAULT_SKIN_WINDOW_TITLE.c_str());
	}
	
	wxFileName filename(_T(RESOURCES_PATH), DEFAULT_SKIN_WINDOW_ICON);
	success = this->defaultSkin.SetWindowIcon(
		wxIcon(filename.GetFullPath(), wxBITMAP_TYPE_ICO));
	if (!success) {
		wxLogFatalError(_T("Setting default window icon '%s' failed"),
			filename.GetFullPath().c_str());
	}
	
	filename = wxFileName(_T(RESOURCES_PATH), DEFAULT_SKIN_BANNER);
	success = this->defaultSkin.SetBanner(
		wxBitmap(filename.GetFullPath(), wxBITMAP_TYPE_ANY));
	if (!success) {
		wxLogFatalError(_T("Setting default banner '%s' failed"),
			filename.GetFullPath().c_str());
	}

	success = this->defaultSkin.SetWelcomeText(DEFAULT_SKIN_WELCOME_TEXT);
	if (!success) {
		wxLogFatalError(_T("Setting default welcome text '%s' failed"),
			DEFAULT_SKIN_WELCOME_TEXT.c_str());
	}
	
	filename = wxFileName(_T(RESOURCES_PATH), DEFAULT_SKIN_MOD_IMAGE_255X112);
	success = this->defaultSkin.SetModImage(
		wxBitmap(filename.GetFullPath(), wxBITMAP_TYPE_ANY));
	if (!success) {
		wxLogFatalError(_T("Setting default mod image '%s' failed"),
			filename.GetFullPath().c_str());
	}
	
	filename = wxFileName(_T(RESOURCES_PATH), DEFAULT_SKIN_MOD_IMAGE_182X80);
	success = this->defaultSkin.SetSmallModImage(
		wxBitmap(filename.GetFullPath(), wxBITMAP_TYPE_ANY));
	if (!success) {
		wxLogFatalError(_T("Setting default small mod image '%s' failed"),
			filename.GetFullPath().c_str());
	}
	
	filename = wxFileName(_T(RESOURCES_PATH), DEFAULT_SKIN_ICON_OK);
	success = this->defaultSkin.SetOkIcon(
		wxBitmap(filename.GetFullPath(), wxBITMAP_TYPE_ANY));
	if (!success) {
		wxLogFatalError(_T("Setting default ok icon '%s' failed"),
			filename.GetFullPath().c_str());
	}
	
	filename = wxFileName(_T(RESOURCES_PATH), DEFAULT_SKIN_ICON_WARNING);
	success = this->defaultSkin.SetWarningIcon(
		wxBitmap(filename.GetFullPath(), wxBITMAP_TYPE_ANY));
	if (!success) {
		wxLogFatalError(_T("Setting default warning icon '%s' failed"),
			filename.GetFullPath().c_str());
	}
	
	filename = wxFileName(_T(RESOURCES_PATH), DEFAULT_SKIN_ICON_WARNING_BIG);
	success = this->defaultSkin.SetBigWarningIcon(
		wxBitmap(filename.GetFullPath(), wxBITMAP_TYPE_ANY));
	if (!success) {
		wxLogFatalError(_T("Setting default big warning icon '%s' failed"),
			filename.GetFullPath().c_str());
	}
	
	filename = wxFileName(_T(RESOURCES_PATH), DEFAULT_SKIN_ICON_ERROR);
	success = this->defaultSkin.SetErrorIcon(
		wxBitmap(filename.GetFullPath(), wxBITMAP_TYPE_ANY));
	if (!success) {
		wxLogFatalError(_T("Setting default error icon '%s' failed"),
			filename.GetFullPath().c_str());
	}
	
	filename = wxFileName(_T(RESOURCES_PATH), DEFAULT_SKIN_ICON_INFO);
	success = this->defaultSkin.SetInfoIcon(
		wxBitmap(filename.GetFullPath(), wxBITMAP_TYPE_ANY));
	if (!success) {
		wxLogFatalError(_T("Setting default info icon '%s' failed"),
			filename.GetFullPath().c_str());
	}
	
	filename = wxFileName(_T(RESOURCES_PATH), DEFAULT_SKIN_ICON_INFO_BIG);
	success = this->defaultSkin.SetBigInfoIcon(
		wxBitmap(filename.GetFullPath(), wxBITMAP_TYPE_ANY));
	if (!success) {
		wxLogFatalError(_T("Setting default big info icon '%s' failed"),
			filename.GetFullPath().c_str());
	}
	
	filename = wxFileName(_T(RESOURCES_PATH), DEFAULT_SKIN_ICON_HELP);
	success = this->defaultSkin.SetHelpIcon(
		wxBitmap(filename.GetFullPath(), wxBITMAP_TYPE_ANY));
	if (!success) {
		wxLogFatalError(_T("Setting default help icon '%s' failed"),
			filename.GetFullPath().c_str());
	}
	
	filename = wxFileName(_T(RESOURCES_PATH), DEFAULT_SKIN_ICON_HELP_BIG);
	success = this->defaultSkin.SetBigHelpIcon(
		wxBitmap(filename.GetFullPath(), wxBITMAP_TYPE_ANY));
	if (!success) {
		wxLogFatalError(_T("Setting default big help icon '%s' failed"),
			filename.GetFullPath().c_str());
	}
	
	filename = wxFileName(_T(RESOURCES_PATH), DEFAULT_SKIN_ICON_IDEAL);
	this->defaultSkin.SetIdealIcon( 
		wxBitmap(filename.GetFullPath(), wxBITMAP_TYPE_ANY));
	if (!success) {
		wxLogFatalError(_T("Setting default ideal icon '%s' failed"),
			filename.GetFullPath().c_str());
	}
	
	success = this->defaultSkin.SetNewsSource(
		NewsSource::FindSource(DEFAULT_SKIN_NEWS_SOURCE));
	if (!success) {
		wxLogFatalError(_T("Setting default news source '%d' failed"),
			DEFAULT_SKIN_NEWS_SOURCE);
	}	
}

const wxString& SkinSystem::GetWindowTitle() const {
	if ( this->TCSkin != NULL
		&& !this->TCSkin->GetWindowTitle().IsEmpty() ) {
			return this->TCSkin->GetWindowTitle();
	} else {
		return this->defaultSkin.GetWindowTitle();
	}
}

const wxIcon& SkinSystem::GetWindowIcon() const {
	if ( this->TCSkin != NULL
		&& this->TCSkin->GetWindowIcon().IsOk() ) {
			return this->TCSkin->GetWindowIcon();
	} else {
		return this->defaultSkin.GetWindowIcon();
	}
}

const wxBitmap& SkinSystem::GetBanner() const {
	if ( this->TCSkin != NULL
		&& this->TCSkin->GetBanner().IsOk() ) {
			return this->TCSkin->GetBanner();
	} else {
		return this->defaultSkin.GetBanner();
	}
}

const wxString& SkinSystem::GetWelcomeText() const {
	if ( this->TCSkin != NULL
		&& !this->TCSkin->GetWelcomeText().IsEmpty() ) {
			return this->TCSkin->GetWelcomeText();
	} else {
		return this->defaultSkin.GetWelcomeText();
	}
}

const wxBitmap& SkinSystem::GetModImage() const {
	if ( this->TCSkin != NULL
		&& this->TCSkin->GetModImage().IsOk() ) {
			return this->TCSkin->GetModImage();
	} else {
		return this->defaultSkin.GetModImage();
	}
}

const wxBitmap& SkinSystem::GetSmallModImage() const {
	if ( this->TCSkin != NULL
		&& this->TCSkin->GetSmallModImage().IsOk() ) {
		return this->TCSkin->GetSmallModImage();
	} else {
		return this->defaultSkin.GetSmallModImage();
	}
}

const wxBitmap& SkinSystem::GetOkIcon() const {
	if ( this->TCSkin != NULL
		&& this->TCSkin->GetOkIcon().IsOk() ) {
			return this->TCSkin->GetOkIcon();
	} else {
		return this->defaultSkin.GetOkIcon();
	}
}

const wxBitmap& SkinSystem::GetWarningIcon() const {
	if ( this->TCSkin != NULL
		&& this->TCSkin->GetWarningIcon().IsOk() ) {
			return this->TCSkin->GetWarningIcon();
	} else {
		return this->defaultSkin.GetWarningIcon();
	}
}

const wxBitmap& SkinSystem::GetBigWarningIcon() const {
	if ( this->TCSkin != NULL
		&& this->TCSkin->GetBigWarningIcon().IsOk() ) {
			return this->TCSkin->GetBigWarningIcon();
	} else {
		return this->defaultSkin.GetBigWarningIcon();
	}
}

const wxBitmap& SkinSystem::GetErrorIcon() const {
	if ( this->TCSkin != NULL
		&& this->TCSkin->GetErrorIcon().IsOk() ) {
			return this->TCSkin->GetErrorIcon();
	} else {
		return this->defaultSkin.GetErrorIcon();
	}
}

const wxBitmap& SkinSystem::GetInfoIcon() const {
	if ( this->TCSkin != NULL
		&& this->TCSkin->GetInfoIcon().IsOk() ) {
			return this->TCSkin->GetInfoIcon();
	} else {
		return this->defaultSkin.GetInfoIcon();
	}
}

const wxBitmap& SkinSystem::GetBigInfoIcon() const {
	if ( this->TCSkin != NULL
		&& this->TCSkin->GetBigInfoIcon().IsOk() ) {
			return this->TCSkin->GetBigInfoIcon();
	} else {
		return this->defaultSkin.GetBigInfoIcon();
	}
}

const wxBitmap& SkinSystem::GetHelpIcon() const {
	if ( this->TCSkin != NULL
		&& this->TCSkin->GetHelpIcon().IsOk() ) {
			return this->TCSkin->GetHelpIcon();
	} else {
		return this->defaultSkin.GetHelpIcon();
	}
}

const wxBitmap& SkinSystem::GetBigHelpIcon() const {
	if ( this->TCSkin != NULL
		&& this->TCSkin->GetBigHelpIcon().IsOk() ) {
			return this->TCSkin->GetBigHelpIcon();
	} else {
		return this->defaultSkin.GetBigHelpIcon();
	}
}

const wxBitmap& SkinSystem::GetIdealIcon() const {
	if ( this->TCSkin != NULL
		&& this->TCSkin->GetIdealIcon().IsOk() ) {
			return this->TCSkin->GetIdealIcon();
	} else {
		return this->defaultSkin.GetIdealIcon();
	}
}

const NewsSource& SkinSystem::GetNewsSource() const {
	if ( this->TCSkin != NULL
		&& this->TCSkin->GetNewsSource() != NULL ) {
		return *this->TCSkin->GetNewsSource();
	} else {
		return *this->defaultSkin.GetNewsSource();
	}
}

void SkinSystem::SetTCSkin(const Skin* skin) {
	wxCHECK_RET(skin != NULL, _T("SetTCSkin() given null Skin"));
	
	// FIXME: Must confirm that generating the event twice isn't problematic
	if (this->TCSkin != NULL) {
		ResetTCSkin();
	}
	
	this->TCSkin = skin;
	
	GenerateTCSkinChanged();
}

// Does nothing if TCSkin is NULL.
void SkinSystem::ResetTCSkin() {
	if (this->TCSkin != NULL) {
		const Skin* temp = this->TCSkin;
		this->TCSkin = NULL;
		delete temp;
		
		GenerateTCSkinChanged();
	}
}

/** Returns true if function is able to get a valid filename object for the
passed paths.  Filename is returned via the param filename. */
bool SkinSystem::SearchFile(wxFileName& filename, wxString currentTC,
							wxString shortmodname, wxString filepath) {
	if (shortmodname.IsEmpty()) { // indicates that the mod is (No mod)
		filename.Assign(
			wxString::Format(_T("%s"), filepath.c_str()));
	} else {
		filename.Assign(
			wxString::Format(_T("%s/%s"), shortmodname.c_str(), filepath.c_str()));
	}

	if ( filename.Normalize(wxPATH_NORM_ALL, currentTC, wxPATH_UNIX) ) {
		if ( filename.IsOk() ) {
			if ( filename.FileExists() ) {
				return true;
			} else {
				wxLogDebug(_T("   File '%s' does not exist"),
					filename.GetFullPath().c_str());
			}
		} else {
			wxLogDebug(_T("   File '%s' is not valid"), filename.GetFullPath().c_str());
		}
	} else {
		wxLogDebug(_T("   Unable to normalize '%s' '%s' '%s'"),	currentTC.c_str(),
			shortmodname.c_str(), filepath.c_str());
	}
	return false;
}

wxBitmap SkinSystem::MakeModListImage(const wxBitmap &orig) {
	wxASSERT(orig.GetWidth() == SkinSystem::ModInfoDialogImageWidth);
	wxASSERT(orig.GetHeight() == SkinSystem::ModInfoDialogImageHeight);
	
	wxImage temp(orig.ConvertToImage());
	wxImage scaledTemp(temp.Scale(SkinSystem::ModListImageWidth,
		SkinSystem::ModListImageHeight,
		wxIMAGE_QUALITY_HIGH));
	
	wxBitmap outimg(scaledTemp);
	wxASSERT(outimg.GetWidth() == SkinSystem::ModListImageWidth);
	wxASSERT(outimg.GetHeight() == SkinSystem::ModListImageHeight);
	
	return outimg;
}

wxBitmap SkinSystem::MakeModInfoDialogImage(const wxBitmap &orig) {
	wxASSERT(orig.GetWidth() == SkinSystem::ModListImageWidth);
	wxASSERT(orig.GetHeight() == SkinSystem::ModListImageHeight);
	
	wxImage temp(orig.ConvertToImage());
	wxImage scaledTemp(temp.Scale(SkinSystem::ModInfoDialogImageWidth,
		SkinSystem::ModInfoDialogImageHeight,
		wxIMAGE_QUALITY_HIGH));
	
	wxBitmap outimg(scaledTemp);
	wxASSERT(outimg.GetWidth() == SkinSystem::ModInfoDialogImageWidth);
	wxASSERT(outimg.GetHeight() == SkinSystem::ModInfoDialogImageHeight);
	
	return outimg;
}
