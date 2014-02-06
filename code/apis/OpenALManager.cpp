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
#include "global/ProfileKeys.h"

#if USE_OPENAL
#include <al.h>
#include <alc.h>
#endif

#include "global/MemoryDebugging.h"

// from FSO, code/sound/openal.cpp, SVN r8840
// enumeration extension
#ifndef ALC_DEFAULT_ALL_DEVICES_SPECIFIER
#define ALC_DEFAULT_ALL_DEVICES_SPECIFIER        0x1012
#endif

#ifndef ALC_ALL_DEVICES_SPECIFIER
#define ALC_ALL_DEVICES_SPECIFIER                0x1013
#endif

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

bool OpenALMan::WasCompiledIn() {
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

wxArrayString GetAvailableDevices(const ALenum deviceType) {
	wxArrayString arr;
#if USE_OPENAL
	wxCHECK_MSG(OpenALMan::IsInitialized(), arr,
		_T("GetAvailableDevices called but OpenALMan not initialized"));
	
	wxCHECK_MSG(deviceType == ALC_DEVICE_SPECIFIER ||
		deviceType == ALC_CAPTURE_DEVICE_SPECIFIER, arr,
		wxString::Format(_T("GetAvailableDevices given invalid specifier %d"),
			deviceType));

	ALenum adjustedDeviceType = deviceType;
	
	alcIsExtensionPresentType isExtensionPresent =
		GetOALFuncPtr(alcIsExtensionPresentType, alcIsExtensionPresent);

	if ( isExtensionPresent != NULL ) {
		if ( (*isExtensionPresent)(NULL, "ALC_ENUMERATION_EXT") != AL_TRUE ) {
			wxLogFatalError(_T("OpenAL does not seem to support device enumeration."));
			return arr;
		}
	} else {
		return arr;
	}

	if ((deviceType == ALC_DEVICE_SPECIFIER) &&
			(*isExtensionPresent)(NULL, "ALC_ENUMERATE_ALL_EXT") == AL_TRUE) {
		adjustedDeviceType = ALC_ALL_DEVICES_SPECIFIER;
	}
	
	alcGetStringType GetString = GetOALFuncPtr(alcGetStringType, alcGetString);

	if ( GetString != NULL ) {
		const ALCchar* devices = (*GetString)(NULL, adjustedDeviceType);
		if ( devices != NULL ) {
			size_t len;
			size_t offset = 0;
			do {
				len = strlen(devices+offset);
				if ( len > 0 ) {
					wxString device(devices+offset, *wxConvCurrent);
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

wxArrayString OpenALMan::GetAvailablePlaybackDevices() {
#if USE_OPENAL
	wxCHECK_MSG(OpenALMan::IsInitialized(), wxArrayString(),
		_T("GetAvailablePlaybackDevices called but OpenALMan not initialized"));
	return GetAvailableDevices(ALC_DEVICE_SPECIFIER);
#else
	return wxArrayString();
#endif	
}

wxArrayString OpenALMan::GetAvailableCaptureDevices() {
#if USE_OPENAL
	wxCHECK_MSG(OpenALMan::IsInitialized(), wxArrayString(),
		_T("GetAvailableCaptureDevices called but OpenALMan not initialized"));
	return GetAvailableDevices(ALC_CAPTURE_DEVICE_SPECIFIER);
#else
	return wxArrayString();
#endif
}

wxString GetSystemDefaultDevice(const ALenum deviceType) {
#if USE_OPENAL
	wxCHECK_MSG( OpenALMan::IsInitialized(), wxEmptyString,
		_T("GetSystemDefaultDevice called but OpenALMan not initialized"));
	wxCHECK_MSG(deviceType == ALC_DEFAULT_DEVICE_SPECIFIER ||
		deviceType == ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER, wxEmptyString,
		wxString::Format(_T("GetSystemDefaultDevice given invalid specifier %d"),
			deviceType));
	
	ALenum adjustedDeviceType = deviceType;
	
	alcIsExtensionPresentType isExtensionPresent =
		GetOALFuncPtr(alcIsExtensionPresentType, alcIsExtensionPresent);
	
	if ((isExtensionPresent != NULL) &&
		(deviceType == ALC_DEFAULT_DEVICE_SPECIFIER) &&
		((*isExtensionPresent)(NULL, "ALC_ENUMERATE_ALL_EXT") == AL_TRUE)) {
			adjustedDeviceType = ALC_DEFAULT_ALL_DEVICES_SPECIFIER;
	}
	
	alcGetStringType GetString = GetOALFuncPtr(alcGetStringType, alcGetString);

	if ( GetString == NULL ) {
		return wxEmptyString;
	} else {
		const ALCchar* defaultDevice = (*GetString)(NULL, adjustedDeviceType);
		if ( defaultDevice == NULL ) {
			wxLogError(_("Unable to get system default OpenAL %sdevice"),
				(deviceType == ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER ?
					_T("capture ") : wxEmptyString));
			return wxEmptyString;
		} else {
			wxString defaultDeviceStr(defaultDevice, *wxConvCurrent);
			wxLogDebug(_("System default OpenAL %sdevice: %s"),
				(deviceType == ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER ?
					_T("capture ") : wxEmptyString),
				defaultDeviceStr.c_str());
			return defaultDeviceStr;
		}
	}
#else
	return wxEmptyString;
#endif
}

wxString OpenALMan::GetSystemDefaultPlaybackDevice() {
#if USE_OPENAL
	wxCHECK_MSG(OpenALMan::IsInitialized(), wxEmptyString,
		_T("GetSystemDefaultPlaybackDevice called but OpenALMan not initialized"));
	return GetSystemDefaultDevice(ALC_DEFAULT_DEVICE_SPECIFIER);
#else
	return wxEmptyString;
#endif
}

wxString OpenALMan::GetSystemDefaultCaptureDevice() {
#if USE_OPENAL
	wxCHECK_MSG(OpenALMan::IsInitialized(), wxEmptyString,
		_T("GetSystemDefaultCaptureDevice called but OpenALMan not initialized"));
	return GetSystemDefaultDevice(ALC_CAPTURE_DEFAULT_DEVICE_SPECIFIER);
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
	if ( OpenDevice == NULL) {
		return _("Unable to get open device function");
	}

	ALCdevice* device = (*OpenDevice)(selectedDevice.char_str());
	if ( device == NULL) {
		wxLogError(_T("alcOpenDevice returned NULL for selected device '%s'"),
			selectedDevice.c_str());
		return _("Error opening device");
	}

	alcCreateContextType CreateContext =
		GetOALFuncPtr(alcCreateContextType,alcCreateContext);
	if ( CreateContext == NULL) {
		return _("Unable to get open context on device function");
	}

	ALCint attributes = 0;
	ALCcontext* context = (*CreateContext)(device,NULL);
	if ( context == NULL) {
		return _("Error in opening context");
	}

	alcMakeContextCurrentType MakeContextCurrent =
		GetOALFuncPtr(alcMakeContextCurrentType,alcMakeContextCurrent);
	if ( MakeContextCurrent == NULL) {
		return _("Unable to get the set context as current function");
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

	alcCloseDeviceType CloseDevice =
		GetOALFuncPtr(alcCloseDeviceType,alcCloseDevice);
	if ( CloseDevice == NULL ) {
		return _("Unable to close device");
	}

	(*CloseDevice)(device);

	return wxString::Format(_("Detected OpenAL version: %s"), Version.c_str());
#else
	return wxEmptyString;
#endif
}

// bits are adapted from GetCurrentVersion() and FSO, sound/ds.cpp, ds_init()
bool OpenALMan::IsEFXSupported(const wxString& playbackDeviceName) {
#if USE_OPENAL
	wxCHECK_MSG( OpenALMan::IsInitialized(), false,
		_T("IsEFXSupported called but OpenALMan not initialized"));
	
	if (playbackDeviceName.IsEmpty()) {
		wxLogError(_T("IsEFXSupported: playback device name is empty"));
		return false;
	}
	
	// clear errors, I have not done any openAL stuff, so make sure that any
	// errors that are active are because of me.
	checkForALError();
	
	alcOpenDeviceType OpenDevice = 
		GetOALFuncPtr(alcOpenDeviceType, alcOpenDevice);

	if (OpenDevice == NULL) {
		wxLogError(_T("IsEFXSupported: Unable to open device."));
		return false;
	}

	ALCdevice* playbackDevice = (*OpenDevice)(playbackDeviceName.char_str());
	
	if (playbackDevice == NULL) {
		wxLogError(
			_T("IsEFXSupported: alcOpenDevice returned NULL when opening device '%s'"),
			playbackDeviceName.c_str());
		return false;
	}

	alcIsExtensionPresentType isExtensionPresent =
		GetOALFuncPtr(alcIsExtensionPresentType, alcIsExtensionPresent);
	
	if (isExtensionPresent == NULL) {
		wxLogError(
			_T("IsEFXSupported: Could not get alcIsExtensionPresent function."));
		return false;
	}
	
	bool hasEFX = (*isExtensionPresent)(playbackDevice, "ALC_EXT_EFX") == AL_TRUE;

	alcCloseDeviceType CloseDevice =
		GetOALFuncPtr(alcCloseDeviceType, alcCloseDevice);
	
	if (CloseDevice == NULL) {
		wxLogError(_T("IsEFXSupported: Unable to close device."));
		return false;
	}

	(*CloseDevice)(playbackDevice);

	return hasEFX;
#else
	return false;
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
