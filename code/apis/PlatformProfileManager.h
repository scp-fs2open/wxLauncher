#ifndef PLATFORMPROFILEMANAGER_H
#define PLATFORMPROFILEMANAGER_H
#include <wx/fileconf.h>
#include "apis/ProfileManager.h"

ProMan::RegistryCodes PlatformPushProfile(wxFileConfig *cfg);
ProMan::RegistryCodes PlatformPullProfile(wxFileConfig *cfg);

#endif