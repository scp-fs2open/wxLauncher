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
#include "apis/JoystickManager.h"
#include "global/ids.h"

inline wxFileName GetPlatformDefaultConfigFilePath() {
	wxFileName path;
#if IS_WIN32
	path.AssignDir(wxStandardPaths::Get().GetUserConfigDir());
	path.AppendDir(_T("FS2 Open"));
#elif IS_APPLE
	path.AssignDir(wxFileName::GetHomeDir());
	path.AppendDir(_T("Library"));
	path.AppendDir(_T("FS2_Open"));
#elif IS_LINUX
	path.AssignDir(wxFileName::GetHomeDir());
	path.AppendDir(_T(".fs2_open"));
#else
# error "One of IS_WIN32, IS_LINUX, IS_APPLE must evaluate to true"
#endif
	return path;
}

#define FSO_CONFIG_FILENAME _T("fs2_open.ini")

#define ReturnChecker(retvalue, location) \
	if ( retvalue != true ) {\
		wxLogError(_T("Unhandled error in writing to the configuration file above line %d"), location);\
		return ProMan::UnknownError;\
	}

ProMan::RegistryCodes FilePushProfile(wxFileConfig *cfg) {
	wxFileName outFileName;
	if ( cfg->Exists(INT_CONFIG_FILE_LOCATION) ) {
		wxString outFileNameString;
		if (cfg->Read(INT_CONFIG_FILE_LOCATION, &outFileNameString)) {
			outFileName.Assign(outFileNameString);
		} else {
			wxLogError(_T("Unable to retrieve Config File location even though config says key exists"));
			return ProMan::UnknownError;
		}
	} else {
		outFileName = GetPlatformDefaultConfigFilePath();
	}
	wxASSERT_MSG( outFileName.Normalize(),
		wxString::Format(_T("Unable to normalize PlatformDefaultConfigFilePath (%s)"),
		outFileName.GetFullPath().c_str()));

	if ( !outFileName.FileExists() && outFileName.DirExists() ) {
		// was given a directory name
		outFileName.SetFullName(FSO_CONFIG_FILENAME);
	}

	wxStringInputStream inConfigStream(_T(""));
	wxFileConfig outConfig(inConfigStream, wxMBConvUTF8());
	bool ret;	

	int width, height, bitdepth;
	cfg->Read(PRO_CFG_VIDEO_RESOLUTION_WIDTH, &width, 1024);
	cfg->Read(PRO_CFG_VIDEO_RESOLUTION_HEIGHT, &height, 768);
	cfg->Read(PRO_CFG_VIDEO_BIT_DEPTH, &bitdepth, 16);

	wxString videocardValue = wxString::Format(_T("OGL -(%dx%d)x%d bit"), width, height, bitdepth);
	ret = outConfig.Write( L"VideocardFs2open", videocardValue);
	ReturnChecker(ret, __LINE__);	


	wxString soundDevice;
	cfg->Read(PRO_CFG_OPENAL_DEVICE, &soundDevice, _T("Generic Software"));

	ret = outConfig.Write( L"SoundDeviceOAL", soundDevice);
	ReturnChecker(ret, __LINE__);


	// GammaD3D

	// Language

	wxString oglAnisotropicFilter;
	cfg->Read(PRO_CFG_VIDEO_ANISOTROPIC, &oglAnisotropicFilter, _T("0.0"));

	ret = outConfig.Write( L"OGL_AnisotropicFilter", oglAnisotropicFilter);
	ReturnChecker(ret, __LINE__);


	wxString connectionSpeedValue;
	cfg->Read(PRO_CFG_NETWORK_SPEED, &connectionSpeedValue, _T("None"));

	ret = outConfig.Write( L"ConnectionSpeed", connectionSpeedValue);
	ReturnChecker(ret, __LINE__);

	
	wxString networkConnectionValue;
	cfg->Read(PRO_CFG_NETWORK_TYPE, &networkConnectionValue, _T("None"));

	ret = outConfig.Write( L"NetworkConnection", networkConnectionValue);
	ReturnChecker(ret, __LINE__);


	// ImageExportNum
	
	// LowMem

	// ForceFullscreen

	// MaxFPS

	// SoundSampleRate

	// SoundSampleBits

	// ScreenshotNum

#if IS_WIN32 // speech is currently not supported in OS X or Linux (although Windows doesn't use this code)
	int inMulti, inTechroom, inBriefings, inGame;
	cfg->Read(PRO_CFG_SPEECH_IN_BRIEFINGS, &inBriefings, true);
	cfg->Read(PRO_CFG_SPEECH_IN_GAME, &inGame, true);
	cfg->Read(PRO_CFG_SPEECH_IN_MULTI, &inMulti, true);
	cfg->Read(PRO_CFG_SPEECH_IN_TECHROOM, &inTechroom, true);

	ret = outConfig.Write( L"SpeechBriefings", inBriefings);
	ReturnChecker(ret, __LINE__);
	ret = outConfig.Write( L"SpeechIngame", inGame);
	ReturnChecker(ret, __LINE__);
	ret = outConfig.Write( L"SpeechMulti",inMulti);
	ReturnChecker(ret, __LINE__);
	ret = outConfig.Write( L"SpeechTechroom", inTechroom);
	ReturnChecker(ret, __LINE__);

	
	int speechVolume;
	cfg->Read(PRO_CFG_SPEECH_VOLUME, &speechVolume, 100);

	ret = outConfig.Write( L"SpeechVolume", speechVolume);
	ReturnChecker(ret, __LINE__);

	int speechVoice;
	cfg->Read(PRO_CFG_SPEECH_VOICE, &speechVoice, 0);

	ret = outConfig.Write( L"SpeechVoice", speechVoice);
	ReturnChecker(ret, __LINE__);
#endif

	// Fullscreen

	int oglAntiAliasSample;
	cfg->Read(PRO_CFG_VIDEO_ANTI_ALIAS, &oglAntiAliasSample, 0);

	ret = outConfig.Write( L"OGL_AntiAliasSamples", oglAntiAliasSample);

	wxString filterMethod;
	cfg->Read(PRO_CFG_VIDEO_TEXTURE_FILTER, &filterMethod, _T("Trilinear"));
	int filterMethodValue = ( filterMethod.StartsWith(_T("Bilinear"))) ? 0 : 1;

	ret = outConfig.Write( L"TextureFilter", filterMethodValue);
	ReturnChecker(ret, __LINE__);


	int currentJoystick;
	cfg->Read(PRO_CFG_JOYSTICK_ID, &currentJoystick, JOYMAN_INVALID_JOYSTICK);

	ret = outConfig.Write( L"CurrentJoystick", currentJoystick);
	ReturnChecker(ret, __LINE__);

	
	int joystickForceFeedback;
	cfg->Read(PRO_CFG_JOYSTICK_FORCE_FEEDBACK, &joystickForceFeedback, false);

	ret = outConfig.Write( L"EnableJoystickFF", joystickForceFeedback);
	ReturnChecker(ret, __LINE__);


	int joystickHit;
	cfg->Read(PRO_CFG_JOYSTICK_DIRECTIONAL, &joystickHit, false);

	ret = outConfig.Write( L"EnableHitEffect", joystickHit);
	ReturnChecker(ret, __LINE__);


	int forcedport;
	cfg->Read(PRO_CFG_NETWORK_PORT, &forcedport, 0);

	if (forcedport != 0) { // only write if it's a valid port
		ret = outConfig.Write( L"ForcePort", forcedport);
		ReturnChecker(ret, __LINE__);
	}

	// PXOBanners

	// ProcessorAffinity

	// PXO folder

	// Network folder
	if (forcedport != 0) { // only write if it's a valid port
		outConfig.SetPath( L"/Network" );
		
		wxString networkIP;
		if ( cfg->Read(PRO_CFG_NETWORK_IP, &networkIP) ) {
			ret = outConfig.Write( L"CustomIP", networkIP);
			ReturnChecker(ret, __LINE__);
		}
		
		outConfig.SetPath( L"/");
	}

	wxLogDebug(_T("Writing fs2_open.ini to %s"), outFileName.GetFullPath().c_str());
	wxFFileOutputStream outFileStream(outFileName.GetFullPath());
	
	// places all fs2_open.ini entries in the Default group
	outFileStream.Write("[Default]\n", 10);

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
		inFileName = GetPlatformDefaultConfigFilePath();
	}
	wxASSERT( inFileName.Normalize() );

	if ( !inFileName.FileExists() && inFileName.DirExists() ) {
		// was given a directory name
		inFileName.SetFullName(FSO_CONFIG_FILENAME);
	}

	wxFFileInputStream inConfigStream(inFileName.GetFullPath(), _T("rb"));
	wxFileConfig inConfig(inConfigStream, wxMBConvUTF8());
	bool ret;

	wxString readString;
	int readNumber;

	ret = inConfig.Read(L"VideocardFs2open", &readString);
	if ( ret == true ) {
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

	ret = inConfig.Read(L"SoundDeviceOAL", &readString);
	if ( ret == true ) {
		cfg->Write(PRO_CFG_OPENAL_DEVICE, readString);
	}

	// GammaD3D


	// Language



	ret = inConfig.Read(L"OGL_AnisotropicFilter", &readString);
	if ( ret == true ) {
		cfg->Write(PRO_CFG_VIDEO_ANISOTROPIC, readString);
	}

	ret = inConfig.Read(L"ConnectionSpeed", &readString);
	if ( ret == true ) {
		cfg->Write(PRO_CFG_NETWORK_SPEED, readString);
	}

	ret = inConfig.Read(L"NetworkConnection", &readString);
	if ( ret == true ) {
		cfg->Write(PRO_CFG_NETWORK_TYPE, readString);
	}

	ret = inConfig.Read(L"SpeechTechroom", &readNumber);
	if ( ret == true ) {
		cfg->Write(PRO_CFG_SPEECH_IN_TECHROOM, readNumber);
	}

	ret = inConfig.Read(L"SpeechBriefings", &readNumber);
	if ( ret == true ) {
		cfg->Write(PRO_CFG_SPEECH_IN_BRIEFINGS, readNumber);
	}

	ret = inConfig.Read(L"SpeechIngame", &readNumber);
	if ( ret == true ) {
		cfg->Write(PRO_CFG_SPEECH_IN_GAME, readNumber);
	}

	ret = inConfig.Read(L"SpeechMulti", &readNumber);
	if ( ret == true ) {
		cfg->Write(PRO_CFG_SPEECH_IN_MULTI, readNumber);
	}

	ret = inConfig.Read(L"SpeechVolume", &readNumber);
	if ( ret == true ) {
		cfg->Write(PRO_CFG_SPEECH_VOLUME, readNumber);
	}

	ret = inConfig.Read(L"SpeechVoice", &readNumber);
	if ( ret == true ) {
		cfg->Write(PRO_CFG_SPEECH_VOICE, readNumber);
	}

	ret = inConfig.Read(L"OGL_AntiAliasSamples", &readNumber);
	if ( ret == true ) {
		cfg->Write(PRO_CFG_VIDEO_ANTI_ALIAS, readNumber);
	}

	ret = inConfig.Read(L"TextureFilter", &readNumber);
	if ( ret == true ) {
		cfg->Write(PRO_CFG_VIDEO_TEXTURE_FILTER, readNumber);
	}

	ret = inConfig.Read(L"CurrentJoystick", &readNumber);
	if ( ret == true ) {
		cfg->Write(PRO_CFG_JOYSTICK_ID, readNumber);
	}

	ret = inConfig.Read(L"EnableJoystickFF", &readNumber);
	if ( ret == true ) {
		cfg->Write(PRO_CFG_JOYSTICK_FORCE_FEEDBACK, readNumber);
	}

	ret = inConfig.Read(L"EnableHitEffect", &readNumber);
	if ( ret == true ) {
		cfg->Write(PRO_CFG_JOYSTICK_DIRECTIONAL, readNumber);
	}

	ret = inConfig.Read(L"SpeechVoice", &readNumber);
	if ( ret == true ) {
		cfg->Write(PRO_CFG_SPEECH_VOICE, readNumber);
	}

	return ProMan::NoError;
}
