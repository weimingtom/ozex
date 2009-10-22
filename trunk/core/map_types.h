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

// Author: Daniil Smelov <dn.smelov@gmail.com>
// Copyright Daniil Smelov 2006-2009


#ifndef __MAP_TYPES_INCLUDED
#define __MAP_TYPES_INCLUDED

typedef void map_stream;


/*--------------------------------------------------------------------------*/
typedef struct 
{
	long		x, y;

	double		lat;
	double		lon;
	
	double		n;
	double		e;
} LLXY;

/*--------------------------------------------------------------------------*/
typedef struct 
{
	double	a;
	double	f;
	double	central_meridian;
	double	origin_latitude;
	double	false_easting;
	double	false_northing;
	double	latitude_1;
	double	longitude_1;
	double	latitude_2;
	double	longitude_2;
	double	std_parallel_1;
	double	std_parallel_2;
	double	scale_factor;
	double	latitude_of_true_scale;
	double	longitude_down_from_pole;
	char*	ellipsoid_code;
} projection_params_table;

#endif