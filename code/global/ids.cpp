/*
 Copyright (C) 2009-2012 wxLauncher Team
 
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

#include "ids.h"

// registry keys
const wxString REG_KEY_FOLDER_LOCATION =			L"Software\\Volition\\Freespace2";

const wxString REG_KEY_VIDEO_RESOLUTION_DEPTH =		L"VideocardFs2open";
const wxString REG_KEY_VIDEO_TEXTURE_FILTER =		L"TextureFilter";
const wxString REG_KEY_VIDEO_ANISOTROPIC =			L"OGL_AnisotropicFilter";
const wxString REG_KEY_VIDEO_ANTI_ALIAS =			L"OGL_AntiAliasSamples";

const wxString REG_KEY_AUDIO_OPENAL_DEVICE =		L"SoundDeviceOAL";
const wxString REG_KEY_AUDIO_FOLDER_CFG =			L"/Sound"; // FileProfileManager
const wxString REG_KEY_AUDIO_FOLDER_REGISTRY =		L"Sound"; // RegistryProfileManager
const wxString REG_KEY_AUDIO_OPENAL_PLAYBACK_DEVICE =L"PlaybackDevice";
const wxString REG_KEY_AUDIO_OPENAL_CAPTURE_DEVICE =L"CaptureDevice";
const wxString REG_KEY_AUDIO_OPENAL_EFX =			L"EnableEFX";
const wxString REG_KEY_AUDIO_OPENAL_SAMPLE_RATE =	L"SampleRate";

const wxString REG_KEY_SPEECH_VOICE =				L"SpeechVoice";
const wxString REG_KEY_SPEECH_VOLUME =				L"SpeechVolume";
const wxString REG_KEY_SPEECH_IN_TECHROOM =			L"SpeechTechroom";
const wxString REG_KEY_SPEECH_IN_BRIEFINGS =		L"SpeechBriefings";
const wxString REG_KEY_SPEECH_IN_GAME =				L"SpeechIngame";
const wxString REG_KEY_SPEECH_IN_MULTI =			L"SpeechMulti";

const wxString REG_KEY_JOYSTICK_ID =				L"CurrentJoystick";
const wxString REG_KEY_JOYSTICK_FORCE_FEEDBACK = 	L"EnableJoystickFF";
const wxString REG_KEY_JOYSTICK_DIRECTIONAL =		L"EnableHitEffect";

const wxString REG_KEY_NETWORK_TYPE =				L"NetworkConnection";
const wxString REG_KEY_NETWORK_SPEED =				L"ConnectionSpeed";
const wxString REG_KEY_NETWORK_PORT =				L"ForcePort";
const wxString REG_KEY_NETWORK_FOLDER_CFG =			L"/Network"; // FileProfileManager
const wxString REG_KEY_DEFAULT_FOLDER_CFG =			L"/Default"; // FileProfileManager
const wxString REG_KEY_NETWORK_FOLDER_REGISTRY =	L"Network"; // RegistryProfileManager
const wxString REG_KEY_NETWORK_IP =					L"CustomIP";

// defaults for basic settings
const long DEFAULT_VIDEO_RESOLUTION_WIDTH =		1024;
const long DEFAULT_VIDEO_RESOLUTION_HEIGHT =	768;
const long DEFAULT_VIDEO_BIT_DEPTH =			32;
const wxString DEFAULT_VIDEO_TEXTURE_FILTER =	_T("Trilinear");
const long DEFAULT_VIDEO_ANISOTROPIC =			0;
const long DEFAULT_VIDEO_ANTI_ALIAS =			0;

const wxString DEFAULT_AUDIO_OPENAL_DEVICE =	_T("no sound");
const wxString DEFAULT_AUDIO_OPENAL_PLAYBACK_DEVICE = wxEmptyString;
const wxString DEFAULT_AUDIO_OPENAL_CAPTURE_DEVICE = wxEmptyString;
const bool DEFAULT_AUDIO_OPENAL_EFX =			false;
const long DEFAULT_AUDIO_OPENAL_SAMPLE_RATE =	0;

const long DEFAULT_SPEECH_VOICE =				0;
const long DEFAULT_SPEECH_VOLUME =				100;
const bool DEFAULT_SPEECH_IN_TECHROOM =			false;
const bool DEFAULT_SPEECH_IN_BRIEFINGS =		false;
const bool DEFAULT_SPEECH_IN_GAME =				false;
const bool DEFAULT_SPEECH_IN_MULTI =			false;

const long DEFAULT_JOYSTICK_ID =				JOYMAN_INVALID_JOYSTICK;
const bool DEFAULT_JOYSTICK_FORCE_FEEDBACK =	false;
const bool DEFAULT_JOYSTICK_DIRECTIONAL =		false;

const wxString DEFAULT_NETWORK_TYPE =			_T("None");
const wxString DEFAULT_NETWORK_SPEED =			_T("None");
const long DEFAULT_NETWORK_PORT =				0;
const wxString DEFAULT_NETWORK_IP =				wxEmptyString;

// constants
const unsigned int JOYMAN_INVALID_JOYSTICK =	99999;
