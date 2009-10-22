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

#include "map_index.h"
#include "qsort.h"

#include "log_stream.h"

static int ll_inside_map(LLXY *corners, long numcorners, double lat, double lon);

/*--------------------------------------------------------------------------*/
map_index::map_index(char* path)
{
	maps_collection = filecollection_init();
	filecollection_collect(maps_collection, path, ".map");

	maps = (map_info**)malloc(maps_collection->files_amount * sizeof(map_info*));
	projections = (map_projection**)malloc(maps_collection->files_amount * sizeof(map_projection*));
	
	for (int i = 0; i < maps_collection->files_amount; i++)
	{
		map_info* mi = maploader_openmap(maps_collection->files[i]);

		maploader_debugmap(mi);
		
		maps[i] = mi;
		
		map_projection* projection =
			new map_projection((char*)mi->projection);

		projection->set_params(&mi->projection_params, mi->width, mi->height);

		projection->bind_by_points(mi->calibrations, mi->calibrations_amount);
		
		projections[i] = projection;
	}	
}

/*--------------------------------------------------------------------------*/
map_index::~map_index()
{
	for (int i = 0; i < maps_collection->files_amount; i++)
	{
		maploader_closemap(maps[i]);
		delete projections[i];
	}
	
	free(maps);
	free(projections);
	
	filecollection_forget(maps_collection);
}

/*--------------------------------------------------------------------------*/
char*	map_index::map_filename(int id)
{
	return maps_collection->files[id];
}

/*--------------------------------------------------------------------------*/
double	map_index::mpp(int id)
{
	return maps[id]->mpp;
}

/*--------------------------------------------------------------------------*/
int		map_index::maps_amount()
{
	return maps_collection->files_amount;
}


/*--------------------------------------------------------------------------*/
long map_compare_by_mpp(const void *a, const void *b, const void *c)
{
	int* x = (int*)a;
	int* y = (int*)b;

	map_info** maps = (map_info**)c;
	
	map_info* map_a = maps[*x];
	map_info* map_b = maps[*y];

	if ( map_a->mpp == map_b->mpp)
		return 0;

	if ( map_a->mpp < map_b->mpp)
		return -1;

	return 1;
}

/*--------------------------------------------------------------------------*/
int	map_index::ll_inside_map(int id, double lat, double lon)
{
	double x, y;
	map_info* mi = maps[id];

	int r = projections[id]->ll_to_xy(&x, &y, lon, lat);
	
	if (r)
	{
		if (x >= mi->width || y > mi->height)
			r = 0;
	}

	return r;
}

/*--------------------------------------------------------------------------*/
int	map_index::maps_for_ll(double lat, double lon, int** indexes)
{
	int		maps_found = 0;
	
	int*	collection = 
		(int*)malloc(maps_collection->files_amount * sizeof(int));

	logstream_write("searching for maps at position: %f %f\n", lat, lon);

	for (int i = 0; i < maps_collection->files_amount; i++)
	{
		logstream_write("checking: %s ...", map_filename(i));
		if (ll_inside_map(i, lat, lon))
		{
			logstream_write(" INSIDE\n");
			collection[maps_found] = i;
			maps_found++;
		}
		else
		{
			logstream_write(" OUTSIDE\n");
		}
	}	
	
	if (maps_found > 1)
	{
		quicksort ((void *)collection, maps_found, sizeof(int), 
					(const void*)maps, map_compare_by_mpp);
	}
	
	*indexes = collection;
	
	return maps_found;
}


