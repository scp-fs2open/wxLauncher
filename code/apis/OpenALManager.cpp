#include <wx/wx.h>
#include <wx/dynlib.h>
#include "generated/configure_launcher.h"
#include "apis/OpenALManager.h"
#include "global/ids.h"

#if USE_OPENAL
#include <al.h>
#include <alc.h>
#endif

#include "global/MemoryDebugging.h"

#if USE_OPENAL
namespace OpenALMan {
wxDynamicLibrary OpenALLib;
bool isInitialized = false;
};
using namespace OpenALMan;
#endif

bool OpenALMan::Initialize() {
#if USE_OPENAL
	if ( isInitialized ) {
		return true;
	} else if ( OpenALLib.Load(_T("OpenAL32")) ) {
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
	template< typename funcPtrType> 
	funcPtrType GetOpenALFunctionPointer(const wxString& name, size_t line);
	bool checkForALError_(size_t line);
};
#define ___GetOALFuncPtr(type, name, line) GetOpenALFunctionPointer<type>(_T(name), line)
#define GetOALFuncPtr(type, name) ___GetOALFuncPtr(type, #name, __LINE__)
#define checkForALError() OpenALMan::checkForALError_(__LINE__)

template< typename funcPtrType> 
funcPtrType 
OpenALMan::GetOpenALFunctionPointer(const wxString& name, size_t line) {
	if ( !OpenALLib.HasSymbol(name) ) {
		wxLogError(
			_T("OpenAL does not have %s() for function containing line %d"),
			name.c_str(), line);
		return NULL;
	}

	funcPtrType pointer = NULL;

	pointer = reinterpret_cast<funcPtrType>(
		OpenALLib.GetSymbol(name));

	if ( pointer == NULL ) {
		wxLogError(_T("Unable to get %s() function from OpenAL, even though it apparently exists for function containing line %d"), name.c_str(), line);
		return NULL;
	}

	return pointer;
}

bool OpenALMan::checkForALError_(size_t line) {
	alGetErrorType getError = GetOALFuncPtr(alGetErrorType, alGetError);
	if ( getError == NULL ) {
		return false;
	}
	ALenum errorcode = (*getError)();
	if ( errorcode == AL_NO_ERROR ) {
		return true;
	} else if ( errorcode == AL_INVALID_NAME ) {
		wxLogError(_T("OpenAL:%d: a bad name (ID) was passed to an OpenAL function"), line);
	} else if ( errorcode == AL_INVALID_ENUM ) {
		wxLogError(_T("OpenAL:%d: an invalid enum value was passed to an OpenAL function"), line);
	} else {
		wxLogError(_T("OpenAL:%d: Unknown error number 0x%08x"), line, errorcode);
	}
	return false;
}
#endif

wxArrayString OpenALMan::GetAvailiableDevices() {
	wxArrayString arr;
#if USE_OPENAL
	wxCHECK_MSG( OpenALMan::IsInitialized(), arr,
		_T("GetAvailiableDevices called but OpenALMan not initialized"));

	alcIsExtensionPresentType isExtentsionPresent = 
		GetOALFuncPtr(alcIsExtensionPresentType, alcIsExtensionPresent);

	if ( isExtentsionPresent != NULL ) {
		if ( (*isExtentsionPresent)(NULL, "ALC_ENUMERATION_EXT") != AL_TRUE ) {
			wxLogError(_T("OpenAL does not seem to support device enumeration"));
			return arr;
		}
	} else {
		return arr;
	}

	alcGetStringType GetString = GetOALFuncPtr(alcGetStringType, alcGetString);

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
	alcGetStringType GetString = GetOALFuncPtr(alcGetStringType,alcGetStringType);

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
	alGetStringType GetString = GetOALFuncPtr(alGetStringType,alGetString);

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

