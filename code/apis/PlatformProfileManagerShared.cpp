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

#include <wx/wx.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <wx/wfstream.h>
#include "generated/configure_launcher.h"
#include "apis/PlatformProfileManager.h"
#include "apis/FlagListManager.h"
#include "controls/LightingPresets.h"
#include "global/ProfileKeys.h"

ProMan::RegistryCodes PushCmdlineFSO(wxFileConfig *cfg) {
	wxString modLine, flagLine, tcPath;
	cfg->Read(PRO_CFG_TC_CURRENT_MODLINE, &modLine);
	cfg->Read(PRO_CFG_TC_CURRENT_FLAG_LINE, &flagLine);
	cfg->Read(PRO_CFG_TC_ROOT_FOLDER, &tcPath);
	
	wxString presetName;
	wxString lightingPresetFlagSet;
	if (cfg->Read(PRO_CFG_LIGHTING_PRESET, &presetName)) {
		lightingPresetFlagSet = LightingPresets::PresetNameToPresetFlagSet(presetName);
	}

	wxString cmdLineString = FlagListManager::GetFlagListManager()->GetConfigLocation(tcPath).GetFullPath();

	cmdLineString += _T("data");
	
#if IS_LINUX // try to rename file in root folder if exists
	wxFileName tcCfgFile(tcPath + wxFileName::GetPathSeparator());
	tcCfgFile.AppendDir(_T("data"));
	tcCfgFile.SetFullName(_T("cmdline_fso.cfg"));
	
	if (tcCfgFile.IsOk() && ::wxFileExists(tcCfgFile.GetFullPath())) {
		wxFileName tcCfgRenameFile(tcCfgFile);
		tcCfgRenameFile.SetFullName(_T("cmdline_fso.old.cfg"));
		
		// rename target exists; attempt to delete it
		if (tcCfgRenameFile.IsOk() && ::wxFileExists(tcCfgRenameFile.GetFullPath())) {
			wxLogWarning(_T("Backup cmdline_fso.old.cfg file %s exists, deleting it"),
				tcCfgRenameFile.GetFullPath().c_str());
			
			if (!::wxRemoveFile(tcCfgRenameFile.GetFullPath())) {
				wxLogError(_T("Could not remove backup cmdline_fso.old.cfg file %s"),
					tcCfgRenameFile.GetFullPath().c_str());
			}
		}
		
		// now try the rename
		if (!::wxRenameFile(tcCfgFile.GetFullPath(),
					tcCfgRenameFile.GetFullPath(), false)) {
			wxLogError(_T("Could not rename root folder copy %s to %s"),
				tcCfgFile.GetFullPath().c_str(),
				tcCfgRenameFile.GetFullPath().c_str());
		} else {
			wxLogInfo(_T("Renamed root folder copy of cmdline_fso.cfg"));
		}
	}
#endif

	// if data folder does not exist in cmdline folder, attempt to create it first
	if (!wxDir::Exists(cmdLineString)) {
		if (!::wxMkdir(cmdLineString)) {
			wxLogError(_T("Couldn't create 'data' folder %s"),
				cmdLineString.c_str());
			return ProMan::UnknownError;
		}
		wxLogDebug(_T("'data' folder %s created"),
			cmdLineString.c_str());
	} else {
		wxLogDebug(_T("'data' folder %s found"),
			cmdLineString.c_str());	
	}

	cmdLineString += wxFileName::GetPathSeparator();
	cmdLineString += _T("cmdline_fso.cfg");
	wxFileName cmdLineFileName(cmdLineString);
	wxFFileOutputStream outStream(cmdLineFileName.GetFullPath(), _T("w+b"));
	if ( !outStream.IsOk() ) {
		return ProMan::UnknownError;
	}
	if ( !modLine.IsEmpty()) {
		outStream.Write("-mod ", 5);

		// Enclose the mod parameter in quotes to escape spaces
		outStream.Write("\"", 1);
		outStream.Write(modLine.char_str(), modLine.size());
		outStream.Write("\"", 1);
	}
	if ( !flagLine.IsEmpty() ) {
		outStream.Write(" ", 1);
		outStream.Write(flagLine.char_str(), flagLine.size());
	}
	if ( !lightingPresetFlagSet.IsEmpty()) {
		outStream.Write(" ", 1);
		outStream.Write(lightingPresetFlagSet.char_str(), lightingPresetFlagSet.size());
	}
	if ( !outStream.Close() ) {
		return ProMan::UnknownError;
	}

	return ProMan::NoError;
}
