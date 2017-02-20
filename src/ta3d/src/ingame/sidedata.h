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
#ifndef __TA3D_INGAME_SideData_H__
# define __TA3D_INGAME_SideData_H__

# include <stdafx.h>
# include <ta3dbase.h>
# include <misc/string.h>
# include <tdf.h>


namespace TA3D
{

	struct IntrElementCoords // INT_ELEMENT
	{
		int	x1, y1;
		int	x2, y2;

	}; // IntrElementCoords


	struct InterfaceData
	{
		//!
		IntrElementCoords EnergyBar;
		//!
		IntrElementCoords EnergyNum;
		//!
		IntrElementCoords EnergyMax;
		//!
		IntrElementCoords Energy0;
		//!
		IntrElementCoords EnergyProduced;
		//!
		IntrElementCoords EnergyConsumed;

		//!
		IntrElementCoords MetalBar;
		//!
		IntrElementCoords MetalNum;
		//!
		IntrElementCoords MetalMax;
		//!
		IntrElementCoords Metal0;
		//!
		IntrElementCoords MetalProduced;
		//!
		IntrElementCoords MetalConsumed;

		//!
		IntrElementCoords UnitName;
		//!
		IntrElementCoords DamageBar;

		//!
		IntrElementCoords UnitName2;
		//!
		IntrElementCoords DamageBar2;

		//!
		IntrElementCoords UnitMetalMake;
		//!
		IntrElementCoords UnitMetalUse;
		//!
		IntrElementCoords UnitEnergyMake;
		//!
		IntrElementCoords UnitEnergyUse;

		//!
		IntrElementCoords Name;
		//!
		IntrElementCoords Description;

		//!
		uint32 metal_color;
		//!
		uint32 energy_color;

	}; // InterfaceData






	/*!
	** \brief
	*/
	class SideData
	{
	public:
		SideData();
		~SideData();

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
		*/
		int	getSideId(const QString& side) const;

		/*!
		** \brief
		*/
		void loadData();

	public:
		//!
		int nb_side;
		//! The name
		QString side_name[TA3D_PLAYERS_HARD_LIMIT];
		//! The prefix
		QString side_pref[TA3D_PLAYERS_HARD_LIMIT];
		//! The commande
		QString side_com[TA3D_PLAYERS_HARD_LIMIT];
		//! The GAF interface
		QString side_int[TA3D_PLAYERS_HARD_LIMIT];
		//! The interface data (position of the gui elements)
		InterfaceData side_int_data[TA3D_PLAYERS_HARD_LIMIT];
		//!
		QString unit_ext;
		//!
		QString unit_dir;
		//!
		QString model_dir;
		//!
		QString download_dir;
		//!
		QString weapon_dir;
		//!
		QString guis_dir;
		//!
		QString gamedata_dir;

	}; // class SideData





	/*!
	** \brief
	*/
	IntrElementCoords read_gui_element(TDFParser* parser, const QString& element, bool bottom = false);


	/*!
	**
	*/
	extern SideData ta3dSideData;



} // namespace TA3D

#endif // __TA3D_INGAME_SideData_H__
