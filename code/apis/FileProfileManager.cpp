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
#include <wx/filename.h>
#include <wx/ffile.h>
#include <wx/wfstream.h>
#include <wx/sstream.h>
#include "generated/configure_launcher.h"
#include "apis/ProfileManager.h"
#include "apis/PlatformProfileManager.h"
#include "apis/FlagListManager.h"
#include "apis/JoystickManager.h"
#include "global/BasicDefaults.h"
#include "global/ProfileKeys.h"
#include "global/RegistryKeys.h"

#include <SDL_filesystem.h>


// NOTE: this function is also used by PushCmdlineFSO() in PlatformProfileManagerShared.cpp
wxFileName GetPlatformDefaultConfigFilePathOld() {
	wxFileName path;
#if IS_WIN32
	path.AssignDir(wxStandardPaths::Get().GetUserConfigDir());
	path.AppendDir(_T("FS2 Open"));
#elif IS_APPLE
	path.AssignHomeDir();
	path.AppendDir(_T("Library"));
	path.AppendDir(_T("FS2_Open"));
#elif IS_LINUX
	path.AssignHomeDir();
	path.AppendDir(_T(".fs2_open"));
#else
# error "One of IS_WIN32, IS_LINUX, IS_APPLE must evaluate to true"
#endif
	return path;
}

wxFileName GetPlatformDefaultConfigFilePathNew() {
	// SDL builds now use the user directory on all platforms
	// The sdl parameters are defined in the FSO code in the file code/osapi.cpp
	char* prefPath = SDL_GetPrefPath("HardLightProductions", "FreeSpaceOpen");

	wxString wxPrefPath = wxString::FromUTF8(prefPath);

	SDL_free(prefPath);

	wxFileName path;
	path.AssignDir(wxPrefPath);

	return path;
}

wxFileName GetPlatformDefaultConfigFilePath(const wxString& tcPath) {
	wxFileName path;
	if (FlagListManager::GetFlagListManager()->GetBuildCaps().sdl) {
		path = GetPlatformDefaultConfigFilePathNew();
	}
	else {
#if IS_LINUX || IS_APPLE // write to folder in home dir
		path = GetPlatformDefaultConfigFilePathOld().GetFullPath().c_str();
#else
		path.AssignDir(tcPath);
#endif
	}

	return path;
}

#define FSO_CONFIG_FILENAME _T("fs2_open.ini")

#define ReturnChecker(retvalue, location) \
	if ( retvalue != true ) {\
		wxLogError(_T("Unhandled error in writing to the configuration file above line %d"), location);\
		return ProMan::UnknownError;\
	}

ProMan::RegistryCodes FilePushProfile(wxFileConfig *cfg) {
	wxFileName configFileName;
	wxString tcPath;
	cfg->Read(PRO_CFG_TC_ROOT_FOLDER, &tcPath);

	if ( cfg->Exists(INT_CONFIG_FILE_LOCATION) ) {
		wxString configFileNameString;
		if (cfg->Read(INT_CONFIG_FILE_LOCATION, &configFileNameString)) {
			configFileName.Assign(configFileNameString);
		} else {
			wxLogError(_T("Unable to retrieve Config File location even though config says key exists"));
			return ProMan::UnknownError;
		}
	} else {
		configFileName = GetPlatformDefaultConfigFilePath(tcPath);
	}
	wxASSERT_MSG( configFileName.Normalize(),
		wxString::Format(_T("Unable to normalize PlatformDefaultConfigFilePath (%s)"),
		configFileName.GetFullPath().c_str()));

	if ( !configFileName.FileExists() && configFileName.DirExists() ) {
		// was given a directory name
		configFileName.SetFullName(FSO_CONFIG_FILENAME);
	}

	wxFFileInputStream configFileInputStream(configFileName.GetFullPath());
	wxStringInputStream configBlankInputStream(_T("")); // in case ini file doesn't exist
	wxInputStream* configInputStreamPtr = &configFileInputStream;
	
	if (!configFileInputStream.IsOk()) {
		wxLogDebug(_T("Could not read from ini file %s, writing new file"),
			configFileName.GetFullPath().c_str());
		configInputStreamPtr = &configBlankInputStream;
	}
	wxFileConfig outConfig(*configInputStreamPtr, wxMBConvUTF8());
	bool ret;
	
	// most settings are written to "Default" folder
	outConfig.SetPath(REG_KEY_DEFAULT_FOLDER_CFG);

	// Video
	int width, height, bitdepth;
	cfg->Read(PRO_CFG_VIDEO_RESOLUTION_WIDTH, &width, DEFAULT_VIDEO_RESOLUTION_WIDTH);
	cfg->Read(PRO_CFG_VIDEO_RESOLUTION_HEIGHT, &height, DEFAULT_VIDEO_RESOLUTION_HEIGHT);
	cfg->Read(PRO_CFG_VIDEO_BIT_DEPTH, &bitdepth, DEFAULT_VIDEO_BIT_DEPTH);

	wxString videocardValue = wxString::Format(_T("OGL -(%dx%d)x%d bit"), width, height, bitdepth);

	ret = outConfig.Write(REG_KEY_VIDEO_RESOLUTION_DEPTH, videocardValue);
	ReturnChecker(ret, __LINE__);

	
	wxString filterMethod;
	cfg->Read(PRO_CFG_VIDEO_TEXTURE_FILTER, &filterMethod, DEFAULT_VIDEO_TEXTURE_FILTER);
	int filterMethodValue = ( filterMethod.StartsWith(_T("Bilinear"))) ? 0 : 1;
	
	ret = outConfig.Write(REG_KEY_VIDEO_TEXTURE_FILTER, filterMethodValue);
	ReturnChecker(ret, __LINE__);
	

	int oglAnisotropicFilter;
	cfg->Read(PRO_CFG_VIDEO_ANISOTROPIC, &oglAnisotropicFilter, DEFAULT_VIDEO_ANISOTROPIC);

	// Caution: FSO expects anisotropic values to be a string,
	// but since we're writing to an .ini file, we can write it out as an int
	ret = outConfig.Write(REG_KEY_VIDEO_ANISOTROPIC, oglAnisotropicFilter);
	ReturnChecker(ret, __LINE__);
	

	int oglAntiAliasSample;
	cfg->Read(PRO_CFG_VIDEO_ANTI_ALIAS, &oglAntiAliasSample, DEFAULT_VIDEO_ANTI_ALIAS);

	ret = outConfig.Write(REG_KEY_VIDEO_ANTI_ALIAS, oglAntiAliasSample);
	ReturnChecker(ret, __LINE__);


	// Audio
	wxString soundDevice;
	cfg->Read(PRO_CFG_OPENAL_DEVICE, &soundDevice, DEFAULT_AUDIO_OPENAL_DEVICE);

	ret = outConfig.Write(REG_KEY_AUDIO_OPENAL_DEVICE, soundDevice);
	ReturnChecker(ret, __LINE__);


	// new sound code settings are written to "Sound" folder
	outConfig.SetPath(REG_KEY_AUDIO_FOLDER_CFG);


	wxString playbackDevice;
	cfg->Read(
		PRO_CFG_OPENAL_DEVICE,
		&playbackDevice,
		DEFAULT_AUDIO_OPENAL_PLAYBACK_DEVICE);

	ret = outConfig.Write(REG_KEY_AUDIO_OPENAL_PLAYBACK_DEVICE, playbackDevice);
	ReturnChecker(ret, __LINE__);


	wxString captureDevice;
	bool hasEntry = cfg->Read(
		PRO_CFG_OPENAL_CAPTURE_DEVICE,
		&captureDevice,
		DEFAULT_AUDIO_OPENAL_CAPTURE_DEVICE);

	if (hasEntry) {
		ret = outConfig.Write(REG_KEY_AUDIO_OPENAL_CAPTURE_DEVICE, captureDevice);
		ReturnChecker(ret, __LINE__);
	}


	int enableEFX;
	hasEntry = cfg->Read(PRO_CFG_OPENAL_EFX, &enableEFX, DEFAULT_AUDIO_OPENAL_EFX);

	if (hasEntry) {
		ret = outConfig.Write(REG_KEY_AUDIO_OPENAL_EFX, enableEFX);
		ReturnChecker(ret, __LINE__);
	}


	int sampleRate;
	cfg->Read(
		PRO_CFG_OPENAL_SAMPLE_RATE,
		&sampleRate,
		DEFAULT_AUDIO_OPENAL_SAMPLE_RATE);

	if (sampleRate != DEFAULT_AUDIO_OPENAL_SAMPLE_RATE) {
		ret = outConfig.Write(REG_KEY_AUDIO_OPENAL_SAMPLE_RATE, sampleRate);
		ReturnChecker(ret, __LINE__);
	}


	outConfig.SetPath(REG_KEY_DEFAULT_FOLDER_CFG);


	// Speech
#if IS_WIN32 // speech is currently not supported in OS X or Linux (although Windows doesn't use this code)
	int speechVoice;
	cfg->Read(PRO_CFG_SPEECH_VOICE, &speechVoice, DEFAULT_SPEECH_VOICE);

	ret = outConfig.Write(REG_KEY_SPEECH_VOICE, speechVoice);
	ReturnChecker(ret, __LINE__);


	int speechVolume;
	cfg->Read(PRO_CFG_SPEECH_VOLUME, &speechVolume, DEFAULT_SPEECH_VOLUME);

	ret = outConfig.Write(REG_KEY_SPEECH_VOLUME, speechVolume);
	ReturnChecker(ret, __LINE__);


	int inTechroom, inBriefings, inGame, inMulti;
	cfg->Read(PRO_CFG_SPEECH_IN_TECHROOM, &inTechroom, DEFAULT_SPEECH_IN_TECHROOM);
	cfg->Read(PRO_CFG_SPEECH_IN_BRIEFINGS, &inBriefings, DEFAULT_SPEECH_IN_BRIEFINGS);
	cfg->Read(PRO_CFG_SPEECH_IN_GAME, &inGame, DEFAULT_SPEECH_IN_GAME);
	cfg->Read(PRO_CFG_SPEECH_IN_MULTI, &inMulti, DEFAULT_SPEECH_IN_MULTI);

	ret = outConfig.Write(REG_KEY_SPEECH_IN_TECHROOM, inTechroom);
	ReturnChecker(ret, __LINE__);

	ret = outConfig.Write(REG_KEY_SPEECH_IN_BRIEFINGS, inBriefings);
	ReturnChecker(ret, __LINE__);

	ret = outConfig.Write(REG_KEY_SPEECH_IN_GAME, inGame);
	ReturnChecker(ret, __LINE__);

	ret = outConfig.Write(REG_KEY_SPEECH_IN_MULTI, inMulti);
	ReturnChecker(ret, __LINE__);
#endif


	// Joystick
	int currentJoystick;
	cfg->Read(PRO_CFG_JOYSTICK_ID, &currentJoystick, DEFAULT_JOYSTICK_ID);

	ret = outConfig.Write(REG_KEY_JOYSTICK_ID, currentJoystick);
	ReturnChecker(ret, __LINE__);

	// Joystick GUID
	wxString currentJoystickGUID = JoyMan::JoystickGUID(currentJoystick);

	ret = outConfig.Write(REG_KEY_JOYSTICK_GUID, currentJoystickGUID);
	ReturnChecker(ret, __LINE__);


	int joystickForceFeedback;
	cfg->Read(
		PRO_CFG_JOYSTICK_FORCE_FEEDBACK,
		&joystickForceFeedback,
		DEFAULT_JOYSTICK_FORCE_FEEDBACK);

	ret = outConfig.Write(REG_KEY_JOYSTICK_FORCE_FEEDBACK, joystickForceFeedback);
	ReturnChecker(ret, __LINE__);


	int joystickHit;
	cfg->Read(PRO_CFG_JOYSTICK_DIRECTIONAL, &joystickHit, DEFAULT_JOYSTICK_DIRECTIONAL);

	ret = outConfig.Write(REG_KEY_JOYSTICK_DIRECTIONAL, joystickHit);
	ReturnChecker(ret, __LINE__);


	// Network
	wxString networkConnectionValue;
	cfg->Read(PRO_CFG_NETWORK_TYPE, &networkConnectionValue, DEFAULT_NETWORK_TYPE);

	ret = outConfig.Write(REG_KEY_NETWORK_TYPE, networkConnectionValue);
	ReturnChecker(ret, __LINE__);


	wxString connectionSpeedValue;
	cfg->Read(PRO_CFG_NETWORK_SPEED, &connectionSpeedValue, DEFAULT_NETWORK_SPEED);

	ret = outConfig.Write(REG_KEY_NETWORK_SPEED, connectionSpeedValue);
	ReturnChecker(ret, __LINE__);


	int forcedport;
	cfg->Read(PRO_CFG_NETWORK_PORT, &forcedport, DEFAULT_NETWORK_PORT);

	if (forcedport != DEFAULT_NETWORK_PORT) {
		ret = outConfig.Write(REG_KEY_NETWORK_PORT, forcedport);
		ReturnChecker(ret, __LINE__);
	} else if (outConfig.Exists(REG_KEY_NETWORK_PORT)) {
		ret = outConfig.DeleteEntry(REG_KEY_NETWORK_PORT, false);
		ReturnChecker(ret, __LINE__);
	}


	// custom IP is written to "Network" folder
	outConfig.SetPath(REG_KEY_NETWORK_FOLDER_CFG);

	wxString networkIP;
	cfg->Read(PRO_CFG_NETWORK_IP, &networkIP, DEFAULT_NETWORK_IP);

	if (networkIP != DEFAULT_NETWORK_IP) {
		ret = outConfig.Write(REG_KEY_NETWORK_IP, networkIP);
		ReturnChecker(ret, __LINE__);
	} else if (outConfig.Exists(REG_KEY_NETWORK_IP)) {
		ret = outConfig.DeleteEntry(REG_KEY_NETWORK_IP, false);
		ReturnChecker(ret, __LINE__);
	}

	outConfig.SetPath(REG_KEY_DEFAULT_FOLDER_CFG);


	wxLogDebug(_T("Writing fs2_open.ini to %s"), configFileName.GetFullPath().c_str());
	wxFFileOutputStream outFileStream(configFileName.GetFullPath());

	outConfig.Save(outFileStream);

	return PushCmdlineFSO(cfg);
}

ProMan::RegistryCodes FilePullProfile(wxFileConfig *cfg) {
	wxFileName inFileName;
	if ( cfg->Exists(INT_CONFIG_FILE_LOCATION) ) {
		wxString inFileNameString;
		if (cfg->Read(INT_CONFIG_FILE_LOCATION, &inFileNameString)) {
			inFileName.Assign(inFileNameString);
		} else {
			wxLogError(_T("Unable to retrieve Config File location even though config says key exists"));
			return ProMan::UnknownError;
		}
	} else {
		inFileName = GetPlatformDefaultConfigFilePathNew();
		inFileName.SetFullName(FSO_CONFIG_FILENAME);

		if (!inFileName.FileExists())
		{
			inFileName = GetPlatformDefaultConfigFilePathOld();
			inFileName.SetFullName(FSO_CONFIG_FILENAME);
		}
	}
	wxASSERT( inFileName.Normalize() );

	if ( !inFileName.FileExists() && inFileName.DirExists() ) {
		// was given a directory name
		inFileName.SetFullName(FSO_CONFIG_FILENAME);
	}

	if (!inFileName.FileExists()) {
		return ProMan::InputFileDoesNotExist;
	}

	wxFFileInputStream inConfigStream(inFileName.GetFullPath(), _T("rb"));
	wxFileConfig inConfig(inConfigStream, wxMBConvUTF8());

	wxString readString;
	int readNumber;


	// Video
	if ( inConfig.Read(REG_KEY_VIDEO_RESOLUTION_DEPTH, &readString) ) {
		// parses VideocardFS2open into its parts
		wxString videoCard(readString);
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

	if ( inConfig.Read(REG_KEY_VIDEO_TEXTURE_FILTER, &readNumber) ) {
		cfg->Write(PRO_CFG_VIDEO_TEXTURE_FILTER, readNumber);
	}

	if ( inConfig.Read(REG_KEY_VIDEO_ANISOTROPIC, &readString) ) {
		long anisotropic;
		// necessary because FSO expects registry value to be a string
		if ( readString.ToLong(&anisotropic) ) {
			cfg->Write(PRO_CFG_VIDEO_ANISOTROPIC, anisotropic);
		}
	}

	if ( inConfig.Read(REG_KEY_VIDEO_ANTI_ALIAS, &readNumber) ) {
		cfg->Write(PRO_CFG_VIDEO_ANTI_ALIAS, readNumber);
	}


	// Audio
	if ( inConfig.Read(REG_KEY_AUDIO_OPENAL_DEVICE, &readString) ) {
		cfg->Write(PRO_CFG_OPENAL_DEVICE, readString);
	}

	if ( inConfig.Read(REG_KEY_AUDIO_OPENAL_PLAYBACK_DEVICE, &readString) &&
			!inConfig.Exists(PRO_CFG_OPENAL_DEVICE)) {
		cfg->Write(PRO_CFG_OPENAL_DEVICE, readString);
	}

	if ( inConfig.Read(REG_KEY_AUDIO_OPENAL_CAPTURE_DEVICE, &readString) ) {
		cfg->Write(PRO_CFG_OPENAL_CAPTURE_DEVICE, readString);
	}

	if ( inConfig.Read(REG_KEY_AUDIO_OPENAL_EFX, &readNumber) ) {
		cfg->Write(PRO_CFG_OPENAL_EFX, readNumber);
	}

	if ( inConfig.Read(REG_KEY_AUDIO_OPENAL_SAMPLE_RATE, &readNumber) ) {
		cfg->Write(PRO_CFG_OPENAL_SAMPLE_RATE, readNumber);
	}


	// Speech
#if IS_WIN32 // Linux/OS X don't yet support speech
	if ( inConfig.Read(REG_KEY_SPEECH_VOICE, &readNumber) ) {
		cfg->Write(PRO_CFG_SPEECH_VOICE, readNumber);
	}

	if ( inConfig.Read(REG_KEY_SPEECH_VOLUME, &readNumber) ) {
		cfg->Write(PRO_CFG_SPEECH_VOLUME, readNumber);
	}
	
	if ( inConfig.Read(REG_KEY_SPEECH_IN_TECHROOM, &readNumber) ) {
		cfg->Write(PRO_CFG_SPEECH_IN_TECHROOM, readNumber);
	}

	if ( inConfig.Read(REG_KEY_SPEECH_IN_BRIEFINGS, &readNumber) ) {
		cfg->Write(PRO_CFG_SPEECH_IN_BRIEFINGS, readNumber);
	}

	if ( inConfig.Read(REG_KEY_SPEECH_IN_GAME, &readNumber) ) {
		cfg->Write(PRO_CFG_SPEECH_IN_GAME, readNumber);
	}

	if ( inConfig.Read(REG_KEY_SPEECH_IN_MULTI, &readNumber) ) {
		cfg->Write(PRO_CFG_SPEECH_IN_MULTI, readNumber);
	}
#endif


	// Joystick
	if ( inConfig.Read(REG_KEY_JOYSTICK_ID, &readNumber) ) {
		cfg->Write(PRO_CFG_JOYSTICK_ID, readNumber);
	}
	
	if ( inConfig.Read(REG_KEY_JOYSTICK_FORCE_FEEDBACK, &readNumber) ) {
		cfg->Write(PRO_CFG_JOYSTICK_FORCE_FEEDBACK, readNumber);
	}
	
	if ( inConfig.Read(REG_KEY_JOYSTICK_DIRECTIONAL, &readNumber) ) {
		cfg->Write(PRO_CFG_JOYSTICK_DIRECTIONAL, readNumber);
	}


	//  Network
	if ( inConfig.Read(REG_KEY_NETWORK_TYPE, &readString) ) {
		cfg->Write(PRO_CFG_NETWORK_TYPE, readString);
	}

	if ( inConfig.Read(REG_KEY_NETWORK_SPEED, &readString) ) {
		cfg->Write(PRO_CFG_NETWORK_SPEED, readString);
	}

	if ( inConfig.Read(REG_KEY_NETWORK_PORT, &readNumber) ) {
		cfg->Write(PRO_CFG_NETWORK_PORT, readNumber);
	}

	if ( inConfig.Read(REG_KEY_NETWORK_IP, &readString) ) {
		cfg->Write(PRO_CFG_NETWORK_IP, readString);
	}


	return ProMan::NoError;
}
