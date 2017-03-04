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
        : QOpenGLTexture(target)
    {}

    GfxTexture::GfxTexture(const QImage &image, MipMapGeneration genMipMaps)
        : QOpenGLTexture(image, genMipMaps)
    {}

    GfxTexture::~GfxTexture()
    {
    }

    void GfxTexture::draw(const float x1, const float y1, const uint32 col, const float scale)
    {
        glDisable(GL_LIGHTING);
        gfx->drawtexture(this, x1, y1, x1 + scale * float(width()), y1 + scale * float(height()), col);
    }

    void GfxTexture::drawCentered(const float x1, const float y1, const uint32 col, const float scale)
    {
        glDisable(GL_LIGHTING);
        gfx->drawtexture(this, x1 - 0.5f * scale * float(width()), y1 - 0.5f * scale * float(height()), x1 + 0.5f * scale * float(width()), y1 + 0.5f * scale * float(height()), col);
    }

    void GfxTexture::drawRotated(const float x1, const float y1, const float angle, const uint32 col, const float scale)
    {
        glDisable(GL_LIGHTING);
        glPushMatrix();
        glTranslatef(x1, y1, 0.0f);
        glRotatef(angle, 0.0f, 0.0f, 1.0f);
        gfx->drawtexture(this, -0.5f * scale * float(width()), -0.5f * scale * float(height()), 0.5f * scale * float(width()), 0.5f * scale * float(height()), col);
        glPopMatrix();
    }
} // namespace Ta3D
