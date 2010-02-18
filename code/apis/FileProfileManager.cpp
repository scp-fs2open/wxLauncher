#include "apis/ProfileManager.h"
#include "apis/PlatformProfileManager.h"
#include "apis/JoystickManager.h"
#include "global/ids.h"

ProMan::RegistryCodes FilePushProfile(wxFileConfig *cfg) {
	return ProMan::InvalidParameters;
}

ProMan::RegistryCodes FilePullProfile(wxFileConfig *cfg) {
	return ProMan::InvalidParameters;
}
