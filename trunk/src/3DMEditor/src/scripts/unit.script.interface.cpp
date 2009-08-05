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

#include "unit.script.interface.h"
#include "unit.script.h"

namespace TA3D
{
    const char *UnitScriptInterface::script_name[] =
        {
            "QueryPrimary","AimPrimary","FirePrimary",
            "QuerySecondary","AimSecondary","FireSecondary",
            "QueryTertiary","AimTertiary","FireTertiary",
            "TargetCleared","StopBuilding","Stop",
            "StartBuilding","Go","Killed",
            "StopMoving","Deactivate","Activate",
            "Create","MotionControl","startmoving",
            "MoveRate1","MoveRate2","MoveRate3",
            "RequestState","TransportPickup","TransportDrop",
            "QueryTransport","BeginTransport","EndTransport",
            "SetSpeed","SetDirection","SetMaxReloadTime",
            "QueryBuildInfo","SweetSpot","RockUnit",
            "QueryLandingPad","SetSFXoccupy", "HitByWeapon",
            "QueryNanoPiece", "AimFromPrimary", "AimFromSecondary",
            "AimFromTertiary"
        };

    const QString UnitScriptInterface::get_script_name(int id)
	{
		if (id < 0)
            return QString();
		if (id >= NB_SCRIPT)            // Special case for weapons
		{
			const int weaponID = (id - NB_SCRIPT) / 4 + 4;
			switch((id - NB_SCRIPT) % 4)
			{
				case 0:         // QueryWeapon
                    return QString("QueryWeapon") + weaponID;
				case 1:         // AimWeapon
                    return QString("AimWeapon") + weaponID;
				case 2:         // AimFromWeapon
                    return QString("AimFromWeapon") + weaponID;
				case 3:         // FireWeapon
                    return QString("FireWeapon") + weaponID;
			}
		}
		return script_name[id];
	}


    int UnitScriptInterface::get_script_id(const QString &name)
    {
        for(int id = 0 ; id < NB_SCRIPT ; id++)
            if ( strcasecmp(script_name[id], name.toStdString().c_str()) == 0)
                return id;
        return -1;
    }
}
