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

#ifndef NEWS_SOURCE_H
#define NEWS_SOURCE_H

#include <vector>
#include <wx/string.h>

enum NewsSourceId {
	NEWS_SOURCE_ID_HLP,
	NEWS_SOURCE_ID_DIASPORA
};

class NewsSource {
public:
	NewsSource(NewsSourceId id, const wxString& newsURL, const wxString& label);
	
	const NewsSourceId GetId() const { return this->id; }
	const wxString& GetNewsUrl() const { return this->newsUrl; }
	const wxString& GetLabel() const { return this->label; }
	
	static const NewsSource* FindSource(NewsSourceId id);
private:
	NewsSourceId id;
	wxString newsUrl;
	wxString label;
	
	static void InitializeSources();
	static std::vector<NewsSource> newsSources;
};

#endif
