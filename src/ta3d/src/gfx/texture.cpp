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
#include <TA3D_NameSpace.h>
#include "glfunc.h"
#include <gaf.h>
#include "gfx.h"
#include <misc/paths.h>
#include <logs/logs.h>

namespace TA3D
{
    GfxTexture::GfxTexture(Target target)
        : m_target(target),
          m_handle(0),
          m_width(0),
          m_height(0),
          m_format(RGB8_UNorm)
    {
        gfx->glGenTextures(1, &m_handle);
        CHECK_GL();
    }

    GfxTexture::GfxTexture(const QImage &image, MipMapGeneration genMipMaps)
        : GfxTexture(Target2D)
    {
        setAutoMipMapGenerationEnabled(genMipMaps);

        PixelFormat pixel_format;
        PixelType pixel_type;
        QImage img = image;
        switch(image.format())
        {
        case QImage::Format_Invalid:
            return;
        case QImage::Format_ARGB32:
            img = image.convertToFormat(QImage::Format_RGBA8888);
        case QImage::Format_RGBA8888:
            pixel_format = RGBA;
            pixel_type = UInt8;
            setFormat(RGBA8_UNorm);
            break;
        case QImage::Format_Indexed8:
            img = image.convertToFormat(QImage::Format_RGB888);
        case QImage::Format_RGB888:
            pixel_format = RGB;
            pixel_type = UInt8;
            setFormat(RGB8_UNorm);
            break;
        case QImage::Format_Grayscale8:
            pixel_format = Luminance;
            pixel_type = UInt8;
            setFormat(Y8_UNorm);
            break;
        default:
            throw std::runtime_error("Unsupported format (" + std::to_string(image.format()) + ")");
        }

        setSize(img.width(), img.height());
        setData(pixel_format, pixel_type, img.bits());
    }

    GfxTexture::~GfxTexture()
    {
        gfx->glDeleteTextures(1, &m_handle);
        CHECK_GL();
    }

    void GfxTexture::setAutoMipMapGenerationEnabled(MipMapGeneration genMipMaps)
    {
        if (bind())
        {
            gfx->glTexParameteri(m_target, GL_GENERATE_MIPMAP, genMipMaps == GenerateMipMaps ? GL_TRUE : GL_FALSE);
            CHECK_GL();
        }
    }

    void GfxTexture::setWrapMode(WrapMode mode)
    {
        if (bind())
        {
            gfx->glTexParameteri(m_target, GL_TEXTURE_WRAP_R, mode);
            CHECK_GL();
            if (m_target == Target2D || m_target == Target3D)
            {
                gfx->glTexParameteri(m_target, GL_TEXTURE_WRAP_S, mode);
                CHECK_GL();
                if (m_target == Target3D)
                {
                    gfx->glTexParameteri(m_target, GL_TEXTURE_WRAP_T, mode);
                    CHECK_GL();
                }
            }
        }
    }

    void GfxTexture::setMinificationFilter(Filter filter)
    {
        if (bind())
        {
            gfx->glTexParameteri(m_target, GL_TEXTURE_MIN_FILTER, filter);
            CHECK_GL();
        }
    }

    void GfxTexture::setMagnificationFilter(Filter filter)
    {
        if (bind())
        {
            gfx->glTexParameteri(m_target, GL_TEXTURE_MAG_FILTER, filter);
            CHECK_GL();
        }
    }

    int GfxTexture::height() const
    {
        return m_height;
    }

    int GfxTexture::width() const
    {
        return m_width;
    }

    bool GfxTexture::bind()
    {
        gfx->glBindTexture(m_target, m_handle);
        CHECK_GL();
        return true;
    }

    void GfxTexture::setData(PixelFormat pixel_format, PixelType pixel_type, const void *data)
    {
        if (bind())
        {
            gfx->glTexImage2D(m_target, 0, m_format, m_width, m_height, 0, pixel_format, pixel_type, data);
            CHECK_GL();
        }
    }

    void GfxTexture::setMaximumAnisotropy(float max_anisotropy)
    {
        if (bind())
        {
            gfx->glTexParameterf(m_target, GL_TEXTURE_MAX_ANISOTROPY, std::min(16.f, std::max(1.f, max_anisotropy)));
            CHECK_GL();
        }
    }

    void GfxTexture::allocateStorage()
    {
        PixelFormat pixel_format;
        PixelType pixel_type;
        switch(m_format)
        {
        case RGB8_UNorm:
            pixel_format = RGB;
            pixel_type = UInt8;
            break;
        case RGBA8_UNorm:
            pixel_format = RGBA;
            pixel_type = UInt8;
            break;
        case RGB16F:
            pixel_format = RGB;
            pixel_type = Float16;
            break;
        case RGBA16F:
            pixel_format = RGBA;
            pixel_type = Float16;
            break;
        case R16F:
            pixel_format = Red;
            pixel_type = Float16;
            break;
        case R32F:
            pixel_format = Red;
            pixel_type = Float32;
            break;
        case Y8_UNorm:
            pixel_format = Luminance;
            pixel_type = UInt8;
            break;
        case D16:
        case D24:
        case D32:
        case D32F:
            pixel_format = Depth;
            pixel_type = Float32;
            break;
        default:
            throw std::runtime_error("Unsupported type : " + std::to_string(m_format));
        }
        allocateStorage(pixel_format, pixel_type);
    }

    void GfxTexture::allocateStorage(PixelFormat pixel_format, PixelType pixel_type)
    {
        setData(pixel_format, pixel_type, nullptr);
    }

    void GfxTexture::setComparisonFunction(ComparisonFunction function)
    {
        if (bind())
        {
            gfx->glTexParameteri(m_target, GL_TEXTURE_COMPARE_FUNC, function);
            CHECK_GL();
        }
    }

    void GfxTexture::setComparisonMode(ComparisonMode mode)
    {
        if (bind())
        {
            gfx->glTexParameteri(m_target, GL_TEXTURE_COMPARE_MODE, mode);
            CHECK_GL();
        }
    }

    void GfxTexture::setFormat(TextureFormat format)
    {
        m_format = format;
    }

    void GfxTexture::setSize(int w, int h)
    {
        m_width = w;
        m_height = h;
    }

    GLint GfxTexture::textureId() const
    {
        return m_handle;
    }

    void GfxTexture::draw(const float x1, const float y1, const uint32 col, const float scale)
    {
        gfx->glDisable(GL_LIGHTING);
        CHECK_GL();
        gfx->drawtexture(this, x1, y1, x1 + scale * float(width()), y1 + scale * float(height()), col);
    }

    void GfxTexture::drawCentered(const float x1, const float y1, const uint32 col, const float scale)
    {
        gfx->glDisable(GL_LIGHTING);
        CHECK_GL();
        gfx->drawtexture(this, x1 - 0.5f * scale * float(width()), y1 - 0.5f * scale * float(height()), x1 + 0.5f * scale * float(width()), y1 + 0.5f * scale * float(height()), col);
    }

    void GfxTexture::drawRotated(const float x1, const float y1, const float angle, const uint32 col, const float scale)
    {
        gfx->glDisable(GL_LIGHTING);
        CHECK_GL();
        glPushMatrix();
        CHECK_GL();
        glTranslatef(x1, y1, 0.0f);
        CHECK_GL();
        glRotatef(angle, 0.0f, 0.0f, 1.0f);
        CHECK_GL();
        gfx->drawtexture(this, -0.5f * scale * float(width()), -0.5f * scale * float(height()), 0.5f * scale * float(width()), 0.5f * scale * float(height()), col);
        glPopMatrix();
        CHECK_GL();
    }
} // namespace Ta3D
