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
#include "../misc/paths.h"
#include "../logs/logs.h"
#include <allegro/internal/aintern.h>
#include <strings.h>
#include "../misc/math.h"


#define TA3D_OPENGL_PREFIX "[OpenGL] "
#define YESNO(X)  (X ? "Yes" : "No")


namespace TA3D
{
namespace Interfaces
{

    void GFX::initAllegroGL()
    {
        install_allegro_gl();
        #ifndef TA3D_PLATFORM_DARWIN
        allegro_gl_clear_settings();         // Initialise AllegroGL
        allegro_gl_set (AGL_STENCIL_DEPTH, 8 );
        allegro_gl_set (AGL_SAMPLE_BUFFERS, 0 );
        allegro_gl_set (AGL_SAMPLES, TA3D::VARS::lp_CONFIG->fsaa );
        allegro_gl_set (AGL_VIDEO_MEMORY_POLICY, AGL_RELEASE );
        allegro_gl_set (AGL_COLOR_DEPTH, TA3D::VARS::lp_CONFIG->color_depth );
        allegro_gl_set (AGL_Z_DEPTH, 32 );
        allegro_gl_set (AGL_FULLSCREEN, TA3D::VARS::lp_CONFIG->fullscreen);
        allegro_gl_set (AGL_DOUBLEBUFFER, 1 );
        allegro_gl_set (AGL_RENDERMETHOD, 1 );
        allegro_gl_set (AGL_SUGGEST, AGL_RENDERMETHOD | AGL_COLOR_DEPTH | AGL_Z_DEPTH
                        | AGL_DOUBLEBUFFER | AGL_FULLSCREEN | AGL_SAMPLES
                        | AGL_SAMPLE_BUFFERS | AGL_STENCIL_DEPTH
                        | AGL_VIDEO_MEMORY_POLICY );
        request_refresh_rate(85);
        #else
        allegro_gl_set(AGL_COLOR_DEPTH, TA3D::VARS::lp_CONFIG->color_depth);
        allegro_gl_set(AGL_DOUBLEBUFFER, TRUE);
        allegro_gl_set(AGL_Z_DEPTH, 32);
        allegro_gl_set(AGL_WINDOWED, TRUE);
        allegro_gl_set(AGL_RENDERMETHOD, TRUE);
        allegro_gl_set (AGL_VIDEO_MEMORY_POLICY, AGL_RELEASE );
        allegro_gl_set (AGL_FULLSCREEN, TA3D::VARS::lp_CONFIG->fullscreen);
        allegro_gl_set(AGL_SAMPLES, TA3D::VARS::lp_CONFIG->fsaa);
        allegro_gl_set(AGL_SAMPLE_BUFFERS, TRUE);
        allegro_gl_set(AGL_SUGGEST, AGL_WINDOWED | AGL_SAMPLES | AGL_SAMPLE_BUFFERS | AGL_COLOR_DEPTH | AGL_VIDEO_MEMORY_POLICY);
        allegro_gl_set(AGL_REQUIRE, AGL_RENDERMETHOD | AGL_DOUBLEBUFFER | AGL_Z_DEPTH);
        #endif

        allegro_gl_use_mipmapping(TRUE);
        allegro_gl_flip_texture(FALSE);


        if (TA3D::VARS::lp_CONFIG->fullscreen )
        {
            # ifdef GFX_OPENGL_FULLSCREEN
            set_gfx_mode(GFX_OPENGL_FULLSCREEN, TA3D::VARS::lp_CONFIG->screen_width, TA3D::VARS::lp_CONFIG->screen_height, 0, 0);
            # else
            allegro_message(TRANSLATE("WARNING: AllegroGL has been built without fullscreen support.\nrunning in fullscreen mode impossible\nplease install libxxf86vm and rebuild both Allegro and AllegroGL\nif you want fullscreen modes.").c_str());
            set_gfx_mode(GFX_OPENGL_WINDOWED, TA3D::VARS::lp_CONFIG->screen_width, TA3D::VARS::lp_CONFIG->screen_height, 0, 0);
            # endif
        }
        else
            set_gfx_mode(GFX_OPENGL_WINDOWED, TA3D::VARS::lp_CONFIG->screen_width, TA3D::VARS::lp_CONFIG->screen_height, 0, 0);

        preCalculations();
        // Install OpenGL extensions
        installOpenGLExtensions();

        if(g_useTextureCompression) // Try to enabled the Texture compression
            allegro_gl_set_texture_format(GL_COMPRESSED_RGB_ARB);
        else
            allegro_gl_set_texture_format(GL_RGB8); 
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
        if (Console)
        {
            Console->stdout_on();
            Console->AddEntry("%sOpenGL informations:", TA3D_OPENGL_PREFIX);
            Console->AddEntry("%sVendor: %s",   TA3D_OPENGL_PREFIX, glGetString(GL_VENDOR));
            Console->AddEntry("%sRenderer: %s", TA3D_OPENGL_PREFIX, glGetString(GL_RENDERER));
            Console->AddEntry("%sVersion: %s",  TA3D_OPENGL_PREFIX, glGetString(GL_VERSION));
            if (ati_workaround)
                Console->AddEntryWarning("ATI card detected ! Using workarounds for ATI cards");
            Console->stdout_off();
        }
        else
        {
            LOG_INFO(TA3D_OPENGL_PREFIX << "OpenGL informations:");
            LOG_INFO(TA3D_OPENGL_PREFIX << "Vendor: " << glGetString(GL_VENDOR));
            LOG_INFO(TA3D_OPENGL_PREFIX << "Renderer: " << glGetString(GL_RENDERER));
            LOG_INFO(TA3D_OPENGL_PREFIX << "Vrsion: " << glGetString(GL_VERSION));
            if (ati_workaround)
                LOG_WARNING("ATI card detected ! Using workarounds for ATI cards");
        }
        LOG_INFO(TA3D_OPENGL_PREFIX << "Texture compression: " << YESNO(g_useTextureCompression));
        LOG_INFO(TA3D_OPENGL_PREFIX << "Stencil Two Side: " << YESNO(g_useStencilTwoSide));
        LOG_INFO(TA3D_OPENGL_PREFIX << "FBO: " << YESNO(g_useFBO));
        LOG_INFO(TA3D_OPENGL_PREFIX << "Shaders: " << YESNO(g_useProgram));
        LOG_INFO(TA3D_OPENGL_PREFIX << "Multi texturing: " << YESNO(MultiTexturing));
    }



    GFX::GFX()
    {
        initAllegroGL();
        ati_workaround = checkVideoCardWorkaround();
        
        TA3D::VARS::pal = NULL;

        width = SCREEN_W;
        height = SCREEN_H;
        x = 0;
        y = 0;
        low_def_limit = 600.0f;

        textureFBO = 0;
        textureDepth = 0;
        glfond = 0;
        normal_font.init();
        small_font.init();
        TA_font.init();
        ta3d_gui_font.init();
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

        if (TA3D::VARS::pal )
            delete[]( TA3D::VARS::pal ); 

        normal_font.destroy();
        small_font.destroy();
        TA_font.destroy();
        ta3d_gui_font.destroy();
    }



    void GFX::Init()
    {
        LOG_DEBUG("Allocating palette memory...");
        TA3D::VARS::pal = new RGB[256];      // Allocate a new palette

        LOG_DEBUG("Loading TA's palette...");
        bool palette = TA3D::UTILS::HPI::load_palette(pal);
        if (!palette)
            LOG_WARNING("Failed to load the palette");

        TA_font.load_gaf_font( "anims\\hattfont12.gaf", 1.0f );

        if (Console)
            Console->AddEntry( "Creating a normal font...");
        normal_font.copy( font , 1.0f );
        normal_font.set_clear( true );
        if (Console)
            Console->AddEntry( "Creating a small font...");
        small_font.copy( font , 0.75f );
        small_font.set_clear( true );

        if(Console)
            Console->AddEntry( "Loading the GUI font..." );
        ta3d_gui_font.load_gaf_font( "anims\\hattfont12.gaf" , 1.0f );

        if(Console)
            Console->AddEntry( "Activating the palette...");
        if(palette)
            set_palette(pal);      // Activate the palette

        if (Console)
            Console->AddEntry( "Loading background...");
        load_background();

        gui_font = ta3d_gui_font;
        gui_font_h = gui_font.height();
        use_normal_alpha_function = true;
        alpha_blending_set = false;
        if(Console)
        {
            Console->stdout_on();
            Console->AddEntry( "Graphics are initialized.");
            Console->stdout_off();
        }
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
        allegro_gl_set_allegro_mode();
    }

    void GFX::unset_2D_mode()
    {
        allegro_gl_unset_allegro_mode();
    }

    void GFX::line(const float x1, const float y1, const float x2, const float y2)			// Basic drawing routines
    {
        glBegin(GL_LINES);
        glVertex2f(x1,y1);
        glVertex2f(x2,y2);
        glEnd();
    }
    void GFX::line(const float x1, const float y1, const float x2, const float y2, const uint32 col)
    {
        set_color(col);
        line(x1,y1,x2,y2);
    }


    void GFX::rect(const float x1, const float y1, const float x2, const float y2)
    {
        glBegin(GL_LINE_LOOP);
        glVertex2f(x1,y1);
        glVertex2f(x2,y1);
        glVertex2f(x2,y2);
        glVertex2f(x1,y2);
        glEnd();
    }
    void GFX::rect(const float x1, const float y1, const float x2, const float y2, const uint32 col)
    {
        set_color(col);
        rect(x1,y1,x2,y2);
    }


    void GFX::rectfill(const float x1, const float y1, const float x2, const float y2)
    {
        glBegin(GL_QUADS);
        glVertex2f(x1,y1);
        glVertex2f(x2,y1);
        glVertex2f(x2,y2);
        glVertex2f(x1,y2);
        glEnd();
    }
    void GFX::rectfill(const float x1, const float y1, const float x2, const float y2, const uint32 col)
    {
        set_color(col);
        rectfill(x1,y1,x2,y2);
    }


    void GFX::circle(const float x, const float y, const float r)
    {
        float d_alpha = DB_PI/(r+1.0f);
        glBegin( GL_LINE_LOOP );
        for( float alpha = 0.0f; alpha <= DB_PI; alpha+=d_alpha )
            glVertex2f( x+r*cos(alpha), y+r*sin(alpha) );
        glEnd();
    }
    void GFX::circle(const float x, const float y, const float r, const uint32 col)
    {
        set_color(col);
        circle(x,y,r);
    }

    void GFX::circle_zoned(const float x, const float y, const float r, const float mx, const float my, const float Mx, const float My)
    {
        float d_alpha = DB_PI/(r+1.0f);
        glBegin( GL_LINE_LOOP );
        for (float alpha = 0.0f; alpha <= DB_PI; alpha += d_alpha)
        {
            float rx = x+r*cos(alpha);
            float ry = y+r*sin(alpha);
            if (rx < mx )		rx = mx;
            else if (rx > Mx )	rx = Mx;
            if (ry < my )		ry = my;
            else if (ry > My )	ry = My;
            glVertex2f( rx, ry );
        }
        glEnd();
    }

    void GFX::circle_zoned(const float x, const float y, const float r, const float mx, const float my, const float Mx, const float My, const uint32 col)
    {
        set_color(col);
        circle_zoned(x,y,r,mx,my,Mx,My);
    }

    void GFX::circlefill(const float x, const float y, const float r)
    {
        float d_alpha = DB_PI/(r+1.0f);
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(x,y);
        for( float alpha = 0.0f; alpha <= DB_PI; alpha += d_alpha)
            glVertex2f( x+r*cos(alpha), y+r*sin(alpha) );
        glEnd();
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
        glBegin(GL_QUADS);
        glTexCoord2f(u1,v1);		glVertex2f(x1,y1);
        glTexCoord2f(u2,v1);		glVertex2f(x2,y1);
        glTexCoord2f(u2,v2);		glVertex2f(x2,y2);
        glTexCoord2f(u1,v2);		glVertex2f(x1,y2);
        glEnd();
    }
    void GFX::drawtexture(const GLuint &tex, const float x1, const float y1, const float x2, const float y2)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D,tex);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f,0.0f);		glVertex2f(x1,y1);
        glTexCoord2f(1.0f,0.0f);		glVertex2f(x2,y1);
        glTexCoord2f(1.0f,1.0f);		glVertex2f(x2,y2);
        glTexCoord2f(0.0f,1.0f);		glVertex2f(x1,y2);
        glEnd();
    }
    void GFX::drawtexture_flip(const GLuint &tex, const float x1, const float y1, const float x2, const float y2)
    {
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D,tex);
        glBegin(GL_QUADS);
        glTexCoord2f(0.0f,0.0f);		glVertex2f(x1,y1);
        glTexCoord2f(0.0f,1.0f);		glVertex2f(x2,y1);
        glTexCoord2f(1.0f,1.0f);		glVertex2f(x2,y2);
        glTexCoord2f(1.0f,0.0f);		glVertex2f(x1,y2);
        glEnd();
    }
    void GFX::drawtexture(const GLuint &tex, const float x1, const float y1, const float x2, const float y2, const uint32 col)
    {
        set_color(col);
        drawtexture( tex, x1, y1, x2, y2 );
    }


    void GFX::print(const GfxFont &font, const float x, const float y, const float z, const String text)		// Font related routines
    {
        print( font, x, y, z, text.c_str() );
    }
    void GFX::print(const GfxFont &font, const float x, const float y, const float z, const uint32 col, const String text)		// Font related routines
    {
        print( font, x, y, z, col, text.c_str() );
    }

    void GFX::print(const GfxFont& font, const float x, const float y, const float z, const char *text)		// Font related routines
    {
        ReInitTexSys( false );
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glEnable(GL_BLEND);
        if(font.clear)
            glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
        else
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_TEXTURE_2D);
        glPushMatrix();
        glScalef(font.size,font.size,1.0f);
        allegro_gl_printf_ex(font.pGl, x/font.size, y/font.size, z, text );
        glPopMatrix();
        glPopAttrib();
    }
    void GFX::print(const GfxFont &font, const float x, const float y, const float z, const uint32 col, const char *text)		// Font related routines
    {
        set_color(col);
        print( font, x, y, z, text );
    }

    void GFX::print(const GfxFont &font, const float x, const float y, const float z, const String text, float s)		// Font related routines
    {
        print( font, x, y, z, text.c_str() , s );
    }
    void GFX::print(const GfxFont &font, const float x, const float y, const float z, const uint32 col, const String text, float s)		// Font related routines
    {
        print( font, x, y, z, col, text.c_str() , s );
    }

    void GFX::print(const GfxFont &font, const float x, const float y, const float z, const char *text,float s)		// Font related routines
    {
        ReInitTexSys( false );
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glEnable(GL_BLEND);
        if(font.clear)
            glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
        else
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_TEXTURE_2D);
        glPushMatrix();
        if (s > 0.0f )	{
            glScalef(s,s,1.0f);
            allegro_gl_printf_ex(font.pGl, x/s, y/s, z, text );
        }
        glPopMatrix();
        glPopAttrib();
    }
    void GFX::print(const GfxFont &font, const float x, const float y, const float z, const uint32 col, const char *text,float s)		// Font related routines
    {
        set_color(col);
        print( font, x, y, z, text , s );
    }


    void GFX::print_center(const GfxFont &font, const float x, const float y, const float z, const String text)		// Font related routines
    {
        print_center( font, x, y, z, text.c_str() );
    }
    void GFX::print_center(const GfxFont &font, const float x, const float y, const float z, const uint32 col, const String text)		// Font related routines
    {
        print_center( font, x, y, z, col, text.c_str() );
    }

    void GFX::print_center(const GfxFont &font, const float x, const float y, const float z, const char *text)		// Font related routines
    {
        ReInitTexSys( false );
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glEnable(GL_BLEND);
        if(font.clear)
            glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
        else
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

        float X = x - 0.5f * font.length( text );

        glEnable(GL_TEXTURE_2D);
        glPushMatrix();
        glScalef(font.size,font.size,1.0f);
        allegro_gl_printf_ex(font.pGl, X/font.size, y/font.size, z, text );
        glPopMatrix();
        glPopAttrib();
    }
    void GFX::print_center(const GfxFont &font, const float x, const float y, const float z, const uint32 col, const char *text)		// Font related routines
    {
        set_color(col);
        print_center( font, x, y, z, text );
    }

    void GFX::print_center(const GfxFont &font, const float x, const float y, const float z, const String text, float s)		// Font related routines
    {
        print_center( font, x, y, z, text.c_str() , s );
    }
    void GFX::print_center(const GfxFont &font, const float x, const float y, const float z, const uint32 col, const String text, float s)		// Font related routines
    {
        print_center( font, x, y, z, col, text.c_str() , s );
    }

    void GFX::print_center(const GfxFont &font, const float x, const float y, const float z, const char *text,float s)		// Font related routines
    {
        ReInitTexSys( false );
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glEnable(GL_BLEND);
        if(font.clear)
            glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
        else
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

        float X = x - 0.5f * font.length( text );

        glEnable(GL_TEXTURE_2D);
        glPushMatrix();
        if (s > 0.0f )
        {
            glScalef(s,s,1.0f);
            allegro_gl_printf_ex(font.pGl, X/s, y/s, z, text );
        }
        glPopMatrix();
        glPopAttrib();
    }


    void GFX::print_center(const GfxFont &font, const float x, const float y, const float z, const uint32 col, const char *text,float s)		// Font related routines
    {
        set_color(col);
        print_center( font, x, y, z, text , s );
    }


    void GFX::print_right(const GfxFont &font, const float x, const float y, const float z, const String text)		// Font related routines
    {
        print_right( font, x, y, z, text.c_str() );
    }
    void GFX::print_right(const GfxFont &font, const float x, const float y, const float z, const uint32 col, const String text)		// Font related routines
    {
        print_right( font, x, y, z, col, text.c_str() );
    }

    void GFX::print_right(const GfxFont &font, const float x, const float y, const float z, const char *text)		// Font related routines
    {
        ReInitTexSys( false );
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glEnable(GL_BLEND);
        if(font.clear)
            glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
        else
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

        float X = x - font.length( text );

        glEnable(GL_TEXTURE_2D);
        glPushMatrix();
        glScalef(font.size,font.size,1.0f);
        allegro_gl_printf_ex(font.pGl, X/font.size, y/font.size, z, text );
        glPopMatrix();
        glPopAttrib();
    }

    void GFX::print_right(const GfxFont &font, const float x, const float y, const float z, const uint32 col, const char *text)		// Font related routines
    {
        set_color(col);
        print_right( font, x, y, z, text );
    }


    void GFX::print_right(const GfxFont &font, const float x, const float y, const float z, const String text, float s)		// Font related routines
    {
        print_right( font, x, y, z, text.c_str() , s);
    }

    void GFX::print_right(const GfxFont &font, const float x, const float y, const float z, const uint32 col, const String text, float s)		// Font related routines
    {
        print_right( font, x, y, z, col, text.c_str() , s);
    }


    void GFX::print_right(const GfxFont &font, const float x, const float y, const float z, const char *text,float s)		// Font related routines
    {
        ReInitTexSys( false );
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glEnable(GL_BLEND);
        if(font.clear)
            glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
        else
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

        float X = x - font.length(text);

        glEnable(GL_TEXTURE_2D);
        glPushMatrix();
        if (s > 0.0f )
        {
            glScalef(s,s,1.0f);
            allegro_gl_printf_ex(font.pGl, X/s, y/s, z, text );
        }
        glPopMatrix();
        glPopAttrib();
    }

    void GFX::print_right(const GfxFont &font, const float x, const float y, const float z, const uint32 col, const char *text,float s)		// Font related routines
    {
        set_color(col);
        print_right( font, x, y, z, text , s );
    }

    GLuint GFX::make_texture(BITMAP *bmp, byte filter_type, bool clamp )
    {
        MutexLocker locker(pMutex);

        int max_tex_size;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE,&max_tex_size);

        if (bmp->w > max_tex_size || bmp->h > max_tex_size )
        {
            BITMAP *tmp = create_bitmap_ex(bitmap_color_depth( bmp ),
                                           Math::Min(bmp->w, max_tex_size), Math::Min(bmp->h, max_tex_size));
            stretch_blit( bmp, tmp, 0, 0, bmp->w, bmp->h, 0, 0, tmp->w, tmp->h );
            GLuint tex = make_texture( tmp, filter_type, clamp );
            destroy_bitmap( tmp );
            return tex;
        }

        glPushAttrib( GL_ALL_ATTRIB_BITS );

        if (ati_workaround && filter_type != FILTER_NONE
            && ( !Math::IsPowerOfTwo(bmp->w) || !Math::IsPowerOfTwo(bmp->h)))
            filter_type = FILTER_LINEAR;

        if (filter_type == FILTER_NONE || filter_type == FILTER_LINEAR )
            allegro_gl_use_mipmapping(false);
        else
            allegro_gl_use_mipmapping(true);
        GLuint gl_tex = allegro_gl_make_texture(bmp);
        if (filter_type == FILTER_NONE || filter_type == FILTER_LINEAR )
            allegro_gl_use_mipmapping(true);
        glBindTexture(GL_TEXTURE_2D, gl_tex);

        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        if (clamp )
        {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
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

        glPopAttrib();

        return gl_tex;
    }


    GLuint GFX::create_texture(int w, int h, byte filter_type, bool clamp )
    {
        BITMAP* tmp = create_bitmap(w, h);
        clear( tmp );
        GLuint tex = make_texture( tmp, filter_type, clamp );
        destroy_bitmap( tmp );
        return tex;
    }

    void GFX::blit_texture( BITMAP *src, GLuint &dst )
    {
        if(!dst)
            return;

        glBindTexture( GL_TEXTURE_2D, dst );

        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);

        glTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, src->w, src->h, GL_RGBA, GL_UNSIGNED_BYTE, src->line[0] );
    }



    GLuint GFX::load_texture(String file, byte filter_type, uint32 *width, uint32 *height, bool clamp )
    {
        if (!exists( file.c_str())) // The file doesn't exist
            return 0;

        set_color_depth(32);
        BITMAP *bmp = load_bitmap( file.c_str(), NULL);
        if (bmp == NULL )	return 0;					// Operation failed
        if (width )		*width = bmp->w;
        if (height )	*height = bmp->h;
        if (bitmap_color_depth(bmp) != 32 )
        {
            BITMAP *tmp = create_bitmap_ex( 32, bmp->w, bmp->h );
            blit( bmp, tmp, 0, 0, 0, 0, bmp->w, bmp->h);
            destroy_bitmap(bmp);
            bmp=tmp;
        }
        bool with_alpha = (String::ToLower(get_extension(file.c_str())) == "tga");
        if (with_alpha)
        {
            with_alpha = false;
            for( int y = 0 ; y < bmp->h && !with_alpha ; y++ )
            {
                for( int x = 0 ; x < bmp->w && !with_alpha ; x++ )
                    with_alpha |= bmp->line[y][(x<<2)+3] != 255;
            }
        }
        if(g_useTextureCompression)
            allegro_gl_set_texture_format( with_alpha ? GL_COMPRESSED_RGBA_ARB : GL_COMPRESSED_RGB_ARB );
        else
            allegro_gl_set_texture_format( with_alpha ? GL_RGBA8 : GL_RGB8 );
        allegro_gl_use_alpha_channel( with_alpha );
        GLuint gl_tex = make_texture( bmp, filter_type, clamp );
        allegro_gl_use_alpha_channel(false);
        destroy_bitmap(bmp);
        return gl_tex;
    }

    GLuint	GFX::load_texture_mask( String file, int level, byte filter_type, uint32 *width, uint32 *height, bool clamp )
    {
        if(!exists( file.c_str()))	// The file doesn't exist
            return 0;

        set_color_depth(32);
        BITMAP *bmp = load_bitmap( file.c_str(), NULL);
        if (bmp == NULL )	return 0;					// Operation failed
        if (width )		*width = bmp->w;
        if (height )	*height = bmp->h;
        if (bitmap_color_depth(bmp) != 32 )
        {
            BITMAP *tmp = create_bitmap_ex( 32, bmp->w, bmp->h );
            blit( bmp, tmp, 0, 0, 0, 0, bmp->w, bmp->h);
            destroy_bitmap(bmp);
            bmp=tmp;
        }
        bool with_alpha = (String::ToLower(get_extension(file.c_str())) == "tga");
        if (with_alpha)
        {
            with_alpha = false;
            for( int y = 0 ; y < bmp->h && !with_alpha ; y++ )
                for( int x = 0 ; x < bmp->w && !with_alpha ; x++ )
                    with_alpha |= bmp->line[y][(x<<2)+3] != 255;
        }
        else
        {
            for( int y = 0 ; y < bmp->h ; y++ )
                for( int x = 0 ; x < bmp->w ; x++ )
                    bmp->line[y][(x<<2)+3] = 255;
        }

        for( int y = 0 ; y < bmp->h ; y++ )
        {
            for( int x = 0 ; x < bmp->w ; x++ )
            {
                if (bmp->line[y][(x<<2)] < level && bmp->line[y][(x<<2)+1] < level
                    && bmp->line[y][(x<<2)+2] < level)
                {
                    bmp->line[y][(x<<2)+3] = 0;
                    with_alpha = true;
                }
            }
        }
        if(g_useTextureCompression)
            allegro_gl_set_texture_format( with_alpha ? GL_COMPRESSED_RGBA_ARB : GL_COMPRESSED_RGB_ARB );
        else
            allegro_gl_set_texture_format( with_alpha ? GL_RGBA8 : GL_RGB8 );
        allegro_gl_use_alpha_channel( with_alpha );
        GLuint gl_tex = make_texture( bmp, filter_type, clamp );
        allegro_gl_use_alpha_channel(false);
        destroy_bitmap(bmp);
        return gl_tex;
    }



    GLuint GFX::load_texture_from_cache( String file, byte filter_type, uint32 *width, uint32 *height, bool clamp )
    {
        if(ati_workaround || !lp_CONFIG->use_texture_cache)
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

            glPushAttrib( GL_ALL_ATTRIB_BITS );

            uint32 rw, rh;
            fread( &rw, 4, 1, cache_file );
            fread( &rh, 4, 1, cache_file );
            if(width)  *width = rw;
            if(height) *height = rh;

            byte *img = new byte[ rw * rh * 5 ];

            int lod_max = 0;
            GLint size, internal_format;

            fread( &lod_max, sizeof( lod_max ), 1, cache_file );
            fread( &internal_format, sizeof( GLint ), 1, cache_file );

            GLuint	tex = gfx->create_texture( 1, 1 );

            glBindTexture( GL_TEXTURE_2D, tex );

            for( int lod = 0 ; lod  < lod_max ; lod++ )
            {
                fread( &size, sizeof( GLint ), 1, cache_file );
                fread( &internal_format, sizeof( GLint ), 1, cache_file );
                fread( img, size, 1, cache_file );
                glCompressedTexImage2DARB( GL_TEXTURE_2D, lod, internal_format, rw, rh, 0, size, img);
            }

            delete img;

            fclose( cache_file );

            glMatrixMode(GL_TEXTURE);
            glLoadIdentity();
            glMatrixMode(GL_MODELVIEW);

            if (clamp )
            {
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
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
            glPopAttrib();
            return tex;
        }
        return 0; // File isn't in cache
    }


    void GFX::save_texture_to_cache( String file, GLuint tex, uint32 width, uint32 height )
    {
        if(ati_workaround || !lp_CONFIG->use_texture_cache)
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

        byte *img = new byte[ rw * rh * 5 ];

        float lod_max_f = Math::Max(log(rw), log(rh)) / log(2.0f);
        int lod_max = ((int) lod_max_f);
        if (lod_max > lod_max_f )
            lod_max++;

        fwrite( &lod_max, sizeof( lod_max ), 1, cache_file );

        GLint size, internal_format;
        glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format );
        fwrite( &internal_format, sizeof( GLint ), 1, cache_file );

        for( int lod = 0 ; lod  < lod_max ; lod++ )
        {
            glGetTexLevelParameteriv( GL_TEXTURE_2D, lod, GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB, &size );
            glGetTexLevelParameteriv( GL_TEXTURE_2D, lod, GL_TEXTURE_INTERNAL_FORMAT, &internal_format );
            glGetCompressedTexImageARB( GL_TEXTURE_2D, lod, img );
            fwrite( &size, sizeof( GLint ), 1, cache_file );
            fwrite( &internal_format, sizeof( GLint ), 1, cache_file );
            fwrite( img, size, 1, cache_file );
        }

        delete img;

        fclose( cache_file );
    }



    GLuint GFX::load_masked_texture(String file, String mask, byte filter_type )
    {
        if (!exists(file.c_str()) || !exists(mask.c_str()))
            return 0;		// The file doesn't exist

        set_color_depth(32);
        BITMAP* bmp = load_bitmap(file.c_str(), NULL);
        if (bmp == NULL )	return 0;					// Operation failed
        BITMAP *alpha;
        set_color_depth(8);
        alpha=load_bitmap( mask.c_str(), NULL );
        if(! alpha)
        {
            destroy_bitmap( alpha );
            return 0;
        }
        set_color_depth(32);
        for(int y = 0; y < bmp->h; ++y)
        {
            for(int x=0;x<bmp->w;x++)
                bmp->line[y][(x<<2)+3]=alpha->line[y][x];
        }
        allegro_gl_use_alpha_channel(true);
        if(g_useTextureCompression)
            allegro_gl_set_texture_format(GL_COMPRESSED_RGBA_ARB);
        else
            allegro_gl_set_texture_format(GL_RGBA8);
        GLuint gl_tex = make_texture( bmp, filter_type );
        allegro_gl_use_alpha_channel(false);
        destroy_bitmap(bmp);
        destroy_bitmap(alpha);
        return gl_tex;
    }


    uint32 GFX::texture_width(const GLuint& gltex)
    {
        GLint width;
        glBindTexture( GL_TEXTURE_2D, gltex);
        glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width );
        return width;
    }

    uint32 GFX::texture_height(const GLuint& gltex)
    {
        GLint height;
        glBindTexture( GL_TEXTURE_2D, gltex);
        glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height );
        return height;
    }


    void GFX::destroy_texture(GLuint& gltex)
    {
        if (gltex )						// Test if the texture exists
            glDeleteTextures(1,&gltex);
        gltex = 0;						// The texture is destroyed
    }

    GLuint GFX::make_texture_from_screen( byte filter_type)				// Copy pixel data from screen to a texture
    {
        BITMAP *tmp = create_bitmap_ex(32, SCREEN_W, SCREEN_H);
        GLuint gltex = make_texture(tmp, filter_type);
        destroy_bitmap(tmp);

        glBindTexture(GL_TEXTURE_2D,gltex);
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, SCREEN_W, SCREEN_H, 0);

        return gltex;
    }

    void GFX::set_alpha_blending()
    {
        glPushAttrib(GL_ENABLE_BIT);					// Push OpenGL attribs to pop them later when we unset alpha blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        alpha_blending_set = true;
    }

    void GFX::unset_alpha_blending()
    {
        if (alpha_blending_set)
            glPopAttrib();								// Pop OpenGL attribs to pop them later when we unset alpha blending
        else
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
            for(uint32 i = 0; i < 8; ++i)
            {
                glActiveTextureARB(GL_TEXTURE0_ARB + i);
                ReInitTexSys();
                if (disable )
                    glDisable(GL_TEXTURE_2D);
            }
            glActiveTextureARB(GL_TEXTURE0_ARB);
	}
    }

    void GFX::SetDefState()
    {
        glClearColor (0, 0, 0, 0);
        glShadeModel (GL_SMOOTH);
        glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
        glPolygonMode (GL_BACK, GL_POINTS);
        glDepthFunc( GL_LESS );
        glEnable (GL_DEPTH_TEST);
        glCullFace (GL_BACK);
        glEnable (GL_CULL_FACE);
        //	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
        glHint(GL_FOG_HINT, GL_FASTEST);
        glHint(GL_LINE_SMOOTH_HINT, GL_FASTEST);
        glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
        glHint(GL_POINT_SMOOTH_HINT, GL_FASTEST);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_FASTEST);
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
        if (tex == 0)       // Release the texture
        {
            glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,0);     // Bind the default FBO
            glViewport(0, 0, SCREEN_W, SCREEN_H);           // Use default viewport
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
        }
    }

} // namespace Interfaces
} // namespace TA3D
