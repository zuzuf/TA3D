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
#ifndef __TA3D_GFX_TOOLKIT_H__
# define __TA3D_GFX_TOOLKIT_H__

# include <stdafx.h>
# include <misc/string.h>
# include <QImage>
# include <logs/logs.h>


# define makeacol(r,g,b,a)   qRgba(r, g, b, a)
# define makecol24(r,g,b)   qRgb(r, g, b)
# define makecol(r,g,b)   makecol24(r,g,b)
# define makeacol32(r,g,b,a)   makeacol(r,g,b,a)

namespace TA3D
{
    inline byte &SurfaceByte(QImage &img, int x, int y)
	{
        LOG_ASSERT(x >= 0 && y >= 0 && x < img.bytesPerLine() && y < img.height());
        return img.scanLine(y)[x];
	}
    inline byte SurfaceByte(const QImage &img, int x, int y)
    {
        LOG_ASSERT(x >= 0 && y >= 0 && x < img.bytesPerLine() && y < img.height());
        return img.scanLine(y)[x];
    }

    inline uint32 &SurfaceInt(QImage &img, int x, int y)
	{
        LOG_ASSERT(img.depth() == 32);
        LOG_ASSERT(x >= 0 && y >= 0 && x < img.width() && y < img.height());
        return ((uint32*)img.scanLine(y))[x];
	}
    inline uint32 SurfaceInt(const QImage &img, int x, int y)
    {
        LOG_ASSERT(img.depth() == 32);
        LOG_ASSERT(x >= 0 && y >= 0 && x < img.width() && y < img.height());
        return ((uint32*)img.scanLine(y))[x];
    }

    inline uint16 &SurfaceShort(QImage &img, int x, int y)
	{
        LOG_ASSERT(img.depth() == 16);
        LOG_ASSERT(x >= 0 && y >= 0 && x < img.width() && y < img.height());
        return ((uint16*)img.scanLine(y))[x];
	}
    inline uint16 SurfaceShort(const QImage &img, int x, int y)
    {
        LOG_ASSERT(img.depth() == 16);
        LOG_ASSERT(x >= 0 && y >= 0 && x < img.width() && y < img.height());
        return ((uint16*)img.scanLine(y))[x];
    }

	template<typename T>
            inline T &SurfaceType(QImage &img, int x, int y)
	{
        LOG_ASSERT(x >= 0 && y >= 0 && x < img.width() && y < img.height());
        return ((T*)img.scanLine(y))[x];
	}
}

# define geta32(x) qAlpha(x)
# define getb32(x) qBlue(x)
# define getg32(x) qGreen(x)
# define getr32(x) qRed(x)

# define getr(x) getr32(x)
# define getg(x) getg32(x)
# define getb(x) getb32(x)
# define geta(x) geta32(x)



namespace TA3D
{

    void masked_blit(const QImage &in, QImage &out, int x0, int y0, int x1, int y1, int w, int h);
    void blit(const QImage &in, QImage &out, int x0, int y0, int x1, int y1, int w, int h);
    void stretch_blit(const QImage &in, QImage &out, int x0, int y0, int w0, int h0, int x1, int y1, int w1, int h1 );
    void stretch_blit_smooth(const QImage &in, QImage &out, int x0, int y0, int w0, int h0, int x1, int y1, int w1, int h1 );

    QImage shrink(const QImage &in, const int w, const int h);

    void putpixel(QImage &bmp, int x, int y, uint32 col);
    uint32 getpixel(const QImage &bmp, int x, int y);

    void circlefill(QImage &bmp, int x, int y, int r, uint32 col);

    void rectfill(QImage &bmp, int x0, int y0, int x1, int y1, uint32 col);

    QImage convert_format_copy(QImage &bmp);
    QImage convert_format_24_copy(QImage &bmp);
    QImage convert_format_16_copy(QImage &bmp);

    void convert_format(QImage &bmp);
    void convert_format_24(QImage &bmp);
    void convert_format_16(QImage &bmp);

	void disable_TA_palette();
	void enable_TA_palette();

    void save_bitmap(const QString &filename, const QImage &bmp);

    void vflip_bitmap(QImage &bmp);
    void hflip_bitmap(QImage &bmp);

    QImage LoadTex(const QString &filename);
    void SaveTex(const QImage &bmp, const QString &filename);
}


#endif
