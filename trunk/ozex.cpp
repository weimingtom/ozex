
#include <iostream>

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <wx/dcbuffer.h>
#include <wx/image.h>
#include <wx/sound.h>

#include "SDL.h"

#include "map_loader.h"
#include "map_container.h"
#include "map_index.h"
#include "map_datums.h"
#include "map_render.h"
#include "wpt_container.h"
#include "log_stream.h"

#include "ozex_gps.h"

#include "ll_geometry.h"

int ozexWindowDX = 0;
int ozexWindowDY = 0;

int ozexButtonDX = 0;
int ozexButtonDY = 0;
int ozexButtonGap = 0;

int ozexMapDX = 0;
int ozexMapDY = 0;

/*----------------------------------------------------------------------------*/
map_index* collection = NULL;
map_render* render = NULL;
int gpsOn = 0, mapOn = 1;

wpt_container* waypoints = NULL;


/*----------------------------------------------------------------------------*/
enum 
{
    IDF_FRAME = wxID_HIGHEST + 1,
    IDP_PANEL,
    
    IDP_ZOOMIN,
    IDP_ZOOMOUT,
 	IDP_NEXTMAP,
 	IDP_PREVMAP,
 	IDP_GPSONOFF,
 	IDP_TOOLS,
 	IDP_ADDPOINT,
	IDP_LOADPOINTS,
	IDP_POINTSLIST,
	IDP_INFO,
	IDP_NIGHT,
	IDP_TARGET,
	IDP_MAPONOFF,
	IDP_PAGEUP,
	IDP_PAGEDOWN,
};


/*----------------------------------------------------------------------------*/
class MapView : public wxPanel 
{
    DECLARE_CLASS(MapView)
    DECLARE_EVENT_TABLE()
    
private:
    SDL_Surface *screen;
    SDL_Surface *bg;
	SDL_Surface* tile;

	wxBitmap* position;
	wxBitmap* radioOn;
	wxBitmap* radioOff;
	wxBitmap* point;
	wxBitmap* pointnew;

    void onPaint(wxPaintEvent &);
    void onEraseBackground(wxEraseEvent &);
    
    void onIdle(wxIdleEvent &);
    void OnMouseDown( wxMouseEvent &event );
    void OnMouseUp( wxMouseEvent &event );
    void OnMouseMove( wxMouseEvent &event );
    void Clear(void);

	long pen_x, pen_y, delta_x, delta_y, pressed;
    
public:

	int isPressed();
    void createScreen();
    MapView(wxFrame *parent);
    ~MapView();
};

#include "SDL_image.h"

/*----------------------------------------------------------------------------*/
int MapView::isPressed()
{
	return pressed;
}

#include "SDL_image.h"

/*----------------------------------------------------------------------------*/
void MapView::Clear(void)
{
/*
	SDL_FillRect(screen, NULL, 0xFFEEEEEE);
	
	return;
*/

    SDL_Surface *view = screen;

	if(SDL_MUSTLOCK(view))
		SDL_LockSurface(view);


//	SDL_BlitSurface (bg, NULL, view, NULL);

/*
	int rects_per_x = view->w/tile->w + 1;
	int rects_per_y = view->h/tile->h + 1;
	
	for (int i = 0; i < rects_per_y; i++)
	{
		for (int j = 0; j < rects_per_x; j++)
		{
			SDL_Rect rect;
		
			rect.x = j * tile->w;
			rect.y = i * tile->h;
			rect.w = tile->w;
			rect.h = tile->h;
			
			SDL_BlitSurface (tile, NULL, view, &rect);
		}
	}
*/

//	SDL_FillRect(view, NULL, 0xFFFFFFFF);

	int colors[] = {0xFFD4D4D4, 0xFFFFFFFF};
		
	int rects_per_x = view->w/16 + 1;
	int rects_per_y = view->h/16 + 1;
	
	int color_index = 0;
		
	for (int i = 0; i < rects_per_y; i++)
	{
		color_index = i & 1;
		for (int j = 0; j < rects_per_x; j++)
		{
			SDL_Rect rect;
		
			rect.x = j * 16;
			rect.y = i * 16;
			rect.w = 16;
			rect.h = 16;
			
			SDL_FillRect(view, &rect, colors[color_index]);
			
			color_index ^= 1;
		}
	}

	if(SDL_MUSTLOCK(view))
		SDL_UnlockSurface(view);
}    

/*----------------------------------------------------------------------------*/
inline void MapView::onEraseBackground(wxEraseEvent &) 
{ 
}

/*----------------------------------------------------------------------------*/
IMPLEMENT_CLASS(MapView, wxPanel)

BEGIN_EVENT_TABLE(MapView, wxPanel)
    EVT_PAINT(MapView::onPaint)
    EVT_ERASE_BACKGROUND(MapView::onEraseBackground)
    EVT_IDLE(MapView::onIdle)
	EVT_LEFT_DOWN( MapView::OnMouseDown )
	EVT_LEFT_UP( MapView::OnMouseUp )
	EVT_MOTION( MapView::OnMouseMove )
END_EVENT_TABLE()

/*----------------------------------------------------------------------------*/
MapView::MapView(wxFrame *parent) : wxPanel(parent, IDP_PANEL), screen(0) 
{
	delta_x = delta_y = 0;
	pressed = 0;

	wxImage::AddHandler(new wxPNGHandler);

	position = new wxBitmap(wxImage(wxT("icons/cursor_standing.png"), wxBITMAP_TYPE_PNG));
	radioOn = new wxBitmap(wxImage(wxT("icons/btn_radio_on.png"), wxBITMAP_TYPE_PNG));
	radioOff = new wxBitmap(wxImage(wxT("icons/btn_radio_off.png"), wxBITMAP_TYPE_PNG));
	point = new wxBitmap(wxImage(wxT("icons/map_point_red.png"), wxBITMAP_TYPE_PNG));
	pointnew = new wxBitmap(wxImage(wxT("icons/map_point_green.png"), wxBITMAP_TYPE_PNG));


	Uint32 rmask, gmask, bmask, amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0xff000000;
		gmask = 0x00ff0000;
		bmask = 0x0000ff00;
		amask = 0x000000ff;
#else
		rmask = 0x000000ff;
		gmask = 0x0000ff00;
		bmask = 0x00ff0000;
		amask = 0xff000000;
#endif

}

/*----------------------------------------------------------------------------*/
MapView::~MapView() 
{
	delete position;
	delete radioOn;
	delete radioOff;
	delete point;
	delete pointnew;

	SDL_FreeSurface(tile);
	SDL_FreeSurface(bg);
    if (screen) 
    {
        SDL_FreeSurface(screen);
    }
}

/*----------------------------------------------------------------------------*/
void MapView::onPaint(wxPaintEvent &) 
{
    if(!screen)
	createScreen();
    if (!screen || !position || !bg) 
        return;
    
    if (SDL_MUSTLOCK(screen)) 
    {
        if (SDL_LockSurface(screen) < 0) 
            return;
    }

	SDL_Rect src, dst;
	render->rect(&src);
	render->rect(&dst);
	
	dst.x += delta_x;
	dst.y += delta_y;

	dst.w -= delta_x;
	dst.h -= delta_y;

	if (!mapOn || pressed || dst.w != screen->w || dst.h != screen->h)
	{
		Clear();
	}
		
	
		

	if (mapOn)
		SDL_BlitSurface (render->surface(), &src, screen, &dst);
    
    wxBitmap bmp(wxImage(screen->w, screen->h, 
                    static_cast<unsigned char *>(screen->pixels), true));
    
	wxPaintDC dc(this);

	dc.DrawBitmap(bmp, 0, 0, false);

	wxPen backup = dc.GetPen();

	wxPen pen;

	pen.SetColour(wxColour(255, 0, 0));
	pen.SetWidth(1);
	dc.SetPen(pen);

	dc.DrawLine(screen->w/2, 0, screen->w/2, screen->h);
	dc.DrawLine(0, screen->h/2, screen->w, screen->h/2);

	pen.SetColour(wxColour(200, 0, 0));
	pen.SetWidth(1);
	dc.SetPen(pen);

	dc.DrawLine(screen->w/2-1, 0, screen->w/2-1, screen->h);
	dc.DrawLine(0, screen->h/2-1, screen->w, screen->h/2-1);
	dc.DrawLine(screen->w/2+1, 0, screen->w/2+1, screen->h);
	dc.DrawLine(0, screen->h/2+1, screen->w, screen->h/2+1);

	dc.SetPen(backup);
	
	map_container* mapview = render->getmap();
	if (mapview && !pressed || (pressed && delta_x == 0 && delta_y == 0))
	{
	
		double nearest = 1000000 * 1000; // 1 mln km 
		int k = 0;
		double center_lat, center_lon;
		render->center_get(&center_lat, &center_lon);

		int n = waypoints->points_avail();

		for (int i = 0; i < n; i ++)
		{
			double lat, lon;
			
			waypoints->point_coords(i, &lat, &lon);


			if (waypoints->point_active(i))
			{
				double dist = ll_to_distance(lat, lon, center_lat, center_lon);

				if (dist < nearest)
				{
					k = i;
					nearest = dist; 
				} 

			}
			
			if (mapOn && !mapview->ll_inside_map(lat, lon))
				continue;
			
			if ( waypoints->point_active(i))
			{
				
				long point_x, point_y, center_x, center_y;
				
				mapview->ll_to_xy_onmap(center_lat, center_lon, &center_x, &center_y);
				mapview->ll_to_xy_onmap(lat, lon, &point_x, &point_y);

				int visible_x = point_x - center_x;
				int visible_y = point_y - center_y;


				int x = ozexMapDX/2 + visible_x - point->GetWidth()/2;
				int y = ozexMapDY/2 + visible_y - point->GetHeight();

				if (x >= 0 && x <= ozexMapDX && y >= 0 && y <= ozexMapDY)
				{
					char* name = waypoints->point_name(i);
					
					if (name && name[0])
					{
						wxString wxname = wxString::FromAscii((const char*)name);
					
						wxFont font = dc.GetFont();
						font.SetPointSize(9);
						font.SetWeight(wxFONTWEIGHT_BOLD);
						dc.SetFont( font );
					
						wxColour f, b;

						b.Set(255, 255, 255);
						dc.SetTextBackground(b);
						f.Set(0, 0, 0);
						dc.SetTextForeground(f);
					
						//dc.SetBackgroundMode(wxSOLID);
						wxCoord w, h;
						dc.GetTextExtent(wxname, &w, &h);

						dc.DrawRectangle(ozexMapDX/2 + visible_x - (w+2)/2, 
												ozexMapDY/2 + visible_y + 1, (w+2), h);
						/*
						dc.DrawEllipse(ozexMapDX/2 + visible_x - w/2, 
									ozexMapDY/2 + visible_y + 1, w, h);
						*/
						dc.DrawText(wxname, ozexMapDX/2 + visible_x - w/2, 
									ozexMapDY/2 + visible_y + 1);
					}								
					
					if (waypoints->added(i))
						dc.DrawBitmap(*pointnew, x, y, true);
					else
						dc.DrawBitmap(*point, x, y, true);
				}
			}
			
			wxString point;
			waypoints->point_coords(k, &lat, &lon);
			wxString status;

			if (n)
			{
				if (nearest < 1000)
				{
					point.Printf(wxT("Nearest waypoint: %.0f metres, "), 
									nearest);
					point += wxString::FromAscii(waypoints->point_name(k));

				}
				else
				if (nearest < 1000 * 10)
				{
					point.Printf(wxT("Nearest waypoint: %2.1f km, "), 
									nearest/1000);
					point += wxString::FromAscii(waypoints->point_name(k));
				}
				else
				{
					point.Printf(wxT("Nearest waypoint: %.0f km, "), 
									nearest/1000);
					point += wxString::FromAscii(waypoints->point_name(k));
				}

				status += point;
				
				
				wxFont font = dc.GetFont();
				font.SetPointSize(9);
				font.SetWeight(wxFONTWEIGHT_BOLD);
				dc.SetFont( font );
			
				wxColour f, b;

				b.Set(255, 255, 255);
				dc.SetTextBackground(b);
				f.Set(0, 0, 0);
				dc.SetTextForeground(f);
			
				wxCoord w, h;
				dc.GetTextExtent(status, &w, &h);

				dc.DrawRectangle(ozexMapDX/2 - (w+2)/2, 
								ozexMapDY - h - 1, (w+2), h + 1);

				dc.DrawText(status, ozexMapDX/2 - w/2, 
							ozexMapDY - h);

				
				
				
			}
				

			
			
		}
		
	}


//	dc.DrawBitmap(*position, screen->w/2 - position->GetWidth()/2, screen->h/2  - position->GetHeight()/2, true);

    if (gpsOn)
    {
	    dc.DrawBitmap(*radioOn, 0, 0, true);
    }
    else
    {
	    dc.DrawBitmap(*radioOff, 0, 0, true);
	}    

    if (SDL_MUSTLOCK(screen)) 
        SDL_UnlockSurface(screen);

}


///////////////////////////////////////////////////////////////////////////////

/*----------------------------------------------------------------------------*/
void  MapView::OnMouseUp( wxMouseEvent &event )
{
	if (gpsOn)
	{
	    event.Skip();
		return;
	}

	if (pressed)
	{
		long x, y;
	
		event.GetPosition(&x, &y);
	
		pen_x = x;
		pen_y = y;

		if (delta_x || delta_y)
		{
			render->scroll_by(-delta_x, -delta_y);

			Refresh(false);
		}
		else
		{
			map_container* mapview = render->getmap();
			
			if (mapview)
			{
				double lat, lon;

				int n = waypoints->points_avail();

				double center_lat, center_lon;
				long center_x, center_y;

				render->center_get(&center_lat, &center_lon);
				mapview->ll_to_xy_onmap(center_lat, center_lon, &center_x, &center_y);
				
				center_x += (x - ozexMapDX/2);
				center_y += (y - ozexMapDY/2);
				
				for (int i = 0; i < n; i ++)
				{
			
					waypoints->point_coords(i, &lat, &lon);
			
					if (waypoints->point_active(i))
					{
					
				
						long point_x, point_y;
				
						mapview->ll_to_xy_onmap(lat, lon, &point_x, &point_y);

						int visible_x = point_x;
						int visible_y = point_y;
					
						if ( center_x >= visible_x - point->GetWidth()/2 &&
							 center_x <= visible_x + point->GetWidth()/2 &&
							 center_y <= visible_y &&
							 center_y >= visible_y - point->GetHeight())
						{
							char* name = waypoints->point_name(i);
					
							wxString wxname(wxT("Delete waypoint \""));
							wxname += wxString::FromAscii((const char*)name);
							wxname += wxString(wxT("\"?"));
						
							int answer = 
								wxMessageBox(wxname,
											 wxT("Confirm"), wxYES_NO | wxICON_QUESTION);
		
							if (answer == wxYES)
							{
								waypoints->point_delete(i);
								Refresh(false);
							}

							break;
						}

					}
				}
				
			}
		
		}
		
		pressed = 0;
		delta_x = 0;
		delta_y = 0;
	}

    event.Skip();
  
}

/*----------------------------------------------------------------------------*/
void  MapView::OnMouseMove( wxMouseEvent &event )
{
	if (gpsOn)
	{
	    event.Skip();
		return;
	}

	
	long x, y;
	
	event.GetPosition(&x, &y);

	if (pressed)
	{
		delta_x = -(pen_x - x);
		delta_y = -(pen_y - y);
		Refresh(false);
	}

    event.Skip();
}


/*----------------------------------------------------------------------------*/
void MapView::OnMouseDown(wxMouseEvent &event)
{
	long x, y;
	
	event.GetPosition(&x, &y);

	if ( x <= radioOn->GetWidth() &&
		y <= radioOn->GetHeight())
	{
		if (gpsOn)
		{
			ozex_gps_close();
			gpsOn = 0;
		}
		else
		{
			int debug = atoi(getenv("OZEX_USE_LOG"));
		
			if (debug)
			{
				ozex_gps_start_fromfile(getenv("OZEX_GPS_LOG"));
			}
			else
			{
				ozex_gps_start(getenv("OZEX_GPS_DEV"), atoi(getenv("OZEX_GPS_BAUDS")));
			}
	
			gpsOn = 1;
		}
	    event.Skip();
		Refresh(false);
	    return;
	}
	else
	if (gpsOn)
	{
	    event.Skip();
		return;
	}


	pen_x = x;
	pen_y = y;
	pressed = 1;
	delta_x = 0;
	delta_y = 0;

/*
	long x, y;
	
	event.GetPosition(&x, &y);
	
	render->scroll_by(ozexMapWidth/2 - , -delta_y);
*/

	
    event.Skip();
}

/*----------------------------------------------------------------------------*/
void MapView::createScreen() 
{
    if (!screen) 
    {
        int width, height;
        GetSize(&width, &height);

		Uint32 rmask, gmask, bmask, amask;

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
			rmask = 0xff000000;
			gmask = 0x00ff0000;
			bmask = 0x0000ff00;
			amask = 0x000000ff;
#else
			rmask = 0x000000ff;
			gmask = 0x0000ff00;
			bmask = 0x00ff0000;
			amask = 0xff000000;
#endif

			screen = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 24,
										rmask, gmask, bmask, amask);

			tile = IMG_Load("./icons/loading_tile.png");


			bg = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32,
										rmask, gmask, bmask, amask);

			int rects_per_x = bg->w/tile->w + 1;
			int rects_per_y = bg->h/tile->h + 1;

			SDL_FillRect(bg, NULL, 0xFFFFFFFF);
	
			for (int i = 0; i < rects_per_y; i++)
			{
				for (int j = 0; j < rects_per_x; j++)
				{
					SDL_Rect rect;
		
					rect.x = j * tile->w;
					rect.y = i * tile->h;
					rect.w = tile->w;
					rect.h = tile->h;
			
					SDL_BlitSurface (tile, NULL, bg, &rect);
				}
			}

    }
}

time_t lastgpstime =0;


/*----------------------------------------------------------------------------*/
void MapView::onIdle(wxIdleEvent &) 
{

	if (gpsOn)
	{
				ozex_gps_update();

	time_t thistime = time(NULL);

		//if (!lastgpstime || thistime - lastgpstime >= 3 )
		{
			lastgpstime = thistime;


			printf("on position\n");
			double lat = ozex_gps_lat();
			double lon = ozex_gps_lon();
		
			render->center_set(lat, lon);
			Refresh(false);
		}
	}

    
    if (SDL_MUSTLOCK(screen)) {
        if (SDL_LockSurface(screen) < 0) {
            return;
        }
    }
    

/*
	SDL_Rect central = { screen->w / 2 - 1, 0, 2, screen->h / 2 - 4}; 
	SDL_FillRect(screen, &central, 0x0000FF00);
	
	central.x = screen->w / 2 - 2;
	central.y = screen->h / 2 + 4;
	central.w = 2;
	central.h = screen->h / 2 - 4;
	SDL_FillRect(screen, &central, 0x0000FF00);
*/

        
    if (SDL_MUSTLOCK(screen)) 
        SDL_UnlockSurface(screen);

    
//    Refresh(false);
    
	//wxMilliSleep(1000);
}



/*----------------------------------------------------------------------------*/
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

		
public:
    MapWindow();
    
    MapView *getPanel();
	void updateButtons();
};


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

	if (firstItem + items < waypoints->points_avail())
	{
		firstItem += items;
		ptlist->SetFirstItem(firstItem);
	}
}


/*----------------------------------------------------------------------------*/
void MapWindow::ShowToolButtons()
{
	btnPointsList->Show();
	btnInfo->Show();
	btnNight->Show();
	btnMapOnOff->Show();

	if (waypoints->points_avail())
		btnPointsList->Enable();
	else
		btnPointsList->Disable();
}

/*----------------------------------------------------------------------------*/
void MapWindow::ShowMapButtons()
{
	btnZoomIn->Show();
	btnZoomOut->Show();
	btnMapPrev->Show();
	btnMapNext->Show();
	btnTools->Show();
	btnLoadPoints->Show();
	btnAddPoint->Show();
	btnQuit->Show();
	updateButtons();
}


/*----------------------------------------------------------------------------*/
void MapWindow::HideToolButtons()
{
	btnPointsList->Hide();
	btnInfo->Hide();
	btnNight->Hide();
	btnMapOnOff->Hide();
}

/*----------------------------------------------------------------------------*/
void MapWindow::HideMapButtons()
{
	btnZoomIn->Hide();
	btnZoomOut->Hide();
	btnMapPrev->Hide();
	btnMapNext->Hide();
	btnTools->Hide();
	btnLoadPoints->Hide();
	btnAddPoint->Hide();
	btnQuit->Hide();
}

/*----------------------------------------------------------------------------*/
void MapWindow::ShowPtListButtons()
{
	btnPageUp->Show();
	btnPageDn->Show();
	btnTarget->Show();
}

/*----------------------------------------------------------------------------*/
void MapWindow::HidePtListButtons()
{
	btnPageUp->Hide();
	btnPageDn->Hide();
	btnTarget->Hide();
}


#if 0
/*----------------------------------------------------------------------------*/
void MapWindow::ShowTools()
{
	btnZoomIn->Hide();
	btnZoomOut->Hide();
	btnMapPrev->Hide();
	btnMapNext->Hide();
	btnTools->Hide();
	btnLoadPoints->Hide();
	btnAddPoint->Hide();
	btnQuit->Hide();
	
	btnPointsList->Show();
	btnInfo->Show();
	btnNight->Show();
	btnTarget->Show();
	
	
	if (waypoints->points_avail())
		btnPointsList->Enable();
	else
		btnPointsList->Disable();

}

/*----------------------------------------------------------------------------*/
void MapWindow::HideTools()
{
	btnZoomIn->Show();
	btnZoomOut->Show();
	btnMapPrev->Show();
	btnMapNext->Show();
	btnTools->Show();
	btnLoadPoints->Show();
	btnAddPoint->Show();
	btnQuit->Show();
	
	btnPointsList->Hide();
	btnInfo->Hide();
	btnNight->Hide();
	btnTarget->Hide();
	
	updateButtons();
}
#endif

/*----------------------------------------------------------------------------*/
void MapWindow::onIdle(wxIdleEvent &)
{
	updateButtons();
}

/*----------------------------------------------------------------------------*/
void MapWindow::updateButtons()
{
//	SetCursor(wxCursor(NULL));
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
	
	mapOn ^= 1;

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
	ptlist->SetSize(0, 0, ozexMapDX, ozexMapDY, wxSIZE_FORCE);
	ptlist->SetWindowStyle(ptlist->GetWindowStyle() & ~wxHSCROLL);
	
	int j = 0;
	
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

			wxString point;

			if (dist < 1000)
			{
				point.Printf(wxT(" %.0f m"), dist);
			}
			else
			if (dist < 1000 * 10)
			{
				point.Printf(wxT(" %2.1f km"), 
								dist/1000);
			}
			else
			{
				point.Printf(wxT(" %.0f km"), 
								dist/1000);
			}

			wxname += point;

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
	
	int n = waypoints->points_avail();
	double nearest = 1000000 * 1000; // 1 mln km 
	int k = 0;

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
	double zoom = render->zoom_get();
	
	render->center_get(&lat, &lon);

	wxString status;
	status.Printf(wxT("Position:\n %f %f\n\n"), lat, lon);

	wxString point;
	waypoints->point_coords(k, &lat, &lon);

	if (n)
	{
		if (nearest < 1000)
		{
			point.Printf(wxT("Nearest waypoint:\n %.0f metres, "), 
							nearest);
			point += wxString::FromAscii(waypoints->point_name(k));

		}
		else
		if (nearest < 1000 * 10)
		{
			point.Printf(wxT("Nearest waypoint: %2.1f km, "), 
							nearest/1000);
			point += wxString::FromAscii(waypoints->point_name(k));
		}
		else
		{
			point.Printf(wxT("Nearest waypoint: %.0f km, "), 
							nearest/1000);
			point += wxString::FromAscii(waypoints->point_name(k));
		}

		status += point;
	}

    wxMessageBox(status,
                 wxT("Connection Status"), wxOK | wxICON_INFORMATION);

}

/*----------------------------------------------------------------------------*/
void MapWindow::onNight(wxCommandEvent &)
{
	HideToolButtons();
	ShowMapButtons();
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


/*----------------------------------------------------------------------------*/
void MapWindow::onLoadPoints(wxCommandEvent &)
{
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

	Close(); 
}

/*----------------------------------------------------------------------------*/
inline MapView* MapWindow::getPanel() 
{ 
	return panel; 
}

/*----------------------------------------------------------------------------*/
inline void MapWindow::onAddPoint(wxCommandEvent &) 
{
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
	
	render->center_get(&lat, &lon);
	
	waypoints->point_add(tmp, lat, lon);

	Refresh(false);
}

/*----------------------------------------------------------------------------*/
inline void MapWindow::onZoomIn(wxCommandEvent &) 
{
	render->zoom_in();
	Refresh(false);
}

/*----------------------------------------------------------------------------*/
inline void MapWindow::onZoomOut(wxCommandEvent &)
{
	render->zoom_out();
	Refresh(false);
}

/*----------------------------------------------------------------------------*/
inline void MapWindow::onNextMap(wxCommandEvent &)
{
	render->map_next();
	updateButtons();
	Refresh(false);
}

/*----------------------------------------------------------------------------*/
inline void MapWindow::onPrevMap(wxCommandEvent &)
{
	render->map_prev();
	updateButtons();
	Refresh(false);
}

/*----------------------------------------------------------------------------*/
inline void MapWindow::onGpsOnOff(wxCommandEvent &)
{
	if (gpsOn)
	{
		ozex_gps_close();
		gpsOn = 0;
	}
	else
	{
		int debug = atoi(getenv("OZEX_USE_LOG"));
		
		if (debug)
		{
			ozex_gps_start_fromfile(getenv("OZEX_GPS_LOG"));
		}
		else
		{
			ozex_gps_start(getenv("OZEX_GPS_LOG"), atoi(getenv("OZEX_GPS_BAUDS")));
		}
	
		gpsOn = 1;
	}
}

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
    Create(0, IDF_FRAME, wxT("Ozex"), wxDefaultPosition,
           wxSize(ozexWindowDX, ozexWindowDY), 0);

	ptlist = 0;
    panel = new MapView(this);
	panel->SetSize(0, 0, ozexMapDX, ozexMapDY, wxSIZE_FORCE);
	panel->Show();

	int y = 0, x = 0;
	
	wxImage::AddHandler(new wxPNGHandler);
	
	/*
	btnGpsOnOff = new wxBitmapButton(this, IDP_GPSONOFF, 
		wxBitmap(wxImage(wxT("icons/ic_menu_mylocation.png"), wxBITMAP_TYPE_PNG)), 
		wxPoint(ozexWindowDX - ozexButtonDX, y), wxSize(ozexButtonDX,ozexButtonDY), 0);
		
	y += ozexButtonDY + ozexButtonGap;		
	*/

// tool buttons	
	
	btnPointsList = new wxBitmapButton(this, IDP_POINTSLIST, 
		wxBitmap(wxImage(wxT("icons/ic_menu_toc.png"), wxBITMAP_TYPE_PNG)), 
		wxPoint(ozexWindowDX - ozexButtonDX, y), wxSize(ozexButtonDX,ozexButtonDY), 0);
	y += ozexButtonDX + ozexButtonGap;
	btnPointsList->Hide();

	btnInfo = new wxBitmapButton(this, IDP_INFO, 
		wxBitmap(wxImage(wxT("icons/ic_menu_info_details.png"), wxBITMAP_TYPE_PNG)), 
		wxPoint(ozexWindowDX - ozexButtonDX, y), wxSize(ozexButtonDX,ozexButtonDY), 0);
	y += ozexButtonDX + ozexButtonGap;
	btnInfo->Hide();

	btnNight = new wxBitmapButton(this, IDP_NIGHT, 
		wxBitmap(wxImage(wxT("icons/ic_menu_night.png"), wxBITMAP_TYPE_PNG)), 
		wxPoint(ozexWindowDX - ozexButtonDX, y), wxSize(ozexButtonDX,ozexButtonDY), 0);
	y += ozexButtonDX + ozexButtonGap;
	btnInfo->Hide();

	btnMapOnOff = new wxBitmapButton(this, IDP_MAPONOFF, 
		wxBitmap(wxImage(wxT("icons/ic_menu_report_image.png"), wxBITMAP_TYPE_PNG)), 
		wxPoint(ozexWindowDX - ozexButtonDX, y), wxSize(ozexButtonDX,ozexButtonDY), 0);
	y += ozexButtonDX + ozexButtonGap;
	btnInfo->Hide();


	Connect( IDP_POINTSLIST, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onPointList );
	Connect( IDP_INFO, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onInfo );
	Connect( IDP_NIGHT, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onNight );
	Connect( IDP_MAPONOFF, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onMapOnOff );

// point list buttons
	y = 0;

	btnPageUp = new wxBitmapButton(this, IDP_PAGEUP, 
		wxBitmap(wxImage(wxT("icons/ic_menu_pageup.png"), wxBITMAP_TYPE_PNG)), 
		wxPoint(ozexWindowDX - ozexButtonDX, y), wxSize(ozexButtonDX,ozexButtonDY), 0);
	y += ozexButtonDX + ozexButtonGap;
	btnPageUp->Hide();

	btnPageDn = new wxBitmapButton(this, IDP_PAGEDOWN, 
		wxBitmap(wxImage(wxT("icons/ic_menu_pagedown.png"), wxBITMAP_TYPE_PNG)), 
		wxPoint(ozexWindowDX - ozexButtonDX, y), wxSize(ozexButtonDX,ozexButtonDY), 0);
	y += ozexButtonDX + ozexButtonGap;
	btnPageDn->Hide();

	btnTarget = new wxBitmapButton(this, IDP_TARGET, 
		wxBitmap(wxImage(wxT("icons/ic_menu_goto.png"), wxBITMAP_TYPE_PNG)), 
		wxPoint(ozexWindowDX - ozexButtonDX, y), wxSize(ozexButtonDX,ozexButtonDY), 0);
	y += ozexButtonDX + ozexButtonGap;
	btnTarget->Hide();
	
	Connect( IDP_TARGET, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onTarget );
	Connect( IDP_PAGEUP, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onPageUp );
	Connect( IDP_PAGEDOWN, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onPageDn );

	
	y = 0;
// map buttons
	
	btnZoomIn = new wxBitmapButton(this, IDP_ZOOMIN, 
		wxBitmap(wxImage(wxT("icons/ic_menu_zoom.png"), wxBITMAP_TYPE_PNG)), 
		wxPoint(ozexWindowDX - ozexButtonDX, y), wxSize(ozexButtonDX,ozexButtonDY), 0);
		
	y += ozexButtonDY + ozexButtonGap;		
	btnZoomOut = new wxBitmapButton(this, IDP_ZOOMOUT, 
		wxBitmap(wxImage(wxT("icons/ic_menu_zoom_out.png"), wxBITMAP_TYPE_PNG)), 
		wxPoint(ozexWindowDX - ozexButtonDX, y), wxSize(ozexButtonDX,ozexButtonDY), 0);		

	y += ozexButtonDY + ozexButtonGap;		
	btnMapPrev = new wxBitmapButton(this, IDP_PREVMAP, 
		wxBitmap(wxImage(wxT("icons/ic_menu_less.png"), wxBITMAP_TYPE_PNG)), 
		wxPoint(ozexWindowDX - ozexButtonDX, y), wxSize(ozexButtonDX,ozexButtonDY), 0);		

	y += ozexButtonDY + ozexButtonGap;		
	btnMapNext = new wxBitmapButton(this, IDP_NEXTMAP, 
		wxBitmap(wxImage(wxT("icons/ic_menu_more.png"), wxBITMAP_TYPE_PNG)), 
		wxPoint(ozexWindowDX - ozexButtonDX, y), wxSize(ozexButtonDX,ozexButtonDY), 0);		

	y += ozexButtonDY + ozexButtonGap;		
	btnLoadPoints = new wxBitmapButton(this, IDP_LOADPOINTS, 
		wxBitmap(wxImage(wxT("icons/ic_menu_archive.png"), wxBITMAP_TYPE_PNG)), 
		wxPoint(ozexWindowDX - ozexButtonDX, y), wxSize(ozexButtonDX,ozexButtonDY), 0);		

		
	y += ozexButtonDY + ozexButtonGap;		
	btnAddPoint = new wxBitmapButton(this, IDP_ADDPOINT, 
		wxBitmap(wxImage(wxT("icons/ic_menu_myplaces.png"), wxBITMAP_TYPE_PNG)), 
		wxPoint(ozexWindowDX - ozexButtonDX, y), wxSize(ozexButtonDX,ozexButtonDY), 0);		

	y += ozexButtonDY + ozexButtonGap;		
	btnTools = new wxBitmapButton(this, IDP_TOOLS, 
		wxBitmap(wxImage(wxT("icons/ic_menu_manage.png"), wxBITMAP_TYPE_PNG)), 
		wxPoint(ozexWindowDX - ozexButtonDX, y), wxSize(ozexButtonDX,ozexButtonDY), 0);		
		
	y += ozexButtonDY + ozexButtonGap;		
	btnQuit = new wxBitmapButton(this, wxID_EXIT, 
		wxBitmap(wxImage(wxT("icons/ic_menu_close_clear_cancel.png"), wxBITMAP_TYPE_PNG)), 
		wxPoint(ozexWindowDX - ozexButtonDX, y), wxSize(ozexButtonDX,ozexButtonDY), 0);		
		
		

	Connect( IDP_GPSONOFF, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onGpsOnOff );
	Connect( IDP_ZOOMIN, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onZoomIn );
	Connect( IDP_ZOOMOUT, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onZoomOut );
	Connect( IDP_NEXTMAP, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onNextMap );
	Connect( IDP_PREVMAP, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onPrevMap );
	Connect( IDP_LOADPOINTS, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onLoadPoints );
	Connect( IDP_ADDPOINT, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onAddPoint );
	Connect( IDP_TOOLS, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onTools );
	Connect( wxID_EXIT, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction) &MapWindow::onFileExit );
	
	updateButtons();
}

/*----------------------------------------------------------------------------*/
void MapWindow::onHelpAbout(wxCommandEvent &) 
{
    wxMessageBox(wxT("Ozex\nCopyright (C) 2005,2010 Daniil Smelov"),
                 wxT("About Ozex"), wxOK | wxICON_INFORMATION);
}


/*----------------------------------------------------------------------------*/
class SDLApp : public wxApp 
{
    DECLARE_CLASS(SDLApp)
    
private:
    MapWindow *frame;

    
public:
    bool OnInit();
    
    int OnRun();
    
    int OnExit();
};

/*----------------------------------------------------------------------------*/
bool SDLApp::OnInit() 
{

	waypoints = new wpt_container();

	ozexWindowDX = atoi(getenv("OZEX_WINDOW_DX"));
	ozexWindowDY = atoi(getenv("OZEX_WINDOW_DY"));

	ozexButtonDX = atoi(getenv("OZEX_BUTTON_DX"));
	ozexButtonDY = atoi(getenv("OZEX_BUTTON_DY"));
	ozexButtonGap = atoi(getenv("OZEX_BUTTON_GAP"));

	ozexMapDX = atoi(getenv("OZEX_MAP_DX"));
	ozexMapDY = atoi(getenv("OZEX_MAP_DY"));

	double lat = atof(getenv("OZEX_LAT"));
        double lon = atof(getenv("OZEX_LON"));
	
	printf("ozexWindowDX: %d\n", ozexWindowDX);
	printf("ozexWindowDY: %d\n", ozexWindowDY);

	printf("ozexButtonDX: %d\n", ozexButtonDX);
	printf("ozexButtonDY: %d\n", ozexButtonDY);
	printf("ozexButtonGap: %d\n", ozexButtonGap);

	printf("ozexMapDX: %d\n", ozexMapDX);
	printf("ozexMapDY: %d\n", ozexMapDY);

	printf("ozexX: %lf\n", lat);
	printf("ozexY: %lf\n", lon);

	logstream_to(NULL);

	mapdatums_init("./datums.xml");
	collection = new map_index(getenv("OZEX_MAPS_PATH"));
	
    frame = new MapWindow;
    frame->SetSize(wxSize(ozexWindowDX, ozexWindowDY));

	render = new map_render(collection, ozexMapDX, ozexMapDY);
    
	render->center_set(lat, lon);
        
    frame->Show();
    
    //SetTopWindow(frame);
    
    return true;
}

/*----------------------------------------------------------------------------*/
int SDLApp::OnRun() 
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
        std::cerr << "unable to init SDL: " << SDL_GetError() << '\n';
        
        return -1;
    }
    
    wxPaintEvent event;
    event.SetEventObject(frame->getPanel());
    frame->getPanel()->AddPendingEvent(event);


    return wxApp::OnRun();
}

/*----------------------------------------------------------------------------*/
int SDLApp::OnExit() 
{
	if (gpsOn)
	{
		ozex_gps_close();
		gpsOn = 0;
	}


	delete waypoints;
	delete render;
	delete collection;
	mapdatums_done();

	printf("SDLApp::OnExit()\n");
    SDL_Quit();
    
    return wxApp::OnExit();
}

IMPLEMENT_CLASS(SDLApp, wxApp)
IMPLEMENT_APP(SDLApp)

