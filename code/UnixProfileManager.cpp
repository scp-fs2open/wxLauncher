#include "ProfileManager.h"
#include "PlatformProfileManager.h"
#include "JoystickManager.h"
#include "ids.h"

ProMan::RegistryCodes PlatformPushProfile(wxFileConfig *cfg) {
	return ProMan::UnkownError;
}

ProMan::RegistryCodes PlatformPullProfile(wxFileConfig *cfg) {
	return ProMan::UnkownError;
}
