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

# include <QOpenGLFunctions>
# include <misc/string.h>
# include <QOpenGLTexture>
# include <zuzuf/smartptr.h>

namespace TA3D
{
    class GfxTexture : public QOpenGLTexture, public zuzuf::ref_count
	{
    public:
        typedef zuzuf::smartptr<GfxTexture> Ptr;
	public:
        GfxTexture(Target target);
        GfxTexture(const QImage &image, MipMapGeneration genMipMaps = GenerateMipMaps);
		~GfxTexture();

		void draw(const float x1, const float y1, const uint32 col = 0xFFFFFFFFU, const float scale = 1.0f);
		void drawRotated(const float x1, const float y1, const float angle, const uint32 col = 0xFFFFFFFFU, const float scale = 1.0f);
		void drawCentered(const float x1, const float y1, const uint32 col = 0xFFFFFFFFU, const float scale = 1.0f);
		void drawFlipped(const float x1, const float y1, const uint32 col = 0xFFFFFFFFU, const float scale = 1.0f);
		void drawFlippedCentered(const float x1, const float y1, const uint32 col = 0xFFFFFFFFU, const float scale = 1.0f);
    }; // class GfxTexture
}

#endif // __TA3D_GfxTexture_H__
