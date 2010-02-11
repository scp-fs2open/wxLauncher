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

	wxArrayString GetAvailiableDevices();
	wxString GetCurrentVersion();
	wxString SystemDefaultDevice();
};

#endif