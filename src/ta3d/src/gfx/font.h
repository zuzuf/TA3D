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

# include "gfx.h"
# include "../misc/hash_table.h"
# include "../threads/thread.h"

# ifdef __FTGL__lower__
#	include <FTGL/ftgl.h>
# else
#	include <FTGL/FTPoint.h>
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

# define LINUX_FONT_PATH    		"/usr/share/fonts"
# define SYSTEM_FONT_PATH 			LINUX_FONT_PATH
# define TA3D_FONT_PATH  			"fonts"



namespace TA3D
{
	class GFX;


	class Font : ObjectSync
	{
	public:
		Font();

		void init();

		float length(const String &txt);
		float height();
		void load( const String &filename, const int size, const int type);
		void destroy();
		int get_size();
		void print(float x, float y, float z, const String &text);

	private:
		friend class GFX;

		FTFont *font;
	}; // class GfxFont




	class FontManager
	{
	public:
		FontManager();
		~FontManager();

		void destroy();

		Font *getFont(String filename, int size, int type);

	private:
		std::list<Font*>            font_list;
		UTILS::cHashTable<Font*>    font_table;
	}; // class FontManager



	//! Font manager
	extern FontManager font_manager;




} // namespace TA3D

#endif // __TA3D_GFX_FONT_H__
