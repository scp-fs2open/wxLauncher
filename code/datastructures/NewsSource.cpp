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

#include <wx/intl.h> // for _()
#include <wx/log.h>
#include "NewsSource.h"

using std::vector;

vector<NewsSource> NewsSource::newsSources;

NewsSource::NewsSource(const NewsSourceId id, const wxString& newsUrl, const wxString& label)
: id(id), newsUrl(newsUrl), label(label) {
	wxASSERT(!newsUrl.IsEmpty());
	wxASSERT(!label.IsEmpty());
}

const NewsSource* NewsSource::FindSource(const NewsSourceId id) {
	wxCHECK_MSG(id != NEWS_SOURCE_ID_INVALID, NULL,
		_T("FindSource given NEWS_SOURCE_ID_INVALID!"));
	
	if (newsSources.empty()) {
		InitializeSources();
	}
	
	for (vector<NewsSource>::const_iterator it = newsSources.begin(),
			end = newsSources.end(); it != end; ++it) {
		if (it->GetId() == id) {
			return &(*it);
		}
	}
	
	wxLogWarning(_T("FindSource(): Unknown news source ID %d, returning NULL"), id);
	
	return NULL;
}

void NewsSource::InitializeSources() {
		wxASSERT_MSG(newsSources.empty(),
			_T("news sources have already been initialized"));
	
	newsSources.push_back(
		NewsSource(
			NEWS_SOURCE_ID_HLP,
			_T("http://www.audiozone.ro/hl/"),
			_("Latest highlights from Hard Light Productions")));
	newsSources.push_back(
		NewsSource(
			NEWS_SOURCE_ID_DIASPORA,
			_T("http://diaspora.hard-light.net/hl.htm"),
			_("Latest Diaspora news")));
}
