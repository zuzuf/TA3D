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

/*--------------------------------------------------------------\
|                          glfunc.h                             |
|      contains functions and variables to set up an OpenGL     |
|  environnement using some OpenGL extensions.                  |
\--------------------------------------------------------------*/

#include <stdafx.h>
#include <TA3D_NameSpace.h>
#include "glfunc.h"
#include <logs/logs.h>



namespace TA3D
{

    bool	MultiTexturing = false;
	bool	g_useTextureCompression = false;
	bool    g_useGenMipMaps = false;
	bool    g_useNonPowerOfTwoTextures = false;


	void installOpenGLExtensions()
	{
        MultiTexturing = gfx->hasOpenGLFeature(QOpenGLFunctions::Multitexture);

        g_useTextureCompression = gfx->hasOpenGLFeature(QOpenGLFunctions::CompressedTextures);
        g_useGenMipMaps = true;
        g_useNonPowerOfTwoTextures = gfx->hasOpenGLFeature(QOpenGLFunctions::NPOTTextures);
	}


} // namespace TA3D

