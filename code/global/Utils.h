/*
 Copyright (C) 2013 wxLauncher Team
 
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

#ifndef UTILS_H
#define UTILS_H

#include <wx/wx.h>
#include <wx/tokenzr.h>

namespace TextUtils {

	struct Words {
		wxString word;
		wxSize size;
	};
	
	WX_DECLARE_OBJARRAY(Words, ArrayOfWords);
	
	/* Fills an array of words from a token stream. */
	void FillArrayOfWordsFromTokens(wxStringTokenizer &tokens,
									wxDC &dc,
									wxFont* testFont,
									ArrayOfWords& words,
									bool useAppleDebugFilter = false);
}

#if _WIN32
#define SZT wxT("%Iu")
#else
#define SZT wxT("%zu")
#endif

#endif
