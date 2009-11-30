#include "HelpManager.h"

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
};