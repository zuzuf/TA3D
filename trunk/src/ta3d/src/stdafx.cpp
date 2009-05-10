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

/*
 **  File: stdafx.cpp
 ** Notes:
 **   Cire: stdafx.h and stdafx.cpp will generate a pch file
 **         (pre compiled headers). Its goal is to include
 **         everything that should be global to the project, as well
 **         as platform specific setups and configurations.
 */

#include "stdafx.h"
#include <sys/stat.h>
#include <fstream>
#include <cmath>

using namespace std;

namespace TA3D
{

	String
		get_path(const String& s)
		{
			String path;
			for( int i = s.size() - 1 ; i >= 0 ; i-- )
			{
				if( s[i] == '/' || s[i] == '\\' )
				{
					path = s;
					path.resize(i);
					break;
				}
			}
			return path;
		}



#if defined TA3D_PLATFORM_WINDOWS && defined TA3D_PLATFORM_MSVC
	void
		ExtractPathFile(const String& szFullFileName, String& szFile, String& szDir)
		{
			char drive[_MAX_DRIVE];
			char dir[_MAX_DIR];
			char fname[_MAX_FNAME];
			char ext[_MAX_EXT];
			// Below extracts our path where the application is being run.
			::_splitpath_s( szFullFileName.c_str(), drive, dir, fname, ext );

			szFile = fname;
			szDir = drive;
			szDir += dir;
		}
#endif


	String GetClientPath(void)
	{
		static bool bName = false;
		static String szName = "";

		if (!bName)
		{
# if defined TA3D_PLATFORM_WINDOWS && defined TA3D_PLATFORM_MSVC
			char fPath[ MAX_PATH ];
			String Tmp;
			::GetModuleFileNameA( NULL, fPath, MAX_PATH );
			ExtractPathFile( fPath, Tmp, szName );
# endif
			bName = true;
		}
		return szName;
	}

	void rest(uint32 msec)
	{
		SDL_Delay(msec);
	}




} // namespace TA3D
