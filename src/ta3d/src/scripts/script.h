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

/*----------------------------------------------------------------------\
  |                                 script.h                              |
  |      contient les classes nécessaires à la gestion des scripts de     |
  | controle du déroulement de la partie. Les scripts peuvent influencer  |
  | considérablement le déroulement de la partie en manipulant les unités |
  | les ressources mais aussi l'écran et déclenche les signaux de défaite |
  | et de victoire.                                                       |
  \----------------------------------------------------------------------*/

#ifndef CLASSE_SCRIPT
# define CLASSE_SCRIPT

# include <stdafx.h>
# include "lua.thread.h"
# include <threads/thread.h>
# include <misc/string.h>
# include <misc/tdf.h>
# include "draw.list.h"

namespace TA3D
{

	class LuaProgram : public LuaThread, public Thread
	{
	private:
		//  Variables to control execution flow
		int         amx,amy,amz;    // Cursor coordinates
		int         amb;            // Mouse button
		int         signal;         // Current signal (0 is none)

	public:
		DrawList    draw_list;      // Display commands list

		static bool passive;        // Passive mode, won't do anything like creating units, move units, etc... used to resync a multiplayer game

		void init();
		void destroy();

		LuaProgram();

		virtual ~LuaProgram()  {   destroy();  }

		virtual int run(float dt, bool alone = false);                  // Run the script
		int check();                        // Display DRAW_LIST commands and check if a signal was sent

	private:
		virtual void register_functions();

	protected:
		virtual void proc(void* param);
		virtual void signalExitThread();

	public:
		static LuaProgram	*inGame;
	};

	void generate_script_from_mission( QString Filename, TDFParser& ota_parser, int schema = 0 );

	int program_print(lua_State *L);
	int program_print_for(lua_State *L);
	int program_line(lua_State *L);
	int program_cls(lua_State *L);
	int program_cls_for(lua_State *L);
	int program_point(lua_State *L);
	int program_triangle(lua_State *L);
	int program_box(lua_State *L);
	int program_fillbox(lua_State *L);
	int program_circle(lua_State *L);
	int program_time(lua_State *L);
	int program_draw_image(lua_State *L);
	int program_draw_image_for(lua_State *L);
	int program_get_image_size(lua_State *L);
	int program_nb_players(lua_State *L);
	int program_get_unit_number_for_player(lua_State *L);
	int program_get_unit_owner(lua_State *L);
	int program_get_unit_number(lua_State *L);
	int program_get_max_unit_number(lua_State *L);
	int program_annihilated(lua_State *L);
	int program_has_unit(lua_State *L);
	int program_create_unit(lua_State *L);
	int program_change_unit_owner(lua_State *L);
	int program_local_player(lua_State *L);
	int program_map_w(lua_State *L);
	int program_map_h(lua_State *L);
	int program_player_side(lua_State *L);
	int program_unit_x(lua_State *L);
	int program_unit_y(lua_State *L);
	int program_unit_z(lua_State *L);
	int program_move_unit(lua_State *L);
	int program_kill_unit(lua_State *L);
	int program_kick_unit(lua_State *L);
	int program_play(lua_State *L);
	int program_play_for(lua_State *L);
	int program_set_cam_pos(lua_State *L);
	int program_clf(lua_State *L);
	int program_start_x(lua_State *L);
	int program_start_z(lua_State *L);
	int program_init_res(lua_State *L);
	int program_give_metal(lua_State *L);
	int program_give_energy(lua_State *L);
	int program_commander(lua_State *L);
	int program_attack(lua_State *L);
	int program_set_unit_health(lua_State *L);
	int program_get_unit_health(lua_State *L);
	int program_is_unit_of_type(lua_State *L);
	int program_add_build_mission(lua_State *L);
	int program_add_move_mission(lua_State *L);
	int program_add_attack_mission(lua_State *L);
	int program_add_patrol_mission(lua_State *L);
	int program_add_wait_mission(lua_State *L);
	int program_add_wait_attacked_mission(lua_State *L);
	int program_add_guard_mission(lua_State *L);
	int program_set_standing_orders(lua_State *L);
	int program_unlock_orders(lua_State *L);
	int program_lock_orders(lua_State *L);
	int program_nb_unit_of_type(lua_State *L);
	int program_create_feature(lua_State *L);
	int program_has_mobile_units(lua_State *L);
	int program_send_signal(lua_State *L);
	int program_allied(lua_State *L);
	int program_get_screen_size(lua_State *L);
} // namespace TA3D

#endif
