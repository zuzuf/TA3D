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

    bool checkGLerror(const char *filename, int line)
    {
        GLuint err = glGetError();
        switch(err)
        {
        case GL_NO_ERROR:   return false;
#define IMPL_ERROR(X)   case X: std::cout << filename << " l." << line << " OpenGL error : " #X << std::endl; break;
        IMPL_ERROR(GL_INVALID_ENUM)
        IMPL_ERROR(GL_INVALID_VALUE)
        IMPL_ERROR(GL_INVALID_OPERATION)
        IMPL_ERROR(GL_STACK_OVERFLOW)
        IMPL_ERROR(GL_STACK_UNDERFLOW)
        IMPL_ERROR(GL_OUT_OF_MEMORY)
        IMPL_ERROR(GL_INVALID_FRAMEBUFFER_OPERATION)
#undef IMPL_ERROR
        default:
            std::cout << filename << " l." << line << " Unknown OpenGL error : " << err << std::endl;
        }
        return true;
    }

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
        CHECK_GL();
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
        format.setProfile(QSurfaceFormat::CompatibilityProfile);
        format.setVersion(2,0);
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
            setPosition((visible_area.width() - TA3D::VARS::lp_CONFIG->screen_width) >> 1,
                        (visible_area.height() - TA3D::VARS::lp_CONFIG->screen_height) >> 1);
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
        CHECK_GL();

		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_tex_size);
        CHECK_GL();

        loadShaders();
    }

    void GFX::destroy_background()
    {
        glbackground = nullptr;
    }

	void GFX::checkConfig() const
	{
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
		SCREEN_W_HALF(0), SCREEN_H_HALF(0), SCREEN_W_INV(0.), SCREEN_H_INV(0.), SCREEN_W_TO_640(0.), SCREEN_H_TO_480(0.),
        low_def_limit(600.), textureFBO(0),
		model_shader(),
        ati_workaround(false), max_tex_size(0),
		alpha_blending_set(false), texture_format(0), build_mipmaps(false), shadowMapMode(false),
		defaultRGBTextureFormat(GL_RGB8), defaultRGBATextureFormat(GL_RGBA8)
	{
        m_context = nullptr;

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

        if (m_context)
            m_context->makeCurrent(this);

		if (textureFBO)
        {
            glDeleteFramebuffers(1,&textureFBO);
            CHECK_GL();
        }

        textureColor = nullptr;
        shadowMap = nullptr;
        default_texture = nullptr;

        model_shader = nullptr;
        drawing2d_color_shader = nullptr;
        drawing2d_texture_shader = nullptr;
        particle_shader = nullptr;

		normal_font = NULL;
		small_font = NULL;
		TA_font = NULL;
		ta3d_gui_font = NULL;

		font_manager.destroy();
		Gui::skin_manager.destroy();

        if (m_context)
            delete m_context;
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
        normal_font = font_manager.find("FreeSans", 10);

		LOG_DEBUG(LOG_PREFIX_GFX << "Creating a small font...");
        small_font = font_manager.find("FreeMono", 8);
		small_font->setBold(true);

		LOG_DEBUG(LOG_PREFIX_GFX << "Loading a big font...");
        TA_font = font_manager.find("FreeSans", 16);

		LOG_DEBUG(LOG_PREFIX_GFX << "Loading the GUI font...");
        ta3d_gui_font = font_manager.find("FreeSerif", 10 * height / 480);
		Gui::gui_font = ta3d_gui_font;

		LOG_DEBUG(LOG_PREFIX_GFX << "Loading a big scaled font...");
        big_font = font_manager.find("FreeSans", 16 * height / 480);
	}


    void GFX::set_alpha(const float a)
	{
		float gl_color[4];
		glGetFloatv(GL_CURRENT_COLOR, gl_color);
        CHECK_GL();
        gl_color[3] = a;
		glColor4fv(gl_color);
        CHECK_GL();
    }


	void GFX::set_2D_mode()
	{
		glPushAttrib(GL_ALL_ATTRIB_BITS);
        CHECK_GL();

		glMatrixMode(GL_PROJECTION);
        CHECK_GL();
        glLoadIdentity();
        CHECK_GL();
        glOrtho(0, width, height, 0, -1.0, 1.0);
        CHECK_GL();
        glMatrixMode(GL_MODELVIEW);
        CHECK_GL();
        glLoadIdentity();
        CHECK_GL();

		glDisable(GL_LIGHTING);
        CHECK_GL();
        glDisable(GL_CULL_FACE);
        CHECK_GL();
        glDisable(GL_BLEND);
        CHECK_GL();
        glDisable(GL_DEPTH_TEST);
        CHECK_GL();
        glDepthMask(GL_FALSE);
        CHECK_GL();
        glEnable(GL_TEXTURE_2D);
        CHECK_GL();

		glColor4ub(0xFF,0xFF,0xFF,0xFF);
        CHECK_GL();
    }

    QMatrix4x4 GFX::get2Dmatrix()
    {
        QMatrix4x4 M;
        M.ortho(0, width, height, 0, -1.0, 1.0);
        return M;
    }

	void GFX::set_2D_clip_rectangle(int x, int y, int w, int h)
	{
		if (w == -1 || h == -1)
		{
			glDisable(GL_SCISSOR_TEST);
            CHECK_GL();
        }
		else
		{
			glScissor(x, height - (y + h), w, h);
            CHECK_GL();
            glEnable(GL_SCISSOR_TEST);
            CHECK_GL();
        }
	}

	void GFX::unset_2D_mode()
	{
		glPopAttrib();
        CHECK_GL();
    }

    void GFX::line_loop(const Vector2D *pts, const size_t nb_elts, const uint32 col)
    {
        drawing2d_color_shader->bind();
        CHECK_GL();
        drawing2d_color_shader->setAttributeArray(0, (GLfloat*)pts, 2);
        CHECK_GL();
        drawing2d_color_shader->enableAttributeArray(0);
        CHECK_GL();
        drawing2d_color_shader->disableAttributeArray(1);
        CHECK_GL();
        drawing2d_color_shader->setAttributeValue(1,
                                            getr(col) * (1.f / 255.f),
                                            getg(col) * (1.f / 255.f),
                                            getb(col) * (1.f / 255.f),
                                            geta(col) * (1.f / 255.f));
        CHECK_GL();
        glDrawArrays(GL_LINE_LOOP, 0, nb_elts);
        CHECK_GL();
        drawing2d_color_shader->disableAttributeArray(0);
        CHECK_GL();
        drawing2d_color_shader->release();
        CHECK_GL();
    }

    void GFX::lines(const Vector2D *pts, const size_t nb_elts, const uint32 col)
    {
        drawing2d_color_shader->bind();
        CHECK_GL();
        drawing2d_color_shader->setAttributeArray(0, (GLfloat*)pts, 2);
        CHECK_GL();
        drawing2d_color_shader->enableAttributeArray(0);
        CHECK_GL();
        drawing2d_color_shader->disableAttributeArray(1);
        CHECK_GL();
        drawing2d_color_shader->setAttributeValue(1,
                                            getr(col) * (1.f / 255.f),
                                            getg(col) * (1.f / 255.f),
                                            getb(col) * (1.f / 255.f),
                                            geta(col) * (1.f / 255.f));
        CHECK_GL();
        glDrawArrays(GL_LINES, 0, nb_elts);
        CHECK_GL();
        drawing2d_color_shader->disableAttributeArray(0);
        CHECK_GL();
        drawing2d_color_shader->release();
        CHECK_GL();
    }

    void GFX::line(const float x1, const float y1, const float x2, const float y2, const uint32 col)
	{
        GLfloat points[4] = { x1,y1, x2,y2 };
        drawing2d_color_shader->bind();
        CHECK_GL();
        drawing2d_color_shader->setAttributeArray(0, points, 2);
        CHECK_GL();
        drawing2d_color_shader->enableAttributeArray(0);
        CHECK_GL();
        drawing2d_color_shader->disableAttributeArray(1);
        CHECK_GL();
        drawing2d_color_shader->setAttributeValue(1,
                                            getr(col) * (1.f / 255.f),
                                            getg(col) * (1.f / 255.f),
                                            getb(col) * (1.f / 255.f),
                                            geta(col) * (1.f / 255.f));
        CHECK_GL();
        glDrawArrays(GL_LINES, 0, 2);
        CHECK_GL();
        drawing2d_color_shader->disableAttributeArray(0);
        CHECK_GL();
        drawing2d_color_shader->release();
        CHECK_GL();
    }

	void GFX::rect(const float x1, const float y1, const float x2, const float y2, const uint32 col)
	{
        GLfloat points[8] = { x1,y1, x2,y1, x2,y2, x1,y2 };
        drawing2d_color_shader->bind();
        CHECK_GL();
        drawing2d_color_shader->setAttributeArray(0, points, 2);
        CHECK_GL();
        drawing2d_color_shader->enableAttributeArray(0);
        CHECK_GL();
        drawing2d_color_shader->disableAttributeArray(1);
        CHECK_GL();
        drawing2d_color_shader->setAttributeValue(1,
                                            getr(col) * (1.f / 255.f),
                                            getg(col) * (1.f / 255.f),
                                            getb(col) * (1.f / 255.f),
                                            geta(col) * (1.f / 255.f));
        CHECK_GL();
        glDrawArrays(GL_LINE_LOOP, 0, 4);
        CHECK_GL();
        drawing2d_color_shader->disableAttributeArray(0);
        CHECK_GL();
        drawing2d_color_shader->release();
        CHECK_GL();
    }


	void GFX::rectfill(const float x1, const float y1, const float x2, const float y2, const uint32 col)
	{
        GLfloat points[8] = { x1,y1, x2,y1,
                              x1,y2, x2,y2 };
        drawing2d_color_shader->bind();
        CHECK_GL();
        drawing2d_color_shader->setAttributeArray(0, points, 2);
        CHECK_GL();
        drawing2d_color_shader->enableAttributeArray(0);
        CHECK_GL();
        drawing2d_color_shader->disableAttributeArray(1);
        CHECK_GL();
        drawing2d_color_shader->setAttributeValue(1,
                                            getr(col) * (1.f / 255.f),
                                            getg(col) * (1.f / 255.f),
                                            getb(col) * (1.f / 255.f),
                                            geta(col) * (1.f / 255.f));
        CHECK_GL();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        CHECK_GL();
        drawing2d_color_shader->disableAttributeArray(0);
        CHECK_GL();
        drawing2d_color_shader->release();
        CHECK_GL();
    }


	void GFX::circle(const float x, const float y, const float r, const uint32 col)
	{
        float d_alpha = DB_PI / (r + 1.0f);
        int n = std::max<int>(128, (int)(DB_PI / d_alpha) + 1);
        d_alpha = DB_PI / (n - 1);
        GLfloat points[256];
        int i = 0;
        for (float alpha = 0.0f ; alpha <= DB_PI ; alpha += d_alpha)
        {
            points[i++] = x + r * cosf(alpha);
            points[i++] = y + r * sinf(alpha);
        }

        drawing2d_color_shader->bind();
        CHECK_GL();
        drawing2d_color_shader->setAttributeArray(0, points, 2);
        CHECK_GL();
        drawing2d_color_shader->enableAttributeArray(0);
        CHECK_GL();
        drawing2d_color_shader->disableAttributeArray(1);
        CHECK_GL();
        drawing2d_color_shader->setAttributeValue(1,
                                            getr(col) * (1.f / 255.f),
                                            getg(col) * (1.f / 255.f),
                                            getb(col) * (1.f / 255.f),
                                            geta(col) * (1.f / 255.f));
        CHECK_GL();
        glDrawArrays(GL_LINE_LOOP, 0, n);
        CHECK_GL();
        drawing2d_color_shader->disableAttributeArray(0);
        CHECK_GL();
        drawing2d_color_shader->release();
        CHECK_GL();
    }

	void GFX::dot_circle_zoned(const float t, const float x, const float y, const float r, const float mx, const float my, const float Mx, const float My, const uint32 col)
	{
        float d_alpha = DB_PI / (r + 1.0f);
        int n = std::max<int>(128, (int)(DB_PI / d_alpha) + 1);
        d_alpha = DB_PI / (n - 1);
        GLfloat points[256];
        int i = 0;
        for (float alpha = 0.0f ; alpha <= DB_PI ; alpha += d_alpha)
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

        drawing2d_color_shader->bind();
        CHECK_GL();
        drawing2d_color_shader->setAttributeArray(0, points, 2);
        CHECK_GL();
        drawing2d_color_shader->enableAttributeArray(0);
        CHECK_GL();
        drawing2d_color_shader->disableAttributeArray(1);
        CHECK_GL();
        drawing2d_color_shader->setAttributeValue(1,
                                            getr(col) * (1.f / 255.f),
                                            getg(col) * (1.f / 255.f),
                                            getb(col) * (1.f / 255.f),
                                            geta(col) * (1.f / 255.f));
        CHECK_GL();
        glDrawArrays(GL_LINE_LOOP, 0, n);
        CHECK_GL();
        drawing2d_color_shader->disableAttributeArray(0);
        CHECK_GL();
        drawing2d_color_shader->release();
        CHECK_GL();
    }

	void GFX::circle_zoned(const float x, const float y, const float r, const float mx, const float my, const float Mx, const float My, const uint32 col)
	{
        float d_alpha = DB_PI / (r + 1.0f);
        int n = std::max<int>(128, (int)(DB_PI / d_alpha) + 1);
        d_alpha = DB_PI / (n - 1);
        GLfloat points[256];
        int i = 0;
        for (float alpha = 0.0f ; alpha <= DB_PI ; alpha += d_alpha)
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

        drawing2d_color_shader->bind();
        CHECK_GL();
        drawing2d_color_shader->setAttributeArray(0, points, 2);
        CHECK_GL();
        drawing2d_color_shader->enableAttributeArray(0);
        CHECK_GL();
        drawing2d_color_shader->disableAttributeArray(1);
        CHECK_GL();
        drawing2d_color_shader->setAttributeValue(1,
                                            getr(col) * (1.f / 255.f),
                                            getg(col) * (1.f / 255.f),
                                            getb(col) * (1.f / 255.f),
                                            geta(col) * (1.f / 255.f));
        CHECK_GL();
        glDrawArrays(GL_LINE_LOOP, 0, n);
        CHECK_GL();
        drawing2d_color_shader->disableAttributeArray(0);
        CHECK_GL();
        drawing2d_color_shader->release();
        CHECK_GL();
    }

	void GFX::circlefill(const float x, const float y, const float r)
	{
		float d_alpha = DB_PI/(r+1.0f);
		int n = (int)(DB_PI / d_alpha) + 4;
        float points[n * 2];
		int i = 0;
		points[i++] = x;
		points[i++] = y;
		for (float alpha = 0.0f; alpha <= DB_PI; alpha+=d_alpha)
		{
			points[i++] = x+r*cosf(alpha);
			points[i++] = y+r*sinf(alpha);
		}
        CHECK_GL();
        glEnableClientState(GL_VERTEX_ARRAY);
        CHECK_GL();
        glDisableClientState(GL_NORMAL_ARRAY);
        CHECK_GL();
        glDisableClientState(GL_COLOR_ARRAY);
        CHECK_GL();
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        CHECK_GL();
        glVertexPointer(2, GL_FLOAT, 0, points);
        CHECK_GL();
        glDrawArrays( GL_TRIANGLE_FAN, 0, i>>1 );
        CHECK_GL();
	}

	void GFX::circlefill(const float x, const float y, const float r, const uint32 col)
	{
		set_color(col);
		circlefill(x,y,r);
	}


	void GFX::rectdot(const float x1, const float y1, const float x2, const float y2, const uint32 col)
	{
        glLineStipple(1, 0x5555);
        CHECK_GL();
        glEnable(GL_LINE_STIPPLE);
        CHECK_GL();
        rect(x1,y1,x2,y2,col);
        glDisable(GL_LINE_STIPPLE);
        CHECK_GL();
    }

    void GFX::drawtexture(const GfxTexture::Ptr &tex,
                          const float x, const float y,
                          const uint32 col)
    {
        drawtexture(tex, x, y, x + tex->width(), y + tex->height(), 0, 0, 1, 1, col);
    }

    void GFX::drawtexture(const GfxTexture::Ptr &tex,
                          const float x1, const float y1,
                          const float x2, const float y2,
                          const uint32 col)
    {
        drawtexture(tex, x1, y1, x2, y2, 0, 0, 1, 1, col);
    }

    void GFX::drawtexture(const GfxTexture::Ptr &tex,
                          const float x1, const float y1,
                          const float x2, const float y2,
                          const float u1, const float v1,
                          const float u2, const float v2,
                          const uint32 col)
	{
        CHECK_GL();
        gfx->glActiveTexture(GL_TEXTURE0);
        CHECK_GL();
        tex->bind();
        CHECK_GL();

        GLfloat points[8] = { x1,y1, x2,y1,
                              x1,y2, x2,y2 };
        GLfloat tcoord[8] = { u1,v1, u2,v1,
                              u1,v2, u2,v2 };
        drawing2d_texture_shader->bind();
        CHECK_GL();
        drawing2d_texture_shader->setAttributeArray(0, points, 2);
        CHECK_GL();
        drawing2d_texture_shader->enableAttributeArray(0);
        CHECK_GL();
        drawing2d_texture_shader->disableAttributeArray(1);
        CHECK_GL();
        drawing2d_texture_shader->setAttributeValue(1,
                                                    getr(col) * (1.f / 255.f),
                                                    getg(col) * (1.f / 255.f),
                                                    getb(col) * (1.f / 255.f),
                                                    geta(col) * (1.f / 255.f));
        CHECK_GL();
        drawing2d_texture_shader->setAttributeArray(2, tcoord, 2);
        CHECK_GL();
        drawing2d_texture_shader->enableAttributeArray(2);
        CHECK_GL();
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        CHECK_GL();
        drawing2d_texture_shader->disableAttributeArray(0);
        CHECK_GL();
        drawing2d_texture_shader->disableAttributeArray(2);
        CHECK_GL();
        drawing2d_texture_shader->release();
        CHECK_GL();
    }

	int GFX::max_texture_size()
	{
		return max_tex_size;
	}

    GfxTexture::Ptr GFX::make_texture(const QImage &bmp, int filter_type, bool clamp)
    {
        GfxTexture::Ptr tex = new GfxTexture(bmp, filter_type <= FILTER_LINEAR ? GfxTexture::DontGenerateMipMaps : GfxTexture::GenerateMipMaps);
        if (clamp)
            tex->setWrapMode(GfxTexture::ClampToEdge);
        else
            tex->setWrapMode(GfxTexture::Repeat);

        tex->setMaximumAnisotropy(lp_CONFIG->anisotropy);

        switch (filter_type)
        {
        case FILTER_NONE:
            tex->setMinificationFilter(GfxTexture::Nearest);
            tex->setMagnificationFilter(GfxTexture::Nearest);
            break;
        case FILTER_LINEAR:
            tex->setMinificationFilter(GfxTexture::Linear);
            tex->setMagnificationFilter(GfxTexture::Linear);
            break;
        case FILTER_BILINEAR:
            tex->setMinificationFilter(GfxTexture::LinearMipMapNearest);
            tex->setMagnificationFilter(GfxTexture::Linear);
            break;
        case FILTER_TRILINEAR:
            tex->setMinificationFilter(GfxTexture::LinearMipMapLinear);
            tex->setMagnificationFilter(GfxTexture::Linear);
            break;
        }

        return tex;
	}

    GfxTexture::Ptr GFX::make_texture_A16F( int w, int h, const float *data, int filter_type, bool clamp )
	{
        GfxTexture::Ptr tex = new GfxTexture(GfxTexture::Target2D);
        tex->setFormat(GfxTexture::R16F);
        tex->setSize(w, h);
        tex->setData(GfxTexture::Alpha, GfxTexture::Float32, data);
        if (clamp)
            tex->setWrapMode(GfxTexture::ClampToEdge);
        else
            tex->setWrapMode(GfxTexture::Repeat);

        switch (filter_type)
        {
        case FILTER_NONE:
            tex->setMinificationFilter(GfxTexture::Nearest);
            tex->setMagnificationFilter(GfxTexture::Nearest);
            break;
        case FILTER_LINEAR:
            tex->setMinificationFilter(GfxTexture::Linear);
            tex->setMagnificationFilter(GfxTexture::Linear);
            break;
        case FILTER_BILINEAR:
            tex->setMinificationFilter(GfxTexture::LinearMipMapNearest);
            tex->setMagnificationFilter(GfxTexture::Linear);
            break;
        case FILTER_TRILINEAR:
            tex->setMinificationFilter(GfxTexture::LinearMipMapLinear);
            tex->setMagnificationFilter(GfxTexture::Linear);
            break;
        }
        return tex;
	}

    GfxTexture::Ptr GFX::make_texture_RGBA32F( int w, int h, const float *data, int filter_type, bool clamp )
	{
        GfxTexture::Ptr tex = new GfxTexture(GfxTexture::Target2D);
        tex->setFormat(GfxTexture::RGBA32F);
        tex->setSize(w, h);
        tex->setData(GfxTexture::RGBA, GfxTexture::Float32, data);
        if (clamp)
            tex->setWrapMode(GfxTexture::ClampToEdge);
        else
            tex->setWrapMode(GfxTexture::Repeat);

        switch (filter_type)
        {
        case FILTER_NONE:
            tex->setMinificationFilter(GfxTexture::Nearest);
            tex->setMagnificationFilter(GfxTexture::Nearest);
            break;
        case FILTER_LINEAR:
            tex->setMinificationFilter(GfxTexture::Linear);
            tex->setMagnificationFilter(GfxTexture::Linear);
            break;
        case FILTER_BILINEAR:
            tex->setMinificationFilter(GfxTexture::LinearMipMapNearest);
            tex->setMagnificationFilter(GfxTexture::Linear);
            break;
        case FILTER_TRILINEAR:
            tex->setMinificationFilter(GfxTexture::LinearMipMapLinear);
            tex->setMagnificationFilter(GfxTexture::Linear);
            break;
        }
        return tex;
	}

    GfxTexture::Ptr GFX::make_texture_RGBA16F( int w, int h, const float *data, int filter_type, bool clamp )
	{
        GfxTexture::Ptr tex = new GfxTexture(GfxTexture::Target2D);
        tex->setFormat(GfxTexture::RGBA16F);
        tex->setSize(w, h);
        tex->setData(GfxTexture::RGBA, GfxTexture::Float32, data);
        if (clamp)
            tex->setWrapMode(GfxTexture::ClampToEdge);
        else
            tex->setWrapMode(GfxTexture::Repeat);

        switch (filter_type)
        {
        case FILTER_NONE:
            tex->setMinificationFilter(GfxTexture::Nearest);
            tex->setMagnificationFilter(GfxTexture::Nearest);
            break;
        case FILTER_LINEAR:
            tex->setMinificationFilter(GfxTexture::Linear);
            tex->setMagnificationFilter(GfxTexture::Linear);
            break;
        case FILTER_BILINEAR:
            tex->setMinificationFilter(GfxTexture::LinearMipMapNearest);
            tex->setMagnificationFilter(GfxTexture::Linear);
            break;
        case FILTER_TRILINEAR:
            tex->setMinificationFilter(GfxTexture::LinearMipMapLinear);
            tex->setMagnificationFilter(GfxTexture::Linear);
            break;
        }
        return tex;
	}

    GfxTexture::Ptr GFX::make_texture_RGB16F( int w, int h, const float *data, int filter_type, bool clamp )
	{
        GfxTexture::Ptr tex = new GfxTexture(GfxTexture::Target2D);
        tex->setFormat(GfxTexture::RGB16F);
        tex->setSize(w, h);
        tex->setData(GfxTexture::RGB, GfxTexture::Float32, data);
        if (clamp)
            tex->setWrapMode(GfxTexture::ClampToEdge);
        else
            tex->setWrapMode(GfxTexture::Repeat);

        switch (filter_type)
        {
        case FILTER_NONE:
            tex->setMinificationFilter(GfxTexture::Nearest);
            tex->setMagnificationFilter(GfxTexture::Nearest);
            break;
        case FILTER_LINEAR:
            tex->setMinificationFilter(GfxTexture::Linear);
            tex->setMagnificationFilter(GfxTexture::Linear);
            break;
        case FILTER_BILINEAR:
            tex->setMinificationFilter(GfxTexture::LinearMipMapNearest);
            tex->setMagnificationFilter(GfxTexture::Linear);
            break;
        case FILTER_TRILINEAR:
            tex->setMinificationFilter(GfxTexture::LinearMipMapLinear);
            tex->setMagnificationFilter(GfxTexture::Linear);
            break;
        }
        return tex;
	}

    GfxTexture::Ptr GFX::create_color_texture(uint32 color)
    {
        QImage img(QSize(1,1), QImage::Format_RGBA8888);
        img.setPixel(0,0, color);
        GfxTexture::Ptr tex = new GfxTexture(img);
        tex->setWrapMode(GfxTexture::Repeat);
        tex->setMinificationFilter(GfxTexture::Nearest);
        tex->setMagnificationFilter(GfxTexture::Nearest);
        return tex;
    }

    GfxTexture::Ptr GFX::create_texture(int w, int h, int filter_type, bool clamp, GfxTexture::TextureFormat format)
	{
        GfxTexture::Ptr tex = new GfxTexture(GfxTexture::Target2D);
        tex->setFormat(format);
        tex->setSize(w, h);
        if (clamp)
            tex->setWrapMode(GfxTexture::ClampToEdge);
        else
            tex->setWrapMode(GfxTexture::Repeat);

        switch (filter_type)
        {
        case FILTER_NONE:
            tex->setMinificationFilter(GfxTexture::Nearest);
            tex->setMagnificationFilter(GfxTexture::Nearest);
            break;
        case FILTER_LINEAR:
            tex->setMinificationFilter(GfxTexture::Linear);
            tex->setMagnificationFilter(GfxTexture::Linear);
            break;
        case FILTER_BILINEAR:
            tex->setMinificationFilter(GfxTexture::LinearMipMapNearest);
            tex->setMagnificationFilter(GfxTexture::Linear);
            break;
        case FILTER_TRILINEAR:
            tex->setMinificationFilter(GfxTexture::LinearMipMapLinear);
            tex->setMagnificationFilter(GfxTexture::Linear);
            break;
        }
        tex->allocateStorage();
        return tex;
	}

    GfxTexture::Ptr GFX::create_texture_RGBA32F(int w, int h, int filter_type, bool clamp )
	{
        return make_texture_RGBA32F(w, h, NULL, filter_type, clamp);
	}

    GfxTexture::Ptr GFX::create_texture_RGB16F(int w, int h, int filter_type, bool clamp )
	{
        return make_texture_RGB16F(w, h, NULL, filter_type, clamp);
	}

    GfxTexture::Ptr GFX::create_texture_RGBA16F(int w, int h, int filter_type, bool clamp )
	{
        return make_texture_RGBA16F(w, h, NULL, filter_type, clamp);
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


    GfxTexture::Ptr GFX::load_texture(const QString& file, int filter_type, uint32 *width, uint32 *height, bool clamp, GLuint texFormat, bool *useAlpha, bool checkSize)
	{
        if (!VFS::Instance()->fileExists(file)) // The file doesn't exist
            return GfxTexture::Ptr();

        const QString &upfile = file.toUpper() + " (" + QString::number(texFormat) + "-" + QString::number(filter_type) + ')';
        HashMap<GfxTexture::Ptr>::Sparse::iterator it = textureIDs.find(upfile);
        if (it != textureIDs.end())		// File already loaded
        {
            if (width)
                *width = (*it)->width();
            if (height)
                *height = (*it)->height();
            return *it;
        }

		const bool compressible = texFormat == GL_COMPRESSED_RGB || texFormat == GL_COMPRESSED_RGBA || texFormat == 0;
        QString cache_filename = !file.isEmpty() ? file + ".bin" : QString();
		cache_filename.replace('/', 'S');
		cache_filename.replace('\\', 'S');
		if (compressible)
		{
            GfxTexture::Ptr gltex = load_texture_from_cache(cache_filename, filter_type, width, height, clamp, useAlpha);
			if (gltex)
				return gltex;
		}

        QImage bmp = load_image(file);
        if (bmp.isNull())
		{
			LOG_ERROR("Failed to load texture `" << file << "`");
            return GfxTexture::Ptr();
		}

		if (width)
            *width = bmp.width();
		if (height)
            *height = bmp.height();

		if (checkSize)
		{
			const int maxTextureSizeAllowed = lp_CONFIG->getMaxTextureSizeAllowed();
            if (std::max(bmp.width(), bmp.height()) > maxTextureSizeAllowed)
                bmp = shrink(bmp, std::min(bmp.width(), maxTextureSizeAllowed), std::min(bmp.height(),maxTextureSizeAllowed));
		}

        bool with_alpha = bmp.hasAlphaChannel();
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
        GfxTexture::Ptr gl_tex = make_texture(bmp, filter_type, clamp);
		if (gl_tex)
		{
            textureIDs[upfile] = gl_tex;

			if (compressible)
                save_texture_to_cache(cache_filename, gl_tex, bmp.width(), bmp.height(), with_alpha);
		}
		return gl_tex;
	}


    GfxTexture::Ptr	GFX::load_texture_mask(const QString& file, uint32 level, int filter_type, uint32 *width, uint32 *height, bool clamp )
	{
        if (!VFS::Instance()->fileExists(file)) // The file doesn't exist
            return GfxTexture::Ptr();

        QImage bmp = load_image(file);
        if (bmp.isNull() )	return GfxTexture::Ptr();					// Operation failed
        if (width )		*width = bmp.width();
        if (height )	*height = bmp.height();
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


    GfxTexture::Ptr GFX::load_texture_from_cache(const QString& file, int filter_type, uint32 *width, uint32 *height, bool clamp, bool *useAlpha )
	{
		if (ati_workaround
			|| !lp_CONFIG->use_texture_cache
			|| !lp_CONFIG->use_texture_compression
			|| !g_useGenMipMaps
			|| !g_useNonPowerOfTwoTextures
			|| lp_CONFIG->developerMode)		// No caching in developer mode
            return GfxTexture::Ptr();
        return GfxTexture::Ptr();

//        QString realFile(TA3D::Paths::Caches);
//		realFile += file;
//		if(TA3D::Paths::Exists(realFile))
//		{
//            QFile cache_file(realFile);
//            cache_file.open(QIODevice::ReadOnly);
//			uint32 mod_hash;
//			cache_file.read((char*)&mod_hash, sizeof(mod_hash));

//            if (mod_hash != hash<QString>()(TA3D_CURRENT_MOD)) // Doesn't correspond to current mod
//			{
//				cache_file.close();
//                return GfxTexture::Ptr();
//			}

//			if (useAlpha)
//                *useAlpha = readChar(&cache_file);
//			else
//                readChar(&cache_file);

//			uint32 rw, rh;
//			cache_file.read((char*)&rw, 4);
//			cache_file.read((char*)&rh, 4);
//			if(width)  *width = rw;
//			if(height) *height = rh;

//			int lod_max = 0;
//			GLint size, internal_format;

//			cache_file.read( (char*)&lod_max, sizeof( lod_max ));

//			GLuint	tex;
//			glEnable(GL_TEXTURE_2D);
//			glGenTextures(1,&tex);

//			if (filter_type == FILTER_NONE || filter_type == FILTER_LINEAR)
//				use_mipmapping(false);
//			else
//				use_mipmapping(true);

//			glBindTexture( GL_TEXTURE_2D, tex );
//            glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);

//			for(int lod = 0 ; lod < lod_max ; ++lod)
//			{
//				GLint w,h,border;
//				cache_file.read( (char*)&size, sizeof( GLint ) );

//				byte *img = new byte[size];

//				cache_file.read( (char*)&internal_format, sizeof( GLint ) );
//				cache_file.read( (char*)&border, sizeof( GLint ) );
//				cache_file.read( (char*)&w, sizeof( GLint ) );
//				cache_file.read( (char*)&h, sizeof( GLint ) );
//				cache_file.read( (char*)img, size );
//				if (lod == 0)
//                    glCompressedTexImage2D( GL_TEXTURE_2D, 0, internal_format, w, h, border, size, img);
//				else
//					glCompressedTexSubImage2D( GL_TEXTURE_2D, lod, 0, 0, w, h, internal_format, size, img);

//				DELETE_ARRAY(img);
//				if (!build_mipmaps)
//					break;
//			}
//			if (build_mipmaps)
//                glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

//			cache_file.close();

//			if (filter_type == FILTER_NONE || filter_type == FILTER_LINEAR)
//				use_mipmapping(true);

//			glMatrixMode(GL_TEXTURE);
//			glLoadIdentity();
//			glMatrixMode(GL_MODELVIEW);

//			if (clamp)
//			{
//				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
//				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
//			}
//			else
//			{
//				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
//				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
//			}
//			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, lp_CONFIG->anisotropy);

//			switch (filter_type)
//			{
//			case FILTER_NONE:
//				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
//				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
//				break;
//			case FILTER_LINEAR:
//				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
//				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
//				break;
//			case FILTER_BILINEAR:
//				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
//				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
//				break;
//			case FILTER_TRILINEAR:
//				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
//				glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
//				break;
//			}
//			return tex;
//		}
//		return 0; // File isn't in cache
	}


    void GFX::save_texture_to_cache(QString file, const GfxTexture::Ptr &tex, uint32 width, uint32 height, bool useAlpha )
	{
		if(ati_workaround
		   || !lp_CONFIG->use_texture_cache
		   || !lp_CONFIG->use_texture_compression
		   || !g_useGenMipMaps
		   || !g_useNonPowerOfTwoTextures
		   || lp_CONFIG->developerMode)
			return;

        file = TA3D::Paths::Caches + file;

        int rw = tex->width(), rh = tex->height();		// Also binds tex

//        GLint compressed;
//        glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, &compressed );
//		// Do not save it if it's not compressed -> save disk space, and it would slow things down
//		if(!compressed)
//			return;

//        QFile cache_file(file);
//        cache_file.open(QIODevice::WriteOnly);

//        if (!cache_file.isOpen())
//			return;

//        uint32 mod_hash = static_cast<uint32>(hash<QString>()(TA3D_CURRENT_MOD)); // Save a hash of current mod

//		cache_file.write( (const char*)&mod_hash, sizeof( mod_hash ) );

//        cache_file.putChar(useAlpha);
//		cache_file.write( (const char*)&width, 4 );
//		cache_file.write( (const char*)&height, 4 );

//		int lod_max = Math::Max(Math::Log2(rw), Math::Log2(rh));
//		if ((1 << lod_max) < rw && (1 << lod_max) < rh )
//			lod_max++;

//		cache_file.write( (const char*)&lod_max, sizeof( lod_max ) );

//		for(int lod = 0 ; lod < lod_max ; ++lod)
//		{
//			GLint size, internal_format;

//			glGetTexLevelParameteriv( GL_TEXTURE_2D, lod, GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB, &size );
//			glGetTexLevelParameteriv( GL_TEXTURE_2D, lod, GL_TEXTURE_INTERNAL_FORMAT, &internal_format );

//			byte *img = new byte[size];
//			memset(img, 0, size);

//            glGetCompressedTexImage( GL_TEXTURE_2D, lod, img );
//			GLint w,h,border;
//			glGetTexLevelParameteriv( GL_TEXTURE_2D, lod, GL_TEXTURE_BORDER, &border );
//			glGetTexLevelParameteriv( GL_TEXTURE_2D, lod, GL_TEXTURE_WIDTH, &w );
//			glGetTexLevelParameteriv( GL_TEXTURE_2D, lod, GL_TEXTURE_HEIGHT, &h );

//			cache_file.write( (const char*)&size, sizeof( GLint ) );
//			cache_file.write( (const char*)&internal_format, sizeof( GLint ) );
//			cache_file.write( (const char*)&border, sizeof( GLint ) );
//			cache_file.write( (const char*)&w, sizeof( GLint ) );
//			cache_file.write( (const char*)&h, sizeof( GLint ) );
//			cache_file.write( (const char*)img, size );

//			DELETE_ARRAY(img);
//		}

//		cache_file.close();
	}



    GfxTexture::Ptr GFX::load_masked_texture(const QString &file, QString mask, int filter_type )
	{
		if ( (!VFS::Instance()->fileExists(file)) || (!VFS::Instance()->fileExists(mask)))
            return GfxTexture::Ptr(); // The file doesn't exist

        QImage bmp = load_image(file);
        if (bmp.isNull())
            return GfxTexture::Ptr();					// Operation failed
        const QImage &alpha = load_image(mask);
        if(alpha.isNull())
            return GfxTexture::Ptr();
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


	void GFX::set_alpha_blending()
	{
		glEnable(GL_BLEND);
        CHECK_GL();
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        CHECK_GL();
        alpha_blending_set = true;
	}


	void GFX::unset_alpha_blending()
	{
		glDisable(GL_BLEND);
        CHECK_GL();
        alpha_blending_set = false;
	}


	void GFX::ReInitTexSys(bool matrix_reset)
	{
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
        CHECK_GL();
        glDisable(GL_TEXTURE_GEN_S);
        CHECK_GL();
        glDisable(GL_TEXTURE_GEN_T);
        CHECK_GL();
        if (matrix_reset)
		{
			glMatrixMode(GL_TEXTURE);
            CHECK_GL();
            glLoadIdentity();
            CHECK_GL();
            glMatrixMode(GL_MODELVIEW);
            CHECK_GL();
        }
	}


	void GFX::ReInitAllTex(bool disable)
	{
		if (MultiTexturing)
		{
			for (unsigned int i = 0; i < 7; ++i)
			{
				glActiveTextureARB(GL_TEXTURE0_ARB + i);
                CHECK_GL();
                ReInitTexSys();
				glClientActiveTexture(GL_TEXTURE0_ARB + i);
                CHECK_GL();
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
                CHECK_GL();
                if (disable)
                {
					glDisable(GL_TEXTURE_2D);
                    CHECK_GL();
                }
            }
			glActiveTextureARB(GL_TEXTURE0_ARB);
            CHECK_GL();
            glClientActiveTexture(GL_TEXTURE0_ARB);
            CHECK_GL();
        }
	}


	void GFX::SetDefState()
	{
        CHECK_GL();
        glClearColor (0, 0, 0, 0);
        CHECK_GL();
        glShadeModel (GL_SMOOTH);
        CHECK_GL();
        glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
        CHECK_GL();
        glDepthFunc( GL_LESS );
        CHECK_GL();
        glEnable (GL_DEPTH_TEST);
        CHECK_GL();
        glCullFace (GL_BACK);
        CHECK_GL();
        glEnable (GL_CULL_FACE);
        CHECK_GL();
        glHint(GL_FOG_HINT, GL_NICEST);
        CHECK_GL();
        glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
        CHECK_GL();
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
        CHECK_GL();
        glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
        CHECK_GL();
        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
        CHECK_GL();
        glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);
        CHECK_GL();
        glFogi (GL_FOG_COORD_SRC, GL_FRAGMENT_DEPTH);
        CHECK_GL();
        glDisable(GL_BLEND);
        CHECK_GL();
        glEnable(GL_LIGHTING);
        CHECK_GL();
        ReInitTexSys();
		alpha_blending_set = false;
	}


	void GFX::ReInitArrays()
	{
		glDisableClientState(GL_VERTEX_ARRAY);
        CHECK_GL();
        glDisableClientState(GL_NORMAL_ARRAY);
        CHECK_GL();
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        CHECK_GL();
        glDisableClientState(GL_COLOR_ARRAY);
        CHECK_GL();
    }


    uint32 GFX::InterfaceMsg(const uint32 MsgID, const QString &)
	{
		if (MsgID != TA3D_IM_GFX_MSG)
			return INTERFACE_RESULT_CONTINUE;
		return INTERFACE_RESULT_CONTINUE;
	}


	void GFX::clearAll()
	{
        CHECK_GL();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        CHECK_GL();
    }


	void GFX::clearScreen()
	{
        CHECK_GL();
        glClear(GL_COLOR_BUFFER_BIT);
        CHECK_GL();
    }


	void GFX::clearDepth()
	{
        CHECK_GL();
        glClear(GL_DEPTH_BUFFER_BIT);
        CHECK_GL();
    }


	void GFX::disable_texturing()
	{
        CHECK_GL();
        glDisable( GL_TEXTURE_2D );
        CHECK_GL();
    }


	void GFX::enable_texturing()
	{
        CHECK_GL();
        glEnable( GL_TEXTURE_2D );
        CHECK_GL();
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
            glbackground = load_texture("gfx/menu1280.jpg", FILTER_LINEAR);
		else
            glbackground = load_texture(((width <= 800) ? "gfx/menu800.jpg" : "gfx/menu1024.jpg"), FILTER_LINEAR);
	}


    void GFX::renderToTexture(const GfxTexture::Ptr &tex, bool useDepth)
	{
        if (!tex)       // Release the texture
        {
            gfx->glBindFramebuffer(GL_FRAMEBUFFER, 0);     // Bind the default FBO
            CHECK_GL();
            gfx->glViewport(0, 0, width, height);           // Use default viewport
            CHECK_GL();
        }
        else
        {
            if (textureFBO == 0)    // Generate a FBO if none has been created yet
            {
                gfx->glGenFramebuffers(1, &textureFBO);
                CHECK_GL();
            }

            gfx->glBindFramebuffer(GL_FRAMEBUFFER, textureFBO);					                    // Bind the FBO
            CHECK_GL();
            gfx->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex->textureId(), 0); // Attach the texture
            CHECK_GL();
            if (useDepth)
            {
                if (!textureDepth || textureDepth->width() != tex->width() || textureDepth->height() != tex->height())
                    textureDepth = create_texture(tex->width(), tex->height(), FILTER_LINEAR, true, GfxTexture::D24);
                gfx->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, textureDepth->textureId(), 0); // Attach the texture
                CHECK_GL();
            }
            else
            {
                gfx->glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0); // Dettach the texture
                CHECK_GL();
            }
            gfx->glViewport(0, 0, tex->width(), tex->height());                                     // Stretch viewport to texture size
            CHECK_GL();
        }
	}


    void GFX::renderToTextureDepth(const GfxTexture::Ptr &tex)
	{
        if (!tex)       // Release the texture
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);     // Bind the default FBO
            CHECK_GL();
            glViewport(0, 0, width, height);           // Use default viewport
            CHECK_GL();
        }
        else
        {
            int tex_w = tex->width();
            int tex_h = tex->height();
            if (!textureFBO)    // Generate a FBO if none has been created yet
            {
                glGenFramebuffers(1, &textureFBO);
                CHECK_GL();
            }

            glBindFramebuffer(GL_FRAMEBUFFER, textureFBO);					                    // Bind the FBO
            CHECK_GL();
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0); // Dettach the texture
            CHECK_GL();
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex->textureId(), 0); // Attach the texture
            CHECK_GL();

            switch (glCheckFramebufferStatus(GL_FRAMEBUFFER))
            {
            case GL_FRAMEBUFFER_COMPLETE:   break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:          std::cout << "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT" << std::endl; break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:  std::cout << "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT" << std::endl; break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:         std::cout << "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER" << std::endl; break;
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:         std::cout << "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER" << std::endl; break;
            case GL_FRAMEBUFFER_UNSUPPORTED:                    std::cout << "GL_FRAMEBUFFER_UNSUPPORTED" << std::endl; break;
            default:
                std::cout << "Unknown FBO status" << std::endl;
            }

            glViewport(0, 0, tex_w, tex_h);                                     // Stretch viewport to texture size
            CHECK_GL();
        }
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
        case 32:    return QImage(QSize(w, h), QImage::Format_ARGB32);
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

    GfxTexture::Ptr GFX::create_shadow_map(int w, int h)
	{
        GfxTexture::Ptr shadowMapTexture = new GfxTexture(GfxTexture::Target2D);
        shadowMapTexture->setSize(w, h);
        shadowMapTexture->setFormat(GfxTexture::D24);
        shadowMapTexture->allocateStorage();

		if (lp_CONFIG->shadow_quality == 2)
		{
            shadowMapTexture->setMinificationFilter(GfxTexture::Nearest);            // We want fast shadows
            shadowMapTexture->setMagnificationFilter(GfxTexture::Nearest);
		}
		else
		{
            shadowMapTexture->setMinificationFilter(GfxTexture::Linear);            // We want smooth shadows
            shadowMapTexture->setMagnificationFilter(GfxTexture::Linear);
		}
        shadowMapTexture->setWrapMode(GfxTexture::ClampToEdge);
        shadowMapTexture->setComparisonFunction(GfxTexture::CompareLessEqual);
        shadowMapTexture->setComparisonMode(GfxTexture::CompareRefToTexture);

		return shadowMapTexture;
	}

	void GFX::delete_shadow_map()
	{
        shadowMap = nullptr;
	}

    GfxTexture::Ptr GFX::get_shadow_map()
	{
        if (!shadowMap)
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
                        if (!model_shader)
                            model_shader = new Shader("shaders/3do_shadow.frag", "shaders/3do_shadow.vert");
                        if (model_shader && model_shader->isLinked())
						{
                            model_shader->bind();
                            CHECK_GL();
                            model_shader->setUniformValue("shadowMap", 7);
                            CHECK_GL();
                            model_shader->setmat4f("light_Projection", shadowMapProjectionMatrix);
                            CHECK_GL();
                        }
						# endif
						break;
					}
				default:
                    if (model_shader)
                    {
                        model_shader->release();
                        CHECK_GL();
                    }
            }
			break;
		default:
            if (model_shader)
            {
                model_shader->release();
                CHECK_GL();
            }
		}
	}

    void GFX::loadShaders()
    {
        if (!drawing2d_color_shader)
        {
            drawing2d_color_shader = new Shader("shaders/2d_color.frag", "shaders/2d_color.vert");
            drawing2d_color_shader->bindAttributeLocation("aVertex", 0);
            CHECK_GL();
            drawing2d_color_shader->bindAttributeLocation("aColor", 1);
            CHECK_GL();
            drawing2d_color_shader->link();
            CHECK_GL();
            drawing2d_color_shader->bind();
            CHECK_GL();
            drawing2d_color_shader->setUniformValue("uMatrix", get2Dmatrix());
            CHECK_GL();
            drawing2d_color_shader->release();
            CHECK_GL();
        }
        if (!drawing2d_texture_shader)
        {
            drawing2d_texture_shader = new Shader("shaders/2d_texture.frag", "shaders/2d_texture.vert");
            drawing2d_texture_shader->bindAttributeLocation("aVertex", 0);
            CHECK_GL();
            drawing2d_texture_shader->bindAttributeLocation("aColor", 1);
            CHECK_GL();
            drawing2d_texture_shader->bindAttributeLocation("aTexCoord", 2);
            CHECK_GL();
            drawing2d_texture_shader->link();
            CHECK_GL();
            drawing2d_texture_shader->bind();
            CHECK_GL();
            drawing2d_texture_shader->setUniformValue("uMatrix", get2Dmatrix());
            CHECK_GL();
            drawing2d_texture_shader->setUniformValue("uTex", 0);
            CHECK_GL();
            drawing2d_texture_shader->release();
            CHECK_GL();
        }
        if (!particle_shader)
        {
            particle_shader = new Shader("shaders/particle.frag", "shaders/particle.vert");
            particle_shader->bindAttributeLocation("aVertex", 0);
            CHECK_GL();
            particle_shader->link();
            CHECK_GL();
            particle_shader->bind();
            CHECK_GL();
            particle_shader->setUniformValue("uTex", 0);
            CHECK_GL();
            particle_shader->release();
            CHECK_GL();
        }
    }


	void GFX::disable_model_shading()
	{
        if (model_shader)
        {
            model_shader->release();
            CHECK_GL();
        }
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
        CHECK_GL();

		glMatrixMode(GL_PROJECTION);
        CHECK_GL();
        glLoadIdentity();
        CHECK_GL();
        glScalef(0.5f, 0.5f, 0.5f);
        CHECK_GL();
        glTranslatef(1.0f, 1.0f, 1.0f);
        CHECK_GL();
        glMultMatrixf(backup);
        CHECK_GL();
        glGetFloatv(GL_PROJECTION_MATRIX, gfx->shadowMapProjectionMatrix);
        CHECK_GL();

		glLoadIdentity();
        CHECK_GL();
        glMultMatrixf(backup);
        CHECK_GL();
        glMatrixMode(GL_MODELVIEW);
        CHECK_GL();
    }

    void GFX::enableShadowMapping()
	{
		glActiveTexture(GL_TEXTURE7);
        CHECK_GL();
        glEnable(GL_TEXTURE_2D);
        CHECK_GL();
        glActiveTexture(GL_TEXTURE0);
        CHECK_GL();
    }

    void GFX::disableShadowMapping()
	{
		glActiveTexture(GL_TEXTURE7);
        CHECK_GL();
        glDisable(GL_TEXTURE_2D);
        CHECK_GL();
        glActiveTexture(GL_TEXTURE0);
        CHECK_GL();
    }

	void GFX::storeShadowMappingState()
	{
		glActiveTexture(GL_TEXTURE7);
        CHECK_GL();
        shadowMapWasActive = glIsEnabled(GL_TEXTURE_2D);
        CHECK_GL();
        glActiveTexture(GL_TEXTURE0);
        CHECK_GL();
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
        CHECK_GL();
    }

    // ---- Input events management ----
    void GFX::keyPressEvent(QKeyEvent *e)
    {
        QWindow::keyPressEvent(e);
        set_key_down(e->key(), e->text());
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
        mouse_z += e->angleDelta().y() / 32;
    }

    void GFX::mouseMoveEvent(QMouseEvent *e)
    {
        QWindow::mouseMoveEvent(e);
        mouse_b = (e->buttons() & 7);
        mouse_x = e->x();
        mouse_y = e->y();
    }
} // namespace TA3D
