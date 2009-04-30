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
#ifndef __TA3D_GfxTexture_H__
# define __TA3D_GfxTexture_H__

# include "gfx.h"



namespace TA3D
{
namespace Interfaces
{

    class GfxTexture
    {
    public:
        uint32		width;
        uint32		height;
        GLuint		tex;
        bool		destroy_tex;

        void init();
        GfxTexture();
        GfxTexture(const GLuint gltex);
        void set(const GLuint gltex);
        void draw(const float x1, const float y1);
        void draw(const float x1, const float y1, const uint32 col);

        void destroy();

    }; // class GfxTexture



}
}

#endif // __TA3D_GfxTexture_H__
