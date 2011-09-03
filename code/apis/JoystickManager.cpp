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

#include <wx/wx.h>

#include "apis/JoystickManager.h"
#include "generated/configure_launcher.h"

#if USE_JOYSTICK && HAS_SDL
#include "SDL.h"
#endif

#include "global/MemoryDebugging.h"

namespace JoyMan {
#if USE_JOYSTICK
	bool isInitialized = false;
	wxArrayString joysticks;
	unsigned int numOfJoysticks = 0; //!< number of plugged in joysticks
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
	return isInitialized;
#else
	return false;
#endif
}

/** Gets JoyMan ready to manage joysticks.
\return true if successful, false otherwise.
\note Will also return false if JoyMan is not compiled in.
\sa JoyMan::WasCompiledIn()
*/
bool JoyMan::Initialize() {
	if ( JoyMan::IsInitialized() ) {
		wxLogDebug(_T("JoyMan already initialized with %d joysticks"),
			joysticks.Count());
		return true;
	}

#if USE_JOYSTICK && IS_WIN32
	UINT num = joyGetNumDevs(); // get the number of joys supported by windows.
	if ( num > 16 ) {
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
	JoyMan::numOfJoysticks = 0;
	JoyMan::joysticks.clear();
	int totalNumberOfJoysticks = 0;
	
	for (UINT counter = 0; counter < num; counter++) {
		memset(reinterpret_cast<void*>(&joyinfo), 0, sizeof(JOYINFO));
		
		result = joyGetPos(counter, &joyinfo);
		wxString joystickName;

		if ( result == JOYERR_NOERROR ) {
			// joystick plugged in
			totalNumberOfJoysticks++;
			memset(reinterpret_cast<void*>(&joycaps), 0, sizeof(JOYCAPS));

			result = joyGetDevCaps(counter, &joycaps, sizeof(JOYCAPS));
			if ( result == JOYERR_NOERROR ) {
				numOfJoysticks++;
				joystickName = wxString(joycaps.szPname, wxMBConvUTF16());
				joysticks.Add(joystickName);
			} else {
				wxLogError(_T("Error in retrieving joystick caps"));
				continue;
			}
		} else if ( result == JOYERR_UNPLUGGED ) {
			// unplugged
			totalNumberOfJoysticks++;
		} else {
			// Joystick doesn't exist, do nothing
		}
	}
	wxLogInfo(_T("Windows reports %d joysticks, %d seem to be plugged in."),
			totalNumberOfJoysticks, numOfJoysticks );
	return true;
#elif USE_JOYSTICK && HAS_SDL
	SDL_InitSubSystem(SDL_INIT_JOYSTICK);
	
	JoyMan::numOfJoysticks = 0;
	JoyMan::joysticks.clear();

	SDL_Joystick *joy = NULL;
	for (int i = 0; i < SDL_NumJoysticks(); i++) {
		joy = SDL_JoystickOpen(i);
		if ( joy != NULL ) {
			wxString joystickName(SDL_JoystickName(i), wxConvLocal);
			joysticks.Add(joystickName);
		}
		SDL_JoystickClose(joy);
	}

	return true;
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
	if ( JoyMan::IsInitialized() ) {
		JoyMan::isInitialized = false;
		JoyMan::numOfJoysticks = 0;
		JoyMan::joysticks.clear();
	}
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
	return JoyMan::joysticks.size();
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

/** \bug Assumes all joysticks support ForceFeedback */
#if USE_JOYSTICK
bool JoyMan::SupportsForceFeedback(unsigned int i) {
	if ( i == JOYMAN_INVALID_JOYSTICK ) {
		return false;
	} else {
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
	if ( i == JOYMAN_INVALID_JOYSTICK ) {
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
#ifdef WINDOWS
	// the same call that the current window launcher uses to open the 
	// calibration tool. Obviously it only works on windows :)
	WinExec("rundll32.exe shell32.dll,Control_RunDLL joy.cpl", SW_SHOWNORMAL);
#endif
}

/** Returns the name of the joystick for display. */
#if USE_JOYSTICK
wxString JoyMan::JoystickName(unsigned int i) {
	return JoyMan::joysticks[i];
#else
wxString JoyMan::JoystickName(unsigned int) {
	return wxEmptyString;
#endif
}

/** Returns true when the joystick is plugged in. */
#if USE_JOYSTICK
bool JoyMan::IsJoystickPluggedIn(unsigned int i) {
	if ( i == JOYMAN_INVALID_JOYSTICK ) {
		return false;
	} else {
		// FIXME because we're using indexes to represent joysticks, there's no guarantee
		//       that an index value always refers to the same joystick
		return i >= JoyMan::joysticks.GetCount() ? false : !JoyMan::joysticks[i].IsEmpty();
	}
#else
bool JoyMan::IsJoystickPluggedIn(unsigned int) {
	return false;
#endif
}
