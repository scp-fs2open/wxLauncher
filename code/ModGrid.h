#ifndef MODGRID_H
#define MODGRID_H

#include <wx/wx.h>
#include <wx/grid.h>

/** Implementation of wxGrid for use on the mod select tab. The resulting
object is a full grid that can be just placed by the layout code. */
class ModGrid: public wxGrid {
public:
	ModGrid(wxWindow* parent, wxSize& size);
};

/** ModGridTable implements wxGridTableBase so that ModGrid can access the 
mods for the TC and display them to the user. */
class ModGridTable: public wxGridTableBase {
public:
	ModGridTable();
	virtual ~ModGridTable();

	/* overriding as per wx documentation. */
	virtual int GetNumberRows();
	virtual int GetNumberCols();

	virtual bool IsEmptyCell(int row, int col);

	virtual wxString GetValue(int row, int col);
	virtual void SetValue(int row, int col, const wxString& value);

	virtual wxString GetTypeName(int row, int col);

	virtual bool CanGetValueAs(int row, int col, const wxString& typeName);
	virtual bool CanSetValueAs(int row, int col, const wxString& typeName);

	virtual long GetValueAsLong(int row, int col);
	virtual double GetValueAsDouble(int row, int col);
	virtual bool GetValueAsBool(int row, int col);

	virtual void SetValueAsLong(int row, int col, long value);
	virtual void SetValueAsDouble(int row, int col, double value);
	virtual void SetValueAsBool(int row, int col, bool value);

	virtual void* GetValueAsCustom(int row, int col, const wxString& typeName);
	virtual void SetValueAsCustom(int row, int col, const wxString& typeName, void* value);

};

/** Renders the mod's name on the ModGrid.  Extends wxGridCellRender so that
it can be plugged into ModGrid. */
class ModGridNameRenderer: public wxGridCellRenderer {
public:
	/** http://docs.wxwidgets.org/stable/wx_wxgridcellrenderer.html#wxgridcellrendererdraw */
	virtual void Draw(wxGrid &grid, wxGridCellAttr &attr, wxDC& dc,
		const wxRect &rect, int row, int col, bool isSelected);
	/** http://docs.wxwidgets.org/stable/wx_wxgridcellrenderer.html#wxgridcellrenderergetbestsize */
	virtual wxSize GetBestSize(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc,
		int row, int col);
};

/** Renders the mod's logo on the ModGrid.  Extends wxGridCellRender so that
it can be plugged into ModGrid. */
class ModGridLogoRenderer: public wxGridCellRenderer {
public:
	/** http://docs.wxwidgets.org/stable/wx_wxgridcellrenderer.html#wxgridcellrendererdraw */
	virtual void Draw(wxGrid &grid, wxGridCellAttr &attr, wxDC& dc,
		const wxRect &rect, int row, int col, bool isSelected);
	/** http://docs.wxwidgets.org/stable/wx_wxgridcellrenderer.html#wxgridcellrenderergetbestsize */
	virtual wxSize GetBestSize(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc,
		int row, int col);
};

/** Renders the mod's Description on the ModGrid.  Extends wxGridCellRender so that
it can be plugged into ModGrid. */
class ModGridDescriptionRenderer: public wxGridCellRenderer {
public:
	/** http://docs.wxwidgets.org/stable/wx_wxgridcellrenderer.html#wxgridcellrendererdraw */
	virtual void Draw(wxGrid &grid, wxGridCellAttr &attr, wxDC& dc,
		const wxRect &rect, int row, int col, bool isSelected);
	/** http://docs.wxwidgets.org/stable/wx_wxgridcellrenderer.html#wxgridcellrenderergetbestsize */
	virtual wxSize GetBestSize(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc,
		int row, int col);
};

/** Renders the mod's options buttons on the ModGrid.  Extends wxGridCellRender so that
it can be plugged into ModGrid. */
class ModGridOptionsRenderer: public wxGridCellRenderer {
public:
	/** http://docs.wxwidgets.org/stable/wx_wxgridcellrenderer.html#wxgridcellrendererdraw */
	virtual void Draw(wxGrid &grid, wxGridCellAttr &attr, wxDC& dc,
		const wxRect &rect, int row, int col, bool isSelected);
	/** http://docs.wxwidgets.org/stable/wx_wxgridcellrenderer.html#wxgridcellrenderergetbestsize */
	virtual wxSize GetBestSize(wxGrid& grid, wxGridCellAttr& attr, wxDC& dc,
		int row, int col);
};

#endif