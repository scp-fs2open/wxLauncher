#include "HelpManager.h"

#include <wx/html/helpctrl.h>
#include "wxIDS.h"
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