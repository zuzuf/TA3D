/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2006  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/

#include "stdafx.h"
#include "TA3D_NameSpace.h"
#if defined TA3D_PLATFORM_LINUX || defined TA3D_PLATFORM_DARWIN
#   include <sys/stat.h>
#endif
#include "misc/paths.h"

// global variables:
TA3D::TA3DCONFIG		*TA3D::VARS::lp_CONFIG = NULL;
TA3D::GFX*              TA3D::VARS::gfx = NULL;						// The gfx object we will use to draw basic things and manage fonts, textures, ...
SDL_Color				*TA3D::VARS::pal = NULL;
uint8					TA3D::VARS::unit_engine_thread_sync;
uint8					TA3D::VARS::weapon_engine_thread_sync;
uint8					TA3D::VARS::particle_engine_thread_sync;
uint8					TA3D::VARS::players_thread_sync;
ObjectSync				*TA3D::VARS::ThreadSynchroniser = NULL;
String					TA3D::VARS::TA3D_CURRENT_MOD="";		// This string stores the path to current mod
SDL_Surface             *TA3D::VARS::screen = NULL;



namespace TA3D
{


	FILE *TA3D_OpenFile(const String &FileName, const String Mode)
	{

        // TODO This should be removed
        TA3D::Paths::MakeDir(TA3D::Paths::ExtractFilePath(FileName));		// Create tree structure if it doesn't exist

        # if defined TA3D_PLATFORM_MSVC
		FILE *file;
		errno_t err;
		if ((err = fopen_s( &file, FileName.c_str(), Mode.c_str())) == 0)
			return file;
        return NULL;
        # else
		return fopen(FileName.c_str(), Mode.c_str());
        # endif
	}


	void TA3D_clear_cache(bool force)							// Clear the cache if needed (useful when mod has changed)
	{
		bool rebuild_cache = false;
		// Check cache date
		String cache_date = lp_CONFIG
            ? format("build info : %s , %s\ncurrent mod : %s\n", __DATE__, __TIME__, lp_CONFIG->last_MOD.c_str())
            : format("build info : %s , %s\ncurrent mod : \n", __DATE__, __TIME__ );

		if(TA3D::Paths::Exists(TA3D::Paths::Caches + "cache_info.txt") && !force)
        {
			FILE *cache_info = TA3D_OpenFile(TA3D::Paths::Caches + "cache_info.txt", "rb");
			if(cache_info)
            {
				char *buf = new char[cache_date.size() + 1];
				if(buf)
                {
					memset(buf, 0, cache_date.size() + 1);
					fread(buf, cache_date.size(), 1, cache_info);
					if( buf == cache_date )
						rebuild_cache = false;
					else
						rebuild_cache = true;
					delete[] buf;
				}
				fclose( cache_info );
			}
		}
		else
			rebuild_cache = true;

		if(rebuild_cache)
        {
            String::List file_list;
            Paths::GlobFiles(file_list, TA3D::Paths::Caches + "*");
            for(String::List::iterator i = file_list.begin() ; i != file_list.end() ; ++i)
                remove( i->c_str() );
			// Update cache date
			FILE *cache_info = TA3D_OpenFile(TA3D::Paths::Caches + "cache_info.txt", "wb");
			if(cache_info)
            {
				fwrite( cache_date.c_str(), cache_date.size(), 1, cache_info);
				putc( 0, cache_info );
				fclose( cache_info );
			}
		}
	}
} // namespace TA3D


