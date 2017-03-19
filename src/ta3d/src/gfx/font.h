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
# include <misc/hash_table.h>
# include <misc/string.h>
# include <deque>
# include <QSharedPointer>
# include <QMap>
# include "texture.h"

class QFont;
class QFontMetricsF;

namespace TA3D
{
	class GFX;


    class Font : public zuzuf::ref_count
	{
    public:
        typedef zuzuf::smartptr<Font>   Ptr;

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
		** \return True if the operator succeeded, False otherwise
		*/
        bool load( const QString &filename, const int size);

		int get_size();
        void print(float x, float y, const quint32 col, const QString &text);
        void print_center(float x, float y, const quint32 col, const QString &text);
        void print_right(float x, float y, const quint32 col, const QString &text);

		void setBold(bool bBold)	{	this->bBold = bBold;	}

    private:
        Font& operator = (const Font& rhs); // Forbidden!

	private:
        //! The Qt Font
        QSharedPointer<QFont> font;
        QSharedPointer<QFontMetricsF> metrics;

        QMap<QChar, GfxTexture::Ptr> glyphs;

		//! The filename of the font
        QString pFontFilename;
		//! Bold style
		bool bBold;
        // Friend
		friend class GFX;

    }; // class Font




	class FontManager
	{
	public:
		FontManager();
		~FontManager();

		void destroy();

        Font::Ptr find(const QString& filename, const int size);

	private:
        Font::Ptr internalRegisterFont(const QString& key, const QString& filename, const int size);

	private:
		//! Font list
        typedef std::deque<Font::Ptr>  FontList;

		FontList  pFontList;
        UTILS::HashMap<Font::Ptr>::Dense    font_table;
	}; // class FontManager



	//! Font manager
	extern FontManager font_manager;




} // namespace TA3D

#endif // __TA3D_GFX_FONT_H__
