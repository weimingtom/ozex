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

#ifndef __WPT_CONTAINER_INCLUDED
#define __WPT_CONTAINER_INCLUDED

/*--------------------------------------------------------------------------*/
typedef struct 
{
	char			name[128];
	unsigned long	time;

	double			lat;
	double			lon;
	long			alt;
	
	int				deleted;
	int				added;
} waypoint;


class wpt_container
{
	public:
		wpt_container();
		~wpt_container();

		char*	point_name(int point);
		int		points_avail();
		void	point_coords(int point, double* lat, double* lon);
		int		point_active(int point);
		int		point_delete(int point);
		void	point_add(char* name, double lat, double lon);
		int		changed();
		int		added(int i);

		void	load(const char* filename);
		void	save(const char* filename);
	
	private:

		int	numpoints;
		waypoint* points;	
		int	unsaved;
};


#endif
