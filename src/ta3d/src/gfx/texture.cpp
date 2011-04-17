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
namespace Interfaces
{


	GfxTexture::GfxTexture()
		:width(0), height(0), tex(0), destroy_tex(false)
	{}

	GfxTexture::~GfxTexture()
	{
		destroy();
	}

	void GfxTexture::load(const String &filename)
	{
		destroy();
		set(gfx->load_texture(filename, FILTER_TRILINEAR, &width, &height));
	}

	void GfxTexture::init()
	{
		width = 0;
		height = 0;
		tex = 0;
		destroy_tex = false;
	}

	void GfxTexture::draw(const float x1, const float y1, const uint32 col, const float scale)
	{
		glDisable(GL_LIGHTING);
		gfx->drawtexture(tex, x1, y1, x1 + scale * float(width), y1 + scale * float(height), col);
	}

	void GfxTexture::drawCentered(const float x1, const float y1, const uint32 col, const float scale)
	{
		glDisable(GL_LIGHTING);
		gfx->drawtexture(tex, x1 - 0.5f * scale * float(width), y1 - 0.5f * scale * float(height), x1 + 0.5f * scale * float(width), y1 + 0.5f * scale * float(height), col);
	}

	void GfxTexture::drawRotated(const float x1, const float y1, const float angle, const uint32 col, const float scale)
	{
		glDisable(GL_LIGHTING);
		glPushMatrix();
		glTranslatef(x1, y1, 0.0f);
		glRotatef(angle, 0.0f, 0.0f, 1.0f);
		gfx->drawtexture(tex, -0.5f * scale * float(width), -0.5f * scale * float(height), 0.5f * scale * float(width), 0.5f * scale * float(height), col);
		glPopMatrix();
	}

	void GfxTexture::drawFlipped(const float x1, const float y1, const uint32 col, const float scale)
	{
		glDisable(GL_LIGHTING);
		gfx->drawtexture_flip(tex, x1, y1, x1 + scale * float(width), y1 + scale * float(height), col);
	}

	void GfxTexture::drawFlippedCentered(const float x1, const float y1, const uint32 col, const float scale)
	{
		glDisable(GL_LIGHTING);
		gfx->drawtexture_flip(tex, x1 - 0.5f * scale * float(width), y1 - 0.5f * scale * float(height), x1 + 0.5f * scale * float(width), y1 + 0.5f * scale * float(height), col);
	}

	GfxTexture::GfxTexture(GLuint gltex, uint32 w, uint32 h)
	{
		destroy_tex = false;
		set( gltex );
		width = w;
		height = h;
	}

	void GfxTexture::set(const GLuint gltex)
	{
		tex = gltex;
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

	void GfxTexture::bind()
	{
		glBindTexture(GL_TEXTURE_2D, tex);
	}


} // namespace Interfaces
} // namespace Ta3D
