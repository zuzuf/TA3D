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
#include "logs/cLogger.h"
#if defined TA3D_PLATFORM_LINUX || defined TA3D_PLATFORM_DARWIN
#   include <sys/stat.h>
#endif
#include "misc/paths.h"

// global variables:
TA3D::TA3DCONFIG		*TA3D::VARS::lp_CONFIG;
TA3D::Interfaces::GFX	*TA3D::VARS::gfx;						// The gfx object we will use to draw basic things and manage fonts, textures, ...
RGB						*TA3D::VARS::pal;
uint8					TA3D::VARS::unit_engine_thread_sync;
uint8					TA3D::VARS::weapon_engine_thread_sync;
uint8					TA3D::VARS::particle_engine_thread_sync;
uint8					TA3D::VARS::players_thread_sync;
I18N_TRANSLATER			TA3D::VARS::i18n;
ObjectSync				*TA3D::VARS::ThreadSynchroniser=NULL;
String					TA3D::VARS::TA3D_CURRENT_MOD="";		// This string stores the path to current mod
int						TA3D::VARS::ascii_to_scancode[ 256 ];



namespace TA3D
{


      // TODO: Construct global config manager class.
      // TODO: Construct global GFX object.
	char *replace_chars(char *str)
	{
		byte *cur_c = (byte*) str;
		while(cur_c[0]!=0) {
			switch(cur_c[0])
			{
			case 232:
			case 233:
				cur_c[0]='e';	break;
			case 195:
			case 224:
				cur_c[0]='a';	break;
			case 231:
				cur_c[0]='c';	break;
			case 244:
				cur_c[0]='o';	break;
			};
			cur_c++;
			}
		return str;
	}

	// Cire: Useful malloc operation.
	void *GetMem( sint32 size, sint32 zero )
	{
		void *result;

		if( zero )
			result = calloc( size, 1 );
		else
			result = malloc( size );

		/*
		TODO: Global DEBUGGER NOTIFICATION HERE.
		if( !result )
		GlobalDebugger->Failed to alloc memory for hpi
		*/
		return result;
	}


	FILE *TA3D_OpenFile( const String &FileName, const String Mode )
	{
		FILE *file;

        // TODO This should be removed
        TA3D::Paths::MakeDir(TA3D::Paths::ExtractFilePath(FileName));		// Create tree structure if it doesn't exist

        # if defined TA3D_PLATFORM_MSVC
		errno_t err;
		if( ( err = fopen_s( &file, FileName.c_str(), Mode.c_str() )) == 0 )
			return file;
        # else
		file=fopen( FileName.c_str(), Mode.c_str() );
		if( file )
			return file;
        # endif
		return NULL;
	}

	String RemoveComments(const String &s)
	{
		String Result(s);
        String::size_type i = Result.find("//");
		if(i != String::npos) // If we have found comments, then remove them and clear the string (remove blanks)
        {
			Result.resize(i);		// Remove comments

            // Find blanks at the end of the string
			for( i = Result.length()-1 ; i>=0 ; --i)
            {
				if( Result[i] != 9 && Result[i] != 32 )
                {
                    ++i;
                    break;
                }
            }
			if(i < Result.length())
                Result.resize(i);
            // Find blanks at the beginning of the string
			for( i = 0 ; i<Result.length() ; ++i)
            {
				if( Result[i] != 9 && Result[i] != 32 )
                {
                    --i;
                    break;
                }
            }

			if( i >= 0 )
				Result.erase( 0, i );
		}
		return Result;
	}


    void ReadVectorString(std::vector<String>& lst, String s, const String& seps, const bool emptyBefore)
	{
        // TODO Should be replaced by
        // boost::algorithm::split(v, *this, boost::is_any_of(separators.c_str()));
        // to make proper optimizations

        if (emptyBefore)
            lst.clear();
		while(!s.empty())
        {
			int i = s.find(seps);
			if( i == -1 )
            {
				lst.push_back(TrimString(s));
				return;
			}
			else
            {
				lst.push_back(TrimString( s.substr(0, i)));
				s = s.substr(i + 1, s.size() - i - 1);
			}
		}
	}


	void TA3D_clear_cache()							// Clear the cache if needed (useful when mod has changed)
	{
		bool rebuild_cache = false;
		// Check cache date
		String cache_date = lp_CONFIG 
            ? format("build info : %s , %s\ncurrent mod : %s\n", __DATE__, __TIME__, lp_CONFIG->last_MOD.c_str())
            : format("build info : %s , %s\ncurrent mod : \n", __DATE__, __TIME__ );

		if(TA3D::Paths::Exists(TA3D::Paths::Caches + "cache_info.txt"))
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
			struct al_ffblk info;
			if (al_findfirst((TA3D::Paths::Caches + "*").c_str(), &info, FA_ALL) == 0)
            {
				do
                {
					delete_file( (TA3D::Paths::Caches + info.name).c_str());
				} while ( !al_findnext(&info) );
				al_findclose(&info);
			}
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


