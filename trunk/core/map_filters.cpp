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

#include "map_filters.h"

/*--------------------------------------------------------------------------*/
typedef struct
{
	Uint8 r;
	Uint8 g;
	Uint8 b;
	Uint8 a;
} rgba;

/*--------------------------------------------------------------------------*/
void map_negative(image_rgb* src)
{
	rgba* p = (rgba*)src->m_data;
	
	for (int i = 0; i < src->m_width * src->m_height; i ++)
	{
		p->r = (Uint8)(255 - p->r);
		p->g = (Uint8)(255 - p->g);
		p->b = (Uint8)(255 - p->b);
		p++;
	} 
}


/*--------------------------------------------------------------------------*/
void map_zoom(image_rgb* src, image_rgb* dst)
{
    int x, y, sx, sy, *sax, *say, *csax, *csay, csx, csy, ex, ey, t1, t2;
    rgba *c00, *c01, *c10, *c11, *sp, *csp, *dp;
    int sgap, dgap;

    /* For interpolation: assume source dimension is one pixel */
    /* smaller to avoid overflow on right and bottom edge.     */
    sx = (int) (65536.0 * (double) (src->m_width - 1) / (double) dst->m_width);
    sy = (int) (65536.0 * (double) (src->m_height - 1) / (double) dst->m_height);

    /* Allocate memory for row increments */
    sax = (int*) alloca ((dst->m_width + 1) * sizeof (Uint32));
    say = (int*) alloca ((dst->m_height + 1) * sizeof (Uint32));

    /* Precalculate row increments */
    csx = 0;
    csax = sax;
    for (x = 0; x <= dst->m_width; x++)
	{
	    *csax = csx;
	    csax++;
	    csx &= 0xffff;
	    csx += sx;
	}
    csy = 0;
    csay = say;
    for (y = 0; y <= dst->m_height; y++)
	{
	    *csay = csy;
	    csay++;
	    csy &= 0xffff;
	    csy += sy;
	}

    /* Pointer setup */
    sp = csp = (rgba *) src->m_data;
    dp = (rgba *) dst->m_data;
    sgap = src->m_pitch - src->m_width * 4;
    dgap = dst->m_pitch - dst->m_width * 4;

    /* Interpolating Zoom */
    /* Scan destination */
    csay = say;
    for (y = 0; y < dst->m_height; y++)
	{
	    /* Setup color source pointers */
	    c00 = csp;
	    c01 = csp;
	    c01++;
	    c10 = (rgba *) ((Uint8 *) csp + src->m_pitch);
	    c11 = c10;
	    c11++;
	    csax = sax;
	    for (x = 0; x < dst->m_width; x++)
		{
		    /* ABGR ordering */
		    /* Interpolate colors */
		    ex = (*csax & 0xffff);
		    ey = (*csay & 0xffff);
		    t1 = ((((c01->r - c00->r) * ex) >> 16) + c00->r) & 0xff;
		    t2 = ((((c11->r - c10->r) * ex) >> 16) + c10->r) & 0xff;
		    dp->r = (((t2 - t1) * ey) >> 16) + t1;
		    t1 = ((((c01->g - c00->g) * ex) >> 16) + c00->g) & 0xff;
		    t2 = ((((c11->g - c10->g) * ex) >> 16) + c10->g) & 0xff;
		    dp->g = (((t2 - t1) * ey) >> 16) + t1;
		    t1 = ((((c01->b - c00->b) * ex) >> 16) + c00->b) & 0xff;
		    t2 = ((((c11->b - c10->b) * ex) >> 16) + c10->b) & 0xff;
		    dp->b = (((t2 - t1) * ey) >> 16) + t1;
		    t1 = ((((c01->a - c00->a) * ex) >> 16) + c00->a) & 0xff;
			t2 = ((((c11->a - c10->a) * ex) >> 16) + c10->a) & 0xff;
		    dp->a = (((t2 - t1) * ey) >> 16) + t1;

		    /* Advance source pointers */
		    csax++;
		    int sstep = (*csax >> 16);
		    c00 += sstep;
		    c01 += sstep;
		    c10 += sstep;
		    c11 += sstep;
		    /* Advance destination pointer */
		    dp++;
		}
	    /* Advance source pointer */
	    csay++;
	    csp = (rgba *) ((Uint8 *) csp + (*csay >> 16) * src->m_pitch);
	    /* Advance destination pointers */
	    dp = (rgba *) ((Uint8 *) dp + dgap);
	}
}
