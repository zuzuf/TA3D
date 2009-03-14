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

#include "../stdafx.h"
#include "unit.script.interface.h"
#include "cob.vm.h"
#include "unit.script.h"

namespace TA3D
{
    UNIT_SCRIPT_INTERFACE *UNIT_SCRIPT_INTERFACE::instanciate( SCRIPT_DATA *data )
    {
        UNIT_SCRIPT_INTERFACE *usi = NULL;

        if ( dynamic_cast<COB_SCRIPT*>(data) )          // Try COB_SCRIPT (OTA COB/BOS)
            usi = new COB_VM();
        else if ( dynamic_cast<LUA_CHUNK*>(data) )      // Try LUA_CHUNK (Lua)
            usi = new UNIT_SCRIPT();

        usi->load( data );
        return usi;
    }

    int UNIT_SCRIPT_INTERFACE::getReturnValue(const String &name)
    {
        MutexLocker mLocker(pMutex);
        if (caller)
            return (static_cast<UNIT_SCRIPT_INTERFACE*>(caller))->getReturnValue( name );
        return return_value.find(String::ToUpper(name));
    }

    void UNIT_SCRIPT_INTERFACE::setReturnValue(const String &name, int value)
    {
        pMutex.lock();
        if (caller)
            (static_cast<UNIT_SCRIPT_INTERFACE*>(caller))->setReturnValue( name, value );
        else
            return_value.insertOrUpdate(String::ToUpper(name), value);
        pMutex.unlock();
    }

    const char *UNIT_SCRIPT_INTERFACE::script_name[] =
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

    String UNIT_SCRIPT_INTERFACE::get_script_name(int id)
    {
        if (id < 0) return String();
        if (id >= NB_SCRIPT)            // Special case for weapons
        {
            int weaponID = (id - NB_SCRIPT) / 4 + 4;
            switch((id - NB_SCRIPT) % 4)
            {
            case 0:         // QueryWeapon
                return format("QueryWeapon%d", weaponID);
            case 1:         // AimWeapon
                return format("AimWeapon%d", weaponID);
            case 2:         // AimFromWeapon
                return format("AimFromWeapon%d", weaponID);
            case 3:         // FireWeapon
                return format("FireWeapon%d", weaponID);
            };
        }
        return script_name[id];
    }

    int UNIT_SCRIPT_INTERFACE::get_script_id(const String &name)
    {
        for(int id = 0 ; id < NB_SCRIPT ; id++)
            if ( strcasecmp(script_name[id], name.c_str()) == 0)
                return id;
        return -1;
    }
}
