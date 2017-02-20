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
#ifndef __TA3D_INGAME_MENUS_UNIT_SELECTOR_H__
# define __TA3D_INGAME_MENUS_UNIT_SELECTOR_H__

# include "base.h"
# include <vector>


namespace TA3D
{
namespace Menus
{


	/*!
	** \brief
	*/
	class UnitSelector : public Abstract
	{
	public:
		/*!
		** \brief Execute an instance of UnitSelector
		**
		** \param preSelectedUnits The name of the file that contains selected units
		** \param[out] useOnly Name of the useOnly file.
		** It is guaranteed this var will be set to preSelectedMap if cancel button is clicked (returned value = false)
		**
		** \return True if selection has been validated by the user, false otherwise
		*/
		static bool Execute(const QString& preSelectedUnits, QString& useOnly);

	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor
		UnitSelector();
		/*!
		** \brief Constructor
		** \param PreSelected map
		*/
		UnitSelector(const QString& preSelectedUnits);
		//! Destructor
		virtual ~UnitSelector();
		//@}


		/*!
		** \brief Get the useOnly file created if any
		**
		** This value is affected by the last call to execute(), and is empty by default
		**
		** \return Name of the useOnly file, empty if none has been selected
		*/
		const QString& selected() const { return pUseOnly; }


	protected:
		virtual bool doInitialize();
		virtual void doFinalize();
		virtual void waitForEvent();
		virtual bool maySwitchToAnotherMenu();

	private:
		void reloadUnitsForGUIControl();
		bool doGoSelectSingleUnit(const int unitIndex);
		void doUpdateUnitInfo();
		void createUseOnlyFile();

	private:
		//! Name of the UseOnly file
		QString pUseOnly;
		//! Default Name of the UseOnly file
		QString pDefaultUseOnly;

		//! List of units
		QStringList pUnitList;

		//! Reference to the unit picture object (Gui)
		Gui::GUIOBJ::Ptr pUnitPicObj;

		//! Last selected index
		int pLastUnitIndex;

		//! The list of units (Gui)
		Gui::GUIOBJ::Ptr pUnitListObj;

	}; // class MapSelector




} // namespace Menus
} // namespace TA3D



#endif // __TA3D_INGAME_MENUS_MAP_SELECTOR_H__
