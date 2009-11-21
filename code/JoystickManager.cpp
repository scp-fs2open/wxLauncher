#include <wx/wx.h>

#include "JoystickManager.h"

#include "wxLauncherSetup.h"

namespace JoyMan {
#if USE_JOYSTICK
	bool isInitialized = false;
	wxArrayString joysticks;
	unsigned int numOfJoysticks = 0;
#endif
};

/** \namespace JoyMan
The JoyMan namespace contains helper functions for working with joysticks on 
the compiled platform.  The interface of this namespace is always aviable
but will only work if preprocessor symbol USE_JOYSTICK is set to 1.
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
	UINT num = joyGetNumDevs();
	if ( num > 16 ) {
		/* 16 is the max because MSDN says that on windows 2000 and later
		the MM api only supports -1 to 16 as the joystick ID to pass into 
		joyGetDevCaps() or joyGetPos() */
		wxLogError(_T("Windows reports that the system has more 16 joysticks!"));
		return false;
	}

	MMRESULT result = JOYERR_NOERROR;
	JOYINFO joyinfo;
	JOYCAPS joycaps;
	
	for (UINT counter = 0; counter < num; counter++) {
		memset(reinterpret_cast<void*>(&joyinfo), 0, sizeof(JOYINFO));
		
		result = joyGetPos(counter, &joyinfo);
		wxString joystickName;

		if ( result == JOYERR_NOERROR ) {
			// joystick plugged in
			memset(reinterpret_cast<void*>(&joycaps), 0, sizeof(JOYCAPS));

			result = joyGetDevCaps(counter, &joycaps, sizeof(JOYCAPS));
			if ( result == JOYERR_NOERROR ) {
				numOfJoysticks++;
				joystickName = wxString(joycaps.szPname, wxMBConvUTF16());
			} else {
				wxLogError(_T("Error in retiving joystick caps"));
				continue;
			}
		} else if ( result == JOYERR_UNPLUGGED ) {
			// unplugged, do nothing
		} else {
			wxLogError(_T("Error in getting joystick position"));
		}
		joysticks.Add(joystickName);
	}
	return true;
#else
	return false;
#endif
}

/** Dismantals JoyMan and frees any memory used.
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

/** \return number of in joysticks that the system reports.
\sa JoyMan::IsPluggedIn() */
unsigned int JoyMan::NumberOfJoysticks() {
#if USE_JOYSTICK
	return JoyMan::numOfJoysticks;
#else
	return 0;
#endif
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



	