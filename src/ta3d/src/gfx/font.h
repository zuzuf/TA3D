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
#ifndef __TA3D_GFX_FONT_H__
# define __TA3D_GFX_FONT_H__

# include <stdafx.h>
# include "gfx.h"
# include <misc/hash_table.h>
# include <threads/thread.h>
# include <misc/string.h>
# include <deque>
# include <QSharedPointer>

# ifdef __FTGL__lower__
#	include <FTGL/ftgl.h>
# else
#	include <FTGL/ftgl.h>
#	include <FTGL/FTBBox.h>
#	include <FTGL/FTGlyph.h>
#	include <FTGL/FTBitmapGlyph.h>
#	include <FTGL/FTExtrdGlyph.h>
#	include <FTGL/FTOutlineGlyph.h>
#	include <FTGL/FTPixmapGlyph.h>
#	include <FTGL/FTPolyGlyph.h>
#	include <FTGL/FTTextureGlyph.h>
#	include <FTGL/FTFont.h>
#	include <FTGL/FTGLBitmapFont.h>
#	include <FTGL/FTGLExtrdFont.h>
#	include <FTGL/FTGLOutlineFont.h>
#	include <FTGL/FTGLPixmapFont.h>
#	include <FTGL/FTGLPolygonFont.h>
#	include <FTGL/FTGLTextureFont.h>

typedef FTGLTextureFont FTTextureFont;
typedef FTGLPolygonFont FTPolygonFont;
typedef FTGLBitmapFont  FTBitmapFont;
typedef FTGLPixmapFont  FTPixmapFont;

# endif


# define FONT_TYPE_POLYGON 			0x0
# define FONT_TYPE_TEXTURE 			0x1
# define FONT_TYPE_BITMAP 			0x2
# define FONT_TYPE_PIXMAP     		0x3

# define TA3D_FONT_PATH  			"fonts"



namespace TA3D
{
	class GFX;


	class Font : ObjectSync
	{
	public:
		//! Type of a font
		enum Type
		{
			typePolygon = 0x0,
			typeTexture = 0x1,
			typeBitmap  = 0x2,
			typePixmap  = 0x3,
			typeTextures = 0x4
		};

	public:
		//! \name Constructors & Destructor
		//@{
		//! Default Constructor
		Font();
		//! Copy constructor
		Font(const Font& rhs);
		//! Destructor
		virtual ~Font();
		//@}

		void init();
		void destroy();

		/*!
		** \brief Get the width of a text
		*/
        float length(const QString &txt);

		/*!
		** \brief Get the height the font
		*/
		float height();

		/*!
		** \brief Load a font from a file
		**
		** \param filename The filename of the Font
		** \param size Size of the font
		** \param type Type of the Font (Texture, Pixmap, Bitmap, Polygon)
		** \return True if the operator succeeded, False otherwise
		*/
        bool load( const QString &filename, const int size, const Type type);

		int get_size();
        void print(float x, float y, float z, const QString &text);

		void setBold(bool bBold)	{	this->bBold = bBold;	}

		//! \name Operators
		//@{
		//! Operator =
		Font& operator = (const Font& rhs);
		//@}

	private:
		/*!
		** \brief Load a font from a file (Without Lock)
		**
		** \param filename The filename of the Font
		** \param size Size of the font
		** \param type Type of the Font (Texture, Pixmap, Bitmap, Polygon)
		** \return True if the operator succeeded, False otherwise
		*/
        bool loadWL(const QString &filename, const int size, const Type type);

	private:
		//! The FT Font
        QSharedPointer<FTFont> font;
		//! The filename of the font
        QString pFontFilename;
		//! Type of the font
		Type pType;
		//! Bold style
		bool bBold;
		// Friend
		friend class GFX;

	}; // class GfxFont




	class FontManager
	{
	public:
		FontManager();
		~FontManager();

		void destroy();

        Font *find(const QString& filename, const int size, const Font::Type type);

	private:
        Font* internalRegisterFont(const QString& key, const QString& filename, const int size, const Font::Type type);

	private:
		//! Font list
		typedef std::deque<Font*>  FontList;

		FontList  pFontList;
		UTILS::HashMap<Font*>::Dense    font_table;
	}; // class FontManager



	//! Font manager
	extern FontManager font_manager;




} // namespace TA3D

#endif // __TA3D_GFX_FONT_H__
