#include <wx/wx.h>
#include <wx/dynlib.h>
#include "generated/configure_launcher.h"
#include "OpenALManager.h"
#include "ids.h"

#if USE_OPENAL
#include <al.h>
#include <alc.h>
#endif

#include "wxLauncherSetup.h"

#if USE_OPENAL
namespace OpenALMan {
wxDynamicLibrary OpenALLib;
bool isInitialized = false;
};
using namespace OpenALMan;
#endif

bool OpenALMan::Initialize() {
#if USE_OPENAL
	if ( OpenALLib.Load(_T("OpenAL32")) ) {
		isInitialized = true;
		return true;
	} else if ( OpenALLib.Load(_T("libopenal")) ) {
		isInitialized = true;
		return true;
	} else {
		return false;
	}
#else
	return false;
#endif
}

bool OpenALMan::DeInitialize() {
#if USE_OPENAL
	OpenALLib.Unload();
	return true;
#else
	return false;
#endif
}

bool OpenALMan::IsInitialized() {
#if USE_OPENAL
	return isInitialized;
#else
	return false;
#endif
}

bool OpenALMan::WasCompliedIn() {
#if USE_OPENAL
	return true;
#else
	return false;
#endif
}

#if USE_OPENAL
typedef const ALCchar* (ALC_APIENTRY *alcGetStringType)(ALCdevice*, ALenum);
typedef ALCboolean (ALC_APIENTRY *alcIsExtensionPresentType)(ALCdevice*, const ALchar*);
typedef const ALchar* (AL_APIENTRY *alGetStringType)(ALenum);
typedef ALenum (AL_APIENTRY *alGetErrorType)(void);
namespace OpenALMan {
	alcGetStringType GetalcGetStringPointer();
	alcIsExtensionPresentType  GetalcIsExtensionPresentPointer();
	alGetStringType GetalGetStringPointer();
	alGetErrorType GetalGetErrorPointer();
	bool checkForALError();
};

alcGetStringType OpenALMan::GetalcGetStringPointer() {
	if ( !OpenALLib.HasSymbol(_T("alcGetString")) ) {
		wxLogError(_T("OpenAL does not have alcGetString()"));
		return NULL;
	}

	alcGetStringType GetString = NULL;
	GetString = reinterpret_cast<alcGetStringType>(
		OpenALLib.GetSymbol(_T("alcGetString")));

	if ( GetString == NULL ) {
		wxLogError(_T("Unable to get alcGetString() function from OpenAL"));
		return NULL;
	}

	return GetString;
}

alGetStringType OpenALMan::GetalGetStringPointer() {
	if ( !OpenALLib.HasSymbol(_T("alGetString")) ) {
		wxLogError(_T("OpenAL does not have alGetString()"));
		return NULL;
	}

	alGetStringType GetString = NULL;
	GetString = reinterpret_cast<alGetStringType>(
		OpenALLib.GetSymbol(_T("alGetString")));

	if ( GetString == NULL ) {
		wxLogError(_T("Unable to get alGetString() function from OpenAL"));
		return NULL;
	}

	return GetString;
}

alcIsExtensionPresentType OpenALMan::GetalcIsExtensionPresentPointer() {

	if ( !OpenALLib.HasSymbol(_T("alcIsExtensionPresent")) ) {
		wxLogError(_T("The OpenAL library does not have alcIsExtensionPresent"));
		return NULL;
	}
	alcIsExtensionPresentType isExtentsionPresent = NULL;
	isExtentsionPresent = reinterpret_cast<ALCboolean (*)(ALCdevice*, const ALchar*)>(
		OpenALLib.GetSymbol(_T("alcIsExtensionPresent")));

	if ( isExtentsionPresent == NULL ) {
		wxLogError(_T("Unable to retrive the pointer to the alcIsExtensionPresent() function"));
		return NULL;
	}
	return isExtentsionPresent;
}

alGetErrorType OpenALMan::GetalGetErrorPointer() {
	if ( !OpenALLib.HasSymbol(_T("alGetError")) ) {
		wxLogError(_T("OpenAL does not have alGetError()"));
		return NULL;
	}

	alGetErrorType getError = NULL;
	getError = reinterpret_cast<alGetErrorType>(
		OpenALLib.GetSymbol(_T("alGetError")));

	if ( getError == NULL ) {
		wxLogError(_T("Unable to get alGetError() function from OpenAL"));
		return NULL;
	}
	return getError;
}

bool OpenALMan::checkForALError() {
	alGetErrorType getError = GetalGetErrorPointer();
	if ( getError == NULL ) {
		return false;
	}
	ALenum errorcode = (*getError)();
	if ( errorcode == AL_NO_ERROR ) {
		return true;
	} else if ( errorcode == AL_INVALID_NAME ) {
		wxLogError(_T("OpenAL: a bad name (ID) was passed to an OpenAL function"));
	} else if ( errorcode == AL_INVALID_ENUM ) {
		wxLogError(_T("OpenAL: an invalid enum value was passed to an OpenAL function"));
	} else {
		wxLogError(_T("OpenAL: Unknown error number 0x%08x"), errorcode);
	}
	return false;
}
#endif

wxArrayString OpenALMan::GetAvailiableDevices() {
	wxArrayString arr;
#if USE_OPENAL
	wxCHECK_MSG( OpenALMan::IsInitialized(), arr,
		_T("GetAvailiableDevices called but OpenALMan not initialized"));
	if ( !OpenALLib.HasSymbol(_T("alcIsExtensionPresent")) ) {
		wxLogError(_T("The OpenAL library does not have alcIsExtensionPresent"));
		return arr;
	}

	alcIsExtensionPresentType isExtentsionPresent = 
		OpenALMan::GetalcIsExtensionPresentPointer();

	if ( isExtentsionPresent != NULL ) {
		if ( (*isExtentsionPresent)(NULL, "ALC_ENUMERATION_EXT") != AL_TRUE ) {
			wxLogError(_T("OpenAL does not seem to support device enumeration"));
			return arr;
		}
	} else {
		return arr;
	}

	alcGetStringType GetString = GetalcGetStringPointer();

	if ( GetString != NULL ) {
		const ALCchar* devices = (*GetString)(NULL, ALC_DEVICE_SPECIFIER);
		if ( devices != NULL ) {
			size_t len;
			size_t offset = 0;
			do {
				len = strlen(devices+offset);
				if ( len > 0 ) {
					wxString device(devices+offset, wxConvUTF8);
					arr.Add(device);
				}
				offset += len+1;
			} while ( len != 0 );
		} else {
			wxLogError(_T("OpenAL gave use NULL for list of devices."));
			return arr;
		}
	} else {
		return arr;
	}
#endif
	return arr;
}

wxString OpenALMan::SystemDefaultDevice() {
#if USE_OPENAL
	alcGetStringType GetString = GetalcGetStringPointer();

	if ( GetString == NULL ) {
		return wxEmptyString;
	} else {
		const ALCchar* device = (*GetString)(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
		if ( device == NULL ) {
			return wxEmptyString;
		} else {
			wxString DefaultDevice(device, wxConvUTF8);
			return DefaultDevice;
		}
	}
#else
	return wxEmptyString;
#endif
}

wxString OpenALMan::GetCurrentVersion() {
#if USE_OPENAL
	alGetStringType GetString = GetalGetStringPointer();

	if ( GetString == NULL ) {
		return _("Unknown version");
	} else {
		const ALCchar* version = (*GetString)(AL_VERSION);
		if ( !checkForALError() || version == NULL ) {
			wxLogError(_T("OpenAL: Unable to retrive Version String"));
			return _("Unknown version");
		} else {
			wxString Version(version, wxConvUTF8);
			return Version;
		}
	}
#else
	return wxEmptyString;
#endif
}

