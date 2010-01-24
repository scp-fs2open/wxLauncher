#include "ProfileManager.h"
#include "PlatformProfileManager.h"
#include "JoystickManager.h"
#include "ids.h"

ProMan::RegistryCodes PlatformPushProfile(wxFileConfig *cfg) {
	return ProMan::InvalidParameters;
}

ProMan::RegistryCodes PlatformPullProfile(wxFileConfig *cfg) {
	return ProMan::InvalidParameters;
}
