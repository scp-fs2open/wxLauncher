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
	wxBitmap* idealIcon;
	wxString* welcomePageText;
	wxBitmap* warningIcon;
	wxBitmap* bigWarningIcon;
};

/** Class used to manage the skinning of the launcher.  The skinable parts of
the app registers with the skin class so that when the skin changes they can 
be updated if needed.  The classes can also register a callback so that they 
can run arbitary code when the skin changes. */
class SkinSystem {
public:
	static bool Initialize();
	static void DeInitialize();
	static bool IsInitialized();
	static SkinSystem* GetSkinSystem();
	
	~SkinSystem();

	wxString GetTitle();
	wxIcon GetIcon();
	wxBitmap GetBanner();
	wxBitmap GetIdealIcon();
	wxBitmap GetWarningIcon();
	wxBitmap GetBigWarningIcon();
	const wxFont& GetFont() const { return this->font; }
	wxString GetWelcomePageText();

	void SetTCSkin(Skin *skin = NULL);

	static wxBitmap* VerifySmallImage(wxString currentTC, wxString shortmodname, wxString filepath);
	static wxIcon* VerifyWindowIcon(wxString currentTC, wxString shortmodname, wxString filepath);
	static wxBitmap* VerifyIdealIcon(wxString currentTC, wxString shortmodname, wxString filepath);
	static wxBitmap MakeModsListImage(const wxBitmap &orig);

	static bool SearchFile(wxFileName* filename, wxString currentTC,
		wxString shortmodname, wxString filepath);

	static const unsigned int IdealIconWidth = 24;
	static const unsigned int IdealIconHeight = 24;
	static const unsigned int InfoWindowImageWidth = 255;
	static const unsigned int InfoWindowImageHeight = 112;
	static const unsigned int ModsListImageWidth = 182;
	static const unsigned int ModsListImageHeight = 80;
	static const unsigned int BigWarningIconWidth = 64;
	static const unsigned int BigWarningIconHeight = 64;


private:
	SkinSystem();
	static SkinSystem* skinSystem;
	
	Skin* defaultSkin;
	Skin* TCSkin;
	
	wxFont font;
};

#endif