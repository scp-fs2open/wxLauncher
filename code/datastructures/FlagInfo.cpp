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