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
# include "misc/kdtree.h"
# include "gfx/texture.h"



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
	// Define the TKit structure required by the KDTree structure
	class UnitTKit
	{
	public:
		typedef Vector3D		Vec;
		typedef std::pair<const Unit*, Vec>		T;

		struct Comparator
		{
			const unsigned int D;

			inline Comparator(const unsigned int N) : D(N)	{}

			inline bool operator() (const T &i, const T &j) const
			{
				if (i.first == j.first)
					return false;
				switch(D)
				{
				case 0:
					return i.second.x < j.second.x;
				case 1:
					return i.second.y < j.second.y;
				case 2:
					return i.second.z < j.second.z;
				};
				return false;
			}
		};

		struct Predicate
		{
			const unsigned int D;
			const float f;

			inline Predicate(const Vec &v, const unsigned int N) : D(N), f(v[N])	{}

			inline bool operator() (const T &i) const
			{
				return i.second[D] < f;
			}
		};

	public:
		static inline const Vec &pos(const T &elt)	{	return elt.second;	}
		static inline float radius(const T &elt)
		{
			const Model* const model = elt.first->model;
			return model ? model->size2 : 0.0f;
		}
		static inline void getTopBottom(const std::vector<T>::const_iterator &begin, const std::vector<T>::const_iterator &end, Vec &top, Vec &bottom);
		static inline unsigned int getPrincipalDirection(const Vec &v)
		{
			const Vector3D m = TA3D::Math::Abs(v);
			if (m.x > m.y)
			{
				if (m.x > m.z)
					return 0U;
				return 2U;
			}
			if (m.y > m.z)
				return 1U;
			return 2U;
		}
	};

	inline void UnitTKit::getTopBottom(const std::vector<T>::const_iterator &begin, const std::vector<T>::const_iterator &end, Vec &top, Vec &bottom)
	{
		top = bottom = begin->second;
		for(std::vector<UnitTKit::T>::const_iterator i = begin ; i != end ; ++i)
		{
			top = Math::Max(top, i->second);
			bottom = Math::Min(bottom, i->second);
		}
	}

	extern int MAX_UNIT_PER_PLAYER;

	void *create_unit(int type_id, int owner, Vector3D pos, bool sync = true, bool script = false);


#define	ICON_UNKNOWN				0x0
#define	ICON_BUILDER				0x1
#define	ICON_GROUND_ASSAULT			0x2
#define	ICON_LANDUNIT				0x3
#define	ICON_DEFENSE				0x4
#define	ICON_ENERGY					0x5
#define	ICON_METAL					0x6
#define	ICON_WATERUNIT				0x7
#define	ICON_COMMANDER				0x8
#define ICON_SUBUNIT				0x9
#define ICON_AIRUNIT				0xA
#define ICON_FACTORY				0xB
#define ICON_KAMIKAZE				0xC


	class INGAME_UNITS :	public ObjectSync,			// Class to manage huge number of units during the game
		protected IInterface,				// It inherits from what we need to use threads
		public Thread
	{
	public:
		/*----------------------- Variables générales ----------------------------------------------*/
		uint32	nb_unit;		// Nombre d'unités
		uint32	max_unit;		// Nombre maximum d'unités stockables dans le tableau
		Unit	*unit;			// Tableau contenant les références aux unités
		uint32	index_list_size;
		uint16	*idx_list;
		uint16	*free_idx;
		uint16	free_index_size[10];

		/*----------------------- Variables réservées au joueur courant ----------------------------*/

		float	nb_attacked;
		float	nb_built;

		uint8	page;

		/*----------------------- Variables reserved to texture data -------------------------------*/

		Interfaces::GfxTexture	icons[13];

		/*----------------------- Variables reserved to precalculations ----------------------------*/

		float	exp_dt_1;
		float	exp_dt_2;
		float	exp_dt_4;
		float	g_dt;
		int		sound_min_ticks;

		/*----------------------- Variables reserved to thread management --------------------------*/

		volatile bool	thread_running;
		volatile bool	thread_ask_to_stop;
		bool	wind_change;
		MAP		*map;
		uint32	next_unit_ID;			// Used to make it unique
		uint32	current_tick;
		uint32	client_tick[10];
		uint32	client_speed[10];
		float	apparent_timefactor;
		uint32	last_tick[5];
		sint32	last_on;				// Indicate the unit index which was under the cursor (mini map orders)

		std::vector< uint32 >			visible_unit;   // A list to store visible units
		KDTree< UnitTKit::T, UnitTKit >	*kdTree[10];			// A KDTree filled with units to speed up target detection (one per player)
		KDTree< UnitTKit::T, UnitTKit >	*kdTreeFriends[10];		// A KDTree filled with units to speed up friend detection (one per player)
		KDTree< UnitTKit::T, UnitTKit >	*kdTreeRepairPads[10];	// A KDTree filled with units to speed up friend detection (one per player)
		bool	shootallMode;

		std::vector<Vector3D>	hbars_bkg;
		std::vector<Vector3D>	hbars;
		std::vector<uint32>		hbars_color;

	public:

		float		*mini_pos;			// Position on mini map
		uint32		*mini_col;			// Colors of units

	private:
		virtual uint32 InterfaceMsg(const uint32 MsgID, const QString &msg);	// Manage signals sent through the interface to unit manager

	protected:
		void	proc(void*);
		void	signalExitThread();

	public:

		void set_wind_change();

		void init(bool register_interface = false);

		INGAME_UNITS();

		void destroy(bool delete_interface = true);

		virtual ~INGAME_UNITS() {destroy(false);}

		void renderTick();

		void kill(int index,int prev,bool sync = true);			// Détruit une unité

		void draw(bool underwater = false, bool limit = false, bool cullface = true, bool height_line = true); // Dessine les unités visibles

		void drawHealthBars(); // Draw health bars / dessine les barres de vie

		void draw_shadow(float t, const Vector3D& Dir, float alpha = 0.5f); // Dessine les ombres des unités visibles

		void draw_mini(float map_w, float map_h, int mini_w, int mini_h); // Repère les unités sur la mini-carte

		void move(float dt, int key_frame = 0, bool wind_change = false);

		int create(int type_id,int owner);

		/*!
		** \brief Select all units from a user selection
		**
		** \param cam The camera
		** \param pos The user selection, from the mouse coordinates
		** /return True if at least one unit has been selected
		*/
		bool selectUnits(const RectTest &reigon);

		int pick(Camera& cam);

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

	bool can_be_built(const Vector3D& Pos, const int unit_type_id, const int player_id );

	bool can_be_there( const int px, const int py, const int unit_type_id, const int player_id, const int unit_id = -1 );

	bool can_be_there_ai( const int px, const int py, const int unit_type_id, const int player_id, const int unit_id = -1, const bool leave_space = false );


} // namespace TA3D


#endif
