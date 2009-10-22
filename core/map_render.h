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

#ifndef __MAP_RENDER_INCLUDED
#define __MAP_RENDER_INCLUDED

#include "map_loader.h"
#include "map_container.h"
#include "map_index.h"

#include <SDL.h>

/*--------------------------------------------------------------------------*/
class map_render
{
	public:
		map_render(map_index* collection, int width, int height);
		~map_render();

		int				center_set(double lat, double lon);
		int				center_get(double* lat, double* lon);
		
		void			zoom_in();
		void			zoom_out();
		
		int				zoom_in_avail();
		int				zoom_out_avail();
		

		void			map_next();
		void			map_prev();

		int				has_map_next();
		int				has_map_prev();

		
		SDL_Surface*	surface();
		void			rect(SDL_Rect* r);
		
		void			scroll_by(int dx, int dy);
		
		double			zoom_get();

		char*			file_name_get();

		map_container*	getmap();

		void			render();
		
		void			inverse();
		
	private:
		void			clear();


	private:
	
		SDL_Surface*	view;
		SDL_Rect		area;
		
		map_index*		maps_collection;

		int				inversed;
		
		struct 
		{
			double			lat, lon;
			int*			maps_indexes;
			int				maps_amount;	
			int				map_current;
			map_container*	mapview;
		} center;

};


#endif

