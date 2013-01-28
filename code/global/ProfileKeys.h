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

#ifndef PROFILE_KEYS_H
#define PROFILE_KEYS_H

#include <wx/stdpaths.h>
#include <wx/string.h>

/** \defgroup Storage Storage locations */
/** @{*/
inline const wxString GetProfileStorageFolder() { return wxStandardPaths::Get().GetUserDataDir(); }
/** @}*/

/** \defgroup Internal Store location */
/** @{ */
#define INT_CONFIG_FILE_LOCATION			_T("/wxlauncher/configlocation")	//!< string
/** @} */

/** \defgroup globalkeys Keys used in global config file */
/** @{*/
extern const wxString GBL_CFG_MAIN_AUTOSAVEPROFILES;	//!< bool
extern const wxString GBL_CFG_MAIN_LASTPROFILE;			//!< string, internal profile name

extern const wxString GBL_CFG_PROXY_TYPE;				//!< string
extern const wxString GBL_CFG_PROXY_SERVER;				//!< string
extern const wxString GBL_CFG_PROXY_PORT;				//!< int

extern const wxString GBL_CFG_NET_DOWNLOAD_NEWS;		//!< bool, true means autodownload
extern const wxString NEWS_LAST_TIME_FORMAT;
extern const wxString GBL_CFG_NET_NEWS_LAST_TIME;		//!< string, formated time as NEWS_LAST_TIME_FORMAT
extern const wxString GBL_CFG_NET_THE_NEWS;				//!< string, the formatted text (workin' for a livin'!)

extern const wxString GBL_CFG_OPT_CONFIG_FRED;			//!< bool, true means show the user the FRED button and allow user to select FRED executable
/** @}*/

/** \defgroup profilekeys Keys used in profiles */
/** @{*/
extern const wxString PRO_CFG_MAIN_NAME;				//!< string, name of profile
extern const wxString PRO_CFG_MAIN_FILENAME;			//!< string, full path to profile
extern const wxString PRO_CFG_MAIN_INITIALIZED;			//!< bool, indicates whether profile has been saved with initial GUI values

extern const wxString PRO_CFG_TC_ROOT_FOLDER;			//!< string, absolute path
extern const wxString PRO_CFG_TC_CURRENT_BINARY;		//!< string, binary name
extern const wxString PRO_CFG_TC_CURRENT_MODLINE;		//!< string, the entire line that should follow -mod
extern const wxString PRO_CFG_TC_CURRENT_MOD;			//!< string, the mod shortname (for modlist)
extern const wxString PRO_CFG_TC_CURRENT_FLAG_LINE;		//!< string, the flags that we as the modline to to make the cmdline
extern const wxString PRO_CFG_TC_CURRENT_FRED;			//!< string, FRED binary's name

extern const wxString PRO_CFG_VIDEO_RESOLUTION_WIDTH;	//!< int
extern const wxString PRO_CFG_VIDEO_RESOLUTION_HEIGHT;	//!< int
extern const wxString CFG_RES_FORMAT_STRING;
extern const wxString PRO_CFG_VIDEO_BIT_DEPTH;			//!< int
extern const wxString PRO_CFG_VIDEO_ANISOTROPIC;		//!< int
extern const wxString PRO_CFG_VIDEO_ANTI_ALIAS;			//!< int
extern const wxString PRO_CFG_VIDEO_TEXTURE_FILTER;		//!< string

extern const wxString PRO_CFG_LIGHTING_PRESET;			//!< string

extern const wxString PRO_CFG_SPEECH_VOICE;				//!< int, same as what the current engine uses
extern const wxString PRO_CFG_SPEECH_VOLUME;			//!< int
extern const wxString PRO_CFG_SPEECH_IN_TECHROOM;		//!< bool
extern const wxString PRO_CFG_SPEECH_IN_BRIEFINGS;		//!< bool
extern const wxString PRO_CFG_SPEECH_IN_GAME;			//!< bool
extern const wxString PRO_CFG_SPEECH_IN_MULTI;			//!< bool

extern const wxString PRO_CFG_NETWORK_TYPE;				//!< string
extern const wxString PRO_CFG_NETWORK_SPEED;			//!< string
extern const wxString PRO_CFG_NETWORK_PORT;				//!< int
extern const wxString PRO_CFG_NETWORK_IP;				//!< string

extern const wxString PRO_CFG_OPENAL_DEVICE;			//!< string
extern const wxString PRO_CFG_OPENAL_CAPTURE_DEVICE;	//!< string
extern const wxString PRO_CFG_OPENAL_EFX;				//!< bool
extern const wxString PRO_CFG_OPENAL_SAMPLE_RATE;		//!< int

extern const wxString PRO_CFG_JOYSTICK_ID;				//!< int
extern const wxString PRO_CFG_JOYSTICK_FORCE_FEEDBACK;	//!< bool
extern const wxString PRO_CFG_JOYSTICK_DIRECTIONAL;		//!< bool
/** @}*/

#endif
