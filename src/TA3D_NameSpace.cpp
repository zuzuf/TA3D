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
#include "cLogger.h"
#if defined TA3D_PLATFORM_LINUX || defined TA3D_PLATFORM_DARWIN
#   include <sys/stat.h>
#endif

// global variables:
TA3D::TA3DCONFIG		*TA3D::VARS::lp_CONFIG;
TA3D::INTERFACES::GFX	*TA3D::VARS::gfx;						// The gfx object we will use to draw basic things and manage fonts, textures, ...
RGB						*TA3D::VARS::pal;
uint8					TA3D::VARS::unit_engine_thread_sync;
uint8					TA3D::VARS::weapon_engine_thread_sync;
uint8					TA3D::VARS::particle_engine_thread_sync;
uint8					TA3D::VARS::players_thread_sync;
String					TA3D::VARS::TA3D_OUTPUT_DIR;
I18N_TRANSLATER			TA3D::VARS::i18n;
ThreadSync				*TA3D::VARS::ThreadSynchroniser=NULL;
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

	void create_path( const String &path )
	{
		if( !file_exists( path.c_str(), FA_DIREC, NULL ) ) {
			String parent = get_path( path );
			if( !parent.empty() )
				create_path( parent );
#if defined TA3D_PLATFORM_WINDOWS
			mkdir( path.c_str() );
#else
			mkdir( path.c_str(), 0x1FF );
#endif
			}
	}

	FILE *TA3D_OpenFile( const String &FileName, const String Mode )
	{
		FILE *file;

		create_path( get_path( FileName ) );		// Create tree structure if it doesn't exist

#if defined TA3D_PLATFORM_MSVC
		errno_t err;
		if( ( err = fopen_s( &file, FileName.c_str(), Mode.c_str() )) == 0 )
			return file;
#else
		file=fopen( FileName.c_str(), Mode.c_str() );
		if( file )
			return file;
#endif
		return NULL;
	}

	String RemoveComments( String &sstring )
	{
		String Result = String( sstring );
		int i = (int)Result.find("//");
		if( i != -1 ) {					// If we have found comments, then remove them and clear the string (remove blanks)
			Result.resize(i);		// Remove comments

			for( i = Result.length()-1 ; i>=0 ; i--)				// Find blanks at the end of the string
				if( Result[i] != 9 && Result[i] != 32 ) {	i++;	break;	}

			if( i < Result.length() )	Result.resize(i);			// Remove them

			for( i = 0 ; i<Result.length() ; i++)				// Find blanks at the beginning of the string
				if( Result[i] != 9 && Result[i] != 32 ) {	i--;	break;	}

			if( i >= 0 )
				Result.erase( 0, i );
			}
		return Result;
	}


    void ReadVectorString(Vector<String>& lst, String s, const String& seps, const bool emptyBefore)
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


	bool TA3D_exists( const String &filename )			// just a wrapper for allegro's exists function which only use C strings
	{
		return exists( filename.c_str() );
	}

	void CheckOutputDir()
	{
#if defined TA3D_PLATFORM_WINDOWS
		TA3D_OUTPUT_DIR = "";
#elif defined TA3D_PLATFORM_LINUX
		char *home = getenv( "HOME" );
		TA3D_OUTPUT_DIR = (home != NULL) ? String( home ) + "/.ta3d" : "" ;
#elif defined TA3D_PLATFORM_MAC
		char *home = getenv( "HOME" );
		TA3D_OUTPUT_DIR = (home != NULL) ? String( home ) + "/.ta3d" : "" ;
#endif

		if( TA3D_OUTPUT_DIR != "" ) {
#if defined TA3D_PLATFORM_WINDOWS
			mkdir( TA3D_OUTPUT_DIR.c_str() );
#else
			mkdir( TA3D_OUTPUT_DIR.c_str(), 0xFFF );
#endif
			TA3D_OUTPUT_DIR += "/";		// Make sure we can use it like this : TA3D_OUTPUT_DIR + filename
#if defined TA3D_PLATFORM_WINDOWS
			mkdir( (TA3D_OUTPUT_DIR+"cache").c_str() );
#else
			mkdir( (TA3D_OUTPUT_DIR+"cache").c_str(), 0x1FF );
#endif
			}
			
#if defined TA3D_PLATFORM_WINDOWS
		mkdir( (TA3D_OUTPUT_DIR+"savegame").c_str() );
#else
		mkdir( (TA3D_OUTPUT_DIR+"savegame").c_str(), 0x1FF );
#endif
	}

	void TA3D_clear_cache()							// Clear the cache if needed (useful when mod has changed)
	{
		bool rebuild_cache = false;

			// Check cache date
		String cache_date = lp_CONFIG ? format("build info : %s , %s\ncurrent mod : %s\n", __DATE__, __TIME__, lp_CONFIG->last_MOD.c_str() ) : format("build info : %s , %s\ncurrent mod : \n", __DATE__, __TIME__ );

		if( TA3D_exists( TA3D_OUTPUT_DIR + "cache/cache_info.txt" ) ) {
			FILE *cache_info = TA3D_OpenFile( TA3D_OUTPUT_DIR + "cache/cache_info.txt", "rb" );
			if( cache_info ) {
				char *buf = new char[ cache_date.size() + 1 ];
				if( buf ) {
					memset( buf, 0, cache_date.size() + 1 );
					fread( buf, cache_date.size(), 1, cache_info );
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

		if( rebuild_cache ) {
			struct al_ffblk info;

			if (al_findfirst((TA3D_OUTPUT_DIR+"cache/*").c_str(), &info, FA_ALL) == 0) {
				do {
					delete_file( (TA3D_OUTPUT_DIR + "cache/" + info.name).c_str() );
					} while ( !al_findnext(&info) );
				al_findclose(&info);
				}

											// Update cache date
			FILE *cache_info = TA3D_OpenFile( TA3D_OUTPUT_DIR + "cache/cache_info.txt", "wb" );
			if( cache_info ) {
				fwrite( cache_date.c_str(), cache_date.size(), 1, cache_info );
				putc( 0, cache_info );
				fclose( cache_info );
				}
			}
	}

	List< String > GetFileList( const String pattern )	// return the list of files corresponding to pattern
	{
		List< String >	result;

		struct al_ffblk info;

		if (al_findfirst(pattern.c_str(), &info, FA_ALL) != 0)
			return result;

		do {
			result.push_back( info.name );
		} while (al_findnext(&info) == 0);

		al_findclose(&info);

		return result;
	}
}
