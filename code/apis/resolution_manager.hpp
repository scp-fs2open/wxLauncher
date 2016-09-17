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

#ifndef APIS_RESOLUTION_MANAGER_H
#define APIS_RESOLUTION_MANAGER_H

#include <wx/wx.h>
#include <wx/dynarray.h>

namespace ResolutionMan {
	enum ApiType {
		API_WIN32,
		API_SDL
	};

	class Resolution : public wxClientData {
	public:
		Resolution(const int width, const int height, const bool isHeader);
		virtual ~Resolution() {}
		const int GetWidth() const { return this->width; }
		const int GetHeight() const { return this->height; }
		const bool IsHeader() const { return this->isHeader; }
		const wxString &GetResString() const { return this->resString; }
		bool IsSameResolution(const Resolution& other) const {
			return ((this->width == other.width) &&
				(this->height == other.height));
		}
		Resolution* New() const;

	private:
		int width;
		int height;
		bool isHeader;
		wxString resString;
	};

	WX_DEFINE_ARRAY_PTR(Resolution *, ResolutionArray);

	void EnumerateGraphicsModes(ApiType type, ResolutionArray& out_modes,
		const long minHorizontalRes, const long minVerticalRes);
};

#endif
