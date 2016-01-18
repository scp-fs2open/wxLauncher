#include <wx/dir.h>
#include <wx/filefn.h>

#include "generated/configure_launcher.h"
#include "apis/FlagListManager.h"
#include "global/Compatibility.h"
#include "global/ProfileKeys.h"

extern wxFileName GetPlatformDefaultConfigFilePathNew();
extern wxFileName GetPlatformDefaultConfigFilePathOld();

namespace Compatibility
{
	bool SynchronizeOldPilots(ProMan* profileManager)
	{
		if (!(FlagListManager::GetFlagListManager()->GetBuildCaps() & FlagListManager::BUILD_CAPS_SDL))
		{
			// Nothing to do, we have an old build
			return true;
		}

		wxLogVerbose(_T("Synchronizing old pilot files..."));
		wxFileName oldConfigFolder;
#if IS_WIN32
		wxString rootPath;
		if (!profileManager->ProfileRead(PRO_CFG_TC_ROOT_FOLDER, &rootPath))
		{
			wxLogWarning(_T("No TC root folder in configuration!"));
			return false;
		}

		oldConfigFolder.AssignDir(rootPath, wxPATH_NATIVE);
#else
		oldConfigFolder.Assign(GetPlatformDefaultConfigFilePathOld());
#endif

		oldConfigFolder.AppendDir(_T("data"));
		oldConfigFolder.AppendDir(_T("players"));

		wxFileName newConfigFolder(GetPlatformDefaultConfigFilePathNew());
		newConfigFolder.AppendDir(_T("data"));
		newConfigFolder.AppendDir(_T("players"));

		if (!oldConfigFolder.DirExists())
		{
			// No old config folder so there are no pilot files to copy
			wxLogVerbose(_T("  Old pilot directory does not exist, nothing to be done."));
			return true;
		}

		// Heuristic to determine if the files were copied before
		if (newConfigFolder.DirExists())
		{
			// new players directory already exists so it was already used before
			// Don't try to copy in this case
			wxLogVerbose(_T("  New pilot directory already exists, was probably copied before."));
			return true;
		}

		if (!newConfigFolder.Mkdir(0777, wxPATH_MKDIR_FULL))
		{
			wxLogError(_T("Failed to create directory '%s'"), newConfigFolder.GetFullPath().c_str());
			return false;
		}

		// If we are here we need to copy the old pilot files
		wxDir oldDir(oldConfigFolder.GetFullPath());

		wxString pilotFile;
		bool cont = oldDir.GetFirst(&pilotFile, wxEmptyString, wxDIR_FILES);

		wxLogStatus(_T("Copying pilot files from '%s' to '%s'."), oldConfigFolder.GetFullPath().c_str(), newConfigFolder.GetFullPath().c_str());
		while (cont)
		{
			wxFileName pilotFileName;
			pilotFileName.Assign(oldConfigFolder.GetFullPath(), pilotFile);

			wxFileName newPilotFile;
			newPilotFile.Assign(newConfigFolder.GetFullPath(), pilotFile);

			wxLogVerbose(_T("  Copying '%s' to '%s'"), pilotFileName.GetFullPath().c_str(), newPilotFile.GetFullPath().c_str());
			if (!wxCopyFile(pilotFileName.GetFullPath(), newPilotFile.GetFullPath()))
			{
				wxLogError(_T("Failed to copy pilot file '%s'!"), pilotFileName.GetFullPath().c_str());
			}

			cont = oldDir.GetNext(&pilotFile);
		}

		wxLogVerbose(_T("  Done copying pilot files."));
		return true;
	}
}
