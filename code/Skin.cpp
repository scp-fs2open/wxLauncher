#include "Skin.h"
#include <wx/artprov.h>
#include <wx/filename.h>
#include "generated/configure_launcher.h"

#include "wxLauncherSetup.h" // Last include for memory debugging

class ArtProvider: public wxArtProvider {
public:
	ArtProvider(SkinSystem *skinSystem);
private:
	SkinSystem *skinSystem;
	virtual wxBitmap CreateBitmap(const wxArtID& id, const wxArtClient& client, const wxSize& size);
};

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
	this->welcomePageText = NULL;
	this->warningIcon = NULL;
	this->bigWarningIcon = NULL;
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
	if (this->welcomePageText != NULL) delete this->welcomePageText;
	if (this->warningIcon != NULL) delete this->warningIcon;
	if (this->bigWarningIcon != NULL) delete this->bigWarningIcon;
}

SkinSystem::SkinSystem(Skin *defaultSkin) {
	if ( defaultSkin != NULL ) {
		this->defaultSkin = defaultSkin;
	} else {
		this->defaultSkin = new Skin();
	}
	this->TCSkin = NULL;
	this->modSkin = NULL;

	wxArtProvider::Push(new ArtProvider(this));

	// Verify that the default skin is complete and if not addin anything missing.

	if ( this->defaultSkin->windowTitle == NULL ) {
		this->defaultSkin->windowTitle = 
			new wxString(_("wxLauncher for the FreeSpace Source Code Project"));
	}

	if ( this->defaultSkin->windowIcon == NULL ) {
		wxFileName filename(_T(RESOURCES_PATH), _T("wxLauncher.ico"));
		wxIcon* temp = new wxIcon(filename.GetFullPath(), wxBITMAP_TYPE_ANY);
		if (temp->IsOk())
			wxLogFatalError( _("Icon not valid") );

		this->defaultSkin->windowIcon = temp;
	}

	if ( this->defaultSkin->welcomeHeader == NULL ) {
		wxFileName filename(_T(RESOURCES_PATH), _T("SCP_Header.png"));
		this->defaultSkin->welcomeHeader = 
			new wxBitmap(filename.GetFullPath(), wxBITMAP_TYPE_ANY);
		wxASSERT(this->defaultSkin->welcomeHeader->IsOk());
	}

	if ( this->defaultSkin->welcomeIcon == NULL ) {
		wxFileName filename(_T(RESOURCES_PATH), _T("welcome.png"));
		this->defaultSkin->welcomeIcon =
			new wxBitmap(filename.GetFullPath(), wxBITMAP_TYPE_ANY);
		wxASSERT(this->defaultSkin->welcomeIcon->IsOk());
	}
	if ( this->defaultSkin->modsIcon == NULL ) {
		wxFileName filename(_T(RESOURCES_PATH), _T("mods.png"));
		this->defaultSkin->modsIcon =
			new wxBitmap(filename.GetFullPath(), wxBITMAP_TYPE_ANY);
		wxASSERT(this->defaultSkin->modsIcon->IsOk());
	}
	if ( this->defaultSkin->basicIcon == NULL ) {
		wxFileName filename(_T(RESOURCES_PATH), _T("basic.png"));
		this->defaultSkin->basicIcon =
			new wxBitmap(filename.GetFullPath(), wxBITMAP_TYPE_ANY);
		wxASSERT(this->defaultSkin->basicIcon->IsOk());
	}
	if ( this->defaultSkin->advancedIcon == NULL ) {
		wxFileName filename(_T(RESOURCES_PATH), _T("advanced.png"));
		this->defaultSkin->advancedIcon = 
			new wxBitmap(filename.GetFullPath(), wxBITMAP_TYPE_ANY);
		wxASSERT(this->defaultSkin->advancedIcon->IsOk());
	}
	if ( this->defaultSkin->installIcon == NULL ) {
		wxFileName filename(_T(RESOURCES_PATH), _T("install.png"));
		this->defaultSkin->installIcon =
			new wxBitmap(filename.GetFullPath(), wxBITMAP_TYPE_ANY);
		wxASSERT(this->defaultSkin->installIcon->IsOk());
	}
	if ( this->defaultSkin->idealIcon == NULL ) {
		wxFileName filename(_T(RESOURCES_PATH), _T("recommended.png"));
		this->defaultSkin->idealIcon = 
			new wxBitmap(filename.GetFullPath(), wxBITMAP_TYPE_ANY);
		wxASSERT(this->defaultSkin->idealIcon->IsOk());
	}

	if ( this->defaultSkin->baseFont == NULL ) {
		this->defaultSkin->baseFont =
			new wxFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
	}

	if ( this->defaultSkin->welcomePageText == NULL ) {
		this->defaultSkin->welcomePageText =
			new wxString(
_("<p><center><b><font size='3'>Welcome to  wxLauncher, your one-stop-shop for Freespace 2 and related content</font></b><br><br>If you're  new to the Freespace 2 universe, you might want to check out these links first:<br> = <a href='http://www.hard-light.net/wiki/index.php/Main_Page'>FS2 Wiki</a> = <a href='http://www.hard-light.net/forums/'>FS2 Forum</a> = <a href='http://en.wikipedia.org/wiki/FreeSpace_2'>Wikipedia FS2 page</a> = <a href='http://scp.indiegames.us/mantis/main_page.php'> Reporting bugs</a> = <br><br>Also, don't  forget the help file, there is a nice '<a href='help://Getting started tutorial'>Getting Started</a>' tutorial there.<br></center></p>"));
	}

	if ( this->defaultSkin->warningIcon == NULL ) {
		wxFileName filename(_T(RESOURCES_PATH), _T("icon_warning.png"));
		this->defaultSkin->warningIcon = new wxBitmap(filename.GetFullPath(), wxBITMAP_TYPE_ANY);
		wxASSERT(this->defaultSkin->warningIcon->IsOk());
	}

	if ( this->defaultSkin->bigWarningIcon == NULL ) {
		wxFileName filename(_T(RESOURCES_PATH), _T("warning_big.png"));
		this->defaultSkin->bigWarningIcon = new wxBitmap(filename.GetFullPath(), wxBITMAP_TYPE_ANY);
		if ( !this->defaultSkin->bigWarningIcon->IsOk() ) {
			delete this->defaultSkin->bigWarningIcon;
			this->defaultSkin->bigWarningIcon = NULL;
		}
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

wxBitmap SkinSystem::GetIdealIcon() {
	if ( this->modSkin != NULL
		&& this->modSkin->idealIcon != NULL ) {
			return *(this->modSkin->idealIcon);
	} else if ( this->TCSkin != NULL
		&& this->TCSkin->idealIcon != NULL ) {
			return *(this->TCSkin->idealIcon);
	} else if ( this->defaultSkin != NULL
		&& this->defaultSkin->idealIcon != NULL ) {
			return *(this->defaultSkin->idealIcon);
	} else {
		wxLogFatalError(_T("Cannot retrive an ideal icon. (0x%h, 0x%h, 0x%h)"),
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
		wxLogFatalError(_T("Cannot retrive a Banner. (0x%h, 0x%h, 0x%h)"),
			this->modSkin, this->TCSkin, this->defaultSkin);
		return wxNullBitmap;
	}
}

wxFont SkinSystem::GetFont() {
	const wxFont *temp = GetFontPointer();
	return (temp == NULL) ? wxNullFont : *temp;
}

const wxFont* SkinSystem::GetFontPointer() {
	if ( this->modSkin != NULL
		&& this->modSkin->baseFont != NULL ) {
			return this->modSkin->baseFont;
	} else if ( this->TCSkin != NULL
		&& this->TCSkin->baseFont != NULL ) {
			return this->TCSkin->baseFont;
	} else if ( this->defaultSkin != NULL
		&& this->defaultSkin->baseFont != NULL ) {
			return this->defaultSkin->baseFont;
	} else {
		wxLogFatalError(_T("Cannot retrive a font. (0x%h, 0x%h, 0x%h)"),
			this->modSkin, this->TCSkin, this->defaultSkin);
		return NULL;
	}
}

wxString SkinSystem::GetWelcomePageText() {
	if ( this->modSkin != NULL
		&& this->modSkin->welcomePageText != NULL ) {
			return *(this->modSkin->welcomePageText);
	} else if ( this->TCSkin != NULL
		&& this->TCSkin->welcomePageText != NULL ) {
			return *(this->TCSkin->welcomePageText);
	} else if ( this->defaultSkin != NULL
		&& this->defaultSkin->welcomePageText ) {
			return *(this->defaultSkin->welcomePageText);
	} else {
		wxLogFatalError(_T("Cannot retrive a font. (0x%h, 0x%h, 0x%h)"),
			this->modSkin, this->TCSkin, this->defaultSkin);
		return wxEmptyString;
	}
}

wxBitmap SkinSystem::GetWarningIcon() {
	if ( this->modSkin != NULL
		&& this->modSkin->warningIcon != NULL ) {
			return *(this->modSkin->warningIcon);
	} else if ( this->TCSkin != NULL
		&& this->TCSkin->warningIcon != NULL ) {
			return *(this->TCSkin->warningIcon);
	} else if ( this->defaultSkin != NULL
		&& this->defaultSkin->warningIcon ) {
			return *(this->defaultSkin->warningIcon);
	} else {
		wxLogFatalError(_T("Cannot retrive a warning icon. (0x%h, 0x%h, 0x%h)"),
			this->modSkin, this->TCSkin, this->defaultSkin);
		return wxNullBitmap;
	}
}

wxBitmap SkinSystem::GetBigWarningIcon() {
	if ( this->modSkin != NULL
		&& this->modSkin->bigWarningIcon != NULL ) {
			return *(this->modSkin->bigWarningIcon);
	} else if ( this->TCSkin != NULL
		&& this->TCSkin->bigWarningIcon != NULL ) {
			return *(this->TCSkin->bigWarningIcon);
	} else if ( this->defaultSkin != NULL
		&& this->defaultSkin->bigWarningIcon ) {
			return *(this->defaultSkin->bigWarningIcon);
	} else {
		wxImage image = this->GetWarningIcon().ConvertToImage();
		image = image.Scale(SkinSystem::BigWarningIconWidth, SkinSystem::BigWarningIconHeight);
		return wxBitmap(image);
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

/** Returns true if function is able to get a valid filename object for the
passed paths.  Filename is returned via the param filename. */
bool SkinSystem::SearchFile(wxFileName *filename, wxString currentTC,
							wxString shortmodname, wxString filepath) {
	filename->Assign(
		wxString::Format(_T("%s/%s"), shortmodname.c_str(), filepath.c_str()));
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

wxBitmap SkinSystem::MakeModsListImage(const wxBitmap &orig) {
	wxImage temp = orig.ConvertToImage();
	wxImage temp1 = temp.Scale(SkinSystem::ModsListImageWidth,
		SkinSystem::ModsListImageHeight,
		wxIMAGE_QUALITY_HIGH);
	wxBitmap outimg = wxBitmap(temp1);
	wxASSERT( outimg.GetWidth() == SkinSystem::ModsListImageWidth);
	wxASSERT( outimg.GetHeight() == SkinSystem::ModsListImageHeight);
	return outimg;
}

ArtProvider::ArtProvider(SkinSystem *skinSystem) {
	this->skinSystem = skinSystem;
}

wxBitmap ArtProvider::CreateBitmap(const wxArtID &id, const wxArtClient &client, const wxSize &size) {
	wxBitmap bitmap;
	if ( id == wxART_HELP ) {
		wxFileName filename(_T(RESOURCES_PATH), _T("helpicon.png"));
		if ( bitmap.LoadFile(filename.GetFullPath(), wxBITMAP_TYPE_ANY) ) {
			return bitmap;
		} else {
			return wxNullBitmap;
		}
	} else {
		return wxNullBitmap;
	}
}
