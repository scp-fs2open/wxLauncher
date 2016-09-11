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

#include "version.h"
#include "generated/configure_launcher.h"

const wxUint8 MAJOR_VERSION = wxlauncher_VERSION_MAJOR;
const wxUint8 MINOR_VERSION = wxlauncher_VERSION_MINOR;
const wxUint8 PATCH_VERSION = wxlauncher_VERSION_PATCH;
const wxUint64 FULL_VERSION = wxlauncher_VERSION_MAJOR*1000000
	+ wxlauncher_VERSION_MINOR*1000 + wxlauncher_VERSION_PATCH;
