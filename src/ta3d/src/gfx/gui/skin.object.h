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
#ifndef __TA3D_GFX_GUI_SKIN_OBJECT_H__
# define __TA3D_GFX_GUI_SKIN_OBJECT_H__

# include <stdafx.h>
# include <misc/string.h>
# include <sdl.h>
# include <misc/tdf.h>


namespace TA3D
{
namespace Gui
{


	/*!
	** \brief
	*/
	class SKIN_OBJECT
	{
	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor
		SKIN_OBJECT();
		//! Destructor
		~SKIN_OBJECT();
		//@}

		/*!
		** \brief
		*/
		void init();

		/*!
		** \brief
		*/
		void destroy();

		/*!
		** \brief
		**
		** \param prefix
		** \param parser
		** \param borderSize
		*/
		void load(TDFParser& parser, const QString& prefix, float borderSize = 1.0f );

		/*!
		** \brief
		**
		** \param X1
		** \param Y1
		** \param X2
		** \param Y2
		** \param bkg
		*/
		void draw(const float X1, const float Y1, const float X2, const float Y2, const bool bkg = true) const;

	public:
		//!
		GLuint  tex;

		//!
		float x1;
		//!
		float y1;
		//!
		float x2;
		//!
		float y2;

		//!
		float t_x1;
		//!
		float t_y1;
		//!
		float t_x2;
		//!
		float t_y2;

		//!
		uint32 w;
		//!
		uint32 h;

		//!
		float sw;
		//!
		float sh;

	}; // class SKIN_OBJECT





} // namespace Gui
} // namespace TA3D

#endif // __TA3D_GFX_GUI_SKIN_OBJECT_H__
