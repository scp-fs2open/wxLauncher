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

#ifndef JOYSTICKMANAGER_H
#define JOYSTICKMANAGER_H

#include <wx/wx.h>

namespace JoyMan {
	enum ApiType {
		API_NATIVE,
		API_SDL
	};

	bool Initialize(ApiType apiType);
	bool DeInitialize();
	bool IsInitialized();
	bool WasCompiledIn();

	unsigned int NumberOfJoysticks();
	unsigned int NumberOfPluggedInJoysticks();
	bool SupportsForceFeedback(unsigned int i);
	bool HasCalibrateTool(unsigned int i);
	void LaunchCalibrateTool(unsigned int i);
	wxString JoystickName(unsigned int i);
	wxString JoystickGUID(unsigned int i);
	bool IsJoystickPluggedIn(unsigned int i);
};

#endif
