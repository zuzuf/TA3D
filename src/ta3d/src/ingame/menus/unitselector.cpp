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

#include "unitselector.h"
#include <misc/paths.h>
#include <misc/files.h>
#include <algorithm>
#include <fbi.h>
#include <logs/logs.h>
#include <languages/i18n.h>
#include <input/mouse.h>
#include <input/keyboard.h>



namespace TA3D
{
namespace Menus
{

	bool UnitSelector::Execute(const QString& preSelectedUnits, QString& useOnly)
	{
		UnitSelector m(preSelectedUnits);
		bool r = m.execute();
		useOnly = (r) ? m.selected() : preSelectedUnits;
        return (r && (!useOnly.isEmpty()));
	}





	UnitSelector::UnitSelector()
		:Abstract(),
		pUseOnly(),
		pDefaultUseOnly(),
		pLastUnitIndex(-1)
	{}


	UnitSelector::UnitSelector(const QString& preSelectedUnits)
		:Abstract(),
		pUseOnly(preSelectedUnits),
		pDefaultUseOnly(preSelectedUnits),
		pLastUnitIndex(-1)
	{}


	UnitSelector::~UnitSelector()
	{
		unit_manager.destroy();
	}


	bool UnitSelector::doInitialize()
	{
		LOG_DEBUG(LOG_PREFIX_MENU_UNITSELECTOR << "Entering...");

		loadAreaFromTDF("unit setup", "gui/unitsetup.area");
		// Load all units, if any
		unit_manager.load_all_units();
		if (!unit_manager.nb_unit) // should abort if no map is present
			return false;

        if (!pUseOnly.isEmpty())          // Load previous selection
		{
			TDFParser useonly_parser(pUseOnly, false, false, true); // In gadgets mode so we can read the special key :)
			for (int i = 0; i < unit_manager.nb_unit ; i++)
				unit_manager.unit_type[i]->not_used = true;
			QString unit_name;
			int i = 0;
            while (!(unit_name = useonly_parser.pullAsString(QString("gadget%1").arg(i))).isEmpty())
			{
				int idx = unit_manager.get_unit_index( unit_name );
				if (idx >= 0)
					unit_manager.unit_type[idx]->not_used = false;
				++i;
			}
		}

		pUnitList.clear();
		for (int i = 0 ; i < unit_manager.nb_unit ; ++i)
			pUnitList.push_back( unit_manager.unit_type[i]->Unitname );
		pUnitList.sort();

		// The mini map object
		pUnitPicObj = pArea->get_object("unitsetup.unitpic");
		if (!pUnitPicObj)
			LOG_ERROR(LOG_PREFIX_MENU_UNITSELECTOR << "Can not find the GUI object `unitsetup.unitpic`");
		else
			pUnitPicObj->Data = 0;

		// The control which contains all available maps
		pUnitListObj = pArea->get_object("unitsetup.unit_list");
		reloadUnitsForGUIControl();
		doGoSelectSingleUnit(0);

		return true;
	}

	void UnitSelector::reloadUnitsForGUIControl()
	{
		if (pUnitListObj)
		{
			// Load all units
            pUnitListObj->Text.clear();
            pUnitListObj->Text.reserve(pUnitList.size());
            for (const QString &i : pUnitList)
			{
                pUnitListObj->Text.push_back(i);
                const int type_id = unit_manager.get_unit_index(i);
				if (type_id >= 0 && !unit_manager.unit_type[type_id]->not_used)
                    pUnitListObj->Text.back() = "<H>" + pUnitListObj->Text.back();
			}
		}
	}


	void UnitSelector::doFinalize()
	{
		// Wait for user to release ESC
		while (key[KEY_ESC])
		{
			SuspendMilliSeconds(TA3D_MENUS_RECOMMENDED_TIME_MS_FOR_RESTING);
			poll_inputs();
		}
		LOG_DEBUG(LOG_PREFIX_MENU_UNITSELECTOR << "Done.");
	}


	void UnitSelector::waitForEvent()
	{
		bool keyIsPressed(false);
		do
		{
			// Grab user events
			pArea->check();
			// Get if a key was pressed
			keyIsPressed = pArea->key_pressed;
			// Wait to reduce CPU consumption
			wait();

		} while (pMouseX == mouse_x && pMouseY == mouse_y && pMouseZ == mouse_z && pMouseB == mouse_b
				 && mouse_b == 0
				 && !key[KEY_ENTER] && !key[KEY_ESC] && !key[KEY_SPACE] && !key[KEY_C]
				 && !keyIsPressed && !pArea->scrolling);
	}


	bool UnitSelector::maySwitchToAnotherMenu()
	{
		// Aborting
		if (pArea->get_state("unitsetup.b_cancel") || key[KEY_ESC])
		{
			pUseOnly = pDefaultUseOnly;
			if (pUnitListObj)
				pUnitListObj->Data = 0;
			return true;
		}

		// Go ! Validate selection !
		if (pArea->get_state("unitsetup.b_ok") || key[KEY_ENTER])
		{
			if (pUnitListObj)
			{
				pUnitListObj->Data = 0;
				createUseOnlyFile();
			}
			return true;
		}

		// Enable/Disable selected unit ! (require pUnitListObj != NULL)
		if (pUnitListObj && pLastUnitIndex >= 0)
		{
			if (pArea->get_state("unitsetup.c_enabled"))        // Enable
			{
                QString &UnitName = pUnitListObj->Text[ pLastUnitIndex ];
				if (UnitName.size() > 0 && UnitName[0] != '<')
                    UnitName = "<H>" + UnitName;
			}
			else                                                // Disable
			{
                QString &UnitName = pUnitListObj->Text[ pLastUnitIndex ];
				if (UnitName.size() > 0 && UnitName[0] == '<')
					UnitName = Substr(UnitName, 3, UnitName.size() - 3);
			}
		}

		// Selection change
		if (pUnitListObj)
			return doGoSelectSingleUnit(pUnitListObj->Pos);

		return false;
	}


	bool UnitSelector::doGoSelectSingleUnit(const int unitIndex)
	{
		// Bounds checking
		if (unitIndex < 0 || unitIndex >= unit_manager.nb_unit || pLastUnitIndex == unitIndex)
			return false;

		// Cached value
		pLastUnitIndex = unitIndex;

		// Logs
		LOG_DEBUG(LOG_PREFIX_MENU_UNITSELECTOR << "`" << pUnitListObj->Text[ unitIndex ]
				  << "` has been selected (indx: " << unitIndex << ")");

		// Update unit info
		doUpdateUnitInfo();

		return false;
	}


	void UnitSelector::doUpdateUnitInfo()
	{
		if (pUnitPicObj)
		{
			QString UnitName = pUnitListObj->Text[ pLastUnitIndex ];
			if (UnitName.size() > 0 && UnitName[0] == '<')
			{
				UnitName = Substr(UnitName, 3, UnitName.size() - 3);
				pArea->set_state("unitsetup.c_enabled", true);
			}
			else
				pArea->set_state("unitsetup.c_enabled", false);
			int type_id = unit_manager.get_unit_index( UnitName );
			if (type_id >= 0)
			{
				if (unit_manager.unit_type[type_id]->unitpic)
				{
					gfx->set_texture_format(gfx->defaultTextureFormat_RGB());
					unit_manager.unit_type[type_id]->glpic = gfx->make_texture(unit_manager.unit_type[type_id]->unitpic, FILTER_LINEAR);
					SDL_FreeSurface(unit_manager.unit_type[type_id]->unitpic);
					unit_manager.unit_type[type_id]->unitpic = NULL;
				}
				pUnitPicObj->Data = unit_manager.unit_type[type_id]->glpic;

				QString info_string;
                info_string += unit_manager.unit_type[type_id]->name + "\n\n";
                info_string += unit_manager.unit_type[type_id]->Description + "\n";
				pArea->caption("unitsetup.unit_info", info_string);
			}
		}
	}

	void UnitSelector::createUseOnlyFile()
	{
        TA3D::Paths::MakeDir(TA3D::Paths::Resources + "useonly");
		pUseOnly = "useonly/useonly.tdf";
        QString filename = TA3D::Paths::Resources + pUseOnly;

        QString s = "// Use Only file\n\n";

		QString UnitName;
		for (unsigned int i = 0; i < pUnitListObj->Text.size(); ++i)            // For each selected unit
		{
			if (pUnitListObj->Text[i][0] == '<')                        // create an empty section with the unit name
			{
				UnitName = pUnitListObj->Text[i];
                s   += '[' + Substr(UnitName, 3, UnitName.size() - 3) + "]\n{\n}\n";
			}
		}

        if (Paths::Files::SaveToFile(filename, s.toUtf8()))
		{
			LOG_INFO(LOG_PREFIX_MENU_UNITSELECTOR << "The useonly file has been saved.");
			return;
		}
		LOG_ERROR(LOG_PREFIX_MENU_UNITSELECTOR << "Impossible to write useonly file: `" << filename << "`");
	}




} // namespace Menus
} // namespace TA3D


