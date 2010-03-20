#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

//#include <wx/dcbuffer.h>
#include <wx/image.h>
//#include <wx/sound.h>
#include "map_render.h"
#include "wpt_container.h"

#include "ll_geometry.h"

#include "mapwindow.h"
#include "ozex.h"

int ozexWindowDX = 0;
int ozexWindowDY = 0;

int ozexButtonDX = 0;
int ozexButtonDY = 0;
int ozexButtonGap = 0;

/*----------------------------------------------------------------------------*/
IMPLEMENT_CLASS(MapWindow, wxFrame)

BEGIN_EVENT_TABLE(MapWindow, wxFrame)
    EVT_MENU(wxID_EXIT, MapWindow::onFileExit)
    EVT_MENU(wxID_ABOUT, MapWindow::onHelpAbout)
    EVT_IDLE(MapWindow::onIdle)
END_EVENT_TABLE()

/*----------------------------------------------------------------------------*/

MapWindow::MapWindow() 
{
	ozexWindowDX = GET_ENV_INT("OZEX_WINDOW_DX", 800);
	ozexWindowDY = GET_ENV_INT("OZEX_WINDOW_DY", 600);

	ozexButtonDX = GET_ENV_INT("OZEX_BUTTON_DX", 64);
	ozexButtonDY = GET_ENV_INT("OZEX_BUTTON_DY", 64);
	ozexButtonGap = GET_ENV_INT("OZEX_BUTTON_GAP", 10);
	printf("ozexWindowDX: %d\n", ozexWindowDX);
	printf("ozexWindowDY: %d\n", ozexWindowDY);

	printf("ozexButtonDX: %d\n", ozexButtonDX);
	printf("ozexButtonDY: %d\n", ozexButtonDY);
	printf("ozexButtonGap: %d\n", ozexButtonGap);
        Create(0, IDF_FRAME, wxT("Ozex"), wxDefaultPosition, wxSize(ozexWindowDX, ozexWindowDY), 0);
        SetSize(wxSize(ozexWindowDX, ozexWindowDY));
	ptlist = 0;
        panel = new MapView(this);
	panel->Show();

	wxImage::AddHandler(new wxPNGHandler);
	
	CreateToolButtons();
	CreatePtListButtons();
	CreateMapButtons();
	updateButtons();
}

wxBitmapButton* MapWindow::addButton(wxWindow* parent, wxWindowID id, wxString icon)
{
	wxBitmapButton* btn = new wxBitmapButton(parent, id, 
		wxBitmap(wxImage(icon, wxBITMAP_TYPE_PNG)));
        btn -> SetSize(wxSize(ozexButtonDX,ozexButtonDY));
	parent -> GetSizer() -> Add(btn, 0, wxBOTTOM | wxEXPAND, ozexButtonGap);
	return btn;
}

void MapWindow::CreateToolButtons()
{
// tool buttons	
	toolBtns = new wxPanel(this, wxID_ANY, wxPoint(ozexWindowDX - ozexButtonDX, 0), wxSize(ozexButtonDX,ozexWindowDY));
	toolBtns -> SetSizer(new wxBoxSizer( wxVERTICAL ));

	btnPointsList = addButton(toolBtns, IDP_POINTSLIST, wxT("icons/ic_menu_toc.png"));
	btnInfo = addButton(toolBtns, IDP_INFO, wxT("icons/ic_menu_info_details.png"));
	btnNight = addButton(toolBtns, IDP_NIGHT, wxT("icons/ic_menu_night.png"));
	btnMapOnOff = addButton(toolBtns, IDP_MAPONOFF, wxT("icons/ic_menu_report_image.png"));

	toolBtns -> Hide();

	Connect( IDP_POINTSLIST, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onPointList );
	Connect( IDP_INFO, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onInfo );
	Connect( IDP_NIGHT, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onNight );
	Connect( IDP_MAPONOFF, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onMapOnOff );
	
}

void MapWindow::CreatePtListButtons()
{
// point list buttons
	pointListBtns = new wxPanel(this, wxID_ANY, wxPoint(ozexWindowDX - ozexButtonDX, 0), wxSize(ozexButtonDX,ozexWindowDY));
	pointListBtns -> SetSizer(new wxBoxSizer( wxVERTICAL ));

	btnPageUp = addButton(pointListBtns, IDP_PAGEUP, wxT("icons/ic_menu_pageup.png"));
	btnPageDn = addButton(pointListBtns, IDP_PAGEDOWN, wxT("icons/ic_menu_pagedown.png"));
	btnTarget = addButton(pointListBtns, IDP_TARGET, wxT("icons/ic_menu_goto.png"));

	pointListBtns -> Hide();
	
	Connect( IDP_TARGET, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onTarget );
	Connect( IDP_PAGEUP, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onPageUp );
	Connect( IDP_PAGEDOWN, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onPageDn );
}

void MapWindow::CreateMapButtons()
{
// map buttons
	mapBtns = new wxPanel(this, wxID_ANY, wxPoint(ozexWindowDX - ozexButtonDX, 0), wxSize(ozexButtonDX,ozexWindowDY));
	mapBtns -> SetSizer(new wxBoxSizer( wxVERTICAL ));
	
	btnZoomIn = addButton(mapBtns, IDP_ZOOMIN, wxT("icons/ic_menu_zoom.png"));
	btnZoomOut = addButton(mapBtns, IDP_ZOOMOUT, wxT("icons/ic_menu_zoom_out.png"));
	btnMapPrev = addButton(mapBtns, IDP_PREVMAP, wxT("icons/ic_menu_less.png"));
	btnMapNext = addButton(mapBtns, IDP_NEXTMAP, wxT("icons/ic_menu_more.png"));
	btnLoadPoints = addButton(mapBtns, IDP_LOADPOINTS, wxT("icons/ic_menu_archive.png"));
	btnAddPoint = addButton(mapBtns, IDP_ADDPOINT, wxT("icons/ic_menu_myplaces.png"));
	btnTools = addButton(mapBtns, IDP_TOOLS, wxT("icons/ic_menu_manage.png"));
	btnQuit = addButton(mapBtns, wxID_EXIT, wxT("icons/ic_menu_close_clear_cancel.png"));		
		

	Connect( IDP_ZOOMIN, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onZoomIn );
	Connect( IDP_ZOOMOUT, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onZoomOut );
	Connect( IDP_NEXTMAP, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onNextMap );
	Connect( IDP_PREVMAP, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onPrevMap );
	Connect( IDP_LOADPOINTS, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onLoadPoints );
	Connect( IDP_ADDPOINT, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onAddPoint );
	Connect( IDP_TOOLS, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onTools );
	Connect( wxID_EXIT, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onFileExit );

}

/*----------------------------------------------------------------------------*/
void MapWindow::onPageUp(wxCommandEvent &)
{
	int items = 13;

	if (firstItem - items > 0)
	{
		firstItem -= items;
		ptlist->SetFirstItem(firstItem);
	}
	else
	{
		firstItem = 0;
		ptlist->SetFirstItem(firstItem);
	}
}

/*----------------------------------------------------------------------------*/
void MapWindow::onPageDn(wxCommandEvent &)
{
	int items = 13;

        wpt_container* waypoints = wxGetApp().getWaypoints();
	if (firstItem + items < waypoints->points_avail())
	{
		firstItem += items;
		ptlist->SetFirstItem(firstItem);
	}
}


/*----------------------------------------------------------------------------*/
void MapWindow::ShowToolButtons()
{
	toolBtns -> Show();
	if (wxGetApp().getWaypoints()->points_avail())
		btnPointsList->Enable();
	else
		btnPointsList->Disable();
}

/*----------------------------------------------------------------------------*/
void MapWindow::ShowMapButtons()
{
	mapBtns -> Show();
	updateButtons();
}


/*----------------------------------------------------------------------------*/
void MapWindow::HideToolButtons()
{
	toolBtns -> Hide();
}

/*----------------------------------------------------------------------------*/
void MapWindow::HideMapButtons()
{
	mapBtns -> Hide();
}

/*----------------------------------------------------------------------------*/
void MapWindow::ShowPtListButtons()
{
	pointListBtns -> Show();
}

/*----------------------------------------------------------------------------*/
void MapWindow::HidePtListButtons()
{
	pointListBtns -> Hide();
}

/*----------------------------------------------------------------------------*/
void MapWindow::onIdle(wxIdleEvent &)
{
	updateButtons();
}

/*----------------------------------------------------------------------------*/
void MapWindow::updateButtons()
{
//	SetCursor(wxCursor(NULL));
	map_render* render = panel->getMapRender();
	if (!render)
		return;

	if (render->has_map_prev())
		btnMapPrev->Enable();
	else
		btnMapPrev->Disable();
	

	if (render->has_map_next())
		btnMapNext->Enable();
	else
		btnMapNext->Disable();
		
	if (render->zoom_in_avail())
		btnZoomIn->Enable();
	else
		btnZoomIn->Disable();
		
	if (render->zoom_out_avail())
		btnZoomOut->Enable();
	else
		btnZoomOut->Disable();

}

/*----------------------------------------------------------------------------*/
void MapWindow::onMapOnOff(wxCommandEvent &)
{
	HideToolButtons();
	ShowMapButtons();
	
	panel -> mapOnOff();

	panel->Refresh(false);
}

/*----------------------------------------------------------------------------*/
void MapWindow::onPointList(wxCommandEvent &)
{
	HideToolButtons();
	ShowPtListButtons();	
	
	panel->Hide();
	
	if (ptlist)
		delete ptlist;
	
	
	firstItem = 0;
	ptlist = new wxListBox(this, wxID_ANY);
	wxSize mapSize = panel -> getMapSize();
	ptlist->SetSize(0, 0, mapSize.GetWidth(), mapSize.GetHeight(), wxSIZE_FORCE);
	ptlist->SetWindowStyle(ptlist->GetWindowStyle() & ~wxHSCROLL);
	
	int j = 0;
	map_render* render = panel -> getMapRender();
	
        wpt_container* waypoints = wxGetApp().getWaypoints();
	for (int i = 0; i < waypoints->points_avail(); i++)
	{
		if (waypoints->point_active(i))
		{
			char* name = waypoints->point_name(i);
			wxString wxname = wxString::FromAscii((const char*)name);
			
			wxname += wxString(wxT(":"));
			
			double lat, lon;
		
			waypoints->point_coords(i, &lat, &lon);
		
			double center_lat, center_lon;
			render->center_get(&center_lat, &center_lon);

			double dist = ll_to_distance(lat, lon, center_lat, center_lon);

			wxname += format_dist(dist);

			ptlist->InsertItems(1, &wxname, j);
			j++;
		}
	}
	
	wxFont font = ptlist->GetFont();
	font.SetPointSize(24);
	font.SetWeight(wxFONTWEIGHT_BOLD);
	ptlist->SetFont( font );
	
	ptlist->Show();
}

/*----------------------------------------------------------------------------*/
void MapWindow::onInfo(wxCommandEvent &)
{
	HideToolButtons();
	ShowMapButtons();
	
        wpt_container* waypoints = wxGetApp().getWaypoints();
	int n = waypoints->points_avail();
	double nearest = 1000000 * 1000; // 1 mln km 
	int k = 0;
	map_render* render = panel -> getMapRender();

	for (int i = 0; i < n; i ++)
	{
		double lat, lon;
		
		waypoints->point_coords(i, &lat, &lon);
		
		if (waypoints->point_active(i))
		{
			double center_lat, center_lon;
			render->center_get(&center_lat, &center_lon);

			double dist = ll_to_distance(lat, lon, center_lat, center_lon);

			if (dist < nearest)
			{
				k = i;
				nearest = dist; 
			}
		}
	}


	wxString s;

	double lat;
	double lon;
	double zoom = render -> zoom_get();
	
	render -> center_get(&lat, &lon);

	wxString status;
	status.Printf(wxT("Position:\n %f %f\n\n"), lat, lon);

	wxString point;
	waypoints->point_coords(k, &lat, &lon);

	if (n)
	{
		point.Printf(wxT("Nearest waypoint: %s, "), format_dist(nearest).c_str());
		point += wxString::FromAscii(waypoints->point_name(k));
		status += point;
	}

	wxMessageBox(status, wxT("Connection Status"), wxOK | wxICON_INFORMATION);

}

/*----------------------------------------------------------------------------*/
void MapWindow::onNight(wxCommandEvent &)
{
	HideToolButtons();
	ShowMapButtons();
	map_render* render = panel -> getMapRender();
	render->inverse();
	render->render();
	Refresh(false);
}

/*----------------------------------------------------------------------------*/
void MapWindow::onTarget(wxCommandEvent &)
{
	HidePtListButtons();
	ShowMapButtons();
	
	
	int j = 0;
	map_render* render = panel -> getMapRender();
        wpt_container* waypoints = wxGetApp().getWaypoints();
	
	for (int i = 0; i < waypoints->points_avail(); i++)
	{
		if (waypoints->point_active(i))
		{
			if (ptlist->IsSelected(j))
			{
				double lat, lon;
		
				waypoints->point_coords(i, &lat, &lon);
				
				render->center_set(lat, lon);

				break;
			}		
			
			j++;
		}
	}

	
	
	
	panel->Show();
}

void MapWindow::SavePoints()
{
        wpt_container* waypoints = wxGetApp().getWaypoints();
	if (waypoints->changed())
	{
		int answer = 
			wxMessageBox(wxT("Save waypoints?"),
				         wxT("Confirm"), wxYES_NO | wxICON_QUESTION);
		
		if (answer == wxYES)
		{
			wxString filename = 
				wxFileSelector(	wxT("Enter filename"),
								wxT(""), wxT(""), 
								wxT("wpt"), 
								wxT("Waypoints (*.wpt)|*.wpt"),
								wxFD_SAVE | wxFD_OVERWRITE_PROMPT, this );

			if ( !filename.empty() )
			{
				waypoints->save(filename.ToAscii());
			}

		}		

	}

}

/*----------------------------------------------------------------------------*/
void MapWindow::onLoadPoints(wxCommandEvent &)
{
	SavePoints();
        wpt_container* waypoints = wxGetApp().getWaypoints();
	wxString filename = 
		wxFileSelector(	wxT("Enter filename"),
						wxT(""), wxT(""), 
						wxT("wpt"), 
						wxT("Waypoints (*.wpt)|*.wpt"),
						wxFD_OPEN | wxFD_FILE_MUST_EXIST, this );

	if ( !filename.empty() )
	{
		waypoints->load(filename.ToAscii());

		int n = waypoints->points_avail();

		if (n)
		{
			double lat, lon;
			map_render* render = panel -> getMapRender();

			waypoints->point_coords(0, &lat, &lon);

			render->center_set(lat, lon);
		}

		Refresh(false);
	}
}


/*----------------------------------------------------------------------------*/
void MapWindow::onTools(wxCommandEvent &)
{
	HideMapButtons();
	ShowToolButtons();
}

/*----------------------------------------------------------------------------*/
inline void MapWindow::onFileExit(wxCommandEvent &) 
{ 
	SavePoints();
	Close(); 
}

/*----------------------------------------------------------------------------*/
MapView* MapWindow::getPanel() 
{ 
	return panel;
}

/*----------------------------------------------------------------------------*/
inline void MapWindow::onAddPoint(wxCommandEvent &) 
{
        wpt_container* waypoints = wxGetApp().getWaypoints();
	int max = -1;

	int n = waypoints->points_avail();

	for (int i = 0; i < n; i ++)
	{
		if (waypoints->point_active(i))
		{
			char* name = waypoints->point_name(i);

			int k = atoi(name);
		
			if (k > max)
				max = k;
		}
	}
	
	if (max == -1)
		max = 1;
	else
		max++;

	char tmp[128];
	
	sprintf(tmp, "%d", max);

	double lat;
	double lon;
	
	map_render* render = panel -> getMapRender();
	render->center_get(&lat, &lon);
	
	waypoints->point_add(tmp, lat, lon);

	Refresh(false);
}

/*----------------------------------------------------------------------------*/
inline void MapWindow::onZoomIn(wxCommandEvent &) 
{
	panel -> getMapRender()->zoom_in();
	Refresh(false);
}

/*----------------------------------------------------------------------------*/
inline void MapWindow::onZoomOut(wxCommandEvent &)
{
	panel -> getMapRender()->zoom_out();
	Refresh(false);
}

/*----------------------------------------------------------------------------*/
inline void MapWindow::onNextMap(wxCommandEvent &)
{
	panel -> getMapRender()->map_next();
	updateButtons();
	Refresh(false);
}

/*----------------------------------------------------------------------------*/
inline void MapWindow::onPrevMap(wxCommandEvent &)
{
	panel -> getMapRender()->map_prev();
	updateButtons();
	Refresh(false);
}

/*----------------------------------------------------------------------------*/
void MapWindow::onHelpAbout(wxCommandEvent &) 
{
    wxMessageBox(wxT("Ozex\nCopyright (C) 2005,2010 Daniil Smelov"),
                 wxT("About Ozex"), wxOK | wxICON_INFORMATION);
}
