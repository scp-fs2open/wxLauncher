#include <wx/wx.h>
#include <wx/filename.h>
#include "global/ids.h"
#include "apis/ProfileManager.h"
#include "tabs/InstallPage.h"
#include "apis/HelpManager.h"
#include "generated/configure_launcher.h"

#include "global/MemoryDebugging.h" // Last include for memory debugging

InstallPage::InstallPage(wxWindow* parent): wxPanel(parent, wxID_ANY) {
}

BEGIN_EVENT_TABLE(InstallPage, wxPanel)
END_EVENT_TABLE()