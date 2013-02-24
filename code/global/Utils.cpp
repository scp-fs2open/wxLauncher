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

#include "generated/configure_launcher.h"

#include "Utils.h"

namespace TextUtils {
	#include <wx/arrimpl.cpp>
	WX_DEFINE_OBJARRAY(ArrayOfWords);

	void FillArrayOfWordsFromTokens(wxStringTokenizer &tokens,
									wxDC &dc,
									wxFont* testFont,
									ArrayOfWords& words,
									const bool useAppleDebugFilter) {
		while ( tokens.HasMoreTokens() ) {
			wxString tok = tokens.GetNextToken();
#if IS_APPLE
			if (useAppleDebugFilter) {
				// left over from tokenizing executable in debug .app
				if (tok.Lower() == _T("(debug)")) {
					tok = _T("");
				}
				// remove the text after ".app" in the FSO executable name
				// the trailing / ensures that the .app indicates an extension
				int DotAppIndex = tok.Find(_T(".app/"));
				if (DotAppIndex != wxNOT_FOUND) {
					tok = tok.Mid(0, DotAppIndex + 4); // 4 to retain ".app"
				}
			}
#endif
			int x, y;
			dc.GetTextExtent(tok, &x, &y, NULL, NULL, testFont);
			
			Words* temp = new Words();
			temp->size.SetWidth(x);
			temp->size.SetHeight(y);
			temp->word = tok;
			
			words.Add(temp);
		}
	}
}
