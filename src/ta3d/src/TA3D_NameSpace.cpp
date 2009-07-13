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




namespace TA3D
{

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


} // namespace TA3D


