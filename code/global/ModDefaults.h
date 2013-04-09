/*
 Copyright (C) 2009-2013 wxLauncher Team
 
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

#ifndef MOD_DEFAULTS_H
#define MOD_DEFAULTS_H

#include <wx/string.h>

// from LightingPresets.h
class Preset;

/** \defgroup moddefaults default values for mod-specific settings */
/** @{*/
extern const wxString DEFAULT_MOD_LAUNCHER_INFO_TEXT;

extern const int DEFAULT_MOD_LAUNCHER_MIN_HORIZONTAL_RES;
extern const int DEFAULT_MOD_LAUNCHER_MIN_VERTICAL_RES;

extern const wxString DEFAULT_MOD_RECOMMENDED_LIGHTING_NAME;
extern const Preset DEFAULT_MOD_RECOMMENDED_LIGHTING_PRESET;
/** @}*/

#endif
