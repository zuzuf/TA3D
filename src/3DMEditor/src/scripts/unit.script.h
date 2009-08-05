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

#ifndef __UnitScript_H__
#define __UnitScript_H__

# include <QtCore>
# include "unit.script.interface.h"
# include "../../../ta3d/src/lua/lua.hpp"

# define SCRIPT_QueryPrimary            0x00
# define SCRIPT_AimPrimary              0x01
# define SCRIPT_FirePrimary             0x02
# define SCRIPT_QuerySecondary  0x03
# define SCRIPT_AimSecondary            0x04
# define SCRIPT_FireSecondary   0x05
# define SCRIPT_QueryTertiary   0x06
# define SCRIPT_AimTertiary             0x07
# define SCRIPT_FireTertiary            0x08
# define SCRIPT_TargetCleared   0x09
# define SCRIPT_stopbuilding            0x0A
# define SCRIPT_stop                            0x0B
# define SCRIPT_startbuilding   0x0C
# define SCRIPT_go                              0x0D
# define SCRIPT_killed                  0x0E
# define SCRIPT_StopMoving              0x0F
# define SCRIPT_Deactivate              0x10
# define SCRIPT_Activate                        0x11
# define SCRIPT_create                  0x12
# define SCRIPT_MotionControl   0x13
# define SCRIPT_startmoving             0x14
# define SCRIPT_MoveRate1               0x15
# define SCRIPT_MoveRate2               0x16
# define SCRIPT_MoveRate3               0x17
# define SCRIPT_RequestState            0x18
# define SCRIPT_TransportPickup 0x19
# define SCRIPT_TransportDrop   0x1A
# define SCRIPT_QueryTransport  0x1B
# define SCRIPT_BeginTransport  0x1C
# define SCRIPT_EndTransport            0x1D
# define SCRIPT_SetSpeed                        0x1E
# define SCRIPT_SetDirection            0x1F
# define SCRIPT_SetMaxReloadTime        0x20
# define SCRIPT_QueryBuildInfo  0x21
# define SCRIPT_SweetSpot               0x22
# define SCRIPT_RockUnit                        0x23
# define SCRIPT_QueryLandingPad 0x24
# define SCRIPT_setSFXoccupy     0x25
# define SCRIPT_HitByWeapon      0x26
# define SCRIPT_QueryNanoPiece   0x27
# define SCRIPT_AimFromPrimary   0x28
# define SCRIPT_AimFromSecondary 0x29
# define SCRIPT_AimFromTertiary  0x2A
# define NB_SCRIPT                              0x2B

# define SCRIPT_QueryWeapon      NB_SCRIPT
# define SCRIPT_AimWeapon        (NB_SCRIPT + 1)
# define SCRIPT_AimFromWeapon    (NB_SCRIPT + 2)


namespace TA3D
{
    /*!
    ** This class represents unit scripts, it's used to script unit behavior
    ** This is a Lua version of TA COB/BOS scripts
    */
    class UnitScript : public UnitScriptInterface
    {
        virtual const char *className() { return "UnitScript"; }
    public:

        UnitScript();
        virtual ~UnitScript();

        /*virtual*/ void load(const QString &code);
        /*virtual*/ int run(float dt, bool alone = false);                  // Run the script

        //! functions used to call/run Lua functions
        /*virtual*/ void call(const QString &functionName, int *parameters = NULL, int nb_params = 0);
        /*virtual*/ int execute(const QString &functionName, int *parameters = NULL, int nb_params = 0);

        //! functions used to create new threads sharing the same environment
        /*virtual*/ UnitScript *fork();
        /*virtual*/ UnitScript *fork(const QString &functionName, int *parameters = NULL, int nb_params = 0);
        UnitScript *fork(lua_State *cL, int n);

        int getNextID();

    private:
        lua_State *L;

        int nextID;

        int n_args;

        QString name;

    private:
        static void register_functions(lua_State *L);
        void register_info();

        /*virtual*/ void setUnitID(uint32 ID);
        /*virtual*/ int getNbPieces();

        void lua_getUnitTable();

        void init();
        void destroy();
    private:
        static lua_State *pLuaVM;
        static lua_State *luaVM();
    };

    void script_explode(int obj, int explosion_type);
    void script_turn_object(int obj, int axis, float angle, float speed);
    void script_move_object(int obj, int axis, float pos, float speed);
    int script_get_value_from_port(int portID, int *param = NULL);
    void script_spin_object(int obj, int axis, float target_speed, float accel);
    void script_show_object(int obj);
    void script_hide_object(int obj);
    void script_dont_cache(int obj);
    void script_cache(int obj);
    void script_shade(int obj);
    void script_dont_shade(int obj);
    void script_emit_sfx(int smoke_type, int from_piece);
    void script_stop_spin(int obj, int axis, float speed);
    void script_move_piece_now(int obj, int axis, float pos);
    void script_turn_piece_now(int obj, int axis, float angle);
    int script_get(int type, int v1, int v2);
    void script_set_value(int type, int v);
    void script_attach_unit(int unit_id, int piece_id);
    void script_drop_unit(int unit_id);
    bool script_is_turning(int obj, int axis);
    bool script_is_moving(int obj, int axis);
}

#endif
