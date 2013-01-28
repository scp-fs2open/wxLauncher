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

#ifndef BASIC_DEFAULTS_H
#define BASIC_DEFAULTS_H

#include <wx/string.h>

/** \defgroup basicdefaults default values for basic settings */
/** @{*/
extern const long DEFAULT_VIDEO_RESOLUTION_WIDTH;
extern const long DEFAULT_VIDEO_RESOLUTION_HEIGHT;
extern const long DEFAULT_VIDEO_BIT_DEPTH;
extern const wxString DEFAULT_VIDEO_TEXTURE_FILTER;
extern const long DEFAULT_VIDEO_ANISOTROPIC;
extern const long DEFAULT_VIDEO_ANTI_ALIAS;

extern const wxString DEFAULT_AUDIO_OPENAL_DEVICE;
extern const wxString DEFAULT_AUDIO_OPENAL_PLAYBACK_DEVICE;
extern const wxString DEFAULT_AUDIO_OPENAL_CAPTURE_DEVICE;
extern const bool DEFAULT_AUDIO_OPENAL_EFX;
extern const long DEFAULT_AUDIO_OPENAL_SAMPLE_RATE;

extern const long DEFAULT_SPEECH_VOICE;
extern const long DEFAULT_SPEECH_VOLUME;
extern const bool DEFAULT_SPEECH_IN_TECHROOM;
extern const bool DEFAULT_SPEECH_IN_BRIEFINGS;
extern const bool DEFAULT_SPEECH_IN_GAME;
extern const bool DEFAULT_SPEECH_IN_MULTI;

extern const long DEFAULT_JOYSTICK_ID;
extern const bool DEFAULT_JOYSTICK_FORCE_FEEDBACK;
extern const bool DEFAULT_JOYSTICK_DIRECTIONAL;

extern const wxString DEFAULT_NETWORK_TYPE;
extern const wxString DEFAULT_NETWORK_SPEED;
extern const long DEFAULT_NETWORK_PORT;
extern const wxString DEFAULT_NETWORK_IP;
/** @}*/

#endif
