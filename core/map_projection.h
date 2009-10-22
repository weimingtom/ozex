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

#ifndef __MAP_PROJECTION_INCLUDED
#define __MAP_PROJECTION_INCLUDED

#include "map_types.h"

/*--------------------------------------------------------------------------*/
#define RAD_TO_DEG	57.29577951308232
#define DEG_TO_RAD	.0174532925199432958

/*--------------------------------------------------------------------------*/
typedef struct 
{ 
	double x, y; 
} XY;

/*--------------------------------------------------------------------------*/
typedef struct 
{ 
	double lon, lat; 
} LL;

/*--------------------------------------------------------------------------*/
typedef struct 
{ 
	double n, e; 
} NE;

/*--------------------------------------------------------------------------*/
typedef struct 
{
	XY xy;
	LL ll;
	NE ne;
} AnchorPoint;

/*--------------------------------------------------------------------------*/
typedef	long (*ConvertFromGeodetic)(double Latitude,double Longitude,double *Easting,double *Northing);
typedef	long (*ConvertToGeodetic)(double Easting,double Northing,double *Latitude,double *Longitude);


/*--------------------------------------------------------------------------*/
enum ProjectionCode 
{
	Albers = 100,					
	Azimuthal_Equidistant,	
	Bonne,					
	Cassini,					
	Cyl_Eq_Area,				
	Eckert4,					
	Eckert6,					
	Equidistant_Cyl,			
	Gnomonic,				
	Van_der_Grinten,			
	Lambert_1,				
	Lambert_2,				
	Mercator,				
	Miller,					
	Mollweide,				
	Neys,					
	NZMG,					
	Oblique_Mercator,		
	Orthographic,			
	Polar_Stereographic,		
	Polyconic,				
	Sinusoidal,				
	Stereographic,			
	Transverse_Mercator,				
	Trans_Cyl_Eq_Area,		
	LatLon,
	British_National_Grid
};

/*--------------------------------------------------------------------------*/
typedef	struct 
{
	ProjectionCode code;
	ConvertFromGeodetic fConvertFromGeodetic;
	ConvertToGeodetic	fConvertToGeodetic;
} projection_info;

/*--------------------------------------------------------------------------*/
typedef struct 
{
	double	Kx[3], Ky[3], Klat[3], Klon[3];
} linear_binding;

/*--------------------------------------------------------------------------*/
class map_projection
{
	public:
		map_projection(char* projection_str);
		~map_projection();

		long set_params(projection_params_table* params, long nMapDX, long nMapDY);
		void use_params(projection_params_table* params);
		
		void bind_by_points(LLXY *pt, int pt_amount);
		
		bool xy_to_ll(double *lat, double *lon, double x, double y);
		bool ll_to_xy(double *x, double *y, double lon, double lat);

	private:
		void init_3x3(	double** a, double *b, int pt_amount, double *P1, double *P2, double *P3);
		void getKx(AnchorPoint *pt, int pt_amount, linear_binding* bind);
		void getKy(AnchorPoint *pt, int pt_amount, linear_binding* bind);
		void getKLat(AnchorPoint *pt, int pt_amount, linear_binding* bind);
		void getKLon(AnchorPoint *pt, int pt_amount, linear_binding* bind);

	private:
		ProjectionCode m_pcode;
		projection_params_table* projection_params;
		
		long m_nMapDX;
		long m_nMapDY;

		linear_binding* bind; // binding coefficients calculated from vertex points

		long m_nID;
};

#endif


