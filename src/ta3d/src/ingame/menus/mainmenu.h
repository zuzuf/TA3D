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
#ifndef __TA3D_INGAME_MENUS_X_MAIN_MENU_H__
# define __TA3D_INGAME_MENUS_X_MAIN_MENU_H__

# include <stdafx.h>
# include <misc/string.h>
# include "base.h"


namespace TA3D
{
namespace Menus
{

	/*!
	** \brief The main menu right after the short introduction sequence
	*/
	class MainMenu : public Abstract
	{
	public:
		/*!
		** \brief Execute an instance of MainMenu
		*/
		static bool Execute();

	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor
		MainMenu();
		//! Destructor
		virtual ~MainMenu();
		//@}

	protected:
		virtual bool doInitialize();
		virtual bool doExecute();
		virtual void doFinalize();
		virtual void waitForEvent();
		virtual bool maySwitchToAnotherMenu();


	private:
		/*!
		** \brief Reset the screen and OpenGL settings
		*/
		void resetScreen();

		/*!
		** \brief Redraw the entire screen for the main menu
		*/
		virtual void redrawTheScreen();

		/*!
		** \brief Grab informations about the current mod
		*/
		void getInfosAboutTheCurrentMod();

		void resetCaptions();

		/*!
		** \brief Go to the option menu
		** \return Always equals to false
		*/
		bool goToMenuOptions();

		/*!
		** \brief Go to the multiplayer menu
		** \return Always equals to false
		*/
		bool goToMenuMultiPlayers();

		/*!
		** \brief Go to the solo menu
		** \return Always equals to false
		*/
		bool goToMenuSolo();

		/*!
		** \brief change video settings
		*/
		void changeVideoSettings();

	private:
		//! Current mod
		QString pCurrentMod;
		//! Caption for the current mod (Cache)
		QString pCurrentModCaption;

		/*!
		** Get if we should not wait for an event (mouse,keyboard...) if enabled
		** \see waitForEvent()
		*/
		bool pDontWaitForEvent;

	}; // class MainMenu



} // namespace Menus
} // namespace TA3D

#endif // __TA3D_INGAME_MENUS_X_MAIN_MENU_H__
