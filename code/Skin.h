#ifndef SKIN_H
#define SKIN_H

#include <wx/wx.h>
#include <wx/filename.h>

/** Struct that holds the skin information.  */
class Skin {
public:
	Skin();
	~Skin();
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
	~SkinSystem();

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
	const wxFont* GetFontPointer();

	void SetTCSkin(Skin *skin = NULL);
	void SetModSkin(Skin *skin = NULL);

	static wxBitmap* VerifySmallImage(wxString currentTC, wxString shortmodname, wxString filepath);
	static wxBitmap* VerifyWindowIcon(wxString currentTC, wxString shortmodname, wxString filepath);
	static wxBitmap* VerifyTabIcon(wxString currentTC, wxString shortmodname, wxString filepath);
	static wxBitmap* VerifyIdealIcon(wxString currentTC, wxString shortmodname, wxString filepath);
	static wxFont* VerifyFontChoice(wxString currentTC, wxString shortmodname,
		wxString fontname=_T(""), int fontsize=0, wxString fontFamilyStr=_T(""),
		wxString fontStyleStr=_T(""), wxString fontWeightStr=_T(""), bool underline=false );
	static wxBitmap MakeModsListImage(const wxBitmap &orig);

	static bool SearchFile(wxFileName* filename, wxString currentTC,
		wxString shortmodname, wxString filepath);

	static const unsigned int TabIconWidth = 64;
	static const unsigned int TabIconHeight = 64;
	static const unsigned int IdealIconWidth = 16;
	static const unsigned int IdealIconHeight = 16;
	static const unsigned int InfoWindowImageWidth = 255;
	static const unsigned int InfoWindowImageHeight = 112;
	static const unsigned int ModsListImageWidth = 182;
	static const unsigned int ModsListImageHeight = 80;


private:
	Skin* defaultSkin;
	Skin* TCSkin;
	Skin* modSkin;
};

#endif