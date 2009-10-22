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

#ifndef __MAP_CONTAINER_INCLUDED
#define __MAP_CONTAINER_INCLUDED

/*--------------------------------------------------------------------------*/
#include "map_types.h"
#include "map_projection.h"
#include "map_loader.h"
#include "ozf_decoder.h"
#include "img_decoder.h"
#include "SDL.h"


/*--------------------------------------------------------------------------*/
typedef struct 
{
	double	zoom;
	int		source;
	double	factor;
} map_scale;

/*--------------------------------------------------------------------------*/
class	map_container
{
	public:
		map_container(char* file);
		~map_container();

		SDL_Surface*	tile_get(long c, long r, long inverse = 0);
		void		tile_forget(SDL_Surface* tile);

		void		ll_to_cr(double latitude, double longitude, long *c, long *r);
		void		ll_to_xy_onmap(double latitude, double longitude, long *x, long *y);
		long		xy_onmap_to_ll(long x, long y, double* latitude, double* longitude);
		void		ll_to_xy_on_tile(double latitude, double longitude, long *x, long *y);

		long		tile_dx(int c = 0, int r = 0);
		long		tile_dy(int c = 0, int r = 0);

		long		tiles_per_x();
		long		tiles_per_y();

		long		scales_amount();
		void		zoom_in();
		void		zoom_out();
		
		int			zoom_in_avail();
		int			zoom_out_avail();
		

		long		ll_inside_map(double latitude, double longitude);

		long		map_width();
		long		map_height();

		double		zoom_level_get();
		void		zoom_level_set(double factor);
		
		char*		file_name_get();
		double		mpp();


		long 		scale_up_possible();
		long 		scale_down_possible();

	private:
		void		build_zoom_table();
		void		map_info_update();
		void		projection_update();
		long 		cr_inside_map(long c, long r);

	private:
		char*			filename;

		map_info*		calibration;
		map_stream*		image;

		map_projection*	projection;

		long			zoom_current;
		map_scale*		zoom_table;
		
		struct
		{
				map_stream* (*open)(char* path);
				void		(*get_tile)(map_stream* s, int scale, int x, int y, unsigned char* data);
				int			(*num_scales)(map_stream* s);
				int			(*num_tiles_per_x)(map_stream* s, int scale);
				int			(*num_tiles_per_y)(map_stream* s, int scale);
				int			(*scale_dx)(map_stream* s, int scale);
				int			(*scale_dy)(map_stream* s, int scale);
				void		(*close)(map_stream*);
		} image_ops;
};

#endif

