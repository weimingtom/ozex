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

#ifndef __MAP_DATUMS_INCLUDED
#define __MAP_DATUMS_INCLUDED

#include <math.h>

/*--------------------------------------------------------------------------*/
typedef struct 
{
	char*	name;
	char*	ellipsoid;
	
	double	a;
	double	invf;
	
	double	dx;
	double	dy;
	double	dz;
		
} map_datum;

/*--------------------------------------------------------------------------*/
void		mapdatums_init(char* filename);
map_datum*	mapdatums_find(char* name);
void		mapdatums_done();
void		mapdatums_debug();

void		mapdatum_to_wgs84(	double Sphi, double Slam, double SH,
								double *Dphi, double *Dlam, double *DH,
								map_datum* params);

#endif