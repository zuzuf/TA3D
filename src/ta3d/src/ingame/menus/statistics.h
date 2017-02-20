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
#ifndef __TA3D_INGAME_MENUS_STATISTICS_H__
# define __TA3D_INGAME_MENUS_STATISTICS_H__

# include <stdafx.h>
# include <misc/string.h>
# include "base.h"


namespace TA3D
{
namespace Menus
{


	/*! \class Statistics
	**
	** \brief All statistics about the last bloody battle
	*/
	class Statistics : public Abstract
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
		Statistics();
		//! Destructor
		virtual ~Statistics();
		//@}

	protected:
		virtual bool doInitialize();
		virtual void doFinalize();
		virtual void waitForEvent();
		virtual bool maySwitchToAnotherMenu();

	private:
		/*!
		** \brief Update the statistics for a single player and a single column
		**
		** \param id The object on the area to update (pArea)
		** \param indx Index of the player
		** \param color The color of the player
		** \param value The text
		*/
		void doUpdateObject(const QString& id, const short indx, const uint32 color, const QString& value);

	}; // class Statistics





} // namespace Menus
} // namespace TA3D


#endif // __TA3D_INGAME_MENUS_STATISTICS_H__
