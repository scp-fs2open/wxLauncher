#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <wx/wx.h>
#include <wx/notebook.h>

class MainWindow: public wxFrame {
public:
	MainWindow();
	~MainWindow();

private:
	wxNotebook* mainTab;
};

#endif