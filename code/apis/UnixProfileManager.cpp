#include "apis/ProfileManager.h"
#include "apis/PlatformProfileManager.h"
#include "apis/JoystickManager.h"
#include "global/ids.h"

ProMan::RegistryCodes PlatformPushProfile(wxFileConfig *cfg) {
	return ProMan::InvalidParameters;
}

ProMan::RegistryCodes PlatformPullProfile(wxFileConfig *cfg) {
	return ProMan::InvalidParameters;
}
