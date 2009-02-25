#include "../stdafx.h"
#include "../ingame/players.h"
#include "../gfx/fx.manager.h"
#include "../UnitEngine.h"

namespace TA3D
{
    //! functions that are called from scripts (COB/BOS and Lua)
    void UNIT::script_explode(int obj, int explosion_type)
    {
        data.axe[0][obj].pos = 0.0f;
        data.axe[0][obj].angle = 0.0f;
        data.axe[1][obj].pos = 0.0f;
        data.axe[1][obj].angle = 0.0f;
        data.axe[2][obj].pos = 0.0f;
        data.axe[2][obj].angle = 0.0f;
        if (visible) // Don't draw things which could tell the player there is something there
        {
            compute_model_coord();
            particle_engine.make_fire( Pos + data.pos[obj],1,10,45.0f);
            int power = Math::Max(unit_manager.unit_type[type_id]->FootprintX, unit_manager.unit_type[type_id]->FootprintZ);
            Vector3D P = Pos + data.pos[obj];
            fx_manager.addExplosion( P, V, power * 3, power * 10.0f );
        }
        data.flag[obj] |= FLAG_EXPLODE;
        data.explosion_flag[obj] = explosion_type;
        data.axe[0][obj].move_speed = (25.0f + (Math::RandFromTable() % 2501) * 0.01f) * (Math::RandFromTable() & 1 ? 1.0f : -1.0f);
        data.axe[0][obj].rot_speed = (Math::RandFromTable() % 7201) * 0.1f - 360.0f;
        data.axe[1][obj].move_speed = 25.0f + (Math::RandFromTable() % 2501) * 0.01f;
        data.axe[1][obj].rot_speed = (Math::RandFromTable() % 7201) * 0.1f - 360.0f;
        data.axe[2][obj].move_speed = (25.0f + (Math::RandFromTable() % 2501) * 0.01f) * (Math::RandFromTable() & 1 ? 1.0f : -1.0f);
        data.axe[2][obj].rot_speed = (Math::RandFromTable() % 7201) * 0.1f - 360.0f;
        data.explode = true;
        data.explode_time = 1.0f;
    }

    void UNIT::script_turn_object(int obj, int axis, float angle, float speed)
    {
        if (axis != 2)
        {
            angle = -angle;
            speed = -speed;
        }
        data.axe[axis][obj].reset_rot();
        data.axe[axis][obj].is_moving = true;
        data.is_moving = true;
        data.axe[axis][obj].rot_angle = -angle;
        data.axe[axis][obj].rot_accel = 0.0f;
        data.axe[axis][obj].rot_angle -= data.axe[axis][obj].angle;
        while(data.axe[axis][obj].rot_angle > 180.0f && !isNaN(data.axe[axis][obj].rot_angle))					// Fait le tour dans le sens le plus rapide
            data.axe[axis][obj].rot_angle -= 360.0f;
        while(data.axe[axis][obj].rot_angle < -180.0f && !isNaN(data.axe[axis][obj].rot_angle))					// Fait le tour dans le sens le plus rapide
            data.axe[axis][obj].rot_angle += 360.0f;
        if (data.axe[axis][obj].rot_angle > 0.0f)
            data.axe[axis][obj].rot_speed = fabsf(speed);
        else
            data.axe[axis][obj].rot_speed = -fabsf(speed);
        data.axe[axis][obj].rot_limit = true;
        data.axe[axis][obj].rot_speed_limit = false;
    }

    void UNIT::script_move_object(int obj, int axis, float pos, float speed)
    {
        if (axis == 0)
            pos = -pos;

        data.axe[axis][obj].reset_move();
        data.axe[axis][obj].move_distance = pos - data.axe[axis][obj].pos;
        data.axe[axis][obj].is_moving = true;
        data.is_moving = true;
        if (data.axe[axis][obj].move_distance < 0.0f)
            data.axe[axis][obj].move_speed = -fabsf(speed * 0.5f);
        else
            data.axe[axis][obj].move_speed = fabsf(speed * 0.5f);
    }

    int UNIT::script_get_value_from_port(int portID, int *param)
    {
        switch(portID)
        {
            case MIN_ID:		// returns the lowest valid unit ID number
                return 0;
            case MAX_ID:		// returns the highest valid unit ID number
                return units.max_unit - 1;
            case MY_ID:		// returns ID of current unit
                return idx;
            case UNIT_TEAM:		// returns team(player ID in TA) of unit given with parameter
                return owner_id;
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
                return the_map->check_rect((((int)(Pos.x + the_map->map_w_d)) >> 3) - (unit_manager.unit_type[type_id]->FootprintX >> 1),
                                            (((int)(Pos.z + the_map->map_h_d)) >> 3) - (unit_manager.unit_type[type_id]->FootprintZ >> 1),
                                            unit_manager.unit_type[type_id]->FootprintX, unit_manager.unit_type[type_id]->FootprintZ, idx) ? 0 : 1;
            case BUILD_PERCENT_LEFT:
                return (port[ BUILD_PERCENT_LEFT ] = (int)build_percent_left + ( (build_percent_left > (int)build_percent_left) ? 1 : 0 ));
            case YARD_OPEN:
            case ACTIVATION:
            case HEALTH:
            case INBUILDSTANCE:
            case BUSY:
            case ARMORED:
            case STANDINGMOVEORDERS:			// A faire : ajouter le support des ordres de mouvement/feu
            case STANDINGFIREORDERS:
                return port[ portID ];
        };
        const char *op[]={"INCONNU","ACTIVATION","STANDINGMOVEORDERS","STANDINGFIREORDERS","HEALTH","INBUILDSTANCE","BUSY","PIECE_XZ","PIECE_Y",
            "UNIT_XZ","UNIT_Y","UNIT_HEIGHT","XZ_ATAN","XZ_HYPOT","ATAN","HYPOT","GROUND_HEIGHT","BUILD_PERCENT_LEFT","YARD_OPEN",
            "BUGGER_OFF","ARMORED"};
        if (portID > 20)
            portID = 0;
        LOG_DEBUG("GET_VALUE_FROM_PORT: opération non gérée : " << op[portID]);
        return 0;
    }

    void UNIT::script_spin_object(int obj, int axis, float target_speed, float accel)
    {
        if (axis == 1)
        {
            target_speed = -target_speed;
            accel = -accel;
        }
        data.axe[axis][obj].reset_rot();
        data.axe[axis][obj].is_moving = true;
        data.is_moving = true;
        data.axe[axis][obj].rot_limit = false;
        data.axe[axis][obj].rot_speed_limit = true;
        data.axe[axis][obj].rot_target_speed = target_speed;
        if (accel)
        {
            if (data.axe[axis][obj].rot_target_speed > data.axe[axis][obj].rot_speed)
                data.axe[axis][obj].rot_accel = fabsf(accel);
            else
                data.axe[axis][obj].rot_accel = -fabsf(accel);
        }
        else
        {
            data.axe[axis][obj].rot_accel = 0;
            data.axe[axis][obj].rot_speed = data.axe[axis][obj].rot_target_speed;
        }
    }

    void UNIT::script_show_object(int obj)
    {
        data.flag[obj] &= (~FLAG_HIDE);
    }

    void UNIT::script_hide_object(int obj)
    {
        data.flag[obj] |= FLAG_HIDE;
    }

    void UNIT::script_emit_sfx(int smoke_type, int from_piece)
    {
        if (visible)
        {
            compute_model_coord();
            if (data.dir[from_piece].x != 0.0f || data.dir[from_piece].y != 0.0f || data.dir[from_piece].z != 0.0f)
            {
                Vector3D dir = data.dir[from_piece];
                switch(smoke_type)
                {
                    case 0:
                        particle_engine.emit_part(Pos + data.pos[from_piece], dir, fire, 1, 10.0f, 2.5f, 5.0f, true);
                        break;
                    case 2:
                    case 3:
                        particle_engine.emit_part(Pos + data.pos[from_piece], dir, 0, 1, 10.0f, 10.0f, 10.0f, false, 0.3f);
                        break;
                    case 257:			// Fumée
                    case 258:
                        particle_engine.emit_part(Pos + data.pos[from_piece], dir, 0, 1, 10.0f, 10.0f, 10.0f, true, 0.3f);
                        break;
                }
            }
            else
                switch(smoke_type)
                {
                    case 0:
                        particle_engine.make_smoke(Pos + data.pos[from_piece], fire, 1, 0.0f, 0.0f, 0.0f, 0.5f);
                        break;
                    case 257:
                    case 258:
                        particle_engine.make_smoke(Pos + data.pos[from_piece], 0, 1, 10.0f, -1.0f, 0.0f, 0.5f);
                        break;
                }
        }
    }

    void UNIT::script_stop_spin(int obj, int axis, float speed)
    {
        if (axis != 2)
            speed = -speed;
        data.axe[axis][obj].reset_rot();
        data.axe[axis][obj].is_moving = true;
        data.is_moving = true;
        data.axe[axis][obj].rot_limit = false;
        data.axe[axis][obj].rot_speed_limit = true;
        data.axe[axis][obj].rot_target_speed = 0.0f;
        if (speed == 0.0f)
        {
            data.axe[axis][obj].rot_speed = 0.0f;
            data.axe[axis][obj].rot_accel = 0.0f;
        }
        else
        {
            if (data.axe[axis][obj].rot_speed > 0.0f)
                data.axe[axis][obj].rot_accel = -fabsf(speed);
            else
                data.axe[axis][obj].rot_accel = fabsf(speed);
        }
    }

    void UNIT::script_move_piece_now(int obj, int axis, float pos)
    {
        data.axe[axis][obj].reset_move();
        data.axe[axis][obj].is_moving = true;
        data.is_moving = true;
        if (axis == 0)
            data.axe[axis][obj].pos = -pos;
        else
            data.axe[axis][obj].pos = pos;
    }

    void UNIT::script_turn_piece_now(int obj, int axis, float angle)
    {
        data.axe[axis][obj].reset_rot();
        data.axe[axis][obj].is_moving = true;
        data.is_moving = true;
        if (axis != 2)
            angle = -angle;
        data.axe[axis][obj].angle = -angle;
    }

    int UNIT::script_get(int type, int v1, int v2)
    {
        switch(type)
        {
            case UNIT_TEAM:		// returns team(player ID in TA) of unit given with parameter
                if (v1 >= 0 && v1 < units.max_unit && (units.unit[ v1 ].flags & 1) )
                    return units.unit[ v1 ].owner_id;
                else
                    return -1;
            case UNIT_BUILD_PERCENT_LEFT:		// basically BUILD_PERCENT_LEFT, but comes with a unit parameter
                if (v1 >= 0 && v1 < units.max_unit && (units.unit[ v1 ].flags & 1) )
                    return (int)units.unit[ v1 ].build_percent_left + ( (units.unit[ v1 ].build_percent_left > (int)units.unit[ v1 ].build_percent_left) ? 1 : 0);
                else
                    return 0;
            case UNIT_ALLIED:		// is unit given with parameter allied to the unit of the current COB script. 0=not allied, not zero allied
                return !isEnemy( v1 );
            case UNIT_IS_ON_THIS_COMP:		// indicates if the 1st parameter(a unit ID) is local to this computer
                if (v1 >= 0 && v1 < units.max_unit && (units.unit[ v1 ].flags & 1) )
                    return !(players.control[ units.unit[ v1 ].owner_id ] & PLAYER_CONTROL_FLAG_REMOTE);
                else
                    return 0;
            case BUILD_PERCENT_LEFT:
                return (port[ BUILD_PERCENT_LEFT ] = (int)build_percent_left + ( (build_percent_left > (int)build_percent_left) ? 1 : 0));
            case ACTIVATION:
            case STANDINGMOVEORDERS:
            case STANDINGFIREORDERS:
            case HEALTH:
            case INBUILDSTANCE:
            case BUSY:
            case YARD_OPEN:
            case BUGGER_OFF:
            case ARMORED:
                return (int)port[type];
            case PIECE_XZ:
                compute_model_coord();
                return PACKXZ((data.pos[v1].x + Pos.x) * 2.0f + the_map->map_w, (data.pos[v1].z + Pos.z) * 2.0f + the_map->map_h);
            case PIECE_Y:
                compute_model_coord();
                return (int)((data.pos[v1].y + Pos.y) * 2.0f) << 16;
            case UNIT_XZ:
                if (v1 >= 0 && v1 < units.max_unit && (units.unit[v1].flags & 1) )
                    return PACKXZ( units.unit[v1].Pos.x * 2.0f + the_map->map_w, units.unit[v1].Pos.z * 2.0f + the_map->map_h );
                else
                    return PACKXZ( Pos.x * 2.0f + the_map->map_w, Pos.z * 2.0f + the_map->map_h );
            case UNIT_Y:
                if (v1 >= 0 && v1 < units.max_unit && (units.unit[v1].flags & 1) )
                    return (int)(units.unit[v1].Pos.y * 2.0f) << 16;
                else
                    return (int)(Pos.y * 2.0f) << 16;
            case UNIT_HEIGHT:
                if (v1 >= 0 && v1 < units.max_unit && (units.unit[v1].flags & 1) )
                    return (int)(units.unit[v1].model->top * 2.0f) << 16;
                else
                    return (int)(model->top * 2.0f) << 16;
            case XZ_ATAN:
                return (int)(atan2f( UNPACKX(v1) , UNPACKZ(v1) ) * RAD2TA - Angle.y * DEG2TA) + 32768;
            case XZ_HYPOT:
                return (int)hypotf( UNPACKX(v1), UNPACKZ(v1) ) << 16;
            case ATAN:
                return (int)(atan2f(v1,v2) * RAD2TA );
            case HYPOT:
                return (int)hypotf(v1,v2);
            case GROUND_HEIGHT:
                return (int)(the_map->get_unit_h(( UNPACKX(v1) - the_map->map_w) * 0.5f,( UNPACKZ(v1) - the_map->map_h) * 0.5f) * 2.0f) << 16;
            default:
                LOG_DEBUG(LOG_PREFIX_SCRIPT << "GET unknown constant " << type);
        }
        return 0;
    }

    void UNIT::script_set_value(int type, int v)
    {
        switch(type)
        {
            case ACTIVATION:
                if (v == 0 )
                    deactivate();
                else
                    activate();
                break;
            case YARD_OPEN:
                port[type] = v;
                if (!the_map->check_rect((((int)(Pos.x + the_map->map_w_d)) >> 3) - (unit_manager.unit_type[type_id]->FootprintX >> 1),
                                        (((int)(Pos.z + the_map->map_h_d)) >> 3) - (unit_manager.unit_type[type_id]->FootprintZ >> 1),
                                        unit_manager.unit_type[type_id]->FootprintX, unit_manager.unit_type[type_id]->FootprintZ, idx))
                    port[type] ^= 1;
                break;
            case BUGGER_OFF:
                port[type] = v;
                if (port[type])
                {
                    int px = ((int)(Pos.x) + the_map->map_w_d) >> 3;
                    int py = ((int)(Pos.z) + the_map->map_h_d) >> 3;
                    for(int y = py - (unit_manager.unit_type[type_id]->FootprintZ >> 1) ; y <= py + (unit_manager.unit_type[type_id]->FootprintZ >> 1) ; y++)
                    {
                        if (y >= 0 && y < (the_map->bloc_h << 1) - 1)
                        {
                            for(int x = px - (unit_manager.unit_type[type_id]->FootprintX >> 1) ; x <= px + (unit_manager.unit_type[type_id]->FootprintX >> 1) ; x++)
                            {
                                if (x >= 0 && x < (the_map->bloc_w << 1) - 1)
                                {
                                    if (the_map->map_data[y][x].unit_idx >= 0 && the_map->map_data[y][x].unit_idx != idx )
                                    {
                                        int cur_idx = the_map->map_data[y][x].unit_idx;
                                        if (units.unit[cur_idx].owner_id == owner_id && units.unit[cur_idx].build_percent_left == 0.0f && (units.unit[cur_idx].mission == NULL
                                            || units.unit[cur_idx].mission->mission != MISSION_MOVE))
                                        {
                                            units.unit[cur_idx].lock();
                                            Vector3D target = units.unit[cur_idx].Pos;
                                            target.z += 100.0f;
                                            units.unit[cur_idx].add_mission(MISSION_MOVE | MISSION_FLAG_AUTO, &target, true);
                                            units.unit[cur_idx].unlock();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                break;
            default:
                port[type] = v;
        }
    }

    void UNIT::script_attach_unit(int unit_id, int piece_id)
    {
        if (unit_id >= 0 && unit_id < units.max_unit && units.unit[ unit_id ].flags)
        {
            UNIT *target_unit = &(units.unit[unit_id]);
            target_unit->hidden = (piece_id < 0);
            bool already_in = false;
            if (target_unit->attached)
                for( int i = 0 ; i < nb_attached ; i++ )		// Check if this unit is already in
                {
                    if (attached_list[ i ] == unit_id)
                    {
                        already_in = true;
                        link_list[ i ] = piece_id;
                    }
                }
            if (!already_in)
            {
                link_list[nb_attached] = piece_id;
                attached_list[nb_attached++] = target_unit->idx;
            }
            target_unit->attached = true;
            if (!already_in)
                target_unit->clear_from_map();
        }
    }

    void UNIT::script_drop_unit(int unit_id)
    {
        if (unit_id >= 0 && unit_id < units.max_unit && units.unit[ unit_id ].flags)
        {
            UNIT *target_unit = &(units.unit[unit_id]);
            target_unit->attached = false;
            target_unit->hidden = false;
            nb_attached--;					// Remove the unit from the attached list
            for( int i = 0 ; i < nb_attached ; i++ )
            {
                if (attached_list[ i ] == unit_id )
                {
                    link_list[ i ] = link_list[ nb_attached ];
                    attached_list[ i ] = attached_list[ nb_attached ];
                    break;
                }
            }
            // Redraw the unit on presence map
            pMutex.unlock();
            target_unit->draw_on_map();
            pMutex.lock();
        }
    }

    bool UNIT::script_is_turning(int obj, int axis)
    {
        float a = data.axe[axis][obj].rot_angle;
        if ((data.axe[axis][obj].rot_speed != 0.0f || data.axe[axis][obj].rot_accel != 0.0f) && (a != 0.0f && data.axe[axis][obj].rot_limit))
            return false;
        else if (data.axe[axis][obj].rot_speed != data.axe[axis][obj].rot_target_speed && data.axe[axis][obj].rot_speed_limit)
            return false;
        data.axe[axis][obj].rot_speed = 0.0f;
        data.axe[axis][obj].rot_accel = 0.0f;
        return true;
    }

    bool UNIT::script_is_moving(int obj, int axis)
    {
        return !(data.axe[axis][obj].move_distance != 0.0f);
    }
}
