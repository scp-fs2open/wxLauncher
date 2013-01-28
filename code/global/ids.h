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

#ifndef IDS_HPP
#define IDS_HPP

/* File contains all IDs and other application constants that are to be used
in the application. */

#define TAB_AREA_WIDTH (639)	//!< the number of pixels wide for the useable area of the tab
#define TAB_AREA_HEIGHT (595)	//!< the number of pixels high for the useable area of the tab

/** The IDs that wxLauncher uses to identify its controls to wxWidgets.  These
are in no particular order, but they are grouped into the tab or section of the
the control that it represents is located on.  

These IDs are also used by the help manager to open the correct help page 
for the context help. */
enum WindowIDS {
	ID_MAINTAB = wxID_HIGHEST + 1, /* wxID_HIGHEST is the maximum event id that wx
							   will use. */
	ID_CLOSE_BUTTON,
	ID_HELP_BUTTON,
	ID_FRED_BUTTON,
	ID_UPDATE_BUTTON,
	ID_PLAY_BUTTON,
	ID_ABOUT_BUTTON,
	ID_FS2_PROCESS,
	ID_FRED2_PROCESS,

	ID_PROFILE_COMBO,
	ID_NEW_PROFILE,
	ID_DELETE_PROFILE,
	ID_SAVE_PROFILE,
	ID_SAVE_DEFAULT_CHECK,
	ID_SUMMARY_HTML_PANEL,
	ID_HEADLINES_HTML_PANEL,

	ID_VIDEO_STATIC_BOX,
	ID_RESOLUTION_COMBO,
	ID_DEPTH_COMBO,
	ID_TEXTURE_FILTER_COMBO,
	ID_ANISOTROPIC_COMBO,
	ID_AA_COMBO,

	// Lighting presets
	ID_PRESETS_OFF,
	ID_PRESET_BASELINE,
	ID_PRESET_DABRAIN,
	ID_PRESET_HERRA_TOHTORI,
	ID_PRESET_CKID,
	ID_PRESET_COLECAMPBELL666,
	ID_PRESET_CASTOR,
	ID_PRESET_SPIDEY,
	ID_PRESET_WOOLIE_WOOL,
	ID_COPY_PRESET_BUTTON,

	ID_SPEECH_TEST_TEXT,
	ID_SPEECH_VOICE_COMBO,
	ID_SPEECH_VOICE_VOLUME,
	ID_SPEECH_PLAY_BUTTON,
	ID_SPEECH_IN_TECHROOM,
	ID_SPEECH_IN_BRIEFING,
	ID_SPEECH_IN_GAME,
	ID_SPEECH_IN_MULTI,
	ID_SPEECH_MORE_VOICES_BUTTON,

	ID_NETWORK_TYPE,
	ID_NETWORK_SPEED,
	ID_NETWORK_PORT,
	ID_NETWORK_IP,

	ID_SELECT_SOUND_DEVICE,
	ID_SELECT_CAPTURE_DEVICE,
	ID_ENABLE_EFX,
	ID_AUDIO_SAMPLE_RATE,
	ID_DOWNLOAD_OPENAL,
	ID_DETECT_OPENAL,

	ID_JOY_SELECTED,
	ID_JOY_FORCE_FEEDBACK,
	ID_JOY_DIRECTIONAL_HIT,
	ID_JOY_CALIBRATE_BUTTON,
	ID_JOY_DETECT_BUTTON,

	ID_PROXY_TYPE,
	ID_PROXY_HTTP_SERVER,
	ID_PROXY_HTTP_PORT,
	ID_PROXY_AUTO_URL,

	ID_MODLISTBOX,
	ID_MODLISTBOX_ACTIVATE_BUTTON,
	ID_MODLISTBOX_INFO_BUTTON,

	ID_STATUSBAR_STATUS_ICON,
	ID_STATUSBAR_PROGRESS_BAR,

	ID_EXE_ROOT_FOLDER_BOX_TEXT,
	ID_EXE_ROOT_FOLDER_BOX,
	ID_EXE_SELECT_ROOT_BUTTON,
	ID_EXE_CHOICE_BOX,
	ID_EXE_CHOICE_REFRESH_BUTTON,
	ID_EXE_FRED_CHOICE_BOX,
	ID_EXE_FRED_CHOICE_REFRESH_BUTTON,

	ID_CLONE_PROFILE_DIALOG,
	ID_CLONE_PROFILE_NEWNAME,
	ID_CLONE_PROFILE_CHECKBOX,
	ID_DELETE_PROFILE_DIALOG,

	// Advanced settings page
	ID_FLAGLISTBOX,
	ID_SELECT_FLAG_SET,
	ID_CUSTOM_FLAGS_TEXT,
	ID_COMMAND_LINE_TEXT,
	ID_FLAG_SET_NOTES_TEXT,

	ID_NET_DOWNLOAD_NEWS,
	ID_EVENT_NET_DOWNLOAD_NEWS,
	ID_MORE_INFO_PRIVACY,
};

enum MainTabImageIDs {
	ID_TAB_WELCOME_IMAGE,
	ID_TAB_MOD_IMAGE,
	ID_TAB_BASIC_SETTINGS_IMAGE,
	ID_TAB_ADV_SETTINGS_IMAGE,
	ID_TAB_INSTALL_IMAGE,
};

enum StatusBarIconIDs {
	ID_SB_NO_CHANGE = -1,
	ID_SB_INFO,
	ID_SB_OK,
	ID_SB_WARNING,
	ID_SB_ERROR,

	ID_SB_MAX_ID,
};

#endif
