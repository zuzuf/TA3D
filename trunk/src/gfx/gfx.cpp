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

#include "../stdafx.h"
#include "../TA3D_NameSpace.h"
#include "glfunc.h"
#include "../gui.h"
#include "../gaf.h"
#include "gfx.h"
#include "gui/skin.manager.h"
#include "../misc/paths.h"
#include "../logs/logs.h"
#include <strings.h>
#include "../misc/math.h"


#define YESNO(X)  (X ? "Yes" : "No")


namespace TA3D
{

    void GFX::set_texture_format(GLuint gl_format)
    {
        texture_format = gl_format == 0 ? GL_RGB8 : gl_format;
    }

    void GFX::use_mipmapping(bool use)
    {
        build_mipmaps = use;
    }

    void GFX::initSDL()
    {
        SDL_Surface *icon = load_image("gfx\\icon.png");
        if (icon)
        {
            SDL_WM_SetIcon(icon, NULL);
            SDL_FreeSurface(icon);
        }

        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, TA3D::VARS::lp_CONFIG->fsaa > 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, TA3D::VARS::lp_CONFIG->fsaa);
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        if (TA3D::VARS::lp_CONFIG->color_depth == 32)
        {
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
        }
        else
        {
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
        }
        SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

        SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 0);

        uint32 flags = SDL_OPENGL | SDL_HWSURFACE;
        if (TA3D::VARS::lp_CONFIG->fullscreen )
            flags |= SDL_FULLSCREEN;

        screen = SDL_SetVideoMode( TA3D::VARS::lp_CONFIG->screen_width, TA3D::VARS::lp_CONFIG->screen_height, TA3D::VARS::lp_CONFIG->color_depth, flags );

        if (screen == NULL)
        {
            LOG_WARNING(LOG_PREFIX_GFX << "SDL_SetVideoMode failed : " << SDL_GetError());
            LOG_WARNING(LOG_PREFIX_GFX << "retrying with GL_DEPTH_SIZE = 16");
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
            screen = SDL_SetVideoMode( TA3D::VARS::lp_CONFIG->screen_width, TA3D::VARS::lp_CONFIG->screen_height, TA3D::VARS::lp_CONFIG->color_depth, flags );
        }

        if (screen == NULL)
        {
            LOG_WARNING(LOG_PREFIX_GFX << "SDL_SetVideoMode failed : " << SDL_GetError());
            LOG_WARNING(LOG_PREFIX_GFX << "retrying with GL_DEPTH_SIZE = 24 and current color depth");
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
            screen = SDL_SetVideoMode( TA3D::VARS::lp_CONFIG->screen_width, TA3D::VARS::lp_CONFIG->screen_height, 0, flags );
        }

        if (screen == NULL)
        {
            LOG_WARNING(LOG_PREFIX_GFX << "SDL_SetVideoMode failed : " << SDL_GetError());
            LOG_WARNING(LOG_PREFIX_GFX << "retrying with GL_DEPTH_SIZE = 16 and current color depth");
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 6);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
            SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 0);
            screen = SDL_SetVideoMode( TA3D::VARS::lp_CONFIG->screen_width, TA3D::VARS::lp_CONFIG->screen_height, 0, flags );
        }

        if (screen == NULL)
        {
            LOG_ERROR(LOG_PREFIX_GFX << "Impossible to set OpenGL video mode!");
            LOG_ERROR(LOG_PREFIX_GFX << "SDL_GetError() = " << SDL_GetError());
            return;
        }

        preCalculations();
        // Install OpenGL extensions
        installOpenGLExtensions();

        if(g_useTextureCompression && lp_CONFIG->use_texture_compression) // Try to enabled the Texture compression
            set_texture_format(GL_COMPRESSED_RGB_ARB);
        else
            set_texture_format(GL_RGB8);
        glViewport(0,0,SCREEN_W,SCREEN_H);

        glGetIntegerv(GL_MAX_TEXTURE_SIZE,&max_tex_size);
    }


    bool GFX::checkVideoCardWorkaround() const
    {
        // Check for ATI workarounds (if an ATI card is present)
        bool workaround = strncasecmp(String::ToUpper((const char*)glGetString(GL_VENDOR)).c_str(), "ATI", 3) == 0;
        // Check for SIS workarounds (if an SIS card is present) (the same than ATI)
        workaround |= strstr(String::ToUpper((const char*)glGetString(GL_VENDOR)).c_str(), "SIS") != NULL;
        return workaround;
    }

    void GFX::displayInfosAboutOpenGL() const
    {
        LOG_INFO(LOG_PREFIX_OPENGL << "OpenGL informations:");
        LOG_INFO(LOG_PREFIX_OPENGL << "Vendor: " << glGetString(GL_VENDOR));
        LOG_INFO(LOG_PREFIX_OPENGL << "Renderer: " << glGetString(GL_RENDERER));
        LOG_INFO(LOG_PREFIX_OPENGL << "Version: " << glGetString(GL_VERSION));
        if (ati_workaround)
            LOG_WARNING("ATI or SIS card detected ! Using workarounds for ATI/SIS cards");
        LOG_INFO(LOG_PREFIX_OPENGL << "Texture compression: " << YESNO(g_useTextureCompression));
        LOG_INFO(LOG_PREFIX_OPENGL << "Stencil Two Side: " << YESNO(g_useStencilTwoSide));
        LOG_INFO(LOG_PREFIX_OPENGL << "FBO: " << YESNO(g_useFBO));
        LOG_INFO(LOG_PREFIX_OPENGL << "Shaders: " << YESNO(g_useProgram));
        LOG_INFO(LOG_PREFIX_OPENGL << "Multi texturing: " << YESNO(MultiTexturing));
    }



    GFX::GFX()
    {
        initSDL();
        ati_workaround = checkVideoCardWorkaround();

        TA3D::VARS::pal = NULL;

        width = SCREEN_W;
        height = SCREEN_H;
        x = 0;
        y = 0;
        low_def_limit = 600.0f;

        textureFBO = 0;
        textureDepth = 0;
        textureColor = 0;
        glfond = 0;
        shadowMap = 0;

        LOG_DEBUG(LOG_PREFIX_GFX << "Creating a normal font...");
        normal_font = font_manager.getFont("FreeSans", 10, FONT_TYPE_TEXTURE);

        LOG_DEBUG(LOG_PREFIX_GFX << "Creating a small font...");
        small_font = font_manager.getFont("FreeMono", 8, FONT_TYPE_TEXTURE);

        LOG_DEBUG(LOG_PREFIX_GFX << "Loading a big font...");
        TA_font = font_manager.getFont("FreeSans", 16, FONT_TYPE_TEXTURE);

        LOG_DEBUG(LOG_PREFIX_GFX << "Loading the GUI font...");
        ta3d_gui_font = font_manager.getFont("FreeSerif", 10 * SCREEN_W / 640, FONT_TYPE_TEXTURE);
        gui_font = ta3d_gui_font;

        LOG_DEBUG(LOG_PREFIX_GFX << "Loading a big scaled font...");
        big_font = font_manager.getFont("FreeSans", 16 * SCREEN_W / 640, FONT_TYPE_TEXTURE);

        InitInterface();
        displayInfosAboutOpenGL();
    }



    GFX::~GFX()
    {
        DeleteInterface();

        if (textureFBO)
            glDeleteFramebuffersEXT(1,&textureFBO);
        if (textureDepth)
            glDeleteRenderbuffersEXT(1,&textureDepth);
        destroy_texture(textureColor);
        destroy_texture(shadowMap);
        destroy_texture(default_texture);

        if (TA3D::VARS::pal )
            delete[]( TA3D::VARS::pal );

        normal_font = NULL;
        small_font = NULL;
        TA_font = NULL;
        ta3d_gui_font = NULL;

        font_manager.destroy();
        skin_manager.destroy();
    }



    void GFX::Init()
    {
        LOG_DEBUG("Allocating palette memory...");
        TA3D::VARS::pal = new SDL_Color[256];      // Allocate a new palette

        LOG_DEBUG("Loading TA's palette...");
        bool palette = TA3D::UTILS::HPI::load_palette(pal);
        if (!palette)
            LOG_WARNING("Failed to load the palette");

        LOG_DEBUG(LOG_PREFIX_GFX << "Loading background...");
        load_background();

        LOG_DEBUG(LOG_PREFIX_GFX << "Loading default texture...");
        default_texture = load_texture("gfx/default.png");

        alpha_blending_set = false;

        LOG_INFO(LOG_PREFIX_GFX << "Graphics are initialized.");
    }



    void GFX::set_alpha(const float a) const
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
        glOrtho(0, SCREEN_W, SCREEN_H, 0, -1.0, 1.0);
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
        float d_alpha = DB_PI/(r+1.0f);
        int n = (int)(DB_PI / d_alpha) + 1;
        float *points = new float[n * 2];
        int i = 0;
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
        glDrawArrays( GL_LINE_LOOP, 0, i>>1 );
        delete[] points;
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
        delete[] points;
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
        delete[] points;
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
        delete[] points;
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


    void GFX::putpixel(const float x, const float y, const uint32 col)
    {
        set_color(col);
        glBegin(GL_POINTS);
        glVertex2f(x,y);
        glEnd();
    }

    uint32 GFX::getpixel(const sint32 x, const sint32 y) const
    {
        uint32 col;
        glReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &col);
        return col;
    }


    void GFX::drawtexture(const GLuint &tex, const float x1, const float y1, const float x2, const float y2, const float u1, const float v1, const float u2, const float v2)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D,tex);

        float points[8] = { x1,y1, x2,y1, x2,y2, x1,y2 };
        float tcoord[8] = { u1,v1, u2,v1, u2,v2, u1,v2 };

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_NORMAL_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, points);
        glTexCoordPointer(2, GL_FLOAT, 0, tcoord);
        glDrawArrays( GL_QUADS, 0, 4 );
    }
    void GFX::drawtexture(const GLuint &tex, const float x1, const float y1, const float x2, const float y2)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D,tex);

        float points[8] = { x1,y1, x2,y1, x2,y2, x1,y2 };
        float tcoord[8] = { 0.0f,0.0f, 1.0f,0.0f, 1.0f,1.0f, 0.0f,1.0f };

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

        float points[8] = { x1,y1, x2,y1, x2,y2, x1,y2 };
        float tcoord[8] = { 0.0f,0.0f, 0.0f,1.0f, 1.0f,1.0f, 1.0f,0.0f };

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


    void GFX::print(Font *font, const float x, const float y, const float z, const String &text)		// Font related routines
    {
        if (!font)  return;

        ReInitTexSys( false );
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_TEXTURE_2D);
        font->print(x, y, z, text);
    }
    void GFX::print(Font *font, const float x, const float y, const float z, const uint32 col, const String &text)		// Font related routines
    {
        set_color(col);
        print( font, x, y, z, text );
    }

    void GFX::print_center(Font *font, const float x, const float y, const float z, const String &text)		// Font related routines
    {
        if (!font)  return;

        ReInitTexSys( false );
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

        float X = x - 0.5f * font->length( text );

        glEnable(GL_TEXTURE_2D);
        font->print(X, y, z, text);
    }
    void GFX::print_center(Font *font, const float x, const float y, const float z, const uint32 col, const String &text)		// Font related routines
    {
        set_color(col);
        print_center( font, x, y, z, text );
    }


    void GFX::print_right(Font *font, const float x, const float y, const float z, const String &text)		// Font related routines
    {
        if (!font)  return;

        ReInitTexSys( false );
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

        float X = x - font->length( text );

        glEnable(GL_TEXTURE_2D);
        font->print(X, y, z, text);
    }

    void GFX::print_right(Font *font, const float x, const float y, const float z, const uint32 col, const String &text)		// Font related routines
    {
        set_color(col);
        print_right( font, x, y, z, text );
    }

    int GFX::max_texture_size()
    {
        return max_tex_size;
    }

    GLuint GFX::make_texture(SDL_Surface *bmp, byte filter_type, bool clamp )
    {
        if (bmp == NULL)
        {
            LOG_WARNING(LOG_PREFIX_GFX << "make_texture used with empty SDL_Surface");
            return 0;
        }
        MutexLocker locker(pMutex);

        if (bmp->w > max_tex_size || bmp->h > max_tex_size )
        {
            SDL_Surface *tmp = create_surface_ex( bmp->format->BitsPerPixel,
                                           Math::Min(bmp->w, max_tex_size), Math::Min(bmp->h, max_tex_size));
            stretch_blit( bmp, tmp, 0, 0, bmp->w, bmp->h, 0, 0, tmp->w, tmp->h );
            GLuint tex = make_texture( tmp, filter_type, clamp );
            SDL_FreeSurface( tmp );
            return tex;
        }

        if (!g_useNonPowerOfTwoTextures && (!Math::IsPowerOfTwo(bmp->w) || !Math::IsPowerOfTwo(bmp->h)))
        {
            int w = 1 << Math::Log2(bmp->w);
            int h = 1 << Math::Log2(bmp->h);
            if (w < bmp->w) w <<= 1;
            if (h < bmp->h) h <<= 1;
            SDL_Surface *tmp = create_surface_ex( bmp->format->BitsPerPixel, w, h);
            stretch_blit_smooth( bmp, tmp, 0, 0, bmp->w, bmp->h, 0, 0, tmp->w, tmp->h );
            GLuint tex = make_texture( tmp, filter_type, clamp );
            SDL_FreeSurface( tmp );
            return tex;
        }

        if (ati_workaround && filter_type != FILTER_NONE
            && ( !Math::IsPowerOfTwo(bmp->w) || !Math::IsPowerOfTwo(bmp->h)))
            filter_type = FILTER_LINEAR;

        if (filter_type == FILTER_NONE || filter_type == FILTER_LINEAR )
            use_mipmapping(false);
        else
            use_mipmapping(true);

        bool can_useGenMipMaps = g_useGenMipMaps && (g_useNonPowerOfTwoTextures || (Math::IsPowerOfTwo(bmp->w) && Math::IsPowerOfTwo(bmp->h)));

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

#ifdef TA3D_PLATFORM_WINDOWS
        bool softMipMaps = build_mipmaps;
#else
        bool softMipMaps = false;
#endif

        //Upload image data to OpenGL
        if (!softMipMaps)
        {
            if (can_useGenMipMaps)        // Automatic mipmaps generation
            {
                if (!build_mipmaps || glGenerateMipmapEXT)
                    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_FALSE );
                else
                    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE );
            }
            switch(bmp->format->BitsPerPixel)
            {
            case 8:
                if (build_mipmaps && !can_useGenMipMaps)        // Software mipmaps generation
                    gluBuild2DMipmaps(GL_TEXTURE_2D, texture_format, bmp->w, bmp->h, GL_LUMINANCE, GL_UNSIGNED_BYTE, bmp->pixels);
                else
                    glTexImage2D(GL_TEXTURE_2D, 0, texture_format, bmp->w, bmp->h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, bmp->pixels);
                break;
            case 24:
                if (build_mipmaps && !can_useGenMipMaps)        // Software mipmaps generation
                    gluBuild2DMipmaps(GL_TEXTURE_2D, texture_format, bmp->w, bmp->h, GL_RGB, GL_UNSIGNED_BYTE, bmp->pixels);
                else
                    glTexImage2D(GL_TEXTURE_2D, 0, texture_format, bmp->w, bmp->h, 0, GL_RGB, GL_UNSIGNED_BYTE, bmp->pixels);
                break;
            case 32:
                if (build_mipmaps && !can_useGenMipMaps)        // Software mipmaps generation
                    gluBuild2DMipmaps(GL_TEXTURE_2D, texture_format, bmp->w, bmp->h, GL_RGBA, GL_UNSIGNED_BYTE, bmp->pixels);
                else
                    glTexImage2D(GL_TEXTURE_2D, 0, texture_format, bmp->w, bmp->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, bmp->pixels);
                break;
            default:
                LOG_DEBUG("SDL_Surface format not supported by texture loader: " << bmp->format->BitsPerPixel << " bpp" );
            };
            if (g_useGenMipMaps && glGenerateMipmapEXT && build_mipmaps)
                glGenerateMipmapEXT(GL_TEXTURE_2D);
        }
        else            // Generate mipmaps here since other methods are unreliable
        {
            int w = bmp->w << 1;
            int h = bmp->h << 1;
            int level = 0;
            do
            {
                w = Math::Max( w / 2, 1 );
                h = Math::Max( h / 2, 1 );
                SDL_Surface *tmp = create_surface_ex( bmp->format->BitsPerPixel, w, h);
                stretch_blit(bmp, tmp, 0, 0, bmp->w, bmp->h, 0, 0, w, h);
                switch(tmp->format->BitsPerPixel)
                {
                case 8:
                    glTexImage2D(GL_TEXTURE_2D, level, texture_format, w, h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, tmp->pixels);
                    break;
                case 24:
                    glTexImage2D(GL_TEXTURE_2D, level, texture_format, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, tmp->pixels);
                    break;
                case 32:
                    glTexImage2D(GL_TEXTURE_2D, level, texture_format, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, tmp->pixels);
                    break;
                default:
                    LOG_DEBUG("SDL_Surface format not supported by texture loader: " << tmp->format->BitsPerPixel << " bpp" );
                };
                SDL_FreeSurface( tmp );
                level++;
            } while(w > 1 || h > 1);
        }

        if (filter_type == FILTER_NONE || filter_type == FILTER_LINEAR )
            use_mipmapping(true);

        return gl_tex;
    }

    GLuint GFX::make_texture_A32F( int w, int h, float *data, byte filter_type, bool clamp )
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

        set_texture_format(GL_RGB8);

        return gl_tex;
    }

    GLuint GFX::make_texture_A16F( int w, int h, float *data, byte filter_type, bool clamp )
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
                         GL_ALPHA16F_ARB,
                         w,
                         h,
                         0,
                         GL_ALPHA,
                         GL_FLOAT,
                         data );

        set_texture_format(GL_RGB8);

        return gl_tex;
    }
    GLuint GFX::make_texture_RGBA32F( int w, int h, float *data, byte filter_type, bool clamp )
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

        set_texture_format(GL_RGB8);

        return gl_tex;
    }

    GLuint GFX::make_texture_RGBA16F( int w, int h, float *data, byte filter_type, bool clamp )
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

        set_texture_format(GL_RGB8);

        return gl_tex;
    }

    GLuint GFX::make_texture_RGB32F( int w, int h, float *data, byte filter_type, bool clamp )
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

        set_texture_format(GL_RGB8);

        return gl_tex;
    }

    GLuint GFX::make_texture_RGB16F( int w, int h, float *data, byte filter_type, bool clamp )
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

        set_texture_format(GL_RGB8);

        return gl_tex;
    }

    GLuint GFX::create_texture(int w, int h, byte filter_type, bool clamp )
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

    GLuint GFX::create_texture_RGB32F(int w, int h, byte filter_type, bool clamp )
    {
        float *tmp = new float[w*h*3];
        GLuint tex = this->make_texture_RGB32F(w, h, tmp, filter_type, clamp);
        delete[] tmp;
        return tex;
    }

    GLuint GFX::create_texture_RGBA32F(int w, int h, byte filter_type, bool clamp )
    {
        float *tmp = new float[w*h*4];
        GLuint tex = this->make_texture_RGBA32F(w, h, tmp, filter_type, clamp);
        delete[] tmp;
        return tex;
    }

    GLuint GFX::create_texture_RGB16F(int w, int h, byte filter_type, bool clamp )
    {
        float *tmp = new float[w*h*3];
        GLuint tex = this->make_texture_RGB16F(w, h, tmp, filter_type, clamp);
        delete[] tmp;
        return tex;
    }

    GLuint GFX::create_texture_RGBA16F(int w, int h, byte filter_type, bool clamp )
    {
        float *tmp = new float[w*h*4];
        GLuint tex = this->make_texture_RGBA16F(w, h, tmp, filter_type, clamp);
        delete[] tmp;
        return tex;
    }

    void GFX::blit_texture( SDL_Surface *src, GLuint dst )
    {
        if(!dst)
            return;

        glBindTexture( GL_TEXTURE_2D, dst );

        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, src->w, src->h, GL_RGBA, GL_UNSIGNED_BYTE, src->pixels );
    }

    // FIXME: ugly thing, we shouldn't need an extra temporary file, image should be loaded directly from memory
    SDL_Surface *GFX::load_image(const String filename)
    {
        if (HPIManager)
        {
            uint32 image_file_size;
            byte *data = HPIManager->PullFromHPI( filename, &image_file_size );
            if (data)
            {
                SDL_RWops *file = SDL_RWFromMem( data, image_file_size);
                SDL_Surface *img = NULL;
                if (Paths::ExtractFileExt(filename).toLower() == ".tga")
                    img = IMG_LoadTGA_RW(file);
                else
                    img = IMG_Load_RW( file, 0);
                SDL_RWclose( file );

                delete[] data;

                if (img)
                {
                    if (img->format->BitsPerPixel == 32)
                        img = convert_format(img);
                    else
                        img = convert_format_24(img);
                }
                else
                    LOG_ERROR(LOG_PREFIX_GFX << "could not load image file: " << filename << " (vfs)");
                return img;
            }
            else
                LOG_ERROR(LOG_PREFIX_GFX << "could not read image file: " << filename << " (vfs)");
            return NULL;
        }
        SDL_Surface *img = IMG_Load(filename.c_str());
        if (img)
        {
            if (img->format->BitsPerPixel == 32)
                img = convert_format(img);
            else
                img = convert_format_24(img);
        }
        else
            LOG_ERROR(LOG_PREFIX_GFX << "could not load image file: " << filename);
        return img;
    }


    GLuint GFX::load_texture(String file, byte filter_type, uint32 *width, uint32 *height, bool clamp, GLuint texFormat )
    {
        if (!exists( file.c_str()) && (HPIManager == NULL || !HPIManager->Exists(file))) // The file doesn't exist
            return 0;

        SDL_Surface *bmp = load_image( file );
        if (bmp == NULL )	return 0;					// Operation failed
        if (width )		*width = bmp->w;
        if (height )	*height = bmp->h;
        bmp = convert_format(bmp);
        bool with_alpha = (String::ToLower(Paths::ExtractFileExt(file)) == ".tga") || (String::ToLower(Paths::ExtractFileExt(file)) == ".png");
        if (with_alpha)
        {
            with_alpha = false;
            for( int y = 0 ; y < bmp->h && !with_alpha ; y++ )
            {
                for( int x = 0 ; x < bmp->w && !with_alpha ; x++ )
                    with_alpha |= geta(SurfaceInt(bmp,x,y)) != 255;
            }
        }
        if (texFormat == 0)
        {
            if (g_useTextureCompression && lp_CONFIG->use_texture_compression)
                set_texture_format( with_alpha ? GL_COMPRESSED_RGBA_ARB : GL_COMPRESSED_RGB_ARB );
            else
                set_texture_format( with_alpha ? GL_RGBA8 : GL_RGB8 );
        }
        else
            set_texture_format( texFormat );
        GLuint gl_tex = make_texture( bmp, filter_type, clamp );
        SDL_FreeSurface(bmp);
        return gl_tex;
    }

    GLuint	GFX::load_texture_mask( String file, int level, byte filter_type, uint32 *width, uint32 *height, bool clamp )
    {
        if (!exists( file.c_str()) && (HPIManager == NULL || !HPIManager->Exists(file))) // The file doesn't exist
            return 0;

        SDL_Surface *bmp = load_image(file);
        if (bmp == NULL )	return 0;					// Operation failed
        if (width )		*width = bmp->w;
        if (height )	*height = bmp->h;
        if (bmp->format->BitsPerPixel != 32 )
            bmp = convert_format( bmp );
        bool with_alpha = (String::ToLower(Paths::ExtractFileExt(file)) == "tga");
        if (with_alpha)
        {
            with_alpha = false;
            for( int y = 0 ; y < bmp->h && !with_alpha ; y++ )
                for( int x = 0 ; x < bmp->w && !with_alpha ; x++ )
                    with_alpha |= (geta(SurfaceInt(bmp,x,y)) != 255);
        }
        else
        {
            for( int y = 0 ; y < bmp->h ; y++ )
                for( int x = 0 ; x < bmp->w ; x++ )
                    SurfaceInt(bmp,x,y) |= makeacol(0,0,0,255);
        }

        for( int y = 0 ; y < bmp->h ; y++ )
        {
            for( int x = 0 ; x < bmp->w ; x++ )
            {
                uint32 c = SurfaceInt(bmp,x,y);
                if (getr(c) < level && getg(c) < level
                    && getb(c) < level)
                {
                    SurfaceInt(bmp,x,y) = makeacol(getr(c), getg(c), getb(c), 0);
                    with_alpha = true;
                }
            }
        }
        if(g_useTextureCompression && lp_CONFIG->use_texture_compression)
            set_texture_format( with_alpha ? GL_COMPRESSED_RGBA_ARB : GL_COMPRESSED_RGB_ARB );
        else
            set_texture_format( with_alpha ? GL_RGBA8 : GL_RGB8 );
        GLuint gl_tex = make_texture( bmp, filter_type, clamp );
        SDL_FreeSurface(bmp);
        return gl_tex;
    }

    bool GFX::is_texture_in_cache( String file )
    {
        if(ati_workaround || !lp_CONFIG->use_texture_cache || !lp_CONFIG->use_texture_compression)
            return false;
        file = TA3D::Paths::Caches + file;
        if (TA3D::Paths::Exists(file))
        {
            FILE *cache_file = TA3D_OpenFile(file, "rb");
            uint32 mod_hash;
            fread(&mod_hash, sizeof( mod_hash ), 1, cache_file);
            fclose(cache_file);

            return mod_hash == TA3D_CURRENT_MOD.hashValue(); // Check if it corresponds to current mod
        }
        return false;
    }


    GLuint GFX::load_texture_from_cache( String file, byte filter_type, uint32 *width, uint32 *height, bool clamp )
    {
        if(ati_workaround || !lp_CONFIG->use_texture_cache || !lp_CONFIG->use_texture_compression || !g_useGenMipMaps || !g_useNonPowerOfTwoTextures)
            return 0;

        file = TA3D::Paths::Caches + file;
        if(TA3D::Paths::Exists(file))
        {
            FILE *cache_file = TA3D_OpenFile(file, "rb");
            uint32 mod_hash;
            fread(&mod_hash, sizeof( mod_hash ), 1, cache_file);

            if (mod_hash != TA3D_CURRENT_MOD.hashValue()) // Doesn't correspond to current mod
            {
                fclose(cache_file);
                return 0;
            }

            uint32 rw, rh;
            fread( &rw, 4, 1, cache_file );
            fread( &rh, 4, 1, cache_file );
            if(width)  *width = rw;
            if(height) *height = rh;

            int lod_max = 0;
            GLint size, internal_format;

            fread( &lod_max, sizeof( lod_max ), 1, cache_file );
            fread( &internal_format, sizeof( GLint ), 1, cache_file );

            GLuint	tex;
            glEnable(GL_TEXTURE_2D);
            glGenTextures(1,&tex);

            glBindTexture( GL_TEXTURE_2D, tex );
            if (glGenerateMipmapEXT)
                glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_FALSE);
            else
                glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE);

            GLint w,h,border;
            fread( &size, sizeof( GLint ), 1, cache_file );

            byte *img = new byte[size];

            fread( &internal_format, sizeof( GLint ), 1, cache_file );
            fread( &border, sizeof( GLint ), 1, cache_file );
            fread( &w, sizeof( GLint ), 1, cache_file );
            fread( &h, sizeof( GLint ), 1, cache_file );
            fread( img, size, 1, cache_file );
            glCompressedTexImage2D( GL_TEXTURE_2D, 0, internal_format, w, h, border, size, img);

            glGenerateMipmapEXT(GL_TEXTURE_2D);

            delete[] img;

            fclose( cache_file );

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
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, lp_CONFIG->anisotropy);

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
            return tex;
        }
        return 0; // File isn't in cache
    }


    void GFX::save_texture_to_cache( String file, GLuint tex, uint32 width, uint32 height )
    {
        if(ati_workaround || !lp_CONFIG->use_texture_cache || !lp_CONFIG->use_texture_compression || !g_useGenMipMaps || !g_useNonPowerOfTwoTextures)
            return;

        file = TA3D::Paths::Caches + file;

        int rw = texture_width( tex ), rh = texture_height( tex );		// Also binds tex

        GLint	compressed;
        glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_ARB, &compressed );
        // Do not save it if it's not compressed -> save disk space, and it would slow things down
        if(!compressed)
            return;

        FILE* cache_file = TA3D_OpenFile( file, "wb" );

        if (cache_file == NULL )
            return;

        uint32 mod_hash = TA3D_CURRENT_MOD.hashValue(); // Save a hash of current mod

        fwrite( &mod_hash, sizeof( mod_hash ), 1, cache_file );

        fwrite( &width, 4, 1, cache_file );
        fwrite( &height, 4, 1, cache_file );

        float lod_max_f = Math::Max(logf(rw), logf(rh)) / logf(2.0f);
        int lod_max = ((int) lod_max_f);
        if (lod_max > lod_max_f )
            lod_max++;

        fwrite( &lod_max, sizeof( lod_max ), 1, cache_file );

        GLint size, internal_format;
        glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format );
        fwrite( &internal_format, sizeof( GLint ), 1, cache_file );

        int lod = 0;

        glGetTexLevelParameteriv( GL_TEXTURE_2D, lod, GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB, &size );
        glGetTexLevelParameteriv( GL_TEXTURE_2D, lod, GL_TEXTURE_INTERNAL_FORMAT, &internal_format );

        byte *img = new byte[size];

        glGetCompressedTexImageARB( GL_TEXTURE_2D, lod, img );
        GLint w,h,border;
        glGetTexLevelParameteriv( GL_TEXTURE_2D, lod, GL_TEXTURE_BORDER, &border );
        glGetTexLevelParameteriv( GL_TEXTURE_2D, lod, GL_TEXTURE_WIDTH, &w );
        glGetTexLevelParameteriv( GL_TEXTURE_2D, lod, GL_TEXTURE_HEIGHT, &h );

        fwrite( &size, sizeof( GLint ), 1, cache_file );
        fwrite( &internal_format, sizeof( GLint ), 1, cache_file );
        fwrite( &border, sizeof( GLint ), 1, cache_file );
        fwrite( &w, sizeof( GLint ), 1, cache_file );
        fwrite( &h, sizeof( GLint ), 1, cache_file );
        fwrite( img, size, 1, cache_file );

        delete[] img;

        fclose( cache_file );
    }



    GLuint GFX::load_masked_texture(String file, String mask, byte filter_type )
    {
        if ( (!exists( file.c_str()) && (HPIManager == NULL || !HPIManager->Exists(file)))
             || (!exists( mask.c_str()) && (HPIManager == NULL || !HPIManager->Exists(mask))))
            return 0;		// The file doesn't exist

        SDL_Surface *bmp = load_image(file);
        if (bmp == NULL )	return 0;					// Operation failed
        SDL_Surface *alpha = load_image( mask );
        if(!alpha)
        {
            SDL_FreeSurface( bmp );
            return 0;
        }
        for(int y = 0; y < bmp->h; ++y)
            for(int x=0;x<bmp->w;x++)
            {
                uint32 c = SurfaceInt(bmp, x, y);
                SurfaceInt(bmp, x, y) = makeacol( getr(c), getg(c), getb(c), geta(SurfaceInt(alpha,x,y)) );
            }
        if(g_useTextureCompression && lp_CONFIG->use_texture_compression)
            set_texture_format(GL_COMPRESSED_RGBA_ARB);
        else
            set_texture_format(GL_RGBA8);
        GLuint gl_tex = make_texture( bmp, filter_type );
        SDL_FreeSurface(bmp);
        SDL_FreeSurface(alpha);
        return gl_tex;
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
            glDeleteTextures(1,&gltex);
        gltex = 0;						// The texture is destroyed
    }

    GLuint GFX::make_texture_from_screen( byte filter_type)				// Copy pixel data from screen to a texture
    {
        GLuint gltex = create_texture(SCREEN_W, SCREEN_H, filter_type);

        glBindTexture(GL_TEXTURE_2D,gltex);
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, SCREEN_W, SCREEN_H, 0);

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
            for(uint32 i = 0; i < 7; ++i)
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
        glFogi (GL_FOG_COORD_SRC, GL_FOG_COORD);
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


    uint32 GFX::InterfaceMsg(const lpcImsg msg)
    {
        if (msg->MsgID != TA3D_IM_GFX_MSG )
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
        SCREEN_W_HALF = SCREEN_W>>1;
        SCREEN_H_HALF = SCREEN_H>>1;
        SCREEN_W_INV = 1.0f / SCREEN_W;
        SCREEN_H_INV = 1.0f / SCREEN_H;
        SCREEN_W_TO_640 = 640.0f / SCREEN_W;
        SCREEN_H_TO_480 = 480.0f / SCREEN_H;
    }


    void GFX::load_background()
    {
        if (SCREEN_W<=800)
            glfond = load_texture( "gfx/menu800.jpg", FILTER_LINEAR);
        else
        {
            if(SCREEN_W<=1024)
                glfond = load_texture( "gfx/menu1024.jpg", FILTER_LINEAR);
            else
                glfond = load_texture( "gfx/menu1280.jpg", FILTER_LINEAR);
        }
    }


    void GFX::renderToTexture( const GLuint tex, bool useDepth )
    {
        if (!g_useFBO && textureFBO != 0)                   // Renders to back buffer when FBO isn't available
        {
            glBindTexture(GL_TEXTURE_2D,textureFBO);        // Copy back buffer to target texture
            glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, texture_width(textureFBO), texture_height(textureFBO), 0);
            textureFBO = 0;
            glViewport(0, 0, SCREEN_W, SCREEN_H);           // Use default viewport
        }

        if (tex == 0)       // Release the texture
        {
            if (g_useFBO)
            {
                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);     // Bind the default FBO
                glViewport(0, 0, SCREEN_W, SCREEN_H);           // Use default viewport
            }
        }
        else
        {
            if (g_useFBO)               // If FBO extension is available then use it
            {
                if (textureFBO == 0)    // Generate a FBO if none has been created yet
                {
                    glGenFramebuffersEXT(1,&textureFBO);
	                glGenRenderbuffersEXT(1,&textureDepth);
	            }

                glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, textureFBO);					                    // Bind the FBO
                glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,tex,0); // Attach the texture
                if (useDepth)
                {
            		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_RENDERBUFFER_EXT,textureDepth);
	                glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, textureDepth);
	                glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,GL_DEPTH_COMPONENT24, texture_width(tex), texture_height(tex));       // Should be enough
	            }
            	else
            	{
	                glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0);
            		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_RENDERBUFFER_EXT,0);
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

    void GFX::renderToTextureDepth( const GLuint tex )
    {
        if (tex == 0)       // Release the texture
        {
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);     // Bind the default FBO
            glViewport(0, 0, SCREEN_W, SCREEN_H);           // Use default viewport
        }
        else
        {
            int tex_w = texture_width(tex);
            int tex_h = texture_height(tex);
            if (textureFBO == 0)    // Generate a FBO if none has been created yet
            {
                glGenFramebuffersEXT(1,&textureFBO);
                glGenRenderbuffersEXT(1,&textureDepth);
            }
            if (textureColor == 0)
                textureColor = create_texture(tex_w, tex_h, FILTER_NONE, true);
            else
            {
                glBindTexture(GL_TEXTURE_2D, textureColor);
                glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, tex_w, tex_h, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
            }

            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, textureFBO);					                    // Bind the FBO
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_COLOR_ATTACHMENT0_EXT,GL_TEXTURE_2D,textureColor,0);
            glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D,tex,0); // Attach the texture

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

    void GFX::runTests()
    {
		InterfaceManager = new IInterfaceManager();

		// Initalizing SDL video
		if( SDL_Init(SDL_INIT_VIDEO) < 0 )
			throw( "SDL_Init(SDL_INIT_VIDEO) yielded unexpected result." );

		// Installing SDL timer
		if( SDL_InitSubSystem(SDL_INIT_TIMER) != 0 )
			throw( "SDL_InitSubSystem(SDL_INIT_TIMER) yielded unexpected result." );

		// Installing SDL timer
		if( SDL_InitSubSystem(SDL_INIT_EVENTTHREAD) != 0 )
			throw( "SDL_InitSubSystem(SDL_INIT_EVENTTHREAD) yielded unexpected result." );

        GFX *test_gfx = new GFX();

        test_gfx->set_2D_mode();

        int         filter[]        = { FILTER_NONE, FILTER_LINEAR, FILTER_BILINEAR, FILTER_TRILINEAR };
        const char  *filterInfo[]   = { "FILTER_NONE", "FILTER_LINEAR", "FILTER_BILINEAR", "FILTER_TRILINEAR" };

        for (int e = 0 ; e < 4 ; e++)
        {
            GLuint tex[11];

            GLuint  texFormat[] =   { GL_COMPRESSED_RGBA_ARB, GL_COMPRESSED_RGB_ARB, GL_RGB8, GL_RGBA8, GL_RGB5, GL_RGB5_A1, GL_RGB4, GL_RGBA4, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT };
            const char  *info[] =   { "COMPRESSED_RGBA", "COMPRESSED_RGB", "RGB8", "RGBA8", "RGB5", "RGB5_A1", "RGB4", "RGBA4", "COMPRESSED_RGBA_S3TC_DXT1", "COMPRESSED_RGBA_S3TC_DXT3", "COMPRESSED_RGBA_S3TC_DXT5" };

            for (int i = 0 ; i < 11 ; i++)
                tex[i] = test_gfx->load_texture("gfx/mdrn_background.jpg", filter[e], NULL, NULL, true, texFormat[i]);

            while (!keypressed())
            {
                poll_inputs();
                rest( 100 );
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Efface l'écran
                for (int i = 0 ; i < 11 ; i++)
                {
                    float dx = (i >> 2) * SCREEN_W / 3.0f;
                    float dy = (i & 3) * 0.25f * SCREEN_H;
                    test_gfx->drawtexture( tex[i], dx, dy, dx + SCREEN_W / 3.0f, dy + 0.25f * SCREEN_H );
                    glDisable( GL_TEXTURE_2D );
                    test_gfx->rect( dx, dy, dx + SCREEN_W / 3.0f, dy + 0.25 * SCREEN_H, 0xFFFFFFFF );
                    test_gfx->print( test_gfx->normal_font, dx + 10, dy + 10, 0.0f, 0xFFFFFFFF, info[i] );
                    test_gfx->print( test_gfx->normal_font, dx + 10, dy + 20, 0.0f, 0xFFFFFFFF, filterInfo[e] );
                }

                test_gfx->flip();
            }

            for (int i = 0 ; i < 11 ; i++)
                test_gfx->destroy_texture( tex[i] );

            while (keypressed())    readkey();
        }

        delete test_gfx;

        delete InterfaceManager;
    }

    SDL_Surface *GFX::create_surface_ex(int bpp, int w, int h)
    {
        return SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, bpp,
                                    0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
    }

    SDL_Surface *GFX::create_surface(int w, int h)
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

    SDL_Surface* GFX::LoadMaskedTextureToBmp(const String& file, const String& filealpha)
    {
        // Load the texture (32Bits)
        SDL_Surface* bmp = gfx->load_image(file);
        LOG_ASSERT(bmp != NULL);

        // Load the mask
        SDL_Surface* alpha = gfx->load_image(filealpha);
        LOG_ASSERT(alpha != NULL);

        // Apply the mask, pixel by pixel
        for (int y = 0; y < bmp->h; ++y)
        {
            for (int x = 0; x < bmp->w; ++x)
                SurfaceByte(bmp, (x << 2) + 3, y) = SurfaceInt(alpha,x,y);
        }

        SDL_FreeSurface(alpha);
        return bmp;
    }

    GLuint GFX::create_shadow_map(int w, int h)
    {
        GLuint shadowMapTexture;

        glGenTextures(1, &shadowMapTexture);
        glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
        glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, w, h, 0,
                    GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);       // We want smooth shadows
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        return shadowMapTexture;
    }

    GLuint GFX::get_shadow_map()
    {
        if (shadowMap == 0)
            shadowMap = create_shadow_map(1024, 1024);
        return shadowMap;
    }

    void GFX::enable_model_shading(int mode)
    {
        if (shadowMapMode)  return;
        switch(lp_CONFIG->shadow_quality)
        {
        case 2:
            switch(mode)
            {
            case 0:
            case 1:
                if (!model_shader.isLoaded())
                    model_shader.load("shaders/3do_shadow.frag", "shaders/3do_shadow.vert");
                if (model_shader.isLoaded())
                {
                    model_shader.on();
                    model_shader.setvar1i("shadowMap", 7);
                    model_shader.setmat4f("light_Projection", shadowMapProjectionMatrix);
                }
                break;
            default:
                model_shader.off();
            };
            break;
        };
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

    void GFX::runOpenGLTests()
    {
		// Initalizing SDL video
		if( SDL_Init(SDL_INIT_VIDEO) < 0 )
			throw( "SDL_Init(SDL_INIT_VIDEO) yielded unexpected result." );

		// Installing SDL timer
		if( SDL_InitSubSystem(SDL_INIT_TIMER) != 0 )
			throw( "SDL_InitSubSystem(SDL_INIT_TIMER) yielded unexpected result." );

		// Installing SDL timer
		if( SDL_InitSubSystem(SDL_INIT_EVENTTHREAD) != 0 )
			throw( "SDL_InitSubSystem(SDL_INIT_EVENTTHREAD) yielded unexpected result." );

        int w = 800;
        int h = 600;
        int bpp = 32;

        SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);

        SDL_GL_SetAttribute(SDL_GL_ACCUM_RED_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_ACCUM_GREEN_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_ACCUM_BLUE_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_ACCUM_ALPHA_SIZE, 0);


        for(int stencil = 0 ; stencil < 2 ; stencil++)
        {
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, stencil ? 8 : 0);
            for(int fsaa = 0 ; fsaa < 4 ; fsaa++)
            {
                SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, fsaa > 1);
                SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, fsaa);
                for(int db_buf = 0 ; db_buf < 2 ; db_buf++)
                {
                    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, db_buf);
                    for(int r = 8 ; r > 0 ; r--)
                    {
                        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, r);
                        for(int g = 8 ; g > 0 ; g--)
                        {
                            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, g);
                            for(int b = 8 ; b > 0 ; b--)
                            {
                                SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, b);
                                for(int a = 8 ; a > 0 ; a--)
                                {
                                    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, a);

                                    for(int depth = 0 ; depth < 2 ; depth++)
                                    {
                                        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, depth ? 24 : 16);

                                        if (SDL_SetVideoMode( w, h, bpp, SDL_OPENGL | SDL_HWSURFACE ))
                                        {
                                            if (String((char*)glGetString(GL_RENDERER)).toLower() != "gdi generic")
                                            {
                                                std::cout << "test passed for following settings:" << std::endl;
                                                std::cout << "stencil = " << (stencil ? 8 : 0) << std::endl;
                                                std::cout << "fsaa = " << fsaa << std::endl;
                                                std::cout << "db_buf = " << db_buf << std::endl;
                                                std::cout << "r = " << r << std::endl;
                                                std::cout << "g = " << g << std::endl;
                                                std::cout << "b = " << b << std::endl;
                                                std::cout << "a = " << a << std::endl;
                                                std::cout << "depth = " << (depth ? 24 : 16) << std::endl;
                                                std::cout << "renderer = " << glGetString(GL_RENDERER) << std::endl << std::endl;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        std::cout << "Test finished" << std::endl;
    }

    void GFX::readShadowMapProjectionMatrix()
    {
        GLfloat backup[16];
        glGetFloatv(GL_PROJECTION_MATRIX, backup);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glScalef(0.5f,0.5f,0.5f);
        glTranslatef(1.0f,1.0f,1.0f);
        glMultMatrixf(backup);
        glGetFloatv(GL_PROJECTION_MATRIX, gfx->shadowMapProjectionMatrix);

        glLoadIdentity();
        glMultMatrixf(backup);
        glMatrixMode(GL_MODELVIEW);
    }
} // namespace TA3D
