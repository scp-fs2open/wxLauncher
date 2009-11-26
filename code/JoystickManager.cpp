#include <wx/wx.h>

#include "JoystickManager.h"

#include "wxLauncherSetup.h"

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
#if USE_JOYSTICK
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
		joysticks.Add(joystickName);
	}
	wxLogInfo(_T("Windows reports %d joysticks, %d seem to be plugged in."),
			totalNumberOfJoysticks, numOfJoysticks );
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
	if ( i == JOYMAN_INVAILD_JOYSTICK ) {
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
	if ( i == JOYMAN_INVAILD_JOYSTICK ) {
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
	// the same call that the current window launcher uses to open the 
	// calibration tool.
	WinExec("rundll32.exe shell32.dll,Control_RunDLL joy.cpl", SW_SHOWNORMAL);
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
	if ( i == JOYMAN_INVAILD_JOYSTICK ) {
		return false;
	} else {
		return !JoyMan::joysticks[i].IsEmpty();
	}
#else
bool JoyMan::IsJoystickPluggedIn(unsigned int) {
	return false;
#endif
}



	