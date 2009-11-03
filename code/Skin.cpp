#include <wx/wx.h>
#include <wx/filename.h>
#include "skin.h"

Skin::Skin() {
	this->windowTitle = NULL;
	this->windowIcon = NULL;
	this->welcomeHeader = NULL;
	this->welcomeIcon = NULL;
	this->modsIcon = NULL;
	this->basicIcon = NULL;
	this->advancedIcon = NULL;
	this->installIcon = NULL;
	this->idealIcon = NULL;
	this->baseFont = NULL;
}

SkinSystem::SkinSystem(Skin *defaultSkin) {
	if ( defaultSkin != NULL ) {
		this->defaultSkin = defaultSkin;
	} else {
		this->defaultSkin = new Skin();
	}
	this->TCSkin = NULL;
	this->modSkin = NULL;

	// Verify that the default skin is complete and if not addin anything missing.

	if ( this->defaultSkin->windowTitle == NULL ) {
		this->defaultSkin->windowTitle = 
			new wxString(_("wxLauncher for the FreeSpace Source Code Project"));
	}

	if ( this->defaultSkin->windowIcon == NULL ) {
		wxIcon* temp = new wxIcon(_T("wxLauncher.ico"));
		if (temp->IsOk())
			wxLogFatalError( _("Icon not valid") );

		this->defaultSkin->windowIcon = temp;
	}

	if ( this->defaultSkin->welcomeHeader == NULL ) {
	}

	if ( this->defaultSkin->welcomeIcon == NULL ) {
		this->defaultSkin->welcomeIcon =
			new wxBitmap(_T("welcome.png"), wxBITMAP_TYPE_PNG);
	}
	if ( this->defaultSkin->modsIcon == NULL ) {
		this->defaultSkin->modsIcon =
			new wxBitmap(_T("mods.png"), wxBITMAP_TYPE_PNG);
	}
	if ( this->defaultSkin->basicIcon == NULL ) {
		this->defaultSkin->basicIcon =
			new wxBitmap(_T("basic.png"), wxBITMAP_TYPE_PNG);
	}
	if ( this->defaultSkin->advancedIcon == NULL ) {
		this->defaultSkin->advancedIcon = 
			new wxBitmap(_T("advanced.png"), wxBITMAP_TYPE_PNG);
	}
	if ( this->defaultSkin->installIcon == NULL ) {
		this->defaultSkin->installIcon =
			new wxBitmap(_T("install.png"), wxBITMAP_TYPE_PNG);
	}

	if ( this->defaultSkin->baseFont == NULL ) {
		this->defaultSkin->baseFont =
			wxFont::New(12,
			wxFONTFAMILY_DECORATIVE,
			wxFONTSTYLE_NORMAL,
			wxFONTWEIGHT_BOLD, false);
	}
}

wxString SkinSystem::GetTitle() {
	if ( this->modSkin != NULL
		&& this->modSkin->windowTitle != NULL ) {
			return *(this->modSkin->windowTitle);
	} else if ( this->TCSkin != NULL
		&& this->TCSkin->windowTitle != NULL ) {
			return *(this->TCSkin->windowTitle);
	} else if ( this->defaultSkin != NULL
		&& this->defaultSkin->windowTitle != NULL ) {
			return *(this->defaultSkin->windowTitle);
	} else {
		wxLogFatalError(
			wxString::Format(_T("Cannot retrive a window title. (0x%h, 0x%h, 0x%h)"),
			this->modSkin, this->TCSkin, this->defaultSkin));
		return wxString();
	}
}

wxBitmap SkinSystem::GetWelcomeIcon() {
	if ( this->modSkin != NULL
		&& this->modSkin->welcomeIcon != NULL ) {
			return *(this->modSkin->welcomeIcon);
	} else if ( this->TCSkin != NULL
		&& this->TCSkin->welcomeIcon != NULL ) {
			return *(this->TCSkin->welcomeIcon);
	} else if ( this->defaultSkin != NULL
		&& this->defaultSkin->welcomeIcon != NULL ) {
			return *(this->defaultSkin->welcomeIcon);
	} else {
		wxLogFatalError(
			wxString::Format(_T("Cannot retrive a welcome icon. (0x%h, 0x%h, 0x%h)"),
			this->modSkin, this->TCSkin, this->defaultSkin));
		return wxNullBitmap;
	}
}

wxBitmap SkinSystem::GetModsIcon() {
	if ( this->modSkin != NULL
		&& this->modSkin->modsIcon != NULL ) {
			return *(this->modSkin->modsIcon);
	} else if ( this->TCSkin != NULL
		&& this->TCSkin->modsIcon != NULL ) {
			return *(this->TCSkin->modsIcon);
	} else if ( this->defaultSkin != NULL
		&& this->defaultSkin->modsIcon != NULL ) {
			return *(this->defaultSkin->modsIcon);
	} else {
		wxLogFatalError(
			wxString::Format(_T("Cannot retrive a mods icon. (0x%h, 0x%h, 0x%h)"),
			this->modSkin, this->TCSkin, this->defaultSkin));
		return wxNullBitmap;
	}
}

wxBitmap SkinSystem::GetBasicIcon() {
	if ( this->modSkin != NULL
		&& this->modSkin->basicIcon != NULL ) {
			return *(this->modSkin->basicIcon);
	} else if ( this->TCSkin != NULL
		&& this->TCSkin->basicIcon != NULL ) {
			return *(this->TCSkin->basicIcon);
	} else if ( this->defaultSkin != NULL
		&& this->defaultSkin->basicIcon != NULL ) {
			return *(this->defaultSkin->basicIcon);
	} else {
		wxLogFatalError(
			wxString::Format(_T("Cannot retrive a basic icon. (0x%h, 0x%h, 0x%h)"),
			this->modSkin, this->TCSkin, this->defaultSkin));
		return wxNullBitmap;
	}
}

wxBitmap SkinSystem::GetAdvancedIcon() {
	if ( this->modSkin != NULL
		&& this->modSkin->advancedIcon != NULL ) {
			return *(this->modSkin->advancedIcon);
	} else if ( this->TCSkin != NULL
		&& this->TCSkin->advancedIcon != NULL ) {
			return *(this->TCSkin->advancedIcon);
	} else if ( this->defaultSkin != NULL
		&& this->defaultSkin->advancedIcon != NULL ) {
			return *(this->defaultSkin->advancedIcon);
	} else {
		wxLogFatalError(
			wxString::Format(_T("Cannot retrive a advanced icon. (0x%h, 0x%h, 0x%h)"),
			this->modSkin, this->TCSkin, this->defaultSkin));
		return wxNullBitmap;
	}
}

wxBitmap SkinSystem::GetInstallIcon() {
	if ( this->modSkin != NULL
		&& this->modSkin->installIcon != NULL ) {
			return *(this->modSkin->installIcon);
	} else if ( this->TCSkin != NULL
		&& this->TCSkin->installIcon != NULL ) {
			return *(this->TCSkin->installIcon);
	} else if ( this->defaultSkin != NULL
		&& this->defaultSkin->installIcon != NULL ) {
			return *(this->defaultSkin->installIcon);
	} else {
		wxLogFatalError(
			wxString::Format(_T("Cannot retrive a install icon. (0x%h, 0x%h, 0x%h)"),
			this->modSkin, this->TCSkin, this->defaultSkin));
		return wxNullBitmap;
	}
}

wxFont SkinSystem::GetFont() {
	if ( this->modSkin != NULL
		&& this->modSkin->baseFont != NULL ) {
			return *(this->modSkin->baseFont);
	} else if ( this->TCSkin != NULL
		&& this->TCSkin->baseFont != NULL ) {
			return *(this->TCSkin->baseFont);
	} else if ( this->defaultSkin != NULL
		&& this->defaultSkin->baseFont != NULL ) {
			return *(this->defaultSkin->baseFont);
	} else {
		wxLogFatalError(
			wxString::Format(_T("Cannot retrive a install icon. (0x%h, 0x%h, 0x%h)"),
			this->modSkin, this->TCSkin, this->defaultSkin));
		return wxNullFont;
	}
}

/** Opens, verifies and resizes (if nessicary) the 255x112 image that is needed
on the mods page. */
wxBitmap* SkinSystem::VerifySmallImage(wxString current, wxString shortmodname,
									   wxString filepath) {
	wxFileName filename(
		wxString::Format(_T("%s/%s"), shortmodname, filepath));
	if ( filename.Normalize(wxPATH_NORM_ALL, current, wxPATH_UNIX) ) {
		if ( filename.IsOk() && filename.FileExists() ) {
			wxLogDebug(wxString::Format(_T("   Opening: %s"), filename.GetFullPath()));
			wxImage image(filename.GetFullPath());
			if ( image.IsOk() ) {
				if ( image.GetWidth() > 255 || image.GetHeight() > 112 ) {
					wxLogDebug(_T("   Resizing."));
					image = image.Scale(255, 112, wxIMAGE_QUALITY_HIGH);
				}
				return new wxBitmap(image);
			} else {
				wxLogDebug(_T("   Image is not Ok!"));
			}
		} else {
			wxLogDebug(
				wxString::Format(
				(filename.IsOk()) ? _T("   Image '%s' does not exist!") : _T("   Image '%s' is not valid!"),
				filename.GetFullPath()));
		}
	} else {
		wxLogDebug(
			wxString::Format(_T("   Unable to normalize '%s' '%s' '%s'"),
			current, shortmodname, filepath));
	}
	return NULL;
}