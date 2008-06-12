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

#ifndef __TA3D_GFX_H__
# define __TA3D_GFX_H__

using namespace TA3D;

namespace TA3D
{
namespace INTERFACES
{


    class GfxFont
    {
    private:
        friend class GFX;
    private:
        FONT* pAl;
        FONT* pGl;
        float size;
        bool clear;

    public:

        GfxFont();

        void init();

        float length( const String txt) const
        { return text_length(pAl, txt.c_str()) * size; }
        
        float height() const
        { return text_height(pAl) * size; }

        void set_clear(const bool val) { clear = val; }
        void load( const char *filename, const float s = 1.0f );
        void load_gaf_font( const char *filename, const float s = 1.0f );
        void copy( FONT *fnt, const float s = 1.0f );
        void destroy();
        void change_size( const float s) { size = s;	}
        float get_size() const { return size; }

    }; // class GfxFont



#define FILTER_NONE			0x0
#define FILTER_LINEAR		0x1
#define FILTER_BILINEAR		0x2
#define FILTER_TRILINEAR	0x3

    class GFX_TEXTURE
    {
    public:
        uint32		width;
        uint32		height;
        GLuint		tex;
        bool		destroy_tex;

        inline void init()	{	width = height = tex = 0;	destroy_tex = false;	}
        GFX_TEXTURE()	{	init();	}
        GFX_TEXTURE( const GLuint gltex );
        void set( const GLuint gltex );
        void draw( const float x1, const float y1 );
        void draw( const float x1, const float y1, const uint32 col );

        void destroy();
    };

    class GFX : 	protected cCriticalSection,
    protected cInterface
    {
        friend class GfxFont;

        bool		alpha_blending_set;

    public:
        int			width;				// Size of this window on the screen
        int			height;
        int			x,y;				// Position on the screen
        GfxFont	normal_font;		// Fonts
        GfxFont	small_font;
        GfxFont	TA_font;
        GfxFont	ta3d_gui_font;

        sint32		SCREEN_W_HALF;
        sint32		SCREEN_H_HALF;
        float		SCREEN_W_INV;
        float		SCREEN_H_INV;
        float		SCREEN_W_TO_640;				// To have mouse sensibility undependent from the resolution
        float		SCREEN_H_TO_480;

        float		low_def_limit;

        GLuint		glfond;

        bool		ati_workaround;		// Need to use workarounds for ATI cards ?

        GFX();

        inline void precalculations()
        {
            SCREEN_W_HALF = SCREEN_W>>1;
            SCREEN_H_HALF = SCREEN_H>>1;
            SCREEN_W_INV = 1.0f / SCREEN_W;
            SCREEN_H_INV = 1.0f / SCREEN_H;
            SCREEN_W_TO_640 = 640.0f / SCREEN_W;
            SCREEN_H_TO_480 = 480.0f / SCREEN_H;
        }

        virtual ~GFX();

        inline void GFX_EnterCS()	{	EnterCS();	}
        inline void GFX_LeaveCS()	{	LeaveCS();	}

        inline void load_background()
        {
            if(SCREEN_W<=800)
                glfond = load_texture( "gfx/menu800.jpg", FILTER_LINEAR );
            else if(SCREEN_W<=1024)
                glfond = load_texture( "gfx/menu1024.jpg", FILTER_LINEAR );
            else
                glfond = load_texture( "gfx/menu1280.jpg", FILTER_LINEAR );
        }

        inline void destroy_background()	{	destroy_texture(glfond);	}

        void Init();

    private:
        uint32 InterfaceMsg( const lpcImsg msg );

    public:

        void set_color(const float r, const float g, const float b) const
        { glColor3f(r,g,b); }

        void set_color(const float r, const float g, const float b, const float a) const
        { glColor4f(r,g,b,a); }

        void set_color(const uint32 col) const
        { glColor4ub(col&0xFF, (col&0xFF00) >> 8, (col&0xFF0000) >> 16, (col&0xFF000000) >> 24); }

        void set_alpha(const float &a);
        const float	get_r(const uint32 &col);
        const float	get_g(const uint32 &col);
        const float	get_b(const uint32 &col);
        const float	get_a(const uint32 &col);
        uint32	makeintcol(float r, float g, float b);
        uint32	makeintcol(float r, float g, float b, float a);

        void line(const float &x1, const float &y1, const float &x2, const float &y2);			// Basic drawing routines
        void rect(const float &x1, const float &y1, const float &x2, const float &y2);
        void rectfill(const float &x1, const float &y1, const float &x2, const float &y2);
        void circle(const float &x, const float &y, const float &r);
        void circlefill(const float &x, const float &y, const float &r);
        void circle_zoned(const float &x, const float &y, const float &r, const float &mx, const float &my, const float &Mx, const float &My);
        void rectdot(const float &x1, const float &y1, const float &x2, const float &y2);
        void drawtexture(const GLuint &tex, const float &x1, const float &y1, const float &x2, const float &y2);
        void drawtexture_flip(const GLuint &tex, const float &x1, const float &y1, const float &x2, const float &y2);
        void drawtexture(const GLuint &tex, const float &x1, const float &y1, const float &x2, const float &y2, const float &u1, const float &v1, const float &u2, const float &v2);

        void line(const float &x1, const float &y1, const float &x2, const float &y2, const uint32 &col);			// Basic drawing routines (with color arguments)
        void rect(const float &x1, const float &y1, const float &x2, const float &y2, const uint32 &col);
        void rectfill(const float &x1, const float &y1, const float &x2, const float &y2, const uint32 &col);
        void circle(const float &x, const float &y, const float &r, const uint32 &col);
        void circlefill(const float &x, const float &y, const float &r, const uint32 &col);
        void circle_zoned(const float &x, const float &y, const float &r, const float &mx, const float &my, const float &Mx, const float &My, const uint32 &col);
        void rectdot(const float &x1, const float &y1, const float &x2, const float &y2, const uint32 &col);
        void drawtexture(const GLuint &tex, const float &x1, const float &y1, const float &x2, const float &y2, const uint32 &col);
        void putpixel(const float &x, const float &y, const uint32 &col);
        uint32     getpixel(const sint32 &x, const sint32 &y);

        void set_2D_mode();
        void unset_2D_mode();

        void print(const GfxFont &font, const float &x, const float &y, const float &z, const String text );		// Font related routines
        void print(const GfxFont &font, const float &x, const float &y, const float &z, const uint32 &col, const String text );

        void print(const GfxFont &font, const float &x, const float &y, const float &z, const char *text );
        void print(const GfxFont &font, const float &x, const float &y, const float &z, const uint32 &col, const char *text );

        void print(const GfxFont &font, const float &x, const float &y, const float &z, const String text, float s);		// Font related routines
        void print(const GfxFont &font, const float &x, const float &y, const float &z, const uint32 &col, const String text, float s);

        void print(const GfxFont &font, const float &x, const float &y, const float &z, const char *text, float s);
        void print(const GfxFont &font, const float &x, const float &y, const float &z, const uint32 &col, const char *text, float s);

        void print_center(const GfxFont &font, const float &x, const float &y, const float &z, const String text );		// Font related routines
        void print_center(const GfxFont &font, const float &x, const float &y, const float &z, const uint32 &col, const String text );

        void print_center(const GfxFont &font, const float &x, const float &y, const float &z, const char *text );
        void print_center(const GfxFont &font, const float &x, const float &y, const float &z, const uint32 &col, const char *text );

        void print_center(const GfxFont &font, const float &x, const float &y, const float &z, const String text, float s);		// Font related routines
        void print_center(const GfxFont &font, const float &x, const float &y, const float &z, const uint32 &col, const String text, float s);

        void print_center(const GfxFont &font, const float &x, const float &y, const float &z, const char *text, float s);
        void print_center(const GfxFont &font, const float &x, const float &y, const float &z, const uint32 &col, const char *text, float s);

        void print_right(const GfxFont &font, const float &x, const float &y, const float &z, const String text );		// Font related routines
        void print_right(const GfxFont &font, const float &x, const float &y, const float &z, const uint32 &col, const String text );

        void print_right(const GfxFont &font, const float &x, const float &y, const float &z, const char *text );
        void print_right(const GfxFont &font, const float &x, const float &y, const float &z, const uint32 &col, const char *text );

        void print_right(const GfxFont &font, const float &x, const float &y, const float &z, const String text, float s);		// Font related routines
        void print_right(const GfxFont &font, const float &x, const float &y, const float &z, const uint32 &col, const String text, float s);

        void print_right(const GfxFont &font, const float &x, const float &y, const float &z, const char *text, float s);
        void print_right(const GfxFont &font, const float &x, const float &y, const float &z, const uint32 &col, const char *text, float s);

        GLuint	make_texture( BITMAP *bmp, byte filter_type = FILTER_TRILINEAR, bool clamp = true );
        GLuint	create_texture( int w, int h, byte filter_type = FILTER_TRILINEAR, bool clamp = true );
        void	blit_texture( BITMAP *src, GLuint &dst );
        GLuint	load_texture( String file, byte filter_type = FILTER_TRILINEAR, uint32 *width = NULL, uint32 *height = NULL, bool clamp = true );
        GLuint	load_texture_mask( String file, int level, byte filter_type = FILTER_TRILINEAR, uint32 *width = NULL, uint32 *height = NULL, bool clamp = true );
        GLuint	load_texture_from_cache( String file, byte filter_type = FILTER_TRILINEAR, uint32 *width = NULL, uint32 *height = NULL, bool clamp = true );
        GLuint	load_masked_texture( String file, String mask, byte filter_type = FILTER_TRILINEAR );
        void	save_texture_to_cache( String file, GLuint tex, uint32 width, uint32 height );
        uint32	texture_width( const GLuint &gltex );
        uint32	texture_height( const GLuint &gltex );
        void	destroy_texture( GLuint &gltex );
        void	disable_texturing();
        void	enable_texturing();

        GLuint	make_texture_from_screen( byte filter_type = FILTER_NONE );

        void flip() const { allegro_gl_flip(); }

        void set_alpha_blending();
        void unset_alpha_blending();

        void ReInitArrays();
        void ReInitTexSys( bool matrix_reset = true );
        void ReInitAllTex( bool disable = false);
        void SetDefState();
    };
}
}


#endif // __TA3D_GFX_H__
