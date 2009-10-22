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

#ifndef __MAP_FILTERS_INCLUDED
#define __MAP_FILTERS_INCLUDED

#include <SDL.h>

/*--------------------------------------------------------------------------*/
typedef struct tagimage_rgb
{
	Uint32 m_width;
	Uint32 m_height;
	Uint8* m_data;
	Uint32 m_pitch;
}image_rgb;

/*--------------------------------------------------------------------------*/
void map_zoom(image_rgb* src, image_rgb* dst);
void map_negative(image_rgb* src);


#endif