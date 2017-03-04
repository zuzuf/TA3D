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
#ifndef __TA3D_INGAME_MENUS_SPLASH_H__
# define __TA3D_INGAME_MENUS_SPLASH_H__

# include <stdafx.h>
# include <misc/string.h>
# include "base.h"
# include <engine.h>


# define TA3D_MENUS_SPLASH_DEFAULT_SPLASH_FILENAME  "intro.txt"


namespace TA3D
{
namespace Menus
{


	/*!
	** \brief A window to make the user wait for the engine's initializtion at startup
	*/
	class Splash : public Abstract
	{
	public:
		/*!
		** \brief Execute an instance of Menus::Splash
		*/
		static bool Execute(Engine& engine);

	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor
		Splash(Engine& engine);
		//! Destructor
		virtual ~Splash();
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

	private:
		//! The Ta3D Engine
		Engine& pEngine;
		//! The texture of the background
        GfxTexture::Ptr pBackgroundTexture;
		//
		float pLeft;
		float pTop;
		//
		float pHeight;
		float pWidth;

	}; // class Splash





} // namespace Menus
} // namespace TA3D

#endif // __TA3D_INGAME_MENUS_SPLASH_H__
