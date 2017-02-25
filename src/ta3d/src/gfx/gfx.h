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



# include <stdafx.h>
# include <misc/string.h>
# include "gfx.toolkit.h"
# include "font.h"
# include "texture.h"
# include "shader.h"
# include <threads/thread.h>
# include <misc/interface.h>
# include <misc/hash_table.h>


# define FILTER_NONE			0x0
# define FILTER_LINEAR		    0x1
# define FILTER_BILINEAR		0x2
# define FILTER_TRILINEAR	    0x3


# define BYTE_TO_FLOAT  0.00390625f



namespace TA3D
{

	class Font;
	class Vector3D;


	class GFX : public ObjectSync, protected IInterface
	{
	public:
        typedef zuzuf::smartptr<GFX>	Ptr;
	public:
		//! \name 2D/3D Mode
		//@{
		//! Set the 2D Mode
		void set_2D_mode();
		//! UnSet the 2D mode
		void unset_2D_mode();
		//! Set the 2D clip rectangle
		void set_2D_clip_rectangle(int x = 0, int y = 0, int w = -1, int h = -1);
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
        ** \return A valid QImage
		*/
        static QImage LoadMaskedTextureToBmp(const QString& file, const QString& filealpha);

	public:
		//! \name Constructor & Destructor
		//@{
		//! Default Constructor
		GFX();
		//! Destructor
		virtual ~GFX();
		//@}

		//! Default configurator based on platform detection
		void detectDefaultSettings();


		//! Set current texture format
		void set_texture_format(GLuint gl_format);

		//! Set current texture format
		void use_mipmapping(bool use);

		void load_background();

		void destroy_background() { destroy_texture(glfond); }

		/*!
		** \brief Load the default textures (background + default.png)
		**
		** This method must be called from the main thread
		*/
		void loadDefaultTextures();

		/*!
		** \brief Load font from files
		*/
		void loadFonts();

		/*!
		** \brief Check that current settings don't try to use unavailable OpenGL extensions. Update config data if needed
		*/
		void checkConfig() const;

		/*!
		** \brief Get if we have to deal with some specific ATI workaround
		*/
		bool atiWorkaround() const {return ati_workaround;}

		//! \name Color management
		//@{
		void set_color(const float r, const float g, const float b) const
		{ glColor3f(r,g,b); }

		void set_color(const float r, const float g, const float b, const float a) const
		{ glColor4f(r,g,b,a); }

		void set_color(const uint32 col) const
		{ glColor4ub( (GLubyte)getr(col), (GLubyte)getg(col), (GLubyte)getb(col), (GLubyte)geta(col)); }

		void set_alpha(const float a) const;

		/*!
		** \brief
		*/
		float get_r(const uint32 col) const  {return (float)getr(col) * BYTE_TO_FLOAT;}
		/*!
		**
		*/
		float get_g(const uint32 col) const  {return (float)getg(col) * BYTE_TO_FLOAT; }
		/*!
		**
		*/
		float get_b(const uint32 col) const {return (float)getb(col) * BYTE_TO_FLOAT;}
		/*!
		**
		*/
		float get_a(const uint32 col) const {return (float)geta(col) * BYTE_TO_FLOAT;}

		inline void loadVertex(const Vector3D &v) const
		{	glVertex3fv((const GLfloat*)&v);	}

		/*!
		**
		*/
		uint32 makeintcol(float r, float g, float b) const
		{ return (uint32)(255.0f * r) | ((uint32)(255.0f * g) << 8) | ((uint32)(255.0f * b) << 16) | 0xFF000000; }
		/*!
		**
		*/
		uint32 makeintcol(float r, float g, float b, float a) const
		{ return (uint32)(255.0f * r) | ((uint32)(255.0f * g) << 8) | ((uint32)(255.0f * b) << 16) | ((uint32)(255.0f * a) << 24); }

		//@} // Color management

		/*!
		** \brief enable/disable the 3DO model vertex/fragment program (the one that fits the current rendering mode, or none if not required)
		*/
		void enable_model_shading(int mode = 0);
		void disable_model_shading();

		void setShadowMapMode(bool mode);
		bool getShadowMapMode();

		void line(const float x1, const float y1, const float x2, const float y2);			// Basic drawing routines
		void rect(const float x1, const float y1, const float x2, const float y2);
		void rectfill(const float x1, const float y1, const float x2, const float y2);
		void circle(const float x, const float y, const float r);
		void circlefill(const float x, const float y, const float r);
		void circle_zoned(const float x, const float y, const float r, const float mx, const float my, const float Mx, const float My);
		void dot_circle_zoned(const float t, const float x, const float y, const float r, const float mx, const float my, const float Mx, const float My);
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
		void dot_circle_zoned(const float t, const float x, const float y, const float r, const float mx, const float my, const float Mx, const float My, const uint32 col);
		void rectdot(const float x1, const float y1, const float x2, const float y2, const uint32 col);
		void drawtexture(const GLuint &tex, const float x1, const float y1, const float x2, const float y2, const uint32 col);
		void drawtexture_flip(const GLuint &tex, const float x1, const float y1, const float x2, const float y2, const uint32 col);

		//! \name Text manipulation
		//@{
        void print(Font *font, const float x, const float y, const float z, const QString &text);		// Font related routines
        void print(Font *font, const float x, const float y, const float z, const uint32 col, const QString &text);

        void print_center(Font *font, const float x, const float y, const float z, const QString &text);		// Font related routines
        void print_center(Font *font, const float x, const float y, const float z, const uint32 col, const QString &text);

        void print_right(Font *font, const float x, const float y, const float z, const QString &text);		// Font related routines
        void print_right(Font *font, const float x, const float y, const float z, const uint32 col, const QString &text);
		//@} // Text manipilation

        GLuint	make_texture( const QImage &bmp, int filter_type = FILTER_TRILINEAR, bool clamp = true);
		GLuint	create_color_texture(uint32 color);
		GLuint	create_texture( int w, int h, int filter_type = FILTER_TRILINEAR, bool clamp = true);
        void	blit_texture( const QImage &src, GLuint dst);
        GLuint	load_texture(const QString& file, int filter_type = FILTER_TRILINEAR, uint32 *width = NULL, uint32 *height = NULL, bool clamp = true, GLuint texFormat = 0, bool *useAlpha = NULL, bool checkSize = false);
        GLuint	load_texture_mask(const QString& file, uint32 level, int filter_type = FILTER_TRILINEAR, uint32 *width = NULL, uint32 *height = NULL, bool clamp = true);
        GLuint	load_texture_from_cache(const QString& file, int filter_type = FILTER_TRILINEAR, uint32 *width = NULL, uint32 *height = NULL, bool clamp = true, bool *useAlpha = NULL);
        GLuint	load_masked_texture( const QString &file, QString mask, int filter_type = FILTER_TRILINEAR);
        void	save_texture_to_cache(QString file, GLuint tex, uint32 width, uint32 height, bool useAlpha);
		uint32	texture_width(const GLuint gltex);
		uint32	texture_height(const GLuint gltex);
		void	destroy_texture( GLuint &gltex);
		void	disable_texturing();
		void	enable_texturing();
        bool    is_texture_in_cache(const QString& file);
		int     max_texture_size();

		GLuint  make_texture_RGB32F( int w, int h, float *data, int filter_type = FILTER_NONE, bool clamp = false);
		GLuint  make_texture_RGBA32F( int w, int h, float *data, int filter_type = FILTER_NONE, bool clamp = false);
		GLuint  make_texture_RGB16F( int w, int h, float *data, int filter_type = FILTER_NONE, bool clamp = false);
		GLuint  make_texture_RGBA16F( int w, int h, float *data, int filter_type = FILTER_NONE, bool clamp = false);
		GLuint  make_texture_A16F( int w, int h, float *data, int filter_type = FILTER_NONE, bool clamp = false);
		GLuint  make_texture_A32F( int w, int h, float *data, int filter_type = FILTER_NONE, bool clamp = false);

		GLuint	create_texture_RGB32F( int w, int h, int filter_type = FILTER_NONE, bool clamp = true);
		GLuint	create_texture_RGBA32F( int w, int h, int filter_type = FILTER_NONE, bool clamp = true);
		GLuint	create_texture_RGB16F( int w, int h, int filter_type = FILTER_NONE, bool clamp = true);
		GLuint	create_texture_RGBA16F( int w, int h, int filter_type = FILTER_NONE, bool clamp = true);
		GLuint  create_shadow_map(int w, int h);

		GLuint make_texture_from_screen(int filter_type = FILTER_NONE);

		GLuint get_shadow_map();
		void delete_shadow_map();
		void readShadowMapProjectionMatrix();

        QImage load_image(const QString &filename);

		void set_alpha_blending();
		void unset_alpha_blending();

		void ReInitArrays();
		void ReInitTexSys( bool matrix_reset = true);
		void ReInitAllTex( bool disable = false);
		void SetDefState();

		//! \brief clearScreen() && clearDepth()
		void clearAll();
		//! \brief clear the color buffer
		void clearScreen();
		//! \brief clear the depth buffer
		void clearDepth();

		/*!
		** \brief Flip the backbuffer to the screen
		*/
		void flip() const { SDL_ShowCursor(SDL_DISABLE);    SDL_GL_SwapBuffers(); }

		/*!
		** \brief set a texture as render target, goes back to normal when passing 0 (do not forget to detach the texture when you're done!)
		*/
		void renderToTexture(const GLuint tex = 0, bool useDepth = false);
		void renderToTextureDepth(const GLuint tex = 0);

		/*!
		** \brief runs several tests on GFX hardware capabilities, should be used only when calling ta3d with --test
		*/
		static void runTests();
		static void runOpenGLTests();

        QImage create_surface_ex(int bpp, int w, int h);
        QImage create_surface(int w, int h);

		/*!
		** \brief returns default texture formats for RGB and RGBA textures
		*/
		GLuint defaultTextureFormat_RGB() const	{	return defaultRGBTextureFormat;	}
		GLuint defaultTextureFormat_RGBA() const	{	return defaultRGBATextureFormat;	}
		GLuint defaultTextureFormat_RGB_compressed() const;
		GLuint defaultTextureFormat_RGBA_compressed() const;

		void enableShadowMapping() const;
		void disableShadowMapping() const;
		void storeShadowMappingState();
		void restoreShadowMappingState() const;

	public:
		int			width;				// Size of this window on the screen
		int			height;
		int			x,y;				// Position on the screen
		Font        *normal_font;		// Fonts
		Font        *small_font;
		Font        *TA_font;
		Font        *ta3d_gui_font;
		Font        *big_font;

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
		GLuint      textureColor;       // Default color texture used by FBO when rendering to depth texture
		GLuint      shadowMap;

		GLfloat     shadowMapProjectionMatrix[16];

		Shader      model_shader;

		bool		ati_workaround;		// Need to use workarounds for ATI cards ?

		int         max_tex_size;
		//! A default texture, loaded at initialization, used for rendering non textured objects with some shaders
		GLuint      default_texture;

		//! A bool to store shadowMap texture ID state
		bool		shadowMapWasActive;

	private:
        virtual uint32 InterfaceMsg(const uint32 MsgID, const QString &msg);

		void preCalculations();
		void initSDL();

		/*!
		** \brief Check if we have to deal with some Video card Workaround
		*/
		bool checkVideoCardWorkaround() const;


	private:
		// One of our friend
		friend class Font;

		bool		alpha_blending_set;
		GLuint      texture_format;
		bool        build_mipmaps;
		bool        shadowMapMode;
		GLuint		defaultRGBTextureFormat;
		GLuint		defaultRGBATextureFormat;
		//! Store all texture IDs
		UTILS::HashMap<Interfaces::GfxTexture>::Sparse	textureIDs;
		//! And for each texture ID, how many times it is used
		UTILS::HashMap<int, GLuint>::Sparse				textureLoad;
		//! And for each texture ID, if there is alpga
		UTILS::HashSet<GLuint>::Sparse					textureAlpha;
		//! And for each texture ID the file which contains the original texture
        UTILS::HashMap<QString, GLuint>::Sparse			textureFile;
	}; // class GFX




	// Those function should be removed
	void reset_keyboard();
	void reset_mouse();

} // namespace TA3D


#endif // __TA3D_GFX_H__
