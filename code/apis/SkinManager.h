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

#include "apis/EventHandlers.h"
#include "datastructures/NewsSource.h"

/** TC skin has changed. */
DECLARE_EVENT_TYPE(EVT_TC_SKIN_CHANGED, wxID_ANY);

/** Holds a skin's information.  */
/** The Set() functions return true on success, false otherwise. */
class Skin {
public:
	Skin();
	
	const wxString& GetWindowTitle() const { return this->windowTitle; }
	bool SetWindowTitle(const wxString& windowTitle);
	
	const wxIcon& GetWindowIcon() const { return this->windowIcon; }
	bool SetWindowIcon(const wxIcon& windowIcon);
	
	const wxBitmap& GetBanner() const { return this->banner; }
	bool SetBanner(const wxBitmap& banner);
	
	const wxString& GetWelcomeText() const { return this->welcomeText; }
	bool SetWelcomeText(const wxString& welcomeText);
	
	const wxBitmap& GetModImage() const { return this->modImage; }
	bool SetModImage(const wxBitmap& modImage);
	
	const wxBitmap& GetSmallModImage() const { return this->smallModImage; }
	bool SetSmallModImage(const wxBitmap& smallModImage);
	
	const wxBitmap& GetOkIcon() const { return this->okIcon; }
	bool SetOkIcon(const wxBitmap& okIcon);
	
	const wxBitmap& GetWarningIcon() const { return this->warningIcon; }
	bool SetWarningIcon(const wxBitmap& warningIcon);
	
	const wxBitmap& GetBigWarningIcon() const { return this->bigWarningIcon; }
	bool SetBigWarningIcon(const wxBitmap& bigWarningIcon);
	
	const wxBitmap& GetErrorIcon() const { return this->errorIcon; }
	bool SetErrorIcon(const wxBitmap& errorIcon);
	
	const wxBitmap& GetInfoIcon() const { return this->infoIcon; }
	bool SetInfoIcon(const wxBitmap& infoIcon);
	
	const wxBitmap& GetBigInfoIcon() const { return this->bigInfoIcon; }
	bool SetBigInfoIcon(const wxBitmap& bigInfoIcon);
	
	const wxBitmap& GetHelpIcon() const { return this->helpIcon; }
	bool SetHelpIcon(const wxBitmap& helpIcon);
	
	const wxBitmap& GetBigHelpIcon() const { return this->bigHelpIcon; }
	bool SetBigHelpIcon(const wxBitmap& bigHelpIcon);
	
	const wxBitmap& GetIdealIcon() const { return this->idealIcon; }
	bool SetIdealIcon(const wxBitmap& idealIcon);
	
	const NewsSource* GetNewsSource() const { return this->newsSource; }
	bool SetNewsSource(const NewsSource* newsSource);
	
private:
	static bool CheckStatusBarIconDimensions(const wxBitmap& icon);
	static bool CheckBigIconDimensions(const wxBitmap& icon);
	
	wxString windowTitle;
	wxIcon windowIcon;
	wxBitmap banner;
	wxString welcomeText;
	
	/** The 255x112 and 182x80 versions of the default mod image. */
	wxBitmap modImage;
	wxBitmap smallModImage;

	wxBitmap okIcon;
	wxBitmap warningIcon;
	wxBitmap bigWarningIcon;
	wxBitmap errorIcon;
	wxBitmap infoIcon;
	wxBitmap bigInfoIcon;
	wxBitmap helpIcon;
	wxBitmap bigHelpIcon;
	wxBitmap idealIcon;
	
	const NewsSource* newsSource;
};

/** Class used to manage the skinning of the launcher.  The skinnable parts of
the app register with the skin system, so that when the skin changes, they can
be updated if needed. */
class SkinSystem {
public:
	static bool Initialize();
	static void DeInitialize();
	static bool IsInitialized();
	static SkinSystem* GetSkinSystem();
	
	~SkinSystem();
	
	static void RegisterTCSkinChanged(wxEvtHandler *handler);
	static void UnRegisterTCSkinChanged(wxEvtHandler *handler);

	const wxString& GetWindowTitle() const;
	const wxIcon& GetWindowIcon() const;
	const wxBitmap& GetBanner() const;
	const wxString& GetWelcomeText() const;
	
	const wxBitmap& GetModImage() const;
	const wxBitmap& GetSmallModImage() const;
	
	const wxBitmap& GetOkIcon() const;
	const wxBitmap& GetWarningIcon() const;
	const wxBitmap& GetBigWarningIcon() const;
	const wxBitmap& GetErrorIcon() const;
	const wxBitmap& GetInfoIcon() const;
	const wxBitmap& GetBigInfoIcon() const;
	const wxBitmap& GetHelpIcon() const;
	const wxBitmap& GetBigHelpIcon() const;
	const wxBitmap& GetIdealIcon() const;
	
	const NewsSource& GetNewsSource() const;
	
	const wxFont& GetFont() const { return this->font; }
	const wxFont& GetMessageFont() const { return this->messageFont; }

	void SetTCSkin(const Skin* skin);
	void ResetTCSkin();

	static wxBitmap MakeModListImage(const wxBitmap &orig);
	static wxBitmap MakeModInfoDialogImage(const wxBitmap &orig);

	static bool SearchFile(wxFileName& filename, wxString currentTC,
		wxString shortmodname, wxString filepath);

	/** Dimensions for banner image on welcome page. */
	static const int BannerMaxWidth = 630;
	static const int BannerHeight   = 150;
	
	/** Dimensions for mod images, depending on where the image appears. */
	static const int ModInfoDialogImageWidth = 255;
	static const int ModInfoDialogImageHeight = 112;
	static const int ModListImageWidth = 182;
	static const int ModListImageHeight = 80;
	
	static const int StatusBarIconWidth = 24;
	static const int StatusBarIconHeight = 24;
	static const int BigIconWidth = 64;
	static const int BigIconHeight = 64;
	static const int HelpIconWidth = 32;
	static const int HelpIconHeight = 32;
	static const int IdealIconWidth = 24;
	static const int IdealIconHeight = 24;

private:
	SkinSystem();
	static SkinSystem* skinSystem;
	
	static EventHandlers TCSkinChangedHandlers;
	
	static void GenerateTCSkinChanged();
	
	void InitializeDefaultSkin();
	
	Skin defaultSkin;
	const Skin* TCSkin;
	
	wxFont font;
	wxFont messageFont;
};

#endif