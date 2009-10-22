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

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef WIN32
#include <malloc.h>
#endif

#include "map_loader.h"
#include "map_datums.h"

#include "tranmerc.h"

#include "log_stream.h"

#define STRING_MAX	1024

#define absd(x) (((x) < 0) ? (-(x)) : (x))

/*--------------------------------------------------------------------------*/
typedef struct 
{
	long 	x;
	long 	y;

	double  lat_deg;
	double  lat_min;
	char	lat_hemisphere;

	double  lon_deg;
	double  lon_min;
	char	lon_hemisphere;

	long	northing;
	long	easting;
} CalibrationPoint;


/*--------------------------------------------------------------------------*/
double dms_to_deg(double deg, double min, double sec)
{
	double d = deg, m = min, s = sec;
	return d + m/60 + s/3600;
}

/*--------------------------------------------------------------------------*/
void deg_to_dms(double dec, double* deg, double* min, double* sec)
{
	double frac;
	long sign  = dec < 0 ? -1 : 1;
	*deg = floor(absd(dec));

	frac = absd(dec) - (double)*deg;
	frac *= 60;

	*min = floor(absd(frac));

	frac = absd(frac) - (double)*min;
	frac *= 60;

	*sec = floor(absd(frac));
}


/*--------------------------------------------------------------------------*/
long NoNumbers(char *s)
{
	char* p = s;

	while ( *p )
	{
		if ( *p >= '0' && *p <= '9' )
			return 0;
		p++;
	}

	return 1;
}



/*--------------------------------------------------------------------------*/
static const double pi = 3.14159265358979323846;

static double deg2rad(double deg)
{
	return deg * pi / 180.0;
}

static double rad2deg(double rad)
{
	return rad * 180.0 / pi;
}

/*--------------------------------------------------------------------------*/
long maploader_parse_point(map_info* mi, char* pszPoint, LLXY* pPoint)
{
	CalibrationPoint cPoint;

	memset(&cPoint, 0, sizeof(cPoint));

	long		n;

	char* pszThis = pszPoint;
	char* pszTail = NULL;

	pszThis = strchr(pszThis, ',');     // skipped PointNN
	if ( !pszThis )
		return 0;

	pszThis++;
	pszThis = strchr(pszThis, ',');     // skipped xy
	if ( !pszThis )
		return 0;

	pszThis++;
	pszTail = strchr(pszThis, ',');     // got x
	if ( !pszTail )
		return 0;

	*pszTail = 0;

	if ( NoNumbers(pszThis) )
		return 0;

	n  = atol(pszThis);

	cPoint.x = n;

	pszTail++;
	pszThis = pszTail;
	pszTail = strchr(pszThis, ',');     // got y
	if ( !pszTail )
		return 0;

	*pszTail = 0;

	n  = atol(pszThis);

	cPoint.y = n;

	pszTail++;

	pszThis = pszTail;

	if(!strncmp(pszThis, "ex", 2))
		return 0;

	pszThis = strchr(pszThis, ',');     // skipped in
	if ( !pszThis )
		return 0;


	pszThis++;
	pszThis = strchr(pszThis, ',');     // skipped deg
	if ( !pszThis )
		return 0;

	pszThis++;
	pszTail = strchr(pszThis, ',');     // got lat deg
	if ( !pszTail )
		return 0;

	*pszTail = 0;

	n  = atol(pszThis);

	cPoint.lat_deg = n;

	pszTail++;
	pszThis = pszTail;
	pszTail = strchr(pszThis, ',');     // got lat min
	if ( !pszTail )
		return 0;

	cPoint.lat_min = atof(pszThis);

	pszTail++;
	pszThis = pszTail;
	pszTail = strchr(pszThis, ',');     // got lat type N/S
	if ( !pszTail )
		return 0;

	cPoint.lat_hemisphere = *pszThis;

	// longitude

	pszTail++;
	pszThis = pszTail;
	pszTail = strchr(pszThis, ',');     // got lat deg
	if ( !pszTail )
		return 0;

	*pszTail = 0;

	n  = atol(pszThis);

	cPoint.lon_deg = n;

	pszTail++;
	pszThis = pszTail;
	pszTail = strchr(pszThis, ',');     // got lat min
	if ( !pszTail )
		return 0;

	cPoint.lon_min = atof(pszThis);

	pszTail++;
	pszThis = pszTail;
	pszTail = strchr(pszThis, ',');     // got lat type N/S
	if ( !pszTail )
		return 0;

	cPoint.lon_hemisphere = *pszThis;

	pszTail++;
	pszThis = pszTail;
	pszTail = strchr(pszThis, ',');     // skipped grid
	if ( !pszTail )
		return 0;

	*pszTail = 0;

	pszTail++;
	pszThis = pszTail;
	pszTail = strchr(pszThis, ',');     // skipped space
	if ( !pszTail )
		return 0;

	*pszTail = 0;

	pszTail++;
	pszThis = pszTail;
	pszTail = strchr(pszThis, ',');     // skipped space
	if ( !pszTail )
		return 0;

	*pszTail = 0;

	cPoint.easting = atol(pszThis);

	pszTail++;
	pszThis = pszTail;
	pszTail = strchr(pszThis, ',');     // skipped space
	if ( !pszTail )
		return 0;

	*pszTail = 0;

	cPoint.northing = atol(pszThis);

	pPoint->x		= cPoint.x;
	pPoint->y		= cPoint.y;

	pPoint->lat = 	dms_to_deg(cPoint.lat_deg,
								cPoint.lat_min,
								0);

	pPoint->lon = 	dms_to_deg(cPoint.lon_deg,
								cPoint.lon_min,
								0);
								
	if (cPoint.lat_hemisphere == 'S') 							
		pPoint->lat = -pPoint->lat;

	if (cPoint.lon_hemisphere == 'W') 							
		pPoint->lon = -pPoint->lon;

	pPoint->n = cPoint.northing;
	pPoint->e = cPoint.easting;
	
	return 1;
}

/*--------------------------------------------------------------------------*/
void maploader_add_anchor(map_info* mi, LLXY* pPoint)
{
	if (mi->calibrations_amount < CALIBRATION_POINTS_MAX - 1)
	{
		memcpy(&mi->calibrations[mi->calibrations_amount], 
				pPoint, sizeof(LLXY));

		mi->calibrations_amount++;
	}
}

/*--------------------------------------------------------------------------*/
long maploader_parse_imagesize(map_info* mi, char* pszPoint)
{
	long		n;

	char* pszThis = pszPoint;
	char* pszTail = NULL;

	pszThis = strchr(pszThis, ',');     // skipped PointNN
	if ( !pszThis )
		return 0;

	pszThis++;
	pszThis = strchr(pszThis, ',');     // skipped xy
	if ( !pszThis )
		return 0;

	pszThis++;
	pszTail = strchr(pszThis, ',');     // got lat min
	if ( !pszTail )
		return 0;

	if ( NoNumbers(pszThis) )
		return 0;

	n  = atol(pszThis);
	mi->width = n;

	pszTail++;
	pszThis = pszTail;
	if ( NoNumbers(pszThis) )
		return 0;

	n  = atol(pszThis);

	mi->height = n;

	return 1;
}

/*--------------------------------------------------------------------------*/
long maploader_parse_corner_xy(map_info* mi, char* pszString)
{
	long		n;

	char* pszThis = pszString;
	char* pszTail = NULL;

	pszThis = strchr(pszThis, ',');     // skipped MMPXY
	if ( !pszThis )
		return 0;

	pszThis++;
	pszTail = strchr(pszThis, ',');     // got corner No
	if ( !pszTail )
		return 0;

	if ( NoNumbers(pszThis) )
		return 0;

	n  = atol(pszThis);
	long i = n-1;

	pszTail++;
	pszThis = pszTail;
	pszTail = strchr(pszThis, ',');     // got x
	if ( !pszTail )
		return 0;

	if ( NoNumbers(pszThis) )
		return 0;

	n  = atol(pszThis);
	mi->corners[i].x = n;

	pszTail++;
	pszThis = pszTail;

	if ( NoNumbers(pszThis) )
		return 0;

	n  = atol(pszThis);
	mi->corners[i].y = n;

	return 1;
}

/*--------------------------------------------------------------------------*/
long maploader_parse_corners_amount(map_info* mi, char* pszString)
{
	char *pszThis = strchr(pszString, ',');     // skipped MMPNUM
	if ( !pszThis )
		return 0;

	pszThis++;

	if ( NoNumbers(pszThis) )
		return 0;

	long n  = atol(pszThis);

	mi->corners_amount	= n;
	mi->corners			= (LLXY*)malloc(n*sizeof(LLXY));

	return 1;
}

/*--------------------------------------------------------------------------*/
long maploader_parse_corner_latlon(map_info* mi, char* pszString)
{
	long		n;

	char* pszThis = pszString;
	char* pszTail = NULL;

	pszThis = strchr(pszThis, ',');     // skipped MMPLL
	if ( !pszThis )
		return 0;

	pszThis++;
	pszTail = strchr(pszThis, ',');     // got corner No
	if ( !pszTail )
		return 0;

	if ( NoNumbers(pszThis) )
		return 0;

	n  = atol(pszThis);
	long i = n-1;

	pszTail++;
	pszThis = pszTail;
	pszTail = strchr(pszThis, ',');     // got lon
	if ( !pszTail )
		return 0;

	if ( NoNumbers(pszThis) )
		return 0;

	mi->corners[i].lon = atof(pszThis);

	pszTail++;
	pszThis = pszTail;

	if ( NoNumbers(pszThis) )	// got lat
		return 0;

	mi->corners[i].lat = atof(pszThis);

	return 1;
}

/*--------------------------------------------------------------------------*/
long maploader_parse_projection(map_info* mi, char* pszString)
{
	char* pszThis = pszString;
	char* pszTail = NULL;

	pszThis = strchr(pszThis, ',');     // skipped "Map Projection"
	if ( !pszThis )
		return 0;

	pszThis++;
	pszTail = strchr(pszThis, ',');     // got projection
	if ( !pszTail )
		return 0;

	*pszTail = 0;

	mi->projection = (char*)malloc(strlen(pszThis) + 1);
	strcpy(mi->projection, pszThis);
	
	return 1;
}

/*--------------------------------------------------------------------------*/
long maploader_parse_projection_params(map_info* mi, char* pszString)
{
	char* pszThis = pszString;
	char* pszTail = NULL;

	//mi->projection_params.ellipsoid_code = 0;

	pszThis = strchr(pszThis, ',');     // skipped "Projection Setup"
	if ( !pszThis )
		return 0;

	pszThis++;
	pszTail = strchr(pszThis, ',');     // got next param
	if ( !pszTail )
		return 0;

	*pszTail = 0;

	if ( NoNumbers(pszThis) )
		return 0;

	mi->projection_params.origin_latitude = atof(pszThis) * (pi / 180.0);

	pszThis = pszTail;

	pszThis++;
	pszTail = strchr(pszThis, ',');     // got next param
	if ( !pszTail )
		return 0;

	*pszTail = 0;

	if ( NoNumbers(pszThis) )
		return 0;

	mi->projection_params.central_meridian = atof(pszThis) * (pi / 180.0);

	pszThis = pszTail;

	pszThis++;
	pszTail = strchr(pszThis, ',');     // got next param
	if ( !pszTail )
		return 0;

	*pszTail = 0;

	if ( NoNumbers(pszThis) )
		return 0;

	mi->projection_params.scale_factor = atof(pszThis);

	pszThis = pszTail;

	pszThis++;
	pszTail = strchr(pszThis, ',');     // got next param
	if ( !pszTail )
		return 0;

	*pszTail = 0;

	if ( NoNumbers(pszThis) )
		return 0;

	mi->projection_params.false_easting = atof(pszThis);

	pszThis = pszTail;

	pszThis++;
	pszTail = strchr(pszThis, ',');     // got next param
	if ( !pszTail )
		return 0;

	*pszTail = 0;

	if ( NoNumbers(pszThis) )
		return 0;

	mi->projection_params.false_northing = atof(pszThis);

	pszThis = pszTail;


	pszThis++;
	pszTail = strchr(pszThis, ',');     // got next param
	if ( !pszTail )
		return 0;

	*pszTail = 0;

	if ( NoNumbers(pszThis) )
		return 0;

	mi->projection_params.latitude_1 = atof(pszThis) * (pi / 180.0);

	pszThis = pszTail;

	pszThis++;
	pszTail = strchr(pszThis, ',');     // got next param
	if ( !pszTail )
		return 0;

	*pszTail = 0;

	if ( NoNumbers(pszThis) )
		return 0;

	mi->projection_params.latitude_2 = atof(pszThis) * (pi / 180.0);

	pszThis = pszTail;

	return 1;
}

/*--------------------------------------------------------------------------*/
long maploader_parse_meters_perpixel(map_info* mi, char* pszString)
{
	char* pszThis = pszString;
	char* pszTail = NULL;

	pszThis = strchr(pszThis, ',');     // skipped "MM1B"
	if ( !pszThis )
		return 0;

	pszThis++;
	
	mi->mpp = atof(pszThis);

	return 1;
}


/*--------------------------------------------------------------------------*/
void maploader_setup_datum(map_info* mi)
{
	map_datum* params =	mapdatums_find(mi->datum);

	mi->projection_params.a = params->a;
	mi->projection_params.f = 1 / params->invf;
	
	mi->projection_params.ellipsoid_code = 
		(char*)malloc(strlen(params->name) + 1);
				
	strcpy(mi->projection_params.ellipsoid_code, params->ellipsoid);
}

/* some maps in Transverse Mercator have calibration points in n/e rather lat/lon */
/*--------------------------------------------------------------------------*/
void maploader_fix_tranmerc(map_info* mi)
{
	if (!strcmp(mi->projection, "Transverse Mercator"))
	{
		int i;
		
		for (i = 0; i < mi->calibrations_amount; i++)
		{
		
			if (mi->calibrations[i].n != 0 &&
				mi->calibrations[i].e != 0)
	
			{
				double lat = 0; 
				double lon = 0;
						
				Set_Transverse_Mercator_Parameters(	mi->projection_params.a,
													mi->projection_params.f,
													deg2rad(mi->projection_params.origin_latitude),
													mi->projection_params.central_meridian,
													mi->projection_params.false_easting,
													mi->projection_params.false_northing,
													mi->projection_params.scale_factor);
	
				Convert_Transverse_Mercator_To_Geodetic (	mi->calibrations[i].e,
															mi->calibrations[i].n,
															&lat,
															&lon);
																							
				mi->calibrations[i].lat = rad2deg(lat);
				mi->calibrations[i].lon = rad2deg(lon);
			}		
		}
	}
}

/*--------------------------------------------------------------------------*/
void maploader_parse_imagefile(map_info* mi, char* s)
{
	char* p = s + strlen(s);
	
	while(1)
	{
		if (p == s)
			break;
			
		if(*p == '\n' || *p == '\r')
		{
			*p = 0;
		}	
		else		
		if(*p == '/' || *p == '\\')
		{
			p++;
			break;
		}	
		
		p--;
	}

	double lat = 0; 
	double lon = 0;
	
	mi->image_file = (char*)malloc(strlen(p) + 1);
	strcpy(mi->image_file, p);
}


/*--------------------------------------------------------------------------*/
void maploader_parse_imagepath(map_info* mi, char* file)
{
	char* tmp = (char*)alloca(strlen(file) + 1);
	strcpy(tmp, file);
	
	char* p = tmp + strlen(tmp);
	
	while(1)
	{
		if (p == tmp)
			break;
		
		if(*p == '/' || *p == '\\')
		{
			p++;
			*p = 0;
			break;
		}	
		
		p--;
	}

	mi->image_path = (char*)malloc(strlen(tmp) + 1);
	strcpy(mi->image_path, tmp);
}

/*--------------------------------------------------------------------------*/
void maploader_fixcoords(map_info* mi)
{
	int i = 0;

	map_datum* params =	mapdatums_find(mi->datum);
												
	// converting map datum to WGS84		
	for ( i = 0; i < mi->calibrations_amount; i++ )
	{
		double height;

		mapdatum_to_wgs84(	mi->calibrations[i].lat,
							mi->calibrations[i].lon,
							0,
							&mi->calibrations[i].lat,
							&mi->calibrations[i].lon,
							&height, params);
	
	}

	// converting map datum to WGS84		
	for ( i = 0; i < mi->corners_amount; i++ )
	{
		double height;

		mapdatum_to_wgs84(	mi->corners[i].lat,
							mi->corners[i].lon,
							0,
							&mi->corners[i].lat,
							&mi->corners[i].lon,
							&height, params);
							
	}

	params =	mapdatums_find("WGS 84");

	if (mi->projection_params.ellipsoid_code)
		free(mi->projection_params.ellipsoid_code);

	mi->projection_params.a = params->a;
	mi->projection_params.f = 1.0 / params->invf;
		
	mi->projection_params.ellipsoid_code = 
		(char*)malloc(strlen(params->name) + 1);
				
	strcpy(mi->projection_params.ellipsoid_code, params->ellipsoid);

	strcpy(mi->datum, "WGS 84");
}


/*--------------------------------------------------------------------------*/
map_info* maploader_openmap(char* filename)
{
	map_info* mi = NULL;
	char pszString[STRING_MAX];
	
	FILE *fp = fopen(filename, "rt");

	if ( fp )
	{
		fgets( pszString, STRING_MAX, fp );

		char* s = strchr(pszString, ' ');

		if ( s )
		{
			*s = 0;

			if ( !strcmp(pszString, "OziExplorer") )
			{
				mi = (map_info*)malloc(sizeof(map_info));
				memset(mi, 0, sizeof(map_info));
				mi->calibrations = (LLXY*)malloc(CALIBRATION_POINTS_MAX * sizeof(LLXY));
				memset(mi->calibrations, 0, CALIBRATION_POINTS_MAX * sizeof(LLXY));
			
				fgets( pszString, STRING_MAX, fp );
				char* szEnd = strchr(pszString, '\r');
				if ( !szEnd )
					szEnd = strchr(pszString, '\n');

				if ( szEnd )
					*szEnd = 0;

				fgets( pszString, STRING_MAX, fp );  // got image path
				maploader_parse_imagefile(mi, pszString);

				maploader_parse_imagepath(mi, filename);

				fgets( pszString, STRING_MAX, fp );  // map code
				fgets( pszString, STRING_MAX, fp );  // datum
				szEnd = strchr(pszString, ',');

				*szEnd = 0;

				mi->datum = (char*)malloc(strlen(pszString) + 1);
				strcpy(mi->datum, pszString);
								
				maploader_setup_datum(mi);

				while ( 1 )
				{
					if (!fgets( pszString, STRING_MAX, fp ) ||
						!strlen(pszString))
						break;

					if ( !strncmp(pszString, "Point", strlen("Point")) )
					{
						LLXY	point;

						memset(&point, 0, sizeof(LLXY));

						if ( maploader_parse_point(mi, pszString, &point) )
							maploader_add_anchor(mi, &point);
					}
					else
					if ( !strncmp(pszString, "IWH", strlen("IWH")) )
					{
						maploader_parse_imagesize(mi, pszString);
					}
					else
					if ( !strncmp(pszString, "MMPNUM", strlen("MMPNUM")) )
					{
						maploader_parse_corners_amount(mi, pszString);
					}
					else
					if ( !strncmp(pszString, "MMPXY", strlen("MMPXY")) )
					{
						maploader_parse_corner_xy(mi, pszString);
					}
					else
					if ( !strncmp(pszString, "MMPLL", strlen("MMPLL")) )
					{
						maploader_parse_corner_latlon(mi, pszString);
					}
					else
					if ( !strncmp(pszString, "MM1B", strlen("MM1B")) )
					{
						maploader_parse_meters_perpixel(mi, pszString);
					}
					else
					if ( !strncmp(pszString, "Map Projection", strlen("Map Projection")) )
					{
						maploader_parse_projection(mi, pszString);
					}
					if ( !strncmp(pszString, "Projection Setup", strlen("Projection Setup")) )
					{
						maploader_parse_projection_params(mi, pszString);
					}
				}
				
				maploader_fix_tranmerc(mi);
				
				maploader_fixcoords(mi);
			}
		}

		fclose(fp);
	}
	
	return mi;
}

/*--------------------------------------------------------------------------*/
void maploader_debugmap(map_info* mi)
{
	int i;
	
	logstream_write("projection: %s\n",		mi->projection);
	logstream_write("datum: %s\n",			mi->datum);
	logstream_write("scale (mpp): %f\n",		mi->mpp);
	logstream_write("image width: %d\n",		mi->width);
	logstream_write("image height: %d\n",	mi->height);
	logstream_write("image file: %s\n",		mi->image_file);
	
	logstream_write("number of calibration points: %d\n",	mi->calibrations_amount);
	logstream_write("calibration points:\n");
	
	for (i = 0; i < mi->calibrations_amount; i++)
	{
		logstream_write("\t%02d: x: %d y: %d lat: %f lon: %f\n", i, 
				mi->calibrations[i].x, mi->calibrations[i].y,
				mi->calibrations[i].lat, mi->calibrations[i].lon);
	}

	logstream_write("number of corners: %d\n",	mi->corners_amount);
	logstream_write("corners:\n");
	
	for (i = 0; i < mi->corners_amount; i++)
	{
		logstream_write("\t%02d: x: %d y: %d lat: %f lon: %f\n", i, 
				mi->corners[i].x, mi->corners[i].y,
				mi->corners[i].lat, mi->corners[i].lon);
	}

}

/*--------------------------------------------------------------------------*/
void maploader_closemap(map_info* mi)
{
	if (mi)
	{
		if (mi->image_file)
			free(mi->image_file);

		if (mi->image_path)
			free(mi->image_path);

		if ( mi->projection_params.ellipsoid_code )
			free(mi->projection_params.ellipsoid_code);

		if ( mi->projection )
			free(mi->projection);
			
		if ( mi->datum )
			free(mi->datum);
	
		if ( mi->calibrations )
			free(mi->calibrations);
			
		if ( mi->corners )	
			free(mi->corners);
			
		free(mi);
	}
}
