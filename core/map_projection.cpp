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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef WIN32
#include <malloc.h>
#endif

#include "map_projection.h"
#include "gauss.h"

#include "albers.h"
#include "azeq.h"
#include "bonne.h"
#include "cassini.h"
#include "cyleqa.h"
#include "eckert4.h"
#include "eckert6.h"
#include "eqdcyl.h"
#include "gnomonic.h"
#include "grinten.h"
#include "lambert_1.h"
#include "lambert_2.h"
#include "mercator.h"
#include "miller.h"
#include "mollweid.h"
#include "neys.h"
#include "nzmg.h"
#include "omerc.h"
#include "orthogr.h"
#include "polarst.h"
#include "polycon.h"
#include "sinusoid.h"
#include "stereogr.h"
#include "tranmerc.h"
#include "trcyleqa.h"
#include "bng.h"

#include "log_stream.h"

/*--------------------------------------------------------------------------*/
extern "C" {

long Convert_Geodetic_To_LL(double Latitude, double Longitude, double *Easting, double *Northing)
{
	*Easting = Longitude;
	*Northing = Latitude;
	return 0;
}

long Convert_LL_To_Geodetic(double Easting, double Northing, double *Latitude, double *Longitude)
{
	*Latitude = Northing;
	*Longitude = Easting;
	return 0;
}

};

/*--------------------------------------------------------------------------*/
static projection_info projections[] = 
{
	{Albers,				Convert_Geodetic_To_Albers,					Convert_Albers_To_Geodetic},					
	{Azimuthal_Equidistant,	Convert_Geodetic_To_Azimuthal_Equidistant,	Convert_Azimuthal_Equidistant_To_Geodetic},	
	{Bonne,					Convert_Geodetic_To_Bonne,					Convert_Bonne_To_Geodetic},					
	{Cassini,				Convert_Geodetic_To_Cassini,				Convert_Cassini_To_Geodetic},				
	{Cyl_Eq_Area,			Convert_Geodetic_To_Cyl_Eq_Area,			Convert_Cyl_Eq_Area_To_Geodetic},			
	{Eckert4,				Convert_Geodetic_To_Eckert4,				Convert_Eckert4_To_Geodetic},				
	{Eckert6,				Convert_Geodetic_To_Eckert6,				Convert_Eckert6_To_Geodetic},				
	{Equidistant_Cyl,		Convert_Geodetic_To_Equidistant_Cyl,		Convert_Equidistant_Cyl_To_Geodetic},		
	{Gnomonic,				Convert_Geodetic_To_Gnomonic,				Convert_Gnomonic_To_Geodetic},				
	{Van_der_Grinten,		Convert_Geodetic_To_Van_der_Grinten,		Convert_Van_der_Grinten_To_Geodetic},		
	{Lambert_1,				Convert_Geodetic_To_Lambert_1,				Convert_Lambert_1_To_Geodetic},				
	{Lambert_2,				Convert_Geodetic_To_Lambert_2,				Convert_Lambert_2_To_Geodetic},				
	{Mercator,				Convert_Geodetic_To_Mercator,				Convert_Mercator_To_Geodetic},				
	{Miller,				Convert_Geodetic_To_Miller,					Convert_Miller_To_Geodetic},					
	{Mollweide,				Convert_Geodetic_To_Mollweide,				Convert_Mollweide_To_Geodetic},				
	{Neys,					Convert_Geodetic_To_Neys,					Convert_Neys_To_Geodetic},					
	{NZMG,					Convert_Geodetic_To_NZMG,					Convert_NZMG_To_Geodetic},					
	{Oblique_Mercator,		Convert_Geodetic_To_Oblique_Mercator,		Convert_Oblique_Mercator_To_Geodetic},		
	{Orthographic,			Convert_Geodetic_To_Orthographic,			Convert_Orthographic_To_Geodetic},			
	{Polar_Stereographic,	Convert_Geodetic_To_Polar_Stereographic,	Convert_Polar_Stereographic_To_Geodetic},	
	{Polyconic,				Convert_Geodetic_To_Polyconic,				Convert_Polyconic_To_Geodetic},				
	{Sinusoidal,			Convert_Geodetic_To_Sinusoidal,				Convert_Sinusoidal_To_Geodetic},				
	{Stereographic,			Convert_Geodetic_To_Stereographic,			Convert_Stereographic_To_Geodetic},			
	{Transverse_Mercator,	Convert_Geodetic_To_Transverse_Mercator,	Convert_Transverse_Mercator_To_Geodetic},				
	{Trans_Cyl_Eq_Area,		Convert_Geodetic_To_Trans_Cyl_Eq_Area,		Convert_Trans_Cyl_Eq_Area_To_Geodetic},		
	{LatLon,				Convert_Geodetic_To_LL,						Convert_LL_To_Geodetic},
};

/*--------------------------------------------------------------------------*/
typedef struct
{
	ProjectionCode	code;
	char*			name;
} projection_id;

/*--------------------------------------------------------------------------*/
/*
+ Latitude/Longitude
+ Mercator
+ Transverse Mercator
(UTM) Universal Transverse Mercator
(BNG) British National Grid
(IG) Irish Grid
(NZG) New Zealand Grid
(SG) Swedish Grid
(SUI) Swiss Grid
(I) France Zone I
(II) France Zone II
(III) France Zone III
(IV) France Zone IV
+ Lambert Conformal Conic
+ (A)Lambert Azimuthual Equal Area
(EQC) Equidistant Conic
+ Sinusoidal
Polyconic (American)
+ Albers Equal Area
Van Der Grinten
Vertical Near-Sided Perspective
(WIV) Wagner IV
+ Bonne
(MT0) Montana State Plane Zone 2500
(ITA1) Italy Grid Zone 1
(ITA2) Italy Grid Zone 2
(VICMAP-TM) Victoria Aust.(pseudo AMG)
(VICGRID) Victoria Australia
(VG94) VICGRID94 Victoria Australia*/

/*--------------------------------------------------------------------------*/
static projection_id known_projections[] =
{
 {LatLon,				"Latitude/Longitude"},
 {Mercator,				"Mercator"},
 {Transverse_Mercator,	"Transverse Mercator"},
 {Lambert_2,			"Lambert Conformal Conic"},
 {Cyl_Eq_Area,			"(A)Lambert Azimuthual Equal Area"},
 
 {Albers,				"Albers Equal Area"},
 {Bonne,				"Bonne"},
 {Sinusoidal,			"Sinusoidal"},
 {Van_der_Grinten,		"Van Der Grinten"}
};


/*--------------------------------------------------------------------------*/
map_projection::map_projection(char* projection_str)
{
	ProjectionCode proj_code = (ProjectionCode)LatLon;

	int projection_found = 0;
	
	for (int i = 0; i < (sizeof(known_projections)/sizeof(known_projections[0])); i++)
	{
		if (!strcmp(projection_str, known_projections[i].name))
		{
			proj_code = known_projections[i].code;
			projection_found = 1;
			break;
		}
	}
	
	if (!projection_found)
		logstream_write("projection %s is not found\n", projection_str);
	else
		logstream_write("projection %s is found\n", projection_str);

	projection_params = (projection_params_table*)malloc(sizeof(projection_params_table));
	bind = (linear_binding*)malloc(sizeof(linear_binding));
	projection_params->ellipsoid_code = 0;

	m_pcode = proj_code;

	m_nID = -1;

	for(int i = 0; i < sizeof(projections)/sizeof(projections[0]); i++)
	{
		if (projections[i].code == proj_code)
		{
			m_nID  = i;
			break;
		}
	}
}

/*--------------------------------------------------------------------------*/
map_projection::~map_projection()
{
	if(projection_params->ellipsoid_code)
		free(projection_params->ellipsoid_code);

	free(projection_params);
	free(bind);
}

/*--------------------------------------------------------------------------*/
long map_projection::set_params(projection_params_table* params, long nMapDX, long nMapDY)
{
	m_nMapDX = nMapDX;
	m_nMapDY = nMapDY;

	if(projection_params->ellipsoid_code)
		free(projection_params->ellipsoid_code);

	memcpy(projection_params, params, sizeof(projection_params_table));

	if(params->ellipsoid_code)
	{
		projection_params->ellipsoid_code = (char*)malloc(strlen(params->ellipsoid_code)+1);
		strcpy(projection_params->ellipsoid_code, params->ellipsoid_code);
	}
	else
		projection_params->ellipsoid_code = 0;

	use_params(params);

	return 0;
}

/*--------------------------------------------------------------------------*/
void map_projection::use_params(projection_params_table* param)
{
	switch (m_pcode)
	{
		case Albers: 
			Set_Albers_Parameters(param->a,param->f,param->origin_latitude,param->central_meridian,param->std_parallel_1,param->std_parallel_2,param->false_easting,param->false_northing);
			break;
		case Azimuthal_Equidistant: 
			Set_Azimuthal_Equidistant_Parameters(param->a,param->f,param->origin_latitude,param->central_meridian,param->false_easting,param->false_northing);
			break;
		case Bonne: 
			Set_Bonne_Parameters(param->a,param->f,param->origin_latitude,param->central_meridian,param->false_easting,param->false_northing);
			break;
		case Cassini: 
			Set_Cassini_Parameters(param->a,param->f,param->origin_latitude,param->central_meridian,param->false_easting,param->false_northing);
			break;
		case Cyl_Eq_Area: 
			Set_Cyl_Eq_Area_Parameters(param->a,param->f,param->origin_latitude,param->central_meridian,param->false_easting,param->false_northing);
			break;
		case Eckert4: 
			Set_Eckert4_Parameters(param->a,param->f,param->central_meridian,param->false_easting,param->false_northing);
			break;
		case Eckert6: 
			Set_Eckert6_Parameters(param->a,param->f,param->central_meridian,param->false_easting,param->false_northing);
			break;
		case Equidistant_Cyl: 
			Set_Equidistant_Cyl_Parameters(param->a,param->f,param->std_parallel_1,param->central_meridian,param->false_easting,param->false_northing);
			break;
		case Gnomonic: 
			Set_Gnomonic_Parameters(param->a,param->f,param->origin_latitude,param->central_meridian,param->false_easting,param->false_northing);
			break;
		case Van_der_Grinten: 
			Set_Van_der_Grinten_Parameters(param->a,param->f,param->central_meridian,param->false_easting,param->false_northing);
			break;
		case Lambert_1: 
			Set_Lambert_1_Parameters(param->a,param->f,param->origin_latitude,param->central_meridian,param->false_easting,param->false_northing,param->scale_factor);
			break;
		case Lambert_2: 
			Set_Lambert_2_Parameters(param->a,param->f,param->origin_latitude,param->central_meridian,param->std_parallel_1,param->std_parallel_2,param->false_easting,param->false_northing);
			break;
		case Mercator: 
			double Scale_Factor;
			Set_Mercator_Parameters(param->a,param->f,param->origin_latitude,param->central_meridian,param->false_easting,param->false_northing,&Scale_Factor);
			break;
		case Miller: 
			Set_Miller_Parameters(param->a,param->f,param->central_meridian,param->false_easting,param->false_northing);
			break;
		case Mollweide: 
			Set_Mollweide_Parameters(param->a,param->f,param->central_meridian,param->false_easting,param->false_northing);
			break;
		case Neys: 
			Set_Neys_Parameters(param->a,param->f,param->origin_latitude,param->central_meridian,param->std_parallel_1,param->false_easting,param->false_northing);
			break;
		case NZMG: 
			Set_NZMG_Parameters(param->ellipsoid_code);
			break;
		case Oblique_Mercator: 
			Set_Oblique_Mercator_Parameters(param->a,param->f,param->origin_latitude,param->latitude_1,param->longitude_1,param->latitude_2,param->longitude_2,param->false_easting,param->false_northing,param->scale_factor);
			break;
		case Orthographic: 
			Set_Orthographic_Parameters(param->a,param->f,param->origin_latitude,param->central_meridian,param->false_easting,param->false_northing);
			break;
		case Polar_Stereographic: 
			Set_Polar_Stereographic_Parameters(param->a,param->f,param->latitude_of_true_scale,param->longitude_down_from_pole,param->false_easting,param->false_northing);
			break;
		case Polyconic: 
			Set_Polyconic_Parameters(param->a,param->f,param->origin_latitude,param->central_meridian,param->false_easting,param->false_northing);
			break;
		case Sinusoidal: 
			Set_Sinusoidal_Parameters(param->a,param->f,param->central_meridian,param->false_easting,param->false_northing);
			break;
		case Stereographic: 
			Set_Stereographic_Parameters(param->a,param->f,param->origin_latitude,param->central_meridian,param->false_easting,param->false_northing);
			break;
		case Transverse_Mercator: 
			Set_Transverse_Mercator_Parameters(param->a,param->f,param->origin_latitude,param->central_meridian,param->false_easting,param->false_northing,param->scale_factor);
			break;
		case Trans_Cyl_Eq_Area: 
			Set_Trans_Cyl_Eq_Area_Parameters(param->a,param->f,param->origin_latitude,param->central_meridian,param->false_easting,param->false_northing,param->scale_factor);
			break;
		case LatLon:
			break;
		default:
			// no such projection code
			break;
	}
}

/*--------------------------------------------------------------------------*/
void map_projection::bind_by_points(LLXY *pa, int pt_amount)
{
	AnchorPoint *pt = (AnchorPoint*)alloca(sizeof(AnchorPoint)*pt_amount);

	use_params(projection_params);

	for (int i = 0; i < pt_amount; i++)
	{
		pt[i].ll.lat = pa[i].lat;
		pt[i].ll.lon = pa[i].lon;
		pt[i].xy.x   = pa[i].x;
		pt[i].xy.y   = pa[i].y;

		projections[m_nID].fConvertFromGeodetic(
				pt[i].ll.lat*DEG_TO_RAD,
				pt[i].ll.lon*DEG_TO_RAD,
				&pt[i].ne.e,
				&pt[i].ne.n	);

	}

	getKx(pt, pt_amount, bind);
	getKy(pt, pt_amount, bind);
	getKLat(pt, pt_amount, bind);
	getKLon(pt, pt_amount, bind);
}

/*--------------------------------------------------------------------------*/
void map_projection::init_3x3(	double** a, double *b, int pt_amount, double *P1, double *P2, double *P3)
{
	for (int i = 0; i < 3; i++)
	{
		b[i] = 0;
		
		for (int j = 0; j < 3; j++)
			a[i][j] = 0;
	}

	for(int i = 0; i < pt_amount; i++)
	{
		a[0][0] += P1[i] * P1[i];
		a[0][1] += P1[i] * P2[i];
		a[0][2] += P1[i];
		a[1][1] += P2[i] * P2[i];
		a[1][2] += P2[i];
		b[0] += P3[i] * P1[i];
		b[1] += P3[i] * P2[i];
		b[2] += P3[i];
	}

	a[1][0] = a[0][1];
	a[2][0] = a[0][2];
	a[2][1] = a[1][2];
	a[2][2] = pt_amount;
}

/*--------------------------------------------------------------------------*/
void map_projection::getKx(AnchorPoint *pt, int pt_amount, linear_binding* bind)
{
	double a0[3], a1[3], a2[3];
	double *a[] = {a0, a1, a2};
	double b[3];

	double *p1 = (double *)alloca(sizeof(double)*pt_amount);
	double *p2 = (double *)alloca(sizeof(double)*pt_amount);
	double *p3 = (double *)alloca(sizeof(double)*pt_amount);

	for (int i = 0; i < pt_amount; i++)
	{
		p1[i] = pt[i].ne.n;
		p2[i] = pt[i].ne.e;
		p3[i] = pt[i].xy.x;
	}
	
	init_3x3((double**)a, b, pt_amount, p1, p2, p3);
	gauss((double**)a, b, bind->Kx, 3);
}

/*--------------------------------------------------------------------------*/
void map_projection::getKy(AnchorPoint *pt, int pt_amount, linear_binding* bind)
{
	double a0[3], a1[3], a2[3];
	double *a[] = {a0, a1, a2};
	double b[3];
	
	double *p1 = (double *)alloca(sizeof(double)*pt_amount);
	double *p2 = (double *)alloca(sizeof(double)*pt_amount);
	double *p3 = (double *)alloca(sizeof(double)*pt_amount);

	for (int i = 0; i < pt_amount; i++)
	{
		p1[i] = pt[i].ne.n;
		p2[i] = pt[i].ne.e;
		p3[i] = pt[i].xy.y;
	}

	init_3x3((double**)a, b, pt_amount, p1, p2, p3);
	gauss((double**)a, b, bind->Ky, 3);
}

/*--------------------------------------------------------------------------*/
void map_projection::getKLat(AnchorPoint *pt, int pt_amount, linear_binding* bind)
{
	double a0[3], a1[3], a2[3];
	double *a[] = {a0, a1, a2};
	double b[3];

	double *p1 = (double *)alloca(sizeof(double)*pt_amount);
	double *p2 = (double *)alloca(sizeof(double)*pt_amount);
	double *p3 = (double *)alloca(sizeof(double)*pt_amount);

	for (int i = 0; i < pt_amount; i++)
	{
		p1[i] = pt[i].xy.x;
		p2[i] = pt[i].xy.y;
		p3[i] = pt[i].ne.n;
	}
	
	init_3x3((double**)a, b, pt_amount, p1, p2, p3);
	gauss((double**)a, b, bind->Klat, 3);
}

/*--------------------------------------------------------------------------*/
void map_projection::getKLon(AnchorPoint *pt, int pt_amount, linear_binding* bind)
{
	double a0[3], a1[3], a2[3];
	double *a[] = {a0, a1, a2};
	double b[3];
	double *p1 = (double *)alloca(sizeof(double)*pt_amount);
	double *p2 = (double *)alloca(sizeof(double)*pt_amount);
	double *p3 = (double *)alloca(sizeof(double)*pt_amount);

	for (int i = 0; i < pt_amount; i++)
	{
		p1[i] = pt[i].xy.x;
		p2[i] = pt[i].xy.y;
		p3[i] = pt[i].ne.e;
	}

	init_3x3((double**)a, b, pt_amount, p1, p2, p3);
	gauss((double**)a, b, bind->Klon, 3);
}

/*--------------------------------------------------------------------------*/
bool map_projection::xy_to_ll(double *lat, double *lon, double x, double y)
{
	double nn,ee;
	nn = bind->Klat[0]*x + bind->Klat[1]*y + bind->Klat[2];
	ee = bind->Klon[0]*x + bind->Klon[1]*y + bind->Klon[2];
	
	use_params(projection_params);

	projections[m_nID].fConvertToGeodetic(ee,nn,lat,lon);

	*lat *= RAD_TO_DEG;
	*lon *= RAD_TO_DEG;

	return true;
}

/*--------------------------------------------------------------------------*/
bool map_projection::ll_to_xy(double *x, double *y, double lon, double lat)
{
	double nn,ee;

	use_params(projection_params);

	projections[m_nID].fConvertFromGeodetic(lat*DEG_TO_RAD,lon*DEG_TO_RAD,&ee,&nn);

	*x = (double)(bind->Kx[0]*nn + bind->Kx[1]*ee + bind->Kx[2]);
	*y = (double)(bind->Ky[0]*nn + bind->Ky[1]*ee + bind->Ky[2]);

	if ((long)(*x) >= 0 && (long)(*x) < m_nMapDX &&
		(long)(*y) >= 0 && (long)(*y) < m_nMapDY)
		return true;

	return false;
}
