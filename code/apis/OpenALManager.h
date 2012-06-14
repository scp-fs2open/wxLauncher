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

#ifndef OPENALMANAGER
#define OPENALMANAGER

#include <wx/wx.h>

#if wxUSE_DYNLIB_CLASS == 0 && USE_OPENAL == 1
#error Cannot compile OPENAL into launcher if wxWidgets has wxUSE_DYNLIB_CLASS set to 0
#endif

namespace OpenALMan {
	bool Initialize();
	bool DeInitialize();
	bool WasCompliedIn();
	bool IsInitialized();
	
	wxArrayString GetAvailablePlaybackDevices();
	wxArrayString GetAvailableCaptureDevices();
	
	wxString GetCurrentVersion();
	
	wxString GetSystemDefaultPlaybackDevice();
	wxString GetSystemDefaultCaptureDevice();
	
	bool IsEFXSupported();
	bool BuildHasNewSoundCode();
};

#endif
