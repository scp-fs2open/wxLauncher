#include <wx/wx.h>
#include <wx/filename.h>
#include "ids.h"
#include "ProfileManager.h"
#include "InstallPage.h"
#include "HelpManager.h"
#include "generated/configure_launcher.h"

#include "wxLauncherSetup.h" // Last include for memory debugging

InstallPage::InstallPage(wxWindow* parent): wxPanel(parent, wxID_ANY) {
}

BEGIN_EVENT_TABLE(InstallPage, wxPanel)
END_EVENT_TABLE()