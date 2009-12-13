#include "mapview.h"

/*-----------------------------------------------------------------------------*/
class MapWindow : public wxFrame 
{
    DECLARE_CLASS(MapWindow)
    DECLARE_EVENT_TABLE()
        
private:
    MapView *panel;
    
    wxListBox* ptlist;
    
    int firstItem;
    
    wxBitmapButton *btnGpsOnOff,
    				*btnZoomIn,
    				*btnZoomOut,
    				*btnMapPrev,
    				*btnMapNext,
    				*btnTools,
    				*btnLoadPoints,
    				*btnAddPoint,
    				*btnQuit,
    				*btnPointsList,
    				*btnInfo,
    				*btnNight,
    				*btnTarget,
    				*btnMapOnOff,
					*btnPageUp,
					*btnPageDn;
    wxPanel *toolBtns, *pointListBtns, *mapBtns;
    
    void onFileExit(wxCommandEvent &);
    void onHelpAbout(wxCommandEvent &);

	void onZoomIn(wxCommandEvent &);
	void onZoomOut(wxCommandEvent &);
	void onNextMap(wxCommandEvent &);
	void onPrevMap(wxCommandEvent &);
	void onGpsOnOff(wxCommandEvent &);
	void onLoadPoints(wxCommandEvent &);
	void onAddPoint(wxCommandEvent &);
	void onTools(wxCommandEvent &);

	void onPointList(wxCommandEvent &);
	void onInfo(wxCommandEvent &);
	void onNight(wxCommandEvent &);
	void onTarget(wxCommandEvent &);
	void onMapOnOff(wxCommandEvent &);

	void onPageUp(wxCommandEvent &);
	void onPageDn(wxCommandEvent &);
	
    void onIdle(wxIdleEvent &);
    
//    void ShowTools();
//    void HideTools();

	void ShowToolButtons();
	void ShowMapButtons();
	void ShowPtListButtons();
	void HideToolButtons();
	void HideMapButtons();
	void HidePtListButtons();

	wxBitmapButton* addButton(wxWindow* parent, wxWindowID id, wxString icon);
	void CreateToolButtons();
	void CreateMapButtons();
	void CreatePtListButtons();
	void SavePoints();
		
public:
    MapWindow();
    
    MapView *getPanel();
	void updateButtons();
};

