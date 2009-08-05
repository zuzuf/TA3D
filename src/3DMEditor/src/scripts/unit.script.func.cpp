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

#include "../mesh.h"
#include "../logs.h"
#include "unit.script.interface.h"

# define DEG2RAD  (M_PI / 180.0f)
# define RAD2DEG  (180.0f / M_PI)

# define DEG2TA   (65536.0f / 360.0f)
# define TA2DEG   (360.0f / 65536.0f)

# define RAD2TA   (RAD2DEG * DEG2TA)
# define TA2RAD   (TA2DEG  * DEG2RAD)

namespace TA3D
{
    //! functions that are called from scripts (COB/BOS and Lua)
    void script_explode(int obj, int explosion_type)
    {
        Mesh *cur = Mesh::instance()->getMeshByScriptID(obj);
        if (!cur)   return;

        cur->axe[0].pos = 0.0f;
        cur->axe[0].angle = 0.0f;
        cur->axe[1].pos = 0.0f;
        cur->axe[1].angle = 0.0f;
        cur->axe[2].pos = 0.0f;
        cur->axe[2].angle = 0.0f;
        cur->anim_flag |= FLAG_EXPLODE;
        cur->explosion_flag = explosion_type;
        cur->axe[0].move_speed = (25.0f + (qrand() % 2501) * 0.01f) * ((qrand() & 1) ? 1.0f : -1.0f);
        cur->axe[0].rot_speed = (qrand() % 7201) * 0.1f - 360.0f;
        cur->axe[1].move_speed = 25.0f + (qrand() % 2501) * 0.01f;
        cur->axe[1].rot_speed = (qrand() % 7201) * 0.1f - 360.0f;
        cur->axe[2].move_speed = (25.0f + (qrand() % 2501) * 0.01f) * (qrand() & 1 ? 1.0f : -1.0f);
        cur->axe[2].rot_speed = (qrand() % 7201) * 0.1f - 360.0f;
        cur->explode = true;
        cur->explode_time = 1.0f;
    }

    void script_turn_object(int obj, int axis, float angle, float speed)
    {
        Mesh *cur = Mesh::instance()->getMeshByScriptID(obj);
        if (!cur)   return;

        if (axis != 2)
        {
            angle = -angle;
            speed = -speed;
        }
        cur->axe[axis].reset_rot();
        cur->axe[axis].is_moving = true;
        cur->axe[axis].rot_angle = -angle;
        cur->axe[axis].rot_accel = 0.0f;
        cur->axe[axis].rot_angle -= cur->axe[axis].angle;
        while(cur->axe[axis].rot_angle > 180.0f && !isnan(cur->axe[axis].rot_angle))					// Fait le tour dans le sens le plus rapide
            cur->axe[axis].rot_angle -= 360.0f;
        while(cur->axe[axis].rot_angle < -180.0f && !isnan(cur->axe[axis].rot_angle))					// Fait le tour dans le sens le plus rapide
            cur->axe[axis].rot_angle += 360.0f;
        if (cur->axe[axis].rot_angle > 0.0f)
            cur->axe[axis].rot_speed = fabsf(speed);
        else
            cur->axe[axis].rot_speed = -fabsf(speed);
        cur->axe[axis].rot_limit = true;
        cur->axe[axis].rot_speed_limit = false;
    }

    void script_move_object(int obj, int axis, float pos, float speed)
    {
        Mesh *cur = Mesh::instance()->getMeshByScriptID(obj);
        if (!cur)   return;

        if (axis == 0)
            pos = -pos;

        cur->axe[axis].reset_move();
        cur->axe[axis].move_distance = pos - cur->axe[axis].pos;
        cur->axe[axis].is_moving = true;
        if (cur->axe[axis].move_distance < 0.0f)
            cur->axe[axis].move_speed = -fabsf(speed * 0.5f);
        else
            cur->axe[axis].move_speed = fabsf(speed * 0.5f);
    }

    int script_get_value_from_port(int portID, int *param)
    {
        switch(portID)
        {
            case MIN_ID:		// returns the lowest valid unit ID number
                return 0;
            case MAX_ID:		// returns the highest valid unit ID number
                return 254;
            case MY_ID:		// returns ID of current unit
                return 0;
            case UNIT_TEAM:		// returns team(player ID in TA) of unit given with parameter
                return 0;
            case VETERAN_LEVEL:		// gets kills * 100
                return 0;			// not yet implemented
            case ATAN:
                if (param)
                {
                    int v1 = param[1];
                    int v2 = param[0];
                    return (int)(atanf((float)v1/v2)+0.5f);
                }
                else
                    return 0;
            case HYPOT:
                if (param)
                {
                    int v1 = param[1];
                    int v2 = param[0];
                    return (int)(sqrtf((float)(v1 * v1 + v2 * v2)) + 0.5f);
                }
                else
                    return 0;
            case BUGGER_OFF:
                return 0;
            case BUILD_PERCENT_LEFT:
                return 0;
            case YARD_OPEN:
            case ACTIVATION:
            case HEALTH:
            case INBUILDSTANCE:
            case BUSY:
            case ARMORED:
            case STANDINGMOVEORDERS:			// A faire : ajouter le support des ordres de mouvement/feu
            case STANDINGFIREORDERS:
                return 0;//port[ portID ];
        };
        const char *op[]={"INCONNU","ACTIVATION","STANDINGMOVEORDERS","STANDINGFIREORDERS","HEALTH","INBUILDSTANCE","BUSY","PIECE_XZ","PIECE_Y",
            "UNIT_XZ","UNIT_Y","UNIT_HEIGHT","XZ_ATAN","XZ_HYPOT","ATAN","HYPOT","GROUND_HEIGHT","BUILD_PERCENT_LEFT","YARD_OPEN",
            "BUGGER_OFF","ARMORED"};
        if (portID > 20)
            portID = 0;
        LOG_DEBUG("GET_VALUE_FROM_PORT: opération non gérée : " << op[portID]);
        return 0;
    }

    void script_spin_object(int obj, int axis, float target_speed, float accel)
    {
        Mesh *cur = Mesh::instance()->getMeshByScriptID(obj);
        if (!cur)   return;

        if (axis == 1)
        {
            target_speed = -target_speed;
            accel = -accel;
        }
        cur->axe[axis].reset_rot();
        cur->axe[axis].is_moving = true;
        cur->axe[axis].rot_limit = false;
        cur->axe[axis].rot_speed_limit = true;
        cur->axe[axis].rot_target_speed = target_speed;
        if (accel != 0.0f)
        {
            if (cur->axe[axis].rot_target_speed > cur->axe[axis].rot_speed)
                cur->axe[axis].rot_accel = fabsf(accel);
            else
                cur->axe[axis].rot_accel = -fabsf(accel);
        }
        else
        {
            cur->axe[axis].rot_accel = 0;
            cur->axe[axis].rot_speed = cur->axe[axis].rot_target_speed;
        }
    }

    void script_show_object(int obj)
    {
        Mesh *cur = Mesh::instance()->getMeshByScriptID(obj);
        if (!cur)   return;

        cur->anim_flag &= (~FLAG_HIDE);
    }

    void script_hide_object(int obj)
    {
        Mesh *cur = Mesh::instance()->getMeshByScriptID(obj);
        if (!cur)   return;

        cur->anim_flag |= FLAG_HIDE;
    }

    void script_dont_cache(int obj)
    {
        Mesh *cur = Mesh::instance()->getMeshByScriptID(obj);
        if (!cur)   return;

        cur->anim_flag |= FLAG_ANIMATED_TEXTURE;
    }

    void script_cache(int obj)
    {
        Mesh *cur = Mesh::instance()->getMeshByScriptID(obj);
        if (!cur)   return;

        cur->anim_flag &= (~FLAG_ANIMATED_TEXTURE);
    }

    void script_shade(int obj)
    {
        Mesh *cur = Mesh::instance()->getMeshByScriptID(obj);
        if (!cur)   return;

        cur->anim_flag &= (~FLAG_DONT_SHADE);
    }

    void script_dont_shade(int obj)
    {
        Mesh *cur = Mesh::instance()->getMeshByScriptID(obj);
        if (!cur)   return;

        cur->anim_flag |= FLAG_DONT_SHADE;
    }

    void script_emit_sfx(int smoke_type, int from_piece)
    {
        Mesh *cur = Mesh::instance()->getMeshByScriptID(from_piece);
        if (!cur)   return;
    }

    void script_stop_spin(int obj, int axis, float speed)
    {
        Mesh *cur = Mesh::instance()->getMeshByScriptID(obj);
        if (!cur)   return;

        if (axis != 2)
            speed = -speed;
        cur->axe[axis].reset_rot();
        cur->axe[axis].is_moving = true;
        cur->axe[axis].rot_limit = false;
        cur->axe[axis].rot_speed_limit = true;
        cur->axe[axis].rot_target_speed = 0.0f;
        if (speed == 0.0f)
        {
            cur->axe[axis].rot_speed = 0.0f;
            cur->axe[axis].rot_accel = 0.0f;
        }
        else
        {
            if (cur->axe[axis].rot_speed > 0.0f)
                cur->axe[axis].rot_accel = -fabsf(speed);
            else
                cur->axe[axis].rot_accel = fabsf(speed);
        }
    }

    void script_move_piece_now(int obj, int axis, float pos)
    {
        Mesh *cur = Mesh::instance()->getMeshByScriptID(obj);
        if (!cur)   return;

        cur->axe[axis].reset_move();
        cur->axe[axis].is_moving = true;
        if (axis == 0)
            cur->axe[axis].pos = -pos;
        else
            cur->axe[axis].pos = pos;
    }

    void script_turn_piece_now(int obj, int axis, float angle)
    {
        Mesh *cur = Mesh::instance()->getMeshByScriptID(obj);
        if (!cur)   return;

        cur->axe[axis].reset_rot();
        cur->axe[axis].is_moving = true;
        if (axis != 2)
            angle = -angle;
        cur->axe[axis].angle = -angle;
    }

    int script_get(int type, int v1, int v2)
    {
        switch(type)
        {
            case VETERAN_LEVEL:
                return 0;
            case MIN_ID:
                return 0;
            case MAX_ID:
                return 254;
            case MY_ID:
                return 0;
            case UNIT_TEAM:		// returns team(player ID in TA) of unit given with parameter
                return 0;
            case UNIT_BUILD_PERCENT_LEFT:		// basically BUILD_PERCENT_LEFT, but comes with a unit parameter
                return 0;
            case UNIT_ALLIED:		// is unit given with parameter allied to the unit of the current COB script. 0=not allied, not zero allied
                return 1;
            case UNIT_IS_ON_THIS_COMP:		// indicates if the 1st parameter(a unit ID) is local to this computer
                return 1;
            case BUILD_PERCENT_LEFT:
                return 0;
            case ACTIVATION:
            case STANDINGMOVEORDERS:
            case STANDINGFIREORDERS:
            case HEALTH:
            case INBUILDSTANCE:
            case BUSY:
            case YARD_OPEN:
            case BUGGER_OFF:
            case ARMORED:
                return 0;//(int)port[type];
            case PIECE_XZ:
                {
                    Vec P = Mesh::instance()->getRelativePosition(v1);
                    return PACKXZ(P.x * 2.0f, P.z * 2.0f);
                }
            case PIECE_Y:
                {
                    Vec P = Mesh::instance()->getRelativePosition(v1);
                    return (int)(P.y * 2.0f) << 16;
                }
            case UNIT_XZ:
                return PACKXZ( 0.0f, 0.0f );
            case UNIT_Y:
                return (int)(0.0f) << 16;
            case UNIT_HEIGHT:
                return 0;//(int)(model->top * 2.0f) << 16;
            case XZ_ATAN:
                return (int)(atan2f( UNPACKX(v1) , UNPACKZ(v1) ) * RAD2TA) + 32768;
            case XZ_HYPOT:
                return (int)hypotf( UNPACKX(v1), UNPACKZ(v1) ) << 16;
            case ATAN:
                return (int)(atan2f(v1,v2) * RAD2TA );
            case HYPOT:
                return (int)hypotf(v1,v2);
            case GROUND_HEIGHT:
                return 0;
            default:
                LOG_DEBUG(LOG_PREFIX_SCRIPT << "GET unknown constant " << type);
        }
        return 0;
    }

    void script_set_value(int type, int v)
    {
        switch(type)
        {
            case ACTIVATION:
//                if (v == 0)
//                    deactivate();
//                else
//                    activate();
                break;
            case YARD_OPEN:
//                port[type] = v;
                break;
            case BUGGER_OFF:
//                port[type] = v;
                break;
//            default:
//                port[type] = v;
        }
    }

    void script_attach_unit(int unit_id, int piece_id)
    {
    }

    void script_drop_unit(int unit_id)
    {
    }

    bool script_is_turning(int obj, int axis)
    {
        Mesh *cur = Mesh::instance()->getMeshByScriptID(obj);
        if (!cur)   return false;

        float a = cur->axe[axis].rot_angle;
        if ((cur->axe[axis].rot_speed != 0.0f || cur->axe[axis].rot_accel != 0.0f) && (a != 0.0f && cur->axe[axis].rot_limit))
            return true;
        else if (cur->axe[axis].rot_speed != cur->axe[axis].rot_target_speed && cur->axe[axis].rot_speed_limit)
            return true;
        cur->axe[axis].rot_speed = 0.0f;
        cur->axe[axis].rot_accel = 0.0f;
        return false;
    }

    bool script_is_moving(int obj, int axis)
    {
        Mesh *cur = Mesh::instance()->getMeshByScriptID(obj);
        if (!cur)   return false;

        return (cur->axe[axis].move_distance != 0.0f);
    }
}
