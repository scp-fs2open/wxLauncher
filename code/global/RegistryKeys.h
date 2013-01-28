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

#ifndef REGISTRY_KEYS_H
#define REGISTRY_KEYS_H

#include <wx/string.h>

/** \defgroup registrykeys Keys used in the registry or fs2_open.ini */
/** @{*/
extern const wxString REG_KEY_FOLDER_LOCATION;

extern const wxString REG_KEY_VIDEO_RESOLUTION_DEPTH;
extern const wxString REG_KEY_VIDEO_TEXTURE_FILTER;
extern const wxString REG_KEY_VIDEO_ANISOTROPIC;
extern const wxString REG_KEY_VIDEO_ANTI_ALIAS;

extern const wxString REG_KEY_AUDIO_OPENAL_DEVICE;
extern const wxString REG_KEY_AUDIO_FOLDER_CFG;
extern const wxString REG_KEY_AUDIO_FOLDER_REGISTRY;
extern const wxString REG_KEY_AUDIO_OPENAL_PLAYBACK_DEVICE;
extern const wxString REG_KEY_AUDIO_OPENAL_CAPTURE_DEVICE;
extern const wxString REG_KEY_AUDIO_OPENAL_EFX;
extern const wxString REG_KEY_AUDIO_OPENAL_SAMPLE_RATE;

extern const wxString REG_KEY_SPEECH_VOICE;
extern const wxString REG_KEY_SPEECH_VOLUME;
extern const wxString REG_KEY_SPEECH_IN_TECHROOM;
extern const wxString REG_KEY_SPEECH_IN_BRIEFINGS;
extern const wxString REG_KEY_SPEECH_IN_GAME;
extern const wxString REG_KEY_SPEECH_IN_MULTI;

extern const wxString REG_KEY_JOYSTICK_ID;
extern const wxString REG_KEY_JOYSTICK_FORCE_FEEDBACK;
extern const wxString REG_KEY_JOYSTICK_DIRECTIONAL;

extern const wxString REG_KEY_NETWORK_TYPE;
extern const wxString REG_KEY_NETWORK_SPEED;
extern const wxString REG_KEY_NETWORK_PORT;
extern const wxString REG_KEY_NETWORK_FOLDER_CFG;
extern const wxString REG_KEY_DEFAULT_FOLDER_CFG;
extern const wxString REG_KEY_NETWORK_FOLDER_REGISTRY;
extern const wxString REG_KEY_NETWORK_IP;
/** @}*/

#endif
