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


#ifndef __MAP_LOADER_INCLUDED
#define __MAP_LOADER_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "map_types.h"

#define CALIBRATION_POINTS_MAX	64

/*--------------------------------------------------------------------------*/
typedef struct 
{
	long					calibrations_amount;
	LLXY*					calibrations;

	long					corners_amount;
	LLXY*					corners;
	
	char*					datum;
	char*					projection;
	projection_params_table	projection_params;

	double					mpp;

	char*					image_path;
	char*					image_file;

	long					width, height;
} map_info;


/*--------------------------------------------------------------------------*/
#ifdef __cplusplus
extern "C" {
#endif

map_info*	maploader_openmap(char* filename);
void		maploader_closemap(map_info*);
void		maploader_debugmap(map_info* mi);

#ifdef __cplusplus
};
#endif

#endif