/*
Copyright (C) 2009-2010 wxLauncher Team

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <wx/wx.h>
#include <wx/dynlib.h>
#include "generated/configure_launcher.h"
#include "apis/FlagListManager.h"
#include "apis/OpenALManager.h"
#include "apis/ProfileManager.h"
#include "global/ids.h"

#if USE_OPENAL
#include <al.h>
#include <alc.h>
#endif

#include "global/MemoryDebugging.h"

const wxByte BUILD_CAP_NEW_SND = 1<<2;

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
#if IS_APPLE
	} else if ( OpenALLib.Load(_T("/System/Library/Frameworks/OpenAL.framework/OpenAL"),
							   wxDL_VERBATIM) ) {
		isInitialized = true;
		return true;
	} else if ( OpenALLib.Load(_T("/Library/Frameworks/OpenAL.framework/OpenAL"),
							   wxDL_VERBATIM) ) {
		isInitialized = true;
		return true;
#else
	} else if ( OpenALLib.Load(_T("OpenAL32")) ) {
		isInitialized = true;
		return true;
	} else if ( OpenALLib.Load(_T("libopenal")) ) {
		isInitialized = true;
		return true;
	} else if ( OpenALLib.Load(_T("OpenAL")) ) {
		isInitialized = true;
		return true;
#endif
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
typedef ALCdevice* (ALC_APIENTRY *alcOpenDeviceType)(const ALCchar *);
typedef ALCboolean (ALC_APIENTRY *alcCloseDeviceType)(ALCdevice *);
typedef ALCcontext* (ALC_APIENTRY *alcCreateContextType)(const ALCdevice*, const ALCint*);
typedef ALCboolean (ALC_APIENTRY *alcMakeContextCurrentType)(ALCcontext*);
typedef void (ALC_APIENTRY *alcDestroyContextType)(ALCcontext*);

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
#if PLATFORM_HAS_BROKEN_OPENAL == 1
	/** \todo a hack to fix certain OpenAL implementations that are not
	clearing the errors correctly. */
	return true;
#else
	return false;
#endif
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
			wxLogFatalError(_T("OpenAL does not seem to support device enumeration."));
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
			wxLogError(_T("OpenAL gave NULL for list of devices."));
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
	alcGetStringType GetString = GetOALFuncPtr(alcGetStringType, alcGetString);

	if ( GetString == NULL ) {
		return wxEmptyString;
	} else {
		const ALCchar* device = (*GetString)(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
		if ( device == NULL ) {
			wxLogError(_("Unable to get system default OpenAL device"));
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
	}
	
	wxString selectedDevice;
	ProMan::GetProfileManager()->ProfileRead(PRO_CFG_OPENAL_DEVICE, &selectedDevice);

	// clear errors, I have not done any openAL stuff, so make sure that any
	// errors that are active are because of me.
	checkForALError();

	alcOpenDeviceType OpenDevice = 
		GetOALFuncPtr(alcOpenDeviceType,alcOpenDevice);
	if ( OpenDevice == NULL || checkForALError() == false) {
		return _("Unable to open device");
	}

	ALCdevice* device = (*OpenDevice)(selectedDevice.char_str());
	if ( device == NULL || checkForALError() == false ) {
		wxLogError(_T("alcOpenDevice returned NULL for selected device '%s'"),
			selectedDevice.c_str());
		return _("Error opening device");
	}

	alcCreateContextType CreateContext =
		GetOALFuncPtr(alcCreateContextType,alcCreateContext);
	if ( OpenDevice == NULL || checkForALError() == false ) {
		return _("Unable to open context on device");
	}

	ALCint attributes = 0;
	ALCcontext* context = (*CreateContext)(device,NULL);
	if ( context == NULL || checkForALError() == false) {
		return _("Error in opening context");
	}

	alcMakeContextCurrentType MakeContextCurrent =
		GetOALFuncPtr(alcMakeContextCurrentType,alcMakeContextCurrent);
	if ( MakeContextCurrent == NULL || checkForALError() == false) {
		return _("Unable to set context as current");
	}

	if ( (*MakeContextCurrent)(context) != ALC_TRUE || checkForALError() == false ) {
		return _("Error in setting context as current");
	}
	
	const ALCchar* version = (*GetString)(AL_VERSION);
	if ( !checkForALError() || version == NULL ) {
		wxLogError(_T("OpenAL: Unable to retrieve Version String"));
		return _("Unknown version");
	}
	wxString Version(version, wxConvUTF8);

	// unset the current context
	(*MakeContextCurrent)(NULL);

	alcDestroyContextType DestroyContext =
		GetOALFuncPtr(alcDestroyContextType,alcDestroyContext);
	if ( DestroyContext == NULL ) {
		return _("Unable to destroy context");
	}

	(*DestroyContext)(context);
	context = NULL;
	if ( checkForALError() == false ) {
		return _("Error in destroying context");
	}

	alcCloseDeviceType CloseDevice =
		GetOALFuncPtr(alcCloseDeviceType,alcCloseDevice);
	if ( CloseDevice == NULL ) {
		return _("Unable to close device");
	}

	(*CloseDevice)(device);
	if ( checkForALError() == false ) {
		return _("Error in closing device");
	}

	return wxString::Format(_("Detected OpenAL version: %s"), Version.c_str());
#else
	return wxEmptyString;
#endif
}

bool OpenALMan::BuildHasNewSoundCode() {
	wxCHECK_MSG(OpenALMan::IsInitialized(), false,
		_T("OpenALMan has not been initialized."));
	wxCHECK_MSG(FlagListManager::IsInitialized(), false,
		_T("FlagListManager has not been initialized."));
	wxCHECK_MSG(FlagListManager::GetFlagListManager()->IsProcessingOK(), false,
		_T("Flag file processing has not (yet) succeeded."));
	
	return FlagListManager::GetFlagListManager()->GetBuildCaps() & BUILD_CAP_NEW_SND;
}
