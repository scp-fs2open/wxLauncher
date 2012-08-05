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

#include "generated/configure_launcher.h"
#include "apis/ProfileManager.h"
#include "apis/ProfileManagerOperator.h"
#include "wxLauncherApp.h"
#include "global/ids.h"

#include "global/MemoryDebugging.h"

int ProManOperator::RunProfileOperator(ProManOperator::profileOperator op)
{
	wxLauncher &app = wxGetApp();

	if (op == add)
	{
		wxFileName sourceFile(app.mFileOperand);
		if (ProMan::GetProfileManager()->
			CreateProfile(app.mProfileOperand, sourceFile))
		{
			return 0;
		}
	}

	return 1;
}
