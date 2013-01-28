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

#include "ProfileKeys.h"

// Global profile keys and constants
const wxString GBL_CFG_MAIN_AUTOSAVEPROFILES	(_T("/main/autosaveprofiles"));
const wxString GBL_CFG_MAIN_LASTPROFILE			(_T("/main/lastprofile"));

const wxString GBL_CFG_PROXY_TYPE				(_T("/proxy/type"));
const wxString GBL_CFG_PROXY_SERVER				(_T("/proxy/server"));
const wxString GBL_CFG_PROXY_PORT				(_T("/proxy/port"));

const wxString GBL_CFG_NET_DOWNLOAD_NEWS		(_T("/net/downloadnews"));
const wxString NEWS_LAST_TIME_FORMAT			(_T("%Y %j %H %M %S"));
const wxString GBL_CFG_NET_NEWS_LAST_TIME		(_T("/net/lastdownloadnews"));
const wxString GBL_CFG_NET_THE_NEWS				(_T("/net/thenews)"));

const wxString GBL_CFG_OPT_CONFIG_FRED			(_T("/opt/configfred"));

// Profile keys and constants
const wxString PRO_CFG_MAIN_NAME				(_T("/main/name"));
const wxString PRO_CFG_MAIN_FILENAME			(_T("/main/filename"));
const wxString PRO_CFG_MAIN_INITIALIZED			(_T("/main/initialized"));

const wxString PRO_CFG_TC_ROOT_FOLDER			(_T("/tc/folder"));
const wxString PRO_CFG_TC_CURRENT_BINARY		(_T("/tc/currentbinary"));
const wxString PRO_CFG_TC_CURRENT_MODLINE		(_T("/tc/currentmodline"));
const wxString PRO_CFG_TC_CURRENT_MOD			(_T("/tc/currentmod"));
const wxString PRO_CFG_TC_CURRENT_FLAG_LINE		(_T("/tc/flags"));
const wxString PRO_CFG_TC_CURRENT_FRED			(_T("/tc/currentfred"));

const wxString PRO_CFG_VIDEO_RESOLUTION_WIDTH	(_T("/video/width"));
const wxString PRO_CFG_VIDEO_RESOLUTION_HEIGHT	(_T("/video/height"));
const wxString CFG_RES_FORMAT_STRING			(_T("%d x %d"));
const wxString PRO_CFG_VIDEO_BIT_DEPTH			(_T("/video/depth"));
const wxString PRO_CFG_VIDEO_ANISOTROPIC		(_T("/video/anisotropic"));
const wxString PRO_CFG_VIDEO_ANTI_ALIAS			(_T("/video/antialias"));
const wxString PRO_CFG_VIDEO_TEXTURE_FILTER		(_T("/video/texturefilter"));

const wxString PRO_CFG_LIGHTING_PRESET			(_T("/lighting/preset"));

const wxString PRO_CFG_SPEECH_VOICE				(_T("/speech/voice"));
const wxString PRO_CFG_SPEECH_VOLUME			(_T("/speech/volume"));
const wxString PRO_CFG_SPEECH_IN_TECHROOM		(_T("/speech/intechroom"));
const wxString PRO_CFG_SPEECH_IN_BRIEFINGS		(_T("/speech/inbriefings"));
const wxString PRO_CFG_SPEECH_IN_GAME			(_T("/speech/ingame"));
const wxString PRO_CFG_SPEECH_IN_MULTI			(_T("/speech/inmulti"));

const wxString PRO_CFG_NETWORK_TYPE				(_T("/network/type"));
const wxString PRO_CFG_NETWORK_SPEED			(_T("/network/speed"));
const wxString PRO_CFG_NETWORK_PORT				(_T("/network/port"));
const wxString PRO_CFG_NETWORK_IP				(_T("/network/ip"));

const wxString PRO_CFG_OPENAL_DEVICE			(_T("/openal/device"));
const wxString PRO_CFG_OPENAL_CAPTURE_DEVICE	(_T("/openal/capturedevice"));
const wxString PRO_CFG_OPENAL_EFX				(_T("/openal/efx"));
const wxString PRO_CFG_OPENAL_SAMPLE_RATE		(_T("/openal/samplerate"));

const wxString PRO_CFG_JOYSTICK_ID				(_T("/joystick/id"));
const wxString PRO_CFG_JOYSTICK_FORCE_FEEDBACK	(_T("/joystick/forcefeedback"));
const wxString PRO_CFG_JOYSTICK_DIRECTIONAL		(_T("/joystick/directional"));
/** @}*/
