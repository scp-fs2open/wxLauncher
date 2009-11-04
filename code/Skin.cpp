#include <wx/wx.h>
#include <wx/filename.h>
#include "skin.h"

#include "wxLauncherSetup.h" // Last include for memory debugging

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

Skin::~Skin() {
	if (this->windowTitle != NULL) delete this->windowTitle;
	if (this->windowIcon != NULL) delete this->windowIcon;
	if (this->welcomeHeader != NULL) delete this->welcomeHeader;
	if (this->welcomeIcon != NULL) delete this->welcomeIcon;
	if (this->modsIcon != NULL) delete this->modsIcon;
	if (this->basicIcon != NULL) delete this->basicIcon;
	if (this->advancedIcon != NULL) delete this->advancedIcon;
	if (this->installIcon != NULL) delete this->installIcon;
	if (this->idealIcon != NULL) delete this->idealIcon;
	if (this->baseFont != NULL) delete this->baseFont;
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
		this->defaultSkin->welcomeHeader = 
			new wxBitmap(_("SCP Header.png"), wxBITMAP_TYPE_PNG);
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

SkinSystem::~SkinSystem() {
	if (this->defaultSkin != NULL) delete this->defaultSkin;
	if (this->TCSkin != NULL) delete this->TCSkin;
	if (this->modSkin != NULL) delete this->modSkin;
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
		wxLogFatalError(_T("Cannot retrive a window title. (0x%h, 0x%h, 0x%h)"),
			this->modSkin, this->TCSkin, this->defaultSkin);
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
		wxLogFatalError(_T("Cannot retrive a welcome icon. (0x%h, 0x%h, 0x%h)"),
			this->modSkin, this->TCSkin, this->defaultSkin);
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
		wxLogFatalError(_T("Cannot retrive a mods icon. (0x%h, 0x%h, 0x%h)"),
			this->modSkin, this->TCSkin, this->defaultSkin);
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
		wxLogFatalError(_T("Cannot retrive a basic icon. (0x%h, 0x%h, 0x%h)"),
			this->modSkin, this->TCSkin, this->defaultSkin);
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
		wxLogFatalError(_T("Cannot retrive a advanced icon. (0x%h, 0x%h, 0x%h)"),
			this->modSkin, this->TCSkin, this->defaultSkin);
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
		wxLogFatalError(_T("Cannot retrive a install icon. (0x%h, 0x%h, 0x%h)"),
			this->modSkin, this->TCSkin, this->defaultSkin);
		return wxNullBitmap;
	}
}

wxBitmap SkinSystem::GetBanner() {
	if ( this->modSkin != NULL
		&& this->modSkin->welcomeHeader != NULL ) {
			return *(this->modSkin->welcomeHeader);
	} else if ( this->TCSkin != NULL
		&& this->TCSkin->welcomeHeader != NULL ) {
			return *(this->TCSkin->welcomeHeader);
	} else if ( this->defaultSkin != NULL
		&& this->defaultSkin->welcomeHeader != NULL ) {
			return *(this->defaultSkin->welcomeHeader);
	} else {
		wxLogFatalError(_T("Cannot retrive a install icon. (0x%h, 0x%h, 0x%h)"),
			this->modSkin, this->TCSkin, this->defaultSkin);
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
		wxLogFatalError(_T("Cannot retrive a install icon. (0x%h, 0x%h, 0x%h)"),
			this->modSkin, this->TCSkin, this->defaultSkin);
		return wxNullFont;
	}
}

/** Opens, verifies and resizes (if nessisary) the 255x112 image that is needed
on the mods page. 
\note Does allocate memory.*/
wxBitmap* SkinSystem::VerifySmallImage(wxString current, wxString shortmodname,
									   wxString filepath) {
	wxFileName filename;
	if ( SkinSystem::SearchFile(&filename, current, shortmodname, filepath) ) {
		wxLogDebug(_T("   Opening: %s"), filename.GetFullPath());
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
	}
	return NULL;
}

/** Opens, verifies, the window icon, returning NULL if anything is not valid.
\note Does allocate memory. */
wxBitmap* SkinSystem::VerifyWindowIcon(wxString current, wxString shortmodname,
									   wxString filepath) {
   wxFileName filename;
   if ( SkinSystem::SearchFile(&filename, current, shortmodname, filepath) ) {
	   wxLogDebug(_T("   Opening: %s"), filename.GetFullPath());

	   wxImage image(filename.GetFullPath());
	   if ( image.IsOk() ) {
		   if ( image.GetWidth() == 32 && image.GetHeight() == 32 ) {
			   return new wxBitmap(image);
		   } else {
			   wxLogDebug(_T("   Image size wrong"));
		   }
	   } else {
		   wxLogDebug(_T("   Image not valid."));
	   }
   }
   return NULL;
}

/** Returns true is function is able to get a valid filename object for the
passed paths.  Filename is returned via the param filename. */
bool SkinSystem::SearchFile(wxFileName *filename, wxString currentTC,
							wxString shortmodname, wxString filepath) {
	filename->Assign(
		wxString::Format(_T("%s/%s"), shortmodname, filepath));
	if ( filename->Normalize(wxPATH_NORM_ALL, currentTC, wxPATH_UNIX) ) {
		if ( filename->IsOk() ) {
			if ( filename->FileExists() ) {
				return true;
			} else {
				wxLogDebug(_T("   File '%s' does not exist"),
					filename->GetFullPath());
			}
		} else {
			wxLogDebug(_T("   File '%s' is not valid"), filename->GetFullPath());
		}
	} else {
		wxLogDebug(_T("   Unable to normalize '%s' '%s' '%s'"),	currentTC,
			shortmodname, filepath);
	}
	return false;
}

/** Verifies that the icon exists, is the correct size (resizing if needed)
and then returns a new wxBitmap that contains the file. */
wxBitmap* SkinSystem::VerifyTabIcon(wxString currentTC, wxString shortmodname,
									wxString filepath) {
	wxFileName filename;
	if ( SkinSystem::SearchFile(&filename, currentTC, shortmodname, filepath) ) {
		wxLogDebug(_T("   Opening: %s"), filename.GetFullPath());

		wxImage image(filename.GetFullPath());
		
		if ( image.IsOk() ) {
			if ( image.GetWidth() != SkinSystem::TabIconWidth
				|| image.GetHeight() != SkinSystem::TabIconHeight ) {
					wxLogDebug(_T("   Resizing image from %dx%d"), 
						image.GetWidth(), image.GetHeight());

					image = image.Scale(
						SkinSystem::TabIconWidth,
						SkinSystem::TabIconHeight,
						wxIMAGE_QUALITY_HIGH);
			}
			return new wxBitmap(image);
		} else {
			wxLogDebug(_T("   Image not valid!"));
		}
	}
	return NULL;
}

/** Verifies that the ideal icon exists and is the correct size. Returns a
new wxBitmap allocated on the heap, otherwise returns NULL if any errors.*/
wxBitmap* SkinSystem::VerifyIdealIcon(wxString currentTC, wxString shortname,
									  wxString filepath) {
	  wxFileName filename;
	  if ( SkinSystem::SearchFile(&filename, currentTC, shortname, filepath) ) {
		  wxLogDebug(_T("   Opening: %s"), filename.GetFullPath());

		  wxImage image(filename.GetFullPath());

		  if ( image.IsOk() ) {
			  if ( image.GetWidth() == SkinSystem::IdealIconWidth 
				  && image.GetHeight() == SkinSystem::IdealIconHeight ) { 
					  return new wxBitmap(image);
			  } else {
				  wxLogDebug(_T("   Icon is incorrect size. Got (%d,%d); Need (%d,%d)"),
					  image.GetWidth(), image.GetHeight(),
					  SkinSystem::IdealIconWidth, SkinSystem::IdealIconHeight);
			  }
		  } else {
			  wxLogDebug(_T("   Icon is not valid."));
		  }
	  }
	  return NULL;
}

/** Returns a valid font object based on the font name and/or size passed in. */
wxFont* SkinSystem::VerifyFontChoice(wxString currentTC, wxString shortmodname,
									 wxString fontname, int fontsize,
									 wxString fontFamilyStr, wxString fontStyleStr,
									 wxString fontWeightStr, bool underline) {
	 WXUNUSED(currentTC);
	 WXUNUSED(shortmodname);
	 
	// interpret the fontfamily string

	 wxFontFamily fontfamily = wxFONTFAMILY_UNKNOWN;
	 fontFamilyStr.MakeLower();
	 if ( fontFamilyStr.StartsWith(_T("decorative")) ) {
		 fontfamily = wxFONTFAMILY_DECORATIVE;
	 } else if ( fontFamilyStr.StartsWith(_T("roman")) ) {
		 fontfamily = wxFONTFAMILY_ROMAN;
	 } else if ( fontFamilyStr.StartsWith(_T("script")) ) {
		 fontfamily = wxFONTFAMILY_SCRIPT;
	 } else if ( fontFamilyStr.StartsWith(_T("swiss")) ) {
		 fontfamily = wxFONTFAMILY_SWISS;
	 } else if ( fontFamilyStr.StartsWith(_T("modern")) ) {
		 fontfamily = wxFONTFAMILY_MODERN;
	 } else if ( fontFamilyStr.StartsWith(_T("teletype")) ) {
		 fontfamily = wxFONTFAMILY_TELETYPE;
	 }

	 wxFontStyle fontstyle = wxFONTSTYLE_MAX;
	 fontStyleStr.MakeLower();
	 if ( fontStyleStr.StartsWith(_T("slant")) ) {
		fontstyle = wxFONTSTYLE_SLANT;
	 } else if ( fontStyleStr.StartsWith(_T("italic")) ) {
		 fontstyle = wxFONTSTYLE_ITALIC;
	 }

	 wxFontWeight fontweight = wxFONTWEIGHT_MAX;
	 fontStyleStr.MakeLower();
	 if ( fontStyleStr.StartsWith(_T("bold")) ) {
		 fontweight = wxFONTWEIGHT_BOLD;
	 } else if ( fontStyleStr.StartsWith(_T("light")) ) {
		 fontweight = wxFONTWEIGHT_LIGHT;
	 }

	 wxFont font(
		 (fontsize < 0) ? 12 : fontsize,	// font size
		 (fontfamily == wxFONTFAMILY_UNKNOWN) ? wxFONTFAMILY_DEFAULT : fontfamily,
		 (fontstyle == wxFONTSTYLE_MAX) ? wxFONTSTYLE_NORMAL : fontstyle,
		 (fontweight == wxFONTWEIGHT_MAX) ? wxFONTWEIGHT_NORMAL : fontweight,
		 underline,
		 fontname);

	 if ( font.IsOk() ) {
		 return new wxFont(font);
	 } else {
		 return NULL;
	 }
}


