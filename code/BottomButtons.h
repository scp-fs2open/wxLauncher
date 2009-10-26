#ifndef BOTTOMBUTTONS_H
#define BOTTOMBUTTONS_H

#include <wx/wx.h>

class BottomButtons: public wxPanel {
public:
	BottomButtons(wxWindow* parent, wxPoint &pos, wxSize &size);
private:
	wxButton *close, *help, *fred, *update, *play;
	wxPanel *gaugePanel, *log;

};

#endif