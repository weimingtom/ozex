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

#include "file_collection.h"
#include "ul.h"
#include "log_stream.h"

/*--------------------------------------------------------------------------*/
unsigned char filecollection_tolower(unsigned char c)
{
	if ((c >= 'A') && (c <= 'Z'))
		return (unsigned char)(c + 0x20);
	
	return c;
}

/*--------------------------------------------------------------------------*/
int filecollection_stricmp(const char *s1, const char *s2)
{
	while (	filecollection_tolower((unsigned char)*s1) == 
			filecollection_tolower((unsigned char)*s2))
	{
		if (*s1 == 0)
			return 0;
		
		s1++;
		s2++;
	}
  
	return	(int)filecollection_tolower((unsigned char)*s1) - 
			(int)filecollection_tolower((unsigned char)*s2);
}

/*--------------------------------------------------------------------------*/
file_collection* filecollection_init()
{
	file_collection* fc = NULL;
	
	fc = (file_collection*)malloc(sizeof(file_collection));
	
	memset(fc, 0, sizeof(file_collection));
	fc->storage_size = FC_LIST_GROW_SIZE;
	fc->files = (char**)malloc(sizeof(char*) * fc->storage_size);
	
	return fc;
}

/*--------------------------------------------------------------------------*/
void filecollection_addfile(file_collection* fc, char* file)
{
	if (fc->files_amount > fc->storage_size - 1)
	{
		char** files = NULL;
		fc->storage_size += FC_LIST_GROW_SIZE;
		files = (char**)malloc(sizeof(char*) * fc->storage_size);
		memcpy(files, fc->files, sizeof(char*) * fc->files_amount);
		free(fc->files);
		fc->files = files;
	}
	
	fc->files[fc->files_amount] = (char*)malloc(strlen(file) + 1);
	strcpy(fc->files[fc->files_amount], file);
	fc->files_amount++;
}

/*--------------------------------------------------------------------------*/
void filecollection_forget(file_collection* fc)
{
	int i = 0;
	
	for (i = 0; i < fc->files_amount; i++ )
		free(fc->files[i]);
	
	free(fc->files);
	free(fc);
}

/*--------------------------------------------------------------------------*/
void filecollection_collect(file_collection* fc, char* path, char* ext)
{
    ulDir *folder;
    struct ulDirEnt* folder_data;

    folder = ulOpenDir(path);

	logstream_write("collecting files with .ext %s\n", ext);

    if ( folder ) 
	{
		folder_data = ulReadDir( folder );
		
		while ( folder_data ) 
		{
			if(	strcmp(folder_data->d_name, ".") &&
				strcmp(folder_data->d_name, ".."))
			{		
				if ( folder_data->d_isdir )
				{
					char local_path[UL_NAME_MAX];
					sprintf(local_path, "%s/%s", path, folder_data->d_name );
					
					filecollection_collect(fc, local_path, ext);
				}
				else
				{
					int l1 = strlen(ext);
					int l2 = strlen(folder_data->d_name);
					
					char* s = &folder_data->d_name[l2 - l1];

					if ( !filecollection_stricmp(s, ext) )
					{
						char local_path[UL_NAME_MAX];
						sprintf(local_path, "%s/%s", path, folder_data->d_name );
												
						filecollection_addfile(fc, local_path);
					}
				}
			}
			
			folder_data = ulReadDir( folder );
		}

		ulCloseDir( folder );
    }
}
