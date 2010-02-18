#ifndef PLATFORMPROFILEMANAGER_H
#define PLATFORMPROFILEMANAGER_H
#include <wx/fileconf.h>
#include "apis/ProfileManager.h"

ProMan::RegistryCodes RegistryPushProfile(wxFileConfig *cfg);
ProMan::RegistryCodes RegistryPullProfile(wxFileConfig *cfg);

ProMan::RegistryCodes FilePushProfile(wxFileConfig *cfg);
ProMan::RegistryCodes FilePullProfile(wxFileConfig *cfg);

#endif