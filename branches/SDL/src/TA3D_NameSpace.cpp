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
int						TA3D::VARS::ascii_to_scancode[ 256 ];
SDL_Surface             *TA3D::VARS::screen = NULL;

int                     TA3D::VARS::mouse_x = 0;
int                     TA3D::VARS::mouse_y = 0;
int                     TA3D::VARS::mouse_z = 0;
int                     TA3D::VARS::mouse_b = 0;
int                     TA3D::VARS::key[0x1000];
std::list<int>          TA3D::VARS::keybuf;



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


	int readkey()
	{
	    int res = VARS::keybuf.front();
	    VARS::keybuf.pop_front();
	    return res;
    }

	void clear_keybuf()
	{
	    VARS::keybuf.clear();
	}

    void poll_keyboard()
    {
        poll_mouse();
    }

    void poll_mouse()
    {
        SDL_Event event;

        while(SDL_PollEvent(&event))
        {
            switch(event.type)
            {
            case SDL_KEYDOWN:
                {
                    VARS::key[ event.key.keysym.sym ] = 1;
                    int c = event.key.keysym.sym;
                    if (c >= KEY_0 && c <= KEY_9)
                        c = c - KEY_0 + '0';
                    if ((key[ KEY_RSHIFT ] || key[ KEY_LSHIFT ] || event.key.keysym.mod == KMOD_CAPS) && c >= 'a' && c <= 'z')
                        c = c + 'A' - 'a';
                    VARS::keybuf.push_back( c );
                    LOG_DEBUG("pressing " << (char)c);
                }
                break;
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                switch(event.button.button)
                {
                case SDL_BUTTON_WHEELDOWN:
                    if (event.button.state == SDL_PRESSED)
                        mouse_z--;
                    break;
                case SDL_BUTTON_WHEELUP:
                    if (event.button.state == SDL_PRESSED)
                        mouse_z++;
                    break;
                };
                break;
            case SDL_KEYUP:
                VARS::key[ event.key.keysym.sym ] = 0;
                break;
            default:
                LOG_DEBUG("Unhandled event");
            };
        }
        mouse_b = SDL_GetMouseState( &mouse_x, &mouse_y );
    }

	void position_mouse(int x, int y)
	{
	    mouse_x = x;
	    mouse_y = y;
	    SDL_WarpMouse(x,y);
	    poll_mouse();
        SDL_GetRelativeMouseState(NULL, NULL);
	}

	void get_mouse_mickeys(int *mx, int *my)
	{
        SDL_GetRelativeMouseState(mx, my);
	}
} // namespace TA3D


