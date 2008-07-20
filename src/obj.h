/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2005  Roland BROCHARD

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

/*-----------------------------------------------------------------------------\
|                                    obj.h                                     |
|        This module is a loader for the OBJ file format, it loads a model     |
|  into an MODEL object that can be used by the 3dmeditor.                     |
\-----------------------------------------------------------------------------*/

#ifndef __TA3D_OBJ
#define __TA3D_OBJ

#include "misc/matrix.h"
#include "3do.h"

MODEL *load_obj( const String &filename, float scale = 20.0f );

#endif
