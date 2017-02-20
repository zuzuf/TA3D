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
#ifndef __TA3D_INGAME_MENUS_INTRO_H__
# define __TA3D_INGAME_MENUS_INTRO_H__

# include <stdafx.h>
# include <misc/string.h>
# include "base.h"


# define TA3D_MENUS_INTRO_DEFAULT_INTRO_FILENAME  "intro.txt"


namespace TA3D
{
namespace Menus
{


	/*!
	** \brief The Intro menu
	*/
	class Intro : public Abstract
	{
	public:
		/*!
		** \brief Execute an instance of Menus::Intro
		*/
		static bool Execute();

	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor
		Intro();
		//! Destructor
		virtual ~Intro();
		//@}


		/*!
		** \brief reload the content of the scrolling text
		*/
		void reloadContent();

	protected:
		virtual bool doInitialize();
		virtual void doFinalize();
		virtual void waitForEvent();
		virtual bool maySwitchToAnotherMenu();
		virtual void redrawTheScreen();

	private:
		/*!
		** \brief Reload the texture of the background
		*/
		void loadBackgroundTexture();

		/*!
		** \brief Scroll the content
		*/
		void scrollTheText();

	private:
		//! The scrolling text
		QStringList pContent;
		//! Cached size of the content
		unsigned int pContentSize;

		uint32 pScrollTimer;

		//! The texture of the background
		GLuint pBackgroundTexture;
		//! Current font height
		float pCurrentFontHeight;

		//! Delta position to display the text
		float pDelta;
		//! The first index in the list where to start from
		unsigned int pStartIndex;

	}; // class Intro





} // namespace Menus
} // namespace TA3D

#endif // __TA3D_INGAME_MENUS_INTRO_H__
