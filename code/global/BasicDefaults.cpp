/*
 Copyright (C) 2009-2013 wxLauncher Team
 
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

#include "BasicDefaults.h"

// internal constants
const unsigned int JOYMAN_INVALID_JOYSTICK =	99999;

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
