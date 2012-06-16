/*
 Copyright (C) 2012 wxLauncher Team
 
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

#include "controls/TruncatableChoice.h"

TruncatableChoice::TruncatableChoice(wxWindow *parent, wxWindowID id)
: wxChoice(parent, id), maxLength(0) {
}

void TruncatableChoice::SetMaxLength(const int maxLength) {
	wxCHECK_RET(maxLength >= 0,
		wxString::Format(_T("Invalid value %d for maxLength."), maxLength));
	
	this->maxLength = maxLength;
	
	if (maxLength == 0) {
		this->SetMinSize(this->GetEffectiveMinSize());
	} else {
		this->SetMinSize(wxSize(maxLength, -1));
	}
}

wxSize TruncatableChoice::DoGetBestSize() const {
	wxASSERT(this->maxLength >= 0);
	
	wxSize bestChoiceSize(wxChoice::DoGetBestSize());
	
	if ((this->maxLength == 0) || (this->maxLength > bestChoiceSize.GetWidth())) {
		return bestChoiceSize;
	} else {
		return wxSize(wxSize(maxLength, bestChoiceSize.GetHeight()));
	}
}
