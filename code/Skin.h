#ifndef SKIN_H
#define SKIN_H

#include <wx/wx.h>

/** Struct that holds the skin information.  */
struct Skin {
	Skin();
	wxString* windowTitle;
	wxIcon* windowIcon;
	wxBitmap* welcomeHeader;
	wxBitmap* welcomeIcon;
	wxBitmap* modsIcon;
	wxBitmap* basicIcon;
	wxBitmap* advancedIcon;
	wxBitmap* installIcon;
	wxBitmap* idealIcon;
	wxFont* baseFont;
};

/** Class used to manage the skinning of the launcher.  The skinable parts of
the app registers with the skin class so that when the skin changes they can 
be updated if needed.  The classes can also register a callback so that they 
can run arbitary code when the skin changes. */
class SkinSystem {
public:
	SkinSystem(Skin* defaultSkin = NULL);

	wxString GetTitle();
	wxIcon GetIcon();
	wxBitmap GetBanner();
	wxBitmap GetWelcomeIcon();
	wxBitmap GetModsIcon();
	wxBitmap GetBasicIcon();
	wxBitmap GetAdvancedIcon();
	wxBitmap GetInstallIcon();
	wxBitmap GetIdealIcon();
	wxFont GetFont();

	void SetTCSkin(Skin *skin = NULL);
	void SetModSkin(Skin *skin = NULL);

private:
	Skin* defaultSkin;
	Skin* TCSkin;
	Skin* modSkin;
};

#endif