#ifndef IDS_HPP
#define IDS_HPP

/* File contains all IDs and other application constantants that are to be used
in the application. */

#define TAB_AREA_WIDTH (639)
#define TAB_AREA_HEIGHT (595)

enum WindowIDS: int {
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
	ID_SPEECH_USING_TECHROOM,
	ID_SPEECH_USING_BRIEFING,
	ID_SPEECH_USING_INGAME,
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

enum MainTabImageIDs: int {
	ID_TAB_WELCOME_IMAGE,
	ID_TAB_MOD_IMAGE,
	ID_TAB_BASIC_SETTINGS_IMAGE,
	ID_TAB_ADV_SETTINGS_IMAGE,
	ID_TAB_INSTALL_IMAGE,
};

enum StatusBarIconIDs: int {
	ID_SB_OK = 0,
	ID_SB_WARNING,
	ID_SB_ERROR,

	ID_SB_MAX_ID,
};

enum ProfileDialogReturnValues: int {
	RETURN_CANCEL,
	RETURN_DO_CLONE,
	RETURN_CONFRIM_DELETE,
};

#endif