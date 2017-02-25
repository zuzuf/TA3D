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
#include <stdafx.h>
#include "gfx.toolkit.h"
#include <misc/paths.h>
#include <TA3D_NameSpace.h>
#include "gfx.h"
#include <QFile>
#include <QDataStream>
#include <QImageWriter>
#include <QFileInfo>
#include <QPainter>
#include <misc/grid.h>

namespace TA3D
{
	static bool use_TA_palette = true;


	void disable_TA_palette()
	{
		use_TA_palette = false;
	}

	void enable_TA_palette()
	{
		use_TA_palette = true;
	}



    void convert_format(QImage &bmp)
	{
        if (bmp.depth() != 32)
		{
            if (bmp.depth() == 8 && use_TA_palette)
                bmp.setColorTable(TA3D::VARS::pal);

            bmp = bmp.convertToFormat(QImage::Format_RGBA8888);
		}
	}

    QImage convert_format_copy(const QImage &bmp)
	{
        if (bmp.depth() != 32)
        {
//            if (bmp.depth() == 8 && use_TA_palette)
//                bmp.setColorTable(TA3D::VARS::pal);

            return bmp.convertToFormat(QImage::Format_RGBA8888);
        }
        return bmp;
    }

    void convert_format_24(QImage &bmp)
	{
        if (bmp.depth() != 24)
		{
            if (bmp.depth() == 8 && use_TA_palette)
                bmp.setColorTable(TA3D::VARS::pal);

            bmp = bmp.convertToFormat(QImage::Format_RGB888);
		}
	}

    QImage convert_format_24_copy(const QImage &bmp)
	{
        if (bmp.depth() != 24)
        {
//            if (bmp.depth() == 8 && use_TA_palette)
//                bmp.setColorTable(TA3D::VARS::pal);

            return bmp.convertToFormat(QImage::Format_RGB888);
        }
        return bmp;
    }

    void convert_format_16(QImage &bmp)
	{
        if (bmp.depth() != 16)
		{
            if (bmp.depth() == 8 && use_TA_palette)
                bmp.setColorTable(TA3D::VARS::pal);

            bmp = bmp.convertToFormat(QImage::Format_RGB16);
		}
	}

    QImage convert_format_16_copy(const QImage &bmp)
	{
        if (bmp.depth() != 16)
        {
//            if (bmp.depth() == 8 && use_TA_palette)
//                bmp.setColorTable(TA3D::VARS::pal);

            return bmp.convertToFormat(QImage::Format_RGB16);
        }
        return bmp;
    }

    void blit(const QImage &in, QImage &out, int x0, int y0, int x1, int y1, int w, int h)
	{
        QPainter painter(&out);
        painter.drawImage(x1, y1, in, x0, y0, w, h);
	}

    void masked_blit(const QImage &in, QImage &out, int x0, int y0, int x1, int y1, int w, int h)
	{
        QPainter painter(&out);
        painter.drawImage(x1, y1, in, x0, y0, w, h);
	}

    void stretch_blit( const QImage &in, QImage &out, int x0, int y0, int w0, int h0, int x1, int y1, int w1, int h1 )
	{
        QPainter painter(&out);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
        painter.drawImage(QRect(x1, y1, w1, h1), in, QRect(x0, y0, w0, h0));
	}

    void stretch_blit_smooth(const QImage &in, QImage &out, int x0, int y0, int w0, int h0, int x1, int y1, int w1, int h1 )
	{
        QPainter painter(&out);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
        painter.drawImage(QRect(x1, y1, w1, h1), in, QRect(x0, y0, w0, h0));
	}

    QImage shrink(const QImage &in, const int w, const int h)
	{
        QImage tmp = gfx->create_surface_ex(in.depth(), in.width(), in.height());
        QImage out = gfx->create_surface_ex(in.depth(), w, h);
		// Gaussian blur pass to remove HF components
        const float sigx = float(in.width()) / float(w) - 1.0f;
        const float sigy = float(in.height()) / float(h) - 1.0f;
		const int sx = static_cast<int>((sigx + 1.0f) * 2.0f);
		const int sy = static_cast<int>((sigy + 1.0f) * 2.0f);
		const int sx2 = 2 * sx - 1;
		const int sy2 = 2 * sy - 1;
        std::vector<uint32> kerX(sx);
        std::vector<uint32> kerY(sy);
		if (sigx > 0.0f)
		{
			uint32 sum = 0U;
			for(int i = 0 ; i < sx ; ++i)
			{
				kerX[i] = uint32(std::exp(-double(i * i) / (2.0 * sigx * sigx)) * 65536.0);
				sum += kerX[i];
				if (i)	sum += kerX[i];
			}
			for(int i = 0 ; i < sx ; ++i)
				kerX[i] = uint32(double(kerX[i]) * 65536.0 / sum);
		}
		else
			for(int i = 0 ; i < sx ; ++i)
				kerX[i] = i == 0 ? 0x10000U : 0U;
		if (sigy > 0.0f)
		{
			uint32 sum = 0U;
			for(int i = 0 ; i < sy ; ++i)
			{
				kerY[i] = uint32(std::exp(-double(i * i) / (2.0 * sigy * sigy)) * 65536.0);
				sum += kerY[i];
				if (i)	sum += kerY[i];
			}
			for(int i = 0 ; i < sy ; ++i)
				kerY[i] = uint32(double(kerY[i]) * 65536.0 / sum);
		}
		else
			for(int i = 0 ; i < sy ; ++i)
				kerY[i] = i == 0 ? 0x10000U : 0U;

        const int twm1 = tmp.width() - 1;
        const int thm1 = tmp.height() - 1;
        const int owm1 = out.width() - 1;
        const int ohm1 = out.height() - 1;
		const uint32 mx = 0x10000U * twm1 / owm1;
		const uint32 my = 0x10000U * thm1 / ohm1;

        switch(in.depth())
		{
		case 24:
            parallel_for<int>(0, in.height(), [&](const int y)
			{
                for(int	x = 0 ; x < out.width() ; ++x)
				{
					const int X = x * mx >> 16;
                    byte *p = (byte*)tmp.scanLine(y) + X * 3;
					uint32 col[3] = { 0U, 0U, 0U };
					const int start = std::max(-sx + 1, -X);
                    const int end = std::min(sx, in.width() - X);
                    byte *c = (byte*)in.scanLine(y) + (X + start) * 3;
					if (end - start == sx2)
					{
						for(int i = -sx + 1 ; i < sx ; ++i, c += 3)
						{
							const uint32 f = kerX[abs(i)];
							col[0] += f * c[0];
							col[1] += f * c[1];
							col[2] += f * c[2];
						}
						p[0] = byte(col[0] >> 16);
						p[1] = byte(col[1] >> 16);
						p[2] = byte(col[2] >> 16);
					}
					else
					{
						uint32 sum = 0U;
						for(int i = start ; i < end ; ++i, c += 3)
						{
							const uint32 f = kerX[abs(i)];
							col[0] += f * c[0];
							col[1] += f * c[1];
							col[2] += f * c[2];
							sum += f;
						}
						p[0] = byte(col[0] / sum);
						p[1] = byte(col[1] / sum);
						p[2] = byte(col[2] / sum);
					}
				}
            });
            parallel_for<int>(0, out.width(), [&](const int x)
			{
				const int X = x * mx >> 16;
                byte *p = (byte*)out.scanLine(0) + x * 3;
                for(int	y = 0 ; y < out.height() ; ++y, p += out.bytesPerLine())
				{
					const int Y = y * my >> 16;
					uint32 col[3] = { 0U, 0U, 0U };
					const int start = std::max(-sy + 1, -Y);
                    const int end = std::min(sy, in.height() - Y);
                    byte *c = (byte*)tmp.scanLine(Y + start) + X * 3;
					if (end - start == sy2)
					{
                        for(int i = -sy + 1 ; i < sy ; ++i, c += tmp.bytesPerLine())
						{
							const uint32 f = kerY[abs(i)];
							col[0] += f * c[0];
							col[1] += f * c[1];
							col[2] += f * c[2];
						}
						p[0] = byte(col[0] >> 16);
						p[1] = byte(col[1] >> 16);
						p[2] = byte(col[2] >> 16);
					}
					else
					{
						uint32 sum = 0U;
                        for(int i = start ; i < end ; ++i, c += tmp.bytesPerLine())
						{
							const uint32 f = kerY[abs(i)];
							col[0] += f * c[0];
							col[1] += f * c[1];
							col[2] += f * c[2];
							sum += f;
						}
						p[0] = byte(col[0] / sum);
						p[1] = byte(col[1] / sum);
						p[2] = byte(col[2] / sum);
					}
				}
            });
			break;
		case 32:
            parallel_for<int>(0, in.height(), [&](const int y)
			{
                for(int	x = 0 ; x < out.width() ; ++x)
				{
					const int X = x * mx >> 16;
                    byte *p = (byte*)tmp.scanLine(y) + (X << 2);
					uint32 col[4] = { 0U, 0U, 0U, 0U };
					const int start = std::max(-sx + 1, -X);
                    const int end = std::min(sx, in.width() - X);
                    byte *c = (byte*)in.scanLine(y) + ((X + start) << 2);
					if (end - start == sx2)
					{
						for(int i = -sx + 1 ; i < sx ; ++i, c += 4)
						{
							const uint32 f = kerX[abs(i)];
							col[0] += f * c[0];
							col[1] += f * c[1];
							col[2] += f * c[2];
							col[3] += f * c[3];
						}
						p[0] = byte(col[0] >> 16);
						p[1] = byte(col[1] >> 16);
						p[2] = byte(col[2] >> 16);
						p[3] = byte(col[3] >> 16);
					}
					else
					{
						uint32 sum = 0U;
						for(int i = start ; i < end ; ++i, c += 4)
						{
							const uint32 f = kerX[abs(i)];
							col[0] += f * c[0];
							col[1] += f * c[1];
							col[2] += f * c[2];
							col[3] += f * c[3];
							sum += f;
						}
						p[0] = byte(col[0] / sum);
						p[1] = byte(col[1] / sum);
						p[2] = byte(col[2] / sum);
						p[3] = byte(col[3] / sum);
					}
				}
            });
            parallel_for<int>(0, out.width(), [&](const int x)
			{
				const int X = x * mx >> 16;
                byte *p = (byte*)out.scanLine(0) + (x << 2);
                for(int	y = 0 ; y < out.height() ; ++y, p += out.bytesPerLine())
				{
					const int Y = y * my >> 16;
					uint32 col[4] = { 0U, 0U, 0U, 0U };
					const int start = std::max(-sy + 1, -Y);
                    const int end = std::min(sy, in.height() - Y);
                    byte *c = (byte*)tmp.scanLine(Y + start) + (X << 2);
					if (end - start == sy2)
					{
                        for(int i = -sy + 1 ; i < sy ; ++i, c += tmp.bytesPerLine())
						{
							const uint32 f = kerY[abs(i)];
							col[0] += f * c[0];
							col[1] += f * c[1];
							col[2] += f * c[2];
							col[3] += f * c[3];
						}
						p[0] = byte(col[0] >> 16);
						p[1] = byte(col[1] >> 16);
						p[2] = byte(col[2] >> 16);
						p[3] = byte(col[3] >> 16);
					}
					else
					{
						uint32 sum = 0U;
                        for(int i = start ; i < end ; ++i, c += tmp.bytesPerLine())
						{
							const uint32 f = kerY[abs(i)];
							col[0] += f * c[0];
							col[1] += f * c[1];
							col[2] += f * c[2];
							col[3] += f * c[3];
							sum += f;
						}
						p[0] = byte(col[0] / sum);
						p[1] = byte(col[1] / sum);
						p[2] = byte(col[2] / sum);
						p[3] = byte(col[3] / sum);
					}
				}
            });
			break;
		}
		return out;
	}

    inline void putpixel(QImage &bmp, int x, int y, uint32 col)
	{
        if (x < 0 || y < 0 || x >= bmp.width() || y >= bmp.height())   return;
        switch(bmp.depth())
		{
			case 8:
				SurfaceByte(bmp, x, y) = byte(col);
				break;
			case 16:
                SurfaceShort(bmp, x, y) = uint16(col);
				break;
			case 24:
				SurfaceByte(bmp, x * 3, y) = getb32(col);
				SurfaceByte(bmp, x * 3 + 1, y) = getg32(col);
				SurfaceByte(bmp, x * 3 + 2, y) = getr32(col);
				break;
			case 32:
				SurfaceInt(bmp, x, y) = col;
				break;
		};
	}

    uint32 getpixel(const QImage &bmp, int x, int y)
	{
        if (x < 0 || y < 0 || x >= bmp.width()|| y >= bmp.height())   return 0;
        switch(bmp.depth())
		{
			case 8:
				return SurfaceByte(bmp, x, y);
			case 16:
                return SurfaceShort(bmp, x, y);
			case 24:
				{
					const int b = SurfaceByte(bmp, x * 3, y);
					const int g = SurfaceByte(bmp, x * 3 + 1, y);
					const int r = SurfaceByte(bmp, x * 3 + 2, y);
					return makecol24(r,g,b);
				}
			case 32:
				return SurfaceInt(bmp, x, y);
		};
		return 0;
	}

    void circlefill(QImage &bmp, int x, int y, int r, const uint32 col)
	{
		const int r2 = r * r;
		const int my = Math::Max(-r, -y);
        const int My = Math::Min(r, bmp.height() - 1 - y);
        switch(bmp.depth())
		{
			case 8:
				for (int sy = my ; sy <= My ; ++sy)
				{
					const int dx = int(sqrtf(float(r2 - sy * sy)));
					const int ax = Math::Max(x - dx, 0);
                    const int bx = Math::Min(x + dx, bmp.width() - 1);
                    memset((byte*)bmp.scanLine(y + sy) + ax, col, bx - ax + 1);
				}
				break;
			case 16:
				{
					const uint16 col16 = uint16(col);
					for (int sy = my ; sy <= My ; ++sy)
					{
						const int dx = int(sqrtf(float(r - sy * sy)));
						const int ax = Math::Max(x - dx, 0);
                        const int bx = Math::Min(x + dx, bmp.width() - 1);
                        uint16 *p = (uint16*)bmp.scanLine(y + sy) + ax;
						for (uint16 *end = p + bx - ax + 1; p != end ; ++p)
							*p = col16;
					}
				}
				break;
			case 24:
				{
					const byte colb = getb32(col);
					const byte colg = getg32(col);
					const byte colr = getr32(col);
					for (int sy = my ; sy <= My ; ++sy)
					{
						const int dx = int(sqrtf(float(r - sy * sy)));
						const int ax = Math::Max(x - dx, 0);
                        const int bx = Math::Min(x + dx, bmp.width() - 1);
                        byte *p = (byte*)bmp.scanLine(y + sy) + ax * 3;
						for (byte *end = p + (bx - ax + 1) * 3 ; p != end ; ++p)
						{
							*p++ = colb;
							*p++ = colg;
							*p = colr;
						}
					}
				}
				break;
			case 32:
				for (int sy = my ; sy <= My ; ++sy)
				{
					const int dx = int(sqrtf(float(r - sy * sy)));
					const int ax = Math::Max(x - dx, 0);
                    const int bx = Math::Min(x + dx, bmp.width() - 1);
                    uint32 *p = (uint32*)bmp.scanLine(y + sy) + ax;
					for (uint32 *end = p + bx - ax + 1; p != end ; ++p)
						*p = col;
				}
				break;
		};
	}

    void rectfill(QImage &bmp, int x0, int y0, int x1, int y1, uint32 col)
	{
        x0 = std::max(x0, 0);
        y0 = std::max(y0, 0);
        x1 = std::min(x1 + 1, bmp.width());
        y1 = std::min(y1 + 1, bmp.height());
        switch(bmp.depth())
        {
        case 8:
            for (int y = y0 ; y < y1 ; ++y)
                memset((byte*)bmp.scanLine(y) + x0, col, x1 - x0);
            break;
        case 16:
            {
                const uint16 col16 = uint16(col);
                for (int y = y0 ; y < y1 ; ++y)
                {
                    uint16 *p = (uint16*)bmp.scanLine(y) + x0;
                    for (uint16 *end = p + x1 - x0 ; p != end ; ++p)
                        *p = col16;
                }
            }
            break;
        case 24:
            {
                const byte colb = getb32(col);
                const byte colg = getg32(col);
                const byte colr = getr32(col);
                for (int y = y0 ; y < y1 ; ++y)
                {
                    byte *p = (byte*)bmp.scanLine(y) + x0 * 3;
                    for (byte *end = p + (x1 - x0) * 3 ; p != end ; ++p)
                    {
                        *p++ = colb;
                        *p++ = colg;
                        *p = colr;
                    }
                }
            }
            break;
        case 32:
            for (int y = y0 ; y < y1 ; ++y)
            {
                uint32 *p = (uint32*)bmp.scanLine(y) + x0;
                for (uint32 *end = p + x1 - x0 ; p != end ; ++p)
                    *p = col;
            }
            break;
        };
	}

    void vflip_bitmap(QImage &bmp)
    {
        bmp = bmp.mirrored(false, true);
    }

    void hflip_bitmap(QImage &bmp)
    {
        bmp = bmp.mirrored(true, false);
    }

    void SaveTex(const QImage &bmp, const QString &filename)
	{
		const int maxTextureSizeAllowed = lp_CONFIG->getMaxTextureSizeAllowed();
        if (std::max(bmp.width(), bmp.height()) > maxTextureSizeAllowed)
		{
            QImage tmp = shrink(bmp, std::min(bmp.width(), maxTextureSizeAllowed), std::min(bmp.height(), maxTextureSizeAllowed));
			SaveTex(tmp, filename);
			return;
		}
        QFile file(filename);
        file.open(QIODevice::WriteOnly);
        if (file.isOpen())
		{
            QByteArray buffer;
            QDataStream stream(&buffer, QIODevice::WriteOnly);
            stream << bmp;
            file.write(qCompress(buffer, 1));
		}
		else
			LOG_ERROR("could not save file : " << filename);
	}

    QImage LoadTex(const QString &filename)
	{
        QFile file(filename);
        file.open(QIODevice::ReadOnly);
        if (file.isOpen())
		{
            const QByteArray &buffer = qUncompress(file.readAll());
            QDataStream stream(buffer);
            QImage bmp;
            stream >> bmp;

			return bmp;
		}
		else
			LOG_ERROR("could not load file : " << filename);
        return QImage();
	}

    void save_bitmap(const QString &filename, const QImage &bmp)
	{
        const QString &ext = QFileInfo(filename).suffix().toUpper();
        const QList<QByteArray> &qt_supported_formats = QImageWriter::supportedImageFormats();
        if (qt_supported_formats.contains(ext.toLatin1()))
            bmp.save(filename);
		else if (ext == ".tex")                      // This is for cached texture data
			SaveTex(bmp, filename);
		else
			LOG_WARNING("save_bitmap : file format not supported : " << ext << " (" << filename << ")");
	}
}
