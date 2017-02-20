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

# include <sdl.h>
# include <misc/string.h>


namespace TA3D
{
namespace Interfaces
{


	class GfxTexture
	{
	public:
		GfxTexture();
		GfxTexture(const GLuint gltex, const uint32 w, const uint32 h);
		~GfxTexture();

		void init();
		void destroy();

        void load(const QString &filename);
		void set(const GLuint gltex);
		void draw(const float x1, const float y1, const uint32 col = 0xFFFFFFFFU, const float scale = 1.0f);
		void drawRotated(const float x1, const float y1, const float angle, const uint32 col = 0xFFFFFFFFU, const float scale = 1.0f);
		void drawCentered(const float x1, const float y1, const uint32 col = 0xFFFFFFFFU, const float scale = 1.0f);
		void drawFlipped(const float x1, const float y1, const uint32 col = 0xFFFFFFFFU, const float scale = 1.0f);
		void drawFlippedCentered(const float x1, const float y1, const uint32 col = 0xFFFFFFFFU, const float scale = 1.0f);

		void bind();
		GLuint get() const	{	return tex;	}
		uint32 getWidth() const	{	return width;	}
		uint32 getHeight() const	{	return height;	}

	public:
		uint32		width;
		uint32		height;
		GLuint		tex;
		bool		destroy_tex;
	}; // class GfxTexture



}
}

#endif // __TA3D_GfxTexture_H__
