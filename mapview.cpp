#include "SDL_image.h"

#include <wx/wxprec.h>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include "ll_geometry.h"
#include "map_container.h"
#include "wpt_container.h"

#include "mapview.h"
#include "ozex.h"
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

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
Uint32			rmask = 0xff000000;
Uint32			gmask = 0x00ff0000;
Uint32			bmask = 0x0000ff00;
Uint32			amask = 0x000000ff;
#else
Uint32			rmask = 0x000000ff;
Uint32			gmask = 0x0000ff00;
Uint32			bmask = 0x00ff0000;
Uint32			amask = 0xff000000;
#endif


/*----------------------------------------------------------------------------*/
MapView::MapView(wxFrame *parent) : wxPanel(parent, IDP_PANEL), screen(0), mapOn(1) 
{
	delta_x = delta_y = 0;
	pressed = 0;

	wxImage::AddHandler(new wxPNGHandler);

	position = new wxBitmap(wxImage(wxT("icons/cursor_standing.png"), wxBITMAP_TYPE_PNG));
	radioOn = new wxBitmap(wxImage(wxT("icons/btn_radio_on.png"), wxBITMAP_TYPE_PNG));
	radioOff = new wxBitmap(wxImage(wxT("icons/btn_radio_off.png"), wxBITMAP_TYPE_PNG));
	point = new wxBitmap(wxImage(wxT("icons/map_point_red.png"), wxBITMAP_TYPE_PNG));
	pointnew = new wxBitmap(wxImage(wxT("icons/map_point_green.png"), wxBITMAP_TYPE_PNG));

	char *t;
	ozexMapDX = GET_ENV_INT("OZEX_MAP_DX",732);
	ozexMapDY = GET_ENV_INT("OZEX_MAP_DY",542);
	

	printf("ozexMapDX: %d\n", ozexMapDX);
	printf("ozexMapDY: %d\n", ozexMapDY);
	collection = new map_index(GET_ENV_STR("OZEX_MAPS_PATH","data/maps"));
	render = new map_render(collection, ozexMapDX, ozexMapDY);
	double lat = GET_ENV_FLOAT("OZEX_LAT",60.382313);
        double lon = GET_ENV_FLOAT("OZEX_LON",29.562426);
	render->center_set(lat, lon);
        printf("ozexX: %lf\n", lat);
	printf("ozexY: %lf\n", lon);
	SetSize(0, 0, ozexMapDX, ozexMapDY, wxSIZE_FORCE);
}

/*----------------------------------------------------------------------------*/
int MapView::isPressed()
{
	return pressed;
}

/*----------------------------------------------------------------------------*/
int MapView::isMapOn()
{
	return mapOn;
}

/*----------------------------------------------------------------------------*/
void MapView::mapOnOff()
{
	mapOn ^= 1;
}

wxSize MapView::getMapSize()
{
	return wxSize(ozexMapDX, ozexMapDY);
}

map_render* MapView::getMapRender()
{
	return render;
}
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
MapView::~MapView() 
{
	delete collection;
	delete render;
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

	DrawPosition(dc);

	DrawWaypoints(dc);

	dc.DrawBitmap(wxGetApp().isGpsOn() ? *radioOn : *radioOff, 0, 0, true);

	if (SDL_MUSTLOCK(screen)) 
		SDL_UnlockSurface(screen);

}

/*----------------------------------------------------------------------------*/
void MapView::DrawPosition(wxPaintDC &dc)
{
//	dc.DrawBitmap(*position, screen->w/2 - position->GetWidth()/2, screen->h/2  - position->GetHeight()/2, true);
	wxPen backup = dc.GetPen();

	wxPen pen;

	pen.SetColour(wxColour(255, 0, 0));
	pen.SetWidth(1);
	dc.SetPen(pen);

	int w = screen->w;
	int h = screen->h;
	int xC = w/2;
	int yC = h/2;

	dc.DrawLine(xC, 0, xC, h);
	dc.DrawLine(0, yC, w, yC);

	pen.SetColour(wxColour(200, 0, 0));
	pen.SetWidth(1);
	dc.SetPen(pen);

	dc.DrawLine(xC - 1, 0, xC - 1, h);
	dc.DrawLine(0, yC - 1, w, yC - 1);
	dc.DrawLine(xC + 1, 0, xC + 1, h);
	dc.DrawLine(0, yC + 1, w, yC + 1);

	dc.SetPen(backup);

}

/*----------------------------------------------------------------------------*/
void MapView::DrawWaypoints(wxPaintDC &dc)
{
	map_container* mapview = render->getmap();
	if (mapview && (!pressed || (pressed && delta_x == 0 && delta_y == 0)))
	{
	
		double nearest = 1000000 * 1000; // 1 mln km 
		int k = 0;
		double center_lat, center_lon;
		render->center_get(&center_lat, &center_lon);

                wpt_container* waypoints = wxGetApp().getWaypoints();
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
			
			DrawWpt(dc, i, center_lat, center_lon);
			UpdateNearestWpt(dc, k, n, nearest);		
			
		}
		
	}
}

///////////////////////////////////////////////////////////////////////////////
void MapView::DrawWpt(wxPaintDC &dc, int i, double center_lat, double center_lon) {
        wpt_container* waypoints = wxGetApp().getWaypoints();
	if ( waypoints->point_active ( i ) )
	{
		map_container* mapview = render->getmap();
		double lat, lon;
			
		waypoints->point_coords(i, &lat, &lon);
	
		long point_x, point_y, center_x, center_y;
	
		mapview->ll_to_xy_onmap ( center_lat, center_lon, &center_x, &center_y );
		mapview->ll_to_xy_onmap ( lat, lon, &point_x, &point_y );
	
		int visible_x = point_x - center_x;
		int visible_y = point_y - center_y;
	
	
		int x = ozexMapDX/2 + visible_x - point->GetWidth() /2;
		int y = ozexMapDY/2 + visible_y - point->GetHeight();
	
		if ( x >= 0 && x <= ozexMapDX && y >= 0 && y <= ozexMapDY )
		{
			char* name = waypoints->point_name ( i );
	
			if ( name && name[0] )
			{
				wxString wxname = wxString::FromAscii ( ( const char* ) name );
	
				wxFont font = dc.GetFont();
				font.SetPointSize ( 9 );
				font.SetWeight ( wxFONTWEIGHT_BOLD );
				dc.SetFont ( font );
	
				wxColour f( 0, 0, 0 ), b( 255, 255, 255 );
	
				dc.SetTextBackground ( b );
				dc.SetTextForeground ( f );
	
				//dc.SetBackgroundMode(wxSOLID);
				wxCoord w, h;
				dc.GetTextExtent ( wxname, &w, &h );
	
				dc.DrawRectangle ( ozexMapDX/2 + visible_x - ( w+2 ) /2,
						ozexMapDY/2 + visible_y + 1, ( w+2 ), h );
				/*
				dc.DrawEllipse(ozexMapDX/2 + visible_x - w/2,
							ozexMapDY/2 + visible_y + 1, w, h);
				*/
				dc.DrawText ( wxname, ozexMapDX/2 + visible_x - w/2,
					ozexMapDY/2 + visible_y + 1 );
			}
	
			dc.DrawBitmap (waypoints->added ( i ) ? *pointnew : *point, x, y, true );
		}
	}
}

void MapView::UpdateNearestWpt(wxPaintDC &dc, int k, int n, double nearest)
{
        wpt_container* waypoints = wxGetApp().getWaypoints();
	wxString point;
	double lat, lon;			
	waypoints->point_coords(k, &lat, &lon);
	wxString status;

	if (n)
	{
		point.Printf(wxT("Nearest waypoint: %s, "), format_dist(nearest).c_str());
		point += wxString::FromAscii(waypoints->point_name(k));
		status += point;		
		
		wxFont font = dc.GetFont();
		font.SetPointSize(9);
		font.SetWeight(wxFONTWEIGHT_BOLD);
		dc.SetFont( font );
	
		wxColour f(0, 0, 0), b(255, 255, 255);

		dc.SetTextBackground(b);
		dc.SetTextForeground(f);
	
		wxCoord w, h;
		dc.GetTextExtent(status, &w, &h);

		dc.DrawRectangle(ozexMapDX/2 - (w+2)/2, 
						ozexMapDY - h - 1, (w+2), h + 1);

		dc.DrawText(status, ozexMapDX/2 - w/2, 
					ozexMapDY - h);			
				
				
	}
}

/*----------------------------------------------------------------------------*/
void  MapView::OnMouseUp( wxMouseEvent &event )
{
	if (wxGetApp().isGpsOn())
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
			EditWpt(FindWptByXyOnMap(x, y));
		}
		
		pressed = 0;
		delta_x = 0;
		delta_y = 0;
	}

    event.Skip();
  
}

int MapView::FindWptByXyOnMap(long x, long y)
{
	map_container* mapview = render->getmap();
	
	if (mapview)
	{
		double lat, lon;

                wpt_container* waypoints = wxGetApp().getWaypoints();
		int n = waypoints->points_avail();

		double center_lat, center_lon;
		long center_x, center_y;

		render->center_get(&center_lat, &center_lon);
		mapview->ll_to_xy_onmap(center_lat, center_lon, &center_x, &center_y);
		
		center_x += (x - ozexMapDX/2);
		center_y += (y - ozexMapDY/2);
		
		for (int i = 0; i < n; i++)
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
					return i;
				}

			}
		}
		
	}
	return -1;
}

void MapView::EditWpt(int i)
{
        wpt_container* waypoints = wxGetApp().getWaypoints();
	if (i < 0 || i >= waypoints->points_avail())
	{
		return;
	}
	char* name = waypoints->point_name(i);

	wxString wxname(wxT("Delete waypoint \""));
	wxname += wxString::FromAscii((const char*)name);
	wxname += wxString(wxT("\"?"));

	int answer = wxMessageBox(wxname,wxT("Confirm"), wxYES_NO | wxICON_QUESTION);

	if (answer == wxYES)
	{
		waypoints->point_delete(i);
		Refresh(false);
	}
}

/*----------------------------------------------------------------------------*/
void  MapView::OnMouseMove( wxMouseEvent &event )
{
	if (wxGetApp().isGpsOn())
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
		wxGetApp().gpsOnOff();
		event.Skip();
		Refresh(false);
		return;
	}
	else
	if (wxGetApp().isGpsOn())
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



/*----------------------------------------------------------------------------*/
void MapView::onIdle(wxIdleEvent &) 
{
	coords_t coords = wxGetApp().readGps();

	if (coords.onPosition)
	{
			printf("on position\n");
		
			render->center_set(coords.lat, coords.lon);
			Refresh(false);

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
