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

#include <stdafx.h>
#include <ingame/players.h>
#include <gfx/fx.manager.h>
#include <UnitEngine.h>



namespace TA3D
{
    //! functions that are called from scripts (COB/BOS and Lua)
    void Unit::script_explode(int obj, int explosion_type)
    {
		data.data[obj].axe[0].pos = 0.0f;
		data.data[obj].axe[0].angle = 0.0f;
		data.data[obj].axe[1].pos = 0.0f;
		data.data[obj].axe[1].angle = 0.0f;
		data.data[obj].axe[2].pos = 0.0f;
		data.data[obj].axe[2].angle = 0.0f;
        if (visible) // Don't draw things which could tell the player there is something there
        {
            compute_model_coord();
			particle_engine.make_fire( Pos + data.data[obj].pos,1,10,45.0f);
			const int power = Math::Max(unit_manager.unit_type[type_id]->FootprintX, unit_manager.unit_type[type_id]->FootprintZ);
			const Vector3D P = Pos + data.data[obj].pos;
			fx_manager.addExplosion( P, V, power * 3, float(power) * 10.0f );
        }
		if (!(explosion_type & EXPLODE_BITMAPONLY))
		{
			data.data[obj].flag |= FLAG_EXPLODE;
			data.data[obj].explosion_flag    = (short)explosion_type;
			data.data[obj].axe[0].move_speed = (25.0f + float(Math::RandomTable() % 2501) * 0.01f) * (Math::RandomTable() & 1 ? 1.0f : -1.0f);
			data.data[obj].axe[0].rot_speed  = float(Math::RandomTable() % 7201) * 0.1f - 360.0f;
			data.data[obj].axe[1].move_speed = 25.0f + float(Math::RandomTable() % 2501) * 0.01f;
			data.data[obj].axe[1].rot_speed  = float(Math::RandomTable() % 7201) * 0.1f - 360.0f;
			data.data[obj].axe[2].move_speed = (25.0f + float(Math::RandomTable() % 2501) * 0.01f) * float(Math::RandomTable() & 1 ? 1.0f : -1.0f);
			data.data[obj].axe[2].rot_speed  = float(Math::RandomTable() % 7201) * 0.1f - 360.0f;
			data.explode = true;
			data.explode_time = 1.0f;
		}
    }


    void Unit::script_turn_object(int obj, int axis, float angle, float speed)
    {
        if (axis != 2)
        {
            angle = -angle;
            speed = -speed;
        }
		data.data[obj].axe[axis].reset_rot();
		data.data[obj].axe[axis].is_moving = true;
		data.is_moving = true;
		data.data[obj].axe[axis].rot_angle = -angle;
		data.data[obj].axe[axis].rot_accel = 0.0f;
		data.data[obj].axe[axis].rot_angle -= data.data[obj].axe[axis].angle;
		while(data.data[obj].axe[axis].rot_angle > 180.0f && !isNaN(data.data[obj].axe[axis].rot_angle))					// Fait le tour dans le sens le plus rapide
			data.data[obj].axe[axis].rot_angle -= 360.0f;
		while(data.data[obj].axe[axis].rot_angle < -180.0f && !isNaN(data.data[obj].axe[axis].rot_angle))					// Fait le tour dans le sens le plus rapide
			data.data[obj].axe[axis].rot_angle += 360.0f;
		if (data.data[obj].axe[axis].rot_angle > 0.0f)
			data.data[obj].axe[axis].rot_speed = fabsf(speed);
        else
			data.data[obj].axe[axis].rot_speed = -fabsf(speed);
		data.data[obj].axe[axis].rot_limit = true;
		data.data[obj].axe[axis].rot_speed_limit = false;
    }

    void Unit::script_move_object(int obj, int axis, float pos, float speed)
    {
        if (axis == 0)
            pos = -pos;

		data.data[obj].axe[axis].reset_move();
		data.data[obj].axe[axis].move_distance = pos - data.data[obj].axe[axis].pos;
		data.data[obj].axe[axis].is_moving = true;
		data.is_moving = true;
		if (data.data[obj].axe[axis].move_distance < 0.0f)
			data.data[obj].axe[axis].move_speed = -fabsf(speed * 0.5f);
        else
			data.data[obj].axe[axis].move_speed = fabsf(speed * 0.5f);
    }

    int Unit::script_get_value_from_port(int portID, int *param)
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
					const int v1 = param[1];
					const int v2 = param[0];
					return (int)(atanf((float)v1 / float(v2)) + 0.5f);
                }
                else
                    return 0;
            case HYPOT:
                if (param)
                {
					const int v1 = param[1];
					const int v2 = param[0];
                    return (int)(sqrtf((float)(v1 * v1 + v2 * v2)) + 0.5f);
                }
                else
                    return 0;
            case BUGGER_OFF:
				return the_map->check_rect((((int)(Pos.x + (float)the_map->map_w_d)) >> 3) - (unit_manager.unit_type[type_id]->FootprintX >> 1),
											(((int)(Pos.z + (float)the_map->map_h_d)) >> 3) - (unit_manager.unit_type[type_id]->FootprintZ >> 1),
                                            unit_manager.unit_type[type_id]->FootprintX, unit_manager.unit_type[type_id]->FootprintZ, idx) ? 0 : 1;
            case BUILD_PERCENT_LEFT:
				return (int)build_percent_left + ( (build_percent_left > (int)build_percent_left) ? 1 : 0 );
            case YARD_OPEN:
            case ACTIVATION:
            case HEALTH:
            case INBUILDSTANCE:
            case BUSY:
            case ARMORED:
            case STANDINGMOVEORDERS:			// A faire : ajouter le support des ordres de mouvement/feu
            case STANDINGFIREORDERS:
                return port[ portID ];
			default:
				LOG_DEBUG(LOG_PREFIX_SCRIPT << "unknown value in GET_VALUE_FROM_PORT: " << portID);
		};
		// const char *op[]={"INCONNU","ACTIVATION","STANDINGMOVEORDERS","STANDINGFIREORDERS","HEALTH","INBUILDSTANCE","BUSY","PIECE_XZ","PIECE_Y",
		//	"UNIT_XZ","UNIT_Y","UNIT_HEIGHT","XZ_ATAN","XZ_HYPOT","ATAN","HYPOT","GROUND_HEIGHT","BUILD_PERCENT_LEFT","YARD_OPEN",
		//	"BUGGER_OFF","ARMORED"};
		if (portID > 20)
			portID = 0;
		//LOG_DEBUG("GET_VALUE_FROM_PORT: opération non gérée : " << op[portID]);
        return 0;
    }

    void Unit::script_spin_object(int obj, int axis, float target_speed, float accel)
    {
        if (axis == 1)
        {
            target_speed = -target_speed;
            accel = -accel;
        }
		data.data[obj].axe[axis].reset_rot();
		data.data[obj].axe[axis].is_moving = true;
		data.is_moving = true;
		data.data[obj].axe[axis].rot_limit = false;
		data.data[obj].axe[axis].rot_speed_limit = true;
		data.data[obj].axe[axis].rot_target_speed = target_speed;
		if (!Yuni::Math::Zero(accel))
        {
			if (data.data[obj].axe[axis].rot_target_speed > data.data[obj].axe[axis].rot_speed)
				data.data[obj].axe[axis].rot_accel = fabsf(accel);
            else
				data.data[obj].axe[axis].rot_accel = -fabsf(accel);
        }
        else
        {
			data.data[obj].axe[axis].rot_accel = 0;
			data.data[obj].axe[axis].rot_speed = data.data[obj].axe[axis].rot_target_speed;
        }
    }

    void Unit::script_show_object(int obj)
    {
		data.data[obj].flag &= (~FLAG_HIDE);
    }

    void Unit::script_hide_object(int obj)
    {
		data.data[obj].flag |= FLAG_HIDE;
    }

    void Unit::script_dont_cache(int obj)
    {
		data.data[obj].flag |= FLAG_ANIMATED_TEXTURE;
    }

    void Unit::script_cache(int obj)
    {
		data.data[obj].flag &= (~FLAG_ANIMATED_TEXTURE);
    }

    void Unit::script_shade(int obj)
    {
		data.data[obj].flag &= (~FLAG_DONT_SHADE);
    }

    void Unit::script_dont_shade(int obj)
    {
		data.data[obj].flag |= FLAG_DONT_SHADE;
    }

    void Unit::script_emit_sfx(int smoke_type, int from_piece)
    {
        if (visible)
        {
            compute_model_coord();
			if (!Yuni::Math::Zero(data.data[from_piece].dir.x)
				|| !Yuni::Math::Zero(data.data[from_piece].dir.y)
				|| !Yuni::Math::Zero(data.data[from_piece].dir.z))
            {
				const Vector3D &dir = data.data[from_piece].dir;
                switch(smoke_type)
                {
                    case 0:
						particle_engine.emit_part(Pos + data.data[from_piece].pos, dir, fire, 1, 10.0f, 2.5f, 5.0f, true);
                        break;
                    case 2:
                    case 3:
						particle_engine.emit_part(Pos + data.data[from_piece].pos, dir, 0, 1, 10.0f, 10.0f, 10.0f, false, 0.3f);
                        break;
                    case 257:			// Fumée
                    case 258:
						particle_engine.emit_part(Pos + data.data[from_piece].pos, dir, 0, 1, 10.0f, 10.0f, 10.0f, true, 0.3f);
                        break;
                }
            }
            else
                switch(smoke_type)
                {
                    case 0:
						particle_engine.make_smoke(Pos + data.data[from_piece].pos, fire, 1, 0.0f, 0.0f, 0.0f, 0.5f);
                        break;
                    case 257:
                    case 258:
						particle_engine.make_smoke(Pos + data.data[from_piece].pos, 0, 1, 10.0f, -1.0f, 0.0f, 0.5f);
                        break;
                }
        }
    }

    void Unit::script_stop_spin(int obj, int axis, float speed)
    {
        if (axis != 2)
            speed = -speed;
		data.data[obj].axe[axis].reset_rot();
		data.data[obj].axe[axis].is_moving = true;
		data.is_moving = true;
		data.data[obj].axe[axis].rot_limit = false;
		data.data[obj].axe[axis].rot_speed_limit = true;
		data.data[obj].axe[axis].rot_target_speed = 0.0f;
		if (Yuni::Math::Zero(speed))
        {
			data.data[obj].axe[axis].rot_speed = 0.0f;
			data.data[obj].axe[axis].rot_accel = 0.0f;
        }
        else
        {
			if (data.data[obj].axe[axis].rot_speed > 0.0f)
				data.data[obj].axe[axis].rot_accel = -fabsf(speed);
            else
				data.data[obj].axe[axis].rot_accel = fabsf(speed);
        }
    }

    void Unit::script_move_piece_now(int obj, int axis, float pos)
    {
		data.data[obj].axe[axis].reset_move();
		data.data[obj].axe[axis].is_moving = true;
		data.is_moving = true;
        if (axis == 0)
			data.data[obj].axe[axis].pos = -pos;
        else
			data.data[obj].axe[axis].pos = pos;
    }

    void Unit::script_turn_piece_now(int obj, int axis, float angle)
    {
		data.data[obj].axe[axis].reset_rot();
		data.data[obj].axe[axis].is_moving = true;
		data.is_moving = true;
        if (axis != 2)
            angle = -angle;
		data.data[obj].axe[axis].angle = -angle;
    }

    int Unit::script_get(int type, int v1, int v2)
    {
        switch(type)
        {
			case CURRENT_SPEED:
				return int(V.norm());
			case HEADING:
				return int(Angle.y * DEG2TA);
			case VETERAN_LEVEL:
                return 100 * kills;
            case MIN_ID:
                return 0;
            case MAX_ID:
                return units.max_unit - 1;
            case MY_ID:
                return idx;
            case UNIT_TEAM:		// returns team(player ID in TA) of unit given with parameter
				if (v1 >= 0 && v1 < (int)units.max_unit && (units.unit[ v1 ].flags & 1) )
                    return units.unit[ v1 ].owner_id;
                else
                    return -1;
            case UNIT_BUILD_PERCENT_LEFT:		// basically BUILD_PERCENT_LEFT, but comes with a unit parameter
				if (v1 >= 0 && v1 < (int)units.max_unit && (units.unit[ v1 ].flags & 1) )
                    return (int)units.unit[ v1 ].build_percent_left + ( (units.unit[ v1 ].build_percent_left > (int)units.unit[ v1 ].build_percent_left) ? 1 : 0);
                else
                    return 0;
            case UNIT_ALLIED:		// is unit given with parameter allied to the unit of the current COB script. 0=not allied, not zero allied
                return !isEnemy( v1 );
            case UNIT_IS_ON_THIS_COMP:		// indicates if the 1st parameter(a unit ID) is local to this computer
				if (v1 >= 0 && v1 < (int)units.max_unit && (units.unit[ v1 ].flags & 1) )
                    return !(players.control[ units.unit[ v1 ].owner_id ] & PLAYER_CONTROL_FLAG_REMOTE);
                else
                    return 0;
            case BUILD_PERCENT_LEFT:
				return (int)build_percent_left + ( (build_percent_left > (int)build_percent_left) ? 1 : 0);
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
				return PACKXZ((data.data[v1].pos.x + Pos.x) * 2.0f + (float)the_map->map_w, (data.data[v1].pos.z + Pos.z) * 2.0f + (float)the_map->map_h);
            case PIECE_Y:
                compute_model_coord();
				return (int)((data.data[v1].pos.y + Pos.y) * 2.0f) << 16;
            case UNIT_XZ:
				if (v1 >= 0 && v1 < (int)units.max_unit && (units.unit[v1].flags & 1) )
					return PACKXZ( units.unit[v1].Pos.x * 2.0f + (float)the_map->map_w, units.unit[v1].Pos.z * 2.0f + (float)the_map->map_h );
                else
					return PACKXZ( Pos.x * 2.0f + (float)the_map->map_w, Pos.z * 2.0f + (float)the_map->map_h );
            case UNIT_Y:
				if (v1 >= 0 && v1 < (int)units.max_unit && (units.unit[v1].flags & 1) )
                    return (int)(units.unit[v1].Pos.y * 2.0f) << 16;
                else
                    return (int)(Pos.y * 2.0f) << 16;
            case UNIT_HEIGHT:
				if (v1 >= 0 && v1 < (int)units.max_unit && (units.unit[v1].flags & 1) )
                    return (int)(units.unit[v1].model->top * 2.0f) << 16;
                else
                    return (int)(model->top * 2.0f) << 16;
            case XZ_ATAN:
                return (int)(atan2f( UNPACKX(v1) , UNPACKZ(v1) ) * RAD2TA - Angle.y * DEG2TA) + 32768;
            case XZ_HYPOT:
                return (int)hypotf( UNPACKX(v1), UNPACKZ(v1) ) << 16;
            case ATAN:
				return (int)(atan2f(float(v1), float(v2)) * RAD2TA );
            case HYPOT:
				return (int)hypotf(float(v1), float(v2));
            case GROUND_HEIGHT:
				return (int)(the_map->get_unit_h(float(UNPACKX(v1) - the_map->map_w) * 0.5f, float(UNPACKZ(v1) - the_map->map_h) * 0.5f) * 2.0f) << 16;
            default:
                LOG_DEBUG(LOG_PREFIX_SCRIPT << "GET unknown constant " << type);
        }
        return 0;
    }

    void Unit::script_set_value(int type, int v)
    {
        switch(type)
        {
            case ACTIVATION:
                if (v == 0)
                    deactivate();
                else
                    activate();
                break;
            case YARD_OPEN:
				port[type] = sint16(v);
				if (!the_map->check_rect((((int)(Pos.x + (float)the_map->map_w_d)) >> 3) - (unit_manager.unit_type[type_id]->FootprintX >> 1),
										(((int)(Pos.z + (float)the_map->map_h_d)) >> 3) - (unit_manager.unit_type[type_id]->FootprintZ >> 1),
                                        unit_manager.unit_type[type_id]->FootprintX, unit_manager.unit_type[type_id]->FootprintZ, idx))
                    port[type] ^= 1;
                break;
            case BUGGER_OFF:
				port[type] = sint16(v);
                if (port[type])
                {
					const int px = ((int)(Pos.x) + the_map->map_w_d) >> 3;
					const int py = ((int)(Pos.z) + the_map->map_h_d) >> 3;
                    for(int y = py - (unit_manager.unit_type[type_id]->FootprintZ >> 1) ; y <= py + (unit_manager.unit_type[type_id]->FootprintZ >> 1) ; y++)
                    {
                        if (y >= 0 && y < (the_map->bloc_h << 1) - 1)
                        {
                            for(int x = px - (unit_manager.unit_type[type_id]->FootprintX >> 1) ; x <= px + (unit_manager.unit_type[type_id]->FootprintX >> 1) ; x++)
                            {
                                if (x >= 0 && x < (the_map->bloc_w << 1) - 1)
                                {
									const int cur_idx = the_map->map_data(x,y).unit_idx;
									if (cur_idx >= 0 && cur_idx < (int)units.max_unit && cur_idx != idx)
									{
										units.unit[cur_idx].lock();
										const int type = units.unit[cur_idx].type_id;
										const UnitType* const tType = type >= 0 ? unit_manager.unit_type[type] : NULL;
										if (units.unit[cur_idx].owner_id == owner_id
											&& tType != NULL
											&& tType->canmove
											&& tType->BMcode == 1
											&& Yuni::Math::Zero(units.unit[cur_idx].build_percent_left)
											&& (!units.unit[cur_idx].mission
												|| (units.unit[cur_idx].mission->mission() & 0xFF) != MISSION_MOVE))
										{
											Vector3D target = units.unit[cur_idx].Pos;
											target.z += 100.0f;
											units.unit[cur_idx].add_mission(MISSION_MOVE | MISSION_FLAG_AUTO, &target, true);
										}
										units.unit[cur_idx].unlock();
									}
								}
                            }
                        }
                    }
                }
                break;
			case ARMORED:
			case INBUILDSTANCE:
				port[type] = sint16(v);
				break;
			default:
				LOG_DEBUG(LOG_PREFIX_SCRIPT << "SET_VALUE unknown constant " << type);
				port[type] = sint16(v);
        }
    }

    void Unit::script_attach_unit(int unit_id, int piece_id)
    {
		if (unit_id >= 0 && unit_id < (int)units.max_unit && units.unit[ unit_id ].flags)
        {
            Unit *target_unit = &(units.unit[unit_id]);
            target_unit->hidden = (piece_id < 0);
            bool already_in = false;
            if (target_unit->attached)
                for( int i = 0 ; i < nb_attached ; i++ )		// Check if this unit is already in
                {
                    if (attached_list[ i ] == unit_id)
                    {
                        already_in = true;
						link_list[ i ] = short(piece_id);
                    }
                }
            if (!already_in)
            {
				link_list[nb_attached] = short(piece_id);
                attached_list[nb_attached++] = target_unit->idx;
            }
            target_unit->attached = true;
            if (!already_in)
                target_unit->clear_from_map();
        }
    }

    void Unit::script_drop_unit(int unit_id)
    {
		if (unit_id >= 0 && unit_id < (int)units.max_unit && units.unit[ unit_id ].flags)
        {
            Unit *target_unit = &(units.unit[unit_id]);
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


    bool Unit::script_is_turning(int obj, int axis)
    {
		const float a = data.data[obj].axe[axis].rot_angle;
		if ((!Yuni::Math::Zero(data.data[obj].axe[axis].rot_speed) || !Yuni::Math::Zero(data.data[obj].axe[axis].rot_accel))
			&& (!Yuni::Math::Zero(a) && data.data[obj].axe[axis].rot_limit))
            return true;
		else
		{
			if (!Yuni::Math::Equals(data.data[obj].axe[axis].rot_speed, data.data[obj].axe[axis].rot_target_speed)
				 && data.data[obj].axe[axis].rot_speed_limit)
            return true;
		}
		data.data[obj].axe[axis].rot_speed = 0.0f;
		data.data[obj].axe[axis].rot_accel = 0.0f;
        return false;
    }


    bool Unit::script_is_moving(int obj, int axis)
    {
		return !Yuni::Math::Zero(data.data[obj].axe[axis].move_distance);
    }
}
