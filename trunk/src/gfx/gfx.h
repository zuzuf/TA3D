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


using namespace TA3D; // TODO Remove this

# include "font.h"
# include "texture.h"
# include "../threads/thread.h"
# include "../misc/interface.h"


# define FILTER_NONE			0x0
# define FILTER_LINEAR		    0x1
# define FILTER_BILINEAR		0x2
# define FILTER_TRILINEAR	    0x3


# define BYTE_TO_FLOAT  0.00390625f

namespace TA3D
{

    class GfxFont;

    
    class GFX : public ObjectSync, protected IInterface
    {
        friend class GfxFont;

        bool		alpha_blending_set;

    public:
        //! \name 2D/3D Mode
        //@{
        //! Set the 2D Mode
        static void set_2D_mode();
        //! UnSet the 2D mode
        static void unset_2D_mode();
        //@}


        /*!
        ** \brief Draw a texture inside a quad surface
        **
        ** \param texture The texture to draw
        ** \param x1 The top-left corned X-coordinate
        ** \param y1 The top-left corned Y-coordinate
        ** \param x2 The bottom-right corned X-coordinate
        ** \param x2 The bottom-right corned Y-coordinate
        */
        static void PutTextureInsideRect(const GLuint texture, const float x1, const float y1, const float x2, const float y2);

        /*!
        ** \brief Load a texture with a mask
        **
        ** \param file The texture file
        ** \param filealpha The mask
        ** \return A valid BITMAP
        */
        static BITMAP* LoadMaskedTextureToBmp(const String& file, const String& filealpha);

    public:
        int			width;				// Size of this window on the screen
        int			height;
        int			x,y;				// Position on the screen
        GfxFont     normal_font;		// Fonts
        GfxFont     small_font;
        GfxFont     TA_font;
        GfxFont     ta3d_gui_font;

        sint32		SCREEN_W_HALF;
        sint32		SCREEN_H_HALF;
        float		SCREEN_W_INV;
        float		SCREEN_H_INV;
        float		SCREEN_W_TO_640;				// To have mouse sensibility undependent from the resolution
        float		SCREEN_H_TO_480;

        float		low_def_limit;

        GLuint		glfond;
        GLuint      textureFBO;         // FBO used by renderToTexture functions
        GLuint      textureDepth;

        bool		ati_workaround;		// Need to use workarounds for ATI cards ?

        GFX();


        virtual ~GFX();

        void load_background();

        void destroy_background() { destroy_texture(glfond); }

        void Init();

    private:
        uint32 InterfaceMsg( const lpcImsg msg );
        
        void preCalculations();
        void initAllegroGL();
        bool checkVideoCardWorkaround() const;
        void displayInfosAboutOpenGL() const;

    public:
        //! \name Color management
        //@{

        void set_color(const float r, const float g, const float b) const
        { glColor3f(r,g,b); }

        void set_color(const float r, const float g, const float b, const float a) const
        { glColor4f(r,g,b,a); }

        void set_color(const uint32 col) const
        { glColor4ub(col & 0xFF, (col & 0xFF00) >> 8, (col & 0xFF0000) >> 16, (col & 0xFF000000) >> 24); }

        void set_alpha(const float a) const;

        /*!
        ** \brief
        */
        float get_r(const uint32 col) const
        { return ( col & 0xFF)*BYTE_TO_FLOAT; }
        /*!
        **
        */
        float get_g(const uint32 col) const
        { return ((col & 0xFF00) >> 8) * BYTE_TO_FLOAT; }
        /*!
        **
        */
        float get_b(const uint32 col) const
        { return ((col & 0xFF0000) >> 16) * BYTE_TO_FLOAT; }
        /*!
        **
        */
        float get_a(const uint32 col) const
        { return ((col & 0xFF000000) >> 24) * BYTE_TO_FLOAT; }


        /*!
        **
        */
        uint32 makeintcol(float r, float g, float b) const
        { return (int)(255.0f * r) | ((int)(255.0f * g) << 8) | ((int)(255.0f * b) << 16) | 0xFF000000; }
        /*!
        **
        */
        uint32 makeintcol(float r, float g, float b, float a) const
        { return (int)(255.0f * r) | ((int)(255.0f * g) << 8) | ((int)(255.0f * b) << 16) | ((int)(255.0f * a) << 24); }

        //@} // Color management


        void line(const float x1, const float y1, const float x2, const float y2);			// Basic drawing routines
        void rect(const float x1, const float y1, const float x2, const float y2);
        void rectfill(const float x1, const float y1, const float x2, const float y2);
        void circle(const float x, const float y, const float r);
        void circlefill(const float x, const float y, const float r);
        void circle_zoned(const float x, const float y, const float r, const float mx, const float my, const float Mx, const float My);
        void rectdot(const float x1, const float y1, const float x2, const float y2);
        void drawtexture(const GLuint &tex, const float x1, const float y1, const float x2, const float y2);
        void drawtexture_flip(const GLuint &tex, const float x1, const float y1, const float x2, const float y2);
        void drawtexture(const GLuint &tex, const float x1, const float y1, const float x2, const float y2, const float u1, const float v1, const float u2, const float v2);

        void line(const float x1, const float y1, const float x2, const float y2, const uint32 col);			// Basic drawing routines (with color arguments)
        void rect(const float x1, const float y1, const float x2, const float y2, const uint32 col);
        void rectfill(const float x1, const float y1, const float x2, const float y2, const uint32 col);
        void circle(const float x, const float y, const float r, const uint32 col);
        void circlefill(const float x, const float y, const float r, const uint32 col);
        void circle_zoned(const float x, const float y, const float r, const float mx, const float my, const float Mx, const float My, const uint32 col);
        void rectdot(const float x1, const float y1, const float x2, const float y2, const uint32 col);
        void drawtexture(const GLuint &tex, const float x1, const float y1, const float x2, const float y2, const uint32 col);
        void putpixel(const float x, const float y, const uint32 col);
        uint32 getpixel(const sint32 x, const sint32 y) const;

        //! \name Text manipulation
        //@{

        void print(const GfxFont &font, const float x, const float y, const float z, const String text );		// Font related routines
        void print(const GfxFont &font, const float x, const float y, const float z, const uint32 col, const String text );

        void print(const GfxFont &font, const float x, const float y, const float z, const char *text );
        void print(const GfxFont &font, const float x, const float y, const float z, const uint32 col, const char *text );

        void print(const GfxFont &font, const float x, const float y, const float z, const String text, float s);		// Font related routines
        void print(const GfxFont &font, const float x, const float y, const float z, const uint32 col, const String text, float s);

        void print(const GfxFont &font, const float x, const float y, const float z, const char *text, float s);
        void print(const GfxFont &font, const float x, const float y, const float z, const uint32 col, const char *text, float s);

        void print_center(const GfxFont &font, const float x, const float y, const float z, const String text );		// Font related routines
        void print_center(const GfxFont &font, const float x, const float y, const float z, const uint32 col, const String text );

        void print_center(const GfxFont &font, const float x, const float y, const float z, const char *text );
        void print_center(const GfxFont &font, const float x, const float y, const float z, const uint32 col, const char *text );

        void print_center(const GfxFont &font, const float x, const float y, const float z, const String text, float s);		// Font related routines
        void print_center(const GfxFont &font, const float x, const float y, const float z, const uint32 col, const String text, float s);

        void print_center(const GfxFont &font, const float x, const float y, const float z, const char *text, float s);
        void print_center(const GfxFont &font, const float x, const float y, const float z, const uint32 col, const char *text, float s);

        void print_right(const GfxFont &font, const float x, const float y, const float z, const String text );		// Font related routines
        void print_right(const GfxFont &font, const float x, const float y, const float z, const uint32 col, const String text );

        void print_right(const GfxFont &font, const float x, const float y, const float z, const char *text );
        void print_right(const GfxFont &font, const float x, const float y, const float z, const uint32 col, const char *text );

        void print_right(const GfxFont &font, const float x, const float y, const float z, const String text, float s);		// Font related routines
        void print_right(const GfxFont &font, const float x, const float y, const float z, const uint32 col, const String text, float s);

        void print_right(const GfxFont &font, const float x, const float y, const float z, const char *text, float s);
        void print_right(const GfxFont &font, const float x, const float y, const float z, const uint32 col, const char *text, float s);

        //@} // Text manipilation


        GLuint	make_texture( BITMAP *bmp, byte filter_type = FILTER_TRILINEAR, bool clamp = true );
        GLuint	create_texture( int w, int h, byte filter_type = FILTER_TRILINEAR, bool clamp = true );
        void	blit_texture( BITMAP *src, GLuint &dst );
        GLuint	load_texture( String file, byte filter_type = FILTER_TRILINEAR, uint32 *width = NULL, uint32 *height = NULL, bool clamp = true, GLuint texFormat = 0 );
        GLuint	load_texture_mask( String file, int level, byte filter_type = FILTER_TRILINEAR, uint32 *width = NULL, uint32 *height = NULL, bool clamp = true );
        GLuint	load_texture_from_cache( String file, byte filter_type = FILTER_TRILINEAR, uint32 *width = NULL, uint32 *height = NULL, bool clamp = true );
        GLuint	load_masked_texture( String file, String mask, byte filter_type = FILTER_TRILINEAR );
        void	save_texture_to_cache( String file, GLuint tex, uint32 width, uint32 height );
        uint32	texture_width( const GLuint &gltex );
        uint32	texture_height( const GLuint &gltex );
        void	destroy_texture( GLuint &gltex );
        void	disable_texturing();
        void	enable_texturing();

        GLuint make_texture_from_screen(byte filter_type = FILTER_NONE);


        void set_alpha_blending();
        void unset_alpha_blending();

        void ReInitArrays();
        void ReInitTexSys( bool matrix_reset = true );
        void ReInitAllTex( bool disable = false);
        void SetDefState();
        
        /*!
        ** \brief Flip the backbuffer to the Allegro screen and clear it
        */
        void flip() const { allegro_gl_flip(); }

        /*!
        ** \brief set a texture as render target, goes back to normal when passing 0 (do not forget to detach the texture when you're done!)
        */
        void renderToTexture( const GLuint tex = 0, bool useDepth = false );

        /*!
        ** \brief runs several tests on GFX hardware capabilities, should be used only when calling ta3d with --test
        */
        static void runTests();

    }; // class GFX




    // Those function should be removed
    void reset_keyboard();
    void reset_mouse();

} // namespace TA3D


#endif // __TA3D_GFX_H__
