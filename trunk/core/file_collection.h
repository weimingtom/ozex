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

#ifndef __FILE_COLLECTION_INCLUDED
#define __FILE_COLLECTION_INCLUDED

#include <stdio.h>
#ifndef UNDER_CE
#include <sys/types.h>
#include <errno.h>
#endif
#include <string.h>
#include <stdlib.h>

#define	FC_LIST_GROW_SIZE	128

/*--------------------------------------------------------------------------*/
typedef struct
{
	unsigned long	files_amount;
	unsigned long	storage_size;
	char**			files;
} file_collection;


#ifdef __cplusplus
extern "C" {
#endif

file_collection* filecollection_init();
void filecollection_forget(file_collection* fc);
void filecollection_collect(file_collection* fc, char* path, char* ext);

#ifdef __cplusplus
};
#endif

#endif