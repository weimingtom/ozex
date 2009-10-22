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

#ifndef __MAP_INDEX_INCLUDED
#define __MAP_INDEX_INCLUDED

#include "file_collection.h"
#include "map_loader.h"
#include "map_projection.h"

/*--------------------------------------------------------------------------*/
class map_index 
{
	public:
		map_index(char* path);
		~map_index();
		
		int		maps_for_ll(double lat, double lon, int** indexes);
		char*	map_filename(int id);
		int		maps_amount();
		double	mpp(int id);

	private:
		int		ll_inside_map(int id, double lat, double lon);
		
		
	private:
		file_collection*	maps_collection;
		map_info**			maps;
		map_projection**	projections;
		
};

#endif