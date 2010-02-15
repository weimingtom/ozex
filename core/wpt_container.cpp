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

#include "wpt_container.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include "log_stream.h"

#define STRING_MAX 1024

#define WAYPOINTS_MAX	2000

/*----------------------------------------------------------------------------*/
wpt_container::wpt_container()
{
	numpoints = 0;
	points = (waypoint*)malloc(WAYPOINTS_MAX*sizeof(waypoint));
	memset(points, 0, WAYPOINTS_MAX*sizeof(waypoint));
	unsaved = 0;
}


/*----------------------------------------------------------------------------*/
int	wpt_container::changed()
{
	return unsaved;
}

/*----------------------------------------------------------------------------*/
void	wpt_container::load(const char* file)
{
	numpoints = 0;

	free(points);

	char *cur_locale=strdup(setlocale(LC_NUMERIC,NULL));
	setlocale(LC_NUMERIC,"C");

	points = (waypoint*)malloc(WAYPOINTS_MAX*sizeof(waypoint));
	memset(points, 0, WAYPOINTS_MAX*sizeof(waypoint));
	unsaved =  0;

	char szDatum[256];
	char szTemp[STRING_MAX];
	
	FILE *fp = fopen(file, "rt");
	
	if (!fp)
	{
		logstream_write("failed to open waypoint file\n");
		return;
	}

	fgets(szTemp, STRING_MAX, fp);
	szTemp[strlen(szTemp)-1] = 0;
	
	if ( !strncmp(szTemp, "OziExplorer Waypoint File", strlen("OziExplorer Waypoint File")) )
	{
		waypoint wpt;

		fgets(szTemp, STRING_MAX, fp);
		szTemp[strlen(szTemp)-1] = 0;

		strcpy(szDatum, szTemp);

		fgets(szTemp, STRING_MAX, fp);      // skipping
		fgets(szTemp, STRING_MAX, fp);      // skipping

		while ( 1 )
		{
			if(!fgets(szTemp, STRING_MAX, fp))
				break;

			memset(&wpt, 0, sizeof(waypoint));
			szTemp[strlen(szTemp)-1] = 0;

//			printf("i: %d\n", numpoints);

			char* pszThis;
			char* pszThat;

			// skipping number - for Lowrance/Eagles and Silva GPS receivers
			pszThis	= szTemp;
			pszThat	= strchr(pszThis, ',');

//			printf("1\n");

			// reading Name - the waypoint name
			pszThis	= pszThat + 1;
			pszThat = strchr(pszThis, ',');
			*pszThat = 0;
			strcpy(wpt.name, pszThis);
			
			int k = strlen(wpt.name)-1;
			
			while(wpt.name[k] == 0x20 && k)
			{
				wpt.name[k] = 0;
				k--;
			}
			
//			printf("wpt name: '%s'\n", wpt.name);

//			printf("2\n");

			// reading latitude
			pszThis	= pszThat + 1;
			pszThat = strchr(pszThis, ',');
			*pszThat = 0;

			wpt.lat = atof(pszThis);

//			printf("3\n");

			// reading longitude
			pszThis	= pszThat + 1;
			pszThat = strchr(pszThis, ',');
			*pszThat = 0;

			wpt.lon = atof(pszThis);

//			printf("4\n");

			// reading time
			pszThis	= pszThat + 1;
			pszThat = strchr(pszThis, ',');
			*pszThat = 0;

//			printf("5\n");


			double pascalDateTime = atof(pszThis);
        	time_t unixDateTime = (pascalDateTime - ((70*365L)+17 + 2)) * (24L*60*60);
			wpt.time = unixDateTime;

			// skipping 10 fields

//			printf("6\n");

#if 0
			pszThis	= pszThat + 1;
			for ( long i = 0; i < 9; i++ )
			{
				pszThat	= strchr(pszThis, ',');
				pszThis	= pszThat + 1;
			}

			pszThat = strchr(pszThis, ',');
			*pszThat = 0;

			wpt.alt = atol(pszThis);

			double height;
			
//			printf("wpt: %s: %f, %f\n", wpt.name, wpt.lat, wpt.lon);
#endif			
			if(numpoints >= WAYPOINTS_MAX - 1)
				break;

			memcpy(&points[numpoints], &wpt, sizeof(waypoint));
			points[numpoints].deleted = 0;
			points[numpoints].added = 0;
			
			numpoints++;

		}
	}
	setlocale(LC_NUMERIC,cur_locale);
	free(cur_locale);
	
	fclose(fp);
}

/*----------------------------------------------------------------------------*/
char*	wpt_container::point_name(int point)
{
	if (point < numpoints)
	{
		return points[point].name;
	}
	
	return 0;
}

/*----------------------------------------------------------------------------*/
int	wpt_container::point_active(int point)
{
	if (point < numpoints)
	{
		return points[point].deleted^1;
	}
	
	return 0;
}

/*----------------------------------------------------------------------------*/
int	wpt_container::point_delete(int point)
{
	if (point < numpoints)
	{
		points[point].deleted = 1;
		unsaved = 1;
	}
}

/*----------------------------------------------------------------------------*/
void wpt_container::point_add(char* name, double lat, double lon)
{
	if(numpoints < WAYPOINTS_MAX - 1)
	{
		strcpy(points[numpoints].name, name);
		points[numpoints].lat = lat;
		points[numpoints].lon = lon;
		points[numpoints].added = 1;
		numpoints++;
		unsaved = 1;
	}
	else
	{
		// replacing existing deleted waypoint
		for (int i = 0; i < numpoints; i++)
		{
			if (points[i].deleted)
			{
				memset(&points[i], 0, sizeof(waypoint));
				strcpy(points[i].name, name);
				points[i].lat = lat;
				points[i].lon = lon;
				points[numpoints].added = 1;

				unsaved = 1;
			}
		}
	}
}

/*----------------------------------------------------------------------------*/
int	wpt_container::points_avail()
{
	return numpoints;
}

/*----------------------------------------------------------------------------*/
int wpt_container::added(int point)
{
	if (point < numpoints)
	{
		return 
		points[point].added;
	}
	
	return 0;
}

/*----------------------------------------------------------------------------*/
void wpt_container::point_coords(int point, double* lat, double* lon)
{
	if (point < numpoints)
	{
		*lat = points[point].lat;
		*lon = points[point].lon;
	}
}

/*----------------------------------------------------------------------------*/
void wpt_container::save(const char* filename)
{
	FILE* fp = fopen(filename, "wt");

	if (!fp)
		return;

	char *cur_locale=strdup(setlocale(LC_NUMERIC,NULL));
	setlocale(LC_NUMERIC,"C");

	int j = 1;	

	fprintf(fp, "OziExplorer Waypoint File Version 1.1\r\n");
	fprintf(fp, "WGS 84\r\n");
	fprintf(fp, "Reserved 2\r\n");
	fprintf(fp, "magellan\r\n");

	for (int i = 0; i < numpoints; i++)
	{
		if (!points[i].deleted)
		{
			fprintf(fp, "%d,%s,  %2.6f,  %2.6f, ,0, 0, 3,         0,     65535,, 0, 0,    0,   -777, 6, 0,17,0,10.0,2,,,\r\n", 
			j, points[i].name, points[i].lat, points[i].lon);

			j++;
		}
	}

	fclose(fp);

	setlocale(LC_NUMERIC,cur_locale);
	free(cur_locale);

	unsaved = 0;
}
		
/*----------------------------------------------------------------------------*/
wpt_container::~wpt_container()
{
	if (points)
		free(points);
}


