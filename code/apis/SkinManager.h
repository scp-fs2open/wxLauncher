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

/** Holds a skin's information.  */
/** The Set() functions return true on success, false otherwise. */
class Skin {
public:
	Skin() { }
	
	const wxString& GetWindowTitle() const { return this->windowTitle; }
	bool SetWindowTitle(const wxString& windowTitle);
	
	const wxIcon& GetWindowIcon() const { return this->windowIcon; }
	bool SetWindowIcon(const wxIcon& windowIcon);
	
	const wxBitmap& GetBanner() const { return this->banner; }
	bool SetBanner(const wxBitmap& banner);
	
	const wxBitmap& GetIdealIcon() const { return this->idealIcon; }
	bool SetIdealIcon(const wxBitmap& idealIcon);
	
	const wxString& GetWelcomeText() const { return this->welcomeText; }
	bool SetWelcomeText(const wxString& welcomeText);
	
	const wxBitmap& GetWarningIcon() const { return this->warningIcon; }
	bool SetWarningIcon(const wxBitmap& warningIcon);
	
	const wxBitmap& GetBigWarningIcon() const { return this->bigWarningIcon; }
	bool SetBigWarningIcon(const wxBitmap& bigWarningIcon);
	
private:
	wxString windowTitle;
	wxIcon windowIcon;
	wxBitmap banner;
	wxBitmap idealIcon;
	wxString welcomeText;
	wxBitmap warningIcon;
	wxBitmap bigWarningIcon;
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

	const wxString& GetWindowTitle() const;
	const wxIcon& GetWindowIcon() const;
	const wxBitmap& GetBanner() const;
	const wxBitmap& GetIdealIcon() const;
	const wxBitmap& GetWarningIcon() const;
	const wxBitmap& GetBigWarningIcon() const;
	const wxString& GetWelcomeText() const;
	
	const wxFont& GetFont() const { return this->font; }
	const wxFont& GetMessageFont() const { return this->messageFont; }

	void SetTCSkin(Skin *skin = NULL);

	static wxBitmap* VerifySmallImage(wxString currentTC, wxString shortmodname, wxString filepath);
	static wxIcon* VerifyWindowIcon(wxString currentTC, wxString shortmodname, wxString filepath);
	static wxBitmap* VerifyIdealIcon(wxString currentTC, wxString shortmodname, wxString filepath);
	static wxBitmap MakeModsListImage(const wxBitmap &orig);

	static bool SearchFile(wxFileName& filename, wxString currentTC,
		wxString shortmodname, wxString filepath);

	static const int IdealIconWidth = 24;
	static const int IdealIconHeight = 24;
	static const unsigned int InfoWindowImageWidth = 255;
	static const unsigned int InfoWindowImageHeight = 112;
	static const int ModsListImageWidth = 182;
	static const int ModsListImageHeight = 80;
	static const int BigWarningIconWidth = 64;
	static const int BigWarningIconHeight = 64;
	static const int StatusBarIconWidth = 24;
	static const int StatusBarIconHeight = 24;


private:
	SkinSystem();
	static SkinSystem* skinSystem;
	
	void InitializeDefaultSkin();
	
	Skin defaultSkin;
	const Skin* TCSkin;
	
	wxFont font;
	wxFont messageFont;
};

#endif