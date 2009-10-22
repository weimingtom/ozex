/**
 * swampex, a map processing library
 *
 * Authors: 
 *
 * Daniil Smelov <dn.smelov@gmail.com>
 *
 * Copyright (C) 2006-2009 Daniil Smelov, Slava Baryshnikov
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */


#include "map_render.h"
#include "SDL_gfxPrimitives.h"
#include "log_stream.h"

/*--------------------------------------------------------------------------*/
map_render::map_render(map_index* collection, int width, int height)
{
	view = NULL;
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

	view = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32,
								rmask, gmask, bmask, amask);
								
	maps_collection = collection;
	
	
	center.mapview		= NULL;
	center.maps_indexes	= NULL;
	center.maps_amount	= 0;	
	center.map_current	= 0;
	
	inversed = 0;
}

/*--------------------------------------------------------------------------*/
map_render::~map_render()
{
	if (view)
		SDL_FreeSurface(view);
		
	if (center.mapview)
		delete center.mapview;
		
	if (center.maps_indexes)
		free(center.maps_indexes);
}


/*--------------------------------------------------------------------------*/
void map_render::inverse()
{
	inversed ^= 1;
}

/*--------------------------------------------------------------------------*/
map_container* map_render::getmap()
{
	return center.mapview;
}

/*--------------------------------------------------------------------------*/
int map_render::center_set(double lat, double lon)
{
	int changemap = 1;
	
	if (center.mapview)
	{
		if (center.mapview->ll_inside_map(lat, lon))
		{
			changemap = 0;
			
			int id_current = center.maps_indexes[center.map_current];
			
			free(center.maps_indexes);
			center.map_current = 0;
			
			center.maps_amount = maps_collection->maps_for_ll(lat, lon, &center.maps_indexes);
			
			for (int i = 0; i < center.maps_amount; i++)
			{
				if (center.maps_indexes[i] == id_current)
				{
					center.map_current = i;
					break;
				}
			}
		}
	}


	if (changemap)
	{
		double	mpp_previous	= 0;
		double	zoom_previous	= 1.0;
		int		have_recent_map	= 0;
		

		int*	indexes = NULL;

		int		maps_available = 
			maps_collection->maps_for_ll(lat, lon, &indexes);

		if (maps_available != 0)
		{	
			if (center.mapview)
			{
				mpp_previous = center.mapview->mpp();
				zoom_previous = center.mapview->zoom_level_get();
				have_recent_map = 1;
			}
		
			if (center.maps_indexes)
				free(center.maps_indexes);

			if (center.mapview)
			{
				delete center.mapview;
				center.mapview = NULL;
			}
	
			center.map_current = 0;
				
			//center.maps_amount = maps_collection->maps_for_ll(lat, lon, &center.maps_indexes);
			center.maps_amount = maps_available;
			center.maps_indexes = indexes;

			if (center.maps_amount)
			{
				int mpp_delta = INT_MAX;

				int id = 0;
			
				if (have_recent_map)
				{
					for ( int i = 0; i < center.maps_amount; i++ )
					{
						double mpp = maps_collection->mpp(center.maps_indexes[i]);

						double delta = mpp - mpp_previous;

						if ( delta < 0 )
							delta = -delta;

						if ( delta < mpp_delta)
						{
							id = i;
							mpp_delta = delta;
						}
					}
				}

				double mpp_visible = mpp_previous * zoom_previous;
			
				center.mapview = 
					new map_container(maps_collection->map_filename(center.maps_indexes[id]));
				
				center.map_current = id;

				if (have_recent_map)
				{				
					double zoom_required = mpp_visible/center.mapview->mpp();
			
					center.mapview->zoom_level_set(zoom_required);
				}
			}
		}
		else
		{
			if (indexes)
				free(indexes);

			if (center.mapview)
			{
				int map_index = 
					center.maps_indexes[center.map_current];
				
				center.map_current = 0;
				free(center.maps_indexes);
				center.maps_indexes = (int*)malloc(sizeof(int));
				center.maps_indexes[0] = map_index;
				center.maps_amount = 1;
			}
		}

	}
	
	center.lat = lat;
	center.lon = lon;

	render();	

	return 0;
}

/*--------------------------------------------------------------------------*/
int	map_render::center_get(double* lat, double* lon)
{
	*lat = center.lat;
	*lon = center.lon;

	return 0;
}

/*--------------------------------------------------------------------------*/
int map_render::zoom_in_avail()
{
	if (center.mapview)
	{
		return
		center.mapview->zoom_in_avail();
	}
	
	return 0;
}

/*--------------------------------------------------------------------------*/
int map_render::zoom_out_avail()
{
	if (center.mapview)
	{
		return 
		center.mapview->zoom_out_avail();
	}
	
	return 0;
}



/*--------------------------------------------------------------------------*/
void map_render::zoom_in()
{
	if (center.mapview)
	{
		center.mapview->zoom_in();
		render();
	}
}

/*--------------------------------------------------------------------------*/
void map_render::zoom_out()
{
	if (center.mapview)
	{
		center.mapview->zoom_out();
		render();
	}
}

/*--------------------------------------------------------------------------*/
int map_render::has_map_next()
{
	if (center.maps_amount <= 1)
		return 0;

	if (center.map_current < center.maps_amount - 1 )
		return 1;	
		
	return 0;
}

/*--------------------------------------------------------------------------*/
int  map_render::has_map_prev()
{
	if (center.maps_amount <= 1)
		return 0;

	if (center.map_current )
		return 1;	
		
	return 0;
}


/*--------------------------------------------------------------------------*/
void map_render::map_next()
{
	if (center.map_current < center.maps_amount - 1 )
	{
		center.map_current++;

		if (center.mapview)
			delete center.mapview;
		
		int i = center.maps_indexes[center.map_current];
		
		center.mapview = 
			new map_container(maps_collection->map_filename(i));
			
		render();
	}
}

/*--------------------------------------------------------------------------*/
void map_render::map_prev()
{
	if (center.map_current > 0 )
	{
		center.map_current--;

		if (center.mapview)
			delete center.mapview;
		
		int i = center.maps_indexes[center.map_current];
		
		center.mapview = 
			new map_container(maps_collection->map_filename(i));
			
		render();
	}
}

/*--------------------------------------------------------------------------*/
double	map_render::zoom_get()
{
	if (center.mapview)
		return center.mapview->zoom_level_get();

	return 0;
}

/*--------------------------------------------------------------------------*/
SDL_Surface* map_render::surface()
{
	return view;
}


/*--------------------------------------------------------------------------*/
void map_render::rect(SDL_Rect* r)
{
	memcpy(r, &area, sizeof(SDL_Rect));
}

/*--------------------------------------------------------------------------*/
char* map_render::file_name_get()
{
	if (center.mapview)
	{
		return 
		center.mapview->file_name_get();
	}
	
	return NULL;
}

/*--------------------------------------------------------------------------*/
void map_render::scroll_by(int dx, int dy)
{
	if (center.mapview)
	{
		long map_x, map_y;
		double lat, lon;
		
		center.mapview->ll_to_xy_onmap(center.lat, center.lon, &map_x, &map_y);

		if (center.mapview->xy_onmap_to_ll(map_x + dx, map_y + dy, &lat, &lon))
			center_set(lat, lon);
	}
}

typedef struct {
  unsigned char b, g, r, a;
} pixel;

void
grayscale (SDL_Surface *surface)
{
  // aquire pointer to pixel data
  // origin is in top-left corner, and data is organized row/scanline-major
  pixel *pixels = (pixel*) surface->pixels;

  // remove all red from left half of the image
  int x;
  int y;
  for (x = 0; x < surface->w; x++) {
    for (y = 0; y < surface->h; y++) {
      int pos = x + y*(surface->w);
      int grayvalue = (pixels[pos].r + pixels[pos].b + pixels[pos].g) / 3;

      // Save the gray value to all channels
      /*
      pixels[pos].r = 0; //grayvalue;
      pixels[pos].b = 0; //grayvalue;
      pixels[pos].g = grayvalue/2;
      */
      pixels[pos].r /= 2;
      pixels[pos].b /= 2;
      pixels[pos].g /= 2;
    }
  }
}

/*--------------------------------------------------------------------------*/
void map_render::render()
{
	int min_x = INT_MAX;
	int min_y = INT_MAX;
	int max_x = INT_MIN;
	int max_y = INT_MIN;

	SDL_FillRect(view, NULL, 0xFFFFFFFF);

	if(SDL_MUSTLOCK(view))
		SDL_LockSurface(view);
	
	if (center.mapview)
	{
		long c, r;
		long x, y;
		SDL_Rect rect;

		center.mapview->ll_to_cr(center.lat, center.lon, &c, &r);
		
		center.mapview->ll_to_xy_on_tile(center.lat, center.lon, &x, &y);

		int tile_w = center.mapview->tile_dx();
		int tile_h = center.mapview->tile_dx();

		int tiles_per_x = view->w/tile_w;
		int tiles_per_y = view->h/tile_h;

		long c_min = c - tiles_per_x/2 - 2;
		long c_max = c + tiles_per_x/2 + 2;
		
		long r_min = r - tiles_per_y/2 - 2;
		long r_max = r + tiles_per_y/2 + 2;
				
		if(c_min < 0) c_min = 0;
		if(r_min < 0) r_min = 0;
		
		if (c_max > center.mapview->tiles_per_x())
			c_max = center.mapview->tiles_per_x();

		if (r_max > center.mapview->tiles_per_y())
			r_max = center.mapview->tiles_per_y();
								
		for (int i = r_min; i < r_max; i++)
		{
			for (int j = c_min; j < c_max; j++)
			{
				rect.x = view->w/2 - x - (c - c_min) * tile_w;
				rect.y = view->h/2 - y - (r - r_min) * tile_h;
				rect.w = tile_w;
				rect.h = tile_h;

				rect.x += (j - c_min) * tile_w;
				rect.y += (i - r_min) * tile_h;
			
			
				SDL_Surface* tile = center.mapview->tile_get(j, i);

				if (tile)
				{
					if (inversed)
					{
						grayscale(tile);
					}
				
					//logstream_write("blitting tile %d %d to: %d %d\n", tile->w, tile->h, rect.x, rect.y);

					if (min_x > rect.x)
						min_x = rect.x;

					if (min_y > rect.y)
						min_y = rect.y;

					if (max_x < rect.x + tile->w)
						max_x = rect.x + tile->w;

					if (max_y < rect.y + tile->h)
						max_y = rect.y + tile->h;
				
					SDL_BlitSurface (tile, NULL, view, &rect);
					
					center.mapview->tile_forget(tile);
				}
			}
		}

	}

//	logstream_write("rendered to (a): %d %d %d %d\n", min_x, min_y, max_x, max_y);

	if (min_x < 0)
		min_x = 0;
	
	if (min_y < 0)
		min_y = 0;

	int w = max_x - min_x;
	int h = max_y - min_y;

//	logstream_write("rendered to (b): %d %d\n", w, h);
		
	if (min_x + w > view->w)
		w = view->w - min_x;

	if (min_y + h > view->h)
		h = view->h - min_y;

//	logstream_write("rendered to (c): %d %d %d %d\n", min_x, min_y, w, h);
	
	area.x = min_x;
	area.y = min_y;
	area.w = w;
	area.h = h;

//	rectangleRGBA(view, min_x, min_y, max_x-1, max_y-1, 0, 255, 0, 255);
		
	if(SDL_MUSTLOCK(view))
		SDL_UnlockSurface(view);
}



