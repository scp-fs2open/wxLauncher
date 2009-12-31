#include "HelpManager.h"

#include <wx/html/helpctrl.h>
#include "ids.h"
#include "wxLauncherSetup.h"

using namespace HelpManager;

namespace HelpManager {
	struct helpLink {
		WindowIDS id;
		wxString location;
	};
	helpLink helpLinks[] = {
#include "../generated/helplinks.cpp"
	};
	bool initialized = false;
	wxHtmlHelpController *controller = NULL;
	size_t numberOfHelpLinks = sizeof(HelpManager::helpLinks)/sizeof(HelpManager::helpLink);
};

bool HelpManager::Initialize() {
	if (HelpManager::IsInitialized()) {
		return false;
	}

	controller = new wxHtmlHelpController();
	wxFileName file(_T("help.htb"));
	if ( file.FileExists() ) {
		controller->AddBook(file, false);
	} else {
		::wxLogWarning(_T("Unable to find help file %s"), file.GetFullName());
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
	::wxLogInfo(_T(" ID %d does not have more specific help"), id);
}

void HelpManager::OpenMainHelpPage() {
	wxCHECK_RET( HelpManager::IsInitialized(), _T("Help is not initialized"));
	HelpManager::controller->DisplayContents();
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