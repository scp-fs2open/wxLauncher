#include "ProfileManager.h"
#include "PlatformProfileManager.h"
#include "JoystickManager.h"
#include "ids.h"

#include <wx/process.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/wfstream.h>
#include <windows.h>

#include "wxLauncherSetup.h"

#define ReturnChecker(retvalue, location) \
	if ( retvalue != ERROR_SUCCESS ) {\
		LPWSTR message = NULL;\
		DWORD nchars = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,\
		NULL,\
		retvalue,\
		0,\
		(LPWSTR)&message,\
		0,\
		NULL);\
		if ( nchars == 0 ) {\
			wxLogFatalError(_T("An error occurred, but unable to generate error message"));\
		} else {\
			wxMBConvUTF16 conv;\
			wxString msg(message, conv, nchars);\
			wxLogError(_T("Unhandled error in setting HKLM key in registry! %s"), msg);\
		}\
		return ProMan::UnknownError;\
	}
		

/* File contains the Win32 incatations for Pushing and Pulling the passed
profile to/from the registry. */

ProMan::RegistryCodes PlatformPushProfile(wxFileConfig *cfg) {
	LONG ret = ERROR_SUCCESS;
	HKEY regHandle = 0;

	ret = RegCreateKeyExW(HKEY_LOCAL_MACHINE,
		L"Software\\Volition\\Freespace2",
		0, // Reserved
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		NULL, // default security.
		&regHandle,
		NULL // I don't care if the key was created before opening
		);
#ifdef REGISTRY_HELPER
	// Do not attempt to call registry_helper.exe if already in registry_helper.exe
	if ( ret != ERROR_SUCCESS ) {
#else
	if ( ret != ERROR_SUCCESS && ret != ERROR_ACCESS_DENIED) {
#endif
		// Call failed
		LPWSTR message = NULL;
		DWORD nchars = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		ret,
		0,
		(LPWSTR)&message,
		0,
		NULL);
		if ( nchars == 0 ) {
			wxLogFatalError(_T("An error occurred, but unable to generate error message"));
		} else {
			wxMBConvUTF16 conv;
			wxString msg(message, conv, nchars);
			wxLogError(_T("Unhandled error in creating HKLM key in registry! %s"), msg);
			LocalFree(message);
		}
		return ProMan::AccessDenied;
#ifndef REGISTRY_HELPER
	} else if ( ret == ERROR_ACCESS_DENIED ) {
		// Only try calling registry_helper.exe if not already in registry_helper.exe
		wxString tempfile = wxFileName::CreateTempFileName(wxStandardPaths::Get().GetTempDir());
		wxLogDebug(_T("Launching helper on %s"), tempfile);
		wxFileOutputStream out(tempfile);
		cfg->Save(out);
		wxArrayString processOutput;
		long ret = wxExecute(wxString::Format(_T("registry_helper.exe push %s"), tempfile), processOutput);
		wxLogDebug(_T(" Registry helper returned %d"), ret);

		for(size_t i = 0; i < processOutput.size(); i++) {
			wxLogInfo(_T(" REG:%s"), processOutput[i]);
		}

		if ( ret == ProMan::NoError ) {
			// no error so just return, because the other process did what I needed.
			return ProMan::NoError;
		} else {
			wxLogError(_T("Unable to write FS2Open settings to the registry (%d)"), ret);
			return static_cast<ProMan::RegistryCodes>(ret);
		}
#endif	
	}
	int width, height, bitdepth;
	cfg->Read(PRO_CFG_VIDEO_RESOLUTION_HEIGHT, &width, 1024);
	cfg->Read(PRO_CFG_VIDEO_RESOLUTION_WIDTH, &height, 768);
	cfg->Read(PRO_CFG_VIDEO_BIT_DEPTH, &bitdepth, 16);

	wxString videocardValue = wxString::Format(_T("OGL -(%dx%d)x%d bit"), height, width, bitdepth);
	ret = RegSetValueExW(
		regHandle,
		L"VideocardFs2open",
		0,
		REG_SZ,
		(BYTE*)videocardValue.c_str(),
		(videocardValue.size() + 1)*2);
	ReturnChecker(ret, __LINE__);


	wxString soundDevice;
	cfg->Read(PRO_CFG_OPENAL_DEVICE, &soundDevice, _T("Generic Software"));

	ret = RegSetValueExW(
		regHandle,
		L"SoundDeviceOAL",
		0,
		REG_SZ,
		(BYTE*)soundDevice.c_str(),
		(soundDevice.size() + 1)*2);
	ReturnChecker(ret, __LINE__);


	// GammaD3D

	// Language

	wxString oglAnisotropicFilter;
	cfg->Read(PRO_CFG_VIDEO_ANISOTROPIC, &oglAnisotropicFilter, _T("0.0"));

	ret = RegSetValueExW(
		regHandle,
		L"OGL_AnisotropicFilter",
		0,
		REG_SZ,
		(BYTE*)oglAnisotropicFilter.c_str(),
		(oglAnisotropicFilter.size() + 1)*2);
	ReturnChecker(ret, __LINE__);


	wxString connectionSpeedValue;
	cfg->Read(PRO_CFG_NETWORK_SPEED, &connectionSpeedValue, _T("None"));

	ret = RegSetValueExW(
		regHandle,
		L"ConnectionSpeed",
		0,
		REG_SZ,
		(BYTE*)connectionSpeedValue.c_str(),
		(connectionSpeedValue.size() + 1)*2);
	ReturnChecker(ret, __LINE__);

	
	wxString networkConnectionValue;
	cfg->Read(PRO_CFG_NETWORK_TYPE, &networkConnectionValue, _T("None"));

	ret = RegSetValueExW(
		regHandle,
		L"NetworkConnection",
		0,
		REG_SZ,
		(BYTE*)networkConnectionValue.c_str(),
		(networkConnectionValue.size() + 1)*2);
	ReturnChecker(ret, __LINE__);


	// ImageExportNum
	
	// LowMem

	// ForceFullscreen

	// MaxFPS

	// SoundSampleRate

	// SoundSampleBits

	// ScreenshotNum

	int inMulti, inTechroom, inBriefings, inGame;
	cfg->Read(PRO_CFG_SPEECH_IN_BRIEFINGS, &inBriefings, true);
	cfg->Read(PRO_CFG_SPEECH_IN_GAME, &inGame, true);
	cfg->Read(PRO_CFG_SPEECH_IN_MULTI, &inMulti, true);
	cfg->Read(PRO_CFG_SPEECH_IN_TECHROOM, &inTechroom, true);

	ret = RegSetValueExW(
		regHandle,
		L"SpeechTechroom",
		0,
		REG_DWORD,
		(BYTE*)&inTechroom,
		sizeof(inTechroom));
	ReturnChecker(ret, __LINE__);
	ret = RegSetValueExW(
		regHandle,
		L"SpeechGame",
		0,
		REG_DWORD,
		(BYTE*)&inGame,
		sizeof(inGame));
	ReturnChecker(ret, __LINE__);
	ret = RegSetValueExW(
		regHandle,
		L"SpeechMulti",
		0,
		REG_DWORD,
		(BYTE*)&inMulti,
		sizeof(inMulti));
	ReturnChecker(ret, __LINE__);
	ret = RegSetValueExW(
		regHandle,
		L"SpeechTechroom",
		0,
		REG_DWORD,
		(BYTE*)&inTechroom,
		sizeof(inTechroom));
	ReturnChecker(ret, __LINE__);

	
	int speechVolume;
	cfg->Read(PRO_CFG_SPEECH_VOLUME, &speechVolume, 100);

	ret = RegSetValueExW(
		regHandle,
		L"SpeechVolume",
		0,
		REG_DWORD,
		(BYTE*)&speechVolume,
		sizeof(speechVolume));
	ReturnChecker(ret, __LINE__);

	int speechVoice;
	cfg->Read(PRO_CFG_SPEECH_VOICE, &speechVoice, 0);

	ret = RegSetValueExW(
		regHandle,
		L"SpeechVoice",
		0,
		REG_DWORD,
		(BYTE*)&speechVoice,
		sizeof(speechVoice));
	ReturnChecker(ret, __LINE__);

	// Fullscreen

	int oglAntiAliasSample;
	cfg->Read(PRO_CFG_VIDEO_ANTI_ALIAS, &oglAntiAliasSample, 0);

	ret = RegSetValueExW(
		regHandle,
		L"OGL_AntiAliasSamples",
		0,
		REG_DWORD,
		(BYTE*)&oglAntiAliasSample,
		sizeof(oglAntiAliasSample));

	wxString filterMethod;
	cfg->Read(PRO_CFG_VIDEO_TEXTURE_FILTER, &filterMethod, _T("Bilinear"));
	int filterMethodValue = ( filterMethod.StartsWith(_T("Bilinear")))?1:0;

	ret = RegSetValueExW(
		regHandle,
		L"TextureFilter",
		0,
		REG_DWORD,
		(BYTE*)&filterMethodValue,
		sizeof(filterMethodValue));
	ReturnChecker(ret, __LINE__);


	int currentJoystick;
	cfg->Read(PRO_CFG_JOYSTICK_ID, &currentJoystick, JOYMAN_INVAILD_JOYSTICK);

	ret = RegSetValueExW(
		regHandle,
		L"CurrentJoystick",
		0,
		REG_DWORD,
		(BYTE*)&currentJoystick,
		sizeof(currentJoystick));
	ReturnChecker(ret, __LINE__);

	
	int joystickForceFeedback;
	cfg->Read(PRO_CFG_JOYSTICK_FORCE_FEEDBACK, &joystickForceFeedback, false);

	ret = RegSetValueExW(
		regHandle,
		L"EnableJoystickFF",
		0,
		REG_DWORD,
		(BYTE*)&joystickForceFeedback,
		sizeof(joystickForceFeedback));
	ReturnChecker(ret, __LINE__);


	int joystickHit;
	cfg->Read(PRO_CFG_JOYSTICK_DIRECTIONAL, &joystickHit, false);

	ret = RegSetValueExW(
		regHandle,
		L"EnableHitEffect",
		0,
		REG_DWORD,
		(BYTE*)&joystickHit,
		sizeof(joystickHit));
	ReturnChecker(ret, __LINE__);


	int computerspeed;
	cfg->Read(PRO_CFG_VIDEO_GENERAL_SETTINGS, &computerspeed, 4);

	ret = RegSetValueExW(
		regHandle,
		L"ComputerSpeed",
		0,
		REG_DWORD,
		(BYTE*)&computerspeed,
		sizeof(computerspeed));
	ReturnChecker(ret, __LINE__);


	int forcedport;
	cfg->Read(PRO_CFG_NETWORK_PORT, &forcedport, 0);

	ret = RegSetValueExW(
		regHandle,
		L"ForcePort",
		0,
		REG_DWORD,
		(BYTE*)&forcedport,
		sizeof(computerspeed));
	ReturnChecker(ret, __LINE__);

	// PXOBanners

	// ProcessorAffinity

	// PXO folder

	// Network folder
	HKEY networkRegHandle = 0;
	ret = RegCreateKeyEx(
		regHandle,
		L"Network",
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		NULL,
		&networkRegHandle,
		NULL);  // just want handle, don't care if it was created or opened
	ReturnChecker(ret, __LINE__);

	wxString networkIP;
	if ( cfg->Read(PRO_CFG_NETWORK_IP, &networkIP) ) {
		ret = RegSetValueExW(
			networkRegHandle,
			L"CustomIP",
			0,
			REG_SZ,
			(BYTE*)networkIP.c_str(),
			(networkIP.size()+1)*2);
		ReturnChecker(ret, __LINE__);
	} else {
		ret = RegDeleteKeyValueW(
			networkRegHandle,
			NULL,
			L"CustomIP");
		ReturnChecker(ret, __LINE__);
	}

	RegCloseKey(regHandle);
	RegCloseKey(networkRegHandle);
	return ProMan::NoError;
}

ProMan::RegistryCodes PlatformPullProfile(wxFileConfig *cfg) {
	return ProMan::UnknownError;
}