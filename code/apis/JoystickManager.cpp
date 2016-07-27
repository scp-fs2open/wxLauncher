/*
Copyright (C) 2009-2011,2015 wxLauncher Team

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

#include <wx/wx.h>

#include "apis/JoystickManager.h"
#include "generated/configure_launcher.h"

#if USE_JOYSTICK && HAS_SDL
#include "SDL.h"
#endif

#include "global/BasicDefaults.h"
#include "global/MemoryDebugging.h"

#include <vector>

namespace JoyMan {
#if USE_JOYSTICK
	ApiType currentApi = API_NATIVE;
#if HAS_SDL
	bool isSdlInitialized = false;
	std::vector<SDL_Joystick*> sdlJoysticks;

	void clearSDLJoystickList()
	{
		for (std::vector<SDL_Joystick*>::iterator iter = sdlJoysticks.begin(); iter != sdlJoysticks.end(); ++iter)
		{
			SDL_JoystickClose(*iter);
		}
		sdlJoysticks.clear();
	}
#endif
#if IS_WIN32
	bool isWinInitialized = false;
	wxArrayString winJoysticks;
#endif
#endif
};

/** \namespace JoyMan
The JoyMan namespace contains helper functions for working with joysticks on 
the compiled platform.  The interface of this namespace is always available
but will only work if preprocessor symbol USE_JOYSTICK is set to 1, and of
course if the platform supports it.
\sa JoyMan::WasCompiledIn();
*/

using namespace JoyMan;

/** \return true when JoyMan is ready to accept queries about joysticks.
\note Always returns false when JoyMan is not compiled in.
\sa JoyMan::WasCompiledIn()
*/
bool JoyMan::IsInitialized() {
#if USE_JOYSTICK
#if HAS_SDL
	if (currentApi == API_SDL)
	{
		return isSdlInitialized;
	}
#endif
#if IS_WIN32
	if (currentApi == API_NATIVE)
	{
		return isWinInitialized;
	}
#endif

	return false;
#else
	return false;
#endif
}

/** Gets JoyMan ready to manage joysticks.
\return true if successful, false otherwise.
\note Will also return false if JoyMan is not compiled in.
\sa JoyMan::WasCompiledIn()
*/
bool JoyMan::Initialize(ApiType apiType) {
	currentApi = apiType;

#if USE_JOYSTICK
	if ( JoyMan::IsInitialized() ) {
		wxLogDebug(_T("JoyMan already initialized"));
		return true;
	}
#else
	wxLogDebug(wxT_2("JoyMan was disabled at compile time"));
	return false;
#endif

#if USE_JOYSTICK
#if IS_WIN32
	if (currentApi == API_NATIVE)
	{
		UINT num = joyGetNumDevs(); // get the number of joys supported by windows.
		if (num > 16) {
			/* greater than 16 is cause for a warning because according to MSDN,
			Windows 2000 and later support -1 thru 16 in the MM api (and windows NT
			only supports 1 or 2).  MSDN also notes if you want more than 16 use
			DirectInput. */
			wxLogWarning(_T("Windows reports that the joystick driver ")
				_T("supports more than 16 joysticks (reports %d)!"), num);
		}

		MMRESULT result = JOYERR_NOERROR;
		JOYINFO joyinfo;
		JOYCAPS joycaps;
		JoyMan::winJoysticks.clear();
		int totalNumberOfJoysticks = 0;

		for (UINT counter = 0; counter < num; counter++) {
			memset(reinterpret_cast<void*>(&joyinfo), 0, sizeof(JOYINFO));

			result = joyGetPos(counter, &joyinfo);
			wxString joystickName;

			if (result == JOYERR_NOERROR) {
				// joystick plugged in
				totalNumberOfJoysticks++;
				memset(reinterpret_cast<void*>(&joycaps), 0, sizeof(JOYCAPS));

				result = joyGetDevCaps(counter, &joycaps, sizeof(JOYCAPS));
				if (result == JOYERR_NOERROR) {
					joystickName = wxString(joycaps.szPname, wxMBConvUTF16());
					winJoysticks.Add(joystickName);
				}
				else {
					wxLogError(_T("Error in retrieving joystick caps"));
					continue;
				}
			}
			else if (result == JOYERR_UNPLUGGED) {
				// unplugged
				totalNumberOfJoysticks++;
			}
			else {
				// Joystick doesn't exist, do nothing
			}
		}
		wxLogInfo(_T("Windows reports %d joysticks, %d seem to be plugged in."),
			totalNumberOfJoysticks, static_cast<int>(winJoysticks.size()));
		return true;
	}
#endif

#if HAS_SDL
	if (currentApi == API_SDL)
	{
		SDL_InitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC);

		JoyMan::clearSDLJoystickList();

		for (int i = 0; i < SDL_NumJoysticks(); i++) {
			SDL_Joystick* joy = SDL_JoystickOpen(i);
			if (joy != NULL) {
				sdlJoysticks.push_back(joy);
			}
		}

		return true;
	}
#endif

	return false;
#else
	return false;
#endif
}

/** Dismantles JoyMan and frees any memory used.
\note Will always return true when JoyMan is not compiled in.
\return Return true if DeInitializtion was successful.
*/
bool JoyMan::DeInitialize() {
#if USE_JOYSTICK
#if HAS_SDL
	if ( isSdlInitialized ) {
		JoyMan::isSdlInitialized = false;
		JoyMan::sdlJoysticks.clear();
		JoyMan::clearSDLJoystickList();

		SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_HAPTIC);
	}
#endif
#if IS_WIN32
	if (isWinInitialized)
	{
		JoyMan::isWinInitialized = false;
		JoyMan::winJoysticks.clear();
	}
#endif
#endif
	return true;
}
/** \return true if the internals of JoyMan were compiled into the launcher. */
bool JoyMan::WasCompiledIn() {
#if USE_JOYSTICK
	return true;
#else
	return false;
#endif
}

/** \return number of joysticks that the system reports as existing.
\sa JoyMan::IsPluggedIn() */
unsigned int JoyMan::NumberOfJoysticks() {
#if USE_JOYSTICK
#if HAS_SDL
	if (currentApi == API_SDL)
	{
		return sdlJoysticks.size();
	}
#endif
#if IS_WIN32
	if (currentApi == API_NATIVE)
	{
		return winJoysticks.size();
	}
#endif
	return 0;
#else
	return 0;
#endif
}

/** \return number of joystsicks that are plugged in.
\sa JoyMan::NumberOfJoysticks()
\sa JoyMan::IsPluggedIn() */
unsigned int JoyMan::NumberOfPluggedInJoysticks() {
	unsigned int total = JoyMan::NumberOfJoysticks();
	unsigned int pluggedIn = 0;
	for ( unsigned int i = 0; i < total; i++) {
		if ( JoyMan::IsJoystickPluggedIn(i) ) {
			pluggedIn++;
		}
	}
	return pluggedIn;
}

/** \return @c true when the joystick supports force feedback, @c false otherwise */
#if USE_JOYSTICK
bool JoyMan::SupportsForceFeedback(unsigned int i) {
	if ( i == static_cast<unsigned int>(DEFAULT_JOYSTICK_ID) ) {
		return false;
	} else {
#if HAS_SDL
		if (currentApi == API_SDL)
		{
			if (sdlJoysticks.size() <= i) {
				return false;
			}
			else {
				SDL_Haptic* haptic = SDL_HapticOpenFromJoystick(sdlJoysticks[i]);
				bool hasForceFeedback = haptic != NULL;
				
				if (hasForceFeedback)
				{
					SDL_HapticClose(haptic);
				}

				return hasForceFeedback;
			}
		}
#endif
		return true;
	}
#else
bool JoyMan::SupportsForceFeedback(unsigned int) {
	return false;
#endif
}

/** \bug Assumes all joysticks have a calibrate tool. */
#if USE_JOYSTICK
bool JoyMan::HasCalibrateTool(unsigned int i) {
	if ( i == static_cast<unsigned int>(DEFAULT_JOYSTICK_ID) ) {
		return false;
	} else {
		return true;
	}
#else
bool JoyMan::HasCalibrateTool(unsigned int) {
	return false;
#endif
}

/** Launch the windows joystick calibration tool. */
void JoyMan::LaunchCalibrateTool(unsigned int WXUNUSED(i)) {
#if IS_WIN32
	// the same call that the current window launcher uses to open the 
	// calibration tool. Obviously it only works on windows :)
	WinExec("rundll32.exe shell32.dll,Control_RunDLL joy.cpl", SW_SHOWNORMAL);
#endif
}

/** Returns the name of the joystick for display. */
#if USE_JOYSTICK
wxString JoyMan::JoystickName(unsigned int i) {
#if HAS_SDL
	if (currentApi == API_SDL)
	{
		if (sdlJoysticks.size() <= i) {
			return wxEmptyString;
		}
		else {
			const char* name = SDL_JoystickName(sdlJoysticks[i]);
			if (name != NULL)
			{
				return wxString::FromUTF8(name);
			} else
			{
				return wxEmptyString;
			}
		}
	}
#endif
#if IS_WIN32
	if (currentApi == API_NATIVE)
	{
		return winJoysticks[i];
	}
#endif
	return wxEmptyString;
#else
wxString JoyMan::JoystickName(unsigned int) {
	return wxEmptyString;
#endif
}

wxString JoyMan::JoystickGUID(unsigned int i)
{
#if USE_JOYSTICK
#if HAS_SDL
	if (currentApi == API_SDL)
	{
		if (sdlJoysticks.size() <= i) {
			return wxEmptyString;
		} else {
			SDL_JoystickGUID guid = SDL_JoystickGetGUID(sdlJoysticks[i]);

			char guidStr[33];
			SDL_JoystickGetGUIDString(guid, guidStr, 33);
			return wxString::FromAscii(guidStr);
		}
	}
#endif
#if IS_WIN32
	if (currentApi == API_NATIVE)
	{
		// No GUIDs on windows
		return wxEmptyString;
	}
#endif
	return wxEmptyString;
#else
	return wxEmptyString;
#endif
}

/** Returns true when the joystick is plugged in. */
#if USE_JOYSTICK
bool JoyMan::IsJoystickPluggedIn(unsigned int i) {
	if ( i == static_cast<unsigned int>(DEFAULT_JOYSTICK_ID) ) {
		return false;
	} else {
		// FIXME because we're using indexes to represent joysticks, there's no guarantee
		//       that an index value always refers to the same joystick
		return i >= JoyMan::NumberOfJoysticks() ? false : !JoyMan::JoystickName(i).IsEmpty();
	}
#else
bool JoyMan::IsJoystickPluggedIn(unsigned int) {
	return false;
#endif
}
