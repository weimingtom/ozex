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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "map_datums.h"
#include "ezxml.h"
#include "log_stream.h"

/*--------------------------------------------------------------------------*/
static const double pi = 3.14159265358979323846;

/*--------------------------------------------------------------------------*/
static void mapdatum_molodensky(double Sphi, double Slam, double SH, double Sa,
								double Sif, double *Dphi, double *Dlam,
								double *DH, double Da, double Dif, double dx,
								double dy, double dz);
/*--------------------------------------------------------------------------*/
#define MAP_DATUMS_MAX	512

/*--------------------------------------------------------------------------*/
static map_datum* map_datums_params = NULL;
static int map_datums_found = 0;

static map_datum map_datum_wgs84 = 
{
	"WGS 84", "WGS 84", 
	6378137.0, 298.25722356,
	0.0, 0.0, 0.0
};

/*--------------------------------------------------------------------------*/
void mapdatums_init(const char* filename)
{
	map_datums_params = (map_datum*)malloc(MAP_DATUMS_MAX * sizeof(map_datum));
	memset(map_datums_params, 0, MAP_DATUMS_MAX * sizeof(map_datum));

	ezxml_t datums = ezxml_parse_file(filename);
	
	if (datums)
	{
		ezxml_t	datum = ezxml_child(datums, "datum");
		
		if(!datums)
			logstream_write("datums section not found!\n");
	
		for (; datum; datum = datum->next)
		{
			const char* name = ezxml_attr(datum, "name");
			const char* ellipsoid = ezxml_attr(datum, "ellipsoid");
			const char* a = ezxml_attr(datum, "a");
			const char* invf = ezxml_attr(datum, "invf");
			const char* dx = ezxml_attr(datum, "dx");
			const char* dy = ezxml_attr(datum, "dy");
			const char* dz = ezxml_attr(datum, "dz");
				
			int i = map_datums_found;
			
			map_datums_params[i].name = (char*)malloc(strlen(name) + 1);
			strcpy(map_datums_params[i].name, name);
			
			map_datums_params[i].ellipsoid = (char*)malloc(strlen(ellipsoid) + 1);
			strcpy(map_datums_params[i].ellipsoid, ellipsoid);
			
			map_datums_params[i].a = atof(a);
			map_datums_params[i].invf = atof(invf);
			
			map_datums_params[i].dx = atof(dx);
			map_datums_params[i].dy = atof(dy);
			map_datums_params[i].dz = atof(dz);
			
			map_datums_found++;
		}
	
		ezxml_free(datums); 
	}

}


/*--------------------------------------------------------------------------*/
void mapdatums_debug()
{
	for (int i = 0; i < map_datums_found; i++)
	{
		logstream_write("%s %s %f %f %f %f %f\n",
				map_datums_params[i].name,
				map_datums_params[i].ellipsoid,
				map_datums_params[i].a,
				map_datums_params[i].invf,
				map_datums_params[i].dx,
				map_datums_params[i].dy,
				map_datums_params[i].dz);
	}
}

/*--------------------------------------------------------------------------*/
map_datum*	mapdatums_find(const char* name)
{
	logstream_write("requested data for datum %s\n", name);

	for (int i = 0; i < map_datums_found; i++)
	{
		if (!strcmp(map_datums_params[i].name, name))
		{
			logstream_write("datum found\n");

			return &map_datums_params[i];
		}
	}

	logstream_write("datum not found, assuming WGS84 params\n");

	return &map_datum_wgs84;
}

/*--------------------------------------------------------------------------*/
void mapdatums_done()
{
	if (map_datums_params)
	{
		for (int i = 0; i < map_datums_found; i++)
		{
			if (map_datums_params[i].name)
				free(map_datums_params[i].name);

			if (map_datums_params[i].ellipsoid)
				free(map_datums_params[i].ellipsoid);
		}
		
		free(map_datums_params);
		
		
		map_datums_params = NULL;
	}
}


/*--------------------------------------------------------------------------*/
double deg_to_rad(double v)
{
    return v*(double)((double)pi/(double)180.);
}

/*--------------------------------------------------------------------------*/
double rad_to_deg(double v)
{
    return v*(double)((double)180./(double)pi);
}


/* @func GPS_Math_Known_Datum_To_WGS84_M **********************************
**
** Transform datum to WGS84 using Molodensky
**
** @param [r] Sphi [double] source latitude (deg)
** @param [r] Slam [double] source longitude (deg)
** @param [r] SH   [double] source height  (metres)
** @param [w] Dphi [double *] dest latitude (deg)
** @param [w] Dlam [double *] dest longitude (deg)
** @param [w] DH   [double *] dest height  (metres)
** @param [r] n    [int32] datum number from GPS_Datum structure
**
** @return [void]
************************************************************************/
void mapdatum_to_wgs84(double Sphi, double Slam, double SH,
				     double *Dphi, double *Dlam, double *DH,
				     map_datum* params)
{
    double Sa;
    double Sif;
    double Da;
    double Dif;
    double x;
    double y;
    double z;
    
    Da  = (double) 6378137.0;
    Dif = (double) 298.257223563;
    
    Sa   = params->a;
    Sif  = params->invf;
    x    = params->dx;
    y    = params->dy;
    z    = params->dz;

    mapdatum_molodensky(Sphi,Slam,SH,Sa,Sif,Dphi,Dlam,DH,Da,Dif,x,y,z);
}


/* @func GPS_Math_Molodensky *******************************************
**
** Transform one datum to another
**
** @param [r] Sphi [double] source latitude (deg)
** @param [r] Slam [double] source longitude (deg)
** @param [r] SH   [double] source height  (metres)
** @param [r] Sa   [double] source semi-major axis (metres)
** @param [r] Sif  [double] source inverse flattening
** @param [w] Dphi [double *] dest latitude (deg)
** @param [w] Dlam [double *] dest longitude (deg)
** @param [w] DH   [double *] dest height  (metres)
** @param [r] Da   [double]   dest semi-major axis (metres)
** @param [r] Dif  [double]   dest inverse flattening
** @param [r] dx  [double]   dx
** @param [r] dy  [double]   dy
** @param [r] dz  [double]   dz
**
** @return [void]
************************************************************************/
static void mapdatum_molodensky(double Sphi, double Slam, double SH, double Sa,
			 double Sif, double *Dphi, double *Dlam,
			 double *DH, double Da, double Dif, double dx,
			 double dy, double dz)
{
    double Sf;
    double Df;
    double esq;
    double bda;
    double da;
    double df;
    double N;
    double M;
    double tmp;
    double tmp2;
    double dphi;
    double dlambda;
    double dheight;
    double phis;
    double phic;
    double lams;
    double lamc;
    
    Sf = (double)1.0 / Sif;
    Df = (double)1.0 / Dif;
    
    esq = (double)2.0*Sf - pow(Sf,(double)2.0);
    bda = (double)1.0 - Sf;
    Sphi = deg_to_rad(Sphi);
    Slam = deg_to_rad(Slam);
    
    da = Da - Sa;
    df = Df - Sf;

    phis = sin(Sphi);
    phic = cos(Sphi);
    lams = sin(Slam);
    lamc = cos(Slam);
    
    N = Sa /  sqrt((double)1.0 - esq*pow(phis,(double)2.0));
    
    tmp = ((double)1.0-esq) /pow(((double)1.0-esq*pow(phis,(double)2.0)),1.5);
    M   = Sa * tmp;

    tmp  = df * ((M/bda)+N*bda) * phis * phic;
    tmp2 = da * N * esq * phis * phic / Sa;
    tmp2 += ((-dx*phis*lamc-dy*phis*lams) + dz*phic);
    dphi = (tmp2 + tmp) / (M + SH);
    
    dlambda = (-dx*lams+dy*lamc) / ((N+SH)*phic);

    dheight =	dx*phic*lamc + dy*phic*lams + dz*phis - da*(Sa/N) +
				df*bda*N*phis*phis;
    
    *Dphi = Sphi + dphi;
    *Dlam = Slam + dlambda;
    *DH   = SH   + dheight;
    
    *Dphi = rad_to_deg(*Dphi);
    *Dlam = rad_to_deg(*Dlam);
}

map_datum*	mapdatums_list(int *count=0)
{
    if (count) *count=map_datums_found;
    return map_datums_params;
}
