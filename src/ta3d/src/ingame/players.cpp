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
#include "players.h"
#include "../misc/paths.h"
#include "sidedata.h"
#include "../UnitEngine.h"




namespace TA3D
{

    PLAYERS	players;		// Objet contenant les données sur les joueurs
    int NB_PLAYERS;

    namespace
    {

        inline bool need_sync(const struct sync &a, const struct sync &b)
        {
            return fabsf( a.x - b.x ) > 0.001f
                ||	fabsf( a.y - b.y ) > 1.0f
                ||	fabsf( a.z - b.z ) > 0.001f
                ||	fabsf( a.vx - b.vx ) > 0.001f
                ||	fabsf( a.vz - b.vz ) > 0.001f
                ||	a.hp != b.hp
                ||	a.orientation != b.orientation
                ||	a.build_percent_left != b.build_percent_left;
        }

    }



    int PLAYERS::add(const String& name, char* SIDE, byte _control, int E, int M, const String &AI_level, uint16 teamMask)
    {
        if (nb_player >= TA3D_PLAYERS_HARD_LIMIT)
        {
            LOG_ERROR("Impossible to add a player : The maximumum limit has been reached");
            return -1;		// Trop de joueurs déjà
        }

        LOG_INFO("Adding a new player: `" << name << "` (" << (int)nb_player << ") of `" << SIDE
                 << "` with E=" << E << ", M=" << M << " AI=" << AI_level);

        r_metal[nb_player]          = 0;
        r_energy[nb_player]         = 0;
        requested_metal[nb_player]  = 0;
        requested_energy[nb_player] = 0;

        metal_u[nb_player]      = 0;
        energy_u[nb_player]     = 0;
        metal_t[nb_player]      = 0;
        energy_t[nb_player]     = 0;
        kills[nb_player]        = 0;
        losses[nb_player]       = 0;
        nom[nb_player]          = name;
        control[nb_player]      = _control;
        nb_unit[nb_player]      = 0;
        energy_total[nb_player] = 0.0f;
        metal_total[nb_player]  = 0.0f;
        com_metal[nb_player]    = M;
        com_energy[nb_player]   = E;
        energy[nb_player]       = E;
        metal[nb_player]        = M;
        energy_s[nb_player]     = E;
        metal_s[nb_player]      = M;
        if (teamMask == 0)
            team[nb_player]     = 1 << nb_player;       // Try to be your own enemy :P
        else
            team[nb_player]     = teamMask;
        side[nb_player++] = SIDE;

        if (_control == PLAYER_CONTROL_LOCAL_HUMAN)
        {
            local_human_id = NB_PLAYERS;
            for (int i = 0; i < ta3dSideData.nb_side ; ++i)
            {
                if (String::ToLower(ta3dSideData.side_name[i]) == String::ToLower(SIDE))
                {
                    side_view = i;
                    break;
                }
            }
        }
        if (_control == PLAYER_CONTROL_LOCAL_AI)
        {
            String filename;
            filename << "ai/" << name << ".ai";
            if (Paths::Exists(filename)) // Load saved data for AI player
                ai_command[NB_PLAYERS].load(filename, NB_PLAYERS);
            else													// Sinon crée un nouveau joueur
                ai_command[NB_PLAYERS].changeName(name);
            ai_command[NB_PLAYERS].setAI(AI_level);
            ai_command[NB_PLAYERS].setPlayerID( NB_PLAYERS );
        }
        return NB_PLAYERS++;
    }


    void PLAYERS::show_resources()
    {
        units.lock();

        int _id = (local_human_id != -1) ? local_human_id : 0;

        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
        glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_COLOR);

        gfx->print(gfx->small_font,ta3dSideData.side_int_data[ players.side_view ].MetalNum.x1,ta3dSideData.side_int_data[ players.side_view ].MetalNum.y1,0.0f,ta3dSideData.side_int_data[ players.side_view ].metal_color,format("%d",(int)metal[_id]));
        gfx->print(gfx->small_font,ta3dSideData.side_int_data[ players.side_view ].MetalProduced.x1,ta3dSideData.side_int_data[ players.side_view ].MetalProduced.y1,0.0f,ta3dSideData.side_int_data[ players.side_view ].metal_color,format("%.2f", metal_t[_id]));

        gfx->print(gfx->small_font,ta3dSideData.side_int_data[ players.side_view ].MetalConsumed.x1,ta3dSideData.side_int_data[ players.side_view ].MetalConsumed.y1,0.0f,ta3dSideData.side_int_data[ players.side_view ].metal_color,format("%.2f", metal_u[_id]));
        gfx->print(gfx->small_font,ta3dSideData.side_int_data[ players.side_view ].Metal0.x1,ta3dSideData.side_int_data[ players.side_view ].Metal0.y1,0.0f,ta3dSideData.side_int_data[ players.side_view ].metal_color,"0");
        gfx->print_right(gfx->small_font,ta3dSideData.side_int_data[ players.side_view ].MetalMax.x1,ta3dSideData.side_int_data[ players.side_view ].MetalMax.y1,0.0f,ta3dSideData.side_int_data[ players.side_view ].metal_color, format( "%d", metal_s[_id] ) );
        gfx->print(gfx->small_font,ta3dSideData.side_int_data[ players.side_view ].EnergyNum.x1,ta3dSideData.side_int_data[ players.side_view ].EnergyNum.y1,0.0f,ta3dSideData.side_int_data[ players.side_view ].energy_color,format("%d",(int)energy[_id]));

        gfx->print(gfx->small_font,ta3dSideData.side_int_data[players.side_view].EnergyProduced.x1,
                   ta3dSideData.side_int_data[players.side_view].EnergyProduced.y1, 0.0f,
                   ta3dSideData.side_int_data[players.side_view ].energy_color,
                   format("%.2f", energy_t[_id]));

        gfx->print(gfx->small_font,ta3dSideData.side_int_data[ players.side_view ].EnergyConsumed.x1,ta3dSideData.side_int_data[ players.side_view ].EnergyConsumed.y1,0.0f,ta3dSideData.side_int_data[ players.side_view ].energy_color,format("%.2f", energy_u[_id]));
        gfx->print(gfx->small_font,ta3dSideData.side_int_data[ players.side_view ].Energy0.x1,ta3dSideData.side_int_data[ players.side_view ].Energy0.y1,0.0f,ta3dSideData.side_int_data[ players.side_view ].energy_color,"0");
        gfx->print_right(gfx->small_font,ta3dSideData.side_int_data[ players.side_view ].EnergyMax.x1,ta3dSideData.side_int_data[ players.side_view ].EnergyMax.y1,0.0f,ta3dSideData.side_int_data[ players.side_view ].energy_color, format( "%d", energy_s[_id] ) );

        glDisable(GL_TEXTURE_2D);

        glDisable(GL_BLEND);
        glBegin(GL_QUADS);			// Dessine les barres de metal et d'énergie
        gfx->set_color( ta3dSideData.side_int_data[ players.side_view ].metal_color );

        if (metal_s[0])
        {
            float metal_percent = metal_s[_id] ? metal[_id] / metal_s[_id] : 0.0f;
            glVertex2f( ta3dSideData.side_int_data[ players.side_view ].MetalBar.x1, ta3dSideData.side_int_data[ players.side_view ].MetalBar.y1 );
            glVertex2f( ta3dSideData.side_int_data[ players.side_view ].MetalBar.x1 + metal_percent * (ta3dSideData.side_int_data[ players.side_view ].MetalBar.x2-ta3dSideData.side_int_data[ players.side_view ].MetalBar.x1), ta3dSideData.side_int_data[ players.side_view ].MetalBar.y1 );
            glVertex2f( ta3dSideData.side_int_data[ players.side_view ].MetalBar.x1 + metal_percent * (ta3dSideData.side_int_data[ players.side_view ].MetalBar.x2-ta3dSideData.side_int_data[ players.side_view ].MetalBar.x1), ta3dSideData.side_int_data[ players.side_view ].MetalBar.y2 );
            glVertex2f( ta3dSideData.side_int_data[ players.side_view ].MetalBar.x1, ta3dSideData.side_int_data[ players.side_view ].MetalBar.y2 );
        }

        gfx->set_color( ta3dSideData.side_int_data[ players.side_view ].energy_color );
        if (energy_s[0])
        {
            float energy_percent = energy_s[_id] ? energy[_id] / energy_s[_id] : 0.0f;
            glVertex2f(ta3dSideData.side_int_data[players.side_view].EnergyBar.x1, ta3dSideData.side_int_data[ players.side_view ].EnergyBar.y1 );
            glVertex2f(ta3dSideData.side_int_data[players.side_view].EnergyBar.x1 + energy_percent * (ta3dSideData.side_int_data[ players.side_view ].EnergyBar.x2-ta3dSideData.side_int_data[ players.side_view ].EnergyBar.x1), ta3dSideData.side_int_data[ players.side_view ].EnergyBar.y1 );
            glVertex2f(ta3dSideData.side_int_data[players.side_view].EnergyBar.x1 + energy_percent * (ta3dSideData.side_int_data[ players.side_view ].EnergyBar.x2-ta3dSideData.side_int_data[ players.side_view ].EnergyBar.x1), ta3dSideData.side_int_data[ players.side_view ].EnergyBar.y2 );
            glVertex2f(ta3dSideData.side_int_data[players.side_view].EnergyBar.x1, ta3dSideData.side_int_data[ players.side_view ].EnergyBar.y2 );
        }
        glEnd();
        glColor4f(1.0f,1.0f,1.0f,1.0f);

        units.unlock();
    }



    void PLAYERS::player_control()
    {
        for (short int i = 0; i < nb_player; ++i)
        {
            if (control[i] == PLAYER_CONTROL_LOCAL_AI && ai_command)
                ai_command[i].monitor();
        }

        if( (units.current_tick % 3) == 0 && last_ticksynced != units.current_tick && network_manager.isConnected())
        {
            last_ticksynced = units.current_tick;

            units.lock();
            for (int e = 0; e < units.nb_unit; ++e)
            {
                int i = units.idx_list[ e ];
                if (i < 0 || i >= units.max_unit)
                    continue;		// Error !!
                units.unlock();

                units.unit[i].lock();
                if (!(units.unit[i].flags & 1))
                {
                    units.unit[i].unlock();
                    units.lock();
                    continue;
                }
                if (units.unit[i].local)
                {
                    struct sync sync;
                    sync.timestamp = units.current_tick;
                    sync.unit = i;
                    sync.flags = 0;
                    if (units.unit[i].flying)
                        sync.flags |= SYNC_FLAG_FLYING;
                    if( units.unit[i].cloaking)
                        sync.flags |= SYNC_FLAG_CLOAKING;
                    sync.x = units.unit[i].Pos.x;
                    sync.y = units.unit[i].Pos.y;
                    sync.z = units.unit[i].Pos.z;
                    sync.vx = units.unit[i].V.x;
                    sync.vz = units.unit[i].V.z;
                    float angle = units.unit[i].Angle.y;
                    while (angle < 0.0f)
                        angle += 360.0f;
                    sync.orientation = (uint16)(angle * 65535.0f / 360.0f);
                    sync.hp = (uint16)units.unit[ i ].hp;
                    sync.build_percent_left = (uint8)(units.unit[ i ].build_percent_left * 2.55f);

                    uint32 latest_sync = units.current_tick;
                    for (int f = 0; f < NB_PLAYERS; ++f)
                        if (g_ta3d_network->isRemoteHuman(f))
                            latest_sync = Math::Min(latest_sync, units.unit[i].last_synctick[f]);

                    bool sync_needed = need_sync(sync, units.unit[i].previous_sync);

                    if (sync_needed)
                    {			// Don't send what isn't needed
                        network_manager.sendSync( &sync );
                        units.unit[i].previous_sync = sync;
                    }
                }
                units.unit[i].unlock();
                units.lock();
            }
            units.unlock();
        }
    }



    void PLAYERS::proc(void*)
    {
        if (thread_is_running)
            return;

        thread_is_running = true;

        players_thread_sync = 0;
        last_ticksynced = 9999;

        while (!thread_ask_to_stop)
        {
            players.player_control();

            /*---------------------- handle Network events ------------------------------*/

            if (ta3d_network)
                ta3d_network->check();

            /*---------------------- end of Network events ------------------------------*/

            ThreadSynchroniser->lock();
            ThreadSynchroniser->unlock();

            players_thread_sync = 1;
            uint32 check_timer = msec_timer;

            while (players_thread_sync && !thread_ask_to_stop)
            {
                rest(1); // Wait until other thread sync with this one
                if (msec_timer - check_timer > 100 && ta3d_network)
                {
                    ta3d_network->check();
                    check_timer = msec_timer;
                }
            }
        }

        thread_is_running = false;
    }

    void PLAYERS::signalExitThread()
    {
        thread_ask_to_stop = true;
        while (thread_is_running)
            rest(1);
        thread_ask_to_stop = false;
    }



    void PLAYERS::stop_threads()
    {
        for (byte i = 0; i < nb_player; ++i)
        {
            if (control[i] == PLAYER_CONTROL_LOCAL_AI && ai_command)
                ai_command[i].stop();
        }
    }


    void PLAYERS::clear()		// Remet à 0 la taille des stocks
    {
        for (short int i = 0; i < nb_player; ++i)
        {
            r_energy[i]         = requested_energy[i];     // Used to modulate the use of resources and distribute them amoung builders
            r_metal[i]          = requested_metal[i];
            requested_energy[i] = 0;     // Used to modulate the use of resources and distribute them amoung builders
            requested_metal[i]  = 0;
            c_energy[i]         = energy[i];
            c_metal[i]          = metal[i];
            c_energy_s[i]       = c_metal_s[i]=0;			// Stocks
            c_metal_t[i]        = c_energy_t[i]=0.0f;		// Production
            c_metal_u[i]        = c_energy_u[i]=0.0f;		// Consommation
            c_commander[i]      = false;
            c_annihilated[i]    = true;
            c_nb_unit[i]        = 0;

            if (r_energy[i] <= energy[i] || r_energy[i] == 0.0f)
                energy_factor[i] = 1.0f;
            else
                energy_factor[i] = 0.9f * energy[i] / r_energy[i];

            if (r_metal[i] <= metal[i] || r_metal[i] == 0.0f)
                metal_factor[i] = 1.0f;
            else
                metal_factor[i] = 0.9f * metal[i] / r_metal[i];
        }
    }


    void PLAYERS::refresh()		// Copy the newly computed values over old ones
    {
        for (byte i = 0; i < nb_player; ++i)
        {
            energy[i]      = c_energy[i];
            metal[i]       = c_metal[i];
            energy_s[i]    = c_energy_s[i];
            metal_s[i]     = c_metal_s[i];				// Stocks
            commander[i]   = c_commander[i];
            annihilated[i] = c_annihilated[i];
            nb_unit[i]     = c_nb_unit[i];
            metal_t[i]     = c_metal_t[i];
            energy_t[i]    = c_energy_t[i];
            metal_u[i]     = c_metal_u[i];
            energy_u[i]    = c_energy_u[i];
        }
    }


    void PLAYERS::init(int E,int M) 	// Initialise les données des joueurs
    {
        ta3d_network = NULL;
        side_view = 0;
        nb_player = 0;
        NB_PLAYERS = 0;
        local_human_id = -1;
        clear();
        refresh();
        nom.resize(TA3D_PLAYERS_HARD_LIMIT);
        side.resize(TA3D_PLAYERS_HARD_LIMIT);
        for (short int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
        {
            com_metal[i] = M;
            com_energy[i] = E;
            control[i] = PLAYER_CONTROL_NONE;
            if (ai_command)
            {
                ai_command[i].destroy();
                ai_command[i].setPlayerID( i );
            }
            energy[i] = E;
            metal[i] = M;
            metal_u[i] = 0;
            energy_u[i] = 0;
            metal_t[i] = 0;
            energy_t[i] = 0;
            kills[i] = 0;
            losses[i] = 0;
            energy_s[i] = E;
            metal_s[i] = M;
            nb_unit[i] = 0;
            energy_total[i] = 0.0f;
            metal_total[i] = 0.0f;

            c_energy[i]      = energy[i];
            c_metal[i]       = metal[i];
            c_energy_s[i]    = c_metal_s[i] = 0;     // Stocks
            c_metal_t[i]     = c_energy_t[i] = 0.0f; // Production
            c_metal_u[i]     = c_energy_u[i] = 0.0f; // Consommation
            c_commander[i]   = false;
            c_annihilated[i] = true;
            c_nb_unit[i]     = 0;
        }
    }



    PLAYERS::PLAYERS()
    {
        thread_is_running = false;
        thread_ask_to_stop = false;

        ai_command = new AI_PLAYER[TA3D_PLAYERS_HARD_LIMIT];
        init();
    }


    void PLAYERS::destroy()
    {
        for (short int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
        {
            if (control[i] == PLAYER_CONTROL_LOCAL_AI && ai_command) // Enregistre les données de l'IA
                ai_command[i].save();
            if (ai_command)
                ai_command[i].destroy();
        }
        nom.clear();
        side.clear();
        init();
    }


    PLAYERS::~PLAYERS()
    {
        destroy();
        nom.clear();
        side.clear();
        if (ai_command)
            delete[] ai_command;
        ai_command = NULL;
        destroyThread();
    }



}
