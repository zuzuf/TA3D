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
# include <QWindow>
# include <QOpenGLFunctions>
# include <QOpenGLContext>
# include "shader.h"
# include <QMatrix4x4>

# define FILTER_NONE			0x0
# define FILTER_LINEAR		    0x1
# define FILTER_BILINEAR		0x2
# define FILTER_TRILINEAR	    0x3


# define BYTE_TO_FLOAT  0.00390625f



namespace TA3D
{

	class Font;
	class Vector3D;
    class Vector2D;


    class GFX : public QWindow, public QOpenGLFunctions, public ObjectSync, protected IInterface
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

        QMatrix4x4 get2Dmatrix();

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

        void destroy_background();

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

        void set_alpha(const float a);

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
        { return qRgb(255.0f * r, 255.0f * g, 255.0f * b);  }
		/*!
		**
		*/
		uint32 makeintcol(float r, float g, float b, float a) const
        { return qRgba(255.0f * r, 255.0f * g, 255.0f * b, 255.0f * a); }

		//@} // Color management

		/*!
		** \brief enable/disable the 3DO model vertex/fragment program (the one that fits the current rendering mode, or none if not required)
		*/
		void enable_model_shading(int mode = 0);
		void disable_model_shading();

		void setShadowMapMode(bool mode);
		bool getShadowMapMode();

        void loadShaders();

		void circlefill(const float x, const float y, const float r);

        void line_loop(const Vector2D *pts, const size_t nb_elts, const uint32 col);			// Basic drawing routines (with color arguments)
        void lines(const Vector2D *pts, const size_t nb_elts, const uint32 col);			// Basic drawing routines (with color arguments)

		void line(const float x1, const float y1, const float x2, const float y2, const uint32 col);			// Basic drawing routines (with color arguments)
		void rect(const float x1, const float y1, const float x2, const float y2, const uint32 col);
		void rectfill(const float x1, const float y1, const float x2, const float y2, const uint32 col);
		void circle(const float x, const float y, const float r, const uint32 col);
		void circlefill(const float x, const float y, const float r, const uint32 col);
		void circle_zoned(const float x, const float y, const float r, const float mx, const float my, const float Mx, const float My, const uint32 col);
		void dot_circle_zoned(const float t, const float x, const float y, const float r, const float mx, const float my, const float Mx, const float My, const uint32 col);
		void rectdot(const float x1, const float y1, const float x2, const float y2, const uint32 col);
        void drawtexture(const GfxTexture::Ptr &tex, const float x, const float y, const uint32 col);
        void drawtexture(const GfxTexture::Ptr &tex, const float x1, const float y1, const float x2, const float y2, const uint32 col);
        void drawtexture(const GfxTexture::Ptr &tex, const float x1, const float y1, const float x2, const float y2, const float u1, const float v1, const float u2, const float v2, const uint32 col);

        GfxTexture::Ptr make_texture( const QImage &bmp, int filter_type = FILTER_TRILINEAR, bool clamp = true);
        GfxTexture::Ptr create_color_texture(uint32 color);
        GfxTexture::Ptr create_texture( int w, int h, int filter_type = FILTER_TRILINEAR, bool clamp = true);
        GfxTexture::Ptr load_texture(const QString& file, int filter_type = FILTER_TRILINEAR, uint32 *width = NULL, uint32 *height = NULL, bool clamp = true, GLuint texFormat = 0, bool *useAlpha = NULL, bool checkSize = false);
        GfxTexture::Ptr load_texture_mask(const QString& file, uint32 level, int filter_type = FILTER_TRILINEAR, uint32 *width = NULL, uint32 *height = NULL, bool clamp = true);
        GfxTexture::Ptr load_texture_from_cache(const QString& file, int filter_type = FILTER_TRILINEAR, uint32 *width = NULL, uint32 *height = NULL, bool clamp = true, bool *useAlpha = NULL);
        GfxTexture::Ptr load_masked_texture( const QString &file, QString mask, int filter_type = FILTER_TRILINEAR);
        void	save_texture_to_cache(QString file, const GfxTexture::Ptr &tex, uint32 width, uint32 height, bool useAlpha);
		void	disable_texturing();
		void	enable_texturing();
        bool    is_texture_in_cache(const QString& file);
		int     max_texture_size();

        GfxTexture::Ptr make_texture_RGBA32F( int w, int h, float *data, int filter_type = FILTER_NONE, bool clamp = false);
        GfxTexture::Ptr make_texture_RGB16F( int w, int h, float *data, int filter_type = FILTER_NONE, bool clamp = false);
        GfxTexture::Ptr make_texture_RGBA16F( int w, int h, float *data, int filter_type = FILTER_NONE, bool clamp = false);
        GfxTexture::Ptr make_texture_A16F( int w, int h, float *data, int filter_type = FILTER_NONE, bool clamp = false);

        GfxTexture::Ptr create_texture_RGBA32F( int w, int h, int filter_type = FILTER_NONE, bool clamp = true);
        GfxTexture::Ptr create_texture_RGB16F( int w, int h, int filter_type = FILTER_NONE, bool clamp = true);
        GfxTexture::Ptr create_texture_RGBA16F( int w, int h, int filter_type = FILTER_NONE, bool clamp = true);
        GfxTexture::Ptr create_shadow_map(int w, int h);

        GfxTexture::Ptr get_shadow_map();
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
        void flip();

		/*!
		** \brief set a texture as render target, goes back to normal when passing 0 (do not forget to detach the texture when you're done!)
		*/
        void renderToTexture(const GfxTexture::Ptr &tex = GfxTexture::Ptr(), bool useDepth = false);
        void renderToTextureDepth(const GfxTexture::Ptr &tex = GfxTexture::Ptr());

        QImage create_surface_ex(int bpp, int w, int h);
        QImage create_surface(int w, int h);

		/*!
		** \brief returns default texture formats for RGB and RGBA textures
		*/
		GLuint defaultTextureFormat_RGB() const	{	return defaultRGBTextureFormat;	}
		GLuint defaultTextureFormat_RGBA() const	{	return defaultRGBATextureFormat;	}
		GLuint defaultTextureFormat_RGB_compressed() const;
		GLuint defaultTextureFormat_RGBA_compressed() const;

        void enableShadowMapping();
        void disableShadowMapping();
		void storeShadowMappingState();
        void restoreShadowMappingState();

    protected:
        virtual void keyPressEvent(QKeyEvent *e);
        virtual void keyReleaseEvent(QKeyEvent *e);
        virtual void mousePressEvent(QMouseEvent *e);
        virtual void mouseReleaseEvent(QMouseEvent *e);
        virtual void wheelEvent(QWheelEvent *e);
        virtual void mouseMoveEvent(QMouseEvent *e);

	public:
		int			width;				// Size of this window on the screen
		int			height;
		int			x,y;				// Position on the screen
        Font::Ptr   normal_font;		// Fonts
        Font::Ptr   small_font;
        Font::Ptr   TA_font;
        Font::Ptr   ta3d_gui_font;
        Font::Ptr   big_font;

		sint32		SCREEN_W_HALF;
		sint32		SCREEN_H_HALF;
		float		SCREEN_W_INV;
		float		SCREEN_H_INV;
		float		SCREEN_W_TO_640;				// To have mouse sensibility undependent from the resolution
		float		SCREEN_H_TO_480;

		float		low_def_limit;

        GfxTexture::Ptr glbackground;
		GLuint      textureFBO;         // FBO used by renderToTexture functions
        GfxTexture::Ptr textureDepth;
        GfxTexture::Ptr textureColor;       // Default color texture used by FBO when rendering to depth texture
        GfxTexture::Ptr shadowMap;

		GLfloat     shadowMapProjectionMatrix[16];

        Shader::Ptr model_shader;
        Shader::Ptr drawing2d_color_shader;
        Shader::Ptr drawing2d_texture_shader;
        Shader::Ptr particle_shader;

		bool		ati_workaround;		// Need to use workarounds for ATI cards ?

		int         max_tex_size;
		//! A default texture, loaded at initialization, used for rendering non textured objects with some shaders
        GfxTexture::Ptr default_texture;

		//! A bool to store shadowMap texture ID state
		bool		shadowMapWasActive;

	private:
        virtual uint32 InterfaceMsg(const uint32 MsgID, const QString &msg);

		void preCalculations();
        void initialize();

		/*!
		** \brief Check if we have to deal with some Video card Workaround
		*/
        bool checkVideoCardWorkaround();


	private:
		// One of our friend
		friend class Font;

        QOpenGLContext *m_context;

		bool		alpha_blending_set;
		GLuint      texture_format;
		bool        build_mipmaps;
		bool        shadowMapMode;
		GLuint		defaultRGBTextureFormat;
		GLuint		defaultRGBATextureFormat;
		//! Store all texture IDs
        UTILS::HashMap<GfxTexture::Ptr>::Sparse	textureIDs;
	}; // class GFX




	// Those function should be removed
	void reset_keyboard();
	void reset_mouse();

    namespace VARS
    {
        extern TA3D::GFX::Ptr   gfx;
    }
} // namespace TA3D

#endif // __TA3D_GFX_H__
