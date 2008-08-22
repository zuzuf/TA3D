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
  |                       UnitEngine.cpp                      |
  |    Contains the unit engine, which simulates units and    |
  | make them interact with each other.                       |
  |                                                           |
  \----------------------------------------------------------*/

#include "stdafx.h"
#include "misc/matrix.h"
#include "TA3D_NameSpace.h"
#include "ta3dbase.h"
#include "3do.h"					// Pour la lecture des fichiers 3D
#include "scripts/cob.h"					// Pour la lecture et l'éxecution des scripts
#include "tdf.h"					// Pour la gestion des éléments du jeu
#include "EngineClass.h"
#include "UnitEngine.h"
#include "network/TA3D_Network.h"
#include "gfx/fx.h"
#include "misc/camera.h"
#include "ingame/sidedata.h"
#include "languages/i18n.h"
#include "misc/math.h"
#include "sounds/manager.h"
#include "ingame/players.h"





/*!
 * \brief Display the executed code if enabled
 */
#define DEBUG_USE_PRINT_CODE 0

#if DEBUG_USE_PRINT_CODE == 1
#   define DEBUG_PRINT_CODE(X)  if (print_code) LOG_DEBUG(X)
#else
#   define DEBUG_PRINT_CODE(X)  
#endif


#define SQUARE(X)  ((X)*(X))




namespace TA3D
{


    INGAME_UNITS units;




    void UNIT::start_mission_script(int mission_type)
    {
        if (NULL == script)
            return;
        switch (mission_type)
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
        }
        if (mission_type != MISSION_STOP)
        {
            flags &= 191;
            //			flags |= 1;
        }
    }


    void UNIT::clear_mission()
    {
        if (NULL == mission)
            return;

        if (mission->mission == MISSION_GET_REPAIRED && mission->p)// Don't forget to detach the planes from air repair pads!
        {
            UNIT *target_unit = (UNIT*)(mission->p);
            target_unit->lock();
            if (target_unit->flags & 1)
            {
                int piece_id = mission->data >= 0 ? mission->data : (-mission->data - 1);
                if (target_unit->pad1 == piece_id) // tell others we've left
                    target_unit->pad1 = 0xFFFF;
                else
                    target_unit->pad2 = 0xFFFF;
            }
            target_unit->unlock();
        }
        MISSION* old = mission;
        mission = mission->next;
        if (old->path)				// Détruit le chemin si nécessaire
            destroy_path(old->path);
        delete old;
    }



    void UNIT::compute_model_coord()
    {
        if (!compute_coord || !model)
            return;
        pMutex.lock();
        compute_coord = false;
        MATRIX_4x4 M;
        float scale = unit_manager.unit_type[type_id]->Scale;

        // Matrice pour le calcul des positions des éléments du modèle de l'unité
        //    M = RotateZ(Angle.z*DEG2RAD) * RotateY(Angle.y * DEG2RAD) * RotateX(Angle.x * DEG2RAD) * Scale(scale);
        M = RotateZYX( Angle.z * DEG2RAD, Angle.y * DEG2RAD, Angle.x * DEG2RAD) * Scale(scale);
        model->compute_coord(&data, &M);
        pMutex.unlock();
    }


    void UNIT::raise_signal(uint32 signal)		// Tue les processus associés
    {
        SCRIPT_ENV_STACK *tmp;
        for (int i = 0; i  < nb_running; ++i)
        {
            tmp = (*script_env)[i].env;
            while (tmp)
            {
                if (tmp->signal_mask == signal)
                {
                    tmp = (*script_env)[i].env;
                    while (tmp != NULL)
                    {
                        (*script_env)[i].env = tmp->next;
                        delete tmp;
                        tmp = (*script_env)[i].env;
                    }
                }
                if (tmp)
                    tmp=tmp->next;
            }
            if ((*script_env)[i].env == NULL)
                (*script_env)[i].running=false;
        }
    }


    void UNIT::init_alloc_data()
    {
        s_var = new std::vector<int>();
        port = new sint16[21];				// Ports
        script_env = new std::vector< SCRIPT_ENV >();	// Environnements des scripts
        script_val = new std::vector<short>();	// Tableau de valeurs retournées par les scripts
        memory = new int[TA3D_PLAYERS_HARD_LIMIT];				// Pour se rappeler sur quelles armes on a déjà tiré
        script_idx = new char[NB_SCRIPT];	// Index of scripts to prevent multiple search
        attached_list = new short[20];
        link_list = new short[20];
        last_synctick = new uint32[TA3D_PLAYERS_HARD_LIMIT];
    }


    void UNIT::toggle_self_destruct()
    {
        if (self_destruct < 0.0f)
            self_destruct = unit_manager.unit_type[type_id]->selfdestructcountdown;
        else
            self_destruct = -1.0f;
    }


    int UNIT::get_script_index(int id)
    {
        if (script_idx[id] != -2)
            return script_idx[id];
        const char *script_name[]=
        {
            "QueryPrimary","AimPrimary","FirePrimary",
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
            "QueryLandingPad"
        };
        script_idx[id] = get_script_index(script_name[id]);
        return script_idx[id];
    }

    bool UNIT::isEnemy(const int t)
    {
        return t >= 0 && t < units.max_unit && !(players.team[units.unit[t].owner_id] & players.team[owner_id]);
    }


    bool UNIT::is_running(int script_index)	// Is the script still running ?
    {
        if (script == NULL)
            return false;
        if (script_index < 0 || script_index >= script->nb_script)
            return false;
        for (int i = 0; i < nb_running; ++i)
        {
            if ((*script_env)[i].running && (*script_env)[i].env != NULL)
            {
                SCRIPT_ENV_STACK *current=(*script_env)[i].env;
                while (current)
                {
                    if ((current->cur&0xFF) == script_index)
                        return true;
                    current = current->next;
                }
            }
        }
        return false;
    }


    int UNIT::get_script_index(const String &script_name)			// Cherche l'indice du script dont on fournit le nom
    {
        return (script) ? script->findFromName(script_name) : -1;
    }

    void UNIT::run_script_function(MAP *map, int id, int nb_param, int *param)	// Launch and run the script, returning it's values to param if not NULL
    {
        pMutex.lock();
        int script_idx = launch_script( id, nb_param, param );
        if (script_idx >= 0)
        {
            float dt = 1.0f / TICKS_PER_SEC;
            for (uint16 n = 0 ; n < 10000 && (*script_env)[ script_idx ].running && (*script_env)[ script_idx ].env != NULL ; n++ ) {
                if (nb_param > 0 && param != NULL)
                    for (int i = 0 ; i < nb_param ; ++i)
                        param[i] = (*script_env)[script_idx].env->var[i];
                if (run_script(dt, script_idx, map, 1))
                    break;
            }
            int e = 0;
            for (int i = 0; i + e < nb_running; ) // Do some cleaning so we don't use all the env table with unused data
            {
                if ((*script_env)[i+e].running)
                {
                    (*script_env)[i]=(*script_env)[i+e];
                    ++i;
                }
                else
                {
                    (*script_env)[i+e].destroy();
                    ++e;
                }
            }
            nb_running -= e;
        }
        pMutex.unlock();
    }

    void UNIT::reset_script()
    {
        pMutex.lock();
        for (int i = 0; i < nb_running; ++i)
            (*script_env)[i].destroy();
        nb_running=0;
        pMutex.unlock();
    }


    void UNIT::stop_moving()
    {
        if (mission->flags & MISSION_FLAG_MOVE)
        {
            mission->flags &= ~MISSION_FLAG_MOVE;
            if (mission->path)
            {
                destroy_path(mission->path);
                mission->path = NULL;
                V.x = V.y = V.z = 0.0f;
            }
            if (!(unit_manager.unit_type[type_id]->canfly && nb_attached > 0)) // Once charged with units the Atlas cannot land
                launch_script(get_script_index(SCRIPT_StopMoving));
            else
                was_moving = false;
            if (!(mission->flags & MISSION_FLAG_DONT_STOP_MOVE))
                V.x = V.y = V.z = 0.0f;		// Stop unit's movement
        }
    }


    void UNIT::lock_command()
    {
        pMutex.lock();
        command_locked = true;
        pMutex.unlock();
    }

    void UNIT::unlock_command()
    {
        pMutex.lock();
        command_locked = false;
        pMutex.unlock();
    }


    void UNIT::activate()
    {
        pMutex.lock();
        if (port[ACTIVATION] == 0)
        {
            play_sound("activate");
            launch_script(get_script_index(SCRIPT_Activate));
            port[ACTIVATION] = 1;
        }
        pMutex.unlock();
    }

    void UNIT::deactivate()
    {
        pMutex.lock();
        if (port[ACTIVATION] != 0)
        {
            play_sound("deactivate");
            launch_script(get_script_index(SCRIPT_Deactivate));
            port[ACTIVATION] = 0;
        }
        pMutex.unlock();
    }



    void UNIT::kill_script(int script_index)		// Fait un peu de ménage
    {
        if (script == NULL || script_index < 0 || script_index>=script->nb_script)
            return;
        pMutex.lock();
        for(int i = 0; i < nb_running; ++i)
        {
            if ((*script_env)[i].running && (*script_env)[i].env != NULL)
            {
                SCRIPT_ENV_STACK *current=(*script_env)[i].env;
                while (current)
                {
                    if ((current->cur&0xFF) == script_index) // Tue le script trouvé
                    {
                        current=(*script_env)[i].env;
                        (*script_env)[i].running=false;
                        while (current)
                        {
                            (*script_env)[i].env=current->next;
                            delete current;
                            current=(*script_env)[i].env;
                        }
                        break;
                    }
                    current=current->next;
                }
            }
        }
        int e = 0;
        for (int i = 0; i + e < nb_running; )// Efface les scripts qui se sont arrêtés
        {
            if ((*script_env)[i+e].running)
            {
                (*script_env)[i]=(*script_env)[i+e];
                ++i;
            }
            else
            {
                (*script_env)[i+e].destroy();
                ++e;
            }
        }
        nb_running -= e;
        pMutex.unlock();
    }



    void UNIT::init(int unit_type, int owner, bool full, bool basic)
    {
        pMutex.lock();

        ID = 0;
        paralyzed = 0.0f;

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
        if (full)
            init_alloc_data();
        for (int i = 0; i <  NB_SCRIPT; ++i)
            script_idx[i]=-2;	// Not yet searched
        just_created=true;
        first_move=true;
        attached=false;
        nb_attached=0;
        mem_size=0;
        planned_weapons=0.0f;
        attacked=false;
        groupe=0;
        weapon.clear();
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
        if (unit_type<0 || unit_type>=unit_manager.nb_unit)
            unit_type=-1;
        port[ACTIVATION]=0;
        mission=NULL;
        def_mission=NULL;
        port[BUILD_PERCENT_LEFT]=0;
        build_percent_left=0.0f;
        memset( last_synctick, 0, 40 );
        if (unit_type != -1)
        {
            if (!basic)
            {
                pMutex.unlock();
                set_mission(MISSION_STANDBY);
                pMutex.lock();
            }
            type_id = unit_type;
            model = unit_manager.unit_type[type_id]->model;
            weapon.resize(unit_manager.unit_type[type_id]->weapon.size());
            hp = unit_manager.unit_type[type_id]->MaxDamage;
            script = unit_manager.unit_type[type_id]->script;
            port[STANDINGMOVEORDERS] = unit_manager.unit_type[type_id]->StandingMoveOrder;
            port[STANDINGFIREORDERS] = unit_manager.unit_type[type_id]->StandingFireOrder;
            if (!basic)
            {
                pMutex.unlock();
                set_mission(unit_manager.unit_type[type_id]->DefaultMissionType);
                pMutex.lock();
            }
            if (script)
            {
                data.load(script->nb_piece);
                launch_script(get_script_index(SCRIPT_create));
            }
        }
        pMutex.unlock();
    }


    void UNIT::clear_def_mission()
    {
        while (def_mission)
        {
            MISSION* old = def_mission;
            def_mission = def_mission->next;
            if (old->path)				// Détruit le chemin si nécessaire
                destroy_path(old->path);
            delete old;
        }
    }


    void UNIT::destroy(bool full)
    {
        while (drawing)
            rest(0);
        pMutex.lock();
        ID = 0;
        for (int i = 0; i < nb_running; ++i)
            (*script_env)[i].destroy();
        while (mission)
            clear_mission();
        clear_def_mission();
        init();
        flags=0;
        groupe=0;
        pMutex.unlock();
        if (full)
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



    bool UNIT::is_on_radar(byte p_mask)
    {
        int px = cur_px>>1;
        int py = cur_py>>1;
        if (px >= 0 && py >= 0 && px < units.map->radar_map->w && py < units.map->radar_map->h && type_id != -1)
            return ( (units.map->radar_map->line[py][px] & p_mask) && !unit_manager.unit_type[type_id]->Stealth && (unit_manager.unit_type[type_id]->fastCategory & CATEGORY_NOTSUB) )
                || ( (units.map->sonar_map->line[py][px] & p_mask) && !(unit_manager.unit_type[type_id]->fastCategory & CATEGORY_NOTSUB) );
        return false;
    }

    void UNIT::add_mission(int mission_type, const Vector3D* target, bool step, int dat, void* pointer,
                           PATH_NODE* path, byte m_flags, int move_data, int patrol_node)
    {
        MutexLocker locker(pMutex);

        if (command_locked && !(mission_type & MISSION_FLAG_AUTO) )
            return;

        mission_type &= ~MISSION_FLAG_AUTO;

        uint32 target_ID = 0;

        if (pointer != NULL)
        {
            switch(mission_type)
            {
                case MISSION_GET_REPAIRED:
                case MISSION_CAPTURE:
                case MISSION_LOAD:
                case MISSION_BUILD_2:
                case MISSION_RECLAIM:
                case MISSION_REPAIR:
                case MISSION_GUARD:
                    target_ID = ((UNIT*)pointer)->ID;
                    break;
                case MISSION_ATTACK:
                    if (!(m_flags&MISSION_FLAG_TARGET_WEAPON) )
                        target_ID = ((UNIT*)pointer)->ID;
                    break;
            }
        }

        bool def_mode = false;
        if (type_id != -1 && !unit_manager.unit_type[type_id]->BMcode)
        {
            switch (mission_type)
            {
                case MISSION_MOVE:
                case MISSION_PATROL:
                case MISSION_GUARD:
                    def_mode = true;
                    break;
            }
        }

        if (pointer == this && !def_mode) // A unit cannot target itself
            return;

        if (mission_type == MISSION_MOVE || mission_type == MISSION_PATROL )
            m_flags |= MISSION_FLAG_MOVE;

        if (type_id != -1 && mission_type == MISSION_BUILD && unit_manager.unit_type[type_id]->BMcode && unit_manager.unit_type[type_id]->Builder && target != NULL)
        {
            bool removed = false;
            MISSION *cur_mission = mission;
            MISSION *prec = cur_mission;
            if (cur_mission)
                cur_mission = cur_mission->next;		// Don't read the first one ( which is being executed )

            while (cur_mission) 	// Reads the mission list
            {
                float x_space = fabs(cur_mission->target.x - target->x);
                float z_space = fabs(cur_mission->target.z - target->z);
                if (!cur_mission->step && cur_mission->mission == MISSION_BUILD && cur_mission->data >= 0 && cur_mission->data < unit_manager.nb_unit
                    && x_space < ((unit_manager.unit_type[ dat ]->FootprintX + unit_manager.unit_type[ cur_mission->data ]->FootprintX) << 2)
                    && z_space < ((unit_manager.unit_type[ dat ]->FootprintZ + unit_manager.unit_type[ cur_mission->data ]->FootprintZ) << 2) ) // Remove it
                {
                    MISSION *tmp = cur_mission;
                    cur_mission = cur_mission->next;
                    prec->next = cur_mission;
                    if (tmp->path)				// Destroy the path if needed
                        destroy_path(tmp->path);
                    delete tmp;
                    removed = true;
                }
                else
                {
                    prec = cur_mission;
                    cur_mission = cur_mission->next;
                }
            }
            if (removed)
                return;
        }

        MISSION *new_mission = new MISSION();
        new_mission->next = NULL;
        new_mission->mission = mission_type;
        new_mission->target_ID = target_ID;
        new_mission->step = step;
        new_mission->time = 0.0f;
        new_mission->data = dat;
        new_mission->p = pointer;
        new_mission->path = path;
        new_mission->last_d = 9999999.0f;
        new_mission->flags = m_flags;
        new_mission->move_data = move_data;
        new_mission->node = patrol_node;

        bool inserted = false;

        if (patrol_node == -1 && mission_type == MISSION_PATROL)
        {
            MISSION *mission_base = def_mode ? def_mission : mission;
            if (mission_base ) // Ajoute l'ordre aux autres
            {
                MISSION *cur = mission_base;
                MISSION *last = NULL;
                patrol_node = 0;
                while (cur != NULL)
                {
                    if (cur->mission == MISSION_PATROL && patrol_node <= cur->node )
                    {
                        patrol_node = cur->node;
                        last = cur;
                    }
                    cur=cur->next;
                }
                new_mission->node = patrol_node + 1;

                if (last)
                {
                    new_mission->next = last->next;
                    last->next = new_mission;
                    inserted = true;
                }
            }
            else
                new_mission->node = 1;
        }

        if (target)
            new_mission->target = *target;

        MISSION* stop = !(inserted) ? new MISSION() : NULL;
        if (stop)
        {
            stop->mission = MISSION_STOP;
            stop->step = true;
            stop->time = 0.0f;
            stop->p = NULL;
            stop->data = 0;
            stop->path = NULL;
            stop->last_d = 9999999.0f;
            stop->flags = m_flags & ~MISSION_FLAG_MOVE;
            stop->move_data = move_data;
            if (step)
            {
                stop->next=NULL;
                new_mission->next=stop;
            }
            else
            {
                stop->next=new_mission;
                new_mission=stop;
            }
        }

        if (step && mission && stop)
        {
            stop->next = def_mode ? def_mission : mission;
            mission = new_mission;
            if (!def_mode )
                start_mission_script(mission->mission);
        }
        else
        {
            MISSION* mission_base = def_mode ? def_mission : mission;
            if (mission_base && !inserted ) // Ajoute l'ordre aux autres
            {
                MISSION* cur = mission_base;
                while (cur->next!=NULL)
                    cur=cur->next;
                if (((( cur->mission == MISSION_MOVE || cur->mission == MISSION_PATROL || cur->mission == MISSION_STANDBY
                          || cur->mission == MISSION_VTOL_STANDBY || cur->mission == MISSION_STOP )		// Don't stop if it's not necessary
                        && ( mission_type == MISSION_MOVE || mission_type == MISSION_PATROL ) )
                      || ( ( cur->mission == MISSION_BUILD || cur->mission == MISSION_BUILD_2 )
                           && mission_type == MISSION_BUILD && type_id != -1 && !unit_manager.unit_type[type_id]->BMcode) )
                    && new_mission->next != NULL ) 	// Prevent factories from closing when already building a unit
                {
                    stop = new_mission->next;
                    delete new_mission;
                    new_mission = stop;
                    new_mission->next = NULL;
                }
                cur->next=new_mission;
            }
            else
            {
                if (mission_base == NULL)
                {
                    if (!def_mode)
                    {
                        mission = new_mission;
                        start_mission_script(mission->mission);
                    }
                    else
                        def_mission = new_mission;
                }
            }
        }
    }



    void UNIT::set_mission(int mission_type, const Vector3D* target, bool step, int dat, bool stopit,
                           void* pointer, PATH_NODE* path, byte m_flags, int move_data)
    {
        MutexLocker locker(pMutex);

        if (command_locked && !( mission_type & MISSION_FLAG_AUTO))
            return;
        mission_type &= ~MISSION_FLAG_AUTO;

        uint32 target_ID = 0;

        if (pointer != NULL)
        {
            switch( mission_type)
            {
                case MISSION_GET_REPAIRED:
                case MISSION_CAPTURE:
                case MISSION_LOAD:
                case MISSION_BUILD_2:
                case MISSION_RECLAIM:
                case MISSION_REPAIR:
                case MISSION_GUARD:
                    target_ID = ((UNIT*)pointer)->ID;
                    break;
                case MISSION_ATTACK:
                    if (!(m_flags&MISSION_FLAG_TARGET_WEAPON) )
                        target_ID = ((UNIT*)pointer)->ID;
                    break;
            }
        }

        if (nanolathe_target >= 0 && network_manager.isConnected())
        {
            nanolathe_target = -1;
            g_ta3d_network->sendUnitNanolatheEvent( idx, -1, false, false );
        }

        bool def_mode = false;
        if (type_id != -1 && !unit_manager.unit_type[type_id]->BMcode)
        {
            switch (mission_type)
            {
                case MISSION_MOVE:
                case MISSION_PATROL:
                case MISSION_GUARD:
                    def_mode = true;
                    break;
            }
        }

        if (pointer == this && !def_mode) // A unit cannot target itself
            return;

        int old_mission=-1;
        if (!def_mode)
        {
            if (mission != NULL)
                old_mission = mission->mission;
        }
        else
            clear_def_mission();

        bool already_running = false;

        if (mission_type == MISSION_MOVE || mission_type == MISSION_PATROL )
            m_flags |= MISSION_FLAG_MOVE;

        switch(old_mission)		// Commandes de fin de mission
        {
            case MISSION_REPAIR:
            case MISSION_RECLAIM:
            case MISSION_BUILD_2: {
                                      launch_script(get_script_index(SCRIPT_stopbuilding));
                                      deactivate();
                                      if (type_id != -1 && !unit_manager.unit_type[type_id]->BMcode) // Delete the unit we were building
                                      {
                                          sint32 prev = -1;
                                          for(int i = units.nb_unit-1; i>=0 ; i--)
                                          {
                                              if (units.idx_list[i] == ((UNIT*)(mission->p))->idx)
                                              {
                                                  prev = i;
                                                  break;
                                              }
                                          }
                                          if (prev >= 0 )
                                              units.kill(((UNIT*)(mission->p))->idx,units.map,prev);
                                      }
                                      else
                                          launch_script(get_script_index(SCRIPT_stop));
                                      break;
                                  }
            case MISSION_ATTACK: {
                                     if (mission_type != MISSION_ATTACK && type_id != -1 &&
                                         (!unit_manager.unit_type[type_id]->canfly
                                          || (unit_manager.unit_type[type_id]->canfly && mission_type != MISSION_MOVE && mission_type != MISSION_PATROL ) ) )
                                         deactivate();
                                     else
                                     {
                                         stopit = false;
                                         already_running = true;
                                     }
                                     break;
                                 }
            case MISSION_MOVE:
            case MISSION_PATROL: {
                                     if (mission_type == MISSION_MOVE || mission_type == MISSION_PATROL
                                         || (type_id != -1 && unit_manager.unit_type[type_id]->canfly && mission_type == MISSION_ATTACK ) )
                                     {
                                         stopit = false;
                                         already_running = true;
                                     }
                                     break;
                                 }
        }

        if (!def_mode)
        {
            while (mission != NULL)
                clear_mission();			// Efface les ordres précédents
            last_path_refresh = 10.0f;
        }

        if (def_mode)
        {
            def_mission = new MISSION();
            def_mission->next = NULL;
            def_mission->mission = mission_type;
            def_mission->target_ID = target_ID;
            def_mission->step = step;
            def_mission->time = 0.0f;
            def_mission->data = dat;
            def_mission->p = pointer;
            def_mission->path = path;
            def_mission->last_d = 9999999.0f;
            def_mission->flags = m_flags;
            def_mission->move_data = move_data;
            def_mission->node = 1;
            if (target)
                def_mission->target=*target;

            if (stopit)
            {
                MISSION *stop = new MISSION();
                stop->next = def_mission;
                stop->mission = MISSION_STOP;
                stop->step = true;
                stop->time = 0.0f;
                stop->p = NULL;
                stop->data = 0;
                stop->path = NULL;
                stop->last_d = 9999999.0f;
                stop->flags = m_flags & ~MISSION_FLAG_MOVE;
                stop->move_data = move_data;
                def_mission = stop;
            }
        }
        else
        {
            mission = new MISSION();
            mission->next = NULL;
            mission->mission = mission_type;
            mission->target_ID = target_ID;
            mission->step = step;
            mission->time = 0.0f;
            mission->data = dat;
            mission->p = pointer;
            mission->path = path;
            mission->last_d = 9999999.0f;
            mission->flags = m_flags;
            mission->move_data = move_data;
            mission->node = 1;
            if (target)
                mission->target=*target;

            if (stopit)
            {
                MISSION *stop = new MISSION();
                stop->next = mission;
                stop->mission = MISSION_STOP;
                stop->step = true;
                stop->time = 0.0f;
                stop->p = NULL;
                stop->data = 0;
                stop->path = NULL;
                stop->last_d = 9999999.0f;
                stop->flags = m_flags & ~MISSION_FLAG_MOVE;
                stop->move_data = move_data;
                mission = stop;
            }
            else
            {
                if (!already_running)
                    start_mission_script(mission->mission);
            }
            c_time=0.0f;
        }
    }



    void UNIT::next_mission()
    {
        last_path_refresh = 10.0f;		// By default allow to compute a new path
        if (nanolathe_target >= 0 && network_manager.isConnected())
        {
            nanolathe_target = -1;
            g_ta3d_network->sendUnitNanolatheEvent( idx, -1, false, false );
        }

        if (mission == NULL)
        {
            command_locked = false;
            if (type_id != -1)
                set_mission( unit_manager.unit_type[type_id]->DefaultMissionType, NULL, false, 0, false);
            return;
        }
        switch (mission->mission) // Commandes de fin de mission
        {
            case MISSION_REPAIR:
            case MISSION_RECLAIM:
            case MISSION_BUILD_2:
                if (mission->next == NULL || (type_id != -1 && unit_manager.unit_type[type_id]->BMcode) || mission->next->mission != MISSION_BUILD)
                {
                    launch_script(get_script_index(SCRIPT_stopbuilding));
                    deactivate();
                }
                break;
            case MISSION_ATTACK:
                deactivate();
                break;
        }
        if (mission->mission==MISSION_STOP && mission->next==NULL)
        {
            command_locked = false;
            mission->data=0;
            return;
        }
        bool old_step = mission->step;
        MISSION *old=mission;
        mission=mission->next;
        if (old->path)				// Détruit le chemin si nécessaire
            destroy_path(old->path);
        delete old;
        if (mission==NULL)
        {
            command_locked = false;
            if (type_id != -1)
                set_mission(unit_manager.unit_type[type_id]->DefaultMissionType);
        }

        // Skip a stop order before a normal order if the unit can fly (prevent planes from looking for a place to land when they don't need to land !!)
        if (type_id != -1 && unit_manager.unit_type[type_id]->canfly && mission->mission == MISSION_STOP && mission->next != NULL && mission->next->mission != MISSION_STOP)
        {
            old = mission;
            mission = mission->next;
            if (old->path)				// Détruit le chemin si nécessaire
                destroy_path(old->path);
            delete old;
        }

        if (old_step && mission && mission->next
            && (mission->mission == MISSION_STOP || mission->mission == MISSION_VTOL_STANDBY || mission->mission == MISSION_STANDBY))
            next_mission();

        start_mission_script(mission->mission);
        c_time=0.0f;
    }


    void UNIT::draw(float t, Camera& cam, MAP* map, bool height_line)
    {
        MutexLocker locker(pMutex);

        if (!(flags & 1) || type_id == -1)
            return;

        visible = false;
        on_radar = false;
        on_mini_radar = false;

        drawn_Pos = Pos;
        drawn_Angle = Angle;

        if (!model || hidden)
            return;		// S'il n'y a pas de modèle associé, on quitte la fonction

        int px = cur_px >> 1;
        int py = cur_py >> 1;
        if (px < 0 || py < 0 || px >= map->bloc_w || py >= map->bloc_h)
            return;	// Unité hors de la carte
        byte player_mask = 1 << players.local_human_id;

        on_radar = on_mini_radar = is_on_radar( player_mask );
        if (map->view[py][px] == 0 || ( map->view[py][px] > 1 && !on_radar ) || ( !on_radar && !(map->sight_map->line[py][px] & player_mask) ) )
            return;	// Unit is not visible

        bool radar_detected = on_radar;

        on_radar &= map->view[py][px] > 1;

        Vector3D D (Pos - cam.pos); // Vecteur "viseur unité" partant de la caméra vers l'unité

        float dist=D.sq();
        if (dist >= 16384.0f && (D % cam.dir) <= 0.0f)
            return;
        if ((D % cam.dir) > cam.zfar2)
            return;		// Si l'objet est hors champ on ne le dessine pas

        if (!cloaked || owner_id == players.local_human_id) // Don't show cloaked units
        {
            visible = true;
            on_radar |= cam.rpos.y > gfx->low_def_limit;
        }
        else
        {
            on_radar |= radar_detected;
            visible = on_radar;
        }

        MATRIX_4x4 M;
        glPushMatrix();
        if (on_radar) // for mega zoom, draw only an icon
        {
            glDisable(GL_DEPTH_TEST);
            glTranslatef( Pos.x, Math::Max(Pos.y,map->sealvl+5.0f), Pos.z);
            glEnable(GL_TEXTURE_2D);
            int unit_nature = ICON_UNKNOWN;
            float size = (D % cam.dir) * 12.0f / gfx->height;

            if (unit_manager.unit_type[type_id]->fastCategory & CATEGORY_KAMIKAZE )
                unit_nature = ICON_KAMIKAZE;
            else if (( unit_manager.unit_type[type_id]->TEDclass & CLASS_COMMANDER ) == CLASS_COMMANDER )
                unit_nature = ICON_COMMANDER;
            else if (( unit_manager.unit_type[type_id]->TEDclass & CLASS_ENERGY ) == CLASS_ENERGY )
                unit_nature = ICON_ENERGY;
            else if (( unit_manager.unit_type[type_id]->TEDclass & CLASS_METAL ) == CLASS_METAL )
                unit_nature = ICON_METAL;
            else if (( unit_manager.unit_type[type_id]->TEDclass & CLASS_TANK ) == CLASS_TANK )
                unit_nature = ICON_TANK;
            else if (unit_manager.unit_type[type_id]->Builder ) {
                if (!unit_manager.unit_type[type_id]->BMcode )
                    unit_nature = ICON_FACTORY;
                else
                    unit_nature = ICON_BUILDER;
            }
            else if (( unit_manager.unit_type[type_id]->TEDclass & CLASS_SHIP ) == CLASS_SHIP )
                unit_nature = ICON_WATERUNIT;
            else if (( unit_manager.unit_type[type_id]->TEDclass & CLASS_FORT ) == CLASS_FORT )
                unit_nature = ICON_DEFENSE;
            else if (( unit_manager.unit_type[type_id]->fastCategory & CATEGORY_NOTAIR ) && ( unit_manager.unit_type[type_id]->fastCategory & CATEGORY_NOTSUB ) )
                unit_nature = ICON_LANDUNIT;
            else if (!( unit_manager.unit_type[type_id]->fastCategory & CATEGORY_NOTAIR ) )
                unit_nature = ICON_AIRUNIT;
            else if (!( unit_manager.unit_type[type_id]->fastCategory & CATEGORY_NOTSUB ) )
                unit_nature = ICON_SUBUNIT;

            glBindTexture( GL_TEXTURE_2D, units.icons[ unit_nature ] );
            glDisable( GL_CULL_FACE );
            glDisable(GL_LIGHTING);
            glDisable(GL_BLEND);
            glTranslatef( model->center.x, model->center.y, model->center.z );
            if (player_color[player_color_map[owner_id]*3] != 0.0f || player_color[player_color_map[owner_id]*3+1] != 0.0f || player_color[player_color_map[owner_id]*3+2] != 0.0f )
            {
                glColor3f(player_color[player_color_map[owner_id]*3],player_color[player_color_map[owner_id]*3+1],player_color[player_color_map[owner_id]*3+2]);
                glBegin(GL_QUADS);
                glTexCoord2f(0.0f, 0.0f);		glVertex3f( -size, 0.0f, -size);
                glTexCoord2f(1.0f, 0.0f);		glVertex3f(  size, 0.0f, -size);
                glTexCoord2f(1.0f, 1.0f);		glVertex3f(  size, 0.0f,  size);
                glTexCoord2f(0.0f, 1.0f);		glVertex3f( -size, 0.0f,  size);
                glEnd();
            }
            else
            {								// If it's black, then invert colors
                glColor3ub(0xFF,0xFF,0xFF);
                glDisable(GL_TEXTURE_2D);
                glBegin(GL_QUADS);
                glTexCoord2f(0.0f, 0.0f);		glVertex3f( -size, 0.0f, -size);
                glTexCoord2f(1.0f, 0.0f);		glVertex3f(  size, 0.0f, -size);
                glTexCoord2f(1.0f, 1.0f);		glVertex3f(  size, 0.0f,  size);
                glTexCoord2f(0.0f, 1.0f);		glVertex3f( -size, 0.0f,  size);
                glEnd();
                glEnable(GL_TEXTURE_2D);
                glBlendFunc( GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
                glEnable(GL_BLEND);
                glBegin(GL_QUADS);
                glTexCoord2f(0.0f, 0.0f);		glVertex3f( -size, 0.0f, -size);
                glTexCoord2f(1.0f, 0.0f);		glVertex3f(  size, 0.0f, -size);
                glTexCoord2f(1.0f, 1.0f);		glVertex3f(  size, 0.0f,  size);
                glTexCoord2f(0.0f, 1.0f);		glVertex3f( -size, 0.0f,  size);
                glEnd();
                glDisable(GL_BLEND);
            }
            glEnable( GL_CULL_FACE );
            if (owner_id == players.local_human_id && sel)
            {
                glDisable( GL_TEXTURE_2D );
                glColor3ub(0xFF,0xFF,0);
                glBegin(GL_LINE_LOOP);
                glVertex3f( -size, 0.0f, -size);
                glVertex3f(  size, 0.0f, -size);
                glVertex3f(  size, 0.0f,  size);
                glVertex3f( -size, 0.0f,  size);
                glEnd();
            }
            glEnable(GL_DEPTH_TEST);
        }
        else
            if (visible)
            {
                glTranslatef( Pos.x, Pos.y, Pos.z );
                glRotatef(Angle.x,1.0f,0.0f,0.0f);
                glRotatef(Angle.z,0.0f,0.0f,1.0f);
                glRotatef(Angle.y,0.0f,1.0f,0.0f);
                float scale=unit_manager.unit_type[type_id]->Scale;
                glScalef(scale,scale,scale);

                //            M=RotateY(Angle.y*DEG2RAD)*RotateZ(Angle.z*DEG2RAD)*RotateX(Angle.x*DEG2RAD)*Scale(scale);			// Matrice pour le calcul des positions des éléments du modèle de l'unité
                M = RotateYZX(Angle.y*DEG2RAD, Angle.z*DEG2RAD, Angle.x*DEG2RAD)*Scale(scale);			// Matrice pour le calcul des positions des éléments du modèle de l'unité

                Vector3D *target=NULL,*center=NULL;
                Vector3D upos;
                bool c_part=false;
                bool reverse=false;
                float size=0.0f;
                OBJECT *src = NULL;
                SCRIPT_DATA *src_data = NULL;
                Vector3D v_target;				// Needed in network mode
                UNIT *unit_target = NULL;
                MODEL *the_model = model;
                drawing = true;

                if (build_percent_left == 0.0f && mission != NULL && port[ INBUILDSTANCE ] != 0 && local )
                {
                    if (c_time>=0.125f)
                    {
                        reverse=(mission->mission==MISSION_RECLAIM);
                        c_time=0.0f;
                        c_part=true;
                        upos.x=upos.y=upos.z=0.0f;
                        upos=upos+Pos;
                        if (mission->p != NULL && (mission->mission == MISSION_REPAIR || mission->mission == MISSION_BUILD
                                                   || mission->mission == MISSION_BUILD_2 || mission->mission == MISSION_CAPTURE))
                        {
                            unit_target = ((UNIT*)mission->p);
                            pMutex.unlock();
                            unit_target->lock();
                            if ((unit_target->flags & 1) && unit_target->model!=NULL)
                            {
                                size=unit_target->model->size2;
                                center=&unit_target->model->center;
                                src = &unit_target->model->obj;
                                src_data = &unit_target->data;
                                unit_target->compute_model_coord();
                            }
                            else
                            {
                                unit_target->unlock();
                                pMutex.lock();
                                unit_target = NULL;
                                c_part = false;
                            }
                        }
                        else
                        {
                            if (mission->mission == MISSION_RECLAIM || mission->mission == MISSION_REVIVE ) // Reclaiming features
                            {
                                int feature_type = features.feature[ mission->data ].type;
                                if (mission->data >= 0 && feature_type >= 0 && feature_manager.feature[ feature_type ].model )
                                {
                                    size = feature_manager.feature[ feature_type ].model->size2;
                                    center = &feature_manager.feature[ feature_type ].model->center;
                                    src = &feature_manager.feature[ feature_type ].model->obj;
                                    src_data = NULL;
                                }
                                else
                                {
                                    D.x = D.y = D.z = 0.f;
                                    center = &D;
                                    size = 32.0f;
                                }
                            }
                            else
                                c_part=false;
                        }
                        target = &(mission->target);
                    }
                }
                else 
                {
                    if (!local && nanolathe_target >= 0 && port[ INBUILDSTANCE ] != 0 )
                    {
                        if (c_time>=0.125f)
                        {
                            reverse = nanolathe_reverse;
                            c_time=0.0f;
                            c_part=true;
                            upos.x=upos.y=upos.z=0.0f;
                            upos=upos+Pos;
                            if (!nanolathe_feature)
                            {
                                unit_target = &(units.unit[ nanolathe_target ]);
                                pMutex.unlock();
                                unit_target->lock();
                                if ((unit_target->flags & 1) && unit_target->model )
                                {
                                    size = unit_target->model->size2;
                                    center = &unit_target->model->center;
                                    src = &unit_target->model->obj;
                                    src_data = &unit_target->data;
                                    unit_target->compute_model_coord();
                                    v_target = unit_target->Pos;
                                }
                                else
                                {
                                    unit_target->unlock();
                                    pMutex.lock();
                                    unit_target = NULL;
                                    c_part = false;
                                }
                            }
                            else // Reclaiming features
                            {
                                int feature_type = features.feature[ nanolathe_target ].type;
                                v_target = features.feature[ nanolathe_target ].Pos;
                                if (feature_type >= 0 && feature_manager.feature[ feature_type ].model )
                                {
                                    size = feature_manager.feature[ feature_type ].model->size2;
                                    center = &feature_manager.feature[ feature_type ].model->center;
                                    src = &feature_manager.feature[ feature_type ].model->obj;
                                    src_data = NULL;
                                }
                                else
                                {
                                    D.x = D.y = D.z = 0.f;
                                    center = &D;
                                    size = 32.0f;
                                }
                            }
                            target = &v_target;
                        }
                    }
                }
                if (c_part)	// Get the nanolathing points
                {
                    if (!unit_manager.unit_type[type_id]->emitting_points_computed ) // Compute model emitting points if not already done
                    {
                        unit_manager.unit_type[type_id]->emitting_points_computed = true;
                        int param[] = { -1 };
                        int querynanopiece_idx = get_script_index( "QueryNanoPiece" );
                        run_script_function( NULL, querynanopiece_idx, 1, param );
                        int first = param[0];
                        int i = 0;
                        do
                        {
                            model->obj.compute_emitter_point( param[ 0 ] );
                            run_script_function( NULL, querynanopiece_idx, 1, param );
                            ++i;
                        } while( first != param[0] && i < 1000 );
                    }
                }

                if (build_percent_left == 0.0f)
                {
                    if (cloaked || ( cloaking && owner_id != players.local_human_id ) )
                        glColor4ub( 0xFF, 0xFF, 0xFF, 0x7F );
                    the_model->draw(t,&data,owner_id==players.local_human_id && sel,false,c_part,build_part,target,&upos,&M,size,center,reverse,owner_id,!cloaked,src,src_data);
                    if (cloaked || ( cloaking && owner_id != players.local_human_id ) )
                        gfx->set_color( 0xFFFFFFFF );
                    if (height_line && h>1.0f && unit_manager.unit_type[type_id]->canfly) // For flying units, draw a line that shows how high is the unit
                    {
                        glPopMatrix();
                        glPushMatrix();
                        glDisable(GL_TEXTURE_2D);
                        glDisable(GL_LIGHTING);
                        glColor3ub(0xFF,0xFF,0);
                        glBegin(GL_LINES);
                        for (float y=Pos.y;y>Pos.y-h;y-=10.0f)
                        {
                            glVertex3f(Pos.x,y,Pos.z);
                            glVertex3f(Pos.x,y-5.0f,Pos.z);
                        }
                        glEnd();
                    }
                }
                else
                {
                    if (build_percent_left<=33.0f)
                    {
                        float h = model->top - model->bottom;
                        double eqn[4]= { 0.0f, 1.0f, 0.0f, -model->bottom - h*(33.0f-build_percent_left)*0.033333f};

                        glClipPlane(GL_CLIP_PLANE0, eqn);
                        glEnable(GL_CLIP_PLANE0);
                        the_model->draw(t,&data,owner_id==players.local_human_id && sel,true,c_part,build_part,target,&upos,&M,size,center,reverse,owner_id,true,src,src_data);

                        eqn[1]=-eqn[1];	eqn[3]=-eqn[3];
                        glClipPlane(GL_CLIP_PLANE0, eqn);
                        the_model->draw(t,&data,owner_id==players.local_human_id && sel,false,false,build_part,target,&upos,&M,size,center,reverse,owner_id);
                        glDisable(GL_CLIP_PLANE0);

                        glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
                        the_model->draw(t,&data,owner_id==players.local_human_id && sel,true,false,build_part,target,&upos,&M,size,center,reverse,owner_id);
                        glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
                    }
                    else
                    {
                        if (build_percent_left<=66.0f)
                        {
                            float h = model->top - model->bottom;
                            double eqn[4]= { 0.0f, 1.0f, 0.0f, -model->bottom - h*(66.0f-build_percent_left)*0.033333f};

                            glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
                            glClipPlane(GL_CLIP_PLANE0, eqn);
                            glEnable(GL_CLIP_PLANE0);
                            glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
                            the_model->draw(t,&data,owner_id==players.local_human_id && sel,true,c_part,build_part,target,&upos,&M,size,center,reverse,owner_id,true,src,src_data);
                            glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

                            eqn[1]=-eqn[1];	eqn[3]=-eqn[3];
                            glClipPlane(GL_CLIP_PLANE0, eqn);
                            the_model->draw(t,&data,owner_id==players.local_human_id && sel,true,false,build_part,target,&upos,&M,size,center,reverse,owner_id);
                            glDisable(GL_CLIP_PLANE0);
                        }
                        else
                        {
                            glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
                            the_model->draw(t,&data,owner_id==players.local_human_id && sel,true,false,build_part,target,&upos,&M,size,center,reverse,owner_id);
                            glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
                        }
                    }
                }
                
                if (lp_CONFIG->underwater_bright && map->water && Pos.y < map->sealvl)
                {
                    double eqn[4]= { 0.0f, -1.0f, 0.0f, map->sealvl - Pos.y };

                    glClipPlane(GL_CLIP_PLANE0, eqn);
                    glEnable(GL_CLIP_PLANE0);

                    glEnable( GL_BLEND );
                    glBlendFunc( GL_ONE, GL_ONE );
                    glDepthFunc( GL_EQUAL );
                    glColor4ub( 0x7F, 0x7F, 0x7F, 0x7F );
                    the_model->draw(t,&data,false,true,false,0,NULL,NULL,NULL,0.0f,NULL,false,owner_id,false);
                    glColor4ub( 0xFF, 0xFF, 0xFF, 0xFF );
                    glDepthFunc( GL_LESS );
                    glDisable( GL_BLEND );

                    glDisable(GL_CLIP_PLANE0);
                }

                if (unit_target)
                {
                    unit_target->unlock();
                    pMutex.lock();
                }
            }
        drawing = false;
        glPopMatrix();
    }



    void UNIT::draw_shadow(const Vector3D& Dir, MAP* map)
    {
        pMutex.lock();
        if (!(flags & 1))
        {
            pMutex.unlock();
            return;
        }

        if (on_radar || hidden)
        {
            pMutex.unlock();
            return;
        }

        if (!model)
            LOG_WARNING("Model is NULL ! (" << __FILE__ << ":" << __LINE__ << ")");

        if (cloaked && owner_id != players.local_human_id ) // Unit is cloaked
        {
            pMutex.unlock();
            return;
        }

        if (!visible)
        {
            Vector3D S_Pos = drawn_Pos-(h/Dir.y)*Dir;//map->hit(Pos,Dir);
            int px=((int)(S_Pos.x)+map->map_w_d)>>4;
            int py=((int)(S_Pos.z)+map->map_h_d)>>4;
            if (px<0 || py<0 || px>=map->bloc_w || py>=map->bloc_h)
            {
                pMutex.unlock();
                return;	// Shadow out of the map
            }
            if (map->view[py][px]!=1)
            {
                pMutex.unlock();
                return;	// Unvisible shadow
            }
        }

        drawing = true;			// Prevent the model to be set to NULL and the data structure from being reset
        pMutex.unlock();

        glPushMatrix();
        glTranslatef(drawn_Pos.x,drawn_Pos.y,drawn_Pos.z);
        glRotatef(drawn_Angle.x,1.0f,0.0f,0.0f);
        glRotatef(drawn_Angle.z,0.0f,0.0f,1.0f);
        glRotatef(drawn_Angle.y,0.0f,1.0f,0.0f);
        float scale = unit_manager.unit_type[type_id]->Scale;
        glScalef(scale,scale,scale);
        glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

        if ((type_id != -1 && unit_manager.unit_type[type_id]->canmove) || shadow_scale_dir < 0.0f)
        {
            Vector3D H = drawn_Pos;
            H.y += 2.0f * model->size2 + 1.0f;
            Vector3D D = map->hit( H, Dir, true, 2000.0f);
            shadow_scale_dir = (D - H).norm();
        }
        //    model->draw_shadow(((shadow_scale_dir*Dir*RotateX(-drawn_Angle.x*DEG2RAD))*RotateZ(-drawn_Angle.z*DEG2RAD))*RotateY(-drawn_Angle.y*DEG2RAD),0.0f,&data);
        model->draw_shadow(shadow_scale_dir*Dir*RotateXZY(-drawn_Angle.x*DEG2RAD, -drawn_Angle.z*DEG2RAD, -drawn_Angle.y*DEG2RAD),0.0f, &data);

        glPopMatrix();

        drawing = false;
    }


    void UNIT::draw_shadow_basic(const Vector3D& Dir,MAP *map)
    {
        pMutex.lock();
        if (!(flags & 1))
        {
            pMutex.unlock();
            return;
        }
        if (on_radar || hidden)
        {
            pMutex.unlock();
            return;
        }

        if (cloaked && owner_id != players.local_human_id ) // Unit is cloaked
        {
            pMutex.unlock();
            return;
        }

        if (!visible)
        {
            Vector3D S_Pos (drawn_Pos - (h / Dir.y) * Dir);//map->hit(Pos,Dir);
            int px = ((int)(S_Pos.x + map->map_w_d)) >> 4;
            int py = ((int)(S_Pos.z + map->map_h_d)) >> 4;
            if (px < 0 || py < 0 || px >= map->bloc_w || py >= map->bloc_h)
            {
                pMutex.unlock();
                return;	// Shadow out of the map
            }
            if (map->view[py][px]!=1)
            {
                pMutex.unlock();
                return;	// Unvisible shadow
            }
        }
        drawing = true;			// Prevent the model to be set to NULL and the data structure from being reset
        pMutex.unlock();

        glPushMatrix();
        glTranslatef(drawn_Pos.x,drawn_Pos.y,drawn_Pos.z);
        glRotatef(drawn_Angle.x,1.0f,0.0f,0.0f);
        glRotatef(drawn_Angle.z,0.0f,0.0f,1.0f);
        glRotatef(drawn_Angle.y,0.0f,1.0f,0.0f);
        float scale = unit_manager.unit_type[type_id]->Scale;
        glScalef(scale,scale,scale);
        glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
        if (unit_manager.unit_type[type_id]->canmove || shadow_scale_dir < 0.0f )
        {
            Vector3D H = drawn_Pos;
            H.y += 2.0f * model->size2 + 1.0f;
            Vector3D D = map->hit( H, Dir, true, 2000.0f);
            shadow_scale_dir = (D - H).norm();
        }
        //    model->draw_shadow_basic(((shadow_scale_dir*Dir*RotateX(-drawn_Angle.x*DEG2RAD))*RotateZ(-drawn_Angle.z*DEG2RAD))*RotateY(-drawn_Angle.y*DEG2RAD),0.0f,&data);
        model->draw_shadow_basic(shadow_scale_dir*Dir*RotateXZY(-drawn_Angle.x*DEG2RAD, -drawn_Angle.z*DEG2RAD, -drawn_Angle.y*DEG2RAD),0.0f,&data);

        glPopMatrix();
    }


    const int UNIT::run_script(const float &dt,const int &id,MAP *map,int max_code)			// Interprète les scripts liés à l'unité
    {
        if (flags==0)
            return 2;
        if (id >= (int)script_env->size() && !(*script_env)[id].running)
            return 2;
        if ((*script_env)[id].wait>0.0f)
        {
            (*script_env)[id].wait-=dt;
            return 1;
        }
        if (script==NULL || (*script_env)[id].env==NULL)
        {
            (*script_env)[id].running=false;
            return 2;	// S'il n'y a pas de script associé on quitte la fonction
        }
        sint16 script_id=((*script_env)[id].env->cur&0xFF);			// Récupère l'identifiant du script en cours d'éxecution et la position d'éxecution
        sint16 pos=((*script_env)[id].env->cur>>8);

        if (script_id<0 || script_id>=script->nb_script)
        {
            (*script_env)[id].running=false;
            return 2;		// Erreur, ce n'est pas un script repertorié
        }

        float divisor(I2PWR16);
        float div=0.5f*divisor;
        bool done=false;
        int nb_code=0;

#if DEBUG_USE_PRINT_CODE == 1
        bool print_code = false;
        //bool	print_code = String::ToLower( unit_manager.unit_type[type_id]->Unitname ) == "armtship" && (String::ToLower( script->name[script_id] ) == "transportpickup" || String::ToLower( script->name[script_id] ) == "boomcalc" );
#endif

        do
        {
            //			uint32 code=script->script_code[script_id][pos];			// Lit un code
            //			pos++;
            nb_code++;
            if (nb_code >= max_code ) done=true;			// Pour éviter que le programme ne fige à cause d'un script
            //			switch(code)			// Code de l'interpréteur
            switch(script->script_code[script_id][pos++])
            {
                case SCRIPT_MOVE_OBJECT:
                    {
                        DEBUG_PRINT_CODE("MOVE_OBJECT");
                        int obj=script->script_code[script_id][pos++];
                        int axis=script->script_code[script_id][pos++];
                        int v1=(*script_env)[id].pop();
                        int v2=(*script_env)[id].pop();
                        if (axis==0)
                            v1=-v1;
                        data.axe[axis][obj].reset_move();
                        data.axe[axis][obj].move_distance=v1*div;
                        data.axe[axis][obj].move_distance-=data.axe[axis][obj].pos;
                        data.axe[axis][obj].is_moving=true;
                        data.is_moving=true;
                        if (data.axe[axis][obj].move_distance<0.0f)
                            data.axe[axis][obj].move_speed=-fabs(v2*div*0.5f);
                        else
                            data.axe[axis][obj].move_speed=fabs(v2*div*0.5f);
                        break;
                    }
                case SCRIPT_WAIT_FOR_TURN:
                    {
                        DEBUG_PRINT_CODE("WAIT_FOR_TURN");
                        int obj=script->script_code[script_id][pos++];
                        int axis=script->script_code[script_id][pos++];
                        float a=data.axe[axis][obj].rot_angle;
                        if ((data.axe[axis][obj].rot_speed!=0.0f || data.axe[axis][obj].rot_accel!=0.0f) && (a!=0.0f && data.axe[axis][obj].rot_limit))
                            pos-=3;
                        else if (data.axe[axis][obj].rot_speed!=data.axe[axis][obj].rot_target_speed && data.axe[axis][obj].rot_speed_limit)
                            pos-=3;
                        else {
                            data.axe[axis][obj].rot_speed = 0.0f;
                            data.axe[axis][obj].rot_accel = 0.0f;
                        }
                        done = true;
                        break;
                    }
                case SCRIPT_RANDOM_NUMBER:
                    {
                        DEBUG_PRINT_CODE("RANDOM_NUMBER");
                        int high=(*script_env)[id].pop();
                        int low=(*script_env)[id].pop();
                        (*script_env)[id].push(((sint32)(Math::RandFromTable()%(high-low+1)))+low);
                        break;
                    }
                case SCRIPT_GREATER_EQUAL:
                    {
                        DEBUG_PRINT_CODE("GREATER_EQUAL");
                        int v2=(*script_env)[id].pop();
                        int v1=(*script_env)[id].pop();
                        (*script_env)[id].push(v1>=v2 ? 1 : 0);
                        break;
                    }
                case SCRIPT_GREATER:
                    {
                        DEBUG_PRINT_CODE("GREATER");
                        int v2=(*script_env)[id].pop();
                        int v1=(*script_env)[id].pop();
                        (*script_env)[id].push(v1>v2 ? 1 : 0);
                        break;
                    }
                case SCRIPT_LESS:
                    {
                        DEBUG_PRINT_CODE("LESS");
                        int v2=(*script_env)[id].pop();
                        int v1=(*script_env)[id].pop();
                        (*script_env)[id].push(v1<v2 ? 1 : 0);
                        break;
                    }
                case SCRIPT_EXPLODE:
                    {
                        DEBUG_PRINT_CODE("EXPLODE");
                        int obj = script->script_code[script_id][pos++];
                        int explosion_type = (*script_env)[id].pop();
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
                        data.flag[obj]|=FLAG_EXPLODE;
                        data.explosion_flag[obj]=explosion_type;
                        data.axe[0][obj].move_speed=(25.0f+(Math::RandFromTable()%2501)*0.01f)*(Math::RandFromTable()&1 ? 1.0f : -1.0f);
                        data.axe[0][obj].rot_speed=(Math::RandFromTable()%7201)*0.1f-360.0f;
                        data.axe[1][obj].move_speed=25.0f+(Math::RandFromTable()%2501)*0.01f;
                        data.axe[1][obj].rot_speed=(Math::RandFromTable()%7201)*0.1f-360.0f;
                        data.axe[2][obj].move_speed=(25.0f+(Math::RandFromTable()%2501)*0.01f)*(Math::RandFromTable()&1 ? 1.0f : -1.0f);
                        data.axe[2][obj].rot_speed=(Math::RandFromTable()%7201)*0.1f-360.0f;
                        data.explode = true;
                        data.explode_time = 1.0f;
                        break;
                    }
                case SCRIPT_TURN_OBJECT:
                    {
                        DEBUG_PRINT_CODE("TURN_OBJECT");
                        int obj=script->script_code[script_id][pos++];
                        int axis=script->script_code[script_id][pos++];
                        int v1=(*script_env)[id].pop();
                        int v2=(*script_env)[id].pop();
                        if (axis!=2)
                        {
                            v1=-v1;
                            v2=-v2;
                        }
                        data.axe[axis][obj].reset_rot();
                        data.axe[axis][obj].is_moving=true;
                        data.is_moving=true;
                        data.axe[axis][obj].rot_angle=-v1*TA2DEG;
                        data.axe[axis][obj].rot_accel=0.0f;
                        data.axe[axis][obj].rot_angle-=data.axe[axis][obj].angle;
                        while(data.axe[axis][obj].rot_angle>180.0f && !isNaN(data.axe[axis][obj].rot_angle))					// Fait le tour dans le sens le plus rapide
                            data.axe[axis][obj].rot_angle-=360.0f;
                        while(data.axe[axis][obj].rot_angle<-180.0f && !isNaN(data.axe[axis][obj].rot_angle))					// Fait le tour dans le sens le plus rapide
                            data.axe[axis][obj].rot_angle+=360.0f;
                        if (data.axe[axis][obj].rot_angle>0.0f)
                            data.axe[axis][obj].rot_speed=fabs(v2*TA2DEG);
                        else
                            data.axe[axis][obj].rot_speed=-fabs(v2*TA2DEG);
                        data.axe[axis][obj].rot_limit=true;
                        data.axe[axis][obj].rot_speed_limit=false;
                        break;
                    }
                case SCRIPT_WAIT_FOR_MOVE:
                    {
                        DEBUG_PRINT_CODE("WAIT_FOR_MOVE");
                        int obj=script->script_code[script_id][pos++];
                        int axis=script->script_code[script_id][pos++];
                        //					float a=data.axe[axis][obj].rot_angle;
                        if (data.axe[axis][obj].move_distance!=0.0f)
                            pos-=3;
                        done=true;
                        break;
                    }
                case SCRIPT_CREATE_LOCAL_VARIABLE:
                    {
                        DEBUG_PRINT_CODE("CREATE_LOCAL_VARIABLE");
                        break;
                    }
                case SCRIPT_SUBTRACT:
                    {
                        DEBUG_PRINT_CODE("SUBSTRACT");
                        int v1=(*script_env)[id].pop();
                        int v2=(*script_env)[id].pop();
                        (*script_env)[id].push(v2-v1);
                        break;
                    }
                case SCRIPT_GET_VALUE_FROM_PORT:
                    {
                        DEBUG_PRINT_CODE("GET_VALUE_FROM_PORT:");
                        int value=(*script_env)[id].pop();
                        DEBUG_PRINT_CODE(value);
                        switch(value)
                        {
                            case MIN_ID:		// returns the lowest valid unit ID number
                                value = 0;
                                break;
                            case MAX_ID:		// returns the highest valid unit ID number
                                value = units.max_unit-1;
                                break;
                            case MY_ID:		// returns ID of current unit
                                value = idx;
                                break;
                            case UNIT_TEAM:		// returns team(player ID in TA) of unit given with parameter
                                value = owner_id;
                                break;
                            case VETERAN_LEVEL:		// gets kills * 100
                                value = 0;			// not yet implemented
                                break;
                            case ATAN:
                                {
                                    int v1=(*script_env)[id].pop();
                                    int v2=(*script_env)[id].pop();
                                    value = (int)(atan((float)v1/v2)+0.5f);
                                }
                                break;
                            case HYPOT:
                                {
                                    int v1=(*script_env)[id].pop();
                                    int v2=(*script_env)[id].pop();
                                    value = (int)(sqrt((float)(v1*v1+v2*v2))+0.5f);
                                }
                                break;
                            case BUGGER_OFF:
                                value = map->check_rect((((int)(Pos.x+map->map_w_d))>>3)-(unit_manager.unit_type[type_id]->FootprintX>>1),(((int)(Pos.z+map->map_h_d))>>3)-(unit_manager.unit_type[type_id]->FootprintZ>>1),unit_manager.unit_type[type_id]->FootprintX,unit_manager.unit_type[type_id]->FootprintZ,idx) ? 0 : 1;
                                break;
                            case BUILD_PERCENT_LEFT:
                                port[ BUILD_PERCENT_LEFT ] = (int)build_percent_left + ( (build_percent_left>(int)build_percent_left) ? 1 : 0 );
                            case YARD_OPEN:
                            case ACTIVATION:
                            case HEALTH:
                            case INBUILDSTANCE:
                            case BUSY:
                            case ARMORED:
                            case STANDINGMOVEORDERS:			// A faire : ajouter le support des ordres de mouvement/feu
                            case STANDINGFIREORDERS:
                                value = port[ value ];
                                break;
                            default:
                                {
                                    const char *op[]={"INCONNU","ACTIVATION","STANDINGMOVEORDERS","STANDINGFIREORDERS","HEALTH","INBUILDSTANCE","BUSY","PIECE_XZ","PIECE_Y",
                                        "UNIT_XZ","UNIT_Y","UNIT_HEIGHT","XZ_ATAN","XZ_HYPOT","ATAN","HYPOT","GROUND_HEIGHT","BUILD_PERCENT_LEFT","YARD_OPEN",
                                        "BUGGER_OFF","ARMORED"};
                                    if (value>20)
                                        value=0;
                                    LOG_DEBUG("GET_VALUE_FROM_PORT: opération non gérée : " << op[value]);
                                }
                                break;
                        };
                        (*script_env)[id].push(value);
                    }
                    break;
                case SCRIPT_LESS_EQUAL:
                    {
                        DEBUG_PRINT_CODE("LESS_EQUAL");
                        int v2=(*script_env)[id].pop();
                        int v1=(*script_env)[id].pop();
                        (*script_env)[id].push(v1<=v2 ? 1 : 0);
                        break;
                    }
                case SCRIPT_SPIN_OBJECT:
                    {
                        DEBUG_PRINT_CODE("SPIN_OBJECT");
                        int obj=script->script_code[script_id][pos++];
                        int axis=script->script_code[script_id][pos++];
                        int v1=(*script_env)[id].pop();
                        int v2=(*script_env)[id].pop();
                        if (axis==1)
                        {
                            v1=-v1;
                            v2=-v2;
                        }
                        data.axe[axis][obj].reset_rot();
                        data.axe[axis][obj].is_moving=true;
                        data.is_moving=true;
                        data.axe[axis][obj].rot_limit=false;
                        data.axe[axis][obj].rot_speed_limit=true;
                        data.axe[axis][obj].rot_target_speed=v1*TA2DEG;
                        if (v2)
                        {
                            if (data.axe[axis][obj].rot_target_speed>data.axe[axis][obj].rot_speed)
                                data.axe[axis][obj].rot_accel=fabs(v2*TA2DEG);
                            else
                                data.axe[axis][obj].rot_accel=-fabs(v2*TA2DEG);
                        }
                        else {
                            data.axe[axis][obj].rot_accel=0;
                            data.axe[axis][obj].rot_speed=data.axe[axis][obj].rot_target_speed;
                        }
                    }
                    break;
                case SCRIPT_SLEEP:
                    {
                        DEBUG_PRINT_CODE("SLEEP");
                        (*script_env)[id].wait=(*script_env)[id].pop()*0.001f;
                        done=true;
                        break;
                    }
                case SCRIPT_MULTIPLY:
                    {
                        DEBUG_PRINT_CODE("MULTIPLY");
                        int v1=(*script_env)[id].pop();
                        int v2=(*script_env)[id].pop();
                        (*script_env)[id].push(v1*v2);
                        break;
                    }
                case SCRIPT_CALL_SCRIPT:
                    {
                        DEBUG_PRINT_CODE("CALL_SCRIPT");
                        int function_id=script->script_code[script_id][pos];			// Lit un code
                        ++pos;
                        int num_param=script->script_code[script_id][pos];			// Lit un code
                        ++pos;
                        (*script_env)[id].env->cur=script_id+(pos<<8);
                        SCRIPT_ENV_STACK *old=(*script_env)[id].env;
                        (*script_env)[id].env=new SCRIPT_ENV_STACK();
                        (*script_env)[id].env->init();
                        (*script_env)[id].env->next=old;
                        (*script_env)[id].env->cur=function_id;
                        for(int i=num_param-1;i>=0;i--)		// Lit les paramètres
                            (*script_env)[id].env->var[i]=(*script_env)[id].pop();
                        done=true;
                        pos=0;
                        script_id=function_id;
                        break;
                    }
                case SCRIPT_SHOW_OBJECT:
                    {
                        DEBUG_PRINT_CODE("SHOW_OBJECT");
                        data.flag[script->script_code[script_id][pos++]]&=(~FLAG_HIDE);
                        break;
                    }
                case SCRIPT_EQUAL:
                    {
                        DEBUG_PRINT_CODE("EQUAL");
                        int v1=(*script_env)[id].pop();
                        int v2=(*script_env)[id].pop();
                        (*script_env)[id].push(v1==v2 ? 1 : 0);
                        break;
                    }
                case SCRIPT_NOT_EQUAL:
                    {
                        DEBUG_PRINT_CODE("NOT_EQUAL");
                        int v1 = (*script_env)[id].pop();
                        int v2 = (*script_env)[id].pop();
                        (*script_env)[id].push(v1!=v2 ? 1 : 0);
                        break;
                    }
                case SCRIPT_IF:
                    {
                        DEBUG_PRINT_CODE("IF");
                        if ((*script_env)[id].pop()!=0)
                            pos++;
                        else
                        {
                            int target_offset=script->script_code[script_id][pos];			// Lit un code
                            pos=target_offset-script->dec_offset[script_id];								// Déplace l'éxecution
                        }
                        break;
                    }
                case SCRIPT_HIDE_OBJECT:
                    {
                        DEBUG_PRINT_CODE("HIDE_OBJECT");
                        data.flag[script->script_code[script_id][pos++]]|=FLAG_HIDE;
                        break;
                    }
                case SCRIPT_SIGNAL:
                    {
                        DEBUG_PRINT_CODE("SIGNAL");
                        (*script_env)[id].env->cur=script_id+(pos<<8);			// Sauvegarde la position
                        raise_signal((*script_env)[id].pop());					// Tue tout les processus utilisant ce signal
                        return 0;
                    }
                case SCRIPT_DONT_CACHE:
                    {
                        DEBUG_PRINT_CODE("DONT_CACHE");
                        ++pos;
                        break;
                    }
                case SCRIPT_SET_SIGNAL_MASK:
                    {
                        DEBUG_PRINT_CODE("SET_SIGNAL_MASK");
                        (*script_env)[id].env->signal_mask=(*script_env)[id].pop();
                        break;
                    }
                case SCRIPT_NOT:
                    {
                        DEBUG_PRINT_CODE("NOT");
                        (*script_env)[id].push(!(*script_env)[id].pop());
                        break;
                    }
                case SCRIPT_DONT_SHADE:
                    {
                        DEBUG_PRINT_CODE("DONT_SHADE");
                        ++pos;
                        break;
                    }
                case SCRIPT_EMIT_SFX:
                    {
                        DEBUG_PRINT_CODE("EMIT_SFX:");
                        int smoke_type=(*script_env)[id].pop();
                        int from_piece=script->script_code[script_id][pos++];
                        DEBUG_PRINT_CODE("smoke_type "<< smoke_type << " from " << from_piece);
                        if (visible)
                        {
                            compute_model_coord();
                            if (data.dir[from_piece].x!=0.0f || data.dir[from_piece].y!=0.0f || data.dir[from_piece].z!=0.0f)
                            {
                                Vector3D dir=data.dir[from_piece];
                                switch(smoke_type)
                                {
                                    case 0:
                                        particle_engine.emit_part(Pos+data.pos[from_piece],dir,fire,1,10.0f,2.5f,5.0f,true);
                                        break;
                                    case 2:
                                    case 3:
                                        particle_engine.emit_part(Pos+data.pos[from_piece],dir,0,1,10.0f,10.0f,10.0f,false, 0.3f);
                                        break;
                                    case 257:			// Fumée
                                    case 258:
                                        particle_engine.emit_part(Pos+data.pos[from_piece],dir,0,1,10.0f,10.0f,10.0f,true, 0.3f);
                                        break;
                                }
                            }
                            else
                                switch(smoke_type)
                                {
                                    case 0:
                                        particle_engine.make_smoke(Pos+data.pos[from_piece],fire,1,0.0f,0.0f,0.0f,0.5f);
                                        break;
                                    case 257:
                                    case 258:
                                        particle_engine.make_smoke(Pos+data.pos[from_piece],0,1,10.0f,-1.0f,0.0f,0.5f);
                                        break;
                                }
                        }
                    }
                    break;
                case SCRIPT_PUSH_CONST:
                    {
                        DEBUG_PRINT_CODE("PUSH_CONST (" << script->script_code[script_id][pos] << ")");
                        (*script_env)[id].push(script->script_code[script_id][pos]);
                        ++pos;
                        break;
                    }
                case SCRIPT_PUSH_VAR:
                    {
                        DEBUG_PRINT_CODE("PUSH_VAR (" << script->script_code[script_id][pos] << ") = "
                                         << (*script_env)[id].env->var[script->script_code[script_id][pos]]);
                        (*script_env)[id].push((*script_env)[id].env->var[script->script_code[script_id][pos]]);
                        ++pos;
                        break;
                    }
                case SCRIPT_SET_VAR:
                    {
                        DEBUG_PRINT_CODE("SET_VAR (" << script->script_code[script_id][pos] << ")");
                        (*script_env)[id].env->var[script->script_code[script_id][pos]]=(*script_env)[id].pop();
                        ++pos;
                        break;
                    }
                case SCRIPT_PUSH_STATIC_VAR:
                    {
                        DEBUG_PRINT_CODE("PUSH_STATIC_VAR");
                        if (script->script_code[script_id][pos] >= s_var->size() )
                            s_var->resize( script->script_code[script_id][pos] + 1 );
                        (*script_env)[id].push((*s_var)[script->script_code[script_id][pos]]);
                        ++pos;
                        break;
                    }
                case SCRIPT_SET_STATIC_VAR:
                    {
                        DEBUG_PRINT_CODE("SET_STATIC_VAR");
                        if (script->script_code[script_id][pos] >= s_var->size() )
                            s_var->resize( script->script_code[script_id][pos] + 1 );
                        (*s_var)[script->script_code[script_id][pos]]=(*script_env)[id].pop();
                        ++pos;
                        break;
                    }
                case SCRIPT_OR:
                    {
                        DEBUG_PRINT_CODE("OR");
                        int v1=(*script_env)[id].pop(),v2=(*script_env)[id].pop();
                        (*script_env)[id].push(v1|v2);
                        break;
                    }
                case SCRIPT_START_SCRIPT:				// Transfère l'éxecution à un autre script
                    {
                        DEBUG_PRINT_CODE("START_SCRIPT");
                        int function_id=script->script_code[script_id][pos++];			// Lit un code
                        int num_param=script->script_code[script_id][pos++];			// Lit un code
                        int s_id=launch_script(function_id, 0, NULL, true);
                        if (s_id>=0)
                        {
                            for(int i=num_param-1;i>=0;i--)		// Lit les paramètres
                                (*script_env)[s_id].env->var[i]=(*script_env)[id].pop();
                            (*script_env)[s_id].env->signal_mask=(*script_env)[id].env->signal_mask;
                        }
                        else
                        {
                            for (int i=0;i<num_param; ++i)		// Enlève les paramètres de la pile
                                (*script_env)[id].pop();
                        }
                        done=true;
                        break;
                    }
                case SCRIPT_RETURN:		// Retourne au script précédent
                    {
                        DEBUG_PRINT_CODE("RETURN");
                        if (script_val->size() <= script_id )
                            script_val->resize( script_id + 1 );
                        (*script_val)[script_id]=(*script_env)[id].env->var[0];
                        SCRIPT_ENV_STACK *old=(*script_env)[id].env;
                        (*script_env)[id].env=(*script_env)[id].env->next;
                        delete old;
                        (*script_env)[id].pop();		// Enlève la valeur retournée
                        if ((*script_env)[id].env)
                        {
                            script_id=((*script_env)[id].env->cur&0xFF);			// Récupère l'identifiant du script en cours d'éxecution et la position d'éxecution
                            pos=((*script_env)[id].env->cur>>8);
                        }
                        done=true;
                        break;
                    }
                case SCRIPT_JUMP:						// Commande de saut
                    {
                        DEBUG_PRINT_CODE("JUMP");
                        int target_offset=script->script_code[script_id][pos];			// Lit un code
                        pos=target_offset-script->dec_offset[script_id];								// Déplace l'éxecution
                        break;
                    }
                case SCRIPT_ADD:
                    {
                        DEBUG_PRINT_CODE("ADD");
                        int v1=(*script_env)[id].pop();
                        int v2=(*script_env)[id].pop();
                        (*script_env)[id].push(v1+v2);
                        break;
                    }
                case SCRIPT_STOP_SPIN:
                    {
                        DEBUG_PRINT_CODE("STOP_SPIN");
                        int obj=script->script_code[script_id][pos++];
                        int axis=script->script_code[script_id][pos++];
                        int v=(*script_env)[id].pop();
                        if (axis!=2)
                            v=-v;
                        data.axe[axis][obj].reset_rot();
                        data.axe[axis][obj].is_moving=true;
                        data.is_moving=true;
                        data.axe[axis][obj].rot_limit=false;
                        data.axe[axis][obj].rot_speed_limit=true;
                        data.axe[axis][obj].rot_target_speed=0.0f;
                        if (v==0)
                        {
                            data.axe[axis][obj].rot_speed=0.0f;
                            data.axe[axis][obj].rot_accel=0.0f;
                        }
                        else
                        {
                            if (data.axe[axis][obj].rot_speed>0.0f)
                                data.axe[axis][obj].rot_accel=-abs(v);
                            else
                                data.axe[axis][obj].rot_accel=abs(v);
                        }
                        break;
                    }
                case SCRIPT_DIVIDE:
                    {
                        DEBUG_PRINT_CODE("DIVIDE");
                        int v1=(*script_env)[id].pop();
                        int v2=(*script_env)[id].pop();
                        (*script_env)[id].push(v2 / v1);
                        break;
                    }
                case SCRIPT_MOVE_PIECE_NOW:
                    {
                        DEBUG_PRINT_CODE("MOVE_PIECE_NOW");
                        int obj=script->script_code[script_id][pos++];
                        int axis=script->script_code[script_id][pos++];
                        data.axe[axis][obj].reset_move();
                        data.axe[axis][obj].is_moving=true;
                        data.is_moving=true;
                        if (axis==0)
                            data.axe[axis][obj].pos=-(*script_env)[id].pop()*div;
                        else
                            data.axe[axis][obj].pos=(*script_env)[id].pop()*div;
                        break;
                    }
                case SCRIPT_TURN_PIECE_NOW:
                    {
                        DEBUG_PRINT_CODE("TURN_PIECE_NOW");
                        int obj=script->script_code[script_id][pos++];
                        int axis=script->script_code[script_id][pos++];
                        int v=(*script_env)[id].pop();
                        data.axe[axis][obj].reset_rot();
                        data.axe[axis][obj].is_moving=true;
                        data.is_moving=true;
                        if (axis!=2)
                            v=-v;
                        data.axe[axis][obj].angle=-v*TA2DEG;
                        break;
                    }
                case SCRIPT_CACHE:
                    DEBUG_PRINT_CODE("CACHE");
                    ++pos;
                    break;	//added
                case SCRIPT_COMPARE_AND:
                    {
                        DEBUG_PRINT_CODE("COMPARE_AND");
                        int v1=(*script_env)[id].pop();
                        int v2=(*script_env)[id].pop();
                        (*script_env)[id].push(v1 && v2);
                        break;
                    }
                case SCRIPT_COMPARE_OR:
                    {
                        DEBUG_PRINT_CODE("COMPARE_OR");
                        int v1=(*script_env)[id].pop();
                        int v2=(*script_env)[id].pop();
                        (*script_env)[id].push(v1 || v2);
                        break;
                    }
                case SCRIPT_CALL_FUNCTION:
                    {
                        DEBUG_PRINT_CODE("CALL_FUNCTION");
                        int function_id=script->script_code[script_id][pos++];			// Lit un code
                        int num_param=script->script_code[script_id][pos++];			// Lit un code
                        (*script_env)[id].env->cur=script_id+(pos<<8);
                        SCRIPT_ENV_STACK *old=(*script_env)[id].env;
                        (*script_env)[id].env=new SCRIPT_ENV_STACK();
                        (*script_env)[id].env->init();
                        (*script_env)[id].env->next=old;
                        (*script_env)[id].env->cur=function_id;
                        for(int i=num_param-1;i>=0;i--)		// Lit les paramètres
                            (*script_env)[id].env->var[i]=(*script_env)[id].pop();
                        done=true;
                        pos = 0;
                        script_id=function_id;
                        break;
                    }
                case SCRIPT_GET:
                    {
                        DEBUG_PRINT_CODE("GET *");
                        (*script_env)[id].pop();
                        (*script_env)[id].pop();
                        int v2=(*script_env)[id].pop();
                        int v1=(*script_env)[id].pop();
                        int val=(*script_env)[id].pop();
                        switch(val)
                        {
                            case UNIT_TEAM:		// returns team(player ID in TA) of unit given with parameter
                                if (v1 >= 0 && v1 < units.max_unit && (units.unit[ v1 ].flags & 1) )
                                    (*script_env)[id].push( units.unit[ v1 ].owner_id );
                                else
                                    (*script_env)[id].push(-1);
                                break;
                            case UNIT_BUILD_PERCENT_LEFT:		// basically BUILD_PERCENT_LEFT, but comes with a unit parameter
                                if (v1 >= 0 && v1 < units.max_unit && (units.unit[ v1 ].flags & 1) )
                                    (*script_env)[id].push((int)units.unit[ v1 ].build_percent_left + ( (units.unit[ v1 ].build_percent_left > (int)units.unit[ v1 ].build_percent_left) ? 1 : 0));
                                else
                                    (*script_env)[id].push(0);
                                break;
                            case UNIT_ALLIED:		// is unit given with parameter allied to the unit of the current COB script. 0=not allied, not zero allied
                                (*script_env)[id].push( !isEnemy( v1 ) );
                                break;
                            case UNIT_IS_ON_THIS_COMP:		// indicates if the 1st parameter(a unit ID) is local to this computer
                                if (v1 >= 0 && v1 < units.max_unit && (units.unit[ v1 ].flags & 1) )
                                    (*script_env)[id].push( !(players.control[ units.unit[ v1 ].owner_id ] & PLAYER_CONTROL_FLAG_REMOTE) );
                                else
                                    (*script_env)[id].push(0);
                                break;
                            case BUILD_PERCENT_LEFT:
                                port[ BUILD_PERCENT_LEFT ] = (int)build_percent_left + ( (build_percent_left > (int)build_percent_left) ? 1 : 0);
                            case ACTIVATION:
                            case STANDINGMOVEORDERS:
                            case STANDINGFIREORDERS:
                            case HEALTH:
                            case INBUILDSTANCE:
                            case BUSY:
                            case YARD_OPEN:
                            case BUGGER_OFF:
                            case ARMORED:
                                (*script_env)[id].push((int)port[val]);
                                break;
                            case PIECE_XZ:
                                compute_model_coord();
                                (*script_env)[id].push( PACKXZ((data.pos[v1].x+Pos.x)*2.0f+map->map_w, (data.pos[v1].z+Pos.z)*2.0f+map->map_h));
#if DEBUG_USE_PRINT_CODE == 1
                                {
                                    int c = (*script_env)[id].pop();
                                    (*script_env)[id].push( c );
                                    DEBUG_PRINT_CODE("PIECE_XZ = " << c);
                                }
#endif
                                break;
                            case PIECE_Y:
                                compute_model_coord();
                                (*script_env)[id].push((int)((data.pos[v1].y + Pos.y)*2.0f)<<16);
#if DEBUG_USE_PRINT_CODE == 1
                                {
                                    int c = (*script_env)[id].pop();
                                    (*script_env)[id].push( c );
                                    DEBUG_PRINT_CODE("PIECE_Y = " << c);
                                }
#endif
                                break;
                            case UNIT_XZ:
                                if (v1 >= 0 && v1<units.max_unit && (units.unit[v1].flags & 1) )
                                    (*script_env)[id].push( PACKXZ( units.unit[v1].Pos.x*2.0f+map->map_w, units.unit[v1].Pos.z*2.0f+map->map_h ));
                                else
                                    (*script_env)[id].push( PACKXZ( Pos.x*2.0f+map->map_w, Pos.z*2.0f+map->map_h ));
#if DEBUG_USE_PRINT_CODE == 1
                                {
                                    int c = (*script_env)[id].pop();
                                    (*script_env)[id].push( c );
                                    DEBUG_PRINT_CODE("UNIT_XY = " << c);
                                }
#endif
                                break;
                            case UNIT_Y:
                                if (v1 >= 0 && v1<units.max_unit && (units.unit[v1].flags & 1) )
                                    (*script_env)[id].push((int)(units.unit[v1].Pos.y * 2.0f)<<16);
                                else
                                    (*script_env)[id].push((int)(Pos.y * 2.0f)<<16);
#if DEBUG_USE_PRINT_CODE == 1
                                {
                                    int c = (*script_env)[id].pop();
                                    (*script_env)[id].push( c );
                                    DEBUG_PRINT_CODE("UNIT_Y = " << c);
                                }
#endif
                                break;
                            case UNIT_HEIGHT:
                                if (v1 >= 0 && v1<units.max_unit && (units.unit[v1].flags & 1) )
                                    (*script_env)[id].push((int)(units.unit[v1].model->top * 2.0f)<<16);
                                else
                                    (*script_env)[id].push((int)(model->top * 2.0f)<<16);
#if DEBUG_USE_PRINT_CODE == 1
                                {
                                    int c = (*script_env)[id].pop();
                                    (*script_env)[id].push( c );
                                    DEBUG_PRINT_CODE("UNIT_HEIGHT =" << c);
                                }
#endif
                                break;
                            case XZ_ATAN:
                                (*script_env)[id].push((int)(atan2( UNPACKX(v1) , UNPACKZ(v1) ) * RAD2TA - Angle.y * DEG2TA) + 32768);
#if DEBUG_USE_PRINT_CODE == 1
                                {
                                    int c = (*script_env)[id].pop();
                                    (*script_env)[id].push( c );
                                    DEBUG_PRINT_CODE("XZ_ATAN[(" << v1 << ") = ("
                                                     << UNPACKX(v1) << "," << UNPACKZ(v1) << ")] = " << c);
                                }
#endif
                                break;
                            case XZ_HYPOT:
                                (*script_env)[id].push((int)hypot( UNPACKX(v1), UNPACKZ(v1) )<<16);
#if DEBUG_USE_PRINT_CODE == 1
                                {
                                    int c = (*script_env)[id].pop();
                                    (*script_env)[id].push( c );
                                    DEBUG_PRINT_CODE("XZ_HYPOT[(" << v1 << ") = ("
                                                     << UNPACKX(v1) << "," << UNPACKZ(v1) << ")] = "<< c);
                                }
#endif
                                break;
                            case ATAN:
                                (*script_env)[id].push((int)(atan2(v1,v2) * RAD2TA ));
#if DEBUG_USE_PRINT_CODE == 1
                                {
                                    int c = (*script_env)[id].pop();
                                    (*script_env)[id].push( c );
                                    DEBUG_PRINT_CODE("ATAN =" << c);
                                }
#endif
                                break;
                            case HYPOT:
                                (*script_env)[id].push((int)hypot(v1,v2));
#if DEBUG_USE_PRINT_CODE == 1
                                {
                                    int c = (*script_env)[id].pop();
                                    (*script_env)[id].push( c );
                                    DEBUG_PRINT_CODE("HYPOT(" << v1 << "," v2 << ") = " << c);
                                }
#endif
                                break;
                            case GROUND_HEIGHT:
                                (*script_env)[id].push((int)(map->get_unit_h(( UNPACKX(v1) - map->map_w)*0.5f,( UNPACKZ(v1) - map->map_h)*0.5f)*2.0f)<<16);
                                break;
                            default:
                                printf("GET constante inconnue %d\n", val);
                        }
                        break;	//added
                    }
                case SCRIPT_SET_VALUE:
                    {
                        DEBUG_PRINT_CODE("SET_VALUE *:");
                        int v1=(*script_env)[id].pop();
                        int v2=(*script_env)[id].pop();
                        DEBUG_PRINT_CODE(v1 << " " << v2);
                        switch(v2)
                        {
                            case ACTIVATION:
                                if (v1 == 0 )
                                    deactivate();
                                else
                                    activate();
                                break;
                            case YARD_OPEN:
                                port[v2] = v1;
                                if (!map->check_rect((((int)(Pos.x+map->map_w_d))>>3)-(unit_manager.unit_type[type_id]->FootprintX>>1),(((int)(Pos.z+map->map_h_d))>>3)-(unit_manager.unit_type[type_id]->FootprintZ>>1),unit_manager.unit_type[type_id]->FootprintX,unit_manager.unit_type[type_id]->FootprintZ,idx))
                                    port[v2] ^= 1;
                                break;
                            case BUGGER_OFF:
                                port[v2]=v1;
                                if (port[v2])
                                {
                                    int px=((int)(Pos.x)+map->map_w_d)>>3;
                                    int py=((int)(Pos.z)+map->map_h_d)>>3;
                                    for(int y=py-(unit_manager.unit_type[type_id]->FootprintZ>>1);y<=py+(unit_manager.unit_type[type_id]->FootprintZ>>1);y++)
                                    {
                                        if (y>=0 && y<(map->bloc_h<<1)-1)
                                        {
                                            for(int x=px-(unit_manager.unit_type[type_id]->FootprintX>>1);x<=px+(unit_manager.unit_type[type_id]->FootprintX>>1);x++)
                                            {
                                                if (x>=0 && x<(map->bloc_w<<1)-1)
                                                {
                                                    if (map->map_data[y][x].unit_idx >= 0 && map->map_data[y][x].unit_idx!=idx )
                                                    {
                                                        int cur_idx=map->map_data[y][x].unit_idx;
                                                        if (units.unit[cur_idx].owner_id==owner_id && units.unit[cur_idx].build_percent_left == 0.0f && (units.unit[cur_idx].mission==NULL || units.unit[cur_idx].mission->mission!=MISSION_MOVE)) {
                                                            units.unit[cur_idx].lock();
                                                            Vector3D target = units.unit[cur_idx].Pos;
                                                            target.z+=100.0f;
                                                            units.unit[cur_idx].add_mission(MISSION_MOVE | MISSION_FLAG_AUTO,&target,true);
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
                                port[v2]=v1;
                        }
                    }
                    break;	//added
                case SCRIPT_ATTACH_UNIT:
                    {
                        DEBUG_PRINT_CODE("ATTACH_UNIT");
                        /*int v3 =*/ (*script_env)[id].pop();
                        int v2 = (*script_env)[id].pop();
                        int v1 = (*script_env)[id].pop();
                        if (v1 >= 0 && v1 < units.max_unit && units.unit[ v1 ].flags )
                        {
                            UNIT *target_unit = &(units.unit[v1]);
                            target_unit->hidden = (v2 < 0);
                            bool already_in = false;
                            if (target_unit->attached )
                                for( int i = 0 ; i < nb_attached ; i++ )		// Check if this unit is already in
                                {
                                    if (attached_list[ i ] == v1 )
                                    {
                                        already_in = true;
                                        link_list[ i ] = v2;
                                    }
                                }
                            if (!already_in )
                            {
                                link_list[nb_attached]=v2;
                                attached_list[nb_attached++]=target_unit->idx;
                            }
                            target_unit->attached=true;
                            if (!already_in )
                                target_unit->clear_from_map();
                        }
                        break;	//added
                    }
                case SCRIPT_DROP_UNIT:
                    {
                        DEBUG_PRINT_CODE("DROP_UNIT *");
                        int v1 = (*script_env)[id].pop();
                        DEBUG_PRINT_CODE("Dropping " << v1);
                        if (v1 >= 0 && v1 < units.max_unit && units.unit[ v1 ].flags)
                        {
                            UNIT *target_unit = &(units.unit[v1]);
                            target_unit->attached = false;
                            target_unit->hidden = false;
                            nb_attached--;					// Remove the unit from the attached list
                            for( int i = 0 ; i < nb_attached ; i++ )
                            {
                                if (attached_list[ i ] == v1 )
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
                        break;	//added
                    }
                default:
                    LOG_ERROR("UNKNOWN " << script->script_code[script_id][--pos] << ", Stopping script");
                    {
                        if (script_val->size() <= script_id )
                            script_val->resize( script_id + 1 );
                        (*script_val)[script_id]=(*script_env)[id].env->var[0];
                        SCRIPT_ENV_STACK *old=(*script_env)[id].env;
                        (*script_env)[id].env=(*script_env)[id].env->next;
                        delete old;
                    }
                    if ((*script_env)[id].env)
                    {
                        script_id=((*script_env)[id].env->cur&0xFF);			// Récupère l'identifiant du script en cours d'éxecution et la position d'éxecution
                        pos=((*script_env)[id].env->cur>>8);
                    }
                    else
                        (*script_env)[id].running=false;
                    done = true;
            };
        } while(!done);

        if ((*script_env)[id].env)
            (*script_env)[id].env->cur=script_id+(pos<<8);
        return 0;
    }



    void UNIT::explode()
    {
        exploding = true;
        if (local && network_manager.isConnected() ) // Sync unit destruction (and corpse creation ;) )
        {
            struct event explode_event;
            explode_event.type = EVENT_UNIT_EXPLODE;
            explode_event.opt1 = idx;
            explode_event.opt2 = severity;
            explode_event.x = Pos.x;
            explode_event.y = Pos.y;
            explode_event.z = Pos.z;
            network_manager.sendEvent( &explode_event );
        }

        int power = Math::Max(unit_manager.unit_type[type_id]->FootprintX, unit_manager.unit_type[type_id]->FootprintZ);
        fx_manager.addFlash( Pos, power * 32 );
        fx_manager.addExplosion( Pos, V, power * 3, power * 10 );

        int param[]={ severity * 100 / unit_manager.unit_type[type_id]->MaxDamage, 0 };
        run_script_function(the_map,get_script_index(SCRIPT_killed),2,param);
        if (attached )
            param[1] = 3;			// When we were flying we just disappear
        bool sinking = the_map->get_unit_h( Pos.x, Pos.z ) <= the_map->sealvl;

        switch( param[1] )
        {
            case 1:			// Some good looking corpse
                {
                    pMutex.unlock();
                    flags = 1;				// Set it to 1 otherwise it won't remove it from map
                    clear_from_map();
                    flags = 4;
                    pMutex.lock();
                    if (cur_px > 0 && cur_py > 0 && cur_px < (the_map->bloc_w<<1) && cur_py < (the_map->bloc_h<<1) )
                        if (the_map->map_data[ cur_py ][ cur_px ].stuff == -1)
                        {
                            int type=feature_manager.get_feature_index(unit_manager.unit_type[type_id]->Corpse);
                            if (type >= 0 )
                            {
                                the_map->map_data[ cur_py ][ cur_px ].stuff = features.add_feature(Pos,type);
                                if (the_map->map_data[ cur_py ][ cur_px ].stuff >= 0 ) 	// Keep unit orientation
                                {
                                    features.feature[ the_map->map_data[ cur_py ][ cur_px ].stuff ].angle = Angle.y;
                                    if (sinking )
                                        features.sink_feature( the_map->map_data[ cur_py ][ cur_px ].stuff );
                                    features.drawFeatureOnMap( the_map->map_data[ cur_py ][ cur_px ].stuff );
                                }
                            }
                        }
                }
                break;
            case 2:			// Some exploded corpse
                {
                    pMutex.unlock();
                    flags = 1;				// Set it to 1 otherwise it won't remove it from map
                    clear_from_map();
                    flags = 4;
                    pMutex.lock();
                    if (cur_px > 0 && cur_py > 0 && cur_px < (the_map->bloc_w<<1) && cur_py < (the_map->bloc_h<<1))
                        if (the_map->map_data[ cur_py ][ cur_px ].stuff == -1) {
                            int type=feature_manager.get_feature_index( (String( unit_manager.unit_type[type_id]->name) + "_heap").c_str() );
                            if (type >= 0 ) {
                                the_map->map_data[ cur_py ][ cur_px ].stuff = features.add_feature(Pos,type);
                                if (the_map->map_data[ cur_py ][ cur_px ].stuff >= 0 ) {			// Keep unit orientation
                                    features.feature[ the_map->map_data[ cur_py ][ cur_px ].stuff ].angle = Angle.y;
                                    if (sinking )
                                        features.sink_feature( the_map->map_data[ cur_py ][ cur_px ].stuff );
                                    features.drawFeatureOnMap( the_map->map_data[ cur_py ][ cur_px ].stuff );
                                }
                            }
                        }
                }
                break;
            default:
                flags = 1;		// Nothing replaced just remove the unit from position map
                pMutex.unlock();
                clear_from_map();
                pMutex.lock();
        }
        pMutex.unlock();
        int w_id = weapons.add_weapon(weapon_manager.get_weapon_index( self_destruct == 0.0f ? unit_manager.unit_type[type_id]->SelfDestructAs : unit_manager.unit_type[type_id]->ExplodeAs ),idx);
        pMutex.lock();
        if (w_id >= 0)
        {
            weapons.weapon[w_id].Pos = Pos;
            weapons.weapon[w_id].target_pos = Pos;
            weapons.weapon[w_id].target = -1;
            weapons.weapon[w_id].just_explode = true;
        }
        for (int i = 0; i < data.nb_piece; ++i)
        {
            if (!(data.flag[i]&FLAG_EXPLODE))// || (data.flag[i]&FLAG_EXPLODE && (data.explosion_flag[i]&EXPLODE_BITMAPONLY)))
                data.flag[i]|=FLAG_HIDE;
        }
    }

    float ballistic_angle(float v, float g, float d, float y_s, float y_e) // Calculs de ballistique pour l'angle de tir
    {
        float v2 = v*v;
        float gd = g*d;
        float v2gd = v2/gd;
        float a = v2gd*(4.0f*v2gd-8.0f*(y_e-y_s)/d)-4.0f;
        if (a<0.0f)				// Pas de solution
            return 360.0f;
        return RAD2DEG*atan(v2gd-0.5f*sqrt(a));
    }

    const int UNIT::move(const float dt, MAP* map, int* path_exec, const int key_frame)
    {
        pMutex.lock();

        bool was_open = port[YARD_OPEN] != 0;
        bool was_flying = flying;
        sint32	o_px = cur_px;
        sint32	o_py = cur_py;
        compute_coord = true;
        Vector3D	old_V = V;			// Store the speed, so we can do some calculations
        bool	b_TargetAngle = false;		// Do we aim, move, ... ?? Need to change unit angle
        float	f_TargetAngle = 0.0f;

        Vector3D NPos = Pos;
        int n_px = cur_px;
        int n_py = cur_py;
        bool precomputed_position = false;

        if (type_id < 0 || type_id >= unit_manager.nb_unit || flags == 0 ) // A unit which cannot exist
        {
            pMutex.unlock();
            LOG_ERROR("UNIT::move : A unit which ");
            return	-1;		// Should NEVER happen
        }

        if (build_percent_left == 0.0f && unit_manager.unit_type[type_id]->isfeature) // Turn this unit into a feature
        {
            if (cur_px > 0 && cur_py > 0 && cur_px < (map->bloc_w<<1) && cur_py < (map->bloc_h<<1) )
            {
                if (map->map_data[ cur_py ][ cur_px ].stuff == -1)
                {
                    int type = feature_manager.get_feature_index(unit_manager.unit_type[type_id]->Corpse);
                    if (type >= 0)
                    {
                        features.lock();
                        map->map_data[ cur_py ][ cur_px ].stuff=features.add_feature(Pos,type);
                        if (map->map_data[ cur_py ][ cur_px ].stuff == -1)
                            LOG_ERROR("Could not turn `" << unit_manager.unit_type[type_id]->Unitname << "` into a feature ! Cannot create the feature");
                        else
                            features.feature[map->map_data[ cur_py ][ cur_px ].stuff].angle = Angle.y;
                        pMutex.unlock();
                        clear_from_map();
                        pMutex.lock();
                        features.drawFeatureOnMap( map->map_data[ cur_py ][ cur_px ].stuff );
                        features.unlock();
                        flags = 4;
                    }
                    else
                        LOG_ERROR("Could not turn `" << unit_manager.unit_type[type_id]->Unitname << "` into a feature ! Feature not found");
                }
            }
            pMutex.unlock();
            return -1;
        }

        if (map->ota_data.waterdoesdamage && Pos.y < map->sealvl)		// The unit is damaged by the "acid" water
            hp -= dt*map->ota_data.waterdamage;

        bool jump_commands = (((idx+key_frame)&0xF) == 0);		// Saute certaines commandes / Jump some commands so it runs faster with lots of units

        if (build_percent_left == 0.0f && self_destruct >= 0.0f) // Self-destruction code
        {
            int old = (int)self_destruct;
            self_destruct -= dt;
            if (old != (int)self_destruct) // Play a sound :-)
                play_sound( format( "count%d", old));
            if (self_destruct <= 0.0f)
            {
                self_destruct = 0.0f;
                hp = 0.0f;
                severity = unit_manager.unit_type[type_id]->MaxDamage;
            }
        }

        if (hp<=0.0f && (local || exploding)) // L'unité est détruite
        {
            if (mission
                && !unit_manager.unit_type[type_id]->BMcode
                && ( mission->mission == MISSION_BUILD_2 || mission->mission == MISSION_BUILD )		// It was building something that we must destroy too
                && mission->p != NULL )
            {
                ((UNIT*)(mission->p))->lock();
                ((UNIT*)(mission->p))->hp = 0.0f;
                ((UNIT*)(mission->p))->built = false;
                ((UNIT*)(mission->p))->unlock();
            }
            death_timer++;
            if (death_timer == 255 ) // Ok we've been dead for a long time now ...
            {
                pMutex.unlock();
                return -1;
            }
            switch(flags&0x17)
            {
                case 1:				// Début de la mort de l'unité	(Lance le script)
                    flags = 4;		// Don't remove the data on the position map because they will be replaced
                    if (build_percent_left == 0.0f && local )
                        explode();
                    else
                        flags = 1;
                    death_delay=1.0f;
                    if (flags == 1 )
                    {
                        pMutex.unlock();
                        return -1;
                    }
                    break;
                case 4:				// Vérifie si le script est terminé
                    if (death_delay<=0.0f || !data.explode )
                    {
                        flags = 1;
                        pMutex.unlock();
                        clear_from_map();
                        return -1;
                    }
                    death_delay -= dt;
                    for(int i=0;i<data.nb_piece;i++)
                        if (!(data.flag[i]&FLAG_EXPLODE))// || (data.flag[i]&FLAG_EXPLODE && (data.explosion_flag[i]&EXPLODE_BITMAPONLY)))
                            data.flag[i]|=FLAG_HIDE;
                    break;
                case 0x14:				// Unit has been captured, this is a FAKE unit, just here to be removed
                    flags=4;
                    return -1;
                default:		// It doesn't explode (it has been reclaimed for example)
                    flags=1;
                    pMutex.unlock();
                    clear_from_map();
                    return -1;
            }
            if (data.nb_piece>0 && build_percent_left == 0.0f)
            {
                data.move(dt,map->ota_data.gravity);
                if (c_time>=0.1f)
                {
                    c_time=0.0f;
                    for(int i=0;i<data.nb_piece;i++)
                        if (data.flag[i]&FLAG_EXPLODE && (data.explosion_flag[i]&EXPLODE_BITMAPONLY)!=EXPLODE_BITMAPONLY)
                        {
                            if (data.explosion_flag[i]&EXPLODE_FIRE)
                            {
                                compute_model_coord();
                                particle_engine.make_smoke(Pos+data.pos[i],fire,1,0.0f,0.0f);
                            }
                            if (data.explosion_flag[i]&EXPLODE_SMOKE)
                            {
                                compute_model_coord();
                                particle_engine.make_smoke(Pos+data.pos[i],0,1,0.0f,0.0f);
                            }
                        }
                }
            }
            goto script_exec;
        }
        else if (!jump_commands && do_nothing() && local)
            if (Pos.x<-map->map_w_d || Pos.x>map->map_w_d || Pos.z<-map->map_h_d || Pos.z>map->map_h_d) {
                Vector3D target = Pos;
                if (target.x < -map->map_w_d+256)
                    target.x = -map->map_w_d+256;
                else if (target.x > map->map_w_d-256)
                    target.x = map->map_w_d-256;
                if (target.z < -map->map_h_d+256)
                    target.z = -map->map_h_d+256;
                else if (target.z > map->map_h_d-256)
                    target.z = map->map_h_d-256;
                add_mission(MISSION_MOVE | MISSION_FLAG_AUTO,&target,true,0,NULL,NULL,0,1);		// Stay on map
            }

        flags &= 0xEF;		// To fix a bug

        if (build_percent_left > 0.0f) // Unit isn't finished
        {
            if (!built && local)
            {
                float frac = 1000.0f / ( 6 * unit_manager.unit_type[type_id]->BuildTime );
                metal_prod = frac * unit_manager.unit_type[type_id]->BuildCostMetal;
                frac *= dt;
                hp -= frac * unit_manager.unit_type[type_id]->MaxDamage;
                build_percent_left += frac * 100.0f;
            }
            goto script_exec;
        }
        else
        {
            if (hp < unit_manager.unit_type[type_id]->MaxDamage && unit_manager.unit_type[type_id]->HealTime > 0)
            {
                hp += unit_manager.unit_type[type_id]->MaxDamage * dt / unit_manager.unit_type[type_id]->HealTime;
                if (hp > unit_manager.unit_type[type_id]->MaxDamage )
                    hp = unit_manager.unit_type[type_id]->MaxDamage;
            }
        }

        if (data.nb_piece>0)
            data.move(dt,units.g_dt);

        if (cloaking)
        {
            int conso_energy = (mission == NULL || !(mission->flags & MISSION_FLAG_MOVE) ) ? unit_manager.unit_type[type_id]->CloakCost : unit_manager.unit_type[type_id]->CloakCostMoving;
            if (players.energy[ owner_id ] >= (energy_cons + conso_energy) * dt ) {
                energy_cons += conso_energy;
                int dx = unit_manager.unit_type[type_id]->mincloakdistance >> 3;
                // byte mask = 1 << owner_id;
                bool found = false;
                for(int y = cur_py - dx ; y <= cur_py + dx && !found ; y++ )
                    if (y >= 0 && y < map->bloc_h_db - 1 )
                        for(int x = cur_px - dx ; x <= cur_px + dx ; x++ )
                            if (x >= 0 && x < map->bloc_w_db - 1 ) {
                                int cur_idx = map->map_data[y][x].unit_idx;

                                if (cur_idx>=0 && cur_idx < units.max_unit && (units.unit[cur_idx].flags & 1) && units.unit[cur_idx].owner_id != owner_id
                                    && SQUARE(unit_manager.unit_type[type_id]->mincloakdistance) < (Pos - units.unit[ cur_idx ].Pos).sq() )
                                {
                                    found = true;
                                    break;
                                }
                            }
                cloaked = !found;
            }
            else
                cloaked = false;
        }
        else
            cloaked = false;

        if (paralyzed > 0.0f)       // This unit is paralyzed
        {
            paralyzed -= dt;
            if (unit_manager.unit_type[type_id]->model)
            {
                Vector3D randVec;
                bool random_vector=false;
                int n = 0;
                for ( int base_n = Math::RandFromTable() ; !random_vector && n < unit_manager.unit_type[type_id]->model->obj.nb_sub_obj ; n++ )
                    random_vector = unit_manager.unit_type[type_id]->model->obj.random_pos( &data, (base_n + n) % unit_manager.unit_type[type_id]->model->obj.nb_sub_obj, &randVec );
                if (random_vector)
                    fx_manager.addElectric( Pos + randVec );
            }
        }

        if (attached || paralyzed > 0.0f)
            goto script_exec;

        if (unit_manager.unit_type[type_id]->canload && nb_attached > 0)
        {
            int e = 0;
            compute_model_coord();
            for (int i = 0; i + e < nb_attached; ++i)
            {
                if (units.unit[attached_list[i]].flags)
                {
                    units.unit[attached_list[i]].Pos = Pos+data.pos[link_list[i]];
                    units.unit[attached_list[i]].cur_px = ((int)(units.unit[attached_list[i]].Pos.x)+map->map_w_d)>>3;
                    units.unit[attached_list[i]].cur_py = ((int)(units.unit[attached_list[i]].Pos.z)+map->map_h_d)>>3;
                    units.unit[attached_list[i]].Angle = Angle;
                }
                else
                {
                    ++e;
                    --i;
                    continue;
                }
                attached_list[i]=attached_list[i+e];
            }
            nb_attached-=e;
        }

        if (planned_weapons > 0.0f)	// Construit des armes / build weapons
        {
            float old=planned_weapons-(int)planned_weapons;
            int idx=-1;
            for( int i = 0 ; i < unit_manager.unit_type[type_id]->weapon.size() ; i++ )
                if (unit_manager.unit_type[type_id]->weapon[i] && unit_manager.unit_type[type_id]->weapon[i]->stockpile)
                {
                    idx=i;
                    break;
                }
            if (idx!=-1 && unit_manager.unit_type[type_id]->weapon[idx]->reloadtime!=0.0f)
            {
                float dn=dt/unit_manager.unit_type[type_id]->weapon[idx]->reloadtime;
                float conso_metal=((float)unit_manager.unit_type[type_id]->weapon[idx]->metalpershot)/unit_manager.unit_type[type_id]->weapon[idx]->reloadtime;
                float conso_energy=((float)unit_manager.unit_type[type_id]->weapon[idx]->energypershot)/unit_manager.unit_type[type_id]->weapon[idx]->reloadtime;
                if (players.metal[owner_id]>=conso_metal*dt && players.energy[owner_id]>=conso_energy*dt)
                {
                    metal_cons+=conso_metal;
                    energy_cons+=conso_energy;
                    planned_weapons-=dn;
                    float last=planned_weapons-(int)planned_weapons;
                    if ((last==0.0f && last!=old) || (last>old && old>0.0f) || planned_weapons<=0.0f)		// On en a fini une / one is finished
                        weapon[idx].stock++;
                    if (planned_weapons<0.0f)
                        planned_weapons=0.0f;
                }
            }
        }

        V_Angle.reset();
        c_time += dt;

        //------------------------------ Beginning of weapon related code ---------------------------------------
        for (int i = 0; i < weapon.size(); ++i)
        {
            if (unit_manager.unit_type[type_id]->weapon[i] == NULL)
                continue;		// Skip that weapon if not present on the unit
            weapon[i].delay += dt;
            weapon[i].time += dt;

            switch ((weapon[i].state & 3))
            {
                case WEAPON_FLAG_IDLE:										// Doing nothing, waiting for orders
                    if (jump_commands)	break;
                    weapon[i].data = -1;
                    break;
                case WEAPON_FLAG_AIM:											// Vise une unité / aiming code
                    if (weapon[i].target == NULL || ((weapon[i].state&WEAPON_FLAG_WEAPON)==WEAPON_FLAG_WEAPON && ((WEAPON*)(weapon[i].target))->weapon_id!=-1)
                        || ((weapon[i].state&WEAPON_FLAG_WEAPON)!=WEAPON_FLAG_WEAPON && (((UNIT*)(weapon[i].target))->flags&1)))
                    {
                        if ((weapon[i].state&WEAPON_FLAG_WEAPON)!=WEAPON_FLAG_WEAPON && weapon[i].target != NULL && ((UNIT*)(weapon[i].target))->cloaked
                            && ((UNIT*)(weapon[i].target))->owner_id != owner_id && !((UNIT*)(weapon[i].target))->is_on_radar( 1 << owner_id))
                        {
                            weapon[i].data = -1;
                            weapon[i].state = WEAPON_FLAG_IDLE;
                            break;
                        }

                        if (!(weapon[i].state & WEAPON_FLAG_COMMAND_FIRE) && unit_manager.unit_type[type_id]->weapon[i]->commandfire) // Not allowed to fire
                        {
                            weapon[i].data = -1;
                            weapon[i].state = WEAPON_FLAG_IDLE;
                            break;
                        }
                        int query_id=-1;
                        switch(i)
                        {
                            case 0:
                                query_id = get_script_index(SCRIPT_QueryPrimary);		break;
                            case 1:
                                query_id = get_script_index(SCRIPT_QuerySecondary);		break;
                            case 2:
                                query_id = get_script_index(SCRIPT_QueryTertiary);		break;
                            default:
                                query_id = get_script_index(format("Query%d",i));
                        }

                        if (!is_running(get_script_index(SCRIPT_RequestState)))
                        {
                            if (weapon[i].delay >= unit_manager.unit_type[type_id]->weapon[ i ]->reloadtime || unit_manager.unit_type[type_id]->weapon[ i ]->stockpile)
                            {
                                run_script_function(map, query_id);			// Run the script that tell us from where to shoot

                                UNIT *target_unit = (weapon[i].state & WEAPON_FLAG_WEAPON ) == WEAPON_FLAG_WEAPON ? NULL : (UNIT*) weapon[i].target;
                                WEAPON *target_weapon = (weapon[i].state & WEAPON_FLAG_WEAPON ) == WEAPON_FLAG_WEAPON ? (WEAPON*) weapon[i].target : NULL;

                                Vector3D target = target_unit==NULL ? (target_weapon==NULL ? weapon[i].target_pos-Pos : target_weapon->Pos-Pos) : target_unit->Pos-Pos;
                                float dist = target.sq();
                                int maxdist = 0;
                                int mindist = 0;

                                if (unit_manager.unit_type[type_id]->attackrunlength>0)
                                {
                                    if (target % V < 0.0f ) {
                                        weapon[i].state = WEAPON_FLAG_IDLE;
                                        weapon[i].data = -1;
                                        break;	// We're not shooting at the target
                                    }
                                    float t = 2.0f/map->ota_data.gravity*fabs(target.y);
                                    mindist = (int)sqrt(t*V.sq())-((unit_manager.unit_type[type_id]->attackrunlength+1)>>1);
                                    maxdist = mindist+(unit_manager.unit_type[type_id]->attackrunlength);
                                }
                                else
                                    maxdist = unit_manager.unit_type[type_id]->weapon[ i ]->range>>1;

                                if (dist > maxdist * maxdist || dist < mindist * mindist )
                                {
                                    weapon[i].state = WEAPON_FLAG_IDLE;
                                    weapon[i].data = -1;
                                    break;	// We're too far from the target
                                }

                                Vector3D target_translation;
                                if (target_unit != NULL )
                                    target_translation = ( target.norm() / unit_manager.unit_type[type_id]->weapon[ i ]->weaponvelocity) * (target_unit->V - V);

                                if (unit_manager.unit_type[type_id]->weapon[ i ]->turret) 	// Si l'unité doit viser, on la fait viser / if it must aim, we make it aim
                                {
                                    if (script_val->size() <= query_id )
                                        script_val->resize( query_id + 1 );
                                    int start_piece = (*script_val)[query_id];
                                    if (start_piece<0 || start_piece>=data.nb_piece)
                                        start_piece=0;
                                    compute_model_coord();

                                    Vector3D target_pos_on_unit;						// Read the target piece on the target unit so we better know where to aim
                                    target_pos_on_unit.x = target_pos_on_unit.y = target_pos_on_unit.z = 0.0f;
                                    if (target_unit != NULL )
                                    {
                                        if (weapon[i].data == -1 )
                                        {
                                            int target_piece[1] = {0};
                                            target_unit->run_script_function( map, target_unit->get_script_index( SCRIPT_SweetSpot ), 1, target_piece );
                                            weapon[i].data = target_piece[0];
                                        }
                                        if (weapon[i].data >= 0 )
                                            target_pos_on_unit = target_unit->data.pos[ weapon[i].data ];
                                    }

                                    target = target + target_translation - data.pos[start_piece];
                                    if (target_unit!=NULL )
                                        target = target + target_pos_on_unit;
                                    dist = target.norm();
                                    target=(1.0f/dist)*target;
                                    Vector3D I,J,IJ,RT;
                                    I.x=0.0f;	I.z=1.0f;	I.y=0.0f;
                                    J.x=1.0f;	J.z=0.0f;	J.y=0.0f;
                                    IJ.x=0.0f;	IJ.z=0.0f;	IJ.y=1.0f;
                                    RT=target;
                                    RT.y=0.0f;
                                    RT.unit();
                                    float angle=acos(I%RT)*RAD2DEG;
                                    if (J%RT<0.0f) angle=-angle;
                                    angle-=Angle.y;
                                    if (angle<-180.0f)	angle+=360.0f;
                                    else if (angle>180.0f)	angle-=360.0f;
                                    int aiming[]={ (int)(angle*DEG2TA), -4096 };
                                    if (unit_manager.unit_type[type_id]->weapon[ i ]->ballistic) // Calculs de ballistique / ballistic calculations
                                    {
                                        Vector3D D=target_unit==NULL ? ( target_weapon == NULL ? Pos + data.pos[start_piece] - weapon[i].target_pos : (Pos+data.pos[start_piece]-target_weapon->Pos) ) : (Pos+data.pos[start_piece]-target_unit->Pos-target_pos_on_unit);
                                        D.y=0.0f;
                                        float v;
                                        if (unit_manager.unit_type[type_id]->weapon[ i ]->startvelocity==0.0f)
                                            v=unit_manager.unit_type[type_id]->weapon[ i ]->weaponvelocity;
                                        else
                                            v=unit_manager.unit_type[type_id]->weapon[ i ]->startvelocity;
                                        if (target_unit==NULL) {
                                            if (target_weapon==NULL )
                                                aiming[1]=(int)(ballistic_angle(v,map->ota_data.gravity,D.norm(),(Pos+data.pos[start_piece]).y,weapon[i].target_pos.y)*DEG2TA);
                                            else
                                                aiming[1]=(int)(ballistic_angle(v,map->ota_data.gravity,D.norm(),(Pos+data.pos[start_piece]).y,target_weapon->Pos.y)*DEG2TA);
                                        }
                                        else
                                            aiming[1]=(int)(ballistic_angle(v,map->ota_data.gravity,D.norm(),(Pos+data.pos[start_piece]).y,target_unit->Pos.y+target_unit->model->center.y*0.5f)*DEG2TA);
                                    }
                                    else
                                    {
                                        Vector3D K=target;
                                        K.y=0.0f;
                                        K.unit();
                                        angle = acos(K%target)*RAD2DEG;
                                        if (target.y<0.0f)
                                            angle=-angle;
                                        angle -= Angle.x;
                                        if (angle>180.0f)	angle-=360.0f;
                                        if (angle<-180.0f)	angle+=360.0f;
                                        if (fabs(angle)>180.0f)
                                        {
                                            weapon[i].state = WEAPON_FLAG_IDLE;
                                            weapon[i].data = -1;
                                            break;
                                        }
                                        aiming[1]=(int)(angle*DEG2TA);
                                    }
                                    if (unit_manager.unit_type[type_id]->weapon[i]->lineofsight)
                                    {
                                        if (!target_unit)
                                        {
                                            if (target_weapon == NULL )
                                                weapon[i].aim_dir=weapon[i].target_pos-(Pos+data.pos[start_piece]);
                                            else
                                                weapon[i].aim_dir=((WEAPON*)(weapon[i].target))->Pos-(Pos+data.pos[start_piece]);
                                        }
                                        else
                                            weapon[i].aim_dir = ((UNIT*)(weapon[i].target))->Pos+target_pos_on_unit-(Pos+data.pos[start_piece]);
                                        weapon[i].aim_dir = weapon[i].aim_dir + target_translation;
                                        weapon[i].aim_dir.unit();
                                    }
                                    else
                                        weapon[i].aim_dir=cos(aiming[1]*TA2RAD)*(cos(aiming[0]*TA2RAD+Angle.y*DEG2RAD)*I+sin(aiming[0]*TA2RAD+Angle.y*DEG2RAD)*J)+sin(aiming[1]*TA2RAD)*IJ;
                                    switch(i)
                                    {
                                        case 0:
                                            launch_script(get_script_index(SCRIPT_AimPrimary),2,aiming);	break;
                                        case 1:
                                            launch_script(get_script_index(SCRIPT_AimSecondary),2,aiming);	break;
                                        case 2:
                                            launch_script(get_script_index(SCRIPT_AimTertiary),2,aiming);	break;
                                        default:
                                            launch_script(get_script_index(format("Aim%d",i)),2,aiming);	break;
                                    }
                                }
                                else
                                    switch(i)
                                    {
                                        case 0:
                                            launch_script(get_script_index(SCRIPT_AimPrimary));	break;
                                        case 1:
                                            launch_script(get_script_index(SCRIPT_AimSecondary));	break;
                                        case 2:
                                            launch_script(get_script_index(SCRIPT_AimTertiary));	break;
                                        default:
                                            launch_script(get_script_index(format("Aim%d",i)));	break;
                                    }
                                weapon[i].time = 0.0f;
                                weapon[i].state = WEAPON_FLAG_SHOOT;									// (puis) on lui demande de tirer / tell it to fire
                                weapon[i].burst=0;
                            }
                        }
                    }
                    else
                    {
                        launch_script(get_script_index(SCRIPT_TargetCleared));
                        weapon[i].state = WEAPON_FLAG_IDLE;
                        weapon[i].data = -1;
                    }
                    break;
                case WEAPON_FLAG_SHOOT:											// Tire sur une unité / fire!
                    if (weapon[i].target == NULL || (( weapon[i].state & WEAPON_FLAG_WEAPON ) == WEAPON_FLAG_WEAPON && ((WEAPON*)(weapon[i].target))->weapon_id!=-1)
                        || (( weapon[i].state & WEAPON_FLAG_WEAPON ) != WEAPON_FLAG_WEAPON && (((UNIT*)(weapon[i].target))->flags&1))) {
                        if (weapon[i].burst > 0 && weapon[i].delay < unit_manager.unit_type[type_id]->weapon[ i ]->burstrate )
                            break;
                        int query_id=-1;
                        int Aim_script = -1;
                        int Fire_script = -1;
                        switch(i)
                        {
                            case 0:
                                query_id = get_script_index(SCRIPT_QueryPrimary);
                                Aim_script = get_script_index(SCRIPT_AimPrimary);
                                Fire_script = get_script_index(SCRIPT_FirePrimary);
                                break;
                            case 1:
                                query_id = get_script_index(SCRIPT_QuerySecondary);
                                Aim_script = get_script_index(SCRIPT_AimSecondary);
                                Fire_script = get_script_index(SCRIPT_FireSecondary);
                                break;
                            case 2:
                                query_id = get_script_index(SCRIPT_QueryTertiary);
                                Aim_script = get_script_index(SCRIPT_AimTertiary);
                                Fire_script = get_script_index(SCRIPT_FireTertiary);
                                break;
                            default:
                                query_id = get_script_index(format("Query%d",i));
                                Aim_script = get_script_index(format("Aim%d",i));
                                Fire_script = get_script_index(format("Fire%d",i));
                        }
                        if (!is_running(Aim_script))
                        {
                            if ((players.metal[owner_id]<unit_manager.unit_type[type_id]->weapon[ i ]->metalpershot
                                 || players.energy[owner_id]<unit_manager.unit_type[type_id]->weapon[ i ]->energypershot)
                                && !unit_manager.unit_type[type_id]->weapon[ i ]->stockpile) {
                                weapon[i].state = WEAPON_FLAG_AIM;		// Pas assez d'énergie pour tirer / not enough energy to fire
                                weapon[i].data = -1;
                                break;
                            }
                            if (unit_manager.unit_type[type_id]->weapon[ i ]->stockpile && weapon[i].stock<=0)
                            {
                                weapon[i].state = WEAPON_FLAG_AIM;		// Plus rien pour tirer / nothing to fire
                                weapon[i].data = -1;
                                break;
                            }
                            else if (unit_manager.unit_type[type_id]->weapon[ i ]->stockpile )
                                weapon[i].stock--;
                            else
                            {													// We use energy and metal only for weapons with no prebuilt ammo
                                players.c_metal[owner_id] -= unit_manager.unit_type[type_id]->weapon[ i ]->metalpershot;
                                players.c_energy[owner_id] -= unit_manager.unit_type[type_id]->weapon[ i ]->energypershot;
                            }
                            run_script_function( map, Fire_script );			// Run the script that tell us from where to shoot
                            if (!unit_manager.unit_type[type_id]->weapon[ i ]->soundstart.empty())	sound_manager->playSound(unit_manager.unit_type[type_id]->weapon[i]->soundstart, &Pos);
                            if (script_val->size() <= query_id )
                                script_val->resize( query_id + 1 );
                            int start_piece = (*script_val)[query_id];
                            if (start_piece>=0 && start_piece<data.nb_piece)
                            {
                                compute_model_coord();
                                Vector3D Dir = data.dir[start_piece];
                                if (Dir.x==0.0f && Dir.y==0.0f && Dir.z==0.0f)
                                {
                                    if (unit_manager.unit_type[type_id]->weapon[ i ]->vlaunch)
                                    {
                                        Dir.x=0.0f;
                                        Dir.y=1.0f;
                                        Dir.z=0.0f;
                                    }
                                    else
                                        Dir = weapon[i].aim_dir;
                                }
                                if (weapon[i].target == NULL )
                                    shoot(-1,Pos+data.pos[start_piece],Dir,i, weapon[i].target_pos );
                                else {
                                    if (weapon[i].state & WEAPON_FLAG_WEAPON )
                                        shoot(((WEAPON*)(weapon[i].target))->idx,Pos+data.pos[start_piece],Dir,i, weapon[i].target_pos);
                                    else
                                        shoot(((UNIT*)(weapon[i].target))->idx,Pos+data.pos[start_piece],Dir,i, weapon[i].target_pos);
                                }
                                weapon[i].burst++;
                                if (weapon[i].burst>=unit_manager.unit_type[type_id]->weapon[i]->burst)
                                    weapon[i].burst=0;
                                weapon[i].delay=0.0f;
                            }
                            if (weapon[i].burst == 0 && unit_manager.unit_type[type_id]->weapon[ i ]->commandfire && !unit_manager.unit_type[type_id]->weapon[ i ]->dropped ) {		// Shoot only once
                                weapon[i].state = WEAPON_FLAG_IDLE;
                                weapon[i].data = -1;
                                if (mission != NULL )
                                    mission->flags |= MISSION_FLAG_COMMAND_FIRED;
                                break;
                            }
                            if (weapon[i].target != NULL && (weapon[i].state & WEAPON_FLAG_WEAPON)!=WEAPON_FLAG_WEAPON && ((UNIT*)(weapon[i].target))->hp>0) {				// La cible est-elle détruite ?? / is target destroyed ??
                                if (weapon[i].burst==0) {
                                    weapon[i].state = WEAPON_FLAG_AIM;
                                    weapon[i].data = -1;
                                    weapon[i].time = 0.0f;
                                }
                            }
                            else if (weapon[i].target != NULL || weapon[i].burst == 0 ) {
                                launch_script(get_script_index(SCRIPT_TargetCleared));
                                weapon[i].state = WEAPON_FLAG_IDLE;
                                weapon[i].data = -1;
                            }
                        }
                    }
                    else
                    {
                        launch_script(get_script_index(SCRIPT_TargetCleared) );
                        weapon[i].state = WEAPON_FLAG_IDLE;
                        weapon[i].data = -1;
                    }
                    break;
            }
        }

        //---------------------------- Beginning of mission execution code --------------------------------------

        if (mission == NULL)
            was_moving = false;

        if (mission)
        {
            mission->time+=dt;
            last_path_refresh += dt;

            //----------------------------------- Beginning of moving code ------------------------------------

            if ((mission->flags & MISSION_FLAG_MOVE) && unit_manager.unit_type[type_id]->canmove && unit_manager.unit_type[type_id]->BMcode)
            {
                if (!was_moving)
                {
                    if (unit_manager.unit_type[type_id]->canfly)
                        activate();
                    launch_script(get_script_index(SCRIPT_MotionControl));
                    launch_script(get_script_index(SCRIPT_startmoving));
                    if (nb_attached==0)
                        launch_script(get_script_index(SCRIPT_MoveRate1));		// For the armatlas
                    else
                        launch_script(get_script_index(SCRIPT_MoveRate2));
                    was_moving = true;
                }
                Vector3D J,I,K;
                K.x = 0.0f;
                K.y = 1.0f;
                K.z = 0.0f;
                Vector3D Target(mission->target);
                if (mission->path && ( !(mission->flags & MISSION_FLAG_REFRESH_PATH) || last_path_refresh < 5.0f ) )
                    Target = mission->path->Pos;
                else
                {// Look for a path to the target
                    if (mission->path)// If we want to refresh the path
                    {
                        Target = mission->target;//mission->path->Pos;
                        destroy_path( mission->path );
                        mission->path = NULL;
                    }
                    mission->flags &= ~MISSION_FLAG_REFRESH_PATH;
                    float dist = (Target-Pos).sq();
                    if (( mission->move_data <= 0 && dist > 100.0f ) || ( ( mission->move_data * mission->move_data << 6 ) < dist))
                    {
                        if (!requesting_pathfinder && last_path_refresh >= 5.0f)
                        {
                            requesting_pathfinder = true;
                            units.requests[ owner_id ].push_back( idx );
                        }
                        if (path_exec[ owner_id ] < MAX_PATH_EXEC && last_path_refresh >= 5.0f && !units.requests[ owner_id ].empty() && idx == units.requests[ owner_id ].front())
                        {
                            units.requests[ owner_id ].pop_front();
                            requesting_pathfinder = false;

                            path_exec[ owner_id ]++;
                            move_target_computed = mission->target;
                            last_path_refresh = 0.0f;
                            if (unit_manager.unit_type[type_id]->canfly)
                            {
                                if (mission->move_data<=0)
                                    mission->path = direct_path(mission->target);
                                else
                                {
                                    Vector3D Dir = mission->target-Pos;
                                    Dir.unit();
                                    mission->path = direct_path(mission->target-(mission->move_data<<3)*Dir);
                                }
                            }
                            else
                            {
                                float dh_max = unit_manager.unit_type[type_id]->MaxSlope * H_DIV;
                                float h_min = unit_manager.unit_type[type_id]->canhover ? -100.0f : map->sealvl - unit_manager.unit_type[type_id]->MaxWaterDepth * H_DIV;
                                float h_max = map->sealvl - unit_manager.unit_type[type_id]->MinWaterDepth * H_DIV;
                                float hover_h = unit_manager.unit_type[type_id]->canhover ? map->sealvl : -100.0f;
                                if (mission->move_data <= 0)
                                    mission->path = find_path(map->map_data,map->h_map,map->path,map->map_w,map->map_h,map->bloc_w<<1,map->bloc_h<<1,
                                                              dh_max, h_min, h_max, Pos, mission->target, unit_manager.unit_type[type_id]->FootprintX, unit_manager.unit_type[type_id]->FootprintZ, idx, 0, hover_h );
                                else
                                    mission->path = find_path(map->map_data,map->h_map,map->path,map->map_w,map->map_h,map->bloc_w<<1,map->bloc_h<<1,
                                                              dh_max, h_min, h_max, Pos, mission->target, unit_manager.unit_type[type_id]->FootprintX, unit_manager.unit_type[type_id]->FootprintZ, idx, mission->move_data, hover_h );

                                if (mission->path == NULL)
                                {
                                    bool place_is_empty = map->check_rect( cur_px-(unit_manager.unit_type[type_id]->FootprintX>>1), cur_py-(unit_manager.unit_type[type_id]->FootprintZ>>1), unit_manager.unit_type[type_id]->FootprintX, unit_manager.unit_type[type_id]->FootprintZ, idx);
                                    if (!place_is_empty)
                                    {
                                        LOG_WARNING("An Unit is blocked !" << __FILE__ << ":" << __LINE__);
                                        mission->flags &= ~MISSION_FLAG_MOVE;
                                    }
                                    else
                                        mission->flags |= MISSION_FLAG_REFRESH_PATH;			// Retry later
                                    launch_script(get_script_index(SCRIPT_StopMoving));
                                    was_moving = false;
                                }

                                if (mission->path == NULL)					// Can't find a path to get where it has been ordered to go
                                    play_sound("cant1");
                            }
                            if (mission->path)// Update required data
                                Target = mission->path->Pos;
                        }
                    }
                    else
                        stop_moving();
                }
                
                if (mission->path) // If we have a path, follow it
                {
                    if ((mission->target - move_target_computed).sq() >= 10000.0f )			// Follow the target above all...
                        mission->flags |= MISSION_FLAG_REFRESH_PATH;
                    J = Target - Pos;
                    J.y = 0.0f;
                    float dist = J.norm();
                    if ((dist > mission->last_d && dist < 15.0f) || mission->path == NULL)
                    {
                        if (mission->path)
                        {
                            float dh_max = unit_manager.unit_type[type_id]->MaxSlope * H_DIV;
                            float h_min = unit_manager.unit_type[type_id]->canhover ? -100.0f : map->sealvl - unit_manager.unit_type[type_id]->MaxWaterDepth * H_DIV;
                            float h_max = map->sealvl - unit_manager.unit_type[type_id]->MinWaterDepth * H_DIV;
                            float hover_h = unit_manager.unit_type[type_id]->canhover ? map->sealvl : -100.0f;
                            mission->path = next_node(mission->path, map->map_data, map->h_map, map->bloc_w_db, map->bloc_h_db, dh_max, h_min, h_max, unit_manager.unit_type[type_id]->FootprintX, unit_manager.unit_type[type_id]->FootprintZ, idx, hover_h );
                        }
                        mission->last_d = 9999999.0f;
                        if (mission->path == NULL) // End of path reached
                        {
                            J = move_target_computed - Pos;
                            J.y = 0.0f;
                            if (J.sq() <= 256.0f || flying)
                            {
                                if (!(mission->flags & MISSION_FLAG_DONT_STOP_MOVE) && (mission == NULL || mission->mission != MISSION_PATROL ) )
                                    play_sound( "arrived1" );
                                mission->flags &= ~MISSION_FLAG_MOVE;
                            }
                            else										// We are not where we are supposed to be !!
                                mission->flags |= MISSION_FLAG_REFRESH_PATH;
                            if (!( unit_manager.unit_type[type_id]->canfly && nb_attached > 0 ) ) {		// Once charged with units the Atlas cannot land
                                launch_script(get_script_index(SCRIPT_StopMoving));
                                was_moving = false;
                            }
                            if (!(mission->flags & MISSION_FLAG_DONT_STOP_MOVE) )
                                V.x = V.y = V.z = 0.0f;		// Stop unit's movement
                        }
                    }
                    else
                        mission->last_d = dist;
                    if (mission->flags & MISSION_FLAG_MOVE)	// Are we still moving ??
                    {
                        if (dist > 0.0f)
                            J = 1.0f / dist * J;

                        b_TargetAngle = true;
                        f_TargetAngle = acos( J.z ) * RAD2DEG;
                        if (J.x < 0.0f ) f_TargetAngle = -f_TargetAngle;

                        if (Angle.y - f_TargetAngle >= 360.0f )	f_TargetAngle += 360.0f;
                        else if (Angle.y - f_TargetAngle <= -360.0f )	f_TargetAngle -= 360.0f;

                        J.z = cos(Angle.y*DEG2RAD);
                        J.x = sin(Angle.y*DEG2RAD);
                        J.y = 0.0f;
                        I.z = -J.x;
                        I.x = J.z;
                        I.y = 0.0f;
                        V = (V%K)*K + (V%J)*J;
                        if (!(dist < 15.0f && fabs( Angle.y - f_TargetAngle ) >= 1.0f))
                        {
                            if (fabs( Angle.y - f_TargetAngle ) >= 45.0f )
                            {
                                if (J % V > 0.0f && V.norm() > unit_manager.unit_type[type_id]->BrakeRate * dt )
                                    V = V - ((( fabs( Angle.y - f_TargetAngle ) - 35.0f ) / 135.0f + 1.0f) * 0.5f * unit_manager.unit_type[type_id]->BrakeRate * dt) * J;
                            }
                            else
                            {
                                float speed = V.norm();
                                float time_to_stop = speed / unit_manager.unit_type[type_id]->BrakeRate;
                                float min_dist = time_to_stop * (speed-unit_manager.unit_type[type_id]->BrakeRate*0.5f*time_to_stop);
                                if (min_dist>=dist && !(mission->flags & MISSION_FLAG_DONT_STOP_MOVE)
                                    && ( mission->next == NULL || ( mission->next->mission != MISSION_MOVE && mission->next->mission != MISSION_PATROL ) ) )	// Brake if needed
                                    V = V-unit_manager.unit_type[type_id]->BrakeRate*dt*J;
                                else if (speed < unit_manager.unit_type[type_id]->MaxVelocity )
                                    V = V + unit_manager.unit_type[type_id]->Acceleration*dt*J;
                                else
                                    V = unit_manager.unit_type[type_id]->MaxVelocity / speed * V;
                            }
                        }
                        else
                        {
                            float speed = V.norm();
                            if (speed > unit_manager.unit_type[type_id]->MaxVelocity )
                                V = unit_manager.unit_type[type_id]->MaxVelocity / speed * V;
                        }
                    }
                }

                NPos = Pos + dt*V;			// Check if the unit can go where V brings it
                if (was_locked ) // Random move to solve the unit lock problem
                {
                    if (V.x > 0.0f)
                        NPos.x += (Math::RandFromTable() % 101) * 0.01f;
                    else
                        NPos.x -= (Math::RandFromTable() % 101) * 0.01f;
                    if (V.z > 0.0f)
                        NPos.z += (Math::RandFromTable() % 101) * 0.01f;
                    else
                        NPos.z -= (Math::RandFromTable() % 101) * 0.01f;

                    if (was_locked >= 5.0f)
                    {
                        was_locked = 5.0f;
                        mission->flags |= MISSION_FLAG_REFRESH_PATH;			// Refresh path because this shouldn't happen unless
                        // obstacles have moved
                    }
                }
                n_px = ((int)(NPos.x)+map->map_w_d+4)>>3;
                n_py = ((int)(NPos.z)+map->map_h_d+4)>>3;
                precomputed_position = true;
                bool locked = false;
                if (!flying )
                {
                    if (n_px != cur_px || n_py != cur_py) // has something changed ??
                    {
                        bool place_is_empty = can_be_there( n_px, n_py, map, type_id, owner_id, idx );
                        if (!(flags & 64) && !place_is_empty)
                        {
                            if (!unit_manager.unit_type[type_id]->canfly)
                            {
                                locked = true;
                                // Check some basic solutions first
                                if (cur_px != n_px
                                    && can_be_there( cur_px, n_py, map, type_id, owner_id, idx ))
                                {
                                    V.z = V.z != 0.0f ? (V.z < 0.0f ? -sqrt( SQUARE(V.z) + SQUARE(V.x) ) : sqrt( SQUARE(V.z) + SQUARE(V.x) ) ) : 0.0f;
                                    V.x = 0.0f;
                                    NPos.x = Pos.x;
                                    n_px = cur_px;
                                }
                                else if (cur_py != n_py && can_be_there( n_px, cur_py, map, type_id, owner_id, idx ))
                                {
                                    V.x = (V.x != 0.0)
                                        ? ((V.x < 0.0f)
                                           ? -sqrt(SQUARE(V.z) + SQUARE(V.x))
                                           : sqrt(SQUARE(V.z) + SQUARE(V.x)))
                                        : 0.0f;
                                    V.z = 0.0f;
                                    NPos.z = Pos.z;
                                    n_py = cur_py;
                                }
                                else if (can_be_there( cur_px, cur_py, map, type_id, owner_id, idx )) {
                                    V.x = V.y = V.z = 0.0f;		// Don't move since we can't
                                    NPos = Pos;
                                    n_px = cur_px;
                                    n_py = cur_py;
                                    mission->flags |= MISSION_FLAG_MOVE;
                                    if (fabs( Angle.y - f_TargetAngle ) <= 0.1f || !b_TargetAngle) // Don't prevent unit from rotating!!
                                    {
                                        if (mission->path)
                                            destroy_path(mission->path);
                                        mission->path = NULL;
                                    }
                                }
                                else
                                    LOG_WARNING("An Unit is blocked !" << __FILE__ << ":" << __LINE__);
                            }
                            else if (!flying && local )
                            {
                                if (Pos.x<-map->map_w_d || Pos.x>map->map_w_d || Pos.z<-map->map_h_d || Pos.z>map->map_h_d)
                                {
                                    Vector3D target = Pos;
                                    if (target.x < -map->map_w_d+256)
                                        target.x = -map->map_w_d+256;
                                    else if (target.x > map->map_w_d-256)
                                        target.x = map->map_w_d-256;
                                    if (target.z < -map->map_h_d+256)
                                        target.z = -map->map_h_d+256;
                                    else if (target.z > map->map_h_d-256)
                                        target.z = map->map_h_d-256;
                                    next_mission();
                                    add_mission(MISSION_MOVE | MISSION_FLAG_AUTO,&target,true,0,NULL,NULL,0,1);		// Stay on map
                                }
                                else
                                    if (!can_be_there( cur_px, cur_py, map, type_id, owner_id, idx ))
                                    {
                                        NPos = Pos;
                                        n_px = cur_px;
                                        n_py = cur_py;
                                        Vector3D target = Pos;
                                        target.x += ((sint32)(Math::RandFromTable()&0x1F))-16;		// Look for a place to land
                                        target.z += ((sint32)(Math::RandFromTable()&0x1F))-16;
                                        mission->flags |= MISSION_FLAG_MOVE;
                                        if (mission->path )
                                            destroy_path(mission->path);
                                        mission->path = direct_path( target );
                                    }
                            }
                        }
                        else if (!(flags & 64) && unit_manager.unit_type[type_id]->canfly && ( mission == NULL ||
                                                                                               ( mission->mission != MISSION_MOVE && mission->mission != MISSION_ATTACK ) ) )
                            flags |= 64;
                    }
                    else
                    {
                        bool place_is_empty = map->check_rect(n_px-(unit_manager.unit_type[type_id]->FootprintX>>1),n_py-(unit_manager.unit_type[type_id]->FootprintZ>>1),unit_manager.unit_type[type_id]->FootprintX,unit_manager.unit_type[type_id]->FootprintZ,idx);
                        if (!place_is_empty)
                        {
                            pMutex.unlock();
                            clear_from_map();
                            pMutex.lock();
                            LOG_WARNING("An Unit is blocked ! (probably spawned on something)" << __FILE__ << ":" << __LINE__);
                        }
                    }
                }
                if (locked)
                    was_locked += dt;
                else
                    was_locked = 0.0f;
            }
            else
            {
                was_moving = false;
                requesting_pathfinder = false;
            }

            if (flying && local) // Force planes to stay on map
            {
                if (Pos.x<-map->map_w_d || Pos.x>map->map_w_d || Pos.z<-map->map_h_d || Pos.z>map->map_h_d) {
                    if (Pos.x < -map->map_w_d )
                        V.x += dt * ( -map->map_w_d - Pos.x ) * 0.1f;
                    else if (Pos.x > map->map_w_d )
                        V.x -= dt * ( Pos.x - map->map_w_d ) * 0.1f;
                    if (Pos.z < -map->map_h_d )
                        V.z += dt * ( -map->map_h_d - Pos.z ) * 0.1f;
                    else if (Pos.z > map->map_h_d )
                        V.z -= dt * ( Pos.z - map->map_h_d ) * 0.1f;
                    float speed = V.norm();
                    if (speed > unit_manager.unit_type[type_id]->MaxVelocity && speed > 0.0f ) {
                        V = unit_manager.unit_type[type_id]->MaxVelocity / speed * V;
                        speed = unit_manager.unit_type[type_id]->MaxVelocity;
                    }
                    if (speed > 0.0f)
                    {
                        Angle.y = acos( V.z / speed ) * RAD2DEG;
                        if (V.x < 0.0f)
                            Angle.y = -Angle.y;
                    }
                }
            }

            //----------------------------------- End of moving code ------------------------------------

            switch(mission->mission)						// Commandes générales / General orders
            {
                case MISSION_WAIT:					// Wait for a specified time (campaign)
                    mission->flags = 0;			// Don't move, do not shoot !! just wait
                    if (mission->time >= mission->data * 0.001f )	// Done :)
                        next_mission();
                    break;
                case MISSION_WAIT_ATTACKED:			// Wait until a specified unit is attacked (campaign)
                    if (mission->data < 0 || mission->data >= units.max_unit || !(units.unit[ mission->data ].flags & 1) )
                        next_mission();
                    else if (units.unit[ mission->data ].attacked )
                        next_mission();
                    break;
                case MISSION_GET_REPAIRED:
                    if (mission->p && (((UNIT*)mission->p)->flags & 1) && ((UNIT*)mission->p)->ID == mission->target_ID ) {
                        UNIT *target_unit = (UNIT*) mission->p;

                        if (!(mission->flags & MISSION_FLAG_PAD_CHECKED) ) {
                            mission->flags |= MISSION_FLAG_PAD_CHECKED;
                            int param[] = { 0, 1 };
                            target_unit->run_script_function( map, target_unit->get_script_index( SCRIPT_QueryLandingPad ), 2, param );
                            mission->data = param[ 0 ];
                        }

                        target_unit->compute_model_coord();
                        int piece_id = mission->data >= 0 ? mission->data : (-mission->data - 1);
                        mission->target = target_unit->Pos + target_unit->data.pos[ piece_id ];

                        Vector3D Dir = mission->target - Pos;
                        Dir.y = 0.0f;
                        float dist = Dir.sq();
                        int maxdist = 6;
                        if (dist > maxdist * maxdist && unit_manager.unit_type[type_id]->BMcode ) {	// Si l'unité est trop loin du chantier
                            mission->flags &= ~MISSION_FLAG_BEING_REPAIRED;
                            c_time = 0.0f;
                            mission->flags |= MISSION_FLAG_MOVE;
                            mission->move_data = maxdist*8/80;
                            if (mission->path )
                                mission->path->Pos = mission->target;			// Update path in real time!
                        }
                        else if (!(mission->flags & MISSION_FLAG_MOVE) ) {
                            b_TargetAngle = true;
                            f_TargetAngle = target_unit->Angle.y;
                            if (mission->data >= 0 ) {
                                mission->flags |= MISSION_FLAG_BEING_REPAIRED;
                                Dir = mission->target - Pos;
                                Pos = Pos + 3.0f * dt * Dir;
                                Pos.x = mission->target.x;
                                Pos.z = mission->target.z;
                                if (Dir.sq() < 3.0f ) {
                                    target_unit->lock();
                                    if (target_unit->pad1 != 0xFFFF && target_unit->pad2 != 0xFFFF ) {		// We can't land here
                                        target_unit->unlock();
                                        next_mission();
                                        if (mission && mission->mission == MISSION_STOP )		// Don't stop we were patroling
                                            next_mission();
                                        break;
                                    }
                                    if (target_unit->pad1 == 0xFFFF )			// tell others we're here
                                        target_unit->pad1 = piece_id;
                                    else target_unit->pad2 = piece_id;
                                    target_unit->unlock();
                                    mission->data = -mission->data - 1;
                                }
                            }
                            else {						// being repaired
                                Pos = mission->target;
                                V.x = V.y = V.z = 0.0f;

                                if (target_unit->port[ ACTIVATION ])
                                {
                                    float conso_energy=((float)(unit_manager.unit_type[target_unit->type_id]->WorkerTime * unit_manager.unit_type[type_id]->BuildCostEnergy)) / unit_manager.unit_type[type_id]->BuildTime;
                                    if (players.energy[owner_id] >= conso_energy * dt)
                                    {
                                        energy_cons += conso_energy;
                                        hp += dt * unit_manager.unit_type[target_unit->type_id]->WorkerTime * unit_manager.unit_type[type_id]->MaxDamage / unit_manager.unit_type[type_id]->BuildTime;
                                    }
                                    if (hp >= unit_manager.unit_type[type_id]->MaxDamage) // Unit has been repaired
                                    {
                                        hp = unit_manager.unit_type[type_id]->MaxDamage;
                                        target_unit->lock();
                                        if (target_unit->pad1 == piece_id )			// tell others we've left
                                            target_unit->pad1 = 0xFFFF;
                                        else target_unit->pad2 = 0xFFFF;
                                        target_unit->unlock();
                                        next_mission();
                                        if (mission && mission->mission == MISSION_STOP )		// Don't stop we were patroling
                                            next_mission();
                                        break;
                                    }
                                    built=true;
                                }
                            }
                        }
                    }
                    else
                        next_mission();
                    break;
                case MISSION_STANDBY_MINE:		// Don't even try to do something else, the unit must die !!
                    if (self_destruct < 0.0f ) {
                        int dx = ((unit_manager.unit_type[type_id]->SightDistance+(int)(h+0.5f))>>3) + 1;
                        int enemy_idx=-1;
                        int sx=Math::RandFromTable()&1;
                        int sy=Math::RandFromTable()&1;
                        // byte mask=1<<owner_id;
                        for(int y=cur_py-dx+sy;y<=cur_py+dx;y+=2) {
                            if (y>=0 && y<map->bloc_h_db-1)
                                for(int x=cur_px-dx+sx;x<=cur_px+dx;x+=2)
                                    if (x>=0 && x<map->bloc_w_db-1 ) {
                                        int cur_idx = map->map_data[y][x].unit_idx;
                                        if (cur_idx>=0 && cur_idx<units.max_unit && (units.unit[cur_idx].flags & 1) && units.unit[cur_idx].owner_id != owner_id
                                            && unit_manager.unit_type[units.unit[cur_idx].type_id]->ShootMe ) {		// This unit is on the sight_map since dx = sightdistance !!
                                            enemy_idx = cur_idx;
                                            break;
                                        }
                                    }
                            if (enemy_idx>=0)	break;
                            sx ^= 1;
                        }
                        if (enemy_idx >= 0 )					// Annihilate it !!!
                            toggle_self_destruct();
                    }
                    break;
                case MISSION_UNLOAD:
                    if (nb_attached > 0 )
                    {
                        Vector3D Dir = mission->target-Pos;
                        Dir.y=0.0f;
                        float dist = Dir.sq();
                        int maxdist=0;
                        if (unit_manager.unit_type[type_id]->TransMaxUnits==1)		// Code for units like the arm atlas
                            maxdist=3;
                        else
                            maxdist=unit_manager.unit_type[type_id]->SightDistance;
                        if (dist>maxdist*maxdist && unit_manager.unit_type[type_id]->BMcode ) {	// Si l'unité est trop loin du chantier
                            c_time = 0.0f;
                            mission->flags |= MISSION_FLAG_MOVE;
                            mission->move_data = maxdist*8/80;
                        }
                        else if (!(mission->flags & MISSION_FLAG_MOVE) )
                        {
                            if (mission->last_d>=0.0f)
                            {
                                if (unit_manager.unit_type[type_id]->TransMaxUnits==1)// Code for units like the arm atlas
                                {
                                    if (attached_list[0] >= 0 && attached_list[0] < units.max_unit				// Check we can do that
                                        && units.unit[ attached_list[0] ].flags && can_be_built( Pos, map, units.unit[ attached_list[0] ].type_id, owner_id ) ) {
                                        launch_script(get_script_index(SCRIPT_EndTransport));

                                        UNIT *target_unit = &(units.unit[ attached_list[0] ]);
                                        target_unit->attached = false;
                                        target_unit->hidden = false;
                                        nb_attached = 0;
                                        pMutex.unlock();
                                        target_unit->draw_on_map();
                                        pMutex.lock();
                                    }
                                    else if (attached_list[0] < 0 || attached_list[0] >= units.max_unit
                                             || units.unit[ attached_list[0] ].flags == 0 )
                                        nb_attached = 0;

                                    next_mission();
                                }
                                else {
                                    if (attached_list[ nb_attached - 1 ] >= 0 && attached_list[ nb_attached - 1 ] < units.max_unit				// Check we can do that
                                        && units.unit[ attached_list[ nb_attached - 1 ] ].flags && can_be_built( mission->target, map, units.unit[ attached_list[ nb_attached - 1 ] ].type_id, owner_id ) ) {
                                        int idx = attached_list[ nb_attached - 1 ];
                                        int param[]= { idx, PACKXZ( mission->target.x*2.0f+map->map_w, mission->target.z*2.0f+map->map_h ) };
                                        launch_script(get_script_index(SCRIPT_TransportDrop),2,param);
                                    }
                                    else if (attached_list[ nb_attached - 1 ] < 0 || attached_list[ nb_attached - 1 ] >= units.max_unit
                                             || units.unit[ attached_list[ nb_attached - 1 ] ].flags == 0 )
                                        nb_attached--;
                                }
                                mission->last_d=-1.0f;
                            }
                            else {
                                if (!is_running(get_script_index(SCRIPT_TransportDrop)) && port[ BUSY ] == 0.0f )
                                    next_mission();
                            }
                        }
                    }
                    else
                        next_mission();
                    break;
                case MISSION_LOAD:
                    if (mission->p!=NULL) {
                        UNIT *target_unit=(UNIT*) mission->p;
                        if (!(target_unit->flags & 1) || target_unit->ID != mission->target_ID ) {
                            next_mission();
                            break;
                        }
                        Vector3D Dir=target_unit->Pos-Pos;
                        Dir.y=0.0f;
                        mission->target=target_unit->Pos;
                        float dist = Dir.sq();
                        int maxdist=0;
                        if (unit_manager.unit_type[type_id]->TransMaxUnits==1)		// Code for units like the arm atlas
                            maxdist=3;
                        else
                            maxdist=unit_manager.unit_type[type_id]->SightDistance;
                        if (dist>maxdist*maxdist && unit_manager.unit_type[type_id]->BMcode) {	// Si l'unité est trop loin du chantier
                            c_time = 0.0f;
                            mission->flags |= MISSION_FLAG_MOVE;
                            mission->move_data = maxdist*8/80;
                        }
                        else if (!(mission->flags & MISSION_FLAG_MOVE) ) {
                            if (mission->last_d>=0.0f) {
                                if (unit_manager.unit_type[type_id]->TransMaxUnits==1) {		// Code for units like the arm atlas
                                    if (nb_attached==0) {
                                        //										int param[] = { (int)((Pos.y - target_unit->Pos.y - target_unit->model->top)*2.0f) << 16 };
                                        int param[] = { (int)((Pos.y - target_unit->Pos.y)*2.0f) << 16 };
                                        launch_script(get_script_index(SCRIPT_BeginTransport),1,param);
                                        run_script_function( map, get_script_index(SCRIPT_QueryTransport),1,param);
                                        target_unit->attached = true;
                                        link_list[nb_attached] = param[0];
                                        target_unit->hidden = param[0] < 0;
                                        attached_list[nb_attached++] = target_unit->idx;
                                        target_unit->clear_from_map();
                                    }
                                    next_mission();
                                }
                                else {
                                    if (nb_attached>=unit_manager.unit_type[type_id]->TransportCapacity) {
                                        next_mission();
                                        break;
                                    }
                                    int param[]= { target_unit->idx };
                                    launch_script(get_script_index(SCRIPT_TransportPickup),1,param);
                                }
                                mission->last_d=-1.0f;
                            }
                            else {
                                if (!is_running(get_script_index(SCRIPT_TransportPickup)) && port[ BUSY ] == 0.0f )
                                    next_mission();
                            }
                        }
                    }
                    else
                        next_mission();
                    break;
                case MISSION_CAPTURE:
                case MISSION_REVIVE:
                case MISSION_RECLAIM:
                    if (mission->p!=NULL)	{		// Récupère une unité
                        UNIT *target_unit=(UNIT*) mission->p;
                        if ((target_unit->flags & 1) && target_unit->ID == mission->target_ID ) {
                            if (mission->mission == MISSION_CAPTURE ) {
                                if (unit_manager.unit_type[target_unit->type_id]->commander || target_unit->owner_id == owner_id ) {
                                    play_sound( "cant1" );
                                    next_mission();
                                    break;
                                }
                                if (!(mission->flags & MISSION_FLAG_TARGET_CHECKED) ) {
                                    mission->flags |= MISSION_FLAG_TARGET_CHECKED;
                                    mission->data = Math::Min(unit_manager.unit_type[target_unit->type_id]->BuildCostMetal * 100, 10000);
                                }
                            }
                            Vector3D Dir=target_unit->Pos-Pos;
                            Dir.y=0.0f;
                            mission->target=target_unit->Pos;
                            float dist=Dir.sq();
                            int maxdist = mission->mission == MISSION_CAPTURE ? (int)(unit_manager.unit_type[type_id]->SightDistance) : (int)(unit_manager.unit_type[type_id]->BuildDistance);
                            if (dist>maxdist*maxdist && unit_manager.unit_type[type_id]->BMcode) {	// Si l'unité est trop loin du chantier
                                c_time=0.0f;
                                mission->flags |= MISSION_FLAG_MOVE;// | MISSION_FLAG_REFRESH_PATH;
                                mission->move_data = maxdist*7/80;
                                mission->last_d = 0.0f;
                            }
                            else if (!(mission->flags & MISSION_FLAG_MOVE) ) {
                                if (mission->last_d>=0.0f) {
                                    int angle = (int)( acos( Dir.z / Dir.norm() ) * RAD2DEG );
                                    if (Dir.x < 0.0f )
                                        angle = -angle;
                                    angle -= (int)Angle.y;
                                    if (angle>180)	angle-=360;
                                    if (angle<-180)	angle+=360;
                                    int param[] = { (int)(angle*DEG2TA) };
                                    launch_script(get_script_index(SCRIPT_startbuilding), 1, param);
                                    launch_script(get_script_index(SCRIPT_go));
                                    mission->last_d=-1.0f;
                                }

                                if (unit_manager.unit_type[type_id]->BMcode && port[ INBUILDSTANCE ] != 0.0f ) {
                                    if (local && network_manager.isConnected() && nanolathe_target < 0 ) {		// Synchronize nanolathe emission
                                        nanolathe_target = target_unit->idx;
                                        g_ta3d_network->sendUnitNanolatheEvent( idx, target_unit->idx, false, mission->mission == MISSION_RECLAIM );
                                    }

                                    play_sound( "working" );
                                    // Récupère l'unité
                                    float recup = dt * 4.5f * unit_manager.unit_type[target_unit->type_id]->MaxDamage / unit_manager.unit_type[type_id]->WorkerTime;
                                    if (mission->mission == MISSION_CAPTURE ) {
                                        mission->data -= (int)(dt * 1000.0f + 0.5f);
                                        if (mission->data <= 0 ) {			// Unit has been captured
                                            pMutex.unlock();

                                            target_unit->clear_from_map();
                                            target_unit->lock();

                                            UNIT *new_unit = (UNIT*) create_unit( target_unit->type_id, owner_id, target_unit->Pos, map);
                                            if (new_unit ) {
                                                new_unit->lock();

                                                new_unit->Angle = target_unit->Angle;
                                                new_unit->hp = target_unit->hp;
                                                new_unit->build_percent_left = target_unit->build_percent_left;

                                                new_unit->unlock();
                                            }

                                            target_unit->flags = 0x14;
                                            target_unit->hp = 0.0f;
                                            target_unit->local = true;		// Force synchronization in networking mode

                                            target_unit->unlock();

                                            pMutex.lock();
                                            launch_script(get_script_index(SCRIPT_stopbuilding));
                                            launch_script(get_script_index(SCRIPT_stop));
                                            next_mission();
                                        }
                                    }
                                    else {
                                        if (recup > target_unit->hp )	recup = target_unit->hp;
                                        target_unit->hp -= recup;
                                        if (dt > 0.0f )
                                            metal_prod += recup * unit_manager.unit_type[target_unit->type_id]->BuildCostMetal / (dt * unit_manager.unit_type[target_unit->type_id]->MaxDamage);
                                        if (target_unit->hp <= 0.0f ) {		// Work done
                                            launch_script(get_script_index(SCRIPT_stopbuilding));
                                            launch_script(get_script_index(SCRIPT_stop));
                                            target_unit->flags |= 0x10;			// This unit is being reclaimed it doesn't explode!
                                            next_mission();
                                        }
                                    }
                                }
                            }
                        }
                        else
                            next_mission();
                    }
                    else if (mission->data>=0 && mission->data<features.max_features )	{	// Reclaim a feature/wreckage
                        features.lock();
                        if (features.feature[mission->data].type <= 0 )	{
                            features.unlock();
                            next_mission();
                            break;
                        }
                        bool feature_locked = true;

                        Vector3D Dir=features.feature[mission->data].Pos-Pos;
                        Dir.y=0.0f;
                        mission->target=features.feature[mission->data].Pos;
                        float dist=Dir.sq();
                        int maxdist = mission->mission == MISSION_REVIVE ? (int)(unit_manager.unit_type[type_id]->SightDistance) : (int)(unit_manager.unit_type[type_id]->BuildDistance);
                        if (dist>maxdist*maxdist && unit_manager.unit_type[type_id]->BMcode) {	// If the unit is too far from its target
                            c_time = 0.0f;
                            mission->flags |= MISSION_FLAG_MOVE;// | MISSION_FLAG_REFRESH_PATH;
                            mission->move_data = maxdist*7/80;
                            mission->last_d=0.0f;
                        }
                        else if (!(mission->flags & MISSION_FLAG_MOVE) ) {
                            if (mission->last_d>=0.0f) {
                                int angle = (int)( acos( Dir.z / Dir.norm() ) * RAD2DEG );
                                if (Dir.x < 0.0f )
                                    angle = -angle;
                                angle -= (int)Angle.y;
                                if (angle>180)	angle-=360;
                                if (angle<-180)	angle+=360;
                                int param[] = { (int)(angle*DEG2TA) };
                                launch_script(get_script_index(SCRIPT_startbuilding), 1, param );
                                launch_script(get_script_index(SCRIPT_go));
                                mission->last_d=-1.0f;
                            }
                            if (unit_manager.unit_type[type_id]->BMcode && port[ INBUILDSTANCE ] != 0 ) {
                                if (local && network_manager.isConnected() && nanolathe_target < 0 ) {		// Synchronize nanolathe emission
                                    nanolathe_target = mission->data;
                                    g_ta3d_network->sendUnitNanolatheEvent( idx, mission->data, true, true );
                                }

                                play_sound( "working" );
                                // Reclaim the object
                                float recup=dt*unit_manager.unit_type[type_id]->WorkerTime;
                                if (recup>features.feature[mission->data].hp)	recup=features.feature[mission->data].hp;
                                features.feature[mission->data].hp-=recup;
                                if (dt > 0.0f && mission->mission == MISSION_RECLAIM ) {
                                    metal_prod+=recup*feature_manager.feature[features.feature[mission->data].type].metal/(dt*feature_manager.feature[features.feature[mission->data].type].damage);
                                    energy_prod+=recup*feature_manager.feature[features.feature[mission->data].type].energy/(dt*feature_manager.feature[features.feature[mission->data].type].damage);
                                }
                                if (features.feature[mission->data].hp <= 0.0f ) {		// Job done
                                    features.removeFeatureFromMap( mission->data );		// Remove the object from map

                                    if (mission->mission == MISSION_REVIVE
                                        && !feature_manager.feature[features.feature[mission->data].type].name.empty() ) {			// Creates the corresponding unit
                                        bool success = false;
                                        String wreckage_name = feature_manager.feature[features.feature[mission->data].type].name;
                                        wreckage_name = wreckage_name.substr( 0, wreckage_name.length() - 5 );		// Remove the _dead/_heap suffix

                                        int wreckage_type_id = unit_manager.get_unit_index( wreckage_name.c_str() );
                                        Vector3D obj_pos = features.feature[mission->data].Pos;
                                        float obj_angle = features.feature[mission->data].angle;
                                        features.unlock();
                                        feature_locked = false;
                                        if (network_manager.isConnected() )
                                            g_ta3d_network->sendFeatureDeathEvent( mission->data );
                                        features.delete_feature(mission->data);			// Delete the object

                                        if (wreckage_type_id >= 0 ) {
                                            pMutex.unlock();
                                            UNIT *unit_p = (UNIT*) create_unit( wreckage_type_id, owner_id, obj_pos, map );

                                            if (unit_p ) {
                                                unit_p->lock();

                                                unit_p->Angle.y = obj_angle;
                                                unit_p->hp = 0.01f;					// Need to be repaired :P
                                                unit_p->build_percent_left = 0.0f;	// It's finished ...
                                                unit_p->unlock();
                                                unit_p->draw_on_map();
                                            }
                                            pMutex.lock();

                                            if (unit_p ) {
                                                mission->mission = MISSION_REPAIR;		// Now let's repair what we've resurrected
                                                mission->p = unit_p;
                                                mission->data = 1;
                                                success = true;
                                            }
                                        }
                                        if (!success ) {
                                            play_sound( "cant1" );
                                            launch_script(get_script_index(SCRIPT_stopbuilding));
                                            launch_script(get_script_index(SCRIPT_stop));
                                            next_mission();
                                        }
                                    }
                                    else {
                                        features.unlock();
                                        feature_locked = false;
                                        if (network_manager.isConnected() )
                                            g_ta3d_network->sendFeatureDeathEvent( mission->data );
                                        features.delete_feature(mission->data);			// Delete the object
                                        launch_script(get_script_index(SCRIPT_stopbuilding));
                                        launch_script(get_script_index(SCRIPT_stop));
                                        next_mission();
                                    }
                                }
                            }
                        }
                        if (feature_locked )
                            features.unlock();
                    }
                    else
                        next_mission();
                    break;
                case MISSION_GUARD:
                    if (jump_commands)	break;
                    if (mission->p!=NULL && (((UNIT*)mission->p)->flags & 1) && ((UNIT*)mission->p)->owner_id==owner_id && ((UNIT*)mission->p)->ID == mission->target_ID) {		// On ne défend pas n'importe quoi
                        if (unit_manager.unit_type[type_id]->Builder)
                        {
                            if (((UNIT*)mission->p)->build_percent_left > 0.0f || ((UNIT*)mission->p)->hp<unit_manager.unit_type[((UNIT*)mission->p)->type_id]->MaxDamage) // Répare l'unité
                            {
                                add_mission(MISSION_REPAIR | MISSION_FLAG_AUTO,&((UNIT*)mission->p)->Pos,true,0,((UNIT*)mission->p),NULL);
                                break;
                            }
                            else
                                if (((UNIT*)mission->p)->mission!=NULL && (((UNIT*)mission->p)->mission->mission==MISSION_BUILD_2 || ((UNIT*)mission->p)->mission->mission==MISSION_REPAIR)) // L'aide à construire
                                {
                                    add_mission(MISSION_REPAIR | MISSION_FLAG_AUTO,&((UNIT*)mission->p)->mission->target,true,0,((UNIT*)mission->p)->mission->p,NULL);
                                    break;
                                }
                        }
                        if (unit_manager.unit_type[type_id]->canattack)
                        {
                            if (((UNIT*)mission->p)->mission!=NULL && ((UNIT*)mission->p)->mission->mission==MISSION_ATTACK) // L'aide à attaquer
                            {
                                add_mission(MISSION_ATTACK | MISSION_FLAG_AUTO,&((UNIT*)mission->p)->mission->target,true,0,((UNIT*)mission->p)->mission->p,NULL);
                                break;
                            }
                        }
                        if (((Vector3D)(Pos-((UNIT*)mission->p)->Pos)).sq()>=25600.0f) // On reste assez près
                        {
                            mission->flags |= MISSION_FLAG_MOVE;// | MISSION_FLAG_REFRESH_PATH;
                            mission->move_data = 10;
                            mission->target = ((UNIT*)mission->p)->Pos;
                            c_time=0.0f;
                            break;
                        }
                    }
                    else
                        next_mission();
                    break;
                case MISSION_PATROL:					// Mode patrouille
                    {
                        pad_timer += dt;

                        if (mission->next == NULL )
                            add_mission(MISSION_PATROL | MISSION_FLAG_AUTO,&Pos,false,0,NULL,NULL,MISSION_FLAG_CAN_ATTACK,0,0);	// Retour à la case départ après l'éxécution de tous les ordres / back to beginning

                        mission->flags |= MISSION_FLAG_CAN_ATTACK;
                        if (unit_manager.unit_type[type_id]->canfly ) // Don't stop moving and check if it can be repaired
                        {
                            mission->flags |= MISSION_FLAG_DONT_STOP_MOVE;

                            if (hp < unit_manager.unit_type[type_id]->MaxDamage && !attacked && pad_timer >= 5.0f ) // Check if a repair pad is free
                            {
                                bool attacking = false;
                                for (int i = 0 ; i < 3; ++i)
                                {
                                    if (weapon[i].state != WEAPON_FLAG_IDLE )
                                    {
                                        attacking = true;
                                        break;
                                    }
                                }
                                if (!attacking )
                                {
                                    pad_timer = 0.0f;
                                    bool going_to_repair_pad = false;
                                    pMutex.unlock();
                                    units.lock();
                                    for (std::list<uint16>::iterator i = units.repair_pads[owner_id].begin(); i != units.repair_pads[owner_id].end(); ++i)
                                    {
                                        units.unit[ *i ].lock();
                                        Vector3D Dir = units.unit[ *i ].Pos - Pos;
                                        Dir.y = 0.0f;
                                        if ((units.unit[ *i ].pad1 == 0xFFFF || units.unit[ *i ].pad2 == 0xFFFF) && units.unit[ *i ].build_percent_left == 0.0f
                                            && Dir.sq() <= SQUARE(unit_manager.unit_type[type_id]->ManeuverLeashLength)) // He can repair us :)
                                            {
                                                int target_idx = *i;
                                                units.unit[ target_idx ].unlock();
                                                pMutex.lock();
                                                add_mission( MISSION_GET_REPAIRED | MISSION_FLAG_AUTO, &units.unit[ *i ].Pos, true, 0, &(units.unit[ *i ]),NULL);
                                                pMutex.unlock();
                                                units.repair_pads[ owner_id ].erase(i);
                                                units.repair_pads[ owner_id ].push_back( target_idx );		// So we don't try it before others :)
                                                going_to_repair_pad = true;
                                                break;
                                            }
                                        units.unit[ *i ].unlock();
                                    }
                                    units.unlock();
                                    pMutex.lock();
                                    if (going_to_repair_pad )
                                        break;
                                }
                            }
                        }

                        if ((mission->flags & MISSION_FLAG_MOVE) == 0 ) // Monitor the moving process
                        {
                            if (!unit_manager.unit_type[type_id]->canfly || ( mission->next == NULL || ( mission->next != NULL && mission->mission != MISSION_PATROL ) ) )
                            {
                                V.x = V.y = V.z = 0.0f;			// Stop the unit
                                if (precomputed_position )
                                {
                                    NPos = Pos;
                                    n_px = cur_px;
                                    n_py = cur_py;
                                }
                            }

                            MISSION* cur = mission;					// Make a copy of current list to make it loop 8)
                            while (cur->next)
                                cur = cur->next;
                            cur->next = new MISSION();
                            *(cur->next) = *mission;
                            cur->next->path = NULL;
                            cur->next->next = NULL;
                            cur->next->flags |= MISSION_FLAG_MOVE;

                            MISSION *cur_start = mission->next;
                            while( cur_start != NULL && cur_start->mission != MISSION_PATROL )
                            {
                                cur = cur_start;
                                while (cur->next)
                                    cur = cur->next;
                                cur->next = new MISSION();
                                *(cur->next) = *cur_start;
                                cur->next->path = NULL;
                                cur->next->next = NULL;
                                cur_start = cur_start->next;
                            }

                            next_mission();
                        }
                    }
                    break;
                case MISSION_STANDBY:
                case MISSION_VTOL_STANDBY:
                    if (jump_commands)	break;
                    if (mission->data>5)
                    {
                        if (mission->next)		// If there is a mission after this one
                        {
                            next_mission();
                            if (mission && (mission->mission == MISSION_STANDBY || mission->mission == MISSION_VTOL_STANDBY))
                                mission->data = 0;
                        }
                    }
                    else
                        mission->data++;
                    break;
                case MISSION_ATTACK:										// Attaque une unité / attack a unit
                    {
                        UNIT *target_unit = (mission->flags & MISSION_FLAG_TARGET_WEAPON) == MISSION_FLAG_TARGET_WEAPON ? NULL : (UNIT*) mission->p;
                        WEAPON *target_weapon = (mission->flags & MISSION_FLAG_TARGET_WEAPON) == MISSION_FLAG_TARGET_WEAPON ? (WEAPON*) mission->p : NULL;
                        if ((target_unit!=NULL && (target_unit->flags&1) && target_unit->ID == mission->target_ID) || (target_weapon!=NULL && target_weapon->weapon_id!=-1) || (target_weapon==NULL && target_unit==NULL) ) {

                            if (target_unit ) {				// Check if we can target the unit
                                byte mask = 1 << owner_id;
                                if (target_unit->cloaked && !target_unit->is_on_radar( mask ) ) {
                                    for( int i = 0 ; i < weapon.size() ; i++ )
                                        if (weapon[ i ].target == mission->p )		// Stop shooting
                                            weapon[ i ].state = WEAPON_FLAG_IDLE;
                                    next_mission();
                                    break;
                                }
                            }

                            if (jump_commands && mission->data != 0
                                && unit_manager.unit_type[type_id]->attackrunlength == 0 )	break;					// Just do basic checks every tick, and advanced ones when needed

                            if (weapon.size() == 0
                                && !unit_manager.unit_type[type_id]->kamikaze ) {		// Check if this units has weapons
                                next_mission();		break;	}

                            Vector3D Dir = target_unit==NULL ? (target_weapon == NULL ? mission->target-Pos : target_weapon->Pos-Pos) : target_unit->Pos-Pos;
                            Dir.y = 0.0f;
                            if (target_weapon || target_unit )
                                mission->target = target_unit==NULL ? target_weapon->Pos : target_unit->Pos;
                            float dist = Dir.sq();
                            int maxdist = 0;
                            int mindist = 0xFFFFF;

                            if (target_unit != NULL && unit_manager.unit_type[target_unit->type_id]->checkCategory( unit_manager.unit_type[type_id]->BadTargetCategory ) ) {
                                next_mission();
                                break;
                            }

                            for( int i = 0 ; i < weapon.size() ; i++ ) {
                                if (unit_manager.unit_type[type_id]->weapon[ i ]==NULL || unit_manager.unit_type[type_id]->weapon[ i ]->interceptor)	continue;
                                int cur_mindist;
                                int cur_maxdist;
                                bool allowed_to_fire = true;
                                if (unit_manager.unit_type[type_id]->attackrunlength>0) {
                                    if (Dir % V < 0.0f )	allowed_to_fire = false;
                                    float t = 2.0f/map->ota_data.gravity*fabs(Pos.y-mission->target.y);
                                    cur_mindist = (int)sqrt(t*V.sq())-((unit_manager.unit_type[type_id]->attackrunlength+1)>>1);
                                    cur_maxdist = cur_mindist+(unit_manager.unit_type[type_id]->attackrunlength);
                                }
                                else {
                                    cur_maxdist = unit_manager.unit_type[type_id]->weapon[ i ]->range>>1;
                                    cur_mindist = 0;
                                }
                                if (maxdist < cur_maxdist )	maxdist = cur_maxdist;
                                if (mindist > cur_mindist )	mindist = cur_mindist;
                                if (allowed_to_fire && dist >= cur_mindist * cur_mindist && dist <= cur_maxdist * cur_maxdist && !unit_manager.unit_type[type_id]->weapon[ i ]->interceptor )
                                {
                                    if (( (weapon[i].state & 3) == WEAPON_FLAG_IDLE || ( (weapon[i].state & 3) != WEAPON_FLAG_IDLE && weapon[i].target != mission->p ) )
                                        && ( target_unit == NULL || ( (!unit_manager.unit_type[type_id]->weapon[ i ]->toairweapon
                                                                       || ( unit_manager.unit_type[type_id]->weapon[ i ]->toairweapon && target_unit->flying ) )
                                                                      && !unit_manager.unit_type[target_unit->type_id]->checkCategory( unit_manager.unit_type[type_id]->w_badTargetCategory[i] ) ) )
                                        && ( ((mission->flags & MISSION_FLAG_COMMAND_FIRE) && (unit_manager.unit_type[type_id]->weapon[ i ]->commandfire || !unit_manager.unit_type[type_id]->candgun) )
                                             || (!(mission->flags & MISSION_FLAG_COMMAND_FIRE) && !unit_manager.unit_type[type_id]->weapon[ i ]->commandfire)
                                             || unit_manager.unit_type[type_id]->weapon[ i ]->dropped ) ) {
                                        weapon[i].state = WEAPON_FLAG_AIM;
                                        weapon[i].target = mission->p;
                                        weapon[i].target_pos = mission->target;
                                        weapon[i].data = -1;
                                        if (mission->flags & MISSION_FLAG_TARGET_WEAPON )
                                            weapon[i].state |= WEAPON_FLAG_WEAPON;
                                        if (unit_manager.unit_type[type_id]->weapon[ i ]->commandfire )
                                            weapon[i].state |= WEAPON_FLAG_COMMAND_FIRE;
                                    }
                                }
                            }

                            if (unit_manager.unit_type[type_id]->kamikaze && unit_manager.unit_type[type_id]->kamikazedistance > maxdist )
                                maxdist = unit_manager.unit_type[type_id]->kamikazedistance;

                            if (mindist > maxdist )	mindist = maxdist;

                            mission->flags |= MISSION_FLAG_CAN_ATTACK;

                            if (unit_manager.unit_type[type_id]->kamikaze				// Kamikaze attack !!
                                && dist <= unit_manager.unit_type[type_id]->kamikazedistance * unit_manager.unit_type[type_id]->kamikazedistance
                                && self_destruct < 0.0f )
                                self_destruct = 0.01f;

                            if (dist>maxdist*maxdist || dist<mindist*mindist) {	// Si l'unité est trop loin de sa cible / if unit isn't where it should be
                                if (!unit_manager.unit_type[type_id]->canmove) {		// Bah là si on peut pas bouger faut changer de cible!! / need to change target
                                    next_mission();
                                    break;
                                }
                                else if (!unit_manager.unit_type[type_id]->canfly || unit_manager.unit_type[type_id]->hoverattack ) {
                                    c_time=0.0f;
                                    mission->flags |= MISSION_FLAG_MOVE;
                                    mission->move_data = maxdist*7/80;
                                }
                            }
                            else if (mission->data == 0)
                            {
                                mission->data = 2;
                                int param[] = { 0 };
                                for( int i = 0 ; i < weapon.size() ; i++ )
                                    if (unit_manager.unit_type[type_id]->weapon[ i ] )
                                        param[ 0 ] = Math::Max(param[0], (int)( unit_manager.unit_type[type_id]->weapon[i]->reloadtime * 1000.0f) * Math::Max(1, (int)unit_manager.unit_type[type_id]->weapon[i]->burst));
                                launch_script(get_script_index(SCRIPT_SetMaxReloadTime),1,param);
                            }

                            if (mission->flags & MISSION_FLAG_COMMAND_FIRED)
                                next_mission();
                        }
                        else
                            next_mission();
                    }
                    break;
                case MISSION_GUARD_NOMOVE:
                    mission->flags |= MISSION_FLAG_CAN_ATTACK;
                    mission->flags &= ~MISSION_FLAG_MOVE;
                    if (mission->next)
                        next_mission();
                    break;
                case MISSION_STOP:											// Arrête tout ce qui était en cours / stop everything running
                    while (mission->next
                        && (mission->next->mission == MISSION_STOP
                            || mission->next->mission == MISSION_STANDBY
                            || mission->next->mission == MISSION_VTOL_STANDBY))     // Don't make a big stop stack :P
                        next_mission();
                    mission->mission = MISSION_STOP;
                    if (jump_commands && mission->data != 0 )	break;
                    if (mission->data>5)
                    {
                        if (mission->next)
                        {
                            next_mission();
                            if (mission!=NULL && mission->mission==MISSION_STOP)		// Mode attente / wait mode
                                mission->data=1;
                        }
                    }
                    else
                    {
                        if (mission->data==0)
                        {
                            launch_script(get_script_index(SCRIPT_StopMoving));		// Arrête tout / stop everything
                            launch_script(get_script_index(SCRIPT_stopbuilding));
                            for( int i = 0 ; i < weapon.size() ; i++ )
                                if (weapon[i].state)
                                {
                                    launch_script(get_script_index(SCRIPT_TargetCleared));
                                    break;
                                }
                            for( int i = 0 ; i < weapon.size() ; i++ )			// Stop weapons
                            {
                                weapon[i].state = WEAPON_FLAG_IDLE;
                                weapon[i].data = -1;
                            }
                        }
                        mission->data++;
                    }
                    break;
                case MISSION_REPAIR:
                    {
                        UNIT *target_unit=(UNIT*) mission->p;
                        if (target_unit!=NULL && (target_unit->flags & 1) && target_unit->build_percent_left == 0.0f && target_unit->ID == mission->target_ID) {
                            if (target_unit->hp>=unit_manager.unit_type[target_unit->type_id]->MaxDamage || !unit_manager.unit_type[type_id]->BMcode) {
                                if (unit_manager.unit_type[type_id]->BMcode)
                                    target_unit->hp=unit_manager.unit_type[target_unit->type_id]->MaxDamage;
                                next_mission();
                            }
                            else
                            {
                                Vector3D Dir=target_unit->Pos-Pos;
                                Dir.y=0.0f;
                                mission->target=target_unit->Pos;
                                float dist=Dir.sq();
                                int maxdist=(int)(unit_manager.unit_type[type_id]->BuildDistance
                                                  + ( (unit_manager.unit_type[target_unit->type_id]->FootprintX + unit_manager.unit_type[target_unit->type_id]->FootprintZ) << 1 ) );
                                if (dist>maxdist*maxdist && unit_manager.unit_type[type_id]->BMcode) {	// Si l'unité est trop loin du chantier
                                    mission->flags |= MISSION_FLAG_MOVE;
                                    mission->move_data = maxdist * 7 / 80;
                                    mission->data = 0;
                                    c_time = 0.0f;
                                }
                                else if (!(mission->flags & MISSION_FLAG_MOVE) ) {
                                    if (mission->data==0) {
                                        mission->data=1;
                                        int angle = (int)( acos( Dir.z / Dir.norm() ) * RAD2DEG );
                                        if (Dir.x < 0.0f )
                                            angle = -angle;
                                        angle -= (int)Angle.y;
                                        if (angle>180)	angle-=360;
                                        if (angle<-180)	angle+=360;
                                        int param[] = { (int)(angle*DEG2TA) };
                                        launch_script(get_script_index(SCRIPT_startbuilding), 1, param );
                                        launch_script(get_script_index(SCRIPT_go));
                                    }

                                    if (port[ INBUILDSTANCE ] != 0.0f ) {
                                        if (local && network_manager.isConnected() && nanolathe_target < 0 ) {		// Synchronize nanolathe emission
                                            nanolathe_target = target_unit->idx;
                                            g_ta3d_network->sendUnitNanolatheEvent( idx, target_unit->idx, false, false );
                                        }

                                        float conso_energy=((float)(unit_manager.unit_type[type_id]->WorkerTime*unit_manager.unit_type[target_unit->type_id]->BuildCostEnergy))/unit_manager.unit_type[target_unit->type_id]->BuildTime;
                                        if (players.energy[owner_id] >= (energy_cons + conso_energy) * dt ) {
                                            energy_cons += conso_energy;
                                            target_unit->hp += dt*unit_manager.unit_type[type_id]->WorkerTime*unit_manager.unit_type[target_unit->type_id]->MaxDamage/unit_manager.unit_type[target_unit->type_id]->BuildTime;
                                        }
                                        target_unit->built=true;
                                    }
                                }
                            }
                        }
                        else if (target_unit!=NULL && target_unit->flags) {
                            Vector3D Dir=target_unit->Pos-Pos;
                            Dir.y=0.0f;
                            mission->target=target_unit->Pos;
                            float dist=Dir.sq();
                            int maxdist=(int)(unit_manager.unit_type[type_id]->BuildDistance
                                              + ( (unit_manager.unit_type[target_unit->type_id]->FootprintX + unit_manager.unit_type[target_unit->type_id]->FootprintZ) << 1 ));
                            if (dist>maxdist*maxdist && unit_manager.unit_type[type_id]->BMcode) {	// Si l'unité est trop loin du chantier
                                c_time=0.0f;
                                mission->flags |= MISSION_FLAG_MOVE;
                                mission->move_data = maxdist*7/80;
                            }
                            else if (!(mission->flags & MISSION_FLAG_MOVE) ) {
                                if (unit_manager.unit_type[type_id]->BMcode ) {
                                    int angle = (int)( acos( Dir.z / Dir.norm() ) * RAD2DEG );
                                    if (Dir.x < 0.0f )
                                        angle = -angle;
                                    angle -= (int)Angle.y;
                                    if (angle>180)	angle-=360;
                                    if (angle<-180)	angle+=360;
                                    int param[] = { (int)(angle*DEG2TA) };
                                    launch_script(get_script_index(SCRIPT_startbuilding), 1, param );
                                    mission->mission = MISSION_BUILD_2;		// Change de type de mission
                                }
                            }
                        }
                    }
                    break;
                case MISSION_BUILD_2:
                    {
                        UNIT *target_unit=(UNIT*) mission->p;
                        if (target_unit->flags && target_unit->ID == mission->target_ID) {
                            target_unit->lock();
                            if (target_unit->build_percent_left <= 0.0f) {
                                target_unit->build_percent_left = 0.0f;
                                if (unit_manager.unit_type[target_unit->type_id]->ActivateWhenBuilt ) {		// Start activated
                                    target_unit->port[ ACTIVATION ] = 0;
                                    target_unit->activate();
                                }
                                if (unit_manager.unit_type[target_unit->type_id]->init_cloaked )				// Start cloaked
                                    target_unit->cloaking = true;
                                if (!unit_manager.unit_type[type_id]->BMcode) // Ordre de se déplacer
                                {
                                    Vector3D target=Pos;
                                    target.z+=128.0f;
                                    if (def_mission == NULL)
                                        target_unit->set_mission(MISSION_MOVE | MISSION_FLAG_AUTO,&target,false,5,true,NULL,NULL,0,5);		// Fait sortir l'unité du bâtiment
                                    else
                                    {
                                        target_unit->mission = new MISSION();
                                        MISSION *target_mission = target_unit->mission;
                                        *target_mission = *def_mission;
                                        target_mission->next = NULL;
                                        target_mission->path = NULL;
                                        while (target_mission->next != NULL)
                                            target_mission = target_mission->next;
                                        MISSION *cur = def_mission->next;
                                        while (cur)// Copy mission list
                                        {
                                            target_mission->next = new MISSION();
                                            target_mission = target_mission->next;
                                            *target_mission = *cur;
                                            target_mission->next = NULL;
                                            target_mission->path = NULL;
                                            cur = cur->next;
                                        }
                                    }
                                }
                                mission->p=NULL;
                                next_mission();
                            }
                            else if (port[ INBUILDSTANCE ] != 0 ) {
                                if (local && network_manager.isConnected() && nanolathe_target < 0 ) // Synchronize nanolathe emission
                                {
                                    nanolathe_target = target_unit->idx;
                                    g_ta3d_network->sendUnitNanolatheEvent( idx, target_unit->idx, false, false );
                                }

                                float conso_metal=((float)(unit_manager.unit_type[type_id]->WorkerTime*unit_manager.unit_type[target_unit->type_id]->BuildCostMetal))/unit_manager.unit_type[target_unit->type_id]->BuildTime;
                                float conso_energy=((float)(unit_manager.unit_type[type_id]->WorkerTime*unit_manager.unit_type[target_unit->type_id]->BuildCostEnergy))/unit_manager.unit_type[target_unit->type_id]->BuildTime;
                                if (players.metal[owner_id]>= (metal_cons + conso_metal)*dt && players.energy[owner_id]>= (energy_cons+conso_energy)*dt)
                                {
                                    metal_cons+=conso_metal;
                                    energy_cons+=conso_energy;
                                    target_unit->build_percent_left-=dt*unit_manager.unit_type[type_id]->WorkerTime*100.0f/unit_manager.unit_type[target_unit->type_id]->BuildTime;
                                    target_unit->hp+=dt*unit_manager.unit_type[type_id]->WorkerTime*unit_manager.unit_type[target_unit->type_id]->MaxDamage/unit_manager.unit_type[target_unit->type_id]->BuildTime;
                                }
                                if (!unit_manager.unit_type[type_id]->BMcode)
                                {
                                    int script_id_buildinfo = get_script_index(SCRIPT_QueryBuildInfo);
                                    if (script_id_buildinfo>=0)
                                    {
                                        compute_model_coord();
                                        Vector3D old_pos = target_unit->Pos;
                                        if (script_val->size() <= script_id_buildinfo )
                                            script_val->resize( script_id_buildinfo + 1 );
                                        target_unit->Pos=Pos+data.pos[(*script_val)[script_id_buildinfo]];
                                        if (unit_manager.unit_type[target_unit->type_id]->Floater || ( unit_manager.unit_type[target_unit->type_id]->canhover && old_pos.y <= map->sealvl ) )
                                            target_unit->Pos.y = old_pos.y;
                                        if (((Vector3D)(old_pos-target_unit->Pos)).sq() > 1000000.0f) // It must be continuous
                                        {
                                            target_unit->Pos.x = old_pos.x;
                                            target_unit->Pos.z = old_pos.z;
                                        }
                                        else
                                        {
                                            target_unit->cur_px = ((int)(target_unit->Pos.x)+map->map_w_d+4)>>3;
                                            target_unit->cur_py = ((int)(target_unit->Pos.z)+map->map_h_d+4)>>3;
                                        }
                                        target_unit->Angle = Angle;
                                        target_unit->Angle.y += data.axe[1][(*script_val)[script_id_buildinfo]].angle;
                                        pMutex.unlock();
                                        target_unit->draw_on_map();
                                        pMutex.lock();
                                    }
                                }
                                mission->target = target_unit->Pos;
                                target_unit->built=true;
                            }
                            else {
                                activate();
                                target_unit->built=true;
                            }
                            target_unit->unlock();
                        }
                        else
                            next_mission();
                    }
                    break;
                case MISSION_BUILD:
                    if (mission->p)
                    {
                        Vector3D Dir = mission->target - Pos;
                        Dir.y = 0.0f;
                        int angle = (int)( acos( Dir.z / Dir.norm() ) * RAD2DEG );
                        if (Dir.x < 0.0f )
                            angle = -angle;
                        angle -= (int)Angle.y;
                        if (angle>180)	angle-=360;
                        if (angle<-180)	angle+=360;
                        int param[] = { (int)(angle*DEG2TA) };
                        launch_script(get_script_index(SCRIPT_startbuilding), 1, param );
                        mission->mission = MISSION_BUILD_2;		// Change mission type
                        ((UNIT*)(mission->p))->built = true;
                        play_sound( "build" );
                    }
                    else
                    {
                        Vector3D Dir = mission->target - Pos;
                        Dir.y = 0.0f;
                        float dist = Dir.sq();
                        int maxdist = (int)(unit_manager.unit_type[type_id]->BuildDistance
                                            + ( (unit_manager.unit_type[mission->data]->FootprintX + unit_manager.unit_type[mission->data]->FootprintZ) << 1 ) );
                        if (dist>maxdist*maxdist && unit_manager.unit_type[type_id]->BMcode) {	// Si l'unité est trop loin du chantier
                            mission->flags |= MISSION_FLAG_MOVE;
                            mission->move_data = maxdist * 7 / 80;
                        }
                        else {
                            if (mission->flags & MISSION_FLAG_MOVE )			// Stop moving if needed
                                stop_moving();
                            if (unit_manager.unit_type[type_id]->BMcode || (!unit_manager.unit_type[type_id]->BMcode && port[ INBUILDSTANCE ] && port[YARD_OPEN] && !port[BUGGER_OFF])) {
                                /*								pMutex.unlock();
                                                                draw_on_map();
                                                                pMutex.lock();*/
                                V.x = 0.0f;
                                V.y = 0.0f;
                                V.z = 0.0f;
                                if (!unit_manager.unit_type[type_id]->BMcode ) {
                                    int script_id_buildinfo = get_script_index(SCRIPT_QueryBuildInfo);
                                    if (script_id_buildinfo >= 0 ) {
                                        int param[] = { -1 };
                                        run_script_function( map, script_id_buildinfo, 1, param );
                                        if (param[0] >= 0 ) {
                                            compute_model_coord();
                                            mission->target = Pos + data.pos[ param[0] ];
                                        }
                                    }
                                }
                                if (map->check_rect((((int)(mission->target.x)+map->map_w_d+4)>>3)-(unit_manager.unit_type[mission->data]->FootprintX>>1),(((int)(mission->target.z)+map->map_h_d+4)>>3)-(unit_manager.unit_type[mission->data]->FootprintZ>>1),unit_manager.unit_type[mission->data]->FootprintX,unit_manager.unit_type[mission->data]->FootprintZ,-1)) // Check it we have an empty place to build our unit
                                {
                                    pMutex.unlock();
                                    mission->p = create_unit(mission->data,owner_id,mission->target,map);
                                    pMutex.lock();
                                    if (mission->p)
                                    {
                                        mission->target_ID = ((UNIT*)mission->p)->ID;
                                        ((UNIT*)(mission->p))->hp = 0.000001f;
                                        ((UNIT*)(mission->p))->built = true;
                                    }
                                    else
                                        LOG_WARNING(idx << " can't create unit! (`" << __FILE__ << "`:" << __LINE__ << ")");
                                }
                                else if (unit_manager.unit_type[type_id]->BMcode)
                                    next_mission();
                            }
                            else {
                                activate();
                                run_script_function( map, get_script_index(SCRIPT_QueryBuildInfo) );
                            }
                        }
                    }
                    break;
            };

            switch(unit_manager.unit_type[type_id]->TEDclass)			// Commandes particulières
            {
                case CLASS_PLANT:
                    switch(mission->mission)
                    {
                        case MISSION_STANDBY:
                        case MISSION_BUILD:
                        case MISSION_BUILD_2:
                        case MISSION_REPAIR:
                            break;
                        default:
                            next_mission();
                    };
                    break;
                case CLASS_WATER:
                case CLASS_VTOL:
                case CLASS_KBOT:
                case CLASS_COMMANDER:
                case CLASS_TANK:
                case CLASS_CNSTR:
                case CLASS_SHIP:
                    {
                        if (!(mission->flags & MISSION_FLAG_MOVE) && !(mission->flags & MISSION_FLAG_DONT_STOP_MOVE)
                            && ((mission->mission!=MISSION_ATTACK && unit_manager.unit_type[type_id]->canfly) || !unit_manager.unit_type[type_id]->canfly)) {
                            if (!flying )
                                V.x = V.z = 0.0f;
                            if (precomputed_position ) {
                                NPos = Pos;
                                n_px = cur_px;
                                n_py = cur_py;
                            }
                        }
                        switch(mission->mission)
                        {
                            case MISSION_ATTACK:
                            case MISSION_PATROL:
                            case MISSION_REPAIR:
                            case MISSION_BUILD:
                            case MISSION_BUILD_2:
                            case MISSION_GET_REPAIRED:
                                if (unit_manager.unit_type[type_id]->canfly)
                                    activate();
                                break;
                            case MISSION_STANDBY:
                                if (mission->next)
                                    next_mission();
                                V.x = V.y = V.z = 0.0f;			// Frottements
                                break;
                            case MISSION_MOVE:
                                mission->flags |= MISSION_FLAG_CAN_ATTACK;
                                if (!(mission->flags & MISSION_FLAG_MOVE) ) {			// Monitor the moving process
                                    if (mission->next
                                        && (mission->next->mission == MISSION_MOVE
                                            || (mission->next->mission == MISSION_STOP && mission->next->next && mission->next->next->mission == MISSION_MOVE) ) )
                                        mission->flags |= MISSION_FLAG_DONT_STOP_MOVE;

                                    if (!(mission->flags & MISSION_FLAG_DONT_STOP_MOVE) )			// If needed
                                        V.x = V.y = V.z = 0.0f;			// Stop the unit
                                    if (precomputed_position ) {
                                        NPos = Pos;
                                        n_px = cur_px;
                                        n_py = cur_py;
                                    }
                                    if ((mission->flags & MISSION_FLAG_DONT_STOP_MOVE) && mission->next && mission->next->mission == MISSION_STOP )			// If needed
                                        next_mission();
                                    next_mission();
                                }
                                break;
                            default:
                                if (unit_manager.unit_type[type_id]->canfly)
                                    deactivate();
                        };
                    }
                    break;
                case CLASS_UNDEF:
                case CLASS_METAL:
                case CLASS_ENERGY:
                case CLASS_SPECIAL:
                case CLASS_FORT:
                    break;
                default:
                    LOG_WARNING("Unknown type :" << unit_manager.unit_type[type_id]->TEDclass);
            };

            switch(mission->mission)		// Pour le code post déplacement
            {
                case MISSION_ATTACK:
//                    if (unit_manager.unit_type[type_id]->canfly && !unit_manager.unit_type[type_id]->hoverattack ) {			// Un avion??
                    if (unit_manager.unit_type[type_id]->canfly)			// Un avion??
                    {
                        activate();
                        mission->flags &= ~MISSION_FLAG_MOVE;			// We're doing it here, so no need to do it twice
                        Vector3D J,I,K;
                        K.x=K.z=0.0f;
                        K.y=1.0f;
                        Vector3D Target = mission->target;
                        J = Target-Pos;
                        J.y = 0.0f;
                        float dist = J.norm();
                        mission->last_d = dist;
                        if (dist > 0.0f)
                            J = 1.0f / dist * J;
                        if (dist > unit_manager.unit_type[type_id]->ManeuverLeashLength ) {
                            b_TargetAngle = true;
                            f_TargetAngle = acos(J.z) * RAD2DEG;
                            if (J.x < 0.0f) f_TargetAngle = -f_TargetAngle;
                        }

                        J.z = cos(Angle.y * DEG2RAD);
                        J.x = sin(Angle.y * DEG2RAD);
                        J.y = 0.0f;
                        I.z = -J.x;
                        I.x = J.z;
                        I.y = 0.0f;
                        V = (V%K)*K+(V%J)*J+units.exp_dt_4*(V%I)*I;
                        float speed = V.sq();
                        if (speed < unit_manager.unit_type[type_id]->MaxVelocity * unit_manager.unit_type[type_id]->MaxVelocity )
                            V=V+unit_manager.unit_type[type_id]->Acceleration*dt*J;
                    }
                    break;
            }

            if (( (mission->flags & MISSION_FLAG_MOVE) || !local ) && !jump_commands )// Set unit orientation if it's on the ground
            {
                if (!unit_manager.unit_type[type_id]->canfly && !unit_manager.unit_type[type_id]->Upright
                    && unit_manager.unit_type[type_id]->TEDclass!=CLASS_SHIP
                    && unit_manager.unit_type[type_id]->TEDclass!=CLASS_WATER
                    && !( unit_manager.unit_type[type_id]->canhover && Pos.y <= map->sealvl ) )
                {
                    Vector3D I,J,K,A,B,C;
                    MATRIX_4x4 M = RotateY((Angle.y+90.0f)*DEG2RAD);
                    I.x = 4.0f;
                    J.z = 4.0f;
                    K.y = 1.0f;
                    A = Pos - unit_manager.unit_type[type_id]->FootprintZ*I*M;
                    B = Pos + (unit_manager.unit_type[type_id]->FootprintX*I-unit_manager.unit_type[type_id]->FootprintZ*J)*M;
                    C = Pos + (unit_manager.unit_type[type_id]->FootprintX*I+unit_manager.unit_type[type_id]->FootprintZ*J)*M;
                    A.y = map->get_unit_h(A.x,A.z);	// Projete le triangle
                    B.y = map->get_unit_h(B.x,B.z);
                    C.y = map->get_unit_h(C.x,C.z);
                    Vector3D D=(B-A)*(B-C);
                    if (D.y>=0.0f) // On ne met pas une unité à l'envers!!
                    {
                        D.unit();
                        float dist_sq = sqrt( D.y*D.y+D.z*D.z );
                        float angle_1= dist_sq != 0.0f ? acos( D.y / dist_sq )*RAD2DEG : 0.0f;
                        if (D.z<0.0f)	angle_1=-angle_1;
                        D=D*RotateX(-angle_1*DEG2RAD);
                        float angle_2=VAngle(D,K)*RAD2DEG;
                        if (D.x>0.0f)	angle_2=-angle_2;
                        if (fabs(angle_1-Angle.x)<=10.0f && fabs(angle_2-Angle.z)<=10.0f) {
                            Angle.x=angle_1;
                            Angle.z=angle_2;
                        }
                    }
                }
                else if (!unit_manager.unit_type[type_id]->canfly )
                    Angle.x = Angle.z = 0.0f;
            }

            bool returning_fire = ( port[ STANDINGFIREORDERS ] == SFORDER_RETURN_FIRE && attacked );
            if (( ((mission->flags & MISSION_FLAG_CAN_ATTACK) == MISSION_FLAG_CAN_ATTACK) || do_nothing() )
                && ( port[ STANDINGFIREORDERS ] == SFORDER_FIRE_AT_WILL || returning_fire )
                && !jump_commands && local ) {
                // Si l'unité peut attaquer d'elle même les unités enemies proches, elle le fait / Attack nearby enemies

                bool can_fire = unit_manager.unit_type[type_id]->AutoFire && unit_manager.unit_type[type_id]->canattack;

                if (!can_fire )
                    for( int i = 0 ; i < weapon.size() && !can_fire ; i++ )
                        can_fire = unit_manager.unit_type[type_id]->weapon[i] != NULL && !unit_manager.unit_type[type_id]->weapon[i]->commandfire && weapon[i].state == WEAPON_FLAG_IDLE;

                if (can_fire) {
                    int dx=(unit_manager.unit_type[type_id]->SightDistance+(int)(h+0.5f))>>3;
                    int enemy_idx=-1;
                    for( int i = 0 ; i < weapon.size() ; i++ )
                        if (unit_manager.unit_type[type_id]->weapon[i] != NULL && (unit_manager.unit_type[type_id]->weapon[i]->range>>4)>dx)
                            dx=unit_manager.unit_type[type_id]->weapon[i]->range>>4;
                    if (unit_manager.unit_type[type_id]->kamikaze && (unit_manager.unit_type[type_id]->kamikazedistance>>3) > dx )
                        dx=unit_manager.unit_type[type_id]->kamikazedistance;

                    int sx=Math::RandFromTable()&0xF;
                    int sy=Math::RandFromTable()&0xF;
                    byte mask=1<<owner_id;
                    for(int y=cur_py-dx+sy;y<=cur_py+dx;y+=0x8) {
                        if (y>=0 && y<map->bloc_h_db-1)
                            for(int x=cur_px-dx+sx;x<=cur_px+dx;x+=0x8)
                                if (x>=0 && x<map->bloc_w_db-1 ) {
                                    bool land_test = true;
                                    IDX_LIST_NODE *cur = map->map_data[y][x].air_idx.head;
                                    for( ; land_test || cur != NULL ; ) {
                                        int cur_idx;
                                        if (land_test )
                                        {
                                            cur_idx = map->map_data[y][x].unit_idx;
                                            land_test = false;
                                        }
                                        else {
                                            cur_idx = cur->idx;
                                            cur = cur->next;
                                        }
                                        if ( isEnemy( cur_idx ) && units.unit[cur_idx].flags
                                             && unit_manager.unit_type[units.unit[cur_idx].type_id]->ShootMe
                                             && ( units.unit[cur_idx].is_on_radar( mask ) ||
                                                  ( (units.map->sight_map->line[y>>1][x>>1] & mask)
                                                    && !units.unit[cur_idx].cloaked ) )
                                             && !unit_manager.unit_type[ units.unit[cur_idx].type_id ]->checkCategory( unit_manager.unit_type[type_id]->BadTargetCategory ) )
                                        {
                                            if (returning_fire)
                                            {
                                                bool breakBool = false;
                                                for(int i = 0 ; i < units.unit[cur_idx].weapon.size() ; i++)
                                                    if( units.unit[cur_idx].weapon[i].state != WEAPON_FLAG_IDLE && units.unit[cur_idx].weapon[i].target == this )
                                                    {
                                                        enemy_idx = cur_idx;
                                                        x = cur_px + dx;
                                                        y = cur_py + dx;
                                                        breakBool = true;
                                                        break;
                                                    }
                                                if (breakBool)  break;
                                            }
                                            else
                                            {
                                                enemy_idx = cur_idx;
                                                x = cur_px + dx;
                                                y = cur_py + dx;
                                                break;
                                            }
                                        }
                                    }
                                }
                        if (enemy_idx>=0)	break;
                    }
                    if (enemy_idx>=0) {			// Si on a trouvé une unité, on l'attaque
                        if (do_nothing() )
                            set_mission(MISSION_ATTACK | MISSION_FLAG_AUTO,&(units.unit[enemy_idx].Pos),false,0,true,&(units.unit[enemy_idx]),NULL);
                        else
                            for( int i = 0 ; i < weapon.size() ; i++ )
                                if (weapon[i].state == WEAPON_FLAG_IDLE && unit_manager.unit_type[type_id]->weapon[ i ] != NULL
                                    && !unit_manager.unit_type[type_id]->weapon[ i ]->commandfire
                                    && !unit_manager.unit_type[type_id]->weapon[ i ]->interceptor
                                    && (!unit_manager.unit_type[type_id]->weapon[ i ]->toairweapon
                                        || ( unit_manager.unit_type[type_id]->weapon[ i ]->toairweapon && units.unit[enemy_idx].flying )
                                        && !unit_manager.unit_type[ units.unit[enemy_idx].type_id ]->checkCategory( unit_manager.unit_type[type_id]->w_badTargetCategory[i] ) ) )
                                {
                                    weapon[i].state = WEAPON_FLAG_AIM;
                                    weapon[i].target = &(units.unit[enemy_idx]);
                                    weapon[i].data = -1;
                                }
                    }
                }
                if (weapon.size() > 0 && unit_manager.unit_type[type_id]->antiweapons && unit_manager.unit_type[type_id]->weapon[0])
                {
                    float coverage=unit_manager.unit_type[type_id]->weapon[0]->coverage*unit_manager.unit_type[type_id]->weapon[0]->coverage;
                    float range=unit_manager.unit_type[type_id]->weapon[0]->range*unit_manager.unit_type[type_id]->weapon[0]->range>>2;
                    int enemy_idx=-1;
                    byte e=0;
                    for(byte i=0;i+e<mem_size;i++)
                    {
                        if (memory[i+e]<0 || memory[i+e]>=weapons.nb_weapon || weapons.weapon[memory[i+e]].weapon_id==-1)
                        {
                            e++;
                            i--;
                            continue;
                        }
                        memory[i] = memory[i+e];
                    }
                    mem_size -= e;
                    for(uint32 f=0;f<weapons.index_list_size;f+=(Math::RandFromTable()&7)+1)
                    {
                        uint32 i = weapons.idx_list[f];
                        // Yes we don't defend against allies :D, can lead to funny situations :P
                        if (weapons.weapon[i].weapon_id!=-1 && !(players.team[ units.unit[weapons.weapon[i].shooter_idx].owner_id ] & players.team[ owner_id ])
                            && weapon_manager.weapon[weapons.weapon[i].weapon_id].targetable)
                        {
                            if (((Vector3D)(weapons.weapon[i].target_pos-Pos)).sq()<=coverage
                                && ((Vector3D)(weapons.weapon[i].Pos-Pos)).sq()<=range)
                            {
                                int idx=-1;
                                for (e = 0; e < mem_size; ++e)
                                {
                                    if (memory[e] == i)
                                    {
                                        idx=i;
                                        break;
                                    }
                                }
                                if (idx == -1)
                                {
                                    enemy_idx=i;
                                    if (mem_size < TA3D_PLAYERS_HARD_LIMIT)
                                    {
                                        memory[mem_size]=i;
                                        mem_size++;
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    if (enemy_idx>=0)			// If we found a target, then attack it, here  we use attack because we need the mission list to act properly
                        add_mission(MISSION_ATTACK | MISSION_FLAG_AUTO,&(weapons.weapon[enemy_idx].Pos),false,0,&(weapons.weapon[enemy_idx]),NULL,12);	// 12 = 4 | 8, targets a weapon and automatic fire
                }
            }
        }

        if (unit_manager.unit_type[type_id]->canfly ) // Set plane orientation
        {
            Vector3D J,K;
            K.x=K.z=0.0f;
            K.y=1.0f;
            J = V * K;

            Vector3D virtual_G;						// Compute the apparent gravity force ( seen from the plane )
            virtual_G.x = virtual_G.z = 0.0f;		// Standard gravity vector
            virtual_G.y = -4.0f * units.g_dt;
            float d = J.sq();
            if (d )
                virtual_G = virtual_G + (((old_V - V) % J) / d) * J;		// Add the opposite of the speed derivative projected on the side of the unit

            d = virtual_G.norm();
            if (d ) {
                virtual_G = -1.0f / d * virtual_G;

                d = sqrt(virtual_G.y*virtual_G.y+virtual_G.z*virtual_G.z);
                float angle_1 = (d != 0.0f) ? acos(virtual_G.y/d)*RAD2DEG : 0.0f;
                if (virtual_G.z<0.0f)	angle_1 = -angle_1;
                virtual_G = virtual_G * RotateX(-angle_1*DEG2RAD);
                float angle_2 = acos( virtual_G % K )*RAD2DEG;
                if (virtual_G.x > 0.0f)	angle_2 = -angle_2;

                if (fabs( angle_1 - Angle.x ) < 360.0f )
                    Angle.x += dt*( angle_1 - Angle.x );				// We need something continuous
                if (fabs( angle_2 - Angle.z ) < 360.0f )
                    Angle.z += dt*( angle_2 - Angle.z );

                if (Angle.x < -360.0f || Angle.x > 360.0f )		Angle.x = 0.0f;
                if (Angle.z < -360.0f || Angle.z > 360.0f )		Angle.z = 0.0f;
            }
        }

        if (build_percent_left==0.0f) {

            // Change the unit's angle the way we need it to be changed

            if (b_TargetAngle && !isNaN(f_TargetAngle) && unit_manager.unit_type[type_id]->BMcode ) {	// Don't remove the class check otherwise factories can spin
                while( !isNaN(f_TargetAngle) && fabs( f_TargetAngle - Angle.y ) > 180.0f ) {
                    if (f_TargetAngle < Angle.y )
                        Angle.y -= 360.0f;
                    else
                        Angle.y += 360.0f;
                }
                if (!isNaN(f_TargetAngle) && fabs( f_TargetAngle - Angle.y ) >= 1.0f ) {
                    float aspeed = unit_manager.unit_type[type_id]->TurnRate;
                    if (f_TargetAngle < Angle.y )
                        aspeed =- aspeed;
                    float a = f_TargetAngle - Angle.y;
                    V_Angle.y = aspeed;
                    float b = f_TargetAngle - (Angle.y + dt*V_Angle.y);
                    if (((a < 0.0f && b > 0.0f) || (a > 0.0f && b < 0.0f)) && !isNaN(f_TargetAngle) ) {
                        V_Angle.y = 0.0f;
                        Angle.y = f_TargetAngle;
                    }
                }
            }

            Angle = Angle + dt * V_Angle;
            Vector3D OPos = Pos;
            if (precomputed_position )
            {
                if (unit_manager.unit_type[type_id]->canmove && unit_manager.unit_type[type_id]->BMcode && !flying )
                    V.y-=units.g_dt;			// L'unité subit la force de gravitation
                Pos = NPos;
                Pos.y = OPos.y + V.y * dt;
                cur_px = n_px;
                cur_py = n_py;
            }
            else
            {
                if (unit_manager.unit_type[type_id]->canmove && unit_manager.unit_type[type_id]->BMcode )
                    V.y-=units.g_dt;			// L'unité subit la force de gravitation
                Pos = Pos+dt*V;			// Déplace l'unité
                cur_px = ((int)(Pos.x)+map->map_w_d+4)>>3;
                cur_py = ((int)(Pos.z)+map->map_h_d+4)>>3;
            }
            if (units.current_tick - ripple_timer >= 7 && Pos.y <= map->sealvl && Pos.y + model->top >= map->sealvl && (unit_manager.unit_type[type_id]->fastCategory & CATEGORY_NOTSUB)
                && cur_px >= 0 && cur_py >= 0 && cur_px < map->bloc_w_db && cur_py < map->bloc_h_db && !map->map_data[ cur_py ][ cur_px ].lava && map->water )
                {
                Vector3D Diff = OPos - Pos;
                Diff.y = 0.0f;
                if (Diff.sq() > 0.1f && lp_CONFIG->waves)
                {
                    ripple_timer = units.current_tick;
                    Vector3D ripple_pos = Pos;
                    ripple_pos.y = map->sealvl + 1.0f;
                    fx_manager.addRipple( ripple_pos, ( ((sint32)(Math::RandFromTable() % 201)) - 100 ) * 0.0001f );
                }
            }
        }
script_exec:
        if (map && !attached && ( (!jump_commands && unit_manager.unit_type[type_id]->canmove) || first_move ))
        {
            float min_h = map->get_unit_h(Pos.x,Pos.z);
            h = Pos.y - min_h;
            if (!unit_manager.unit_type[type_id]->Floater && !unit_manager.unit_type[type_id]->canfly && !unit_manager.unit_type[type_id]->canhover && h > 0.0f && unit_manager.unit_type[type_id]->WaterLine == 0.0f )
                Pos.y = min_h;
            else if (unit_manager.unit_type[type_id]->canhover && Pos.y < map->sealvl)
            {
                Pos.y = map->sealvl;
                if (V.y<0.0f)
                    V.y=0.0f;
            }
            else if (unit_manager.unit_type[type_id]->Floater)
            {
                Pos.y = map->sealvl+unit_manager.unit_type[type_id]->AltFromSeaLevel*H_DIV;
                V.y=0.0f;
            }
            else if (unit_manager.unit_type[type_id]->WaterLine)
            {
                Pos.y=map->sealvl-unit_manager.unit_type[type_id]->WaterLine*H_DIV;
                V.y=0.0f;
            }
            else if (!unit_manager.unit_type[type_id]->canfly && Pos.y > Math::Max( min_h, map->sealvl ) && unit_manager.unit_type[type_id]->BMcode)	// Prevent non flying units from "jumping"
            {
                Pos.y = Math::Max(min_h, map->sealvl);
                if (V.y<0.0f)
                    V.y=0.0f;
            }
            if (min_h>Pos.y) {
                Pos.y=min_h;
                if (V.y<0.0f)
                    V.y=0.0f;
            }
            if (unit_manager.unit_type[type_id]->canfly && build_percent_left==0.0f && local)
            {
                if (mission && ( (mission->flags & MISSION_FLAG_MOVE) || mission->mission == MISSION_BUILD || mission->mission == MISSION_BUILD_2 || mission->mission == MISSION_REPAIR
                                 || mission->mission == MISSION_ATTACK || mission->mission == MISSION_MOVE || mission->mission == MISSION_GET_REPAIRED || mission->mission == MISSION_PATROL
                                 || mission->mission == MISSION_RECLAIM || nb_attached > 0 || Pos.x < -map->map_w_d || Pos.x > map->map_w_d || Pos.z < -map->map_h_d || Pos.z > map->map_h_d )) {
                    if (!(mission->mission == MISSION_GET_REPAIRED && (mission->flags & MISSION_FLAG_BEING_REPAIRED) ) )
                    {
                        float ideal_h=Math::Max(min_h,map->sealvl)+unit_manager.unit_type[type_id]->CruiseAlt*H_DIV;
                        V.y=(ideal_h-Pos.y)*2.0f;
                    }
                    flying = true;
                }
                else
                {
                    if (can_be_there( cur_px, cur_py, units.map, type_id, owner_id, idx ))		// Check it can be there
                    {
                        float ideal_h = min_h;
                        V.y=(ideal_h-Pos.y)*1.5f;
                        flying = false;
                    }
                    else				// There is someone there, find an other place to land
                    {
                        flying = true;
                        if (mission == NULL
                        || (mission->mission != MISSION_STOP && mission->mission != MISSION_VTOL_STANDBY && mission->mission != MISSION_STANDBY)
                        || mission->data > 5)   // Wait for MISSION_STOP to check if we have some work to do
                        {                                                                               // This prevents planes from keeping looking for a place to land
                            Vector3D next_target = Pos;                                                 // instead of going back to work :/
                            float find_angle = (Math::RandFromTable() % 360) * DEG2RAD;
                            next_target.x += cos( find_angle ) * (32.0f + unit_manager.unit_type[type_id]->FootprintX * 8.0f);
                            next_target.z += sin( find_angle ) * (32.0f + unit_manager.unit_type[type_id]->FootprintZ * 8.0f);
                            add_mission( MISSION_MOVE | MISSION_FLAG_AUTO, &next_target, true );
                        }
                    }
                }
            }
            port[GROUND_HEIGHT] = (int)(Pos.y-min_h+0.5f);
        }
        port[HEALTH] = (int)hp*100 / unit_manager.unit_type[type_id]->MaxDamage;
        if (nb_running>0)
        {
            for(int i=0;i<nb_running;i++)
                run_script(dt,i,map);
            int e=0;
            for(int i=0;i+e<nb_running;)				// Efface les scripts qui se sont arrêtés
            {
                if ((*script_env)[i+e].running)
                {
                    (*script_env)[i] = (*script_env)[i+e];
                    i++;
                }
                else
                {
                    (*script_env)[i+e].destroy();
                    e++;
                }
            }
            nb_running-=e;
        }
        yardmap_timer--;
        if (hp > 0.0f && 
            ((o_px != cur_px || o_py != cur_py || first_move || (was_flying ^ flying) || ((port[YARD_OPEN] != 0.0f) ^ was_open) || yardmap_timer == 0) && build_percent_left <= 0.0f || !drawn))
        {
            first_move = build_percent_left > 0.0f;
            pMutex.unlock();
            draw_on_map();
            pMutex.lock();
            yardmap_timer = TICKS_PER_SEC + (Math::RandFromTable() & 15);
        }

        built=false;
        attacked=false;
        pMutex.unlock();
        return 0;
    }

    bool UNIT::hit(Vector3D P,Vector3D Dir,Vector3D* hit_vec, float length)
    {
        pMutex.lock();
        if (!(flags&1))
        {
            pMutex.unlock();
            return false;
        }
        if (model)
        {
            Vector3D c_dir=model->center+Pos-P;
            if (c_dir.norm()-length <=model->size2 )
            {
                float scale=unit_manager.unit_type[type_id]->Scale;
                //            MATRIX_4x4 M=RotateX(-Angle.x*DEG2RAD)*RotateZ(-Angle.z*DEG2RAD)*RotateY(-Angle.y*DEG2RAD)*Scale(1.0f/scale);
                MATRIX_4x4 M = RotateXZY(-Angle.x*DEG2RAD, -Angle.z*DEG2RAD, -Angle.y*DEG2RAD)*Scale(1.0f/scale);
                Vector3D RP=(P-Pos) * M;
                bool is_hit=model->hit(RP,Dir,&data,hit_vec,M);
                if (is_hit) {
                    //                *hit_vec=(*hit_vec)*(RotateY(Angle.y*DEG2RAD)*RotateZ(Angle.z*DEG2RAD)*RotateX(Angle.x*DEG2RAD)*Scale(scale))+Pos;
                    *hit_vec = ((*hit_vec) * RotateYZX(Angle.y*DEG2RAD, Angle.z*DEG2RAD, Angle.x*DEG2RAD))*Scale(scale)+Pos;
                    *hit_vec=((*hit_vec-P)%Dir)*Dir+P;
                }

                pMutex.unlock();
                return is_hit;
            }
        }
        pMutex.unlock();
        return false;
    }

    bool UNIT::hit_fast(Vector3D P,Vector3D Dir,Vector3D* hit_vec, float length)
    {
        pMutex.lock();
        if (!(flags&1))	{
            pMutex.unlock();
            return false;
        }
        if (model)
        {
            Vector3D c_dir = model->center+Pos-P;
            if (c_dir.sq() <= ( model->size2 + length ) * ( model->size2 + length ) ) {
                float scale=unit_manager.unit_type[type_id]->Scale;
                //            MATRIX_4x4 M = RotateX(-Angle.x*DEG2RAD)*RotateZ(-Angle.z*DEG2RAD)*RotateY(-Angle.y*DEG2RAD)*Scale(1.0f/scale);
                MATRIX_4x4 M = RotateXZY(-Angle.x*DEG2RAD, -Angle.z*DEG2RAD, -Angle.y*DEG2RAD)*Scale(1.0f/scale);
                Vector3D RP = (P - Pos) * M;
                bool is_hit = model->hit_fast(RP,Dir,&data,hit_vec,M);
                if (is_hit) {
                    //                *hit_vec=(*hit_vec)*(RotateY(Angle.y*DEG2RAD)*RotateZ(Angle.z*DEG2RAD)*RotateX(Angle.x*DEG2RAD)*Scale(scale))+Pos;
                    *hit_vec = ((*hit_vec)*RotateYZX(Angle.y*DEG2RAD, Angle.z*DEG2RAD, Angle.x*DEG2RAD))*Scale(scale)+Pos;
                    *hit_vec=((*hit_vec-P)%Dir)*Dir+P;
                }

                pMutex.unlock();
                return is_hit;
            }
        }
        pMutex.unlock();
        return false;
    }

    void UNIT::show_orders(bool only_build_commands, bool def_orders)				// Dessine les ordres reçus
    {
        if (!def_orders)
            show_orders( only_build_commands, true );

        pMutex.lock();

        bool low_def = (Camera::inGame->rpos.y > gfx->low_def_limit);

        MISSION *cur = def_orders ? def_mission : mission;
        if (low_def )
        {
            glEnable(GL_BLEND);
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            glDisable(GL_CULL_FACE);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            glColor4ub(0xFF,0xFF,0xFF,0xFF);
        }
        else
        {
            glEnable(GL_BLEND);
            glEnable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            glDisable(GL_CULL_FACE);
            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
            glColor4ub(0xFF,0xFF,0xFF,0xFF);
        }
        Vector3D p_target=Pos;
        Vector3D n_target=Pos;
        float rab=(msec_timer%1000)*0.001f;
        uint32	remaining_build_commands = !(unit_manager.unit_type[type_id]->BMcode) ? 0 : 0xFFFFFFF;

        std::list<Vector3D>	points;

        while(cur)
        {
            if (cur->step) 	// S'il s'agit d'une étape on ne la montre pas
            {
                cur=cur->next;
                continue;
            }
            if (!only_build_commands)
            {
                int curseur=anim_cursor(CURSOR_CROSS_LINK);
                float dx = 0.5f * cursor[CURSOR_CROSS_LINK].ofs_x[curseur];
                float dz = 0.5f * cursor[CURSOR_CROSS_LINK].ofs_y[curseur];
                float x,y,z;
                float dist = ((Vector3D)(cur->target-p_target)).norm();
                int rec = (int)(dist / 30.0f);
                switch (cur->mission)
                {
                    case MISSION_LOAD:
                    case MISSION_UNLOAD:
                    case MISSION_GUARD:
                    case MISSION_PATROL:
                    case MISSION_MOVE:
                    case MISSION_BUILD:
                    case MISSION_BUILD_2:
                    case MISSION_REPAIR:
                    case MISSION_ATTACK:
                    case MISSION_RECLAIM:
                    case MISSION_REVIVE:
                    case MISSION_CAPTURE:
                        if ((cur->p && ((UNIT*)(cur->p))->ID != cur->target_ID) || (cur->flags & MISSION_FLAG_TARGET_WEAPON) )
                        {
                            cur = cur->next;
                            continue;	// Don't show this, it'll be removed
                        }
                        n_target=cur->target;
                        n_target.y = Math::Max(units.map->get_unit_h( n_target.x, n_target.z ), units.map->sealvl);
                        if (rec > 0)
                        {
                            if (low_def)
                            {
                                glDisable(GL_DEPTH_TEST);
                                glColor4ub( 0xFF, 0xFF, 0xFF, 0x7F );
                                glBegin( GL_QUADS );
                                Vector3D D = n_target - p_target;
                                D.y = D.x;
                                D.x = D.z;
                                D.z = -D.y;
                                D.y = 0.0f;
                                D.unit();
                                D = 5.0f * D;
                                Vector3D P;
                                P = p_target - D;	glVertex3fv( (GLfloat*)&P );
                                P = p_target + D;	glVertex3fv( (GLfloat*)&P );
                                P = n_target + D;	glVertex3fv( (GLfloat*)&P );
                                P = n_target - D;	glVertex3fv( (GLfloat*)&P );
                                glEnd();
                                glColor4ub( 0xFF, 0xFF, 0xFF, 0xFF );
                                glEnable(GL_DEPTH_TEST);
                            }
                            else
                            {
                                for (int i = 0; i < rec; ++i)
                                {
                                    x = p_target.x+(n_target.x-p_target.x)*(i+rab)/rec;
                                    z = p_target.z+(n_target.z-p_target.z)*(i+rab)/rec;
                                    y = Math::Max(units.map->get_unit_h( x, z ), units.map->sealvl);
                                    y += 0.75f;
                                    x -= dx;
                                    z -= dz;
                                    points.push_back(Vector3D(x, y, z));
                                }
                            }
                        }
                        p_target = n_target;
                }
            }
            glDisable(GL_DEPTH_TEST);
            switch(cur->mission)
            {
                case MISSION_BUILD:
                    if (cur->p!=NULL)
                        cur->target=((UNIT*)(cur->p))->Pos;
                    if (cur->data>=0 && cur->data<unit_manager.nb_unit && remaining_build_commands > 0 )
                    {
                        remaining_build_commands--;
                        float DX = (unit_manager.unit_type[cur->data]->FootprintX<<2);
                        float DZ = (unit_manager.unit_type[cur->data]->FootprintZ<<2);
                        float blue = 0.0f, green = 1.0f;
                        if (only_build_commands)
                        {
                            blue = 1.0f;
                            green = 0.0f;
                        }
                        glPushMatrix();
                        glTranslatef(cur->target.x,Math::Max( cur->target.y, units.map->sealvl ),cur->target.z);
                        glDisable(GL_CULL_FACE);
                        glDisable(GL_TEXTURE_2D);
                        glEnable(GL_BLEND);
                        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                        glBegin(GL_QUADS);
                        glColor4f(0.0f,green,blue,1.0f);
                        glVertex3f(-DX,0.0f,-DZ);			// First quad
                        glVertex3f(DX,0.0f,-DZ);
                        glColor4f(0.0f,green,blue,0.0f);
                        glVertex3f(DX+2.0f,5.0f,-DZ-2.0f);
                        glVertex3f(-DX-2.0f,5.0f,-DZ-2.0f);

                        glColor4f(0.0f,green,blue,1.0f);
                        glVertex3f(-DX,0.0f,-DZ);			// Second quad
                        glVertex3f(-DX,0.0f,DZ);
                        glColor4f(0.0f,green,blue,0.0f);
                        glVertex3f(-DX-2.0f,5.0f,DZ+2.0f);
                        glVertex3f(-DX-2.0f,5.0f,-DZ-2.0f);

                        glColor4f(0.0f,green,blue,1.0f);
                        glVertex3f(DX,0.0f,-DZ);			// Third quad
                        glVertex3f(DX,0.0f,DZ);
                        glColor4f(0.0f,green,blue,0.0f);
                        glVertex3f(DX+2.0f,5.0f,DZ+2.0f);
                        glVertex3f(DX+2.0f,5.0f,-DZ-2.0f);

                        glEnd();
                        glDisable(GL_BLEND);
                        glEnable(GL_TEXTURE_2D);
                        glEnable(GL_CULL_FACE);
                        glPopMatrix();
                        if (unit_manager.unit_type[cur->data]->model!=NULL)
                        {
                            glEnable(GL_LIGHTING);
                            glEnable(GL_CULL_FACE);
                            glEnable(GL_DEPTH_TEST);
                            glPushMatrix();
                            glTranslatef(cur->target.x,cur->target.y,cur->target.z);
                            glColor4f(0.0f,green,blue,0.5f);
                            glDepthFunc( GL_GREATER );
                            unit_manager.unit_type[cur->data]->model->obj.draw(0.0f,NULL,false,false,false);
                            glDepthFunc( GL_LESS );
                            unit_manager.unit_type[cur->data]->model->obj.draw(0.0f,NULL,false,false,false);
                            glPopMatrix();
                            glEnable(GL_BLEND);
                            glEnable(GL_TEXTURE_2D);
                            glDisable(GL_LIGHTING);
                            glDisable(GL_CULL_FACE);
                            glDisable(GL_DEPTH_TEST);
                            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                        }
                        glPushMatrix();
                        glTranslatef(cur->target.x,Math::Max( cur->target.y, units.map->sealvl ),cur->target.z);
                        glDisable(GL_CULL_FACE);
                        glDisable(GL_TEXTURE_2D);
                        glEnable(GL_BLEND);
                        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                        glBegin(GL_QUADS);
                        glColor4f(0.0f,green,blue,1.0f);
                        glVertex3f(-DX,0.0f,DZ);			// Fourth quad
                        glVertex3f(DX,0.0f,DZ);
                        glColor4f(0.0f,green,blue,0.0f);
                        glVertex3f(DX+2.0f,5.0f,DZ+2.0f);
                        glVertex3f(-DX-2.0f,5.0f,DZ+2.0f);
                        glEnd();
                        glPopMatrix();
                        glEnable(GL_BLEND);
                        if (low_def )
                            glDisable(GL_TEXTURE_2D);
                        else
                            glEnable(GL_TEXTURE_2D);
                        glDisable(GL_CULL_FACE);
                        glColor4f(1.0f,1.0f,1.0f,1.0f);
                    }
                    break;
                case MISSION_UNLOAD:
                case MISSION_LOAD:
                case MISSION_MOVE:
                case MISSION_BUILD_2:
                case MISSION_REPAIR:
                case MISSION_RECLAIM:
                case MISSION_REVIVE:
                case MISSION_PATROL:
                case MISSION_GUARD:
                case MISSION_ATTACK:
                case MISSION_CAPTURE:
                    if (!only_build_commands)
                    {
                        if (cur->p!=NULL)
                            cur->target=((UNIT*)(cur->p))->Pos;
                        int cursor_type = CURSOR_ATTACK;
                        switch( cur->mission )
                        {
                            case MISSION_GUARD:		cursor_type = CURSOR_GUARD;		break;
                            case MISSION_ATTACK:	cursor_type = CURSOR_ATTACK;	break;
                            case MISSION_PATROL:	cursor_type = CURSOR_PATROL;	break;
                            case MISSION_RECLAIM:	cursor_type = CURSOR_RECLAIM;	break;
                            case MISSION_BUILD_2:
                            case MISSION_REPAIR:	cursor_type = CURSOR_REPAIR;	break;
                            case MISSION_MOVE:		cursor_type = CURSOR_MOVE;		break;
                            case MISSION_LOAD:		cursor_type = CURSOR_LOAD;		break;
                            case MISSION_UNLOAD:	cursor_type = CURSOR_UNLOAD;	break;
                            case MISSION_REVIVE:	cursor_type = CURSOR_REVIVE;	break;
                            case MISSION_CAPTURE:	cursor_type = CURSOR_CAPTURE;	break;
                        }
                        int curseur=anim_cursor( cursor_type );
                        float x=cur->target.x - 0.5f * cursor[cursor_type].ofs_x[curseur];
                        float y=cur->target.y + 1.0f;
                        float z=cur->target.z - 0.5f * cursor[cursor_type].ofs_y[curseur];
                        float sx = 0.5f * (cursor[cursor_type].bmp[curseur]->w - 1);
                        float sy = 0.5f * (cursor[cursor_type].bmp[curseur]->h - 1);
                        if (low_def)
                            glEnable(GL_TEXTURE_2D);
                        glBindTexture(GL_TEXTURE_2D, cursor[cursor_type].glbmp[curseur]);
                        glBegin(GL_QUADS);
                        glTexCoord2f(0.0f,0.0f);  glVertex3f(x,y,z);
                        glTexCoord2f(1.0f,0.0f);  glVertex3f(x+sx,y,z);
                        glTexCoord2f(1.0f,1.0f);  glVertex3f(x+sx,y,z+sy);
                        glTexCoord2f(0.0f,1.0f);  glVertex3f(x,y,z+sy);
                        glEnd();
                        if (low_def)
                            glDisable(GL_TEXTURE_2D);
                    }
                    break;
            }
            glEnable(GL_DEPTH_TEST);
            cur = cur->next;
        }

        if (!points.empty())
        {
            int curseur=anim_cursor(CURSOR_CROSS_LINK);
            float sx = 0.5f * (cursor[CURSOR_CROSS_LINK].bmp[curseur]->w - 1);
            float sy = 0.5f * (cursor[CURSOR_CROSS_LINK].bmp[curseur]->h - 1);

            Vector3D* P = new Vector3D[points.size() << 2];
            float* T = new float[points.size() << 3];

            int n = 0;
            for (std::list<Vector3D>::const_iterator i = points.begin(); i != points.end(); ++i)
            {
                P[n] = *i;
                T[n<<1] = 0.0f;		T[(n<<1)+1] = 0.0f;
                ++n;

                P[n] = *i;	P[n].x += sx;
                T[n<<1] = 1.0f;		T[(n<<1)+1] = 0.0f;
                ++n;

                P[n] = *i;	P[n].x += sx;	P[n].z += sy;
                T[n<<1] = 1.0f;		T[(n<<1)+1] = 1.0f;
                ++n;

                P[n] = *i;	P[n].z += sy;
                T[n<<1] = 0.0f;		T[(n<<1)+1] = 1.0f;
                ++n;
            }

            glDisableClientState( GL_NORMAL_ARRAY );
            glDisableClientState( GL_COLOR_ARRAY );
            glEnableClientState( GL_VERTEX_ARRAY );
            glEnableClientState( GL_TEXTURE_COORD_ARRAY );

            glVertexPointer( 3, GL_FLOAT, 0, P);
            glClientActiveTextureARB(GL_TEXTURE0_ARB );
            glTexCoordPointer(2, GL_FLOAT, 0, T);
            glBindTexture(GL_TEXTURE_2D, cursor[CURSOR_CROSS_LINK].glbmp[curseur]);

            glDrawArrays(GL_QUADS, 0, n);

            delete[] P;
            delete[] T;
        }
        glDisable(GL_BLEND);
        pMutex.unlock();
    }






    void INGAME_UNITS::set_wind_change()
    {
        pMutex.lock();
        wind_change = true;
        pMutex.unlock();
    }


    INGAME_UNITS::INGAME_UNITS()
        :repair_pads(), requests()
    {
        InitThread();
        init();
    }


    void INGAME_UNITS::destroy(bool delete_interface)
    {
        pMutex.lock();

        if (delete_interface)
        {
            for (byte i = 0; i < 13; ++i)
                gfx->destroy_texture(icons[i]);

            DeleteInterface();			// Shut down the interface
        }

        if ( mini_pos )			delete[] mini_pos;
        if ( mini_col )			delete[] mini_col;

        if (idx_list)			delete[] idx_list;
        if (free_idx)			delete[] free_idx;
        if (max_unit>0 && unit)			// Destroy all units
            for(int i = 0; i < max_unit; ++i)
                unit[i].destroy(true);
        if (unit)
            delete[] unit;
        pMutex.unlock();
        init();
    }


    void INGAME_UNITS::give_order_move(int player_id, const Vector3D& target,bool set,byte flags)
    {
        pMutex.lock();

        for (uint16 e = 0; e < index_list_size; ++e)
        {
            uint16 i = idx_list[e];
            if ((unit[i].flags & 1) && unit[i].owner_id==player_id && unit[i].sel && unit[i].build_percent_left == 0.0f && unit_manager.unit_type[unit[i].type_id]->canmove)
            {
                if (set)
                    unit[i].set_mission(MISSION_MOVE, &target, false, 0, true, NULL, NULL, flags);
                else
                    unit[i].add_mission(MISSION_MOVE, &target, false, 0, NULL, NULL, flags);
                if (unit_manager.unit_type[unit[i].type_id]->BMcode && set)
                    unit[i].play_sound("ok1");
            }
        }
        pMutex.unlock();
    }


    void INGAME_UNITS::give_order_patrol(int player_id, const Vector3D& target, bool set)
    {
        pMutex.lock();
        for (uint16 e = 0; e < index_list_size; ++e)
        {
            uint16 i = idx_list[e];
            if ((unit[i].flags & 1) && unit[i].owner_id==player_id && unit[i].sel && unit[i].build_percent_left ==0.0f && unit_manager.unit_type[unit[i].type_id]->canpatrol)
            {
                if (set)
                    unit[i].set_mission(MISSION_PATROL, &target, false, 0, true, NULL, NULL);
                else
                    unit[i].add_mission(MISSION_PATROL, &target, false, 0, NULL, NULL);
                if (unit_manager.unit_type[unit[i].type_id]->BMcode && set)
                    unit[i].play_sound("ok1");
            }
        }
        pMutex.unlock();
    }


    void INGAME_UNITS::give_order_guard(int player_id,int target,bool set)
    {
        pMutex.lock();
        for (uint16 e = 0; e < index_list_size; ++e)
        {
            uint16 i = idx_list[e];
            if ((unit[i].flags & 1) && unit[i].owner_id==player_id && unit[i].sel && unit[i].build_percent_left ==0.0f && unit_manager.unit_type[unit[i].type_id]->canguard)
            {
                if (set)
                    unit[i].set_mission(MISSION_GUARD,&unit[target].Pos,false,0,true,&(unit[target]),NULL);
                else
                    unit[i].add_mission(MISSION_GUARD,&unit[target].Pos,false,0,&(unit[target]),NULL);
                if (unit_manager.unit_type[unit[i].type_id]->BMcode && set )
                    unit[i].play_sound( "ok1" );
            }
        }
        pMutex.unlock();
    }


    void INGAME_UNITS::give_order_unload(int player_id, const Vector3D& target, bool set)
    {
        pMutex.lock();
        for (uint16 e = 0; e < index_list_size; ++e)
        {
            uint16 i = idx_list[e];
            if ((unit[i].flags & 1) && unit[i].owner_id==player_id && unit[i].sel && unit[i].build_percent_left == 0.0f && unit_manager.unit_type[unit[i].type_id]->canload
                && unit_manager.unit_type[unit[i].type_id]->BMcode && unit[i].nb_attached > 0 )
            {
                if (set)
                {
                    unit[i].set_mission(MISSION_UNLOAD, &target, false, 0, true, NULL, NULL);
                    unit[i].play_sound("ok1");
                }
                else
                    unit[i].add_mission(MISSION_UNLOAD, &target, false, 0, NULL, NULL);
            }
        }
        pMutex.unlock();
    }



    void INGAME_UNITS::give_order_load(int player_id, int target, bool set)
    {
        pMutex.lock();
        if (unit[target].flags == 0 || !unit_manager.unit_type[unit[target].type_id]->canmove)
        {
            pMutex.unlock();
            return;	
        }
        switch(unit_manager.unit_type[unit[target].type_id]->TEDclass)
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
        }
        for (uint16 e = 0; e < index_list_size; ++e)
        {
            uint16 i = idx_list[e];
            if ((unit[i].flags & 1) && unit[i].owner_id==player_id && unit[i].sel && unit[i].build_percent_left == 0.0f && unit_manager.unit_type[unit[i].type_id]->canload
                && unit_manager.unit_type[unit[i].type_id]->BMcode)
            {
                if (set)
                {
                    unit[i].set_mission(MISSION_LOAD, &unit[target].Pos, false, 0, true, &(unit[target]), NULL);
                    unit[i].play_sound("ok1");
                }
                else
                    unit[i].add_mission(MISSION_LOAD, &unit[target].Pos, false, 0, &(unit[target]), NULL);
            }
        }
        pMutex.unlock();
    }



    void INGAME_UNITS::give_order_build(int player_id, int unit_type_id, const Vector3D& target, bool set)
    {
        if (unit_type_id < 0)
            return;

        Vector3D t(target);
        t.x = ((int)(t.x) + map->map_w_d) >> 3;
        t.z = ((int)(t.z) + map->map_h_d) >> 3;
        t.y = map->get_max_rect_h((int)t.x, (int)t.z, unit_manager.unit_type[unit_type_id]->FootprintX,
                                            unit_manager.unit_type[unit_type_id]->FootprintZ);
        if (unit_manager.unit_type[unit_type_id]->floatting())
            t.y = Math::Max(t.y,map->sealvl+(unit_manager.unit_type[unit_type_id]->AltFromSeaLevel-unit_manager.unit_type[unit_type_id]->WaterLine)*H_DIV);
        t.x = t.x * 8.0f - map->map_w_d;
        t.z = t.z * 8.0f - map->map_h_d;

        pMutex.lock();
        for( uint16 e = 0; e < index_list_size; ++e)
        {
            uint16 i = idx_list[e];
            if ((unit[i].flags & 1) && unit[i].owner_id==player_id && unit[i].sel && unit[i].build_percent_left == 0.0f && unit_manager.unit_type[unit[i].type_id]->Builder)
            {
                if (set)
                    unit[i].set_mission(MISSION_BUILD, &t, false, unit_type_id);
                else
                    unit[i].add_mission(MISSION_BUILD, &t, false, unit_type_id);
            }
        }
        pMutex.unlock();
    }


    void INGAME_UNITS::init(bool register_interface)
    {
        pMutex.lock();

        next_unit_ID = 1;
        mini_pos = NULL;
        mini_col = NULL;
        requests.clear();
        repair_pads.clear();
        repair_pads.resize(TA3D_PLAYERS_HARD_LIMIT);

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

        if (register_interface)
        {
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
        for (short int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
            free_index_size[i] = 0;
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


    bool INGAME_UNITS::selectUnits(Camera& cam, const Rect<int>& pos)
    {
        pMutex.lock();

        cam.setView();
        MATRIX_4x4 modelView;
        MATRIX_4x4 project;

        int	viewportCoords[4] = {0, 0, 0, 0};
        glGetIntegerv(GL_VIEWPORT, viewportCoords);
        glGetFloatv(GL_MODELVIEW_MATRIX,  (float*)modelView.E);
        glGetFloatv(GL_PROJECTION_MATRIX, (float*)project.E);

        modelView = Transpose(modelView);
        project = Transpose(project);

        float VW =  (viewportCoords[2] - viewportCoords[0]) * 0.5f;
        float VH = -(viewportCoords[3] - viewportCoords[1]) * 0.5f;

        MATRIX_4x4 T(modelView * project); // Matrice de transformation

        int X1 = Math::Min(pos.x1, pos.x2);
        int Y1 = Math::Min(pos.y1, pos.y2);
        int X2 = Math::Max(pos.x1, pos.x2);
        int Y2 = Math::Max(pos.y1, pos.y2);

        bool selected = false;

        for (uint16 e = 0; e < index_list_size; ++e) 
        {
            uint16 i = idx_list[e];
            pMutex.unlock();
            unit[i].lock();

            // Select only units completely built and visible
            if (unit[i].owner_id == players.local_human_id && (unit[i].flags & 1) && unit[i].build_percent_left == 0.0f
                && unit[i].visible)	
            {
                if (TA3D_SHIFT_PRESSED && unit[i].sel)
                {
                    selected = true;
                }
                else
                {
                    if (!TA3D_SHIFT_PRESSED)
                        unit[i].sel = false;

                    Vector3D Vec(unit[i].Pos - cam.pos);
                    float d = Vec.sq();
                    if (!(d > 16384.0f && (Vec % cam.dir) <= 0.0f))
                    {
                        Vector3D UPos (glNMult(unit[i].Pos, T)); // The unit position
                        UPos.x = UPos.x * VW + VW;
                        UPos.y = UPos.y * VH - VH;
                        if (X1 <= UPos.x && X2 >= UPos.x && Y1 <= UPos.y && Y2 >= UPos.y)
                        {
                            unit[i].sel = true;
                            selected = true;
                        }
                    }
                }
            }
            unit[i].unlock();
            pMutex.lock();
        }
        pMutex.unlock();
        return selected;
    }


    int INGAME_UNITS::pick(Camera& cam, int sensibility)
    {
        int index = -1;

        if (nb_unit<=0)
            return -1;

        // Things didn't change :-) seen from the mouse cursor since the screen wasn't refreshed
        if (last_on != -1)
            return last_on;		

        Vector3D Dir;
        Dir = cam.dir + cam.widthFactor * 2.0f * (mouse_x-gfx->SCREEN_W_HALF) * gfx->SCREEN_W_INV
            * cam.side-1.5f * (mouse_y-gfx->SCREEN_H_HALF)
            * gfx->SCREEN_H_INV * cam.up;
        Dir.unit();		// Direction pointée par le curseur

        bool detectable=false;
        int i;

        pMutex.lock();
        for(uint16 e = 0; e < index_list_size; ++e)
        {
            i = idx_list[e];
            pMutex.unlock();

            unit[ i ].lock();
            if (!(unit[i].flags & 1) || !unit[i].visible )
            {
                unit[i].unlock();
                pMutex.lock();
                continue;		// Si l'unité n'existe pas on la zappe
            }
            unit[i].flags &= 0xFD;	// Enlève l'indicateur de possibilité d'intersection
            Vector3D center (unit[i].model->center + unit[i].Pos - cam.pos);
            float size = unit[i].model->size * unit_manager.unit_type[unit[i].type_id]->Scale * unit_manager.unit_type[unit[i].type_id]->Scale;
            center = Dir * center;
            float dist=center.sq();
            if (dist<size)
            {
                detectable=true;
                unit[i].flags|=0x2;		// Unité détectable
            }
            unit[i].unlock();
            pMutex.lock();
        }
        pMutex.unlock();

        if (!detectable) // If no unit is near the cursor, then skip the precise method
        {
            last_on = index;
            return index;
        }

        float best_dist = 1000000.0f;

        pMutex.lock();
        for(uint16 e=0;e<index_list_size;e++)
        {
            i = idx_list[e];
            pMutex.unlock();

            unit[i].lock();
            if (!(unit[i].flags & 1) || !unit[i].visible )
            {
                unit[i].unlock();
                pMutex.lock();
                continue;		// Si l'unité n'existe pas on la zappe
            }
            if ((unit[i].flags&0x2)==0x2) // Si l'unité existe et est sélectionnable
            {
                unit[i].flags&=0xFD;
                Vector3D D;
                if (unit[i].hit(cam.pos, Dir, &D, 1000000.0f)) // Vecteur "viseur unité" partant de la caméra vers l'unité
                {
                    float dist = (D - cam.pos).sq();
                    if (dist < best_dist || index == -1 )
                    {
                        best_dist = dist;
                        index = i;
                    }
                }
            }
            unit[i].unlock();
            pMutex.lock();
        }
        pMutex.unlock();

        last_on = index;
        return index;
    }

    int INGAME_UNITS::pick_minimap()
    {
        int index=-1;

        if (nb_unit<=0)
            return index;

        // Things didn't change :-) seen from the mouse cursor since the screen wasn't refreshed
        if (last_on != -1 )
            return last_on;		

        int i;

        float conv_x = ((float)map->mini_w) / map->map_w * 128.0f / 252.0f;
        float conv_z = ((float)map->mini_h) / map->map_h * 128.0f / 252.0f;

        byte player_mask = 1 << players.local_human_id;

        pMutex.lock();
        for(uint16 e=0;e<index_list_size;e++)
        {
            i = idx_list[e];
            pMutex.unlock();

            unit[ i ].lock();
            if (!(unit[i].flags & 1) )
            {
                unit[ i ].unlock();
                pMutex.lock();
                continue;		// Si l'unité n'existe pas on la zappe
            }

            if (!unit[i].visible ) // Additional checks that have to be done
            {
                int px = unit[i].cur_px >> 1;
                int py = unit[i].cur_py >> 1;
                if (px < 0 || py < 0 || px >= map->bloc_w || py >= map->bloc_h )
                {
                    unit[ i ].unlock();
                    pMutex.lock();
                    continue;	// Out of the map
                }
                if (!( map->view_map->line[ py ][ px ] & player_mask ) && !(map->sight_map->line[ py ][ px ] & player_mask)
                    && !unit[i].is_on_radar( player_mask ) )
                {
                    unit[ i ].unlock();
                    pMutex.lock();
                    continue;	// Not visible
                }
            }

            int x = (int)(unit[i].Pos.x * conv_x + 64.5f);
            int y = (int)(unit[i].Pos.z * conv_z + 64.5f);

            if (x == mouse_x && y == mouse_y )
            {
                last_on = i;
                unit[ i ].unlock();
                return i;
            }

            if (abs(mouse_x - x) <= 1 && abs(mouse_y - y) <= 1 )
                index = i;

            unit[i].unlock();
            pMutex.lock();
        }
        pMutex.unlock();

        last_on = index;
        return index;
    }

    int UNIT::shoot(int target,Vector3D startpos,Vector3D Dir,int w_id,const Vector3D &target_pos)
    {
        WEAPON_DEF *pW = unit_manager.unit_type[type_id]->weapon[ w_id ];        // Critical information, we can't lose it so we save it before unlocking this unit
        int owner = owner_id;
        if (get_script_index( SCRIPT_RockUnit ) >= 0 ) // Don't do calculations that won't be used
        {
            Vector3D D = Dir * RotateY( -Angle.y * DEG2RAD );
            int param[] = { (int)(-10.0f*DEG2TA*D.z), (int)(-10.0f*DEG2TA*D.x) };
            launch_script( get_script_index( SCRIPT_RockUnit ), 2, param );
        }

        if (pW->startsmoke && visible)
            particle_engine.make_smoke(startpos,0,1,0.0f,-1.0f,0.0f, 0.3f);

        pMutex.unlock();

        weapons.lock();

        int w_idx = weapons.add_weapon(pW->nb_id,idx);

        if (network_manager.isConnected() && local) // Send synchronization packet
        {
            struct event event;
            event.type = EVENT_WEAPON_CREATION;
            event.opt1 = idx;
            event.opt2 = target;
            event.opt3 = units.current_tick; // Will be used to extrapolate those data on client side
            event.opt4 = pW->damage;
            event.opt5 = owner_id;
            event.x = target_pos.x;
            event.y = target_pos.y;
            event.z = target_pos.z;
            event.vx = startpos.x;
            event.vy = startpos.y;
            event.vz = startpos.z;
            event.dx = (sint16)(Dir.x * 16384.0f);
            event.dy = (sint16)(Dir.y * 16384.0f);
            event.dz = (sint16)(Dir.z * 16384.0f);
            memcpy( event.str, pW->internal_name.c_str(), pW->internal_name.size() + 1 );

            network_manager.sendEvent( &event );
        }

        weapons.weapon[w_idx].damage = pW->damage;
        weapons.weapon[w_idx].Pos = startpos;
        weapons.weapon[w_idx].local = local;
        if (pW->startvelocity==0.0f && !pW->selfprop)
            weapons.weapon[w_idx].V = pW->weaponvelocity*Dir;
        else
            weapons.weapon[w_idx].V = pW->startvelocity*Dir;
//        if (pW->dropped || !pW->lineofsight)
            weapons.weapon[w_idx].V = weapons.weapon[w_idx].V+V;
        weapons.weapon[w_idx].owner = owner;
        weapons.weapon[w_idx].target=target;
        if (target >= 0 )
        {
            if (pW->interceptor)
                weapons.weapon[w_idx].target_pos = weapons.weapon[target].Pos;
            else
                weapons.weapon[w_idx].target_pos = target_pos;
        }
        else
            weapons.weapon[w_idx].target_pos = target_pos;

        weapons.weapon[w_idx].stime = 0.0f;
        weapons.weapon[w_idx].visible = visible;        // Not critical so we don't duplicate this
        weapons.unlock();
        pMutex.lock();
        return w_idx;
    }


    void UNIT::draw_on_map()
    {
        if (type_id == -1 || !(flags & 1) )
            return;

        if (drawn )	clear_from_map();
        if (attached )	return;

        drawn_flying = flying;
        if (flying )
            units.map->air_rect( cur_px-(unit_manager.unit_type[type_id]->FootprintX>>1), cur_py-(unit_manager.unit_type[type_id]->FootprintZ>>1), unit_manager.unit_type[type_id]->FootprintX, unit_manager.unit_type[type_id]->FootprintZ, idx );
        else
        {
            // First check we're on a "legal" place if it can move
            pMutex.lock();
            if (unit_manager.unit_type[type_id]->canmove && unit_manager.unit_type[type_id]->BMcode
                && !can_be_there( cur_px, cur_py, units.map, type_id, owner_id ) )
            {
                // Try to find a suitable place

                bool found = false;
                for( int r = 1 ; r < 20 && !found ; r++ ) // Circular check
                {
                    int r2 = r * r;
                    for( int y = 0 ; y <= r ; y++ )
                    {
                        int x = (int)(sqrt( r2 - y * y ) + 0.5f);
                        if (can_be_there( cur_px+x, cur_py+y, units.map, type_id, owner_id ) )
                        {
                            cur_px += x;
                            cur_py += y;
                            found = true;
                            break;
                        }
                        if (can_be_there( cur_px+x, cur_py+y, units.map, type_id, owner_id ) )
                        {
                            cur_px -= x;
                            cur_py += y;
                            found = true;
                            break;
                        }
                        if (can_be_there( cur_px+x, cur_py+y, units.map, type_id, owner_id ) )
                        {
                            cur_px += x;
                            cur_py -= y;
                            found = true;
                            break;
                        }
                        if (can_be_there( cur_px+x, cur_py+y, units.map, type_id, owner_id ) )
                        {
                            cur_px -= x;
                            cur_py -= y;
                            found = true;
                            break;
                        }
                    }
                }
                if (found)
                {
                    Pos.x = (cur_px<<3) + 4 - units.map->map_w_d;
                    Pos.z = (cur_py<<3) + 4 - units.map->map_h_d;
                    if (mission && (mission->flags & MISSION_FLAG_MOVE) )
                        mission->flags |= MISSION_FLAG_REFRESH_PATH;
                }
                else
                    printf("error: units overlaps on yardmap !!\n");

            }
            pMutex.unlock();

            units.map->rect( cur_px-(unit_manager.unit_type[type_id]->FootprintX>>1), cur_py-(unit_manager.unit_type[type_id]->FootprintZ>>1), unit_manager.unit_type[type_id]->FootprintX, unit_manager.unit_type[type_id]->FootprintZ, idx, unit_manager.unit_type[type_id]->yardmap, port[YARD_OPEN]!=0.0f );
            drawn_open = port[YARD_OPEN]!=0.0f;
        }
        drawn_x = cur_px;
        drawn_y = cur_py;
        drawn = true;
    }

    void UNIT::clear_from_map()
    {
        if (!drawn)
            return;

        int type = type_id;

        if (type == -1 || !(flags & 1) )
            return;

        drawn = false;
        if (drawn_flying )
            units.map->air_rect( drawn_x-(unit_manager.unit_type[type]->FootprintX>>1), drawn_y-(unit_manager.unit_type[type]->FootprintZ>>1), unit_manager.unit_type[type]->FootprintX, unit_manager.unit_type[type]->FootprintZ, idx, true );
        else
            units.map->rect( drawn_x-(unit_manager.unit_type[type]->FootprintX>>1), drawn_y-(unit_manager.unit_type[type]->FootprintZ>>1), unit_manager.unit_type[type]->FootprintX, unit_manager.unit_type[type]->FootprintZ, -1, unit_manager.unit_type[type]->yardmap, drawn_open );
    }

    void UNIT::draw_on_FOW( bool jamming )
    {
        if (hidden || build_percent_left != 0.0f )
            return;

        bool system_activated = (port[ACTIVATION] && unit_manager.unit_type[type_id]->onoffable) || !unit_manager.unit_type[type_id]->onoffable;

        if (jamming )
        {
            radar_jam_range = system_activated ? (unit_manager.unit_type[type_id]->RadarDistanceJam >> 3) : 0;
            sonar_jam_range = system_activated ? (unit_manager.unit_type[type_id]->SonarDistanceJam >> 3) : 0;

            units.map->update_player_visibility( owner_id, cur_px, cur_py, 0, 0, 0, radar_jam_range, sonar_jam_range, true );
        }
        else
        {
            sint16 cur_sight = ((int)h + unit_manager.unit_type[type_id]->SightDistance) >> 3;
            radar_range = system_activated ? (unit_manager.unit_type[type_id]->RadarDistance >> 3) : 0;
            sonar_range = system_activated ? (unit_manager.unit_type[type_id]->SonarDistance >> 3) : 0;

            units.map->update_player_visibility( owner_id, cur_px, cur_py, cur_sight, radar_range, sonar_range, 0, 0, false, old_px != cur_px || old_py != cur_py || cur_sight != sight );

            sight = cur_sight;
            old_px = cur_px;
            old_py = cur_py;
        }
    }

    const void UNIT::play_sound( const String &key )
    {
        pMutex.lock();
        if (owner_id == players.local_human_id && msec_timer - last_time_sound >= units.sound_min_ticks )
        {
            last_time_sound = msec_timer;
            sound_manager->playTDFSound(unit_manager.unit_type[type_id]->soundcategory, key , &Pos);
        }
        pMutex.unlock();
    }

    int UNIT::launch_script(int id,int nb_param,int *param,bool force)			// Start a script as a separate "thread" of the unit
    {
        MutexLocker locker(pMutex);

        if (!script || id < 0 || id >= script->nb_script)
            return -2;
        if (!force)
        {
            if (is_running(id))			// le script tourne déjà / script already running
                return -1;
        }
        if (nb_running >= 25)	// Too much scripts running
        {
            LOG_WARNING("Too much script running");
            return -3;
        }

        if (local && network_manager.isConnected() ) // Send synchronization event
        {
            struct event event;
            event.type = EVENT_UNIT_SCRIPT;
            event.opt1 = idx;
            event.opt2 = force;
            event.opt3 = id;
            event.opt4 = nb_param;
            memcpy( event.str, param, sizeof(int) * nb_param );
            network_manager.sendEvent( &event );
        }

        if (script_env->size() <= nb_running )
            script_env->resize( nb_running + 1);
        (*script_env)[nb_running].init();
        (*script_env)[nb_running].env = new SCRIPT_ENV_STACK();
        (*script_env)[nb_running].env->init();
        (*script_env)[nb_running].env->cur=id;
        (*script_env)[nb_running].running=true;
        if (nb_param>0 && param!=NULL)
        {
            for(int i=0;i<nb_param;i++)
                (*script_env)[nb_running].env->var[i]=param[i];
        }
        return nb_running++;
    }

    void *create_unit( int type_id, int owner, Vector3D pos, MAP *map, bool sync, bool script )
    {
        int id = units.create(type_id,owner);
        if (id>=0)
        {
            units.unit[id].lock();

            if (network_manager.isConnected() )
            {
                units.unit[id].local = g_ta3d_network->isLocal( owner );
                if (sync) // Send event packet if needed
                {
                    struct event event;
                    event.type = EVENT_UNIT_CREATION;
                    event.opt1 = id;
                    event.opt2 = script ? (owner | 0x1000) : owner;
                    event.x = pos.x;
                    event.z = pos.z;
                    memcpy( event.str, unit_manager.unit_type[type_id]->Unitname.c_str(), unit_manager.unit_type[type_id]->Unitname.size() + 1 );
                    network_manager.sendEvent( &event );
                }
            }

            units.unit[id].Pos=pos;
            units.unit[id].build_percent_left=100.0f;
            units.unit[id].cur_px = ((int)(units.unit[id].Pos.x)+map->map_w_d+4)>>3;
            units.unit[id].cur_py = ((int)(units.unit[id].Pos.z)+map->map_h_d+4)>>3;
            units.unit[id].unlock();

            units.unit[id].draw_on_map();

            return &(units.unit[id]);
        }
        return NULL;
    }


    bool can_be_there_ai(const int px, const int py, MAP *map, const int unit_type_id,
                         const int player_id, const int unit_id )
    {
        if (unit_type_id<0 || unit_type_id>=unit_manager.nb_unit || !map)
            return false;

        int w = unit_manager.unit_type[unit_type_id]->FootprintX;
        int h = unit_manager.unit_type[unit_type_id]->FootprintZ;
        int x = px-(w>>1);
        int y = py-(h>>1);
        int side = unit_manager.unit_type[unit_type_id]->ExtractsMetal == 0.0f ? 12 : 0;
        if (x < 0 || y < ((int)map->get_zdec(x,0)+7>>3) || x+w>=(map->bloc_w<<1) || y+h>=(map->bloc_h<<1))	return false;	// check if it is inside the map

        if (!map->check_rect( px - ((w + side)>>1), py - ((h + side)>>1), w + side, h + side, unit_id))
            return false;		// There is already something
        float dh = fabs(map->check_rect_dh(x,y,w,h));
        float max_depth = map->check_max_depth(x,y,w,h);
        float min_depth = map->check_min_depth(x,y,w,h);

        if (dh>unit_manager.unit_type[unit_type_id]->MaxSlope*H_DIV
            && !( unit_manager.unit_type[unit_type_id]->canhover && min_depth <= map->sealvl ) )
            return false;	// Check the slope, check if hovering too

        // Check if unit can be there
        if (min_depth<unit_manager.unit_type[unit_type_id]->MinWaterDepth*H_DIV
            || (!unit_manager.unit_type[unit_type_id]->canhover && max_depth>unit_manager.unit_type[unit_type_id]->MaxWaterDepth*H_DIV))
            return false;

        if (!map->check_vents(x,y,w,h,unit_manager.unit_type[unit_type_id]->yardmap))
            return false;

        if (map->check_lava((x+1)>>1,(y+1)>>1,(w+1)>>1,(h+1)>>1))
            return false;

        return true;
    }

    bool can_be_there( const int px, const int py, MAP *map, const int unit_type_id,
                       const int player_id, const int unit_id )
    {
        if (unit_type_id<0 || unit_type_id>=unit_manager.nb_unit || !map)
            return false;

        int w = unit_manager.unit_type[unit_type_id]->FootprintX;
        int h = unit_manager.unit_type[unit_type_id]->FootprintZ;
        int x = px-(w>>1);
        int y = py-(h>>1);
        if (x < 0 || y < ((int)map->get_zdec(x,0)+7>>3) || x+w>=(map->bloc_w<<1) || y+h>=(map->bloc_h<<1))
            return false;	// check if it is inside the map

        if (!map->check_rect(x,y,w,h,unit_id))
            return false;		// There is already something

        float dh = fabs(map->check_rect_dh(x,y,w,h));
        float max_depth = map->check_max_depth(x,y,w,h);
        float min_depth = map->check_min_depth(x,y,w,h);

        if (dh>unit_manager.unit_type[unit_type_id]->MaxSlope*H_DIV
            && !( unit_manager.unit_type[unit_type_id]->canhover && min_depth <= map->sealvl ) )
            return false;	// Check the slope, check if hovering too

        // Check if unit can be there
        if (min_depth<unit_manager.unit_type[unit_type_id]->MinWaterDepth*H_DIV
            || (!unit_manager.unit_type[unit_type_id]->canhover && max_depth>unit_manager.unit_type[unit_type_id]->MaxWaterDepth*H_DIV))
            return false;

        if (!map->check_vents(x,y,w,h,unit_manager.unit_type[unit_type_id]->yardmap))
            return false;

        if (map->check_lava((x+1)>>1,(y+1)>>1,(w+1)>>1,(h+1)>>1))
            return false;

        return true;
    }

    bool can_be_built(const Vector3D& Pos, MAP *map,const int unit_type_id, const int player_id )
    {
        if (unit_type_id<0 || unit_type_id>=unit_manager.nb_unit || !map)
            return false;

        int w = unit_manager.unit_type[unit_type_id]->FootprintX;
        int h = unit_manager.unit_type[unit_type_id]->FootprintZ;
        int x = (((int)(Pos.x)+map->map_w_d+4)>>3)-(w>>1);
        int y = (((int)(Pos.z)+map->map_h_d+4)>>3)-(h>>1);
        if (x < 0 || y < ((int)map->get_zdec(x,0)+7>>3) || x+w>=(map->bloc_w<<1) || y+h>=(map->bloc_h<<1))
            return false;	// check if it is inside the map

        if (!map->check_rect(x,y,w,h,-1))
            return false;		// There already something
        float dh = fabs(map->check_rect_dh(x,y,w,h));
        float max_depth = map->check_max_depth(x,y,w,h);
        float min_depth = map->check_min_depth(x,y,w,h);

        if (!map->check_rect_discovered( x, y, w, h, 1<<player_id ) )
            return false;

        if (dh>unit_manager.unit_type[unit_type_id]->MaxSlope*H_DIV)
            return false;	// Check the slope

        // Check if unit can be there
        if (min_depth<unit_manager.unit_type[unit_type_id]->MinWaterDepth*H_DIV || max_depth>unit_manager.unit_type[unit_type_id]->MaxWaterDepth*H_DIV)
            return false;
        //	if (depth>0 && (unit_manager.unit_type[unit_type_id]->Category&NOTSUB))	return false;

        if (!map->check_vents(x,y,w,h,unit_manager.unit_type[unit_type_id]->yardmap))
            return false;

        if (map->check_lava((x+1)>>1,(y+1)>>1,(w+1)>>1,(h+1)>>1))
            return false;

        return true;
    }


    void INGAME_UNITS::complete_menu(int index,bool hide_info,bool hide_bpic)
    {
        pMutex.lock();

        bool pointed_only = false;
        if (last_on >= 0 && ( last_on >= max_unit || unit[ last_on ].flags == 0 ) ) 	last_on = -1;
        if (index<0 || index>=max_unit || unit[index].flags==0 || unit[index].type_id < 0 ) {
            if (last_on >= 0 )
                pointed_only = true;
            else {
                pMutex.unlock();
                return;		// On n'affiche que des données sur les unités EXISTANTES
            }
        }

        set_uformat(U_ASCII);

        UNIT *target = pointed_only ? NULL : (unit[index].mission!=NULL ? (UNIT*) unit[index].mission->p : NULL);
        if (target && target->flags==0)
            target = NULL;

        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);

        if (!pointed_only && !hide_bpic )
        {
            int stock=0;
            for(int i = 0 ; i < unit_manager.unit_type[unit[index].type_id]->weapon.size() ; i++)
                if (unit_manager.unit_type[unit[index].type_id]->weapon[i] && unit_manager.unit_type[unit[index].type_id]->weapon[i]->stockpile)
                {
                    stock = unit[index].weapon[i].stock;
                    break;
                }

            if ((unit_manager.unit_type[unit[index].type_id]->Builder && !unit_manager.unit_type[unit[index].type_id]->BMcode)
                || unit[index].planned_weapons>0.0f || stock>0) // Affiche la liste de construction
            {
                int page=unit_manager.unit_type[unit[index].type_id]->page;

                glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_COLOR);
                for( int i = 0 ; i < unit_manager.unit_type[unit[index].type_id]->nb_unit ; i++ ) // Affiche les différentes images d'unités constructibles
                {
                    if (unit_manager.unit_type[unit[index].type_id]->Pic_p[i] != page )
                        continue;
                    int px = unit_manager.unit_type[unit[index].type_id]->Pic_x[ i ];
                    int py = unit_manager.unit_type[unit[index].type_id]->Pic_y[ i ];
                    int pw = unit_manager.unit_type[unit[index].type_id]->Pic_w[ i ];
                    int ph = unit_manager.unit_type[unit[index].type_id]->Pic_h[ i ];

                    int nb=0;
                    MISSION *m=unit[index].mission;
                    while(m)
                    {
                        if ((m->mission==MISSION_BUILD || m->mission==MISSION_BUILD_2) && m->data==unit_manager.unit_type[unit[index].type_id]->BuildList[i])
                            nb++;
                        m=m->next;
                    }
                    if (nb>0)
                    {
                        char buf[10];
                        uszprintf(buf,10,"%d",nb);
                        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                        gfx->print(gfx->TA_font,px+pw*0.5f-0.5f*gfx->TA_font.length(buf),py+ph*0.5f-0.5f*gfx->TA_font.height(),0.0f,0xFFFFFFFF,buf);
                    }
                    else
                    {
                        if (unit_manager.unit_type[unit[index].type_id]->BuildList[i] == -1) // Il s'agit d'une arme / It's a weapon
                        {
                            char buf[10];
                            if ((int)unit[index].planned_weapons==unit[index].planned_weapons)
                                uszprintf(buf,10,"%d(%d)",(int)unit[index].planned_weapons,stock);
                            else
                                uszprintf(buf,10,"%d(%d)",(int)unit[index].planned_weapons+1,stock);
                            glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                            gfx->print(gfx->TA_font,px+pw*0.5f-0.5f*gfx->TA_font.length(buf),py+ph*0.5f-0.5f*gfx->TA_font.height(),0.0f,0xFFFFFFFF,buf);
                        }
                    }
                }
            }
        }

        if (last_on >= 0 )
        {
            index = last_on;
            if (unit[index].owner_id == players.local_human_id ) {
                target = unit[index].mission!=NULL ? (UNIT*) unit[index].mission->p : NULL;
                if (target && target->flags==0)
                    target=NULL;
            }
            else
                target = NULL;
        }

        if (!hide_info)
        {
            pMutex.unlock();
            unit[index].lock();

            if (unit[index].type_id >= 0 && (unit[index].flags & 1) )
            {
                glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                gfx->print_center(gfx->normal_font, ta3dSideData.side_int_data[ players.side_view ].UnitName.x1, ta3dSideData.side_int_data[ players.side_view ].UnitName.y1,0.0f,0xFFFFFFFF,unit_manager.unit_type[unit[index].type_id]->name);
                if (target && unit[index].mission && (unit[index].mission->flags & MISSION_FLAG_TARGET_WEAPON) != MISSION_FLAG_TARGET_WEAPON)
                {
                    unit[index].unlock();
                    target->lock();
                    if ((target->flags & 1) && target->type_id >= 0 )
                    {
                        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                        gfx->print_center(gfx->normal_font, ta3dSideData.side_int_data[ players.side_view ].UnitName2.x1, ta3dSideData.side_int_data[ players.side_view ].UnitName2.y1,0.0f,0xFFFFFFFF,unit_manager.unit_type[target->type_id]->name);
                    }
                    target->unlock();
                    unit[index].lock();
                }
                else
                    if (unit[index].planned_weapons>0.0f && unit[index].owner_id == players.local_human_id )
                    {
                        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
                        gfx->print_center(gfx->normal_font, ta3dSideData.side_int_data[ players.side_view ].UnitName2.x1, ta3dSideData.side_int_data[ players.side_view ].UnitName2.y1,0.0f,0xFFFFFFFF,I18N::Translate("weapon"));
                    }

                glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_COLOR);
                glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

                if (unit[index].owner_id == players.local_human_id  )
                {
                    char buf[10];
                    gfx->set_color( ta3dSideData.side_int_data[ players.side_view ].metal_color );
                    uszprintf(buf,10,"+%f",unit[index].cur_metal_prod);	*(strstr(buf,".")+2)=0;
                    gfx->print_center(gfx->small_font, ta3dSideData.side_int_data[ players.side_view ].UnitMetalMake.x1, ta3dSideData.side_int_data[ players.side_view ].UnitMetalMake.y1,0.0f,buf);
                    uszprintf(buf,10,"-%f",unit[index].cur_metal_cons);	*(strstr(buf,".")+2)=0;
                    gfx->print_center(gfx->small_font, ta3dSideData.side_int_data[ players.side_view ].UnitMetalUse.x1, ta3dSideData.side_int_data[ players.side_view ].UnitMetalUse.y1,0.0f,buf);

                    gfx->set_color( ta3dSideData.side_int_data[ players.side_view ].energy_color );
                    uszprintf(buf,10,"+%f",unit[index].cur_energy_prod);	*(strstr(buf,".")+2)=0;
                    gfx->print_center(gfx->small_font, ta3dSideData.side_int_data[ players.side_view ].UnitEnergyMake.x1, ta3dSideData.side_int_data[ players.side_view ].UnitEnergyMake.y1,0.0f,buf);
                    uszprintf(buf,10,"-%f",unit[index].cur_energy_cons);	*(strstr(buf,".")+2)=0;
                    gfx->print_center(gfx->small_font, ta3dSideData.side_int_data[ players.side_view ].UnitEnergyUse.x1, ta3dSideData.side_int_data[ players.side_view ].UnitEnergyUse.y1,0.0f,buf);
                }

                glColor4ub(0xFF,0xFF,0xFF,0xFF);

                glDisable(GL_TEXTURE_2D);

                glDisable(GL_BLEND);

                glBegin(GL_QUADS);
                glColor4ub(0xFF,0,0,0xFF);

                if (unit[index].owner_id == players.local_human_id || !unit_manager.unit_type[unit[index].type_id]->HideDamage )
                {
                    glVertex2i( ta3dSideData.side_int_data[ players.side_view ].DamageBar.x1, ta3dSideData.side_int_data[ players.side_view ].DamageBar.y1 );
                    glVertex2i( ta3dSideData.side_int_data[ players.side_view ].DamageBar.x2, ta3dSideData.side_int_data[ players.side_view ].DamageBar.y1 );
                    glVertex2i( ta3dSideData.side_int_data[ players.side_view ].DamageBar.x2, ta3dSideData.side_int_data[ players.side_view ].DamageBar.y2 );
                    glVertex2i( ta3dSideData.side_int_data[ players.side_view ].DamageBar.x1, ta3dSideData.side_int_data[ players.side_view ].DamageBar.y2 );
                }

                if (unit[index].owner_id == players.local_human_id )
                {
                    if (target && (unit[index].mission->flags & MISSION_FLAG_TARGET_WEAPON)!=MISSION_FLAG_TARGET_WEAPON )
                    {
                        unit[index].unlock();
                        target->lock();
                        if ((target->flags & 1) && target->type_id >= 0 && !unit_manager.unit_type[target->type_id]->HideDamage )	// Si l'unité a une cible
                        {
                            glVertex2i( ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x1, ta3dSideData.side_int_data[ players.side_view ].DamageBar2.y1 );
                            glVertex2i( ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x2, ta3dSideData.side_int_data[ players.side_view ].DamageBar2.y1 );
                            glVertex2i( ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x2, ta3dSideData.side_int_data[ players.side_view ].DamageBar2.y2 );
                            glVertex2i( ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x1, ta3dSideData.side_int_data[ players.side_view ].DamageBar2.y2 );
                        }
                        target->unlock();
                        unit[index].lock();
                    }
                    else 
                        if (unit[index].planned_weapons>0.0f )
                        {
                            glVertex2i( ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x1, ta3dSideData.side_int_data[ players.side_view ].DamageBar2.y1 );
                            glVertex2i( ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x2, ta3dSideData.side_int_data[ players.side_view ].DamageBar2.y1 );
                            glVertex2i( ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x2, ta3dSideData.side_int_data[ players.side_view ].DamageBar2.y2 );
                            glVertex2i( ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x1, ta3dSideData.side_int_data[ players.side_view ].DamageBar2.y2 );
                        }
                }

                glColor3ub(0,0xFF,0);

                if (unit[index].hp>0 && ( unit[index].owner_id == players.local_human_id || !unit_manager.unit_type[unit[index].type_id]->HideDamage ) )
                {
                    glVertex2f( ta3dSideData.side_int_data[ players.side_view ].DamageBar.x1, ta3dSideData.side_int_data[ players.side_view ].DamageBar.y1 );
                    glVertex2f( ta3dSideData.side_int_data[ players.side_view ].DamageBar.x1 + unit[index].hp / unit_manager.unit_type[unit[index].type_id]->MaxDamage * (ta3dSideData.side_int_data[ players.side_view ].DamageBar.x2-ta3dSideData.side_int_data[ players.side_view ].DamageBar.x1), ta3dSideData.side_int_data[ players.side_view ].DamageBar.y1 );
                    glVertex2f( ta3dSideData.side_int_data[ players.side_view ].DamageBar.x1 + unit[index].hp / unit_manager.unit_type[unit[index].type_id]->MaxDamage * (ta3dSideData.side_int_data[ players.side_view ].DamageBar.x2-ta3dSideData.side_int_data[ players.side_view ].DamageBar.x1), ta3dSideData.side_int_data[ players.side_view ].DamageBar.y2 );
                    glVertex2f( ta3dSideData.side_int_data[ players.side_view ].DamageBar.x1, ta3dSideData.side_int_data[ players.side_view ].DamageBar.y2 );
                }

                if (unit[index].owner_id == players.local_human_id )
                {
                    if (target && (unit[index].mission->flags & MISSION_FLAG_TARGET_WEAPON)!=MISSION_FLAG_TARGET_WEAPON )
                    {
                        unit[index].unlock();
                        target->lock();
                        if ((target->flags & 1) && target->type_id >= 0 && !unit_manager.unit_type[target->type_id]->HideDamage && target->hp>0)
                        {
                            glVertex2f( ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x1, ta3dSideData.side_int_data[ players.side_view ].DamageBar2.y1 );
                            glVertex2f( ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x1 + target->hp / unit_manager.unit_type[target->type_id]->MaxDamage * (ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x2-ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x1), ta3dSideData.side_int_data[ players.side_view ].DamageBar2.y1 );
                            glVertex2f( ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x1 + target->hp / unit_manager.unit_type[target->type_id]->MaxDamage * (ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x2-ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x1), ta3dSideData.side_int_data[ players.side_view ].DamageBar2.y2 );
                            glVertex2f( ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x1, ta3dSideData.side_int_data[ players.side_view ].DamageBar2.y2 );
                        }
                        target->unlock();
                        unit[index].lock();
                    }
                    else 
                        if (unit[index].planned_weapons>0.0f ) 	// construit une arme / build a weapon
                        {
                            float p=1.0f-(unit[index].planned_weapons-(int)unit[index].planned_weapons);
                            if (p==1.0f)
                                p=0.0f;
                            glVertex2f( ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x1, ta3dSideData.side_int_data[ players.side_view ].DamageBar2.y1 );
                            glVertex2f( ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x1 + p * (ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x2-ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x1), ta3dSideData.side_int_data[ players.side_view ].DamageBar2.y1 );
                            glVertex2f( ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x1 + p * (ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x2-ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x1), ta3dSideData.side_int_data[ players.side_view ].DamageBar2.y2 );
                            glVertex2f( ta3dSideData.side_int_data[ players.side_view ].DamageBar2.x1, ta3dSideData.side_int_data[ players.side_view ].DamageBar2.y2 );
                        }
                }

                glEnd();
            }

            unit[index].unlock();
            pMutex.lock();
        }
        else
        {
            glDisable( GL_BLEND );
            glDisable( GL_TEXTURE_2D );
        }
        glColor4ub(0xFF,0xFF,0xFF,0xFF);
        set_uformat(U_UTF8);

        pMutex.unlock();
    }

    void INGAME_UNITS::move(float dt,MAP *map,int key_frame,bool wind_change)
    {
        if (nb_unit<=0 || unit==NULL)
        {
            rest(1);
            return;// No units to move
        }

        players.clear();		// Réinitialise le compteur de ressources

        if (requests.empty())
            requests.resize(TA3D_PLAYERS_HARD_LIMIT);

        int pathfinder_calls[TA3D_PLAYERS_HARD_LIMIT];

        for (short int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
            pathfinder_calls[i] = requests[i].empty() ? -1 : requests[i].front();

        uint32 i;
        pMutex.lock();
        for (uint16 e = 0; e < index_list_size; ++e) // Compte les stocks de ressources et les productions
        {
            i = idx_list[e];
            pMutex.unlock();

            unit[i].lock();

            if (unit[i].just_created && unit_manager.unit_type[unit[i].type_id]->ExtractsMetal ) // Compute amount of metal extracted by sec
            {
                int metal_base = 0;
                int px=unit[i].cur_px;
                int py=unit[i].cur_py;
                int start_x = px - (unit_manager.unit_type[unit[i].type_id]->FootprintX >> 1 );
                int start_y = py - (unit_manager.unit_type[unit[i].type_id]->FootprintZ >> 1 );
                int end_y = start_y + unit_manager.unit_type[unit[i].type_id]->FootprintZ;
                int end_x = start_x + unit_manager.unit_type[unit[i].type_id]->FootprintX;
                for( int ry = start_y ; ry <= end_y ; ++ry)
                {
                    if (ry >= 0 && ry < map->bloc_h_db )
                    {
                        for( int rx = start_x ; rx <= end_x ; rx++ )
                        {
                            if (rx >= 0 && rx < map->bloc_w_db )
                            {
                                if (map->map_data[ry][rx].stuff>=0)
                                {
                                    metal_base = feature_manager.feature[features.feature[map->map_data[ry][rx].stuff].type].metal * unit_manager.unit_type[unit[i].type_id]->FootprintZ * unit_manager.unit_type[unit[i].type_id]->FootprintX;
                                    ry = end_y;
                                    break;
                                }
                                else
                                    metal_base += map->ota_data.SurfaceMetal;
                            }
                        }
                    }
                }
                unit[i].metal_extracted = metal_base * unit_manager.unit_type[unit[i].type_id]->ExtractsMetal;

                int param[] = { metal_base<<2 };
                unit[i].run_script_function( map, unit[i].get_script_index(SCRIPT_SetSpeed),1,param);
                unit[i].just_created=false;
            }

            if (unit[i].build_percent_left==0.0f)
            {
                unit[i].metal_prod=0.0f;
                unit[i].metal_cons=0.0f;
                unit[i].energy_prod=0.0f;
                unit[i].energy_cons=0.0f;
                players.c_metal_s[unit[i].owner_id]+=unit_manager.unit_type[unit[i].type_id]->MetalStorage;
                players.c_energy_s[unit[i].owner_id]+=unit_manager.unit_type[unit[i].type_id]->EnergyStorage;
                players.c_commander[unit[i].owner_id]|=(unit_manager.unit_type[unit[i].type_id]->TEDclass==CLASS_COMMANDER);
                unit[i].energy_prod+=unit_manager.unit_type[unit[i].type_id]->EnergyMake;
                if ((unit[i].port[ACTIVATION] || !unit_manager.unit_type[unit[i].type_id]->onoffable)
                    && unit_manager.unit_type[unit[i].type_id]->EnergyUse<=players.energy[unit[i].owner_id])
                {
                    unit[i].metal_prod+=unit_manager.unit_type[unit[i].type_id]->MakesMetal+unit_manager.unit_type[unit[i].type_id]->MetalMake;
                    if (unit_manager.unit_type[unit[i].type_id]->ExtractsMetal)	// Extracteur de métal
                        unit[i].metal_prod += unit[i].metal_extracted;
                    if (unit_manager.unit_type[unit[i].type_id]->WindGenerator) // Wind Generator
                    {
                        unit[i].energy_prod+=map->wind*unit_manager.unit_type[unit[i].type_id]->WindGenerator*0.0002f;
                        if (wind_change)
                        {
                            int param[] = { (int)(map->wind*50.0f) };
                            unit[i].launch_script(unit[i].get_script_index(SCRIPT_SetSpeed),1,param);
                            param[0]=(int)((map->wind_dir-unit[i].Angle.y)*DEG2TA);
                            unit[i].launch_script(unit[i].get_script_index(SCRIPT_SetDirection),1,param);
                            unit[i].launch_script(unit[i].get_script_index(SCRIPT_go));
                        }
                    }
                    if (unit_manager.unit_type[unit[i].type_id]->TidalGenerator)	// Tidal Generator
                        unit[i].energy_prod+=map->ota_data.tidalstrength;
                    if (unit_manager.unit_type[unit[i].type_id]->EnergyUse<0)
                        unit[i].energy_prod-=unit_manager.unit_type[unit[i].type_id]->EnergyUse;
                    else
                        unit[i].energy_cons=unit_manager.unit_type[unit[i].type_id]->EnergyUse;
                }
            }
            unit[i].unlock();
            pMutex.lock();
        }
        pMutex.unlock();

        exp_dt_1=exp(-dt);
        exp_dt_2=exp(-2.0f*dt);
        exp_dt_4=exp(-4.0f*dt);
        g_dt=dt*map->ota_data.gravity;
        int *path_exec = new int[players.nb_player];
        memset( path_exec, 0, sizeof( int ) * players.nb_player);
        pMutex.lock();
        for (uint16 e = 0 ; e < index_list_size ; ++e)
        {
            i = idx_list[e];
            pMutex.unlock();
            unit[ i ].lock();

            if (unit[ i ].flags == 0 ) // ho ho what is it doing there ??
            {
                unit[ i ].unlock();
                kill(i,map,e);
                --e;			// Can't skip a unit
                pMutex.lock();
                continue;
            }

            if (unit[i].owner_id==players.local_human_id)
            {
                if (unit[i].attacked || (unit[i].mission!=NULL && unit[i].mission->mission==MISSION_ATTACK))
                    nb_attacked+=100;
                if (unit[i].built)
                    nb_built++;
            }
            players.c_nb_unit[unit[i].owner_id]++;			// Compte les unités de chaque joueur
            unit[i].unlock();
            if (unit[i].move(dt,map,path_exec,key_frame) == -1) // Vérifie si l'unité a été détruite
            {
                if (unit[i].local ) // Don't kill remote units, since we're told when to kill them
                {
                    kill(i,map,e);
                    e--;			// Can't skip a unit
                }
            }
            else
            {
                unit[ i ].lock();
                players.c_metal_t[unit[i].owner_id] += unit[i].metal_prod;
                players.c_metal_u[unit[i].owner_id] += unit[i].metal_cons;
                players.c_energy_t[unit[i].owner_id] += unit[i].energy_prod;
                players.c_energy_u[unit[i].owner_id] += unit[i].energy_cons;

                unit[i].cur_energy_cons = unit[i].energy_cons;
                unit[i].cur_energy_prod = unit[i].energy_prod;
                unit[i].cur_metal_cons = unit[i].metal_cons;
                unit[i].cur_metal_prod = unit[i].metal_prod;
                unit[ i ].unlock();
            }
            pMutex.lock();
        }
        pMutex.unlock();

        delete[] path_exec;

        float exp_r = exp(-dt*0.1f);
        nb_attacked*=exp_r;
        nb_built*=exp_r;

        pMutex.lock();

        for(i=0;i<players.nb_player; ++i)
        {
            players.c_annihilated[i] = !players.c_nb_unit[i]; // Has this player units ?
            if (players.c_commander[i])
            {
                players.c_metal_s[i]+=players.com_metal[i];
                players.c_energy_s[i]+=players.com_energy[i];
            }
        }
        for (i=0; i < players.nb_player; ++i)
        {
            players.c_metal[i]+=dt*(players.c_metal_t[i]-players.c_metal_u[i]);
            players.c_energy[i]+=dt*(players.c_energy_t[i]-players.c_energy_u[i]);
            players.metal_total[i]+=dt*players.metal_t[i];
            players.energy_total[i]+=dt*players.energy_t[i];
            if (players.c_metal[i]<0.0f)
                players.c_metal[i]=0.0f;
            else
                if (players.c_metal[i]>players.c_metal_s[i])
                    players.c_metal[i]=players.c_metal_s[i];
            if (players.c_energy[i]<0.0f)
                players.c_energy[i]=0.0f;
            else
                if (players.c_energy[i]>players.c_energy_s[i])
                    players.c_energy[i]=players.c_energy_s[i];
        }

        for (short int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
        {
            if (!requests[i].empty() && pathfinder_calls[i] == requests[i].front())
                requests[i].pop_front();
        }

        players.refresh();
        pMutex.unlock();
    }

    int INGAME_UNITS::create(int type_id,int owner)
    {
        if (type_id<0 || type_id>=unit_manager.nb_unit)	return -1;
        if (owner<0 || owner>=NB_PLAYERS)	return -1;
        if (nb_unit>=MAX_UNIT_PER_PLAYER*NB_PLAYERS)		return -1;
        if (free_index_size[owner] <= 0 && max_unit > 0 )	return -1;

        pMutex.lock();

        nb_unit++;
        if (nb_unit>max_unit && max_unit == 0)
        {
            if (mini_col )		delete[]	mini_col;
            if (mini_pos )		delete[]	mini_pos;

            max_unit=MAX_UNIT_PER_PLAYER*NB_PLAYERS;

            mini_col = new uint32[ max_unit ];
            mini_pos = new float[ max_unit * 2 ];

            UNIT *n_unit = new UNIT[max_unit];
            uint16	*n_idx = new uint16[max_unit];
            uint16	*n_new_idx = new uint16[max_unit];
            if (index_list_size>0)
                memcpy(n_idx,idx_list,index_list_size<<1);
            if (free_idx)
                memcpy(n_new_idx,free_idx,max_unit<<1);
            if (idx_list)	delete[]	idx_list;
            if (free_idx)	delete[]	free_idx;
            idx_list = n_idx;
            free_idx = n_new_idx;
            for(uint16 i = 0; i<max_unit;i++)
                free_idx[i] = i;
            for (short int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
                free_index_size[i] = MAX_UNIT_PER_PLAYER;
            for (int i = 0; i < max_unit; ++i)
            {
                n_unit[i].init(-1,-1,i>=nb_unit-1);
                n_unit[i].flags=0;
                n_unit[i].idx=i;
            }
            if (unit)
            {
                memcpy(n_unit,unit,sizeof(UNIT)*(nb_unit-1));
                delete[] unit;
            }
            unit=n_unit;
        }
        if (!unit)
            LOG_CRITICAL("Memory alloc failed");
        if (free_index_size[owner]<=0) {
            pMutex.unlock();
            LOG_WARNING("Unit limit reached !");
            return -1;
        }
        int unit_index = free_idx[owner*MAX_UNIT_PER_PLAYER+free_index_size[owner]-1];
        free_index_size[owner]--;
        unit[unit_index].init(type_id,owner);
        unit[unit_index].ID = next_unit_ID++;		// So now we know who is this unit :)

        // Angle de 10° maximum
        unit[unit_index].Angle.y = (((sint32)(Math::RandFromTable() % 20001)) - 10000) * 0.0001f * unit_manager.unit_type[type_id]->BuildAngle * TA2DEG; 

        idx_list[index_list_size++] = unit_index;

        if (unit_manager.unit_type[type_id]->IsAirBase)			// Say we're here !
            repair_pads[ owner ].push_front(unit_index);

        players.nb_unit[owner]++;
        pMutex.unlock();

        return unit_index;
    }

    void INGAME_UNITS::draw_mini(float map_w,float map_h,int mini_w,int mini_h,SECTOR **map_data)				// Repère les unités sur la mini-carte
    {
        if (nb_unit<=0 || unit==NULL)
        {
            last_on = -1;
            return;		// Pas d'unités à dessiner
        }

        float rw = 128.0f * mini_w / 252 / map_w;
        float rh = 128.0f * mini_h / 252 / map_h;

        glDisable(GL_TEXTURE_2D);
        glPointSize(3.0f);

        byte mask=1<<players.local_human_id;
        int b_w=(int)map_w>>3;
        int b_h=(int)map_h>>3;
        int nb = 0;

        uint32 player_col_32[TA3D_PLAYERS_HARD_LIMIT];
        uint32 player_col_32_h[TA3D_PLAYERS_HARD_LIMIT];
        for (short int i = 0; i < players.nb_player; ++i)
        {
            player_col_32[i] =  makeacol( (int)(player_color[ player_color_map[ i ] * 3 ] * 255.0f),
                                          (int)(player_color[ player_color_map[ i ] * 3 + 1 ] * 255.0f),
                                          (int)(player_color[ player_color_map[ i ] * 3 + 2 ] * 255.0f),
                                          i );
            player_col_32_h[i] =  makeacol( (int)(player_color[ player_color_map[ i ] * 3 ] * 127.5f),
                                            (int)(player_color[ player_color_map[ i ] * 3 + 1 ] * 127.5f),
                                            (int)(player_color[ player_color_map[ i ] * 3 + 2 ] * 127.5f),
                                            i );
        }

        pMutex.lock();
        for (uint16 e=0 ; e < index_list_size ; e++)
        {
            uint16 i = idx_list[e];
            pMutex.unlock();

            units.unit[ i ].lock();

            if (unit[i].flags&1)
            {
                int px=unit[i].cur_px;
                int py=unit[i].cur_py;
                if (px<0 || py<0 || px>=b_w || py>=b_h)
                {
                    units.unit[ i ].unlock();
                    pMutex.lock();
                    continue;
                }
                if ((!(map->view_map->line[py>>1][px>>1]&mask) || !(map->sight_map->line[py>>1][px>>1]&mask) || (unit[i].cloaked && unit[i].owner_id != players.local_human_id ) ) && !unit[i].on_mini_radar )
                {
                    units.unit[ i ].unlock();
                    pMutex.lock();
                    continue;	// Unité non visible / Unit is not visible
                }
                //			unit[i].flags|=0x10;
                mini_pos[ nb << 1 ] = unit[i].Pos.x;
                mini_pos[ (nb << 1) + 1 ] = unit[i].Pos.z;
                mini_col[ nb++ ] = player_col_32_h[ unit[i].owner_id ];
            }
            units.unit[ i ].unlock();
            pMutex.lock();
        }
        pMutex.unlock();
        glEnableClientState(GL_VERTEX_ARRAY);		// vertex coordinates
        glEnableClientState(GL_COLOR_ARRAY);		// Colors(for fog of war)
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);		// vertex coordinates

        glColorPointer(4, GL_UNSIGNED_BYTE, 0, mini_col);
        glVertexPointer( 2, GL_FLOAT, 0, mini_pos);
        glPushMatrix();
        glTranslatef( 63.0f, 64.0f, 0.0f );
        glScalef( rw, rh, 0.0f );
        glDrawArrays(GL_POINTS, 0, nb);		// draw the points

        glPopMatrix();
        glPushMatrix();
        glTranslatef( 65.0f, 64.0f, 0.0f );
        glScalef( rw, rh, 0.0f );
        glDrawArrays(GL_POINTS, 0, nb);		// draw the points

        glPopMatrix();
        glPushMatrix();
        glTranslatef( 64.0f, 65.0f, 0.0f );
        glScalef( rw, rh, 0.0f );
        glDrawArrays(GL_POINTS, 0, nb);		// draw the points

        glPopMatrix();
        glPushMatrix();
        glTranslatef( 64.0f, 63.0f, 0.0f );
        glScalef( rw, rh, 0.0f );
        glDrawArrays(GL_POINTS, 0, nb);		// draw the points

        glPopMatrix();
        glPushMatrix();
        glTranslatef( 64.0f, 64.0f, 0.0f );
        glScalef( rw, rh, 0.0f );
        for( int i = 0 ; i < nb ; i++ )
            mini_col[ i ] = player_col_32[ geta( mini_col[ i ] ) ];
        glDrawArrays(GL_POINTS, 0, nb);		// draw the points
        glPopMatrix();

        int cur_id = -1;
        glBegin( GL_POINTS );
        pMutex.lock();
        for(uint16 e=0;e<index_list_size; ++e)
        {
            uint16 i = idx_list[e];
            pMutex.unlock();

            units.unit[ i ].lock();
            if (units.unit[ i ].cur_px < 0 || units.unit[ i ].cur_py < 0 || units.unit[ i ].cur_px >= b_w || units.unit[ i ].cur_py >= b_h ) {
                units.unit[ i ].unlock();
                pMutex.lock();
                continue;
            }

            if ((unit[i].flags&1) && ( (unit[i].owner_id==players.local_human_id && unit[i].sel) || i == last_on ) )
            {
                cur_id = unit[i].owner_id;
                float pos_x=unit[i].Pos.x*rw+64.0f;
                float pos_y=unit[i].Pos.z*rh+64.0f;
                if (unit[i].radar_range > 0 )
                {
                    glEnd();
                    glPointSize(1.0f);
                    gfx->circle_zoned( pos_x, pos_y, (unit[i].radar_range << 3) * rw, 0.0f, 0.0f, 127.0f, 127.0f, 0xFFFFFFFF );
                    glPointSize(3.0f);
                    glBegin( GL_POINTS );
                }
                if (unit[i].sonar_range > 0 )
                {
                    glEnd();
                    glPointSize(1.0f);
                    gfx->circle_zoned( pos_x, pos_y, (unit[i].sonar_range << 3) * rw, 0.0f, 0.0f, 127.0f, 127.0f, makecol( 0, 255, 0 ) );
                    glPointSize(3.0f);
                    glBegin( GL_POINTS );
                }
                glColor3ub(0xFF,0xFF,0xFF);
                glVertex2f(pos_x-1.0f,pos_y);
                glVertex2f(pos_x+1.0f,pos_y);
                glVertex2f(pos_x,pos_y-1.0f);
                glVertex2f(pos_x,pos_y+1.0f);

                glColor3f(player_color[3*player_color_map[cur_id]],player_color[3*player_color_map[cur_id]+1],player_color[3*player_color_map[cur_id]+2]);
                glVertex2f(pos_x,pos_y);
            }
            units.unit[ i ].unlock();
            pMutex.lock();
        }
        pMutex.unlock();
        glEnd();
        glPointSize(1.0f);
        glEnable(GL_TEXTURE_2D);

        last_on = -1;
    }

    void INGAME_UNITS::kill(int index,MAP *map,int prev,bool sync)			// Détruit une unité
    {
        if (index<0 || index>=max_unit || prev<0 || prev>=index_list_size)	// On ne peut pas détruire une unité qui n'existe pas
            return;

        unit[index].lock();

        if (unit[index].local && network_manager.isConnected() && sync ) // Send EVENT_UNIT_DEATH
        {
            struct event event;
            event.type = EVENT_UNIT_DEATH;
            event.opt1 = index;
            network_manager.sendEvent( &event );
        }

        if (unit[ index ].type_id >= 0 && unit_manager.unit_type[ unit[ index ].type_id ]->IsAirBase ) // Remove it from repair_pads list
        {
            int owner_id = unit[ index ].owner_id;

            pMutex.lock();
            for (std::list<uint16>::iterator i = repair_pads[owner_id].begin(); i != repair_pads[owner_id].end(); ++i)
            {
                if (*i == index)
                {
                    repair_pads[owner_id].erase(i);
                    break;
                }
            }
            pMutex.unlock();
        }

        if (unit[index].flags & 1 )
        {
            if (unit[ index ].mission
                && !unit_manager.unit_type[ unit[ index ].type_id ]->BMcode
                && ( unit[ index ].mission->mission == MISSION_BUILD_2 || unit[ index ].mission->mission == MISSION_BUILD )		// It was building something that we must destroy too
                && unit[ index ].mission->p != NULL )
            {
                ((UNIT*)(unit[ index ].mission->p))->lock();
                ((UNIT*)(unit[ index ].mission->p))->hp = 0.0f;
                ((UNIT*)(unit[ index ].mission->p))->built = false;
                ((UNIT*)(unit[ index ].mission->p))->unlock();
            }
            players.nb_unit[ unit[index].owner_id ]--;
            players.losses[ unit[index].owner_id ]++;		// Statistics
        }

        unit[index].unlock();
        unit[index].clear_from_map();
        unit[index].lock();

        if (unit[index].type_id >= 0 && unit_manager.unit_type[unit[index].type_id]->canload && unit[index].nb_attached>0)
        {
            for(int i = 0 ; i < unit[index].nb_attached ; ++i)
            {
                unit[unit[index].attached_list[i]].lock();
                unit[unit[index].attached_list[i]].hp = 0.0f;
                unit[unit[index].attached_list[i]].unlock();
            }
        }
        unit[index].unlock();
        unit[index].destroy();		// Détruit l'unité

        pMutex.lock();

        uint16 owner = index/MAX_UNIT_PER_PLAYER;
        free_idx[ MAX_UNIT_PER_PLAYER * owner + free_index_size[ owner ]++ ] = index;
        idx_list[ prev ] = idx_list[ --index_list_size ];
        --nb_unit; // Unité détruite

        pMutex.unlock();
    }



    void INGAME_UNITS::draw(Camera& cam, MAP* map, bool underwater, bool limit, bool cullface, bool height_line)					// Dessine les unités visibles
    {
        if (nb_unit <= 0 || !unit)
            return;		// Pas d'unités à dessiner

        glEnable(GL_LIGHTING);
        if (cullface)
            glEnable(GL_CULL_FACE);
        else
            glDisable(GL_CULL_FACE);

        glDisable(GL_BLEND);
        glColor4ub(0xFF,0xFF,0xFF,0xFF);
        float sea_lvl = limit ? map->sealvl-5.0f : map->sealvl;
        float virtual_t = (float)current_tick / TICKS_PER_SEC;
        cam.setView();
        pMutex.lock();
        bool low_def = cam.rpos.y > gfx->low_def_limit;
        if (low_def)
            glDisable(GL_DEPTH_TEST);

        for (uint16 e = 0; e < index_list_size; ++e)
        {
            uint16 i = idx_list[e];
            pMutex.unlock();

            unit[i].lock();
            if ((unit[i].flags & 1)
            && (low_def || (unit[i].Pos.y + unit[i].model->bottom <= map->sealvl && underwater)
            || (unit[i].Pos.y + unit[i].model->top >= sea_lvl && !underwater))) // Si il y a une unité / If there is a unit
            {
                unit[i].unlock();
                unit[i].draw(virtual_t, cam, map, height_line);
            }
            else
                unit[i].unlock();
            pMutex.lock();
        }
        pMutex.unlock();

        glDisable(GL_ALPHA_TEST);

        if (low_def)
            glEnable(GL_DEPTH_TEST);

        if (!cullface)
            glEnable(GL_CULL_FACE);
    }



    void INGAME_UNITS::draw_shadow(Camera& cam, const Vector3D& Dir, MAP* map, float alpha)	// Dessine les ombres des unités visibles
    {
        if (nb_unit<=0 || unit==NULL) // Pas d'unités à dessiner
            return;

        cam.setView();

        if (g_useStencilTwoSide) // Si l'extension GL_EXT_stencil_two_side est disponible
        {
            glEnable(GL_STENCIL_TEST);
            glEnable(GL_STENCIL_TEST_TWO_SIDE_EXT);
            glDisable(GL_CULL_FACE);
            glClear(GL_STENCIL_BUFFER_BIT);
            glDepthMask(GL_FALSE);
            glColorMask(0, 0, 0,  0);
            glStencilFunc(GL_ALWAYS, 128, 0xffffffff);

            glActiveStencilFaceEXT(GL_FRONT);
            glStencilOp(GL_KEEP, GL_KEEP, GL_INCR_WRAP_EXT);
            glActiveStencilFaceEXT(GL_BACK);
            glStencilOp(GL_KEEP, GL_KEEP, GL_DECR_WRAP_EXT);

            for (uint16 e = 0; e < index_list_size; ++e)
            {
                pMutex.lock();
                uint16 i = idx_list[e];
                pMutex.unlock();

                gfx->lock();

                unit[i].lock();
                if (unit[i].flags & 1)				// Si il y a une unité
                    unit[i].draw_shadow(Dir, map);
                unit[i].unlock();

                gfx->unlock();
            }
        }
        else // Si elle ne l'est pas
        {
            glDepthMask(GL_FALSE);
            glColorMask(0,0,0,0);

            glClear(GL_STENCIL_BUFFER_BIT);
            glEnable(GL_STENCIL_TEST);
            glStencilFunc(GL_ALWAYS,128, 0xffffffff);
            glEnable(GL_CULL_FACE);

            for (uint16 e = 0; e < index_list_size; ++e)
            {
                pMutex.lock();
                uint16 i = idx_list[e];
                pMutex.unlock();

                gfx->lock();
                unit[i].lock();
                if (unit[i].flags & 1) // Si il y a une unité
                    unit[i].draw_shadow_basic(Dir, map);
                unit[i].unlock();
                gfx->unlock();
            }
        }

        gfx->lock();
        features.draw_shadow(cam, Dir);

        glColorMask(0xFF,0xFF,0xFF,0xFF);
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_NOTEQUAL,0, 0xffffffff);
        glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glColor4f(0.0f,0.0f,0.0f,alpha);
        glBegin(GL_QUADS);
        Vector3D P = cam.rpos + cam.shakeVector + 1.1f * (cam.dir + 0.75f * cam.up - cam.widthFactor * cam.side);
        glVertex3fv((const GLfloat*) &P);
        P = cam.rpos + cam.shakeVector + 1.1f * ( cam.dir + 0.75f * cam.up + cam.widthFactor * cam.side);
        glVertex3fv((const GLfloat*) &P);
        P = cam.rpos + cam.shakeVector + 1.1f * (cam.dir - 0.75f * cam.up + cam.widthFactor * cam.side);
        glVertex3fv((const GLfloat*) &P);
        P = cam.rpos + cam.shakeVector + 1.1f * (cam.dir - 0.75f * cam.up - cam.widthFactor * cam.side);
        glVertex3fv((const GLfloat*) &P);
        glEnd();
        glDepthMask(GL_TRUE);
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_BLEND);
        glColor4ub(0xFF,0xFF,0xFF,0xFF);

        gfx->unlock();
    }

    void INGAME_UNITS::remove_order(int player_id, const Vector3D& target)
    {
        pMutex.lock();
        for (uint16 e = 0; e < index_list_size; ++e)
        {
            uint16 i = idx_list[e];
            pMutex.unlock();
            unit[i].lock();
            if ((unit[i].flags & 1) && !unit[i].command_locked && unit[i].owner_id==player_id && unit[i].sel && unit[i].build_percent_left==0.0f) // && unit_manager.unit_type[unit[i].type_id]->Builder) {
            {
                MISSION *mission = unit_manager.unit_type[unit[i].type_id]->BMcode ? unit[i].mission : unit[i].def_mission;
                MISSION *prec = mission;
                if (mission != NULL && unit_manager.unit_type[unit[i].type_id]->BMcode)
                    mission = mission->next;		// Don't read the first one ( which is being executed )

                MISSION fake;
                if (!unit_manager.unit_type[unit[i].type_id]->BMcode ) // It's a hack to make sure it will work with first given order
                {
                    fake.next = mission;
                    prec = &fake;
                }

                while (mission) // Reads the mission list
                {
                    if (mission->mission == MISSION_BUILD )
                    {
                        prec = mission;
                        mission = mission->next;
                    }
                    else
                    {
                        if (!mission->step && (mission->target - target).sq() < 256.0f) // Remove it
                        {
                            MISSION *tmp = mission;
                            mission = mission->next;
                            prec->next = mission;
                            if (tmp->path)				// Destroy the path if needed
                                destroy_path(tmp->path);
                            delete tmp;
                        }
                        else {
                            prec = mission;
                            mission = mission->next;
                        }
                    }
                }
            }
            unit[i].unlock();
            pMutex.lock();
        }
        pMutex.unlock();
        }



        uint32 INGAME_UNITS::InterfaceMsg(const lpcImsg msg)
        {
            if (msg->MsgID == TA3D_IM_GUI_MSG )	// for GUI messages, test if it's a message for us
            {
                if (msg->lpParm1 == NULL)
                    return INTERFACE_RESULT_HANDLED;		// Oups badly written things
                String message((const char*) msg->lpParm1);				// Get the string associated with the signal
                if (!message.toLower().empty())
                {
                    if (message == "pause game")
                    {
                        lp_CONFIG->pause = true;
                        return INTERFACE_RESULT_HANDLED;
                    }
                    if (message == "resume game")
                    {
                        lp_CONFIG->pause = false;
                        return INTERFACE_RESULT_HANDLED;
                    }
                    if (message == "toggle pause")
                    {
                        lp_CONFIG->pause ^= true;
                        return INTERFACE_RESULT_HANDLED;
                    }
                }
            }
            return INTERFACE_RESULT_CONTINUE;						// Temporary, for now it does nothing
        }


        int INGAME_UNITS::Run()
        {
            thread_running = true;
            float dt = 1.0f / TICKS_PER_SEC;
            uint32 unit_timer = msec_timer;
            uint32 tick_timer;
            float counter = 0.0f;
            int tick = 1000 / TICKS_PER_SEC;
            tick_timer = msec_timer;
            uint32 net_timer = msec_timer;
            float step = 1.0f;
            if (lp_CONFIG->timefactor > 0.0f )	step = 1.0f / lp_CONFIG->timefactor;
            current_tick = 0;
            for (short int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
                client_tick[i] = client_speed[i] = 0;
            apparent_timefactor = lp_CONFIG->timefactor;

            unit_engine_thread_sync = 0;

            ThreadSynchroniser->lock();

            while( !thread_ask_to_stop )
            {
                counter += step;

                move( dt, map, current_tick, wind_change );					// Animate units

                pMutex.lock();

                if (map->fog_of_war )
                {
                    gfx->lock();

                    if (!(current_tick & 0xF) ) {
                        if (map->fog_of_war & FOW_GREY )
                            memset( map->sight_map->line[0], 0, map->sight_map->w * map->sight_map->h );		// Clear FOW map
                        memset( map->radar_map->line[0], 0, map->radar_map->w * map->radar_map->h );		// Clear radar map
                        memset( map->sonar_map->line[0], 0, map->sonar_map->w * map->sonar_map->h );		// Clear sonar map

                        for( int i = 0; i < index_list_size ; i++ )			// update fog of war, radar and sonar data
                            unit[ idx_list[ i ] ].draw_on_FOW();

                        for( int i = 0; i < index_list_size ; i++ )			// update radar and sonar jamming data
                            unit[ idx_list[ i ] ].draw_on_FOW( true );
                    }

                    gfx->unlock();
                }

                wind_change = false;
                pMutex.unlock();

                uint32 min_tick = 1000 * current_tick + 30000;
                if (network_manager.isConnected() )
                {
                    if (network_manager.isServer() )
                    {
                        for (sint8 i = 0 ; i < players.nb_player ; ++i)
                            if (g_ta3d_network->isRemoteHuman( i ) )
                                min_tick = Math::Min(min_tick, client_tick[i]);
                    }
                    else
                        for (sint8 i = 0 ; i < players.nb_player ; ++i)
                            if (g_ta3d_network->isRemoteHuman( i ) && client_tick[i] > 0 )
                                min_tick = Math::Min(min_tick, client_tick[i]);
                }
                min_tick /= 1000;

                if (network_manager.isConnected() && min_tick > current_tick )
                {
                    int delay = (min_tick - current_tick) * 250 / TICKS_PER_SEC;
                    tick += delay;
                }

                while (msec_timer - tick_timer + 1 < tick)
                    rest(1);

                while( msec_timer - tick_timer >= tick + 200 ) // Prevent the game to run too fast for too long, we don't have to speed up to compute what we hadn't time to
                {
                    counter += 1.0f;
                    tick = (int)( ( (counter + step ) * 1000 ) / TICKS_PER_SEC );		// For perfect sync with tick clock
                }

                tick = (int)( ( (counter + step ) * 1000 ) / TICKS_PER_SEC );		// For perfect sync with tick clock
                if (lp_CONFIG->timefactor > 0.0f )	step = 1.0f / lp_CONFIG->timefactor;
                else	step = 1.0f;

                ThreadSynchroniser->unlock();

                while( lp_CONFIG->pause && !thread_ask_to_stop )
                {
                    lp_CONFIG->paused = true;
                    rest(10); // in pause mode wait for pause to be false again
                }
                lp_CONFIG->paused = false;

                if (network_manager.isConnected())
                {
                    net_timer = msec_timer - net_timer;
                    for (sint8 i = 0 ; i < players.nb_player ; ++i)
                    {
                        if (g_ta3d_network->isRemoteHuman( i ) )
                            client_tick[ i ] += client_speed[ i ] * net_timer / (1000 * TICKS_PER_SEC);
                    }

                    net_timer = msec_timer;

                    network_manager.sendSpecial(format("TICK %d %d", current_tick + 1, (int)(1000.0f * apparent_timefactor) ));		// + 1 to prevent it from running too slow
                    if (current_tick > min_tick + TICKS_PER_SEC )
                    {
                        while( current_tick > min_tick && !thread_ask_to_stop )
                        {
                            players_thread_sync = 0;
                            rest(1);

                            min_tick = current_tick * 1000;
                            if (network_manager.isServer() )
                            {
                                for(sint8 i = 0; i < players.nb_player; ++i)
                                {
                                    if (g_ta3d_network->isRemoteHuman( i ) )
                                        min_tick = Math::Min(min_tick, client_tick[i]);
                                }
                            }
                            else
                            {
                                for (int i = 0; i < players.nb_player; ++i)
                                {
                                    if (g_ta3d_network->isRemoteHuman( i ) && client_tick[i] > 0 )
                                        min_tick = Math::Min(min_tick, client_tick[i]);
                                }
                            }
                            min_tick /= 1000;
                        }
                    }
                    else 
                    {
                        if (current_tick > min_tick )
                            tick += ( current_tick - min_tick ) * 250 / TICKS_PER_SEC;
                    }
                }

                unit_engine_thread_sync = 1;
                while( unit_engine_thread_sync && !thread_ask_to_stop )
                {
                    if (unit_engine_thread_sync && weapon_engine_thread_sync && particle_engine_thread_sync && players_thread_sync ) // Sync engine threads
                    {
                        unit_engine_thread_sync = 0;
                        weapon_engine_thread_sync = 0;
                        particle_engine_thread_sync = 0;
                        players_thread_sync = 0;

                        current_tick++;		// To have a common time value
                        break;
                    }
                    rest( 1 );			// Wait until other thread sync with this one
                }

                ThreadSynchroniser->lock();

                last_tick[ 0 ] = last_tick[ 1 ];
                last_tick[ 1 ] = last_tick[ 2 ];
                last_tick[ 2 ] = last_tick[ 3 ];
                last_tick[ 3 ] = last_tick[ 4 ];
                last_tick[ 4 ] = msec_timer;

                if (last_tick[ 0 ] != 0 && last_tick[4] != last_tick[0] )
                    apparent_timefactor = 4000.0f / ( (last_tick[ 4 ] - last_tick[ 0 ]) * TICKS_PER_SEC );
            }

            ThreadSynchroniser->unlock();
            thread_running = false;
            thread_ask_to_stop = false;
            LOG_INFO("Unit engine: " << (float)(current_tick * 1000) / (msec_timer - unit_timer) << " ticks/sec");

            return 0;
        }

        void INGAME_UNITS::SignalExitThread()
        {
            if (thread_running)
                thread_ask_to_stop = true;
        }


    } // namespace TA3D

