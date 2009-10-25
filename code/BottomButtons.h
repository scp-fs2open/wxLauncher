#ifndef BOTTOMBUTTONS_H
#define BOTTOMBUTTONS_H

#include <wx/wx.h>

class JobData {
};

WX_DECLARE_HASH_MAP( int, JobData*, wxIntegerHash, wxIntegerEqual, JobTable);

class BottomButtons: public wxPanel {
public:
	BottomButtons(wxWindow* parent, wxPoint &pos, wxSize &size);

	void Done();
	void UpdateBars();

private:
	wxButton *close, *help, *fred, *log, *update, *play;
	wxGauge* bar;

	JobTable* joblist;
	wxFont *disabledFont, *enabledFont;

};

#endif