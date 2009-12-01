#ifndef HELPMANAGER_H
#define HELPMANAGER_H

#include <wx/wx.h>
#include "wxIDS.h"

namespace HelpManager {
	bool Initialize();
	bool DeInitialize();
	bool IsInitialized();

	void OpenHelpById(WindowIDS id);
};

#endif