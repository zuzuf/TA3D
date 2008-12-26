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

# define MAX_UNIT_PER_PLAYER		2000		// 250 Unités par joueur maximum

# include "fbi.h"
# include "ingame/weapons/weapons.h"
# include "threads/cThread.h"
# include "misc/camera.h"
# include <list>
# include <vector>

# define UNPACKX(xz) ((sint16)((xz)>>16))
# define UNPACKZ(xz) ((sint16)((xz)&0xFFFF))
# define PACKXZ(x,z) ((((int)(x))<<16) | (((int)(z))&0xFFFF))


# define MISSION_FLAG_CAN_ATTACK		0x01
# define MISSION_FLAG_SEARCH_PATH	0x02
# define MISSION_FLAG_TARGET_WEAPON	0x04
# define MISSION_FLAG_COMMAND_FIRE	0x08
# define MISSION_FLAG_MOVE			0x10
# define MISSION_FLAG_REFRESH_PATH	0x20
# define MISSION_FLAG_DONT_STOP_MOVE	0x40
# define MISSION_FLAG_COMMAND_FIRED	0x80
# define MISSION_FLAG_TARGET_CHECKED	0x08			// For MISSION_CAPTURE to tell when data has been set to the time left before capture is finished
# define MISSION_FLAG_PAD_CHECKED	0x08			// For MISSION_GET_REPAIRED to tell when data has been set to the landing pad
# define MISSION_FLAG_BEING_REPAIRED	0x04			// For MISSION_GET_REPAIRED to tell the unit is being repaired



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

    void *create_unit(int type_id,int owner,Vector3D pos,MAP *map,bool sync=true,bool script=false);






    struct MISSION			// Structure pour stocker les ordres
    {
        uint8		mission;
        PATH_NODE	*path;		// Chemin emprunté par l'unité si besoin pour la mission
        Vector3D		target;
        MISSION 	*next;		// Mission suivante
        bool		step;		// Etape d'une mission
        float		time;		// Temps écoulé depuis la création de l'ordre
        int			data;		// Données de l'ordre
        byte		flags;		// Données supplémentaires
        float		last_d;		// Dernière distance enregistrée
        void		*p;			// Pointer to whatever we need
        uint32		target_ID;	// Identify a target unit
        int			move_data;	// Required data for the moving part of the order
        uint16		node;		// Tell which patrol node is this mission
    };

    class SCRIPT_ENV_STACK			// Pile pour la gestion des scripts
    {
    public:
        int					var[15];
        uint32				signal_mask;
        sint32				cur;
        SCRIPT_ENV_STACK	*next;

        inline void init()
        {
            for(uint8 i=0;i<15;i++)
                var[i]=0;
            cur=0;
            signal_mask=0;
            next=NULL;
        }

        inline SCRIPT_ENV_STACK()
        {
            init();
        }
    };

    class SCRIPT_ENV			// Classe pour la gestion de l'environnement des scripts
    {
    public:
        SCRIPT_STACK		*stack;			// Pile utilisée par les scripts
        SCRIPT_ENV_STACK	*env;			// Pile d'éxecution des scripts
        float				wait;
        bool				running;

        inline void init()
        {
            stack=NULL;
            env=NULL;
            wait=0.0f;
            running=false;
        }

        SCRIPT_ENV()
        {
            init();
        }

        inline void destroy()
        {
            while(stack) {
                SCRIPT_STACK *next=stack->next;
                delete stack;
                stack=next;
            }
            while(env) {
                SCRIPT_ENV_STACK *next=env->next;
                delete env;
                env=next;
            }
            init();
        }

        inline ~SCRIPT_ENV()
        {
            //		destroy();
        }

        inline void push(int v)
        {
            SCRIPT_STACK *new_stack=new SCRIPT_STACK;
            new_stack->next=stack;
            stack=new_stack;
            stack->val=v;
        }

        inline int pop()
        {
            if (stack==NULL)// Si la pile est vide, renvoie 0 et un message pour le débuggage
            {
                # ifdef DEBUG_MODE
                LOG_WARNING("COB VM: stack is empty !");
                # endif
                return 0;
            }
            int v=stack->val;
            SCRIPT_STACK *old=stack;
            stack=stack->next;
            delete old;
            return v;
        }
    };

#define WEAPON_FLAG_IDLE			0x0				// When IDLE the weapon can target a unit
#define WEAPON_FLAG_AIM				0x1				// The weapon is aiming
#define WEAPON_FLAG_SHOOT			0x2				// Fire!!
#define WEAPON_FLAG_WEAPON			0x4				// The weapon is targeting a weapon
#define WEAPON_FLAG_COMMAND_FIRE	0x8				// The unit didn't auto select this target, it's on user command

    class WEAPON_DATA
    {
    public:
        byte	state;
        uint16	burst;
        uint16	stock;
        float	delay;
        float	time;
        Vector3D	target_pos;
        void	*target;
        sint16	data;
        byte	flags;
        Vector3D	aim_dir;		// Vecteur de visée

        inline void init()
        {
            state = WEAPON_FLAG_IDLE;
            burst = 0;
            stock = 0;
            delay = 0.0f;
            target = NULL;
            target_pos.x = target_pos.y = target_pos.z = 0.0f;
            time = 0.0f;
            data = -1;
            aim_dir.x = aim_dir.y = aim_dir.z = 0.0f;
        }

        WEAPON_DATA()
        {
            init();
        }
    };

#define SCRIPT_QueryPrimary		0x00
#define SCRIPT_AimPrimary		0x01
#define SCRIPT_FirePrimary		0x02
#define SCRIPT_QuerySecondary	0x03
#define SCRIPT_AimSecondary		0x04
#define SCRIPT_FireSecondary	0x05
#define SCRIPT_QueryTertiary	0x06
#define SCRIPT_AimTertiary		0x07
#define SCRIPT_FireTertiary		0x08
#define SCRIPT_TargetCleared	0x09
#define SCRIPT_stopbuilding		0x0A
#define SCRIPT_stop				0x0B
#define SCRIPT_startbuilding	0x0C
#define SCRIPT_go				0x0D
#define SCRIPT_killed			0x0E
#define SCRIPT_StopMoving		0x0F
#define SCRIPT_Deactivate		0x10
#define SCRIPT_Activate			0x11
#define SCRIPT_create			0x12
#define SCRIPT_MotionControl	0x13
#define SCRIPT_startmoving		0x14
#define SCRIPT_MoveRate1		0x15
#define SCRIPT_MoveRate2		0x16
#define SCRIPT_MoveRate3		0x17
#define SCRIPT_RequestState		0x18
#define SCRIPT_TransportPickup	0x19
#define SCRIPT_TransportDrop	0x1A
#define SCRIPT_QueryTransport	0x1B
#define SCRIPT_BeginTransport	0x1C
#define SCRIPT_EndTransport		0x1D
#define SCRIPT_SetSpeed			0x1E
#define SCRIPT_SetDirection		0x1F
#define SCRIPT_SetMaxReloadTime	0x20
#define SCRIPT_QueryBuildInfo	0x21
#define SCRIPT_SweetSpot		0x22
#define SCRIPT_RockUnit			0x23
#define SCRIPT_QueryLandingPad	0x24
#define NB_SCRIPT				0x25

    class UNIT	: public ObjectSync	// Classe pour la gestion des unités	/ Class to store units's data
    {
    public:
        SCRIPT					*script;		// Scripts concernant l'unité
        std::vector<int>		*s_var;			// Tableau de variables pour les scripts
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
        SCRIPT_DATA				data;			// Données pour l'animation de l'unité par le script
        bool					drawing;
        sint16					*port;			// Ports
        MISSION					*mission;		// Orders given to the unit
        MISSION					*def_mission;	// Orders given to units built by this plant
        byte					nb_running;		// Nombre de scripts lancés en même temps
        std::vector< SCRIPT_ENV >	*script_env;	// Environnements des scripts
        byte					flags;			// Pour indiquer entre autres au gestionnaire d'unités si l'unité existe
        // 0	-> nothing
        // 1	-> the unit exists
        // 4	-> being killed
        // 64	-> landed (for planes)
        float					c_time;			// Compteur de temps entre 2 émissions de particules par une unité de construction
        bool					compute_coord;	// Indique s'il est nécessaire de recalculer les coordonnées du modèle 3d
        std::vector< short >			*script_val;	// Tableau de valeurs retournées par les scripts
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
        char					*script_idx;	// Index of scripts to prevent multiple search
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
        WEAPON_DATA				weapon[3];
        bool					was_moving;
        float					last_path_refresh;
        float					shadow_scale_dir;
        bool					hidden;				// Used when unit is attached to another one but is hidden (i.e. transport ship)
        bool					flying;
        bool					cloaked;			// Is the unit cloaked
        bool					cloaking;			// Cloaking the unit if enough energy

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
    public:
        uint32			sync_hash;
        uint32			*last_synctick;
        bool			local;
        bool			exploding;
        struct	sync	previous_sync;		// previous sync data
        sint32			nanolathe_target;
        bool			nanolathe_reverse;
        bool			nanolathe_feature;

    public:
        bool do_nothing()
        {
            return (mission==NULL || ((mission->mission==MISSION_STOP || mission->mission==MISSION_STANDBY || mission->mission==MISSION_VTOL_STANDBY) && mission->next==NULL)) && !port[ INBUILDSTANCE ];
        }

        bool do_nothing_ai()
        {
            return (mission==NULL || ((mission->mission==MISSION_STOP || mission->mission==MISSION_STANDBY || mission->mission==MISSION_VTOL_STANDBY || mission->mission==MISSION_MOVE) && mission->next==NULL)) && !port[ INBUILDSTANCE ];
        }

        float damage_modifier()
        {
            return port[ ARMORED ] ? unit_manager.unit_type[ type_id ].DamageModifier : 1.0f;
        }

        void draw_on_map();
        void clear_from_map();

        void draw_on_FOW(bool jamming = false);

        bool is_on_radar(byte p_mask);

        void start_mission_script(int mission_type);

        void next_mission();

        void clear_mission();

        void add_mission(int mission_type,Vector3D *target=NULL,bool step=false,int dat=0,void *pointer=NULL,PATH_NODE *path=NULL,byte m_flags=0,int move_data=0,int patrol_node=-1);

        void set_mission(int mission_type,Vector3D *target=NULL,bool step=false,int dat=0,bool stopit=true,void *pointer=NULL,PATH_NODE *path=NULL,byte m_flags=0,int move_data=0);

        void compute_model_coord();

        void raise_signal(uint32 signal);		// Tue les processus associés

        void init_alloc_data();

        void toggle_self_destruct();

        void lock_command();

        void unlock_command();

        void init(int unit_type= - 1, int owner = -1, bool full = false, bool basic = false);

        void clear_def_mission();

        void destroy(bool full = false);

        void draw(float t,Camera *cam,MAP *map, bool height_line=true);

        void draw_shadow(Camera *cam, const Vector3D& Dir, MAP *map);

        void draw_shadow_basic(Camera *cam, const Vector3D& Dir, MAP *map);

        int get_script_index(int id);

        int get_script_index(const char *script_name);	 // Cherche l'indice du script dont on fournit le nom

        int launch_script(int id,int nb_param=0,int *param=NULL,bool force=false);			// Start a script as a separate "thread" of the unit

        bool is_running(int script_index); // Is the script still running ?

        void run_script_function(MAP* map, int id, int nb_param = 0, int *param = NULL); // Launch and run the script, returning it's values to param if not NULL

        void kill_script(int script_index);	// Fait un peu de ménage

        void reset_script();

        const void play_sound(const String& key);

        const int run_script(const float &dt,const int &id,MAP *map, int max_code = MAX_CODE_PER_TICK);			// Interprète les scripts liés à l'unité

        const int move( const float dt,MAP *map, int *path_exec, const int key_frame = 0 );

        void show_orders( bool only_build_commands=false, bool def_orders=false );				// Dessine les ordres reçus

        void activate();

        void deactivate();

        int shoot(int target,Vector3D startpos,Vector3D Dir,int w_id,const Vector3D& target_pos);

        bool hit(Vector3D P,Vector3D Dir,Vector3D *hit_vec, float length = 100.0f);

        bool hit_fast(Vector3D P,Vector3D Dir,Vector3D* hit_vec, float length = 100.0f);

        void stop_moving();

        void explode();

    }; // class UNIT

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
    public cThread
    {
    public:
        typedef std::vector< std::list< uint16 > >  RepairPodsList;
    public:
        /*----------------------- Variables générales ----------------------------------------------*/
        uint16	nb_unit;		// Nombre d'unités
        uint16	max_unit;		// Nombre maximum d'unités stockables dans le tableau
        UNIT	*unit;			// Tableau contenant les références aux unités
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

        std::vector< std::list< uint16 > >	requests;		// Store all the request for pathfinder calls

    public:

        GLushort	*mini_idx;			// Array to draw the units on mini map
        float		*mini_pos;			// Position on mini map
        uint32		*mini_col;			// Colors of units

    private:
        uint32		InterfaceMsg(const lpcImsg msg);	// Manage signals sent through the interface to unit manager

    protected:
        int			Run();
        void		SignalExitThread();

    public:

        void set_wind_change();

        void init(bool register_interface = false);

        INGAME_UNITS();

        void destroy(bool delete_interface = true);

        ~INGAME_UNITS() {destroy(false);}

        void kill(int index,MAP *map,int prev,bool sync = true);			// Détruit une unité

        void draw(Camera *cam,MAP *map,bool underwater=false,bool limit=false,bool cullface = true,bool height_line=true);					// Dessine les unités visibles

        void draw_shadow(Camera *cam, const Vector3D& Dir,MAP *map,float alpha=0.5f);					// Dessine les ombres des unités visibles

        void draw_mini(float map_w,float map_h,int mini_w,int mini_h,SECTOR **map_data);				// Repère les unités sur la mini-carte

        void move(float dt,MAP *map=NULL,int key_frame=0,bool wind_change=false);

        int create(int type_id,int owner);

        bool select(Camera *cam,int sel_x[],int sel_y[]);

        int pick(Camera* cam,int sensibility=1);

        int pick_minimap();

        void give_order_move(int player_id,Vector3D target,bool set=true,byte flags=0);

        void give_order_patrol(int player_id,Vector3D target,bool set=true);

        void give_order_guard(int player_id,int target,bool set=true);

        void give_order_unload(int player_id,Vector3D target,bool set=true);

        void give_order_load(int player_id,int target,bool set=true);

        void give_order_build(int player_id,int unit_type_id, Vector3D target,bool set=true);

        void remove_order(int player_id, Vector3D target);

        void complete_menu(int index,bool hide_info = false, bool hide_bpic = false );

    }; // class INGAME_UNITS





    extern INGAME_UNITS units;

    bool can_be_built(const Vector3D& Pos, MAP *map, const int unit_type_id, const int player_id );

    bool can_be_there( const int px, const int py, MAP *map, const int unit_type_id, const int player_id, const int unit_id = -1 );

    bool can_be_there_ai( const int px, const int py, MAP *map, const int unit_type_id, const int player_id, const int unit_id = -1 );


} // namespace TA3D

#endif
