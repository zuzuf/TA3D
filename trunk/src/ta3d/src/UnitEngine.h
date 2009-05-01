/*  TA3D, a remake of Total Annihilation
	Copyright (C) 2005   Roland BROCHARD

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

/*----------------------------------------------------------\
  |                         UnitEngine.h                      |
  |    Contains the unit engine, which simulates units and    |
  | make them interact with each other.                       |
  |                                                           |
  \----------------------------------------------------------*/

#ifndef __UNITENGINE_CLASS
# define __UNITENGINE_CLASS

# include "fbi.h"
# include "ingame/weapons/weapons.h"
# include "threads/thread.h"
# include "misc/camera.h"
# include <list>
# include <vector>
# include "misc/recttest.h"
# include "scripts/unit.script.interface.h"
# include "engine/weapondata.h"
# include "engine/mission.h"
# include "engine/unit.h"



# define SIGNAL_ORDER_NONE		    0x0
# define SIGNAL_ORDER_MOVE          0x1
# define SIGNAL_ORDER_PATROL        0x2
# define SIGNAL_ORDER_GUARD	        0x3
# define SIGNAL_ORDER_ATTACK		0x4
# define SIGNAL_ORDER_RECLAM		0x5
# define SIGNAL_ORDER_STOP		    0x6
# define SIGNAL_ORDER_ONOFF		    0x7
# define SIGNAL_ORDER_LOAD		    0x8
# define SIGNAL_ORDER_UNLOAD		0x9
# define SIGNAL_ORDER_REPAIR		0xA
# define SIGNAL_ORDER_CAPTURE	    0xB
# define SIGNAL_ORDER_DGUN		    0xC



namespace TA3D
{
	extern int MAX_UNIT_PER_PLAYER;

	void *create_unit(int type_id, int owner, Vector3D pos, MAP *map, bool sync = true, bool script = false);


#define	ICON_UNKNOWN		0x0
#define	ICON_BUILDER		0x1
#define	ICON_TANK			0x2
#define	ICON_LANDUNIT		0x3
#define	ICON_DEFENSE		0x4
#define	ICON_ENERGY			0x5
#define	ICON_METAL			0x6
#define	ICON_WATERUNIT		0x7
#define	ICON_COMMANDER		0x8
#define ICON_SUBUNIT		0x9
#define ICON_AIRUNIT		0xA
#define ICON_FACTORY		0xB
#define ICON_KAMIKAZE		0xC


	class INGAME_UNITS :	public ObjectSync,			// Class to manage huge number of units during the game
	protected IInterface,				// It inherits from what we need to use threads
	public Thread
	{
		virtual const char *className() { return "INGAME_UNITS"; }
	public:
		typedef std::vector< std::list< uint16 > >  RepairPodsList;
	public:
		/*----------------------- Variables générales ----------------------------------------------*/
		uint16	nb_unit;		// Nombre d'unités
		uint16	max_unit;		// Nombre maximum d'unités stockables dans le tableau
		Unit	*unit;			// Tableau contenant les références aux unités
		uint16	index_list_size;
		uint16	*idx_list;
		uint16	*free_idx;
		uint16	free_index_size[10];
		RepairPodsList repair_pads;		// Lists of built repair pads

		/*----------------------- Variables réservées au joueur courant ----------------------------*/

		float	nb_attacked;
		float	nb_built;

		uint8	page;

		/*----------------------- Variables reserved to texture data -------------------------------*/

		GLuint	icons[13];

		/*----------------------- Variables reserved to precalculations ----------------------------*/

		float	exp_dt_1;
		float	exp_dt_2;
		float	exp_dt_4;
		float	g_dt;
		int		sound_min_ticks;

		/*----------------------- Variables reserved to thread management --------------------------*/

		bool	thread_running;
		bool	thread_ask_to_stop;
		bool	wind_change;
		MAP		*map;
		uint32	next_unit_ID;			// Used to make it unique
		uint32	current_tick;
		uint32	client_tick[10];
		uint32	client_speed[10];
		float	apparent_timefactor;
		uint32	last_tick[5];
		sint32	last_on;				// Indicate the unit index which was under the cursor (mini map orders)

		std::vector< uint16 >               visible_unit;   // A list to store visible units
		std::vector< std::list< uint16 > >	requests;		// Store all the request for pathfinder calls

	public:

		float		*mini_pos;			// Position on mini map
		uint32		*mini_col;			// Colors of units

	private:
		uint32		InterfaceMsg(const lpcImsg msg);	// Manage signals sent through the interface to unit manager

	protected:
		void	proc(void*);
		void	signalExitThread();

	public:

		void set_wind_change();

		void init(bool register_interface = false);

		INGAME_UNITS();

		void destroy(bool delete_interface = true);

		~INGAME_UNITS() {destroy(false);}

		void kill(int index,MAP *map,int prev,bool sync = true);			// Détruit une unité

		void draw(MAP *map, bool underwater = false, bool limit = false, bool cullface = true, bool height_line = true); // Dessine les unités visibles

		void draw_shadow(const Vector3D& Dir, MAP* map, float alpha = 0.5f); // Dessine les ombres des unités visibles

		void draw_mini(float map_w, float map_h, int mini_w, int mini_h, SECTOR** map_data); // Repère les unités sur la mini-carte

		void move(float dt, MAP* map = NULL, int key_frame = 0, bool wind_change = false);

		int create(int type_id,int owner);

		/*!
		** \brief Select all units from a user selection
		**
		** \param cam The camera
		** \param pos The user selection, from the mouse coordinates
		** /return True if at least one unit has been selected
		*/
		bool selectUnits(const RectTest &reigon);

		int pick(Camera& cam,int sensibility=1);

		int pick_minimap();

		void give_order_move(int player_id, const Vector3D& target, bool set = true, byte flags = 0);

		void give_order_patrol(int player_id, const Vector3D& target, bool set = true);

		void give_order_guard(int player_id, int target, bool set = true);

		void give_order_unload(int player_id, const Vector3D& target,bool set = true);

		void give_order_load(int player_id,int target,bool set = true);

		void give_order_build(int player_id, int unit_type_id, const Vector3D& target, bool set = true);

		bool remove_order(int player_id, const Vector3D& target);

		void complete_menu(int index, bool hide_info = false, bool hide_bpic = false);

	}; // class INGAME_UNITS





	extern INGAME_UNITS units;

	bool can_be_built(const Vector3D& Pos, MAP *map, const int unit_type_id, const int player_id );

	bool can_be_there( const int px, const int py, MAP *map, const int unit_type_id, const int player_id, const int unit_id = -1 );

	bool can_be_there_ai( const int px, const int py, MAP *map, const int unit_type_id, const int player_id, const int unit_id = -1, const bool leave_space = false );


} // namespace TA3D


#endif
