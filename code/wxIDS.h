#ifndef IDS_HPP
#define IDS_HPP

/* File contains all IDs and other application constantants that are to be used
in the application. */

#define TAB_AREA_WIDTH (639)
#define TAB_AREA_HEIGHT (595)

enum WindowIDS {
	ID_MAINTAB = wxID_HIGHEST, /* wxID_HIGHEST is the maximum event id that wx
							   will use. */
	ID_CLOSE_BUTTON,
	ID_HELP_BUTTON,
	ID_FRED_BUTTON,
	ID_UPDATE_BUTTON,
	ID_PLAY_BUTTON,
	ID_ABOUT_BUTTON,
	ID_PROFILE_COMBO,
	ID_NEW_PROFILE,
	ID_DELETE_PROFILE,
	ID_SAVE_PROFILE,
	ID_SAVE_DEFAULT_CHECK,
	ID_SUMMARY_HTML_PANEL,
	ID_HEADLINES_HTML_PANEL,

	ID_VIDEO_STATIC_BOX,
	ID_GRAPHICS_COMBO,
	ID_RESOLUTION_COMBO,
	ID_DEPTH_COMBO,
	ID_TEXTURE_FILTER_COMBO,
	ID_ANISOTROPIC_COMBO,
	ID_AA_COMBO,
	ID_GS_COMBO,
	ID_LARGE_TEXTURE_CHECK,
	ID_FONT_DISTORTION_CHECK,

	ID_SPEECH_TEST_TEXT,
	ID_SPEECH_VOICE_COMBO,
	ID_SPEECH_VOICE_VOLUME,
	ID_SPEECH_PLAY_BUTTON,
	ID_SPEECH_IN_TECHROOM,
	ID_SPEECH_IN_BRIEFING,
	ID_SPEECH_IN_GAME,
	ID_SPEECH_MORE_VOICES_BUTTON,

	ID_NETWORK_TYPE,
	ID_NETWORK_SPEED,
	ID_NETWORK_PORT,
	ID_NETWORK_IP,

	ID_SELECT_SOUND_DEVICE,
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

	ID_EXE_ROOT_FOLDER_BOX,
	ID_EXE_SELECT_ROOT_BUTTON,
	ID_EXE_CHOICE_BOX,

	ID_CLONE_PROFILE_DIALOG,
	ID_DELETE_PROFILE_DIALOG,
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

enum ProfileDialogReturnValues {
	RETURN_CANCEL,
	RETURN_DO_CLONE,
	RETURN_CONFRIM_DELETE,
};

// Storage locations
#define GET_PROFILE_STORAGEFOLDER()			wxStandardPaths::Get().GetUserDataDir()

// Keys used in global config file
#define GBL_CFG_MAIN_AUTOSAVEPROFILES		_T("/main/autosaveprofiles")
#define GBL_CFG_MAIN_LASTPROFILE			_T("/main/lastprofile")

#define GBL_CFG_PROXY_TYPE					_T("/proxy/type")	// string
#define GBL_CFG_PROXY_SERVER				_T("/proxy/server")	// string
#define GBL_CFG_PROXY_PORT					_T("/proxy/port")	// int

// Keys used in profiles
#define PRO_CFG_MAIN_NAME					_T("/main/name")
#define PRO_CFG_MAIN_FILENAME				_T("/main/filename")

#define PRO_CFG_TC_ROOT_FOLDER				_T("/tc/folder")
#define PRO_CFG_TC_CURRENT_BINARY			_T("/tc/currentbinary")

#define PRO_CFG_VIDEO_API					_T("/video/api")		// string
#define PRO_CFG_VIDEO_RESOLUTION_WIDTH		_T("/video/width")		// int
#define PRO_CFG_VIDEO_RESOLUTION_HEIGHT		_T("/video/height")		// int
#define CFG_RES_FORMAT_STRING				_T("%d x %d")
#define PRO_CFG_VIDEO_BIT_DEPTH				_T("/video/depth")		// int
#define PRO_CFG_VIDEO_ANISOTROPIC			_T("/video/anistropic")	// int
#define PRO_CFG_VIDEO_ANTI_ALIAS			_T("/video/antialias")	// int
#define PRO_CFG_VIDEO_TEXTURE_FILTER		_T("/video/texturefilter")	// string
#define PRO_CFG_VIDEO_GENERAL_SETTINGS		_T("/video/general")	// int
#define PRO_CFG_VIDEO_USE_LARGE_TEXTURES	_T("/video/largetextures")	// bool
#define PRO_CFG_VIDEO_FIX_FONT_DISTORTION	_T("/video/fixfontdistortion")	// bool

#define PRO_CFG_SPEECH_VOICE				_T("/speech/voice")		// int, same as what the current engine uses
#define PRO_CFG_SPEECH_VOLUME				_T("/speech/volume")	// int
#define PRO_CFG_SPEECH_IN_TECHROOM			_T("/speech/intechroom")// bool
#define PRO_CFG_SPEECH_IN_BRIEFINGS			_T("/speech/inbriefings")// bool
#define PRO_CFG_SPEECH_IN_GAME				_T("/speech/ingame")	// bool

#define PRO_CFG_NETWORK_TYPE				_T("/network/type")		// string
#define PRO_CFG_NETWORK_SPEED				_T("/network/speed")	// string
#define PRO_CFG_NETWORK_PORT				_T("/network/prot")		// int
#define PRO_CFG_NETWORK_IP					_T("/network/ip")		// string

#define PRO_CFG_OPENAL_DEVICE				_T("/openal/device")	// string

#define PRO_CFG_JOYSTICK_ID					_T("/joystick/id")		// int
#define PRO_CFG_JOYSTICK_FORCE_FEEDBACK		_T("/joystick/forcefeedback") // bool
#define PRO_CFG_JOYSTICK_DIRECTIONAL		_T("/joystick/directional") // bool
#endif