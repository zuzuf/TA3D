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
#ifndef __TA3D_GFX_GUI_SKIN_MANAGER_H__
# define __TA3D_GFX_GUI_SKIN_MANAGER_H__

# include <stdafx.h>
# include <misc/string.h>
# include "skin.h"
# include <misc/hash_table.h>

namespace TA3D
{
namespace Gui
{


	/*!
	** \brief
	*/
	class SKIN_MANAGER
	{
	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor
		SKIN_MANAGER();
		//! Destructor
		~SKIN_MANAGER();
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
		** \param filename
		*/
		Skin *load(const QString& filename, const float scale = 1.0f);

	private:
		std::vector<Skin*>  skins;
		UTILS::HashMap<Skin*>::Dense  hash_skin;
	}; // class SKIN_MANAGER




	extern SKIN_MANAGER skin_manager;




} // namespace Gui
} // namespace TA3D

#endif // __TA3D_GFX_GUI_SKIN_MANAGER_H__
