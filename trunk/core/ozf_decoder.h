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

#ifndef __OZF_DECODER_INCLUDED
#define __OZF_DECODER_INCLUDED

/*--------------------------------------------------------------------------*/
#ifndef UNDER_CE
#include <sys/types.h>
#endif
#include <stdio.h>

#include "map_types.h"

#define	OZF_STREAM_DEFAULT		0
#define OZF_STREAM_ENCRYPTED	1

/*--------------------------------------------------------------------------*/
typedef struct
{
	long width;
	long height;
	short xtiles;
	short ytiles;

	long palette[256];
 } ozf_image_header;

/*--------------------------------------------------------------------------*/
typedef struct
{
	short magic;
	long dummy1;
	long dummy2;
	long dummy3;
	long dummy4;

	long width;
	long height;

	short depth;
	short bpp;

	long dummy5;

	long memsiz;

	long dummy6;
	long dummy7;
	long dummy8;
	long version;
 } ozf2_header;

/*--------------------------------------------------------------------------*/
typedef struct
{
	long size;
	long width;
	long height;
	short depth;
	short bpp;
} ozf3_header;

/*--------------------------------------------------------------------------*/
typedef struct
{
	ozf_image_header	header;
	
	unsigned long		tiles;
	unsigned long*		tiles_table;
	
	long				encryption_depth;
	
} ozf_image;

/*--------------------------------------------------------------------------*/
typedef struct 
{
	FILE*				file;
	int					type;
	unsigned long		key;
	unsigned long		size;

	unsigned long		scales;
	unsigned long*		scales_table;
	ozf_image*			images;
	
	ozf2_header*		ozf2;
	ozf3_header*		ozf3;
	
} ozf_stream;

/*--------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

void*		ozf_open(char* path);
void		ozf_get_tile(map_stream* s, int scale, int x, int y, unsigned char* data);
int			ozf_num_scales(map_stream* s);
int			ozf_num_tiles_per_x(map_stream*, int scale);
int			ozf_num_tiles_per_y(map_stream*, int scale);
int			ozf_scale_dx(map_stream*, int scale);
int			ozf_scale_dy(map_stream*, int scale);
void		ozf_close(map_stream*);

#ifdef __cplusplus
};
#endif

#endif
