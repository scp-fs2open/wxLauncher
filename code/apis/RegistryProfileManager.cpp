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

#include "apis/ProfileManager.h"
#include "apis/PlatformProfileManager.h"
#include "apis/JoystickManager.h"
#include "global/BasicDefaults.h"
#include "global/ProfileKeys.h"
#include "global/RegistryKeys.h"
#include "generated/configure_launcher.h"

#include <wx/process.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/wfstream.h>
#if PLATFORM_USES_REGISTRY == 1
#include <Windows.h>
#include <Sddl.h>

#if _MSC_VER >= 1800
#include <VersionHelpers.h>
#endif

#endif

#include "global/MemoryDebugging.h"

#if PLATFORM_USES_REGISTRY == 1
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
		
#define UNKOWN_ERROR_MSG _T("Unhandled error number %ld from query above line %d")
#define REG_DATA_NOT_DWORD _T("Registry key lookup above line %d is not a DWORD")
#define REG_DATA_NOT_STRING _T("Registry key lookup above line %d is not a DWORD")
#endif

#if PLATFORM_USES_REGISTRY == 1

typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS)(HANDLE, PBOOL);

class WinAPI
{
private:
	static bool inited;

	static LPFN_ISWOW64PROCESS fnIsWOW64Process;
public:
	static bool IsInited()
	{
		return inited;
	}

	static bool GetUserSID(wxString& outStr)
	{
		HANDLE hToken = NULL;
		if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken) == FALSE)
		{
			wxLogError(_("Failed to get process token. Error Code: 0x%08x"), GetLastError());
			return false;
		}

		DWORD dwBufferSize;
		GetTokenInformation(hToken, TokenUser, NULL, 0, &dwBufferSize);

		PTOKEN_USER ptkUser = (PTOKEN_USER) new BYTE[dwBufferSize];
		if (ptkUser == NULL)
		{
			wxLogError(_T("ptkUser is NULL"));
			return false;
		}

		if (GetTokenInformation(hToken, TokenUser, ptkUser, dwBufferSize, &dwBufferSize) == 0)
		{
			wxLogError(_("Failed to get token information.  Error Code: 0x%08x"), GetLastError());
			return false;
		}

		if (IsValidSid(ptkUser->User.Sid) == FALSE)
		{
			wxLogError(_("SID structure is Invalid"));
			delete[] ptkUser;
			return false;
		}

		LPTSTR sidName = NULL;
		if (ConvertSidToStringSid(ptkUser->User.Sid, &sidName) == 0)
		{
			wxLogError(_("Failed to convert SID structure to string. Error Code: 0x%08x"),
				GetLastError());
			delete[] ptkUser;
			return false;
		}

		outStr.assign(sidName);

		LocalFree(sidName);
		delete[] ptkUser;

		return true;
	}

	static void Init()
	{
		HMODULE hKernel32 = GetModuleHandle(TEXT("Kernel32"));
		fnIsWOW64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(hKernel32, "IsWow64Process");

		/* Use "Version Helper APIs" if available because GetVersionEx may
		not be available in versions of Windows after 8.1 */
#if _MSC_VER >= 1800
		if (IsWindowsVistaOrGreater())
		{
#else
		OSVERSIONINFO versionInfo;
		versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		GetVersionEx(&versionInfo);

		/* The first version of Windows to have registry virtualization is
		Vista, aka 6.0 */
		if (versionInfo.dwMajorVersion >= 6)
		{
#endif
			userSIDValid = GetUserSID(userSID);
		}

		inited = true;
	}

	static bool IsWow64()
	{
		BOOL bIsWow64 = FALSE;
		if (fnIsWOW64Process != NULL)
		{
			if (!fnIsWOW64Process(GetCurrentProcess(), &bIsWow64))
			{
				wxLogDebug(_("Failed to determine if process runs under WOW64"));
			}
		}
		return bIsWow64 == TRUE;
	}

	static wxString userSID;
	static bool userSIDValid;
};

bool WinAPI::inited = false;
bool WinAPI::userSIDValid = false;
wxString WinAPI::userSID(wxT_2(""));
LPFN_ISWOW64PROCESS WinAPI::fnIsWOW64Process = NULL;

HKEY GetRegistryKeyname(wxString& out_keyname)
{
	// Every compiler from Visual Studio 2008 onward should have support for UAC
#if _MSC_VER > 1400
	if (WinAPI::userSIDValid)
	{
		if (WinAPI::IsWow64())
		{
			out_keyname = wxString::Format(wxT_2("%s_Classes\\VirtualStore\\Machine\\Software\\Wow6432Node\\%s"),
				WinAPI::userSID.c_str(), REG_KEY_FOLDER_LOCATION.c_str());
		}
		else
		{
			out_keyname = wxString::Format(wxT_2("%s_Classes\\VirtualStore\\Machine\\Software\\%s"),
				WinAPI::userSID.c_str(), REG_KEY_FOLDER_LOCATION.c_str());
		}
		return HKEY_USERS;
	}
	else
	{
		out_keyname = wxString::Format(wxT_2("Software\\%s"),
			REG_KEY_FOLDER_LOCATION.c_str());
		return HKEY_LOCAL_MACHINE;
	}
#else
	out_keyname = REG_KEY_FOLDER_LOCATION;
	return HKEY_LOCAL_MACHINE;
#endif
}

#endif


/* File contains the Win32 incatations for Pushing and Pulling the passed
profile to/from the registry. */

ProMan::RegistryCodes RegistryPushProfile(wxFileConfig *cfg) {
#if PLATFORM_USES_REGISTRY == 1
	if (!WinAPI::IsInited())
	{
		WinAPI::Init();
	}

	wxString keyName;
	HKEY useKey = GetRegistryKeyname(keyName);

	LONG ret = ERROR_SUCCESS;
	HKEY regHandle = 0;

	ret = RegCreateKeyExW(useKey,
		keyName.wc_str(),
		0, // Reserved
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		NULL, // default security.
		&regHandle,
		NULL // I don't care if the key was created before opening
		);

	if ( ret != ERROR_SUCCESS ) {
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
	}

	// Video
	int width, height, bitdepth;
	cfg->Read(PRO_CFG_VIDEO_RESOLUTION_WIDTH, &width, DEFAULT_VIDEO_RESOLUTION_WIDTH);
	cfg->Read(PRO_CFG_VIDEO_RESOLUTION_HEIGHT, &height, DEFAULT_VIDEO_RESOLUTION_HEIGHT);
	cfg->Read(PRO_CFG_VIDEO_BIT_DEPTH, &bitdepth, DEFAULT_VIDEO_BIT_DEPTH);

	wxString videocardValue = wxString::Format(_T("OGL -(%dx%d)x%d bit"), width, height, bitdepth);
	ret = RegSetValueExW(
		regHandle,
		REG_KEY_VIDEO_RESOLUTION_DEPTH.wc_str(),
		0,
		REG_SZ,
		(BYTE*)videocardValue.wc_str(),
		(videocardValue.size() + 1)*2);
	ReturnChecker(ret, __LINE__);


	wxString filterMethod;
	cfg->Read(PRO_CFG_VIDEO_TEXTURE_FILTER, &filterMethod, DEFAULT_VIDEO_TEXTURE_FILTER);
	int filterMethodValue = ( filterMethod.StartsWith(_T("Bilinear"))) ? 0 : 1;

	ret = RegSetValueExW(
		regHandle,
		REG_KEY_VIDEO_TEXTURE_FILTER.wc_str(),
		0,
		REG_DWORD,
		(BYTE*)&filterMethodValue,
		sizeof(filterMethodValue));
	ReturnChecker(ret, __LINE__);


	int oglAnisotropicFilterInt;
	cfg->Read(PRO_CFG_VIDEO_ANISOTROPIC,
		&oglAnisotropicFilterInt,
		DEFAULT_VIDEO_ANISOTROPIC);

	// Since FSO expects anisotropic to be a string, we must convert it
	wxString oglAnisotropicFilter(
		wxString::Format(_T("%d"), oglAnisotropicFilterInt));

	ret = RegSetValueExW(
		regHandle,
		REG_KEY_VIDEO_ANISOTROPIC.wc_str(),
		0,
		REG_SZ,
		(BYTE*)oglAnisotropicFilter.wc_str(),
		(oglAnisotropicFilter.size() + 1)*2);
	ReturnChecker(ret, __LINE__);


	int oglAntiAliasSample;
	cfg->Read(PRO_CFG_VIDEO_ANTI_ALIAS, &oglAntiAliasSample, DEFAULT_VIDEO_ANTI_ALIAS);
		
	ret = RegSetValueExW(
		regHandle,
		REG_KEY_VIDEO_ANTI_ALIAS.wc_str(),
		0,
		REG_DWORD,
		(BYTE*)&oglAntiAliasSample,
		sizeof(oglAntiAliasSample));
	ReturnChecker(ret, __LINE__);


	// Audio
	wxString soundDevice;
	cfg->Read(PRO_CFG_OPENAL_DEVICE, &soundDevice, DEFAULT_AUDIO_OPENAL_DEVICE);

	ret = RegSetValueExW(
		regHandle,
		REG_KEY_AUDIO_OPENAL_DEVICE.wc_str(),
		0,
		REG_SZ,
		(BYTE*)soundDevice.wc_str(),
		(soundDevice.size() + 1)*2);
	ReturnChecker(ret, __LINE__);


	// Audio folder (for new sound code settings)
	HKEY audioRegHandle = 0;
	ret = RegCreateKeyExW(
		regHandle,
		REG_KEY_AUDIO_FOLDER_REGISTRY.wc_str(),
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		NULL,
		&audioRegHandle,
		NULL);  // just want handle, don't care if it was created or opened
	ReturnChecker(ret, __LINE__);


	wxString playbackDevice;
	cfg->Read(
		PRO_CFG_OPENAL_DEVICE,
		&playbackDevice,
		DEFAULT_AUDIO_OPENAL_PLAYBACK_DEVICE);

	ret = RegSetValueExW(
		audioRegHandle,
		REG_KEY_AUDIO_OPENAL_PLAYBACK_DEVICE.wc_str(),
		0,
		REG_SZ,
		(BYTE*)playbackDevice.wc_str(),
		(playbackDevice.size() + 1)*2);
	ReturnChecker(ret, __LINE__);


	wxString captureDevice;
	bool hasEntry = cfg->Read(
		PRO_CFG_OPENAL_CAPTURE_DEVICE,
		&captureDevice,
		DEFAULT_AUDIO_OPENAL_CAPTURE_DEVICE);

	if (hasEntry) {
		ret = RegSetValueExW(
			audioRegHandle,
			REG_KEY_AUDIO_OPENAL_CAPTURE_DEVICE.wc_str(),
			0,
			REG_SZ,
			(BYTE*)captureDevice.wc_str(),
			(captureDevice.size() + 1)*2);
		ReturnChecker(ret, __LINE__);
	}


	int enableEFX;
	hasEntry = cfg->Read(PRO_CFG_OPENAL_EFX, &enableEFX, DEFAULT_AUDIO_OPENAL_EFX);

	if (hasEntry) {
		ret = RegSetValueExW(
			audioRegHandle,
			REG_KEY_AUDIO_OPENAL_EFX.wc_str(),
			0,
			REG_DWORD,
			(BYTE*)&enableEFX,
			sizeof(enableEFX));
		ReturnChecker(ret, __LINE__);
	}


	int sampleRate;
	cfg->Read(
		PRO_CFG_OPENAL_SAMPLE_RATE,
		&sampleRate,
		DEFAULT_AUDIO_OPENAL_SAMPLE_RATE);

	if (sampleRate != DEFAULT_AUDIO_OPENAL_SAMPLE_RATE) {
		ret = RegSetValueExW(
			audioRegHandle,
			REG_KEY_AUDIO_OPENAL_SAMPLE_RATE.wc_str(),
			0,
			REG_DWORD,
			(BYTE*)&sampleRate,
			sizeof(sampleRate));
		ReturnChecker(ret, __LINE__);
	}


	RegCloseKey(audioRegHandle);


	// Speech
	int speechVoice;
	cfg->Read(PRO_CFG_SPEECH_VOICE, &speechVoice, DEFAULT_SPEECH_VOICE);

	ret = RegSetValueExW(
		regHandle,
		REG_KEY_SPEECH_VOICE.wc_str(),
		0,
		REG_DWORD,
		(BYTE*)&speechVoice,
		sizeof(speechVoice));
	ReturnChecker(ret, __LINE__);


	int speechVolume;
	cfg->Read(PRO_CFG_SPEECH_VOLUME, &speechVolume, DEFAULT_SPEECH_VOLUME);

	ret = RegSetValueExW(
		regHandle,
		REG_KEY_SPEECH_VOLUME.wc_str(),
		0,
		REG_DWORD,
		(BYTE*)&speechVolume,
		sizeof(speechVolume));
	ReturnChecker(ret, __LINE__);


	int inTechroom, inBriefings, inGame, inMulti;
	cfg->Read(PRO_CFG_SPEECH_IN_TECHROOM, &inTechroom, DEFAULT_SPEECH_IN_TECHROOM);
	cfg->Read(PRO_CFG_SPEECH_IN_BRIEFINGS, &inBriefings, DEFAULT_SPEECH_IN_BRIEFINGS);
	cfg->Read(PRO_CFG_SPEECH_IN_GAME, &inGame, DEFAULT_SPEECH_IN_GAME);
	cfg->Read(PRO_CFG_SPEECH_IN_MULTI, &inMulti, DEFAULT_SPEECH_IN_MULTI);

	ret = RegSetValueExW(
		regHandle,
		REG_KEY_SPEECH_IN_TECHROOM.wc_str(),
		0,
		REG_DWORD,
		(BYTE*)&inTechroom,
		sizeof(inTechroom));
	ReturnChecker(ret, __LINE__);

	ret = RegSetValueExW(
		regHandle,
		REG_KEY_SPEECH_IN_BRIEFINGS.wc_str(),
		0,
		REG_DWORD,
		(BYTE*)&inBriefings,
		sizeof(inBriefings));
	ReturnChecker(ret, __LINE__);

	ret = RegSetValueExW(
		regHandle,
		REG_KEY_SPEECH_IN_GAME.wc_str(),
		0,
		REG_DWORD,
		(BYTE*)&inGame,
		sizeof(inGame));
	ReturnChecker(ret, __LINE__);

	ret = RegSetValueExW(
		regHandle,
		REG_KEY_SPEECH_IN_MULTI.wc_str(),
		0,
		REG_DWORD,
		(BYTE*)&inMulti,
		sizeof(inMulti));
	ReturnChecker(ret, __LINE__);


	// Joystick
	int currentJoystick;
	cfg->Read(PRO_CFG_JOYSTICK_ID, &currentJoystick, DEFAULT_JOYSTICK_ID);

	ret = RegSetValueExW(
		regHandle,
		REG_KEY_JOYSTICK_ID.wc_str(),
		0,
		REG_DWORD,
		(BYTE*)&currentJoystick,
		sizeof(currentJoystick));
	ReturnChecker(ret, __LINE__);

	// Joystick GUID
	wxString currentJoystickGUID = JoyMan::JoystickGUID(currentJoystick);

	ret = RegSetValueExW(
		regHandle,
		REG_KEY_JOYSTICK_GUID.wc_str(),
		0,
		REG_SZ,
		(BYTE*)currentJoystickGUID.wc_str(),
		(currentJoystickGUID.size() + 1) * 2);
	ReturnChecker(ret, __LINE__);

	
	int joystickForceFeedback;
	cfg->Read(
		PRO_CFG_JOYSTICK_FORCE_FEEDBACK,
		&joystickForceFeedback,
		DEFAULT_JOYSTICK_FORCE_FEEDBACK);

	ret = RegSetValueExW(
		regHandle,
		REG_KEY_JOYSTICK_FORCE_FEEDBACK.wc_str(),
		0,
		REG_DWORD,
		(BYTE*)&joystickForceFeedback,
		sizeof(joystickForceFeedback));
	ReturnChecker(ret, __LINE__);


	int joystickHit;
	cfg->Read(PRO_CFG_JOYSTICK_DIRECTIONAL, &joystickHit, DEFAULT_JOYSTICK_DIRECTIONAL);

	ret = RegSetValueExW(
		regHandle,
		REG_KEY_JOYSTICK_DIRECTIONAL.wc_str(),
		0,
		REG_DWORD,
		(BYTE*)&joystickHit,
		sizeof(joystickHit));
	ReturnChecker(ret, __LINE__);


	// Network
	wxString networkConnectionValue;
	cfg->Read(PRO_CFG_NETWORK_TYPE, &networkConnectionValue, DEFAULT_NETWORK_TYPE);

	ret = RegSetValueExW(
		regHandle,
		REG_KEY_NETWORK_TYPE.wc_str(),
		0,
		REG_SZ,
		(BYTE*)networkConnectionValue.wc_str(),
		(networkConnectionValue.size() + 1)*2);
	ReturnChecker(ret, __LINE__);


	wxString connectionSpeedValue;
	cfg->Read(PRO_CFG_NETWORK_SPEED, &connectionSpeedValue, DEFAULT_NETWORK_SPEED);

	ret = RegSetValueExW(
		regHandle,
		REG_KEY_NETWORK_SPEED.wc_str(),
		0,
		REG_SZ,
		(BYTE*)connectionSpeedValue.wc_str(),
		(connectionSpeedValue.size() + 1)*2);
	ReturnChecker(ret, __LINE__);


	int forcedport;
	cfg->Read(PRO_CFG_NETWORK_PORT, &forcedport, DEFAULT_NETWORK_PORT);

	if (forcedport != DEFAULT_NETWORK_PORT) {
		ret = RegSetValueExW(
			regHandle,
			REG_KEY_NETWORK_PORT.wc_str(),
			0,
			REG_DWORD,
			(BYTE*)&forcedport,
			sizeof(forcedport));
		ReturnChecker(ret, __LINE__);
	} else {
		wxLogDebug(_T("Forced Port is default. Clearing entry if it exists."));

		ret = RegDeleteValueW(
			regHandle,
			REG_KEY_NETWORK_PORT.wc_str());

		if (ret != ERROR_FILE_NOT_FOUND) { // ignore if entry doesn't exist
			ReturnChecker(ret, __LINE__);
		}
	}


	wxString networkIP;
	cfg->Read(PRO_CFG_NETWORK_IP, &networkIP, DEFAULT_NETWORK_IP);

	// Network folder (for custom IP address)
	HKEY networkRegHandle = 0;
	ret = RegCreateKeyExW(
		regHandle,
		REG_KEY_NETWORK_FOLDER_REGISTRY.wc_str(),
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		NULL,
		&networkRegHandle,
		NULL);  // just want handle, don't care if it was created or opened
	ReturnChecker(ret, __LINE__);

	if (networkIP != DEFAULT_NETWORK_IP) {
		ret = RegSetValueExW(
			networkRegHandle,
			REG_KEY_NETWORK_IP.wc_str(),
			0,
			REG_SZ,
			(BYTE*)networkIP.wc_str(),
			(networkIP.size()+1)*2);
		ReturnChecker(ret, __LINE__);
	} else {
		wxLogDebug(_T("Custom IP is default. Clearing entry if it exists."));

		ret = RegDeleteValueW(
			networkRegHandle,
			REG_KEY_NETWORK_IP.wc_str());

		if (ret != ERROR_FILE_NOT_FOUND) { // ignore if entry doesn't exist
			ReturnChecker(ret, __LINE__);
		}
	}

	RegCloseKey(networkRegHandle);


	RegCloseKey(regHandle);

	return PushCmdlineFSO(cfg);
#else // PLATFORM_USES_REGISTRY
	return ProMan::SupportNotCompiledIn;
#endif // PLATFORM_USES_REGISTRY
}

ProMan::RegistryCodes RegistryPullProfile(wxFileConfig *cfg) {
#if PLATFORM_USES_REGISTRY == 1
	if (!WinAPI::IsInited())
	{
		WinAPI::Init();
	}

	wxString keyName;
	HKEY useKey = GetRegistryKeyname(keyName);

	LONG ret = ERROR_SUCCESS;
	HKEY regHandle = 0;

	ret = RegCreateKeyExW(useKey,
		keyName.wc_str(),
		0, // Reserved
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_READ,
		NULL, // default security.
		&regHandle,
		NULL // I don't care if the key was created before opening
		);

	if ( ret != ERROR_SUCCESS ) {
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
	}
	wxMBConvUTF16 textConv;
	DWORD type = 0;
	BYTE data[MAX_PATH*2];
	memset(static_cast<void*>(data), 0, MAX_PATH*2);
	DWORD dataSize = sizeof(data);

	// Video
	ret = RegQueryValueEx(
		regHandle,
		REG_KEY_VIDEO_RESOLUTION_DEPTH.wc_str(),
		NULL, // Reserved
		&type,
		data,
		&dataSize);
	if ( ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND ) {
		wxLogError(UNKOWN_ERROR_MSG, ret, __LINE__);
		return ProMan::UnknownError;
	} else if ( type != REG_SZ && ret == ERROR_SUCCESS ) {
		wxLogWarning(REG_DATA_NOT_STRING, __LINE__);
	} else {
		// parses VideocardFS2open into its parts
		const char* data1 = reinterpret_cast<char*>(data);
		wxString videoCard(data1, textConv, dataSize);
		wxString rest, rest1, rest2, rest3;

		long width = 0, height = 0, bitdepth = 0;
		if ( videoCard.StartsWith(_T("OGL -("), &rest) ) {
			int xLocation = rest.Find(_T('x'));
			if ( xLocation != wxNOT_FOUND ) {
				wxString widthStr(rest.Mid(0, xLocation));
				rest1 = rest.Mid(xLocation);
				if ( !widthStr.ToLong(&width, 10) ) {
					width = 0;
				}
				int bLocation = rest1.Find(_T(')'));
				if ( bLocation != wxNOT_FOUND ) {
					wxString heightStr(rest1.Mid(0, bLocation));
					rest2 = rest1.Mid(bLocation+1);
					if ( !heightStr.ToLong(&height, 10) ) {
						height = 0;
					}
					int spaceLoc = rest2.Find(_T(' '));
					if ( spaceLoc != wxNOT_FOUND ) {
						wxString bitStr(rest2.Mid(0, spaceLoc));
						if ( !bitStr.ToLong(&bitdepth, 10) ) {
							bitdepth = 0;
						}
					}
				}
			}
		}
		if ( width > 0 ) {
			cfg->Write(PRO_CFG_VIDEO_RESOLUTION_WIDTH, width);
		} 
		if ( height > 0 ) {
			cfg->Write(PRO_CFG_VIDEO_RESOLUTION_HEIGHT, height);
		}
		if ( bitdepth > 0 ) {
			cfg->Write(PRO_CFG_VIDEO_BIT_DEPTH, bitdepth);
		}
	}


	DWORD numberdata;
	
	type = 0;
	numberdata = 0;
	dataSize = sizeof(numberdata);

	ret = RegQueryValueExW(
		regHandle,
		REG_KEY_VIDEO_TEXTURE_FILTER.wc_str(),
		NULL,
		&type,
		reinterpret_cast<LPBYTE>(&numberdata),
		&dataSize);
	if ( ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND ) {
		wxLogError(UNKOWN_ERROR_MSG, ret, __LINE__);
		return ProMan::UnknownError;
	} else if ( type != REG_DWORD && ret == ERROR_SUCCESS) {
		wxLogWarning(REG_DATA_NOT_DWORD, __LINE__);
	} else {
		cfg->Write(PRO_CFG_VIDEO_TEXTURE_FILTER, static_cast<long>(numberdata));
	}


	type = 0;
	memset(static_cast<void*>(data), 0, MAX_PATH*2);
	dataSize = sizeof(data);

	ret = RegQueryValueExW(
		regHandle,
		REG_KEY_VIDEO_ANISOTROPIC.wc_str(),
		NULL,
		&type,
		data,
		&dataSize);
	if ( ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND ) {
		wxLogError(UNKOWN_ERROR_MSG, ret, __LINE__);
		return ProMan::UnknownError;
	} else if ( type != REG_SZ && ret == ERROR_SUCCESS) {
		wxLogWarning(REG_DATA_NOT_STRING, __LINE__);
	} else {
		const char* data1 = reinterpret_cast<char*>(data);
		wxString ani(data1, textConv, dataSize);

		long anisotropic;
		if ( !ani.IsEmpty() && ani.ToLong(&anisotropic)) {
			cfg->Write(PRO_CFG_VIDEO_ANISOTROPIC, anisotropic);
		}
	}


	type = 0;
	numberdata = 0;
	dataSize = sizeof(numberdata);

	ret = RegQueryValueExW(
		regHandle,
		REG_KEY_VIDEO_ANTI_ALIAS.wc_str(),
		NULL,
		&type,
		reinterpret_cast<LPBYTE>(&numberdata),
		&dataSize);
	if ( ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND ) {
		wxLogError(UNKOWN_ERROR_MSG, ret, __LINE__);
		return ProMan::UnknownError;
	} else if ( type != REG_DWORD && ret == ERROR_SUCCESS) {
		wxLogWarning(REG_DATA_NOT_DWORD, __LINE__);
	} else {
		cfg->Write(PRO_CFG_VIDEO_ANTI_ALIAS, static_cast<long>(numberdata));
	}


	// Audio
	type = 0;
	memset(static_cast<void*>(data), 0, MAX_PATH*2);
	dataSize = sizeof(data);

	ret = RegQueryValueExW(
		regHandle,
		REG_KEY_AUDIO_OPENAL_DEVICE.wc_str(),
		NULL,
		&type,
		data,
		&dataSize);
	if ( ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND ) {
		wxLogError(UNKOWN_ERROR_MSG, ret, __LINE__);
		return ProMan::UnknownError;
	} else if ( type != REG_SZ && ret == ERROR_SUCCESS) {
		wxLogWarning(REG_DATA_NOT_STRING, __LINE__);
	} else {
		const char* data1 = reinterpret_cast<char*>(data);
		wxString soundDevice(data1, textConv, dataSize);

		if ( !soundDevice.IsEmpty() ) {
			cfg->Write(PRO_CFG_OPENAL_DEVICE, soundDevice);
		}
	}

	// Audio folder
	HKEY audioRegHandle = 0;
	ret = RegCreateKeyExW(
		regHandle,
		REG_KEY_AUDIO_FOLDER_REGISTRY.wc_str(),
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE | KEY_READ, // need write to make sure we get the virtualized registry when reading.
		NULL,
		&audioRegHandle,
		NULL);  // just want handle, don't care if it was created or opened
	ReturnChecker(ret, __LINE__);


	type = 0;
	memset(static_cast<void*>(data), 0, MAX_PATH*2);
	dataSize = sizeof(data);

	ret = RegQueryValueExW(
		audioRegHandle,
		REG_KEY_AUDIO_OPENAL_PLAYBACK_DEVICE.wc_str(),
		NULL,
		&type,
		data,
		&dataSize);
	if ( ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND ) {
		wxLogError(UNKOWN_ERROR_MSG, ret, __LINE__);
		return ProMan::UnknownError;
	} else if ( type != REG_SZ && ret == ERROR_SUCCESS) {
		wxLogWarning(REG_DATA_NOT_STRING, __LINE__);
	} else {
		const char* data1 = reinterpret_cast<char*>(data);
		wxString playbackDevice(data1, textConv, dataSize);

		if ( !playbackDevice.IsEmpty() && !cfg->Exists(PRO_CFG_OPENAL_DEVICE)) {
			cfg->Write(PRO_CFG_OPENAL_DEVICE, playbackDevice);
		}
	}


	type = 0;
	memset(static_cast<void*>(data), 0, MAX_PATH*2);
	dataSize = sizeof(data);

	ret = RegQueryValueExW(
		audioRegHandle,
		REG_KEY_AUDIO_OPENAL_CAPTURE_DEVICE.wc_str(),
		NULL,
		&type,
		data,
		&dataSize);
	if ( ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND ) {
		wxLogError(UNKOWN_ERROR_MSG, ret, __LINE__);
		return ProMan::UnknownError;
	} else if ( type != REG_SZ && ret == ERROR_SUCCESS) {
		wxLogWarning(REG_DATA_NOT_STRING, __LINE__);
	} else {
		const char* data1 = reinterpret_cast<char*>(data);
		wxString captureDevice(data1, textConv, dataSize);

		if ( !captureDevice.IsEmpty() ) {
			cfg->Write(PRO_CFG_OPENAL_CAPTURE_DEVICE, captureDevice);
		}
	}


	type = 0;
	numberdata = 0;
	dataSize = sizeof(numberdata);

	ret = RegQueryValueExW(
		audioRegHandle,
		REG_KEY_AUDIO_OPENAL_EFX.wc_str(),
		NULL,
		&type,
		reinterpret_cast<LPBYTE>(&numberdata),
		&dataSize);
	if ( ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND ) {
		wxLogError(UNKOWN_ERROR_MSG, ret, __LINE__);
		return ProMan::UnknownError;
	} else if ( type != REG_DWORD && ret == ERROR_SUCCESS) {
		wxLogWarning(REG_DATA_NOT_DWORD, __LINE__);
	} else {
		cfg->Write(PRO_CFG_OPENAL_EFX, static_cast<long>(numberdata));
	}


	type = 0;
	numberdata = 0;
	dataSize = sizeof(numberdata);

	ret = RegQueryValueExW(
		audioRegHandle,
		REG_KEY_AUDIO_OPENAL_SAMPLE_RATE.wc_str(),
		NULL,
		&type,
		reinterpret_cast<LPBYTE>(&numberdata),
		&dataSize);
	if ( ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND ) {
		wxLogError(UNKOWN_ERROR_MSG, ret, __LINE__);
		return ProMan::UnknownError;
	} else if ( type != REG_DWORD && ret == ERROR_SUCCESS) {
		wxLogWarning(REG_DATA_NOT_DWORD, __LINE__);
	} else {
		cfg->Write(PRO_CFG_OPENAL_SAMPLE_RATE, static_cast<long>(numberdata));
	}


	// Speech
	type = 0;
	numberdata = 0;
	dataSize = sizeof(numberdata);

	ret = RegQueryValueExW(
		regHandle,
		REG_KEY_SPEECH_VOICE.wc_str(),
		NULL,
		&type,
		reinterpret_cast<LPBYTE>(&numberdata),
		&dataSize);
	if ( ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND ) {
		wxLogError(UNKOWN_ERROR_MSG, ret, __LINE__);
		return ProMan::UnknownError;
	} else if ( type != REG_DWORD && ret == ERROR_SUCCESS) {
		wxLogWarning(REG_DATA_NOT_DWORD, __LINE__);
	} else {
		cfg->Write(PRO_CFG_SPEECH_VOICE, static_cast<long>(numberdata));
	}


	type = 0;
	numberdata = 0;
	dataSize = sizeof(numberdata);

	ret = RegQueryValueExW(
		regHandle,
		REG_KEY_SPEECH_VOLUME.wc_str(),
		NULL,
		&type,
		reinterpret_cast<LPBYTE>(&numberdata),
		&dataSize);
	if ( ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND ) {
		wxLogError(UNKOWN_ERROR_MSG, ret, __LINE__);
		return ProMan::UnknownError;
	} else if ( type != REG_DWORD && ret == ERROR_SUCCESS) {
		wxLogWarning(REG_DATA_NOT_DWORD, __LINE__);
	} else {
		cfg->Write(PRO_CFG_SPEECH_VOLUME, static_cast<long>(numberdata));
	}


	type = 0;
	numberdata = 0;
	dataSize = sizeof(numberdata);

	ret = RegQueryValueExW(
		regHandle,
		REG_KEY_SPEECH_IN_TECHROOM.wc_str(),
		NULL,
		&type,
		reinterpret_cast<LPBYTE>(&numberdata),
		&dataSize);
	if ( ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND ) {
		wxLogError(UNKOWN_ERROR_MSG, ret, __LINE__);
		return ProMan::UnknownError;
	} else if ( type != REG_DWORD && ret == ERROR_SUCCESS) {
		wxLogWarning(REG_DATA_NOT_DWORD, __LINE__);
	} else {
		cfg->Write(PRO_CFG_SPEECH_IN_TECHROOM, static_cast<long>(numberdata));
	}


	type = 0;
	numberdata = 0;
	dataSize = sizeof(numberdata);

	ret = RegQueryValueExW(
		regHandle,
		REG_KEY_SPEECH_IN_BRIEFINGS.wc_str(),
		NULL,
		&type,
		reinterpret_cast<LPBYTE>(&numberdata),
		&dataSize);
	if ( ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND ) {
		wxLogError(UNKOWN_ERROR_MSG, ret, __LINE__);
		return ProMan::UnknownError;
	} else if ( type != REG_DWORD && ret == ERROR_SUCCESS) {
		wxLogWarning(REG_DATA_NOT_DWORD, __LINE__);
	} else {
		cfg->Write(PRO_CFG_SPEECH_IN_BRIEFINGS, static_cast<long>(numberdata));
	}


	type = 0;
	numberdata = 0;
	dataSize = sizeof(numberdata);

	ret = RegQueryValueExW(
		regHandle,
		REG_KEY_SPEECH_IN_GAME.wc_str(),
		NULL,
		&type,
		reinterpret_cast<LPBYTE>(&numberdata),
		&dataSize);
	if ( ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND ) {
		wxLogError(UNKOWN_ERROR_MSG, ret, __LINE__);
		return ProMan::UnknownError;
	} else if ( type != REG_DWORD && ret == ERROR_SUCCESS) {
		wxLogWarning(REG_DATA_NOT_DWORD, __LINE__);
	} else {
		cfg->Write(PRO_CFG_SPEECH_IN_GAME, static_cast<long>(numberdata));
	}


	type = 0;
	numberdata = 0;
	dataSize = sizeof(numberdata);

	ret = RegQueryValueExW(
		regHandle,
		REG_KEY_SPEECH_IN_MULTI.wc_str(),
		NULL,
		&type,
		reinterpret_cast<LPBYTE>(&numberdata),
		&dataSize);
	if ( ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND ) {
		wxLogError(UNKOWN_ERROR_MSG, ret, __LINE__);
		return ProMan::UnknownError;
	} else if ( type != REG_DWORD && ret == ERROR_SUCCESS) {
		wxLogWarning(REG_DATA_NOT_DWORD, __LINE__);
	} else {
		cfg->Write(PRO_CFG_SPEECH_IN_MULTI, static_cast<long>(numberdata));
	}

		
	// Joystick
	type = 0;
	numberdata = 0;
	dataSize = sizeof(numberdata);

	ret = RegQueryValueExW(
		regHandle,
		REG_KEY_JOYSTICK_ID.wc_str(),
		NULL,
		&type,
		reinterpret_cast<LPBYTE>(&numberdata),
		&dataSize);
	if ( ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND ) {
		wxLogError(UNKOWN_ERROR_MSG, ret, __LINE__);
		return ProMan::UnknownError;
	} else if ( type != REG_DWORD && ret == ERROR_SUCCESS) {
		wxLogWarning(REG_DATA_NOT_DWORD, __LINE__);
	} else {
		cfg->Write(PRO_CFG_JOYSTICK_ID, static_cast<long>(numberdata));
	}


	type = 0;
	numberdata = 0;
	dataSize = sizeof(numberdata);

	ret = RegQueryValueExW(
		regHandle,
		REG_KEY_JOYSTICK_FORCE_FEEDBACK.wc_str(),
		NULL,
		&type,
		reinterpret_cast<LPBYTE>(&numberdata),
		&dataSize);
	if ( ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND ) {
		wxLogError(UNKOWN_ERROR_MSG, ret, __LINE__);
		return ProMan::UnknownError;
	} else if ( type != REG_DWORD && ret == ERROR_SUCCESS) {
		wxLogWarning(REG_DATA_NOT_DWORD, __LINE__);
	} else {
		cfg->Write(PRO_CFG_JOYSTICK_FORCE_FEEDBACK, static_cast<long>(numberdata));
	}


	type = 0;
	numberdata = 0;
	dataSize = sizeof(numberdata);

	ret = RegQueryValueExW(
		regHandle,
		REG_KEY_JOYSTICK_DIRECTIONAL.wc_str(),
		NULL,
		&type,
		reinterpret_cast<LPBYTE>(&numberdata),
		&dataSize);
	if ( ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND ) {
		wxLogError(UNKOWN_ERROR_MSG, ret, __LINE__);
		return ProMan::UnknownError;
	} else if ( type != REG_DWORD && ret == ERROR_SUCCESS) {
		wxLogWarning(REG_DATA_NOT_DWORD, __LINE__);
	} else {
		cfg->Write(PRO_CFG_JOYSTICK_DIRECTIONAL, static_cast<long>(numberdata));
	}
		

	// Network
	type = 0;
	memset(static_cast<void*>(data), 0, MAX_PATH*2);
	dataSize = sizeof(data);

	ret = RegQueryValueExW(
		regHandle,
		REG_KEY_NETWORK_TYPE.wc_str(),
		NULL,
		&type,
		data,
		&dataSize);
	if ( ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND ) {
		wxLogError(UNKOWN_ERROR_MSG, ret, __LINE__);
		return ProMan::UnknownError;
	} else if ( type != REG_SZ && ret == ERROR_SUCCESS) {
		wxLogWarning(REG_DATA_NOT_STRING, __LINE__);
	} else {
		const char* data1 = reinterpret_cast<char*>(data);
		wxString connection(data1, textConv, dataSize);

		if ( !connection.IsEmpty() ) {
			cfg->Write(PRO_CFG_NETWORK_TYPE, connection);
		}
	}


	type = 0;
	memset(static_cast<void*>(data), 0, MAX_PATH*2);
	dataSize = sizeof(data);

	ret = RegQueryValueExW(
		regHandle,
		REG_KEY_NETWORK_SPEED.wc_str(),
		NULL,
		&type,
		data,
		&dataSize);
	if ( ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND ) {
		wxLogError(UNKOWN_ERROR_MSG, ret, __LINE__);
		return ProMan::UnknownError;
	} else if ( type != REG_SZ && ret == ERROR_SUCCESS) {
		wxLogWarning(REG_DATA_NOT_STRING, __LINE__);
	} else {
		const char* data1 = reinterpret_cast<char*>(data);
		wxString speed(data1, textConv, dataSize);

		if ( !speed.IsEmpty() ) {
			cfg->Write(PRO_CFG_NETWORK_SPEED, speed);
		}
	}


	type = 0;
	numberdata = 0;
	dataSize = sizeof(numberdata);

	ret = RegQueryValueExW(
		regHandle,
		REG_KEY_NETWORK_PORT.wc_str(),
		NULL,
		&type,
		reinterpret_cast<LPBYTE>(&numberdata),
		&dataSize);
	if ( ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND ) {
		wxLogError(UNKOWN_ERROR_MSG, ret, __LINE__);
		return ProMan::UnknownError;
	} else if ( type != REG_DWORD && ret == ERROR_SUCCESS) {
		wxLogWarning(REG_DATA_NOT_DWORD, __LINE__);
	} else {
		cfg->Write(PRO_CFG_NETWORK_PORT, static_cast<long>(numberdata));
	}


	// Network folder
	HKEY networkRegHandle = 0;
	ret = RegCreateKeyExW(
		regHandle,
		REG_KEY_NETWORK_FOLDER_REGISTRY.wc_str(),
		0,
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE | KEY_READ, // need write to make sure we get the virtualized registry when reading.
		NULL,
		&networkRegHandle,
		NULL);  // just want handle, don't care if it was created or opened
	ReturnChecker(ret, __LINE__);


	type = 0;
	memset(static_cast<void*>(data), 0, MAX_PATH*2);
	dataSize = sizeof(data);

	ret = RegQueryValueExW(
		networkRegHandle,
		REG_KEY_NETWORK_IP.wc_str(),
		NULL,
		&type,
		data,
		&dataSize);
	if ( ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND ) {
		wxLogError(UNKOWN_ERROR_MSG, ret, __LINE__);
		return ProMan::UnknownError;
	} else if ( type != REG_SZ && ret == ERROR_SUCCESS) {
		wxLogWarning(REG_DATA_NOT_STRING, __LINE__);
	} else {
		const char* data1 = reinterpret_cast<char*>(data);
		wxString ip(data1, textConv, dataSize);

		if ( !ip.IsEmpty() ) {
			cfg->Write(PRO_CFG_NETWORK_IP, ip);
		}
	}


	RegCloseKey(audioRegHandle);
	RegCloseKey(networkRegHandle);
	RegCloseKey(regHandle);
		
	return ProMan::NoError;
#else // PLATFORM_USES_REGISTRY
	return ProMan::SupportNotCompiledIn;
#endif // PLATFORM_USES_REGISTRY
}
