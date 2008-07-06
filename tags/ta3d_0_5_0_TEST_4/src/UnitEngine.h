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

#define UNPACKX(xz) ((sint16)((xz)>>16))
#define UNPACKZ(xz) ((sint16)((xz)&0xFFFF))
#define PACKXZ(x,z) ((((int)(x))<<16) | (((int)(z))&0xFFFF))


#define MISSION_FLAG_CAN_ATTACK		0x01
#define MISSION_FLAG_SEARCH_PATH	0x02
#define MISSION_FLAG_TARGET_WEAPON	0x04
#define MISSION_FLAG_COMMAND_FIRE	0x08
#define MISSION_FLAG_MOVE			0x10
#define MISSION_FLAG_REFRESH_PATH	0x20
#define MISSION_FLAG_DONT_STOP_MOVE	0x40
#define MISSION_FLAG_COMMAND_FIRED	0x80
#define MISSION_FLAG_TARGET_CHECKED	0x08			// For MISSION_CAPTURE to tell when data has been set to the time left before capture is finished
#define MISSION_FLAG_PAD_CHECKED	0x08			// For MISSION_GET_REPAIRED to tell when data has been set to the landing pad
#define MISSION_FLAG_BEING_REPAIRED	0x04			// For MISSION_GET_REPAIRED to tell the unit is being repaired



namespace TA3D
{

    void *create_unit(int type_id,int owner,VECTOR pos,MAP *map,bool sync=true,bool script=false);






    struct MISSION			// Structure pour stocker les ordres
    {
        uint8		mission;
        PATH_NODE	*path;		// Chemin emprunté par l'unité si besoin pour la mission
        VECTOR		target;
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
            if(stack==NULL) {			// Si la pile est vide, renvoie 0 et un message pour le débuggage
#ifdef DEBUG_MODE
                Console->AddEntry("COB VM: stack is empty!\n");
#endif
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
        VECTOR	target_pos;
        void	*target;
        sint16	data;
        byte	flags;
        VECTOR	aim_dir;		// Vecteur de visée

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
        std::vector< int >			*s_var;			// Tableau de variables pour les scripts
        MODEL					*model;			// Modèle représentant l'objet
        byte					owner_id;		// Numéro du propriétaire de l'unité
        short					type_id;		// Type d'unité
        float					hp;				// Points de vide restant à l'unité
        VECTOR					Pos;			// Vecteur position
        VECTOR					drawn_Pos;		// To prevent the shadow to be drawn where the unit will be on next frame
        VECTOR					V;				// Vitesse de l'unité
        VECTOR					Angle;			// Orientation dans l'espace
        VECTOR					drawn_Angle;	// Idem drawn_Pos
        VECTOR					V_Angle;		// Variation de l'orientation dans l'espace
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

        VECTOR			move_target_computed;
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

        inline bool do_nothing()
        {
            return (mission==NULL || ((mission->mission==MISSION_STOP || mission->mission==MISSION_STANDBY || mission->mission==MISSION_VTOL_STANDBY) && mission->next==NULL)) && !port[ INBUILDSTANCE ];
        }

        inline bool do_nothing_ai()
        {
            return (mission==NULL || ((mission->mission==MISSION_STOP || mission->mission==MISSION_STANDBY || mission->mission==MISSION_VTOL_STANDBY || mission->mission==MISSION_MOVE) && mission->next==NULL)) && !port[ INBUILDSTANCE ];
        }

        inline float damage_modifier()
        {
            return port[ ARMORED ] ? unit_manager.unit_type[ type_id ].DamageModifier : 1.0f;
        }

        void draw_on_map();
        void clear_from_map();

        void draw_on_FOW( bool jamming = false );

        bool is_on_radar( byte p_mask );

        inline void start_mission_script(int mission_type)
        {
            if(script==NULL)	return;
            switch(mission_type)
            {
                case MISSION_ATTACK:
                    //			activate();
                    break;
                case MISSION_PATROL:
                case MISSION_MOVE:
                    break;
                case MISSION_BUILD_2:
                    break;
                case MISSION_RECLAIM:
                    break;
            };
            if( mission_type != MISSION_STOP ) {
                flags &= 191;
                //			flags |= 1;
            }
        }

        void next_mission();

        inline void clear_mission()
        {
            if(mission==NULL)	return;

            if( mission->mission == MISSION_GET_REPAIRED && mission->p ) {		// Don't forget to detach the planes from air repair pads!
                UNIT *target_unit = (UNIT*)(mission->p);
                target_unit->lock();
                if( target_unit->flags & 1 ) {
                    int piece_id = mission->data >= 0 ? mission->data : (-mission->data - 1);
                    if( target_unit->pad1 == piece_id )			// tell others we've left
                        target_unit->pad1 = 0xFFFF;
                    else target_unit->pad2 = 0xFFFF;
                }
                target_unit->unlock();
            }

            MISSION *old=mission;
            mission=mission->next;
            if(old->path)				// Détruit le chemin si nécessaire
                destroy_path(old->path);
            free(old);
        }

        void add_mission(int mission_type,VECTOR *target=NULL,bool step=false,int dat=0,void *pointer=NULL,PATH_NODE *path=NULL,byte m_flags=0,int move_data=0,int patrol_node=-1);

        void set_mission(int mission_type,VECTOR *target=NULL,bool step=false,int dat=0,bool stopit=true,void *pointer=NULL,PATH_NODE *path=NULL,byte m_flags=0,int move_data=0);

        inline void compute_model_coord()
        {
            if(!compute_coord)	return;
            if(model==NULL)	return;		// S'il n'y a pas de modèle associé, on quitte la fonction
            pMutex.lock();
            compute_coord=false;
            MATRIX_4x4 M;
            float scale=unit_manager.unit_type[type_id].Scale;
            M=RotateZ(Angle.z*DEG2RAD)*RotateY(Angle.y*DEG2RAD)*RotateX(Angle.x*DEG2RAD)*Scale(scale);			// Matrice pour le calcul des positions des éléments du modèle de l'unité
            model->compute_coord(&data,&M);
            pMutex.unlock();
        }

        inline void raise_signal(uint32 signal)		// Tue les processus associés
        {
            SCRIPT_ENV_STACK *tmp;
            for(int i=0;i<nb_running;i++) {
                tmp = (*script_env)[i].env;
                while(tmp) {
                    if(tmp->signal_mask==signal) {
                        tmp = (*script_env)[i].env;
                        while(tmp!=NULL) {
                            (*script_env)[i].env=tmp->next;
                            delete tmp;
                            tmp = (*script_env)[i].env;
                        }
                    }
                    if(tmp)
                        tmp=tmp->next;
                }
                if( (*script_env)[i].env==NULL)
                    (*script_env)[i].running=false;
            }
        }

        inline void init_alloc_data()
        {
            s_var = new std::vector< int >;
            port = new sint16[21];				// Ports
            script_env = new std::vector< SCRIPT_ENV >;	// Environnements des scripts
            script_val = new std::vector< short >;	// Tableau de valeurs retournées par les scripts
            memory = new int[10];				// Pour se rappeler sur quelles armes on a déjà tiré
            script_idx = new char[NB_SCRIPT];	// Index of scripts to prevent multiple search
            attached_list = new short[20];
            link_list = new short[20];
            last_synctick = new uint32[10];
        }

        inline void toggle_self_destruct()
        {
            if( self_destruct < 0.0f )
                self_destruct = unit_manager.unit_type[ type_id ].selfdestructcountdown;
            else
                self_destruct = -1.0f;
        }

        inline void lock_command()
        {
            pMutex.lock();
            command_locked = true;
            pMutex.unlock();
        }

        inline void unlock_command()
        {
            pMutex.lock();
            command_locked = false;
            pMutex.unlock();
        }

        inline void init(int unit_type=-1,int owner=-1,bool full=false,bool basic=false)
        {
            pMutex.lock();

            ID = 0;

            yardmap_timer = 1;
            death_timer = 0;

            drawing = false;

            local = true;		// Is local by default, set to remote by create_unit when needed

            nanolathe_target = -1;		// Used for remote units only
            nanolathe_reverse = false;
            nanolathe_feature = false;

            exploding = false;

            command_locked = false;

            pad1 = 0xFFFF; pad2 = 0xFFFF;
            pad_timer = 0.0f;

            requesting_pathfinder = false;

            was_locked = 0.0f;

            metal_extracted = 0.0f;

            on_mini_radar = false;
            move_target_computed.x = move_target_computed.y = move_target_computed.z = 0.0f;

            self_destruct = -1;		// Don't auto destruct!!

            drawn_open = drawn_flying = false;
            drawn_x = drawn_y = 0;
            drawn = true;

            old_px = old_py = -10000;

            flying = false;

            cloaked = false;
            cloaking = false;

            hidden = false;
            shadow_scale_dir = -1.0f;
            last_path_refresh = 0.0f;
            metal_prod = metal_cons = energy_prod = energy_cons = cur_metal_prod = cur_metal_cons = cur_energy_prod = cur_energy_cons = 0.0f;
            last_time_sound = msec_timer;
            ripple_timer = msec_timer;
            was_moving = false;
            cur_px=0;
            cur_py=0;
            sight = 0;
            radar_range = 0;
            sonar_range = 0;
            radar_jam_range = 0;
            sonar_jam_range = 0;
            severity=0;
            if(full)
                init_alloc_data();
            for(int i=0;i<NB_SCRIPT;i++)	script_idx[i]=-2;	// Not yet searched
            just_created=true;
            first_move=true;
            attached=false;
            nb_attached=0;
            mem_size=0;
            planned_weapons=0.0f;
            attacked=false;
            groupe=0;
            weapon[0].init();
            weapon[1].init();
            weapon[2].init();
            h=0.0f;
            compute_coord=true;
            c_time=0.0f;
            flags=1;
            sel=false;
            script=NULL;
            model=NULL;
            owner_id=owner;
            type_id=-1;
            hp=0.0f;
            V.x=V.y=V.z=0.0f;
            Pos=V;
            data.init();
            Angle.x=Angle.y=Angle.z=0.0f;
            V_Angle=Angle;
            nb_running=0;
            int i;
            script_env->clear();
            script_val->clear();
            for(i=0;i<21;i++)
                port[i]=0;
            s_var->clear();
            if(unit_type<0 || unit_type>=unit_manager.nb_unit)
                unit_type=-1;
            port[ACTIVATION]=0;
            mission=NULL;
            def_mission=NULL;
            port[BUILD_PERCENT_LEFT]=0;
            build_percent_left=0.0f;
            memset( last_synctick, 0, 40 );
            if(unit_type!=-1)
            {
                if( !basic )
                {
                    pMutex.unlock();
                    set_mission(MISSION_STANDBY);
                    pMutex.lock();
                }
                type_id=unit_type;
                model=unit_manager.unit_type[type_id].model;
                hp=unit_manager.unit_type[type_id].MaxDamage;
                script=unit_manager.unit_type[type_id].script;
                port[STANDINGMOVEORDERS]=unit_manager.unit_type[type_id].StandingMoveOrder;
                port[STANDINGFIREORDERS]=unit_manager.unit_type[type_id].StandingFireOrder;
                if( !basic )
                {
                    pMutex.unlock();
                    set_mission(unit_manager.unit_type[type_id].DefaultMissionType);
                    pMutex.lock();
                }
                if(script)
                {
                    data.load(script->nb_piece);
                    launch_script(get_script_index(SCRIPT_create));
                }
            }
            pMutex.unlock();
        }

        inline void clear_def_mission()
        {
            while( def_mission ) {
                MISSION *old = def_mission;
                def_mission = def_mission->next;
                if(old->path)				// Détruit le chemin si nécessaire
                    destroy_path(old->path);
                free(old);
            }
        }

        inline void destroy(bool full=false)
        {
            while( drawing )	rest(0);
            pMutex.lock();
            ID = 0;
            for(int i=0;i<nb_running;i++)
                (*script_env)[i].destroy();
            while(mission) clear_mission();
            clear_def_mission();
            init();
            flags=0;
            groupe=0;
            pMutex.unlock();
            if(full)
            {
                delete	 s_var;			// Tableau de variables pour les scripts
                delete[] port;			// Ports
                delete	 script_env;	// Environnements des scripts
                delete	 script_val;	// Tableau de valeurs retournées par les scripts
                delete[] memory;	// Pour se rappeler sur quelles armes on a déjà tiré
                delete[] script_idx;	// Index of scripts to prevent multiple search
                delete[] attached_list;
                delete[] link_list;
                delete[] last_synctick;
            }
        }

        void draw(float t,Camera *cam,MAP *map, bool height_line=true);

        void draw_shadow(Camera *cam,VECTOR Dir,MAP *map);

        void draw_shadow_basic(Camera *cam,VECTOR Dir,MAP *map);

        inline int get_script_index(int id)
        {
            if(script_idx[id]!=-2)	return script_idx[id];
            const char *script_name[]= {	"QueryPrimary","AimPrimary","FirePrimary",
                "QuerySecondary","AimSecondary","FireSecondary",
                "QueryTertiary","AimTertiary","FireTertiary",
                "TargetCleared","stopbuilding","stop",
                "startbuilding","go","killed",
                "StopMoving","Deactivate","Activate",
                "create","MotionControl","startmoving",
                "MoveRate1","MoveRate2","MoveRate3",
                "RequestState","TransportPickup","TransportDrop",
                "QueryTransport","BeginTransport","EndTransport",
                "SetSpeed","SetDirection","SetMaxReloadTime",
                "QueryBuildInfo","SweetSpot","RockUnit",
                "QueryLandingPad"};
            script_idx[id]=get_script_index(script_name[id]);
            return script_idx[id];
        }

        inline int get_script_index(const char *script_name)			// Cherche l'indice du script dont on fournit le nom
        {
            if(script)
                for(int i=0;i<script->nb_script;i++)
                    if(strcasecmp(script->name[i],script_name)==0)
                        return i;
            return -1;
        }

        int launch_script(int id,int nb_param=0,int *param=NULL,bool force=false);			// Start a script as a separate "thread" of the unit

        inline bool is_running(int script_index)								// Is the script still running ?
        {
            if(script==NULL)	return false;
            if(script_index<0 || script_index>=script->nb_script)	return false;
            for(int i=0;i<nb_running;i++)
                if((*script_env)[i].running && (*script_env)[i].env!=NULL) {
                    SCRIPT_ENV_STACK *current=(*script_env)[i].env;
                    while(current) {
                        if((current->cur&0xFF)==script_index)
                            return true;
                        current=current->next;
                    }
                }
            return false;
        }

        inline void run_script_function( MAP *map, int id, int nb_param=0, int *param=NULL )	// Launch and run the script, returning it's values to param if not NULL
        {
            pMutex.lock();
            int script_idx = launch_script( id, nb_param, param );
            if( script_idx >= 0 ) {
                float dt = 1.0f / TICKS_PER_SEC;
                for(uint16 n = 0 ; n < 10000 && (*script_env)[ script_idx ].running && (*script_env)[ script_idx ].env != NULL ; n++ ) {
                    if( nb_param > 0 && param != NULL )
                        for( int i = 0 ; i < nb_param ; i++ )
                            param[i] = (*script_env)[ script_idx ].env->var[ i ];
                    if( run_script( dt, script_idx, map, 1 ) )	break;
                }
                int e=0;
                for(int i=0;i+e<nb_running;) {				// Do some cleaning so we don't use all the env table with unused data
                    if((*script_env)[i+e].running) {
                        (*script_env)[i]=(*script_env)[i+e];
                        i++;
                    }
                    else {
                        (*script_env)[i+e].destroy();
                        e++;
                    }
                }
                nb_running-=e;
            }
            pMutex.unlock();
        }

        inline void kill_script(int script_index)		// Fait un peu de ménage
        {
            if(script==NULL)	return;
            if(script_index<0 || script_index>=script->nb_script)	return;
            pMutex.lock();
            for(int i=0;i<nb_running;i++)
                if((*script_env)[i].running && (*script_env)[i].env!=NULL) {
                    SCRIPT_ENV_STACK *current=(*script_env)[i].env;
                    while(current) {
                        if((current->cur&0xFF)==script_index) {		// Tue le script trouvé
                            current=(*script_env)[i].env;
                            (*script_env)[i].running=false;
                            while(current) {
                                (*script_env)[i].env=current->next;
                                delete current;
                                current=(*script_env)[i].env;
                            }
                            break;
                        }
                        current=current->next;
                    }
                }
            int e=0;
            for(int i=0;i+e<nb_running;) {				// Efface les scripts qui se sont arrêtés
                if((*script_env)[i+e].running) {
                    (*script_env)[i]=(*script_env)[i+e];
                    i++;
                }
                else {
                    (*script_env)[i+e].destroy();
                    e++;
                }
            }
            nb_running-=e;
            pMutex.unlock();
        }

        inline void reset_script()
        {
            pMutex.lock();
            for(int i=0;i<nb_running;i++)
                (*script_env)[i].destroy();
            nb_running=0;
            pMutex.unlock();
        }

        const void play_sound( const String &key );

        const int run_script(const float &dt,const int &id,MAP *map, int max_code = MAX_CODE_PER_TICK);			// Interprète les scripts liés à l'unité

        const int move( const float dt,MAP *map, int *path_exec, const int key_frame = 0 );

        void show_orders( bool only_build_commands=false, bool def_orders=false );				// Dessine les ordres reçus

        inline void activate()
        {
            pMutex.lock();
            if( port[ACTIVATION] == 0 ) {
                play_sound( "activate" );
                launch_script(get_script_index(SCRIPT_Activate));
                port[ACTIVATION] = 1;
            }
            pMutex.unlock();
        }

        inline void deactivate()
        {
            pMutex.lock();
            if( port[ACTIVATION] != 0 ) {
                play_sound( "deactivate" );
                launch_script(get_script_index(SCRIPT_Deactivate));
                port[ACTIVATION] = 0;
            }
            pMutex.unlock();
        }

        int shoot(int target,VECTOR startpos,VECTOR Dir,int w_id,const VECTOR &target_pos);

        bool hit(VECTOR P,VECTOR Dir,VECTOR *hit_vec, float length = 100.0f);

        bool hit_fast(VECTOR P,VECTOR Dir,VECTOR *hit_vec, float length = 100.0f);

        inline void stop_moving()
        {
            if( mission->flags & MISSION_FLAG_MOVE ) {
                mission->flags &= ~MISSION_FLAG_MOVE;
                if( mission->path ) {
                    destroy_path( mission->path );
                    mission->path = NULL;
                    V.x = V.y = V.z = 0.0f;
                }
                if( !( unit_manager.unit_type[ type_id ].canfly && nb_attached > 0 ) )		// Once charged with units the Atlas cannot land
                    launch_script(get_script_index(SCRIPT_StopMoving));
                else
                    was_moving = false;
                if( !(mission->flags & MISSION_FLAG_DONT_STOP_MOVE) )
                    V.x = V.y = V.z = 0.0f;		// Stop unit's movement
            }
        }

        void explode();
    };

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
        uint32		InterfaceMsg( const lpcImsg msg );	// Manage signals sent through the interface to unit manager

    protected:
        int			Run();
        void		SignalExitThread();

    public:

        inline void set_wind_change()
        {
            pMutex.lock();
            wind_change = true;
            pMutex.unlock();
        }

        inline void init( bool register_interface = false )
        {
            pMutex.lock();

            next_unit_ID = 1;
            mini_idx = NULL;
            mini_pos = NULL;
            mini_col = NULL;
            requests.clear();
            repair_pads.clear();
            repair_pads.resize( 10 );

            last_on = -1;
            current_tick = 0;
            last_tick[0]=0;
            last_tick[1]=0;
            last_tick[2]=0;
            last_tick[3]=0;
            last_tick[4]=0;
            apparent_timefactor = 1.0f;
            thread_running = false;
            thread_ask_to_stop = false;

            if( register_interface ) {
                InitInterface();		// Initialization of the interface

                icons[ ICON_UNKNOWN ] = gfx->load_texture( "gfx/tactical icons/unknown.tga" );
                icons[ ICON_BUILDER ] = gfx->load_texture( "gfx/tactical icons/builder.tga" );
                icons[ ICON_TANK ] = gfx->load_texture( "gfx/tactical icons/tank.tga" );
                icons[ ICON_LANDUNIT ] = gfx->load_texture( "gfx/tactical icons/landunit.tga" );
                icons[ ICON_DEFENSE ] = gfx->load_texture( "gfx/tactical icons/defense.tga" );
                icons[ ICON_ENERGY ] = gfx->load_texture( "gfx/tactical icons/energy.tga" );
                icons[ ICON_METAL ] = gfx->load_texture( "gfx/tactical icons/metal.tga" );
                icons[ ICON_WATERUNIT ] = gfx->load_texture( "gfx/tactical icons/waterunit.tga" );
                icons[ ICON_COMMANDER ] = gfx->load_texture( "gfx/tactical icons/commander.tga" );
                icons[ ICON_SUBUNIT ] = gfx->load_texture( "gfx/tactical icons/subunit.tga" );
                icons[ ICON_AIRUNIT ] = gfx->load_texture( "gfx/tactical icons/airunit.tga" );
                icons[ ICON_FACTORY ] = gfx->load_texture( "gfx/tactical icons/factory.tga" );
                icons[ ICON_KAMIKAZE ] = gfx->load_texture( "gfx/tactical icons/kamikaze.tga" );
            }

            sound_min_ticks = 500;
            index_list_size=0;
            for(int i=0;i<10;i++)	free_index_size[i]=0;
            idx_list=free_idx=NULL;
            page=0;
            nb_unit=0;
            unit=NULL;
            max_unit=0;
            nb_attacked=0.0f;
            nb_built=0.0f;
            exp_dt_1=0.0f;
            exp_dt_2=0.0f;
            exp_dt_4=0.0f;
            g_dt=0.0f;

            pMutex.unlock();
        }

        inline INGAME_UNITS() : repair_pads(), requests()
        {
            InitThread();
            init();
        }

        inline void destroy( bool delete_interface = true )
        {
            pMutex.lock();

            if( delete_interface ) {
                for( int i = 0 ; i < 13 ; i++ )
                    gfx->destroy_texture( icons[ i ] );

                DeleteInterface();			// Shut down the interface
            }

            if( mini_idx )			delete[] mini_idx;
            if( mini_pos )			delete[] mini_pos;
            if( mini_col )			delete[] mini_col;

            if(idx_list)			delete[] idx_list;
            if(free_idx)			delete[] free_idx;
            if(max_unit>0 && unit)			// Destroy all units
                for(int i=0;i<max_unit;i++)
                    unit[i].destroy(true);
            if(unit)
                delete[] unit;
//                free(unit);
            pMutex.unlock();

            init();
        }

        ~INGAME_UNITS()
        {
            destroy( false );
        }

        void kill(int index,MAP *map,int prev,bool sync = true);			// Détruit une unité

        void draw(Camera *cam,MAP *map,bool underwater=false,bool limit=false,bool cullface = true,bool height_line=true);					// Dessine les unités visibles

        void draw_shadow(Camera *cam,VECTOR Dir,MAP *map,float alpha=0.5f);					// Dessine les ombres des unités visibles

        void draw_mini(float map_w,float map_h,int mini_w,int mini_h,SECTOR **map_data);				// Repère les unités sur la mini-carte

        void move(float dt,MAP *map=NULL,int key_frame=0,bool wind_change=false);

        int create(int type_id,int owner);

        bool select(Camera *cam,int sel_x[],int sel_y[]);

        int pick(Camera* cam,int sensibility=1);

        int pick_minimap();

        inline void give_order_move(int player_id,VECTOR target,bool set=true,byte flags=0)
        {
            pMutex.lock();

            for(uint16 e=0;e<index_list_size;e++) {
                uint16 i = idx_list[e];
                if( (unit[i].flags & 1) && unit[i].owner_id==player_id && unit[i].sel && unit[i].build_percent_left == 0.0f && unit_manager.unit_type[unit[i].type_id].canmove) {
                    if(set)
                        unit[i].set_mission(MISSION_MOVE,&target,false,0,true,NULL,NULL,flags);
                    else
                        unit[i].add_mission(MISSION_MOVE,&target,false,0,NULL,NULL,flags);
                    if( unit_manager.unit_type[unit[i].type_id].BMcode && set )
                        unit[i].play_sound( "ok1" );
                }
            }
            pMutex.unlock();
        }

        inline void give_order_patrol(int player_id,VECTOR target,bool set=true)
        {
            pMutex.lock();
            for(uint16 e=0;e<index_list_size;e++) {
                uint16 i = idx_list[e];
                if( (unit[i].flags & 1) && unit[i].owner_id==player_id && unit[i].sel && unit[i].build_percent_left ==0.0f && unit_manager.unit_type[unit[i].type_id].canpatrol) {
                    if(set)
                        unit[i].set_mission(MISSION_PATROL,&target,false,0,true,NULL,NULL);
                    else
                        unit[i].add_mission(MISSION_PATROL,&target,false,0,NULL,NULL);
                    if( unit_manager.unit_type[unit[i].type_id].BMcode && set )
                        unit[i].play_sound( "ok1" );
                }
            }
            pMutex.unlock();
        }

        inline void give_order_guard(int player_id,int target,bool set=true)
        {
            pMutex.lock();
            for(uint16 e=0;e<index_list_size;e++) {
                uint16 i = idx_list[e];
                if( (unit[i].flags & 1) && unit[i].owner_id==player_id && unit[i].sel && unit[i].build_percent_left ==0.0f && unit_manager.unit_type[unit[i].type_id].canguard) {
                    if(set)
                        unit[i].set_mission(MISSION_GUARD,&unit[target].Pos,false,0,true,&(unit[target]),NULL);
                    else
                        unit[i].add_mission(MISSION_GUARD,&unit[target].Pos,false,0,&(unit[target]),NULL);
                    if( unit_manager.unit_type[unit[i].type_id].BMcode && set )
                        unit[i].play_sound( "ok1" );
                }
            }
            pMutex.unlock();
        }

        inline void give_order_unload(int player_id,VECTOR target,bool set=true)
        {
            pMutex.lock();
            for(uint16 e=0;e<index_list_size;e++) {
                uint16 i = idx_list[e];
                if( (unit[i].flags & 1) && unit[i].owner_id==player_id && unit[i].sel && unit[i].build_percent_left == 0.0f && unit_manager.unit_type[unit[i].type_id].canload
                    && unit_manager.unit_type[unit[i].type_id].BMcode && unit[i].nb_attached > 0 ) {
                    if(set)
                        unit[i].set_mission(MISSION_UNLOAD,&target,false,0,true,NULL,NULL);
                    else
                        unit[i].add_mission(MISSION_UNLOAD,&target,false,0,NULL,NULL);
                    if( set )
                        unit[i].play_sound( "ok1" );
                }
            }
            pMutex.unlock();
        }

        inline void give_order_load(int player_id,int target,bool set=true)
        {
            pMutex.lock();
            if(unit[target].flags==0 || !unit_manager.unit_type[unit[target].type_id].canmove)	{	pMutex.unlock();	return;		}
            switch(unit_manager.unit_type[unit[target].type_id].TEDclass)
            {
                case CLASS_UNDEF:
                case CLASS_WATER:
                case CLASS_SHIP:
                case CLASS_PLANT:
                case CLASS_SPECIAL:
                case CLASS_FORT:
                    pMutex.unlock();
                    return;
                    break;
            };
            for(uint16 e=0;e<index_list_size;e++) {
                uint16 i = idx_list[e];
                if( (unit[i].flags & 1) && unit[i].owner_id==player_id && unit[i].sel && unit[i].build_percent_left == 0.0f && unit_manager.unit_type[unit[i].type_id].canload
                    && unit_manager.unit_type[unit[i].type_id].BMcode) {
                    if(set)
                        unit[i].set_mission(MISSION_LOAD,&unit[target].Pos,false,0,true,&(unit[target]),NULL);
                    else
                        unit[i].add_mission(MISSION_LOAD,&unit[target].Pos,false,0,&(unit[target]),NULL);
                    if( set )
                        unit[i].play_sound( "ok1" );
                }
            }
            pMutex.unlock();
        }

        inline void give_order_build(int player_id,int unit_type_id,VECTOR target,bool set=true)
        {
            if( unit_type_id < 0 )	return;

            target.x=((int)(target.x)+map->map_w_d)>>3;
            target.z=((int)(target.z)+map->map_h_d)>>3;
            target.y=max(map->get_max_rect_h((int)target.x,(int)target.z, unit_manager.unit_type[ unit_type_id ].FootprintX, unit_manager.unit_type[ unit_type_id ].FootprintZ ),map->sealvl);
            target.x=target.x*8.0f-map->map_w_d;
            target.z=target.z*8.0f-map->map_h_d;

            pMutex.lock();
            for(uint16 e=0;e<index_list_size;e++)
            {
                uint16 i = idx_list[e];
                if( (unit[i].flags & 1) && unit[i].owner_id==player_id && unit[i].sel && unit[i].build_percent_left == 0.0f && unit_manager.unit_type[unit[i].type_id].Builder) {
                    if(set)
                        unit[i].set_mission(MISSION_BUILD,&target,false,unit_type_id);
                    else
                        unit[i].add_mission(MISSION_BUILD,&target,false,unit_type_id);
                }
            }
            pMutex.unlock();
        }

        void remove_order(int player_id,VECTOR target);

#define SIGNAL_ORDER_NONE		0x0
#define SIGNAL_ORDER_MOVE		0x1
#define SIGNAL_ORDER_PATROL		0x2
#define SIGNAL_ORDER_GUARD		0x3
#define SIGNAL_ORDER_ATTACK		0x4
#define SIGNAL_ORDER_RECLAM		0x5
#define SIGNAL_ORDER_STOP		0x6
#define SIGNAL_ORDER_ONOFF		0x7
#define SIGNAL_ORDER_LOAD		0x8
#define SIGNAL_ORDER_UNLOAD		0x9
#define SIGNAL_ORDER_REPAIR		0xA
#define SIGNAL_ORDER_CAPTURE	0xB
#define SIGNAL_ORDER_DGUN		0xC

        void complete_menu(int index,bool hide_info = false, bool hide_bpic = false );
    };

    extern INGAME_UNITS units;

    bool can_be_built( const VECTOR Pos, MAP *map, const int unit_type_id, const int player_id );

    bool can_be_there( const int px, const int py, MAP *map, const int unit_type_id, const int player_id, const int unit_id = -1 );

    bool can_be_there_ai( const int px, const int py, MAP *map, const int unit_type_id, const int player_id, const int unit_id = -1 );


} // namespace TA3D

#endif
