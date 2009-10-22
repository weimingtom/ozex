// Ozi Explorer streams decoding routines
// Author: Daniil Smelov <dn.smelov@gmail.com>
// Copyright Daniil Smelov 2006-2009


#ifndef __IMG_DECODER_INCLUDED
#define __IMG_DECODER_INCLUDED

/*--------------------------------------------------------------------------*/
#ifndef UNDER_CE
#include <sys/types.h>
#endif
#include <stdio.h>
#include "map_types.h"
#include <SDL.h>


/*--------------------------------------------------------------------------*/
typedef struct 
{
	SDL_Surface*		image;
} img_stream;

/*--------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

map_stream*	img_open(char* path);
void		img_get_tile(map_stream* s, int scale, int x, int y, unsigned char* data);
int			img_num_scales(map_stream* s);
int			img_num_tiles_per_x(map_stream* s, int scale);
int			img_num_tiles_per_y(map_stream* s, int scale);
int			img_scale_dx(map_stream* s, int scale);
int			img_scale_dy(map_stream* s, int scale);
void		img_close(map_stream*);

#ifdef __cplusplus
};
#endif

#endif
