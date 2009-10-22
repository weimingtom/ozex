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

#include "map_container.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef UNDER_CE
#include <sys/types.h>
#endif

#include "ul.h"
#include "log_stream.h"

#include "map_filters.h"
#include "SDL_rotozoom.h"
#include "SDL_gfxPrimitives.h"



double
mcround(double x)
{
        double t;

//        if (!isfinite(x))
//                return (x);

        if (x >= 0.0) {
                t = floor(x);
                if (t - x <= -0.5)
                        t += 1.0;
                return (t);
        } else {
                t = floor(-x);
                if (t + x <= -0.5)
                        t += 1.0;
                return (-t);
        }
}



/*--------------------------------------------------------------------------*/
static double zoom_levels_supported[] =
{
	0.005,
	0.010,
	0.020,
	0.050,
	0.060,
	0.070,
	0.080,
	0.090,
	0.100,
	0.250,
	0.500,
	0.750,
	1.000,
	1.250,
	1.500,
	1.750,
	2.000,
	2.500,
	3.000,
	4.000,
	5.000,
	7.500,
	10.0,
	20.0,
};

/*--------------------------------------------------------------------------*/
#define ZOOMS_SUPPORTED (sizeof(zoom_levels_supported)/sizeof(zoom_levels_supported[0]))

/*--------------------------------------------------------------------------*/
map_container::map_container(char* file)
{
	filename = (char*)malloc(strlen(file) + 1);
	strcpy(filename, file);
	
	zoom_current = 0;
	
	projection = NULL;
	calibration = maploader_openmap(filename);
	
	char imagefile[UL_NAME_MAX];

	logstream_write("map container: %s\n", calibration->image_path);
	logstream_write("map container: %s\n", calibration->image_file);
	
	strcpy(imagefile, calibration->image_path);
	strcat(imagefile, calibration->image_file);
	
	image_ops.open = img_open;
	image_ops.get_tile = img_get_tile;
	image_ops.num_scales = img_num_scales;
	image_ops.num_tiles_per_x = img_num_tiles_per_x;
	image_ops.num_tiles_per_y = img_num_tiles_per_y;
	image_ops.scale_dx = img_scale_dx;
	image_ops.scale_dy = img_scale_dy;
	image_ops.close = img_close;

	char* ext = imagefile + strlen(imagefile);
	
	do
	{
		ext--; 
		if (*ext == '.')
			break;
	} while (ext > imagefile);

	logstream_write("img: %s\n", imagefile);
	logstream_write("ext: %s\n", ext);

	if (!strcmp(ext, ".ozf") || !strcmp(ext, ".ozf2") ||
		!strcmp(ext, ".ozfx3"))
	{
		image_ops.open = ozf_open;
		image_ops.get_tile = ozf_get_tile;
		image_ops.num_scales = ozf_num_scales;
		image_ops.num_tiles_per_x = ozf_num_tiles_per_x;
		image_ops.num_tiles_per_y = ozf_num_tiles_per_y;
		image_ops.scale_dx = ozf_scale_dx;
		image_ops.scale_dy = ozf_scale_dy;
		image_ops.close = ozf_close;
	}
	
	
	image = image_ops.open(imagefile);
	
	build_zoom_table();

	zoom_level_set(1.0);

	map_info_update();
}

/*--------------------------------------------------------------------------*/
map_container::~map_container()
{
	maploader_closemap(calibration);
	image_ops.close(image);
	
	if (projection)
		delete projection;
		
	if (filename)
		free(filename);
		
	if (zoom_table)
		free(zoom_table);
}

/*--------------------------------------------------------------------------*/
void map_container::build_zoom_table()
{
	zoom_table = (map_scale*)malloc(ZOOMS_SUPPORTED * sizeof(map_scale));
	memset(zoom_table, 0, ZOOMS_SUPPORTED * sizeof(map_scale));

	int scales = image_ops.num_scales(image);

	logstream_write("mapinfo: %d %d\n", calibration->width, calibration->height);
	logstream_write("building scales table\n");
	
	for (int i = 0; i < ZOOMS_SUPPORTED; i++)
	{
		int k = 0;
		double delta = DBL_MAX;
		double ozf_zoom;
		
		for (int j = 0; j < scales; j++)
		{
			int dx, dy;
		
			dx = image_ops.scale_dx(image, j);
			dy = image_ops.scale_dy(image, j);
		
			double a = dy;
			double b = calibration->height;
		
			double percents = (a / b) * 100;
			
			//percents = mcround(percents);
			
			double zoom = percents / 100;

			// if current zoom is < 100% - we need to select 
			// nearest upper native zoom
			// otherwize we need to select
			// any nearest zoom

			if (zoom_levels_supported[i] < 1.0)
				if (zoom_levels_supported[i] > zoom)
					continue;
			
			double d = zoom - zoom_levels_supported[i];
			
			d = d < 0 ? -d : d;
			
			if (d < delta)
			{
				delta = d;
				k = j;
				ozf_zoom = zoom;
			}
		}		
		

		zoom_table[i].zoom		= zoom_levels_supported[i];
		zoom_table[i].source	= k;
		zoom_table[i].factor	= zoom_levels_supported[i] / ozf_zoom;

		logstream_write("scale: %f, selected source scale: %f (%d), factor: %f\n", 
				zoom_table[i].zoom, ozf_zoom, k, zoom_table[i].zoom/ozf_zoom);
	}

}

/*--------------------------------------------------------------------------*/
SDL_Surface* map_container::tile_get(long c, long r, long inverse)
{
	SDL_Surface* tile = NULL;
    Uint32 rmask, gmask, bmask, amask;
	
	if (c < 0 || c > tiles_per_x() - 1)
		return NULL;

	if (r < 0 || r > tiles_per_y() - 1)
		return NULL;

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

//	logstream_write("processing request for tile data, c = %d, r = %d\n",
//					c, r);
		
	tile = SDL_CreateRGBSurface(SDL_SWSURFACE, 64, 64, 32,
								rmask, gmask, bmask, amask);

	if (tile)
	{
		if(SDL_MUSTLOCK(tile))
			SDL_LockSurface(tile);
				
		image_ops.get_tile(image, zoom_table[zoom_current].source, c, r, (unsigned char*)tile->pixels);
		
		if(SDL_MUSTLOCK(tile))
			SDL_UnlockSurface(tile);
			
		if (tile && (c == tiles_per_x() - 1 || r == tiles_per_y() - 1))
		{
			double crop_dx = tile->w;
			double crop_dy = tile->h;
			
			if(c == tiles_per_x() - 1)
			{
				long w = image_ops.scale_dx(image, zoom_table[zoom_current].source);
				long dx = w - (c)*64;
				crop_dx = dx;
			}
			

			if(r == tiles_per_y() - 1)
			{
				long h = image_ops.scale_dy(image, zoom_table[zoom_current].source);
				long dy = h - (r) * 64;
				crop_dy = dy;
			}
			
			if (crop_dx <= tile->w && crop_dy <= tile->h && crop_dx && crop_dy)
			{
//				logstream_write("cropping from %d %d to %f %f\n", 
//										tile->w, tile->h, 
//										crop_dx, crop_dy);

				SDL_Surface* cropped = 
					SDL_CreateRGBSurface(SDL_SWSURFACE, crop_dx, crop_dy, 32,
										rmask, gmask, bmask, amask);
				
							
				for(int i = 0; i < crop_dy; i++)
				{
					unsigned char* s = (unsigned char*)tile->pixels;
					unsigned char* d = (unsigned char*)cropped->pixels;
				
					s += i*tile->pitch;
					d += i*cropped->pitch;
				
					memcpy(d, s, cropped->pitch);
				}
	
				SDL_FreeSurface(tile);
				
				tile = cropped;
			}
			else
			{
				SDL_FreeSurface(tile);
				tile = NULL;
			}
		}
			
		if (tile && zoom_table[zoom_current].factor != 1.0)
		{
			int smoothing = SMOOTHING_OFF;
			
			if (zoom_table[zoom_current].zoom < 1.0)
				smoothing = SMOOTHING_ON;
			
			SDL_Surface* scaled = 
				zoomSurface( tile, zoom_table[zoom_current].factor, zoom_table[zoom_current].factor, 
								smoothing );
			
			SDL_FreeSurface(tile);
						
			tile = scaled;
		}
		
	
		if (tile && inverse)
		{
			if(SDL_MUSTLOCK(tile)) SDL_LockSurface(tile);

			image_rgb src;

			src.m_data		= (Uint8*)tile->pixels;
			src.m_width		= tile->w;
			src.m_height	= tile->h;
			src.m_pitch		= tile->pitch;

			map_negative(&src);
		
			if(SDL_MUSTLOCK(tile)) SDL_UnlockSurface(tile);
		}
		
	}
	
	//if (tile)
	//rectangleRGBA(tile, 0, 0, tile->w-1, tile->h-1, 255, 0, 0, 255);
								  
	return tile;
}


/*--------------------------------------------------------------------------*/
void map_container::tile_forget(SDL_Surface* tile)
{
	if (tile)
		SDL_FreeSurface(tile);
}

/*--------------------------------------------------------------------------*/
void map_container::ll_to_cr(double latitude, double longitude, long *c, long *r)
{
	long x, y;
	
	ll_to_xy_onmap(latitude, longitude, &x, &y);

	long tile_x = (abs)((long)mcround(x))/(64 * zoom_table[zoom_current].factor);
	long tile_y = (abs)((long)mcround(y))/(64 * zoom_table[zoom_current].factor);

	logstream_write("converting ll to cr, lat = %f, lon = %f, c = %d, r = %d\n",
					latitude, longitude, tile_x, tile_y);


	*c = tile_x;
	*r = tile_y;

}

/*--------------------------------------------------------------------------*/
void map_container::ll_to_xy_onmap(double latitude, double longitude, long *x, long *y)
{
	double dbl_x, dbl_y;
	
	projection->ll_to_xy(&dbl_x, &dbl_y, longitude, latitude);

	*x = (long)mcround(dbl_x);
	*y = (long)mcround(dbl_y);

	logstream_write("converting ll to map xy, lat = %f, lon = %f, x = %d, y = %d\n",
					latitude, longitude, *x, *y);
}

/*--------------------------------------------------------------------------*/
long map_container::xy_onmap_to_ll(long x, long y, double* latitude, double* longitude)
{
	long r =
	projection->xy_to_ll(latitude, longitude,
						(double)x,
						(double)y);
			
	logstream_write("converting map xy to ll , x = %d, y = %d, lat = %f, lon = %f\n",
					x, y, *latitude, *longitude);
			
						
	return r;					
}

/*--------------------------------------------------------------------------*/
void map_container::ll_to_xy_on_tile(double latitude, double longitude, long *x, long *y)
{
	long map_x, map_y;
	long c, r;

	ll_to_xy_onmap(latitude, longitude, &map_x,	&map_y);
	
	ll_to_cr(latitude, longitude, &c, &r);

	*x = map_x - c * (64 * zoom_table[zoom_current].factor);
	*y = map_y - r * (64 * zoom_table[zoom_current].factor);
	
	logstream_write("converting ll to tile xy, lat = %f, lon = %f, x = %d, y = %d\n",
					latitude, longitude, *x, *y);
	
}

/*--------------------------------------------------------------------------*/
long map_container::tile_dx(int c, int r)
{
	if (c > tiles_per_x() - 1 ||
		r == tiles_per_y() - 1)
	{
		return 0;
	}

	double dx = 64;

	if (c == tiles_per_x() - 1)
	{
		long w = image_ops.scale_dx(image, zoom_table[zoom_current].source);
		dx = w - (w / 64)*64;
	}
	
	return dx * zoom_table[zoom_current].factor;
}

/*--------------------------------------------------------------------------*/
long map_container::tile_dy(int c, int r)
{
	if (c > tiles_per_x() - 1 ||
		r == tiles_per_y() - 1)
		return 0;

	double dy = 64;
	
	if ( r == tiles_per_y() - 1)
	{
		long h = image_ops.scale_dy(image, zoom_table[zoom_current].source);
		dy = h - (h / 64)*64;
	}
	
	
	return dy * zoom_table[zoom_current].factor;
}

/*--------------------------------------------------------------------------*/
long map_container::tiles_per_x()
{
	return image_ops.num_tiles_per_x(image, zoom_table[zoom_current].source);
}

/*--------------------------------------------------------------------------*/
long map_container::tiles_per_y()
{
	return image_ops.num_tiles_per_y(image, zoom_table[zoom_current].source);
}

/*--------------------------------------------------------------------------*/
long map_container::scales_amount()
{
	return ZOOMS_SUPPORTED;
}


/*--------------------------------------------------------------------------*/
int	map_container::zoom_in_avail()
{
	if (zoom_current < ZOOMS_SUPPORTED - 1)
	{
		return 1;
	}
	
	return 0;
}

/*--------------------------------------------------------------------------*/
int map_container::zoom_out_avail()
{
	if (zoom_current)
	{
		return 1;
	}
	
	return 0;
}

/*--------------------------------------------------------------------------*/
void map_container::zoom_in()
{
	if (zoom_current < ZOOMS_SUPPORTED - 1)
	{
		zoom_current++;
		
		logstream_write("requested zoom, %d\n",
				(long)(zoom_table[zoom_current].zoom * 100));
				
		map_info_update();
	}
}

/*--------------------------------------------------------------------------*/
void map_container::zoom_out()
{
	if (zoom_current)
	{
		zoom_current--;

		logstream_write("requested zoom, %d\n",
				(long)(zoom_table[zoom_current].zoom * 100));
		
		map_info_update();
	}
}

/*--------------------------------------------------------------------------*/
long map_container::ll_inside_map(double latitude, double longitude)
{
	double x, y;

	return
	projection->ll_to_xy(&x, &y, longitude, latitude);
}

/*--------------------------------------------------------------------------*/
long map_container::map_width()
{
	double width = 0;
	
	width = image_ops.scale_dx(image, zoom_table[zoom_current].source);
	width *= zoom_table[zoom_current].factor;

	logstream_write("returning map width, %d\n",
					(long)mcround(width));
	
	return (long)mcround(width);
}

/*--------------------------------------------------------------------------*/
long map_container::map_height()
{
	double height = 0;
	
	height = image_ops.scale_dy(image, zoom_table[zoom_current].source);
	height *= zoom_table[zoom_current].factor;

	logstream_write("returning map height, %d\n",
					(long)mcround(height));
	
	return (long)mcround(height);
}

/*--------------------------------------------------------------------------*/
double map_container::zoom_level_get()
{
	return
	zoom_table[zoom_current].zoom;
}

/*--------------------------------------------------------------------------*/
void map_container::zoom_level_set(double factor)
{
	double zoom_delta = DBL_MAX;

	logstream_write("requested zoom, %f\n", factor);

	for (int i = 0; i < ZOOMS_SUPPORTED; i++)
	{
		double delta = zoom_table[i].zoom - factor;

		if ( delta < 0 )
			delta = -delta;

		if ( delta < zoom_delta)
		{
			logstream_write("selecting %f\n", zoom_table[i].zoom);

			zoom_current = i;
			zoom_delta = delta;
		}
	}
	
	
	map_info_update();
}

/*--------------------------------------------------------------------------*/
char* map_container::file_name_get()
{
	return filename;
}

/*--------------------------------------------------------------------------*/
double map_container::mpp()
{
	return calibration->mpp;
}

/*--------------------------------------------------------------------------*/
long map_container::scale_up_possible()
{
	if (zoom_current < ZOOMS_SUPPORTED - 1)
		return 1;
		
	return 0;
}

/*--------------------------------------------------------------------------*/
long map_container::scale_down_possible()
{
	if (zoom_current)
		return 1;
	
	return 0;
}

/*--------------------------------------------------------------------------*/
void map_container::map_info_update()
{
	projection_update();
}

/*--------------------------------------------------------------------------*/
void map_container::projection_update()
{
	if(projection)
		delete projection;

	projection =
		new map_projection((char*)calibration->projection);

	LLXY* calibrations = 
		(LLXY*)malloc(calibration->calibrations_amount * sizeof(LLXY));

	memcpy(	calibrations, calibration->calibrations,
			calibration->calibrations_amount * sizeof(LLXY));

	for ( long i = 0; i < calibration->calibrations_amount; i++ )
	{
		calibrations[i].x *= zoom_table[zoom_current].zoom;
		calibrations[i].y *= zoom_table[zoom_current].zoom;
	}

	projection->set_params(&calibration->projection_params, map_width(), map_height());

	projection->bind_by_points(calibrations, calibration->calibrations_amount);

	free(calibrations);
}

/*--------------------------------------------------------------------------*/
long map_container::cr_inside_map(long c, long r)
{
	double lt, lg;

	long x = 0;
	long y = 0;

	x = c * tile_dx();
	y = r * tile_dy();

	xy_onmap_to_ll(x, y, &lt, &lg);

	if ( ll_inside_map(lt, lg) )
		return 1;

	y = r * tile_dx() + tile_dy();

	xy_onmap_to_ll(x, y, &lt, &lg);

	if ( ll_inside_map(lt, lg) )
		return 1;

	y = r * tile_dy();
	x = c * tile_dx() + tile_dx();

	xy_onmap_to_ll(x, y, &lt, &lg);

	if ( ll_inside_map(lt, lg) )
		return 1;

	y = r * tile_dx() + tile_dy();
	x = c * tile_dx() + tile_dx();

	xy_onmap_to_ll(x, y, &lt, &lg);

	if ( ll_inside_map(lt, lg) )
		return 1;

	return 0;
}

