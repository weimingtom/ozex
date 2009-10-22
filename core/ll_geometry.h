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

#ifndef __VDF_INCLUDED
#define	__VDF_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

void llbd_to_ll(	double lat0, double lon0, double brng, double dist,
					double &lat1, double &lon1);

double ll_to_distance(double lt1, double lg1, double lt2, double lg2);

double ll_to_bearing(double lt1, double lg1, double lt2, double lg2);

double bearing_to_direction(double degs);

#ifdef __cplusplus
};
#endif

#endif

