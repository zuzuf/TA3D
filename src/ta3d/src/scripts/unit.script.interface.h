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

#ifndef __UnitScriptInterface_H__
# define __UnitScriptInterface_H__

# include "../misc/string.h"
# include "script.interface.h"
# include "../misc/hash_table.h"

# define SCRIPT_QueryPrimary		0x00
# define SCRIPT_AimPrimary		0x01
# define SCRIPT_FirePrimary		0x02
# define SCRIPT_QuerySecondary	0x03
# define SCRIPT_AimSecondary		0x04
# define SCRIPT_FireSecondary	0x05
# define SCRIPT_QueryTertiary	0x06
# define SCRIPT_AimTertiary		0x07
# define SCRIPT_FireTertiary		0x08
# define SCRIPT_TargetCleared	0x09
# define SCRIPT_stopbuilding		0x0A
# define SCRIPT_stop				0x0B
# define SCRIPT_startbuilding	0x0C
# define SCRIPT_go				0x0D
# define SCRIPT_killed			0x0E
# define SCRIPT_StopMoving		0x0F
# define SCRIPT_Deactivate		0x10
# define SCRIPT_Activate			0x11
# define SCRIPT_create			0x12
# define SCRIPT_MotionControl	0x13
# define SCRIPT_startmoving		0x14
# define SCRIPT_MoveRate1		0x15
# define SCRIPT_MoveRate2		0x16
# define SCRIPT_MoveRate3		0x17
# define SCRIPT_RequestState		0x18
# define SCRIPT_TransportPickup	0x19
# define SCRIPT_TransportDrop	0x1A
# define SCRIPT_QueryTransport	0x1B
# define SCRIPT_BeginTransport	0x1C
# define SCRIPT_EndTransport		0x1D
# define SCRIPT_SetSpeed			0x1E
# define SCRIPT_SetDirection		0x1F
# define SCRIPT_SetMaxReloadTime	0x20
# define SCRIPT_QueryBuildInfo	0x21
# define SCRIPT_SweetSpot		0x22
# define SCRIPT_RockUnit			0x23
# define SCRIPT_QueryLandingPad	0x24
# define SCRIPT_setSFXoccupy     0x25
# define SCRIPT_HitByWeapon      0x26
# define SCRIPT_QueryNanoPiece   0x27
# define SCRIPT_AimFromPrimary   0x28
# define SCRIPT_AimFromSecondary 0x29
# define SCRIPT_AimFromTertiary  0x2A
# define NB_SCRIPT				0x2B

# define SCRIPT_QueryWeapon      NB_SCRIPT
# define SCRIPT_AimWeapon        (NB_SCRIPT + 1)
# define SCRIPT_AimFromWeapon    (NB_SCRIPT + 2)
# define SCRIPT_FireWeapon       (NB_SCRIPT + 3)

namespace TA3D
{
    /*!
    ** This class is an interface for all unit scripts types
    */
    class UnitScriptInterface : public ScriptInterface
    {
    public:
        static UnitScriptInterface *instanciate( ScriptData *data );
        static const String get_script_name(int id);
        static int get_script_id(const String &name);

    protected:
        uint32                  unitID;
        UTILS::cHashTable<int>  return_value;
    public:

        virtual void setUnitID(uint32 ID) = 0;

        virtual int getNbPieces() = 0;

        int getReturnValue(const String &name);
        void setReturnValue(const String &name, int value);

    private:
        static const char *script_name[];

    };
}

#endif
