#include <wx/dir.h>
#include <wx/filefn.h>
#include <wx/sstream.h>
#include <wx/wfstream.h>

#include "generated/configure_launcher.h"
#include "apis/FlagListManager.h"
#include "global/Compatibility.h"
#include "global/ProfileKeys.h"

#if IS_WIN32
#include "RegistryKeys.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

extern HKEY GetRegistryKeyname(wxString& keyNameOut);

bool enumRegistryValue(HKEY parent, DWORD index, wxString& valueNameOut, wxString& valueOut) {
	valueNameOut.Clear();
	valueOut.Clear();

	WCHAR valueNameBuffer[512];
	DWORD valueNameBufferSize = static_cast<DWORD>(sizeof(valueNameBuffer));

	BYTE valueBuffer[512];
	DWORD valueBufferSize = static_cast<DWORD>(sizeof(valueBuffer));

	DWORD type;

	LONG ret = RegEnumValueW(parent, index, valueNameBuffer, &valueNameBufferSize, NULL, &type, valueBuffer, &valueBufferSize);

	if (ret != ERROR_SUCCESS) {
		return false;
	}

	valueNameOut = wxString(valueNameBuffer, valueNameBufferSize);

	switch (type) {
	case REG_BINARY:
		valueOut = wxString::From8BitData(reinterpret_cast<char*>(valueBuffer), valueBufferSize);
		break;
	case REG_DWORD:
	{
		uint32_t val = *reinterpret_cast<uint32_t*>(valueBuffer); // I feel dirty...
		valueOut << val;
		break;
	}
	case REG_QWORD:
	{
		uint64_t val = *reinterpret_cast<uint64_t*>(valueBuffer); // I feel dirty...
		valueOut << val;
		break;
	}
	case REG_SZ:
		valueOut = wxString(reinterpret_cast<wchar_t*>(valueBuffer), (valueBufferSize / sizeof(WCHAR)) - 1);
		break;

	default:
		return false;
	}

	return true;
}

void copyValuesIntoConfig(HKEY key, wxFileConfig& config, const wxString& sectionName) {
	config.SetPath(sectionName);

	for (DWORD i = 0; ; ++i) {
		wxString name;
		wxString value;
		if (!enumRegistryValue(key, i, name, value))
		{
			break;
		}
		config.Write(name, value);
	}
}
#endif

#define FSO_CONFIG_FILENAME _T("fs2_open.ini")

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

		wxLogStatus(_T("Synchronizing old pilot files..."));
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
			wxLogStatus(_T("  Old pilot directory does not exist, nothing to be done."));
			return true;
		}

		// Heuristic to determine if the files were copied before
		if (newConfigFolder.DirExists())
		{
			// new players directory already exists so it was already used before
			// Don't try to copy in this case
			wxLogStatus(_T("  New pilot directory already exists, was probably copied before."));
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

			wxLogStatus(_T("  Copying '%s' to '%s'"), pilotFileName.GetFullPath().c_str(), newPilotFile.GetFullPath().c_str());
			if (!wxCopyFile(pilotFileName.GetFullPath(), newPilotFile.GetFullPath()))
			{
				wxLogError(_T("Failed to copy pilot file '%s'!"), pilotFileName.GetFullPath().c_str());
			}

			cont = oldDir.GetNext(&pilotFile);
		}

		wxLogStatus(_T("  Done copying pilot files."));
		return true;
	}

	bool MigrateOldConfig()
	{
		if (!(FlagListManager::GetFlagListManager()->GetBuildCaps() & FlagListManager::BUILD_CAPS_SDL))
		{
			// Nothing to do, we have an old build
			return true;
		}

		wxFileName newName = GetPlatformDefaultConfigFilePathNew();
		newName.SetFullName(FSO_CONFIG_FILENAME);

		if (wxFile::Exists(newName.GetFullPath())) {
			// New file already exists, nothing to do here
			return true;
		}

		wxLogStatus(_T("Migrating old configuration..."));
#if IS_WIN32
		// On Windows this is implemented by iterating through the registry and copying the values to the config file
		wxString keyName;
		HKEY useKey = GetRegistryKeyname(keyName);

		LONG ret = ERROR_SUCCESS;
		HKEY regHandle = 0;
		ret = RegOpenKeyExW(useKey,
			keyName.wc_str(),
			0,
			KEY_READ,
			&regHandle
			);
		if (ret != ERROR_SUCCESS) {
			// Key does not exists or some other error, assume it doesn't exist
			return true; // No old config, nothing to do here
		}

		wxStringInputStream configBlankInputStream(_T(""));

		wxFileConfig outConfig(configBlankInputStream, wxMBConvUTF8());

		copyValuesIntoConfig(regHandle, outConfig, REG_KEY_DEFAULT_FOLDER_CFG);

		for (DWORD i = 0;; ++i) {
			WCHAR subkey[255];
			DWORD subkeyLen = 255;

			LONG res = RegEnumKeyExW(regHandle, i, subkey, &subkeyLen, NULL, NULL, NULL, NULL);
			if (res != ERROR_SUCCESS)
				break;

			HKEY handle;
			res = RegOpenKeyExW(regHandle, subkey, 0, KEY_READ, &handle);
			if (res != ERROR_SUCCESS) {
				continue;
			}
			wxString sectionName(subkey, subkeyLen);

			sectionName = wxString::FromAscii("/") + sectionName;

			copyValuesIntoConfig(handle, outConfig, sectionName);
			RegCloseKey(handle);
		}

		RegCloseKey(regHandle);

		wxLogStatus(_T("Writing fs2_open.ini to %s"), newName.GetFullPath().c_str());
		wxFFileOutputStream outFileStream(newName.GetFullPath());

		outConfig.Save(outFileStream, wxMBConvUTF8());
#else
		wxFileName oldName = GetPlatformDefaultConfigFilePathOld();
		oldName.SetFullName(FSO_CONFIG_FILENAME);

		if (!wxCopyFile(oldName.GetFullPath(), newName.GetFullPath())) {
			wxLogError(_T("Failed to copy old configuration file to new location!"));
			return false;
		}
#endif

		wxLogStatus(_T("Migration finished."));
		return true;
	}
}
