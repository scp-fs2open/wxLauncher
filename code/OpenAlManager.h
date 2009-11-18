#ifndef OPENALMANAGER
#define OPENALMANAGER

#include <wx/wx.h>

namespace OpenALMan {
	static bool Initialize();
	static bool DeInitialize();
	static bool WasCompliedIn();
	static bool IsInitialized();

	static wxArrayString GetAvailiableDevices();
	static wxString GetCurrentVersion();
};

#endif