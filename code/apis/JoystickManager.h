#ifndef JOYSTICKMANAGER_H
#define JOYSTICKMANAGER_H

#include <wx/wx.h>

#define JOYMAN_INVAILD_JOYSTICK 99999

namespace JoyMan {
	bool Initialize();
	bool DeInitialize();
	bool IsInitialized();
	bool WasCompiledIn();

	unsigned int NumberOfJoysticks();
	unsigned int NumberOfPluggedInJoysticks();
	bool SupportsForceFeedback(unsigned int i);
	bool HasCalibrateTool(unsigned int i);
	void LaunchCalibrateTool(unsigned int i);
	wxString JoystickName(unsigned int i);
	bool IsJoystickPluggedIn(unsigned int i);
};

#endif