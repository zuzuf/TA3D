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

#include "../stdafx.h"
#include "../TA3D_NameSpace.h"
#include "glfunc.h"
#include "../gui.h"
#include "../gaf.h"
#include "gfx.h"
#include "../misc/paths.h"
#include "../logs/logs.h"




namespace TA3D
{
    namespace Interfaces
    {

        GfxTexture::GfxTexture()
            :width(0), height(0), tex(0), destroy_tex(false)
        {}


        void GfxTexture::init()
        {
            width = 0;
            height = 0;
            tex = 0;
            destroy_tex = false;
        }


        void GfxTexture::draw( const float x1, const float y1 )
        {
            gfx->drawtexture( tex, x1, y1, x1 + width, y1 + height );
        }

        void GfxTexture::draw( const float x1, const float y1, const uint32 col )
        {
            gfx->drawtexture(tex, x1, y1, x1 + width, y1 + height, col);
        }

        GfxTexture::GfxTexture( const GLuint gltex )
        {
            destroy_tex = true;
            set( gltex );
        }

        void GfxTexture::set( const GLuint gltex )
        {
            tex = gltex;
            width = gfx->texture_width( tex );
            height = gfx->texture_height( tex );
        }

        void GfxTexture::destroy()
        {
            width = 0;
            height = 0;
            if(destroy_tex)
            {
                gfx->destroy_texture(tex);
                destroy_tex = false;
            }
            else
                tex = 0;
        }




    }
}
