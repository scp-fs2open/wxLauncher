#include "ProfileManager.h"
#include "PlatformProfileManager.h"
#include "ids.h"

#include <wx/process.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/wfstream.h>
#include <windows.h>

#include "wxLauncherSetup.h"

/* File contains the Win32 incatations for Pushing and Pulling the passed
profile to/from the registry. */

ProMan::RegistryCodes PlatformPushProfile(wxFileConfig *cfg) {
	LONG ret = ERROR_SUCCESS;
	HKEY regHandle = 0;

	ret = RegCreateKeyExW(HKEY_LOCAL_MACHINE,
		L"Software\\Volition\\Freespace2",
		0, // Reserved
		NULL,
		REG_OPTION_NON_VOLATILE,
		KEY_WRITE,
		NULL, // default security.
		&regHandle,
		NULL // I don't care if the key was created before opening
		);
#ifdef REGISTRY_HELPER
	// Do not attempt to call registry_helper.exe if already in registry_helper.exe
	if ( ret != ERROR_SUCCESS ) {
#else
	if ( ret != ERROR_SUCCESS && ret != ERROR_ACCESS_DENIED) {
#endif
		// Call failed
		LPWSTR message = NULL;
		DWORD nchars = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		ret,
		0,
		(LPWSTR)&message,
		0,
		NULL);
		if ( nchars == 0 ) {
			wxLogFatalError(_T("An error occurred, but unable to generate error message"));
		} else {
			wxMBConvUTF16 conv;
			wxString msg(message, conv, nchars);
			wxLogError(_T("Unhandled error in creating HKLM key in registry! %s"), msg);
		}
		return ProMan::AccessDenied;
#ifndef REGISTRY_HELPER
	} else if ( ret == ERROR_ACCESS_DENIED ) {
		// Only try calling registry_helper.exe if not already in registry_helper.exe
		wxString tempfile = wxFileName::CreateTempFileName(wxStandardPaths::Get().GetTempDir());
		wxFileOutputStream out(tempfile);
		cfg->Save(out);
		wxArrayString processOutput;
		long ret = wxExecute(wxString::Format(_T("registry_helper.exe push %s"), tempfile), processOutput);
		if ( ret == ProMan::NoError ) {
			// no error so just return, because the other process did what I needed.
			return ProMan::NoError;
		} else {
			wxLogError(_T("Unable to write FS2Open settings to the registry (%d)"), ret);
			return static_cast<ProMan::RegistryCodes>(ret);
		}
#endif	
	}
	RegCloseKey(regHandle);
	return ProMan::NoError;
}

ProMan::RegistryCodes PlatformPullProfile(wxFileConfig *cfg) {
	return ProMan::UnknownError;
}