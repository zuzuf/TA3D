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

//
//	the gfx module
//
//	it contains basic gfx commands like font management, background management, etc...
//

#include <stdafx.h>
#include <TA3D_NameSpace.h>
#include "glfunc.h"
#include <gaf.h>
#include "gfx.h"
#include "gui/skin.manager.h"
#include <misc/paths.h>
#include <logs/logs.h>
#include <strings.h>
#include <misc/math.h>
#include <input/keyboard.h>
#include <input/mouse.h>
#include "gui/base.h"
#include <backtrace.h>
#include <QFile>
#include <iostream>
#include <QApplication>
#include <QScreen>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWheelEvent>

namespace TA3D
{
    TA3D::GFX::Ptr  TA3D::VARS::gfx;						// The gfx object we will use to draw basic things and manage fonts, textures, ...

	void GFX::set_texture_format(GLuint gl_format)
	{
		texture_format = gl_format == 0 ? defaultRGBTextureFormat : gl_format;
	}

	void GFX::use_mipmapping(bool use)
	{
		build_mipmaps = use;
	}

	void GFX::detectDefaultSettings()
	{
        const GLubyte *glStr = glGetString(GL_VENDOR);
        QString glVendor = glStr ? ToUpper((const char*)glStr) : QString();

		enum VENDOR { Unknown, Ati, Nvidia, Sis, Intel };

		VENDOR glVendorID = Unknown;
        if (glVendor.contains("ATI"))
			glVendorID = Ati;
        else if (glVendor.contains("NVIDIA"))
			glVendorID = Nvidia;
        else if (glVendor.contains("SIS"))
			glVendorID = Sis;
        else if (glVendor.contains("INTEL"))
			glVendorID = Intel;

#ifdef TA3D_PLATFORM_LINUX
		switch(glVendorID)
		{
		case Ati:
			lp_CONFIG->shadow_quality = 3;
			lp_CONFIG->water_quality = 4;
			lp_CONFIG->fsaa = 0;
			lp_CONFIG->anisotropy = 1;

			lp_CONFIG->color_depth = 32;
			lp_CONFIG->shadowmap_size = 2;

			lp_CONFIG->particle = true;
			lp_CONFIG->explosion_particles = true;
			lp_CONFIG->waves = false;
			lp_CONFIG->far_sight = true;

			lp_CONFIG->detail_tex = true;

			lp_CONFIG->use_texture_cache = false;
			lp_CONFIG->use_texture_compression = false;

			lp_CONFIG->render_sky = true;
			lp_CONFIG->low_definition_map = false;

			lp_CONFIG->disable_GLSL = false;
			break;

		case Nvidia:
			lp_CONFIG->shadow_quality = 3;
			lp_CONFIG->water_quality = 4;
			lp_CONFIG->fsaa = 0;
			lp_CONFIG->anisotropy = 1;

			lp_CONFIG->color_depth = 32;
			lp_CONFIG->shadowmap_size = 2;

			lp_CONFIG->particle = true;
			lp_CONFIG->explosion_particles = true;
			lp_CONFIG->waves = false;
			lp_CONFIG->far_sight = true;

			lp_CONFIG->detail_tex = true;

			lp_CONFIG->use_texture_cache = true;
			lp_CONFIG->use_texture_compression = true;

			lp_CONFIG->render_sky = true;
			lp_CONFIG->low_definition_map = false;

			lp_CONFIG->disable_GLSL = false;
			break;

		case Sis:
		case Intel:
			lp_CONFIG->shadow_quality = 0;
			lp_CONFIG->water_quality = 0;
			lp_CONFIG->fsaa = 0;
			lp_CONFIG->anisotropy = 1;

			lp_CONFIG->color_depth = 16;
			lp_CONFIG->shadowmap_size = 0;

			lp_CONFIG->particle = true;
			lp_CONFIG->explosion_particles = false;
			lp_CONFIG->waves = false;
			lp_CONFIG->far_sight = false;

			lp_CONFIG->detail_tex = false;

			lp_CONFIG->use_texture_cache = false;
			lp_CONFIG->use_texture_compression = true;

			lp_CONFIG->render_sky = false;
			lp_CONFIG->low_definition_map = true;

			lp_CONFIG->disable_GLSL = true;
			break;

		case Unknown:
			lp_CONFIG->shadow_quality = 0;
			lp_CONFIG->water_quality = 0;
			lp_CONFIG->fsaa = 0;
			lp_CONFIG->anisotropy = 1;

			lp_CONFIG->color_depth = 32;
			lp_CONFIG->shadowmap_size = 0;

			lp_CONFIG->particle = true;
			lp_CONFIG->explosion_particles = false;
			lp_CONFIG->waves = false;
			lp_CONFIG->far_sight = false;

			lp_CONFIG->detail_tex = false;

			lp_CONFIG->use_texture_cache = false;
			lp_CONFIG->use_texture_compression = true;

			lp_CONFIG->render_sky = false;
			lp_CONFIG->low_definition_map = true;

			lp_CONFIG->disable_GLSL = true;
			break;
		};
#elif defined TA3D_PLATFORM_WINDOWS
		switch(glVendorID)
		{
		case Ati:
			lp_CONFIG->shadow_quality = 3;
			lp_CONFIG->water_quality = 4;
			lp_CONFIG->fsaa = 0;
			lp_CONFIG->anisotropy = 1;

			lp_CONFIG->color_depth = 32;
			lp_CONFIG->shadowmap_size = 2;

			lp_CONFIG->particle = true;
			lp_CONFIG->explosion_particles = true;
			lp_CONFIG->waves = false;
			lp_CONFIG->far_sight = true;

			lp_CONFIG->detail_tex = true;

			lp_CONFIG->use_texture_cache = false;
			lp_CONFIG->use_texture_compression = false;

			lp_CONFIG->render_sky = true;
			lp_CONFIG->low_definition_map = false;

			lp_CONFIG->disable_GLSL = false;
			break;

		case Nvidia:
			lp_CONFIG->shadow_quality = 3;
			lp_CONFIG->water_quality = 4;
			lp_CONFIG->fsaa = 0;
			lp_CONFIG->anisotropy = 1;

			lp_CONFIG->color_depth = 32;
			lp_CONFIG->shadowmap_size = 2;

			lp_CONFIG->particle = true;
			lp_CONFIG->explosion_particles = true;
			lp_CONFIG->waves = false;
			lp_CONFIG->far_sight = true;

			lp_CONFIG->detail_tex = true;

			lp_CONFIG->use_texture_cache = true;
			lp_CONFIG->use_texture_compression = true;

			lp_CONFIG->render_sky = true;
			lp_CONFIG->low_definition_map = false;

			lp_CONFIG->disable_GLSL = false;
			break;

		case Sis:
		case Intel:
			lp_CONFIG->shadow_quality = 0;
			lp_CONFIG->water_quality = 0;
			lp_CONFIG->fsaa = 0;
			lp_CONFIG->anisotropy = 1;

			lp_CONFIG->color_depth = 16;
			lp_CONFIG->shadowmap_size = 0;

			lp_CONFIG->particle = true;
			lp_CONFIG->explosion_particles = false;
			lp_CONFIG->waves = false;
			lp_CONFIG->far_sight = false;

			lp_CONFIG->detail_tex = false;

			lp_CONFIG->use_texture_cache = false;
			lp_CONFIG->use_texture_compression = true;

			lp_CONFIG->render_sky = false;
			lp_CONFIG->low_definition_map = true;

			lp_CONFIG->disable_GLSL = true;
			break;

		case Unknown:
			lp_CONFIG->shadow_quality = 0;
			lp_CONFIG->water_quality = 0;
			lp_CONFIG->fsaa = 0;
			lp_CONFIG->anisotropy = 1;

			lp_CONFIG->color_depth = 32;
			lp_CONFIG->shadowmap_size = 0;

			lp_CONFIG->particle = true;
			lp_CONFIG->explosion_particles = false;
			lp_CONFIG->waves = false;
			lp_CONFIG->far_sight = false;

			lp_CONFIG->detail_tex = false;

			lp_CONFIG->use_texture_cache = false;
			lp_CONFIG->use_texture_compression = true;

			lp_CONFIG->render_sky = false;
			lp_CONFIG->low_definition_map = true;

			lp_CONFIG->disable_GLSL = true;
			break;
		};
#elif defined TA3D_PLATFORM_MAC
		switch(glVendorID)
		{
		case Ati:
			lp_CONFIG->shadow_quality = 3;
			lp_CONFIG->water_quality = 4;
			lp_CONFIG->fsaa = 0;
			lp_CONFIG->anisotropy = 1;

			lp_CONFIG->color_depth = 32;
			lp_CONFIG->shadowmap_size = 2;

			lp_CONFIG->particle = true;
			lp_CONFIG->explosion_particles = true;
			lp_CONFIG->waves = false;
			lp_CONFIG->far_sight = true;

			lp_CONFIG->detail_tex = true;

			lp_CONFIG->use_texture_cache = false;
			lp_CONFIG->use_texture_compression = false;

			lp_CONFIG->render_sky = true;
			lp_CONFIG->low_definition_map = false;

			lp_CONFIG->disable_GLSL = false;
			break;

		case Nvidia:
			lp_CONFIG->shadow_quality = 3;
			lp_CONFIG->water_quality = 4;
			lp_CONFIG->fsaa = 0;
			lp_CONFIG->anisotropy = 1;

			lp_CONFIG->color_depth = 32;
			lp_CONFIG->shadowmap_size = 2;

			lp_CONFIG->particle = true;
			lp_CONFIG->explosion_particles = true;
			lp_CONFIG->waves = false;
			lp_CONFIG->far_sight = true;

			lp_CONFIG->detail_tex = true;

			lp_CONFIG->use_texture_cache = true;
			lp_CONFIG->use_texture_compression = true;

			lp_CONFIG->render_sky = true;
			lp_CONFIG->low_definition_map = false;

			lp_CONFIG->disable_GLSL = false;
			break;

		case Sis:
		case Intel:
			lp_CONFIG->shadow_quality = 0;
			lp_CONFIG->water_quality = 0;
			lp_CONFIG->fsaa = 0;
			lp_CONFIG->anisotropy = 1;

			lp_CONFIG->color_depth = 16;
			lp_CONFIG->shadowmap_size = 0;

			lp_CONFIG->particle = true;
			lp_CONFIG->explosion_particles = false;
			lp_CONFIG->waves = false;
			lp_CONFIG->far_sight = false;

			lp_CONFIG->detail_tex = false;

			lp_CONFIG->use_texture_cache = false;
			lp_CONFIG->use_texture_compression = true;

			lp_CONFIG->render_sky = false;
			lp_CONFIG->low_definition_map = true;

			lp_CONFIG->disable_GLSL = true;
			break;

		case Unknown:
			lp_CONFIG->shadow_quality = 0;
			lp_CONFIG->water_quality = 0;
			lp_CONFIG->fsaa = 0;
			lp_CONFIG->anisotropy = 1;

			lp_CONFIG->color_depth = 32;
			lp_CONFIG->shadowmap_size = 0;

			lp_CONFIG->particle = true;
			lp_CONFIG->explosion_particles = false;
			lp_CONFIG->waves = false;
			lp_CONFIG->far_sight = false;

			lp_CONFIG->detail_tex = false;

			lp_CONFIG->use_texture_cache = false;
			lp_CONFIG->use_texture_compression = true;

			lp_CONFIG->render_sky = false;
			lp_CONFIG->low_definition_map = true;

			lp_CONFIG->disable_GLSL = true;
			break;
		};
#endif
	}

    void GFX::initialize()
	{
        setSurfaceType(OpenGLSurface);

        QSurfaceFormat format;
        format.setStencilBufferSize(8);
        format.setSamples(std::max<int>(1, TA3D::VARS::lp_CONFIG->fsaa));
        format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
        if (TA3D::VARS::lp_CONFIG->color_depth == 32)
        {
            format.setRedBufferSize(8);
            format.setGreenBufferSize(8);
            format.setBlueBufferSize(8);
            format.setAlphaBufferSize(0);
        }
        else
        {
            format.setRedBufferSize(5);
            format.setGreenBufferSize(6);
            format.setBlueBufferSize(5);
            format.setAlphaBufferSize(0);
        }
        format.setDepthBufferSize(24);
        setFormat(format);

        const QSize wnd_size(TA3D::VARS::lp_CONFIG->screen_width,
                             TA3D::VARS::lp_CONFIG->screen_height);

        setMinimumSize(wnd_size);
        setMaximumSize(wnd_size);
        resize(wnd_size);

        if (TA3D::VARS::lp_CONFIG->fullscreen)
            showFullScreen();
        else
            setVisible(true);
        qApp->processEvents();

        if (!TA3D::VARS::lp_CONFIG->fullscreen)
        {
            // We want a centered window
            const QRect &visible_area = screen()->geometry();
            setPosition(visible_area.width() - TA3D::VARS::lp_CONFIG->screen_width >> 1,
                        visible_area.height() - TA3D::VARS::lp_CONFIG->screen_height >> 1);
        }

        m_context = new QOpenGLContext(this);
        m_context->setFormat(requestedFormat());
        if (!m_context->create())
        {
            LOG_ERROR(LOG_PREFIX_GFX << "Impossible to set OpenGL video mode!");
            criticalMessage("Impossible to set OpenGL video mode!");
            exit(1);
            return;
        }
        qApp->processEvents();

        if (!m_context->makeCurrent(this))
        {
            LOG_ERROR(LOG_PREFIX_GFX << "Impossible make OpenGL context current!");
            criticalMessage("Impossible make OpenGL context current!");
            exit(1);
            return;
        }

        initializeOpenGLFunctions();

        const QImage &icon = load_image("gfx/icon.png");
        if (!icon.isNull())
            setIcon(QIcon(QPixmap::fromImage(icon)));

		preCalculations();
		// Install OpenGL extensions
		installOpenGLExtensions();

        if (m_context->format().redBufferSize() == 8)
		{
            defaultRGBTextureFormat = GL_RGB8;
            defaultRGBATextureFormat = GL_RGBA8;
		}
		else
		{
            defaultRGBTextureFormat = GL_RGB5;
            defaultRGBATextureFormat = GL_RGB5_A1;
        }

		// Check everything is supported
		checkConfig();

		if(g_useTextureCompression && lp_CONFIG->use_texture_compression) // Try to enabled the Texture compression
			set_texture_format(GL_COMPRESSED_RGB_ARB);
		else
			set_texture_format(defaultRGBTextureFormat);
		glViewport(0,0,width,height);

		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_tex_size);
	}


	void GFX::checkConfig() const
	{
		if (!g_useFBO)
			lp_CONFIG->water_quality = Math::Min(lp_CONFIG->water_quality, sint16(1));
		if (!g_useProgram)
		{
			lp_CONFIG->water_quality = Math::Min(lp_CONFIG->water_quality, sint16(1));
			lp_CONFIG->disable_GLSL = true;
			lp_CONFIG->detail_tex = false;
			lp_CONFIG->shadow_quality = Math::Min(lp_CONFIG->shadow_quality, sint16(1));
		}
# ifdef TA3D_PLATFORM_MAC
		// For some reasons, the texture compression makes ta3d completely unstable on OS X,
		// at least with an ATI video card.
		// Currently, we don't know if it also affects NVidia cards
		lp_CONFIG->use_texture_compression = false;
# else
		if (!g_useTextureCompression)
			lp_CONFIG->use_texture_compression = false;
# endif

        if (!m_context->hasExtension("GL_ARB_shadow"))
			lp_CONFIG->shadow_quality = Math::Min(lp_CONFIG->shadow_quality, sint16(1));
	}


    bool GFX::checkVideoCardWorkaround()
	{
		// Check for ATI workarounds (if an ATI card is present)
        bool workaround = Substr(ToUpper((const char*)glGetString(GL_VENDOR)),0,3) == "ATI";
		// Check for SIS workarounds (if an SIS card is present) (the same than ATI)
        workaround |= ToUpper((const char*)glGetString(GL_VENDOR)).contains("SIS");
		return workaround;
	}


	GFX::GFX()
		:width(0), height(0), x(0), y(0),
		normal_font(NULL), small_font(NULL), TA_font(NULL), ta3d_gui_font(NULL), big_font(NULL),
		SCREEN_W_HALF(0), SCREEN_H_HALF(0), SCREEN_W_INV(0.), SCREEN_H_INV(0.), SCREEN_W_TO_640(0.), SCREEN_H_TO_480(0.),
		low_def_limit(600.), glfond(0), textureFBO(0), textureDepth(0), textureColor(0), shadowMap(0),
		model_shader(),
		ati_workaround(false), max_tex_size(0), default_texture(0),
		alpha_blending_set(false), texture_format(0), build_mipmaps(false), shadowMapMode(false),
		defaultRGBTextureFormat(GL_RGB8), defaultRGBATextureFormat(GL_RGBA8)
	{
        gfx = this;

        initialize();
        // Initialize the GFX Engine
		if (lp_CONFIG->first_start)
		{
			// Guess best settings
			detectDefaultSettings();
			lp_CONFIG->first_start = false;
		}
		ati_workaround = checkVideoCardWorkaround();

		LOG_DEBUG("Allocating palette memory...");
        TA3D::VARS::pal.resize(256);      // Allocate a new palette

		LOG_DEBUG("Loading TA's palette...");
        bool palette = TA3D::UTILS::load_palette(pal);
		if (!palette)
			LOG_WARNING("Failed to load the palette");

		InitInterface();
	}



	GFX::~GFX()
	{
		DeleteInterface();

		if (textureFBO)
            glDeleteFramebuffers(1,&textureFBO);
		if (textureDepth)
            glDeleteRenderbuffers(1,&textureDepth);
		destroy_texture(textureColor);
		destroy_texture(shadowMap);
		destroy_texture(default_texture);

		normal_font = NULL;
		small_font = NULL;
		TA_font = NULL;
		ta3d_gui_font = NULL;

		font_manager.destroy();
		Gui::skin_manager.destroy();
	}



	void GFX::loadDefaultTextures()
	{
		LOG_DEBUG(LOG_PREFIX_GFX << "Loading background...");
		load_background();

		LOG_DEBUG(LOG_PREFIX_GFX << "Loading default texture...");
		default_texture = load_texture("gfx/default.png");

		alpha_blending_set = false;
	}


	void GFX::loadFonts()
	{
		LOG_DEBUG(LOG_PREFIX_GFX << "Creating a normal font...");
		normal_font = font_manager.find("FreeSans", 10, Font::typeTexture);

		LOG_DEBUG(LOG_PREFIX_GFX << "Creating a small font...");
		small_font = font_manager.find("FreeMono", 8, Font::typeTexture);
		small_font->setBold(true);

		LOG_DEBUG(LOG_PREFIX_GFX << "Loading a big font...");
		TA_font = font_manager.find("FreeSans", 16, Font::typeTexture);

		LOG_DEBUG(LOG_PREFIX_GFX << "Loading the GUI font...");
		ta3d_gui_font = font_manager.find("FreeSerif", 10 * height / 480, Font::typeTexture);
		Gui::gui_font = ta3d_gui_font;

		LOG_DEBUG(LOG_PREFIX_GFX << "Loading a big scaled font...");
		big_font = font_manager.find("FreeSans", 16 * height / 480, Font::typeTexture);
	}


    void GFX::set_alpha(const float a)
	{
		float gl_color[4];
		glGetFloatv(GL_CURRENT_COLOR, gl_color);
		gl_color[3] = a;
		glColor4fv(gl_color);
	}


	void GFX::set_2D_mode()
	{
		glPushAttrib(GL_ALL_ATTRIB_BITS);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, width, height, 0, -1.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glDisable(GL_LIGHTING);
		glDisable(GL_CULL_FACE);
		glDisable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		glEnable(GL_TEXTURE_2D);

		glColor4ub(0xFF,0xFF,0xFF,0xFF);
	}

	void GFX::set_2D_clip_rectangle(int x, int y, int w, int h)
	{
		if (w == -1 || h == -1)
		{
			glDisable(GL_SCISSOR_TEST);
		}
		else
		{
			glScissor(x, height - (y + h), w, h);
			glEnable(GL_SCISSOR_TEST);
		}
	}

	void GFX::unset_2D_mode()
	{
		glPopAttrib();
	}

	void GFX::line(const float x1, const float y1, const float x2, const float y2)			// Basic drawing routines
	{
		float points[4] = { x1,y1, x2,y2 };
		glEnableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, points);
		glDrawArrays( GL_LINES, 0, 2 );
	}
	void GFX::line(const float x1, const float y1, const float x2, const float y2, const uint32 col)
	{
		set_color(col);
		line(x1,y1,x2,y2);
	}


	void GFX::rect(const float x1, const float y1, const float x2, const float y2)
	{
		float points[8] = { x1,y1, x2,y1, x2,y2, x1,y2 };
		glEnableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, points);
		glDrawArrays( GL_LINE_LOOP, 0, 4 );
	}
	void GFX::rect(const float x1, const float y1, const float x2, const float y2, const uint32 col)
	{
		set_color(col);
		rect(x1,y1,x2,y2);
	}


	void GFX::rectfill(const float x1, const float y1, const float x2, const float y2)
	{
		float points[8] = { x1,y1, x2,y1, x2,y2, x1,y2 };
		glEnableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, points);
		glDrawArrays( GL_QUADS, 0, 4 );
	}
	void GFX::rectfill(const float x1, const float y1, const float x2, const float y2, const uint32 col)
	{
		set_color(col);
		rectfill(x1,y1,x2,y2);
	}


	void GFX::circle(const float x, const float y, const float r)
	{
		float d_alpha = DB_PI / (r + 1.0f);
		int n = (int)(DB_PI / d_alpha) + 1;
		float *points = new float[n * 2];
		int i = 0;
		for (float alpha = 0.0f ; alpha <= DB_PI ; alpha += d_alpha)
		{
			points[i++] = x + r * cosf(alpha);
			points[i++] = y + r * sinf(alpha);
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, points);
		glDrawArrays( GL_LINE_LOOP, 0, i >> 1 );
		DELETE_ARRAY(points);
	}
	void GFX::circle(const float x, const float y, const float r, const uint32 col)
	{
		set_color(col);
		circle(x,y,r);
	}

	void GFX::circle_zoned(const float x, const float y, const float r, const float mx, const float my, const float Mx, const float My)
	{
		float d_alpha = DB_PI/(r+1.0f);
		int n = (int)(DB_PI / d_alpha) + 2;
		float *points = new float[n * 2];
		int i = 0;
		for (float alpha = 0.0f; alpha <= DB_PI; alpha+=d_alpha)
		{
			float rx = x+r*cosf(alpha);
			float ry = y+r*sinf(alpha);
			if (rx < mx )		rx = mx;
			else if (rx > Mx )	rx = Mx;
			if (ry < my )		ry = my;
			else if (ry > My )	ry = My;
			points[i++] = rx;
			points[i++] = ry;
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, points);
		glDrawArrays( GL_LINE_LOOP, 0, i>>1 );
		DELETE_ARRAY(points);
	}

	void GFX::dot_circle_zoned(const float t, const float x, const float y, const float r, const float mx, const float my, const float Mx, const float My)
	{
		float d_alpha = DB_PI/(r+1.0f);
		int n = (int)(DB_PI / d_alpha) + 2;
		float *points = new float[n * 2];
		int i = 0;
		for (float alpha = 0.0f; alpha <= DB_PI; alpha+=d_alpha)
		{
			float rx = x+r*cosf(alpha+t);
			float ry = y+r*sinf(alpha+t);
			if (rx < mx )		rx = mx;
			else if (rx > Mx )	rx = Mx;
			if (ry < my )		ry = my;
			else if (ry > My )	ry = My;
			points[i++] = rx;
			points[i++] = ry;
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, points);
		glDrawArrays( GL_LINES, 0, i>>1 );
		DELETE_ARRAY(points);
	}

	void GFX::dot_circle_zoned(const float t, const float x, const float y, const float r, const float mx, const float my, const float Mx, const float My, const uint32 col)
	{
		set_color(col);
		dot_circle_zoned(t,x,y,r,mx,my,Mx,My);
	}

	void GFX::circle_zoned(const float x, const float y, const float r, const float mx, const float my, const float Mx, const float My, const uint32 col)
	{
		set_color(col);
		circle_zoned(x,y,r,mx,my,Mx,My);
	}

	void GFX::circlefill(const float x, const float y, const float r)
	{
		float d_alpha = DB_PI/(r+1.0f);
		int n = (int)(DB_PI / d_alpha) + 4;
		float *points = new float[n * 2];
		int i = 0;
		points[i++] = x;
		points[i++] = y;
		for (float alpha = 0.0f; alpha <= DB_PI; alpha+=d_alpha)
		{
			points[i++] = x+r*cosf(alpha);
			points[i++] = y+r*sinf(alpha);
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, points);
		glDrawArrays( GL_TRIANGLE_FAN, 0, i>>1 );
		DELETE_ARRAY(points);
	}

	void GFX::circlefill(const float x, const float y, const float r, const uint32 col)
	{
		set_color(col);
		circlefill(x,y,r);
	}


	void GFX::rectdot(const float x1, const float y1, const float x2, const float y2)
	{
		glLineStipple(1, 0x5555);
		glEnable(GL_LINE_STIPPLE);
		rect(x1,y1,x2,y2);
		glDisable(GL_LINE_STIPPLE);
	}
	void GFX::rectdot(const float x1, const float y1, const float x2, const float y2, const uint32 col)
	{
		set_color(col);
		rectdot(x1,y1,x2,y2);
	}


	void GFX::drawtexture(const GLuint &tex, const float x1, const float y1, const float x2, const float y2, const float u1, const float v1, const float u2, const float v2)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex);

		const float points[8] = { x1,y1, x2,y1, x2,y2, x1,y2 };
		const float tcoord[8] = { u1,v1, u2,v1, u2,v2, u1,v2 };

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, points);
		glTexCoordPointer(2, GL_FLOAT, 0, tcoord);
		glDrawArrays(GL_QUADS, 0, 4);
	}


	void GFX::drawtexture(const GLuint &tex, const float x1, const float y1, const float x2, const float y2)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,tex);

		const float points[8] = { x1,y1, x2,y1, x2,y2, x1,y2 };
		static const float tcoord[8] = { 0.0f,0.0f, 1.0f,0.0f, 1.0f,1.0f, 0.0f,1.0f };

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, points);
		glTexCoordPointer(2, GL_FLOAT, 0, tcoord);
		glDrawArrays( GL_QUADS, 0, 4 );
	}


	void GFX::drawtexture_flip(const GLuint &tex, const float x1, const float y1, const float x2, const float y2)
	{
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D,tex);

		const float points[8] = { x1,y1, x2,y1, x2,y2, x1,y2 };
		static const float tcoord[8] = {0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f};

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, points);
		glTexCoordPointer(2, GL_FLOAT, 0, tcoord);
		glDrawArrays( GL_QUADS, 0, 4 );
	}


	void GFX::drawtexture(const GLuint &tex, const float x1, const float y1, const float x2, const float y2, const uint32 col)
	{
		set_color(col);
		drawtexture( tex, x1, y1, x2, y2 );
	}

	void GFX::drawtexture_flip(const GLuint &tex, const float x1, const float y1, const float x2, const float y2, const uint32 col)
	{
		set_color(col);
		drawtexture_flip(tex, x1, y1, x2, y2);
	}

    void GFX::print(Font *font, const float x, const float y, const float z, const QString &text)		// Font related routines
	{
        Q_ASSERT(NULL != font);
        if (!text.isEmpty())
		{
			ReInitTexSys(false);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_TEXTURE_2D);
			font->print(x, y, z, text);
		}
	}


    void GFX::print(Font *font, const float x, const float y, const float z, const uint32 col, const QString &text)		// Font related routines
	{
		set_color(col);
		print( font, x, y, z, text );
	}

    void GFX::print_center(Font *font, const float x, const float y, const float z, const QString &text)		// Font related routines
	{
		if (font)
		{
			ReInitTexSys( false );
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

			const float X = x - 0.5f * font->length( text );

			glEnable(GL_TEXTURE_2D);
			font->print(X, y, z, text);
		}
	}


    void GFX::print_center(Font *font, const float x, const float y, const float z, const uint32 col, const QString &text)		// Font related routines
	{
		set_color(col);
		print_center( font, x, y, z, text );
	}


    void GFX::print_right(Font *font, const float x, const float y, const float z, const QString &text)		// Font related routines
	{
		if (!font)  return;

		ReInitTexSys( false );
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

		float X = x - font->length( text );

		glEnable(GL_TEXTURE_2D);
		font->print(X, y, z, text);
	}

    void GFX::print_right(Font *font, const float x, const float y, const float z, const uint32 col, const QString &text)		// Font related routines
	{
		set_color(col);
		print_right( font, x, y, z, text );
	}

	int GFX::max_texture_size()
	{
		return max_tex_size;
	}

    GLuint GFX::make_texture(const QImage &bmp, int filter_type, bool clamp)
	{
        if (bmp.isNull())
		{
            LOG_WARNING(LOG_PREFIX_GFX << "make_texture used with empty QImage");
			return 0;
		}
		MutexLocker locker(pMutex);

        if (bmp.width() > max_tex_size || bmp.height() > max_tex_size)
		{
            QImage tmp = create_surface_ex( bmp.depth(),
                                            Math::Min(bmp.width(), max_tex_size),
                                            Math::Min(bmp.height(), max_tex_size));
            stretch_blit( bmp, tmp, 0, 0, bmp.width(), bmp.height(), 0, 0, tmp.width(), tmp.height() );
            return make_texture( tmp, filter_type, clamp );
		}

        if (!g_useNonPowerOfTwoTextures && (!Math::IsPowerOfTwo(bmp.width()) || !Math::IsPowerOfTwo(bmp.height())))
		{
            int w = 1 << Math::Log2(bmp.width());
            int h = 1 << Math::Log2(bmp.height());
            if (w < bmp.width()) w <<= 1;
            if (h < bmp.height()) h <<= 1;
            QImage tmp = create_surface_ex( bmp.depth(), w, h);
            stretch_blit_smooth( bmp, tmp, 0, 0, bmp.width(), bmp.height(), 0, 0, tmp.width(), tmp.height() );
            return make_texture( tmp, filter_type, clamp );
		}

		if (ati_workaround && filter_type != FILTER_NONE
            && ( !Math::IsPowerOfTwo(bmp.width()) || !Math::IsPowerOfTwo(bmp.height())))
			filter_type = FILTER_LINEAR;

		if (filter_type == FILTER_NONE || filter_type == FILTER_LINEAR )
			use_mipmapping(false);
		else
			use_mipmapping(true);

        bool can_useGenMipMaps = g_useGenMipMaps && (g_useNonPowerOfTwoTextures || (Math::IsPowerOfTwo(bmp.width()) && Math::IsPowerOfTwo(bmp.height())));

		GLuint gl_tex = 0;
		glGenTextures(1,&gl_tex);

		glBindTexture(GL_TEXTURE_2D, gl_tex);

		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);

		if (clamp)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		}
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, lp_CONFIG->anisotropy);

		switch (filter_type)
		{
        case FILTER_NONE:
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
            break;
        case FILTER_LINEAR:
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
            break;
        case FILTER_BILINEAR:
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
            break;
        case FILTER_TRILINEAR:
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
            break;
		}

#ifdef TA3D_PLATFORM_WINDOWS
		bool softMipMaps = build_mipmaps;
#else
        bool softMipMaps = !can_useGenMipMaps;
#endif

		//Upload image data to OpenGL
		if (!softMipMaps)
		{
            // Automatic mipmaps generation
            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE );
            switch (bmp.depth())
			{
            case 8:
                glTexImage2D(GL_TEXTURE_2D, 0, texture_format, bmp.width(), bmp.height(), 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, bmp.bits());
                break;
            case 24:
                glTexImage2D(GL_TEXTURE_2D, 0, texture_format, bmp.width(), bmp.height(), 0, GL_RGB, GL_UNSIGNED_BYTE, bmp.bits());
                break;
            case 32:
                glTexImage2D(GL_TEXTURE_2D, 0, texture_format, bmp.width(), bmp.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bmp.bits());
                break;
            default:
                LOG_DEBUG("QImage format not supported by texture loader: " << (int) bmp.depth() << " bpp" );
			}

            if (build_mipmaps)
                // Automatic mipmaps generation
                glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE );
		}
		else            // Generate mipmaps here since other methods are unreliable
		{
            int w = bmp.width() << 1;
            int h = bmp.height() << 1;
			int level = 0;
			if (w >= 1 && h >= 1)
				do
				{
					w = Math::Max(w / 2, 1);
					h = Math::Max(h / 2, 1);
                    QImage tmp = create_surface_ex( bmp.depth(), w, h);
                    stretch_blit(bmp, tmp, 0, 0, bmp.width(), bmp.height(), 0, 0, w, h);
                    switch (tmp.depth())
					{
						case 8:
                            glTexImage2D(GL_TEXTURE_2D, level, texture_format, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, tmp.bits());
							break;
						case 24:
                            glTexImage2D(GL_TEXTURE_2D, level, texture_format, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, tmp.bits());
							break;
						case 32:
                            glTexImage2D(GL_TEXTURE_2D, level, texture_format, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp.bits());
							break;
						default:
                            LOG_DEBUG("QImage format not supported by texture loader: " << (int) tmp.depth() << " bpp" );
					}
					++level;
				} while(w > 1 || h > 1);
		}

		if (filter_type == FILTER_NONE || filter_type == FILTER_LINEAR )
			use_mipmapping(true);

		return gl_tex;
	}


	GLuint GFX::make_texture_A32F( int w, int h, float *data, int filter_type, bool clamp )
	{
		MutexLocker locker(pMutex);

		set_texture_format(GL_ALPHA32F_ARB);

		use_mipmapping(false);

		GLuint gl_tex = create_texture(w,h,FILTER_NONE,clamp);

		use_mipmapping(true);
		glBindTexture(GL_TEXTURE_2D, gl_tex);

		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);

		if (clamp )
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		}

		switch(filter_type)
		{
			case FILTER_NONE:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
				break;
			case FILTER_LINEAR:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
				break;
			case FILTER_BILINEAR:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
				break;
			case FILTER_TRILINEAR:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
				break;
		}

		glTexImage2D( GL_TEXTURE_2D,
					  0,
					  GL_ALPHA32F_ARB,
					  w,
					  h,
					  0,
					  GL_ALPHA,
					  GL_FLOAT,
					  data );

		set_texture_format(defaultRGBTextureFormat);

		return gl_tex;
	}



	GLuint GFX::make_texture_A16F( int w, int h, float *data, int filter_type, bool clamp )
	{
		MutexLocker locker(pMutex);

		set_texture_format(GL_ALPHA16F_ARB);

		use_mipmapping(false);

		GLuint gl_tex = create_texture(w,h,FILTER_NONE,clamp);

		use_mipmapping(true);
		glBindTexture(GL_TEXTURE_2D, gl_tex);

		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);

        if (clamp)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		}

		switch(filter_type)
		{
        case FILTER_NONE:
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
            break;
        case FILTER_LINEAR:
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
            break;
        case FILTER_BILINEAR:
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
            break;
        case FILTER_TRILINEAR:
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
            break;
		}

		glTexImage2D( GL_TEXTURE_2D,
					  0,
					  GL_ALPHA16F_ARB,
					  w,
					  h,
					  0,
					  GL_ALPHA,
					  GL_FLOAT,
					  data );

		set_texture_format(defaultRGBTextureFormat);

		return gl_tex;
	}
	GLuint GFX::make_texture_RGBA32F( int w, int h, float *data, int filter_type, bool clamp )
	{
		MutexLocker locker(pMutex);

		set_texture_format(GL_RGBA32F_ARB);

		use_mipmapping(false);

		GLuint gl_tex = create_texture(w,h,FILTER_NONE,clamp);

		use_mipmapping(true);
		glBindTexture(GL_TEXTURE_2D, gl_tex);

		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);

		if (clamp )
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		}

		switch(filter_type)
		{
			case FILTER_NONE:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
				break;
			case FILTER_LINEAR:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
				break;
			case FILTER_BILINEAR:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
				break;
			case FILTER_TRILINEAR:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
				break;
		}

		glTexImage2D( GL_TEXTURE_2D,
					  0,
					  GL_RGBA32F_ARB,
					  w,
					  h,
					  0,
					  GL_RGBA,
					  GL_FLOAT,
					  data );

		set_texture_format(defaultRGBTextureFormat);

		return gl_tex;
	}

	GLuint GFX::make_texture_RGBA16F( int w, int h, float *data, int filter_type, bool clamp )
	{
		MutexLocker locker(pMutex);

		set_texture_format(GL_RGBA16F_ARB);

		use_mipmapping(false);

		GLuint gl_tex = create_texture(w,h,FILTER_NONE,clamp);

		use_mipmapping(true);
		glBindTexture(GL_TEXTURE_2D, gl_tex);

		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);

		if (clamp )
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		}

		switch(filter_type)
		{
			case FILTER_NONE:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
				break;
			case FILTER_LINEAR:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
				break;
			case FILTER_BILINEAR:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
				break;
			case FILTER_TRILINEAR:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
				break;
		}

		glTexImage2D( GL_TEXTURE_2D,
					  0,
					  GL_RGBA16F_ARB,
					  w,
					  h,
					  0,
					  GL_RGBA,
					  GL_FLOAT,
					  data );

		set_texture_format(defaultRGBTextureFormat);

		return gl_tex;
	}

	GLuint GFX::make_texture_RGB32F( int w, int h, float *data, int filter_type, bool clamp )
	{
		MutexLocker locker(pMutex);

		set_texture_format(GL_RGB32F_ARB);

		use_mipmapping(false);

		GLuint gl_tex = create_texture(w,h,FILTER_NONE,clamp);

		use_mipmapping(true);
		glBindTexture(GL_TEXTURE_2D, gl_tex);

		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);

		if (clamp )
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		}

		switch(filter_type)
		{
			case FILTER_NONE:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
				break;
			case FILTER_LINEAR:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
				break;
			case FILTER_BILINEAR:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
				break;
			case FILTER_TRILINEAR:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
				break;
		}

		glTexImage2D( GL_TEXTURE_2D,
					  0,
					  GL_RGB32F_ARB,
					  w,
					  h,
					  0,
					  GL_RGB,
					  GL_FLOAT,
					  data );

		set_texture_format(defaultRGBTextureFormat);

		return gl_tex;
	}

	GLuint GFX::make_texture_RGB16F( int w, int h, float *data, int filter_type, bool clamp )
	{
		MutexLocker locker(pMutex);

		set_texture_format(GL_RGB16F_ARB);

		use_mipmapping(false);

		GLuint gl_tex = create_texture(w,h,FILTER_NONE,clamp);

		use_mipmapping(true);
		glBindTexture(GL_TEXTURE_2D, gl_tex);

		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);

		if (clamp )
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		}

		switch(filter_type)
		{
			case FILTER_NONE:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
				break;
			case FILTER_LINEAR:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
				break;
			case FILTER_BILINEAR:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
				break;
			case FILTER_TRILINEAR:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
				break;
		}

		glTexImage2D( GL_TEXTURE_2D,
					  0,
					  GL_RGB16F_ARB,
					  w,
					  h,
					  0,
					  GL_RGB,
					  GL_FLOAT,
					  data );

		set_texture_format(defaultRGBTextureFormat);

		return gl_tex;
	}

    GLuint GFX::create_color_texture(uint32 color)
    {
        MutexLocker locker(pMutex);

        GLuint gl_tex = 0;
        glGenTextures(1,&gl_tex);

        glBindTexture(GL_TEXTURE_2D, gl_tex);

        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);

		glTexImage2D(GL_TEXTURE_2D, 0, defaultRGBATextureFormat, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &color);

        return gl_tex;
    }

	GLuint GFX::create_texture(int w, int h, int filter_type, bool clamp )
	{
		MutexLocker locker(pMutex);

		if (w > max_tex_size || h > max_tex_size )
			return create_texture( Math::Min(w, max_tex_size), Math::Min(h, max_tex_size), filter_type, clamp);

		if (ati_workaround && filter_type != FILTER_NONE
			&& ( !Math::IsPowerOfTwo(w) || !Math::IsPowerOfTwo(h)))
			filter_type = FILTER_LINEAR;

		GLuint gl_tex = 0;
		glGenTextures(1,&gl_tex);

		glBindTexture(GL_TEXTURE_2D, gl_tex);

		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);

		if (clamp )
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
		}

		switch(filter_type)
		{
			case FILTER_NONE:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
				break;
			case FILTER_LINEAR:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
				break;
			case FILTER_BILINEAR:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
				break;
			case FILTER_TRILINEAR:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
				break;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, texture_format, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		return gl_tex;
	}

	GLuint GFX::create_texture_RGB32F(int w, int h, int filter_type, bool clamp )
	{
        return make_texture_RGB32F(w, h, NULL, filter_type, clamp);
	}

	GLuint GFX::create_texture_RGBA32F(int w, int h, int filter_type, bool clamp )
	{
        return make_texture_RGBA32F(w, h, NULL, filter_type, clamp);
	}

	GLuint GFX::create_texture_RGB16F(int w, int h, int filter_type, bool clamp )
	{
        return make_texture_RGB16F(w, h, NULL, filter_type, clamp);
	}

	GLuint GFX::create_texture_RGBA16F(int w, int h, int filter_type, bool clamp )
	{
        return make_texture_RGBA16F(w, h, NULL, filter_type, clamp);
	}

    void GFX::blit_texture(const QImage &src, GLuint dst )
	{
		if(!dst)
			return;

		glBindTexture( GL_TEXTURE_2D, dst );

		glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);

        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, src.width(), src.height(), GL_RGBA, GL_UNSIGNED_BYTE, src.bits() );
	}

    QImage GFX::load_image(const QString &filename)
	{
        QIODevice *vfile = VFS::Instance()->readFile(filename);
		if (vfile)
        {
            QImage img;
            img.loadFromData(vfile->readAll());

			delete vfile;

            if (!img.isNull())
            {
                if (img.format() == QImage::Format_Indexed8)
                    img.setColorTable(TA3D::VARS::pal);
                if (img.hasAlphaChannel())
                    convert_format(img);
                else
                    convert_format_24(img);
            }
            else
                LOG_ERROR(LOG_PREFIX_GFX << "could not load image file: " << filename << " (vfs)");
            return img;
        }
        else
            LOG_ERROR(LOG_PREFIX_GFX << "could not read image file: " << filename << " (vfs)");
        return QImage();
	}


    GLuint GFX::load_texture(const QString& file, int filter_type, uint32 *width, uint32 *height, bool clamp, GLuint texFormat, bool *useAlpha, bool checkSize)
	{
        if (!VFS::Instance()->fileExists(file)) // The file doesn't exist
            return 0;

        const QString &upfile = file.toUpper() + " (" + QString::number(texFormat) + "-" + QString::number(filter_type) + ')';
		HashMap<Interfaces::GfxTexture>::Sparse::iterator it = textureIDs.find(upfile);
		if (it != textureIDs.end() && it->tex > 0)		// File already loaded
		{
			if (textureLoad[it->tex] > 0)
			{
				++textureLoad[it->tex];
				if (width)
					*width = it->getWidth();
				if (height)
					*height = it->getHeight();
				if (useAlpha)
					*useAlpha = textureAlpha.contains(it->tex);
				return it->tex;
			}
		}

		const bool compressible = texFormat == GL_COMPRESSED_RGB || texFormat == GL_COMPRESSED_RGBA || texFormat == 0;
        QString cache_filename = !file.isEmpty() ? file + ".bin" : QString();
		cache_filename.replace('/', 'S');
		cache_filename.replace('\\', 'S');
		if (compressible)
		{
			GLuint gltex = load_texture_from_cache(cache_filename, filter_type, width, height, clamp, useAlpha);
			if (gltex)
				return gltex;
		}

        QImage bmp = load_image(file);
        if (bmp.isNull())
		{
			LOG_ERROR("Failed to load texture `" << file << "`");
			return 0;
		}

		if (width)
            *width = bmp.width();
		if (height)
            *height = bmp.height();
        convert_format(bmp);

		if (checkSize)
		{
			const int maxTextureSizeAllowed = lp_CONFIG->getMaxTextureSizeAllowed();
            if (std::max(bmp.width(), bmp.height()) > maxTextureSizeAllowed)
                bmp = shrink(bmp, std::min(bmp.width(), maxTextureSizeAllowed), std::min(bmp.height(),maxTextureSizeAllowed));
		}

        const QString ext = Paths::ExtractFileExt(file).toLower();
		bool with_alpha = (ext == ".tga" || ext == ".png" || ext == ".tif");
		if (with_alpha)
		{
			with_alpha = false;
            for (int y = 0 ; y < bmp.height() && !with_alpha; ++y)
			{
                for (int x = 0; x < bmp.width() && !with_alpha; ++x)
					with_alpha |= geta(SurfaceInt(bmp, x, y)) != 255;
			}
		}
		if (useAlpha)
			*useAlpha = with_alpha;
		if (texFormat == 0)
		{
			if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
				set_texture_format(with_alpha ? GL_COMPRESSED_RGBA_ARB : GL_COMPRESSED_RGB_ARB);
			else
				set_texture_format(with_alpha ? defaultRGBATextureFormat : defaultRGBTextureFormat);
		}
		else
			set_texture_format(texFormat);
		GLuint gl_tex = make_texture(bmp, filter_type, clamp);
		if (gl_tex)
		{
            textureIDs[upfile] = Interfaces::GfxTexture(gl_tex, bmp.width(), bmp.height());
			textureFile[gl_tex] = upfile;
			textureLoad[gl_tex] = 1;
			if (with_alpha)
				textureAlpha.insert(gl_tex);

			if (compressible)
                save_texture_to_cache(cache_filename, gl_tex, bmp.width(), bmp.height(), with_alpha);
		}
		return gl_tex;
	}


    GLuint	GFX::load_texture_mask(const QString& file, uint32 level, int filter_type, uint32 *width, uint32 *height, bool clamp )
	{
        if (!VFS::Instance()->fileExists(file)) // The file doesn't exist
			return 0;

        QImage bmp = load_image(file);
        if (bmp.isNull() )	return 0;					// Operation failed
        if (width )		*width = bmp.width();
        if (height )	*height = bmp.height();
        if (bmp.depth() != 32 )
            convert_format( bmp );
		bool with_alpha = (Paths::ExtractFileExt(file).toLower() == "tga");
		if (with_alpha)
		{
			with_alpha = false;
            for (int y = 0 ; y < bmp.height() && !with_alpha ; y++ )
                for (int x = 0 ; x < bmp.width() && !with_alpha ; x++ )
					with_alpha |= (geta(SurfaceInt(bmp,x,y)) != 255);
		}
		else
		{
            for (int y = 0 ; y < bmp.height() ; y++ )
                for (int x = 0 ; x < bmp.width() ; x++ )
					SurfaceInt(bmp,x,y) |= makeacol(0,0,0,255);
		}

        for (int y = 0; y < bmp.height(); ++y)
		{
            for (int x = 0; x < bmp.width(); ++x)
			{
				const uint32 c = SurfaceInt(bmp,x,y);
				if (getr(c) < level && getg(c) < level && getb(c) < level)
				{
					SurfaceInt(bmp,x,y) = makeacol(getr(c), getg(c), getb(c), 0);
					with_alpha = true;
				}
			}
		}
		if(g_useTextureCompression && lp_CONFIG->use_texture_compression)
			set_texture_format( with_alpha ? GL_COMPRESSED_RGBA_ARB : GL_COMPRESSED_RGB_ARB );
		else
			set_texture_format( with_alpha ? defaultRGBATextureFormat : defaultRGBTextureFormat );
        return make_texture( bmp, filter_type, clamp );
	}


    bool GFX::is_texture_in_cache(const QString& file)
	{
		if (ati_workaround
			|| !lp_CONFIG->use_texture_cache
			|| !lp_CONFIG->use_texture_compression
			|| lp_CONFIG->developerMode)
			return false;
        QString realFile(TA3D::Paths::Caches);
		realFile += file;
		if (TA3D::Paths::Exists(realFile))
		{
            QFile cache_file(realFile);
            cache_file.open(QIODevice::ReadOnly);
			uint32 mod_hash;
			cache_file.read((char*)&mod_hash, sizeof( mod_hash ));
			cache_file.close();

            return mod_hash == hash<QString>()(TA3D_CURRENT_MOD); // Check if it corresponds to current mod
		}
		return false;
	}


    GLuint GFX::load_texture_from_cache(const QString& file, int filter_type, uint32 *width, uint32 *height, bool clamp, bool *useAlpha )
	{
		if (ati_workaround
			|| !lp_CONFIG->use_texture_cache
			|| !lp_CONFIG->use_texture_compression
			|| !g_useGenMipMaps
			|| !g_useNonPowerOfTwoTextures
			|| lp_CONFIG->developerMode)		// No caching in developer mode
			return 0;

        QString realFile(TA3D::Paths::Caches);
		realFile += file;
		if(TA3D::Paths::Exists(realFile))
		{
            QFile cache_file(realFile);
            cache_file.open(QIODevice::ReadOnly);
			uint32 mod_hash;
			cache_file.read((char*)&mod_hash, sizeof(mod_hash));

            if (mod_hash != hash<QString>()(TA3D_CURRENT_MOD)) // Doesn't correspond to current mod
			{
				cache_file.close();
				return 0;
			}

			if (useAlpha)
                *useAlpha = readChar(&cache_file);
			else
                readChar(&cache_file);

			uint32 rw, rh;
			cache_file.read((char*)&rw, 4);
			cache_file.read((char*)&rh, 4);
			if(width)  *width = rw;
			if(height) *height = rh;

			int lod_max = 0;
			GLint size, internal_format;

			cache_file.read( (char*)&lod_max, sizeof( lod_max ));

			GLuint	tex;
			glEnable(GL_TEXTURE_2D);
			glGenTextures(1,&tex);

			if (filter_type == FILTER_NONE || filter_type == FILTER_LINEAR)
				use_mipmapping(false);
			else
				use_mipmapping(true);

			glBindTexture( GL_TEXTURE_2D, tex );
            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);

			for(int lod = 0 ; lod < lod_max ; ++lod)
			{
				GLint w,h,border;
				cache_file.read( (char*)&size, sizeof( GLint ) );

				byte *img = new byte[size];

				cache_file.read( (char*)&internal_format, sizeof( GLint ) );
				cache_file.read( (char*)&border, sizeof( GLint ) );
				cache_file.read( (char*)&w, sizeof( GLint ) );
				cache_file.read( (char*)&h, sizeof( GLint ) );
				cache_file.read( (char*)img, size );
				if (lod == 0)
                    glCompressedTexImage2D( GL_TEXTURE_2D, 0, internal_format, w, h, border, size, img);
				else
					glCompressedTexSubImage2D( GL_TEXTURE_2D, lod, 0, 0, w, h, internal_format, size, img);

				DELETE_ARRAY(img);
				if (!build_mipmaps)
					break;
			}
			if (build_mipmaps)
                glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

			cache_file.close();

			if (filter_type == FILTER_NONE || filter_type == FILTER_LINEAR)
				use_mipmapping(true);

			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);

			if (clamp)
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
			}
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, lp_CONFIG->anisotropy);

			switch (filter_type)
			{
			case FILTER_NONE:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
				break;
			case FILTER_LINEAR:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
				break;
			case FILTER_BILINEAR:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
				break;
			case FILTER_TRILINEAR:
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
				break;
			}
			return tex;
		}
		return 0; // File isn't in cache
	}


    void GFX::save_texture_to_cache( QString file, GLuint tex, uint32 width, uint32 height, bool useAlpha )
	{
		if(ati_workaround
		   || !lp_CONFIG->use_texture_cache
		   || !lp_CONFIG->use_texture_compression
		   || !g_useGenMipMaps
		   || !g_useNonPowerOfTwoTextures
		   || lp_CONFIG->developerMode)
			return;

        file = TA3D::Paths::Caches + file;

		int rw = texture_width( tex ), rh = texture_height( tex );		// Also binds tex

        GLint compressed;
        glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, &compressed );
		// Do not save it if it's not compressed -> save disk space, and it would slow things down
		if(!compressed)
			return;

        QFile cache_file(file);
        cache_file.open(QIODevice::WriteOnly);

        if (!cache_file.isOpen())
			return;

        uint32 mod_hash = static_cast<uint32>(hash<QString>()(TA3D_CURRENT_MOD)); // Save a hash of current mod

		cache_file.write( (const char*)&mod_hash, sizeof( mod_hash ) );

        cache_file.putChar(useAlpha);
		cache_file.write( (const char*)&width, 4 );
		cache_file.write( (const char*)&height, 4 );

		int lod_max = Math::Max(Math::Log2(rw), Math::Log2(rh));
		if ((1 << lod_max) < rw && (1 << lod_max) < rh )
			lod_max++;

		cache_file.write( (const char*)&lod_max, sizeof( lod_max ) );

		for(int lod = 0 ; lod < lod_max ; ++lod)
		{
			GLint size, internal_format;

			glGetTexLevelParameteriv( GL_TEXTURE_2D, lod, GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB, &size );
			glGetTexLevelParameteriv( GL_TEXTURE_2D, lod, GL_TEXTURE_INTERNAL_FORMAT, &internal_format );

			byte *img = new byte[size];
			memset(img, 0, size);

            glGetCompressedTexImage( GL_TEXTURE_2D, lod, img );
			GLint w,h,border;
			glGetTexLevelParameteriv( GL_TEXTURE_2D, lod, GL_TEXTURE_BORDER, &border );
			glGetTexLevelParameteriv( GL_TEXTURE_2D, lod, GL_TEXTURE_WIDTH, &w );
			glGetTexLevelParameteriv( GL_TEXTURE_2D, lod, GL_TEXTURE_HEIGHT, &h );

			cache_file.write( (const char*)&size, sizeof( GLint ) );
			cache_file.write( (const char*)&internal_format, sizeof( GLint ) );
			cache_file.write( (const char*)&border, sizeof( GLint ) );
			cache_file.write( (const char*)&w, sizeof( GLint ) );
			cache_file.write( (const char*)&h, sizeof( GLint ) );
			cache_file.write( (const char*)img, size );

			DELETE_ARRAY(img);
		}

		cache_file.close();
	}



    GLuint GFX::load_masked_texture(const QString &file, QString mask, int filter_type )
	{
		if ( (!VFS::Instance()->fileExists(file)) || (!VFS::Instance()->fileExists(mask)))
			return 0; // The file doesn't exist

        QImage bmp = load_image(file);
        if (bmp.isNull())
			return 0;					// Operation failed
        const QImage &alpha = load_image(mask);
        if(alpha.isNull())
			return 0;
        for (int y = 0; y < bmp.height(); ++y)
		{
            for (int x = 0; x < bmp.width() ; ++x)
			{
				const uint32 c = SurfaceInt(bmp, x, y);
				SurfaceInt(bmp, x, y) = makeacol( getr(c), getg(c), getb(c), geta(SurfaceInt(alpha,x,y)) );
			}
		}
		if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
			set_texture_format(GL_COMPRESSED_RGBA_ARB);
		else
			set_texture_format(defaultTextureFormat_RGBA());
        return make_texture( bmp, filter_type );
	}


	uint32 GFX::texture_width(const GLuint gltex)
	{
		GLint width;
		glBindTexture( GL_TEXTURE_2D, gltex);
		glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width );
		return width;
	}

	uint32 GFX::texture_height(const GLuint gltex)
	{
		GLint height;
		glBindTexture( GL_TEXTURE_2D, gltex);
		glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height );
		return height;
	}


	void GFX::destroy_texture(GLuint& gltex)
	{
		if (gltex)						// Test if the texture exists
		{
			// Look for it in the texture table
			HashMap<int, GLuint>::Sparse::iterator it = textureLoad.find(gltex);
			if (it != textureLoad.end())
			{
				--(*it);			// Update load info
				if (*it > 0)		// Still used elsewhere, so don't delete it
				{
					gltex = 0;
					return;
				}
				textureLoad.erase(it);
				textureAlpha.erase(gltex);

                HashMap<QString, GLuint>::Sparse::iterator file_it = textureFile.find(gltex);
				if (file_it != textureFile.end())
				{
                    if (!file_it->isEmpty())
						textureIDs.erase(*file_it);
					textureFile.erase(file_it);
				}
			}
			glDeleteTextures(1, &gltex);		// Delete the OpenGL object
		}
		gltex = 0;						// The texture is destroyed
	}

	GLuint GFX::make_texture_from_screen( int filter_type)				// Copy pixel data from screen to a texture
	{
		GLuint gltex = create_texture(width, height, filter_type);

		glBindTexture(GL_TEXTURE_2D,gltex);
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, width, height, 0);

		return gltex;
	}


	void GFX::set_alpha_blending()
	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		alpha_blending_set = true;
	}


	void GFX::unset_alpha_blending()
	{
		glDisable(GL_BLEND);
		alpha_blending_set = false;
	}


	void GFX::ReInitTexSys(bool matrix_reset)
	{
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
		glDisable(GL_TEXTURE_GEN_S);
		glDisable(GL_TEXTURE_GEN_T);
		if (matrix_reset)
		{
			glMatrixMode(GL_TEXTURE);
			glLoadIdentity();
			glMatrixMode(GL_MODELVIEW);
		}
	}


	void GFX::ReInitAllTex(bool disable)
	{
		if (MultiTexturing)
		{
			for (unsigned int i = 0; i < 7; ++i)
			{
				glActiveTextureARB(GL_TEXTURE0_ARB + i);
				ReInitTexSys();
				glClientActiveTexture(GL_TEXTURE0_ARB + i);
				glDisableClientState(GL_TEXTURE_COORD_ARRAY);
				if (disable)
					glDisable(GL_TEXTURE_2D);
			}
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glClientActiveTexture(GL_TEXTURE0_ARB);
		}
	}


	void GFX::SetDefState()
	{
		glClearColor (0, 0, 0, 0);
		glShadeModel (GL_SMOOTH);
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		glDepthFunc( GL_LESS );
		glEnable (GL_DEPTH_TEST);
		glCullFace (GL_BACK);
		glEnable (GL_CULL_FACE);
		glHint(GL_FOG_HINT, GL_NICEST);
		glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
		glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
		glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);
        glFogi (GL_FOG_COORD_SRC, GL_FRAGMENT_DEPTH);
		glDisable(GL_BLEND);
		glEnable(GL_LIGHTING);
		ReInitTexSys();
		alpha_blending_set = false;
	}


	void GFX::ReInitArrays()
	{
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
	}


    uint32 GFX::InterfaceMsg(const uint32 MsgID, const QString &)
	{
		if (MsgID != TA3D_IM_GFX_MSG)
			return INTERFACE_RESULT_CONTINUE;
		return INTERFACE_RESULT_CONTINUE;
	}


	void GFX::clearAll()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}


	void GFX::clearScreen()
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}


	void GFX::clearDepth()
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}


	void GFX::disable_texturing()
	{
		glDisable( GL_TEXTURE_2D );
	}


	void GFX::enable_texturing()
	{
		glEnable( GL_TEXTURE_2D );
	}


	void GFX::preCalculations()
	{
        width = QWindow::width();
        height = QWindow::height();
		SCREEN_W_HALF = width >> 1;
		SCREEN_H_HALF = height >> 1;
		SCREEN_W_INV = 1.0f / float(width);
		SCREEN_H_INV = 1.0f / float(height);
		SCREEN_W_TO_640 = 640.0f / float(width);
		SCREEN_H_TO_480 = 480.0f / float(height);
	}


	void GFX::load_background()
	{
		if (width > 1024)
			glfond = load_texture("gfx/menu1280.jpg", FILTER_LINEAR);
		else
			glfond = load_texture(((width <= 800) ? "gfx/menu800.jpg" : "gfx/menu1024.jpg"), FILTER_LINEAR);
	}


	void GFX::renderToTexture(const GLuint tex, bool useDepth)
	{
		if (!g_useFBO && textureFBO != 0)                   // Renders to back buffer when FBO isn't available
		{
            glBindTexture(GL_TEXTURE_2D, textureFBO);        // Copy back buffer to target texture
			glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, texture_width(textureFBO), texture_height(textureFBO), 0);
			textureFBO = 0;
			glViewport(0, 0, width, height);           // Use default viewport
		}

		if (tex == 0)       // Release the texture
		{
			if (g_useFBO)
			{
                glBindFramebuffer(GL_FRAMEBUFFER,0);     // Bind the default FBO
				glViewport(0, 0, width, height);           // Use default viewport
			}
		}
		else
		{
			if (g_useFBO)               // If FBO extension is available then use it
			{
				if (textureFBO == 0)    // Generate a FBO if none has been created yet
				{
                    glGenFramebuffers(1,&textureFBO);
                    glGenRenderbuffers(1,&textureDepth);
				}

                glBindFramebuffer(GL_FRAMEBUFFER, textureFBO);					                    // Bind the FBO
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0); // Attach the texture
				if (useDepth)
				{
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, textureDepth);
                    glBindRenderbuffer(GL_RENDERBUFFER, textureDepth);
                    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, texture_width(tex), texture_height(tex));       // Should be enough
				}
				else
				{
                    glBindRenderbuffer(GL_RENDERBUFFER, 0);
                    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
				}
				glViewport(0,0,texture_width(tex),texture_height(tex));                                     // Stretch viewport to texture size
			}
			else                        // We're going to render to back buffer and then copy back our work :)
			{
				textureFBO = tex;       // Save this here
				glViewport(0,0,texture_width(tex),texture_height(tex));                                     // Stretch viewport to texture size
			}
		}
	}


	void GFX::renderToTextureDepth(const GLuint tex)
	{
		if (tex == 0)       // Release the texture
		{
            glBindFramebuffer(GL_FRAMEBUFFER, 0);     // Bind the default FBO
			glViewport(0, 0, width, height);           // Use default viewport
		}
		else
		{
			int tex_w = texture_width(tex);
			int tex_h = texture_height(tex);
			if (textureFBO == 0)    // Generate a FBO if none has been created yet
			{
                glGenFramebuffers(1, &textureFBO);
                glGenRenderbuffers(1, &textureDepth);
			}
			if (textureColor == 0)
				textureColor = create_texture(tex_w, tex_h, FILTER_NONE, true);
			else
			{
				glBindTexture(GL_TEXTURE_2D, textureColor);
				glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex_w, tex_h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
			}

            glBindFramebuffer(GL_FRAMEBUFFER, textureFBO);					                    // Bind the FBO
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColor, 0);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex, 0); // Attach the texture

			glViewport(0, 0, tex_w, tex_h);                                     // Stretch viewport to texture size
		}
	}


	void GFX::PutTextureInsideRect(const GLuint texture, const float x1, const float y1, const float x2, const float y2)
	{
		glBindTexture(GL_TEXTURE_2D, texture);
		glBegin(GL_QUADS);
		//
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(x1, y1);

		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(x2, y1);

		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(x2, y2);

		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(x1, y2);
		//
		glEnd();
	}

    QImage GFX::create_surface_ex(int bpp, int w, int h)
	{
        switch(bpp)
        {
        case 8:
            {
                QImage img(QSize(w, h), QImage::Format_Indexed8);
                img.setColorTable(TA3D::VARS::pal);
                return img;
            }
        case 16:    return QImage(QSize(w, h), QImage::Format_RGB16);
        case 24:    return QImage(QSize(w, h), QImage::Format_RGB888);
        case 32:    return QImage(QSize(w, h), QImage::Format_RGBA8888);
        }
        return QImage();
	}

    QImage GFX::create_surface(int w, int h)
	{
		return create_surface_ex(32, w, h);
	}

	void reset_keyboard()
	{
		clear_keybuf();
	}
	void reset_mouse()
	{
	}

    QImage GFX::LoadMaskedTextureToBmp(const QString& file, const QString& filealpha)
	{
		// Load the texture (32Bits)
        QImage bmp = gfx->load_image(file);
        LOG_ASSERT(!bmp.isNull());

		// Load the mask
        const QImage &alpha = gfx->load_image(filealpha);
        LOG_ASSERT(!alpha.isNull());

		// Apply the mask, pixel by pixel
        for (int y = 0; y < bmp.height(); ++y)
		{
            for (int x = 0; x < bmp.width(); ++x)
				SurfaceByte(bmp, (x << 2) + 3, y) = byte(SurfaceInt(alpha,x,y));
		}

		return bmp;
	}

	GLuint GFX::create_shadow_map(int w, int h)
	{
		GLuint shadowMapTexture;

		glGenTextures(1, &shadowMapTexture);
		glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
		glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0,
					  GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
		if (lp_CONFIG->shadow_quality == 2)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);       // We want fast shadows
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);       // We want smooth shadows
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		return shadowMapTexture;
	}

	void GFX::delete_shadow_map()
	{
		if (shadowMap)
			destroy_texture(shadowMap);
		shadowMap = 0;
	}

	GLuint GFX::get_shadow_map()
	{
		if (shadowMap == 0)
		{
			switch(lp_CONFIG->shadowmap_size)
			{
			case 0:
				shadowMap = create_shadow_map(256, 256);
				break;
			case 1:
				shadowMap = create_shadow_map(512, 512);
				break;
			case 2:
				shadowMap = create_shadow_map(1024, 1024);
				break;
			case 3:
				shadowMap = create_shadow_map(2048, 2048);
				break;
			default:
				shadowMap = create_shadow_map(1024, 1024);
			};
		}
		return shadowMap;
	}

	void GFX::enable_model_shading(int mode)
	{
		if (shadowMapMode)
			return;
		switch(lp_CONFIG->shadow_quality)
		{
		case 3:
		case 2:
			switch(mode)
			{
				case 0:
				case 1:
					{
						#ifndef TA3D_PLATFORM_MAC
						if (!model_shader.isLoaded())
							model_shader.load("shaders/3do_shadow.frag", "shaders/3do_shadow.vert");
						if (model_shader.isLoaded())
						{
							model_shader.on();
							model_shader.setvar1i("shadowMap", 7);
							model_shader.setmat4f("light_Projection", shadowMapProjectionMatrix);
						}
						# endif
						break;
					}
				default:
					model_shader.off();
			}
			break;
		default:
			model_shader.off();
		}
	}


	void GFX::disable_model_shading()
	{
		model_shader.off();
	}

	void GFX::setShadowMapMode(bool mode)
	{
		shadowMapMode = mode;
	}

	bool GFX::getShadowMapMode()
	{
		return shadowMapMode;
	}

	void GFX::readShadowMapProjectionMatrix()
	{
		GLfloat backup[16];
		glGetFloatv(GL_PROJECTION_MATRIX, backup);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glScalef(0.5f, 0.5f, 0.5f);
		glTranslatef(1.0f, 1.0f, 1.0f);
		glMultMatrixf(backup);
		glGetFloatv(GL_PROJECTION_MATRIX, gfx->shadowMapProjectionMatrix);

		glLoadIdentity();
		glMultMatrixf(backup);
		glMatrixMode(GL_MODELVIEW);
	}

    void GFX::enableShadowMapping()
	{
		glActiveTexture(GL_TEXTURE7);
		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
	}

    void GFX::disableShadowMapping()
	{
		glActiveTexture(GL_TEXTURE7);
		glDisable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
	}

	void GFX::storeShadowMappingState()
	{
		glActiveTexture(GL_TEXTURE7);
		shadowMapWasActive = glIsEnabled(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
	}

    void GFX::restoreShadowMappingState()
	{
		if (shadowMapWasActive)
			enableShadowMapping();
		else
			disableShadowMapping();
	}

	GLuint GFX::defaultTextureFormat_RGB_compressed() const
	{
		if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
			return GL_COMPRESSED_RGB_ARB;
		return defaultRGBTextureFormat;
	}

	GLuint GFX::defaultTextureFormat_RGBA_compressed() const
	{
		if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
			return GL_COMPRESSED_RGBA_ARB;
		return defaultRGBATextureFormat;
	}

    void GFX::flip()
    {
        m_context->swapBuffers(this);
    }

    // ---- Input events management ----
    void GFX::keyPressEvent(QKeyEvent *e)
    {
        QWindow::keyPressEvent(e);
        set_key_down(e->key());
    }

    void GFX::keyReleaseEvent(QKeyEvent *e)
    {
        QWindow::keyReleaseEvent(e);
        set_key_up(e->key());
    }

    void GFX::mousePressEvent(QMouseEvent *e)
    {
        QWindow::mousePressEvent(e);
        mouse_b = (e->buttons() & 7);
        mouse_x = e->x();
        mouse_y = e->y();
    }

    void GFX::mouseReleaseEvent(QMouseEvent *e)
    {
        QWindow::mouseReleaseEvent(e);
        mouse_b = (e->buttons() & 7);
        mouse_x = e->x();
        mouse_y = e->y();
    }

    void GFX::wheelEvent(QWheelEvent *e)
    {
        QWindow::wheelEvent(e);
        mouse_z += e->delta();
    }

    void GFX::mouseMoveEvent(QMouseEvent *e)
    {
        QWindow::mouseMoveEvent(e);
        mouse_b = (e->buttons() & 7);
        mouse_x = e->x();
        mouse_y = e->y();
    }
} // namespace TA3D
