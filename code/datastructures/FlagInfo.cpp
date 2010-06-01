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

/*
This file holds the internal list of flags that are not 
returned by the FS2_Open when queryed.

Let's hope this list will never change and not be even 
need to change after the next FS2_Open release.

It will be kept for backward compatibility though
*/


FlagInfo flagInfo[] = {
	//Lighting 
	{_T("-ambient_factor"), _("Lighting"), true},
	{_T("-no_emissive_light"), _("Lighting"), false},
	{_T("-spec_exp"), _("Lighting"), true},
	{_T("-spec_point"), _("Lighting"), true},
	{_T("-spec_static"), _("Lighting"), true},
	{_T("-spec_tube"), _("Lighting"), true},
	{_T("-ogl_spec"), _("Lighting"), true},
	//Graphics
	{_T("-height"), _("Graphics"), false},
	{_T("-clipdist"), _("Graphics"), true},
	{_T("-res"), _("Graphics"), true},
	//Gameplay
	//{_T("-mod"), _("Gameplay"), true}, // This flag is handled elsewhere, but kept on list for completeness
	{_T("-fov"), _("Gameplay"), true},
	//Multiplayer
	{_T("-gamename"), _("Multiplayer"), true},
	{_T("-password"), _("Multiplayer"), true},
	{_T("-allowabove"), _("Multiplayer"), true},
	{_T("-allowbelow"), _("Multiplayer"), true},
	{_T("-port"), _("Multiplayer"), true},
	{_T("-connect"), _("Multiplayer"), true}, //string, ip to connnect to
	{_T("-timeout"), _("Multiplayer"), true}, //int, time before timeout
	{_T("-cap_object_update"), _("Multiplayer"), true},
};