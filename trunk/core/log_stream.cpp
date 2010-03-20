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

#include "log_stream.h"

/*--------------------------------------------------------------------------*/
static FILE* logstream = NULL;

/*--------------------------------------------------------------------------*/
void logstream_to(FILE* f)
{
	logstream = f;
}

/*--------------------------------------------------------------------------*/
void logstream_write(const char *fmt, ...)
{
	if (logstream)
	{
        va_list ap;

        va_start(ap, fmt);
//        fprintf(log, "%d|", line);

        vfprintf(logstream, fmt, ap);

        va_end(ap);
		
		fflush(logstream);
	}
}
