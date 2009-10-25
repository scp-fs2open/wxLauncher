#ifndef IDS_HPP
#define IDS_HPP

/* File contains all IDs that are to be used in the application. */

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
	ID_SAVE_DEFAULT_COMBO,
	ID_SUMMARY_HTML_PANEL,
	ID_HEADLINES_HTML_PANEL,
};

enum MainTabImageIDs: int {
	ID_TAB_WELCOME_IMAGE,
	ID_TAB_MOD_IMAGE,
	ID_TAB_BASIC_SETTINGS_IMAGE,
	ID_TAB_ADV_SETTINGS_IMAGE,
	ID_TAB_INSTALL_IMAGE,
};

#endif