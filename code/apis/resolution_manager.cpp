/*
Copyright (C) 2016 wxLauncher Team

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

#include "apis/resolution_manager.hpp"
#include "global/ProfileKeys.h"

#if HAS_SDL
#include "SDL.h"
#endif

/** \namespace ResolutionMan
The ResolutionMan namespace contains the code that gets the
available resolutions for the BasicSettingsTab.
*/

using namespace ResolutionMan;

#ifdef WIN32
std::vector<Resolution> EnumerateGraphicsModes_win32()
{
	DEVMODE deviceMode;
	DWORD modeCounter = 0;
	BOOL result;
	std::vector<Resolution> out_modes;

	wxLogDebug(_T("Enumerating graphics modes with Win32"));

	do {
		memset(&deviceMode, 0, sizeof(DEVMODE));
		deviceMode.dmSize = sizeof(DEVMODE);

		result = EnumDisplaySettings(NULL, modeCounter, &deviceMode);

		if (result == TRUE) {
			wxLogDebug(_T(" %dx%d %d bit %d hertz (%d)"),
				deviceMode.dmPelsWidth,
				deviceMode.dmPelsHeight,
				deviceMode.dmBitsPerPel,
				deviceMode.dmDisplayFrequency,
				deviceMode.dmDisplayFlags);

			Resolution candidate;
			candidate.width = deviceMode.dmPelsWidth;
			candidate.height = deviceMode.dmPelsHeight;

			// check to see if the resolution has already been added
			// since the current out_modes list is unsorted we need to
			// iterate over all of the existing ones
			bool resExists = false;
			for (auto it = out_modes.begin(), end = out_modes.end(); it != end; ++it)
			{
				if (it->width == candidate.width && it->height == candidate.height) {
					resExists = true;
					break; // no need to keep looking
				}
			}

			if (!resExists)
			{
				out_modes.push_back(candidate);
			}
		}
		modeCounter++;
	} while (result == TRUE);

	return out_modes;
}
#endif

#if HAS_SDL
std::vector<Resolution> EnumerateGraphicsModes_sdl()
{
	wxLogDebug(_T("Enumerating graphics modes with SDL"));
	// FSO currently only supports the primary display
	const int DISPLAY_INDEX = 0;
	std::vector<Resolution> out_modes;

#if IS_APPLE || IS_WIN32
	// It seems that linux cannot handle having the video subsystem inited
	// too late
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) < 0) {
		wxLogFatalError(wxT_2("SDL video subsystem failed to initialize!"));
	}
#endif

	if (SDL_GetNumVideoDisplays() > 0) {
		int numDisplayModes = SDL_GetNumDisplayModes(DISPLAY_INDEX);

		SDL_DisplayMode mode;

		for (int i = 0; i < numDisplayModes; ++i) {
			if (SDL_GetDisplayMode(DISPLAY_INDEX, i, &mode) != 0) {
				wxLogWarning(_T("SDL_GetDisplayMode failed: %s"),
					SDL_GetError());
				continue;
			}

			Resolution res{};
			res.width = mode.w;
			res.height = mode.h;
			out_modes.push_back(res);
		}
	}
	else {
		wxLogWarning(_T("SDL reported no displays!"));
	}

#if IS_APPLE || IS_WIN32
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
#endif
#endif

	return out_modes;
}

#if !HAS_SDL && !defined(WIN32)
#error No implementation of EnumerateGraphicsModes available
#endif

/** Get available graphics modes for API and return them sorted */
std::vector<Resolution> ResolutionMan::EnumerateGraphicsModes(ApiType type)
{
#if IS_WIN32
	if (type == ResolutionMan::API_WIN32) {
		return EnumerateGraphicsModes_win32();
	}
#endif
#if HAS_SDL
	if (type == ResolutionMan::API_SDL) {
		return EnumerateGraphicsModes_sdl();
	}
#endif
	return std::vector<Resolution>();
}
