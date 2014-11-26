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

#include "apis/HelpManager.h"
#include "generated/configure_launcher.h"
#include <wx/filename.h>
#include <wx/html/helpctrl.h>
#include "global/ids.h"

#include "global/MemoryDebugging.h"

using namespace HelpManager;

namespace HelpManager {
	struct helpLink {
		WindowIDS id;
		wxString location;
	};
	helpLink helpLinks[] = {
#include "generated/helplinks.cpp"
	};
	bool initialized = false;
	wxHtmlHelpController *controller = NULL;
	size_t numberOfHelpLinks = sizeof(HelpManager::helpLinks)/sizeof(HelpManager::helpLink);
	class ExternLinkHandler: public wxEvtHandler {
	public:
		void LinkClicked(wxHtmlLinkEvent &event);
	};
	ExternLinkHandler* externLinkHandler;
	class HtmlHelpController: public wxHtmlHelpController {
	public:
		virtual wxHtmlHelpFrame* CreateHelpFrame(wxHtmlHelpData *data);
	};
};

bool HelpManager::Initialize() {
	if (HelpManager::IsInitialized()) {
		return false;
	}

	externLinkHandler = new ExternLinkHandler();
	controller = new HtmlHelpController();
	wxFileName file(_T(HELP_HTB_LOCATION));
	if ( file.FileExists() ) {
		controller->AddBook(file, false);
	} else {
		wxLogWarning(_T("Unable to find help file %s"), file.GetFullName().c_str());
		delete controller;
		controller = NULL;
		return false;
	}

	HelpManager::initialized = true;
	return true;
}

bool HelpManager::DeInitialize() {
	if ( HelpManager::IsInitialized()) {
		HelpManager::initialized = false;
		delete controller;
		delete externLinkHandler;
	}
	return true;
}

bool HelpManager::IsInitialized() {
	return HelpManager::initialized;
}

void HelpManager::OpenHelpById(WindowIDS id) {
	wxCHECK_RET( HelpManager::IsInitialized(), _("Help manager is not initialized"));

	// find id if it exists
	for (size_t i = 0; i < HelpManager::numberOfHelpLinks; i++) {
		if (HelpManager::helpLinks[i].id == id) {
			HelpManager::controller->Display(
				HelpManager::helpLinks[i].location);
			return;
		}
	}
	wxLogInfo(_(" ID %d does not have more specific help"), id);
}

void HelpManager::OpenMainHelpPage() {
	wxCHECK_RET( HelpManager::IsInitialized(), _T("Help is not initialized"));
	// Hacky way to get the help tree to expand
	wxString faq(_T("jfaq.htm"));
	wxString index(_T("index.htm"));
	HelpManager::OpenHelpByString(faq);
	HelpManager::OpenHelpByString(index);
}

/** Opens the help file passing str to the help controller.
The help controller tries to find a page in the manual in 4 ways:
\li as a direct filename of the document in the manual
\li as a chapter name (based on the page title displayed in the contents)
\li a word from the index
\li any word (will open search pane and do a search)

\note Capitalization matters.
*/
void HelpManager::OpenHelpByString(wxString& str) {
	wxCHECK_RET( HelpManager::IsInitialized(), _("Help manager is not initialized"));

	HelpManager::controller->Display(str);
}


void HelpManager::ExternLinkHandler::LinkClicked(wxHtmlLinkEvent &event) {
	wxHtmlLinkInfo info(event.GetLinkInfo());
	wxString href(info.GetHref());
	if ( href.StartsWith(_T("http://")) ) {
		::wxLaunchDefaultBrowser(href);
	} else {
		event.Skip(); // not external, so I don't want it.
	}
}
wxHtmlHelpFrame* HelpManager::HtmlHelpController::CreateHelpFrame(wxHtmlHelpData *data) {
	wxHtmlHelpFrame* frame = new wxHtmlHelpFrame(data);
	frame->SetController(this);
	frame->Create(m_parentWindow, -1, wxEmptyString, m_FrameStyle, m_Config, m_ConfigRoot);
	frame->SetTitleFormat(m_titleFormat);
	frame->GetEventHandler()	// hook in to allow veto on the links so we can open external browsers
		->Connect(wxEVT_COMMAND_HTML_LINK_CLICKED, wxHtmlLinkEventHandler(ExternLinkHandler::LinkClicked), NULL, HelpManager::externLinkHandler);
	m_helpFrame = frame;
	return frame;
}
