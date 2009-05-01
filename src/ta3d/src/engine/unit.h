#ifndef __TA3D_ENGINE_UNIT_H__
# define __TA3D_ENGINE_UNIT_H__

# include "../stdafx.h"
# include "../threads/thread.h"
# include "../misc/vector.h"
# include "../ai/pathfinding.h"
# include "../scripts/unit.script.interface.h"
# include "mission.h"
# include "weapondata.h"
# include "unit.defines.h"
# include "../fbi.h"


namespace TA3D
{



	class Unit	: public ObjectSync	// Classe pour la gestion des unités	/ Class to store units's data
	{
	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor
		Unit();
		//! Destructor
		virtual ~Unit();
		//@}


		//! functions called from scripts (COB/BOS and Lua) (see unit.script.func module in scripts)
		void script_explode(int obj, int explosion_type);
		void script_turn_object(int obj, int axis, float angle, float speed);
		void script_move_object(int obj, int axis, float pos, float speed);
		int script_get_value_from_port(int portID, int *param = NULL);
		void script_spin_object(int obj, int axis, float target_speed, float accel);
		void script_show_object(int obj);
		void script_hide_object(int obj);
		void script_cache(int obj);
		void script_dont_cache(int obj);
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

		float damage_modifier() const
		{return port[ARMORED] ? unit_manager.unit_type[type_id]->DamageModifier : 1.0f;}

		bool isEnemy(const int t);

		void draw_on_map();
		void clear_from_map();

		void draw_on_FOW(bool jamming = false);

		bool is_on_radar(byte p_mask);

		void start_mission_script(int mission_type);

		void next_mission();

		void clear_mission();

		void add_mission(int mission_type, const Vector3D* target = NULL, bool step = false, int dat = 0,
						 void* pointer = NULL, PATH_NODE* path = NULL, byte m_flags = 0,
						 int move_data = 0, int patrol_node = -1);

		void set_mission(int mission_type, const Vector3D* target = NULL, bool step = false, int dat = 0,
						 bool stopit = true, void* pointer = NULL, PATH_NODE* path = NULL, byte m_flags = 0,
						 int move_data = 0);

		void compute_model_coord();

		void init_alloc_data();

		void toggle_self_destruct();

		void lock_command();

		void unlock_command();

		void init(int unit_type= - 1, int owner = -1, bool full = false, bool basic = false);

		void clear_def_mission();

		void destroy(bool full = false);

		void draw(float t, MAP *map, bool height_line = true);

		void draw_shadow(const Vector3D& Dir, MAP *map);

		void draw_shadow_basic(const Vector3D& Dir, MAP *map);

		int launch_script(const int id, int nb_param = 0, int *param = NULL);			        // Start a script as a separate "thread" of the unit

		int run_script_function(const int id, int nb_param = 0, int *param = NULL); // Launch and run the script, returning it's values to param if not NULL

		void reset_script();

		void play_sound(const String& key);

		const int move( const float dt,MAP *map, int *path_exec, const int key_frame = 0 );

		void show_orders( bool only_build_commands=false, bool def_orders=false );				// Dessine les ordres reçus

		void activate();

		void deactivate();

		int shoot(int target,Vector3D startpos,Vector3D Dir,int w_id,const Vector3D& target_pos);

		bool hit(Vector3D P,Vector3D Dir,Vector3D *hit_vec, float length = 100.0f);

		bool hit_fast(Vector3D P,Vector3D Dir,Vector3D* hit_vec, float length = 100.0f);

		void stop_moving();

		void explode();

		inline bool do_nothing()
		{
			return (!mission || ((mission->mission == MISSION_STOP || mission->mission == MISSION_STANDBY || mission->mission == MISSION_VTOL_STANDBY) && mission->next == NULL)) && !port[INBUILDSTANCE];
		}

		inline bool do_nothing_ai()
		{
			return (!mission || ((mission->mission == MISSION_STOP || mission->mission == MISSION_STANDBY || mission->mission == MISSION_VTOL_STANDBY || mission->mission == MISSION_MOVE) && !mission->next)) && !port[INBUILDSTANCE];
		}

	public:
		UnitScriptInterface     *script;		// Scripts concernant l'unité
		MODEL					*model;			// Modèle représentant l'objet
		byte					owner_id;		// Numéro du propriétaire de l'unité
		short					type_id;		// Type d'unité
		float					hp;				// Points de vide restant à l'unité
		Vector3D				Pos;			// Vecteur position
		Vector3D				drawn_Pos;		// To prevent the shadow to be drawn where the unit will be on next frame
		Vector3D				V;				// Vitesse de l'unité
		Vector3D				Angle;			// Orientation dans l'espace
		Vector3D				drawn_Angle;	// Idem drawn_Pos
		Vector3D				V_Angle;		// Variation de l'orientation dans l'espace
		bool					sel;			// Unité sélectionnée?
		ANIMATION_DATA          data;			// Données pour l'animation de l'unité par le script
		bool					drawing;
		sint16					*port;			// Ports
		Mission					*mission;		// Orders given to the unit
		Mission					*def_mission;	// Orders given to units built by this plant
		byte					flags;			// Pour indiquer entre autres au gestionnaire d'unités si l'unité existe
		int                     kills;          // How many kills did this unit
		// 0	-> nothing
		// 1	-> the unit exists
		// 4	-> being killed
		// 64	-> landed (for planes)
		float					c_time;			// Compteur de temps entre 2 émissions de particules par une unité de construction
		bool					compute_coord;	// Indique s'il est nécessaire de recalculer les coordonnées du modèle 3d
		uint16					idx;			// Indice dans le tableau d'unité
		uint32					ID;				// the number of the unit (in total creation order) so we can identify it even if we move it :)
		float					h;				// Altitude (par rapport au sol)
		bool					visible;		// Indique si l'unité est visible / Tell if the unit is currently visible
		bool					on_radar;		// Radar drawing mode (icons)
		bool					on_mini_radar;	// On minimap radar
		short					groupe;			// Indique si l'unité fait partie d'un groupe
		bool					built;			// Indique si l'unité est en cours de construction (par une autre unité)
		bool					attacked;		// Indique si l'unité est attaquée
		float					planned_weapons;	// Armes en construction / all is in the name
		int						*memory;		// Pour se rappeler sur quelles armes on a déjà tiré
		byte					mem_size;
		bool					attached;
		short					*attached_list;
		short					*link_list;
		byte					nb_attached;
		bool					just_created;
		bool					first_move;
		int						severity;
		sint16					cur_px;
		sint16					cur_py;
		float					metal_prod;
		float					metal_cons;
		float					energy_prod;
		float					energy_cons;
		uint32					last_time_sound;	// Remember last time it played a sound, so we don't get a unit SHOUTING for a simple move
		float					cur_metal_prod;
		float					cur_metal_cons;
		float					cur_energy_prod;
		float					cur_energy_cons;
		uint32					ripple_timer;
		WeaponData::Vector 		weapon;
		float                   death_delay;
		bool					was_moving;
		float					last_path_refresh;
		float					shadow_scale_dir;
		bool					hidden;				// Used when unit is attached to another one but is hidden (i.e. transport ship)
		bool					flying;
		bool					cloaked;			// Is the unit cloaked
		bool					cloaking;			// Cloaking the unit if enough energy
		float                   paralyzed;

		// Following variables are used to control the drawing of the unit on the presence maps
		bool			drawn_open;			// Used to store the last state the unit was drawn on the presence map (opened or closed)
		bool			drawn_flying;
		sint32			drawn_x, drawn_y;
		bool			drawn;

		// Following variables are used to control the drawing of FOW data
		uint16			sight;
		uint16			radar_range;
		uint16			sonar_range;
		uint16			radar_jam_range;
		uint16			sonar_jam_range;
		sint16			old_px;
		sint16			old_py;

		Vector3D		move_target_computed;
		float			was_locked;

		float			self_destruct;		// Count down for self-destruction
		float			build_percent_left;
		float			metal_extracted;

		bool			requesting_pathfinder;
		uint16			pad1, pad2;			// Repair pads currently used
		float			pad_timer;			// Store last try to find a free landing pad

		bool			command_locked;		// Used for missions

		uint8			yardmap_timer;		// To redraw the unit on yardmap periodically
		uint8			death_timer;		// To remove dead units

		// Following variables are used to control the synchronization of data between game clients
		uint32			sync_hash;
		uint32			*last_synctick;
		bool			local;
		bool			exploding;
		struct	sync	previous_sync;		// previous sync data
		sint32			nanolathe_target;
		bool			nanolathe_reverse;
		bool			nanolathe_feature;

		// Following variables are used by the renderer
		bool            visibility_checked;

	private:
		void start_building(const Vector3D &dir);

	}; // class Unit



} // namespace TA3D


#endif // __TA3D_ENGINE_UNIT_H__
