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
#include <misc/paths.h>
#include "sidedata.h"
#include <UnitEngine.h>
#include <engine.h>



namespace TA3D
{

	PLAYERS	players;		// Objet contenant les données sur les joueurs
	int NB_PLAYERS;


	static inline bool NeedSynchronization(struct sync &a, const struct sync &b)
	{
		int mask = 0;
		mask |= (fabsf(a.x - b.x) > 0.001f) ? SYNC_MASK_X : 0;
		mask |= (fabsf(a.y - b.y) > 1.0f) ? SYNC_MASK_Y : 0;
		mask |= (fabsf(a.z - b.z) > 0.001f) ? SYNC_MASK_Z : 0;
		mask |= (fabsf(a.vx - b.vx) > 0.001f) ? SYNC_MASK_VX : 0;
		mask |= (fabsf(a.vz - b.vz) > 0.001f) ? SYNC_MASK_VZ : 0;
		mask |= (a.hp != b.hp) ? SYNC_MASK_HP : 0;
		mask |= (a.orientation != b.orientation) ? SYNC_MASK_O : 0;
		mask |= (a.build_percent_left != b.build_percent_left) ? SYNC_MASK_BPL : 0;
		a.mask = (uint8)mask;
		// Make sure we don't add errors
		if (!(mask & SYNC_MASK_X))	a.x = b.x;
		if (!(mask & SYNC_MASK_Y))	a.y = b.y;
		if (!(mask & SYNC_MASK_Z))	a.z = b.z;
		if (!(mask & SYNC_MASK_VX))	a.vx = b.vx;
		if (!(mask & SYNC_MASK_VZ))	a.vz = b.vz;
		return mask != 0;
	}



	int PLAYERS::add(const QString& name, const QString &SIDE, byte _control, unsigned int E, unsigned int M, const QString &AI_level, uint16 teamMask)
	{
		if (pPlayerCount >= TA3D_PLAYERS_HARD_LIMIT)
		{
			LOG_ERROR("Impossible to add a player : The maximumum limit has been reached");
			return -1;		// Trop de joueurs déjà
		}

		// Noticing logs of a new player
		LOG_INFO("Adding a new player: `" << name << "` (" << pPlayerCount << ") of `" << SIDE
			<< "` with E=" << E << ", M=" << M << " AI=" << AI_level);

		// Initializing the player
		r_metal[pPlayerCount]          = 0.f;
		r_energy[pPlayerCount]         = 0.f;
		requested_metal[pPlayerCount]  = 0.f;
		requested_energy[pPlayerCount] = 0.f;

		metal_u[pPlayerCount]      = 0.f;
		energy_u[pPlayerCount]     = 0.f;
		metal_t[pPlayerCount]      = 0.f;
		energy_t[pPlayerCount]     = 0.f;
		kills[pPlayerCount]        = 0;
		losses[pPlayerCount]       = 0;
        this->name[pPlayerCount]   = name;
		control[pPlayerCount]      = _control;
		nb_unit[pPlayerCount]      = 0;
		energy_total[pPlayerCount] = 0.0f;
		metal_total[pPlayerCount]  = 0.0f;
		com_metal[pPlayerCount]    = M;
		com_energy[pPlayerCount]   = E;
		energy[pPlayerCount]       = float(E);
		metal[pPlayerCount]        = float(M);
		energy_s[pPlayerCount]     = E;
		metal_s[pPlayerCount]      = M;
		if (teamMask == 0)
			team[pPlayerCount]     = uint16(1 << pPlayerCount);       // Try to be your own enemy :P
		else
			team[pPlayerCount]     = teamMask;
		side[pPlayerCount++] = SIDE;

		if (_control == PLAYER_CONTROL_LOCAL_HUMAN)
		{
			local_human_id = NB_PLAYERS;
			for (int i = 0; i < ta3dSideData.nb_side ; ++i)
			{
				if (ToLower(ta3dSideData.side_name[i]) == ToLower(SIDE))
				{
					side_view = uint8(i);
					break;
				}
			}
		}
		if (_control == PLAYER_CONTROL_LOCAL_AI)
		{
            QString filename = "ai/" + name + ".ai";
			if (Paths::Exists(filename)) // Load saved data for AI player
				ai_command[NB_PLAYERS].load(filename, NB_PLAYERS);
			else													// Sinon crée un nouveau joueur
				ai_command[NB_PLAYERS].changeName(name);
			ai_command[NB_PLAYERS].setAI(AI_level);
			ai_command[NB_PLAYERS].setPlayerID(NB_PLAYERS);
		}
		return ++NB_PLAYERS;
	}


	void PLAYERS::show_resources()
	{
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
		glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_COLOR);

		units->lock();

		const int _id = (local_human_id != -1) ? local_human_id : 0;

		const InterfaceData &side_data = ta3dSideData.side_int_data[ players.side_view ];

        gfx->small_font->print((float)side_data.MetalNum.x1,
                               (float)side_data.MetalNum.y1,
                               side_data.metal_color, QString::number((int)metal[_id]));
        gfx->small_font->print((float)side_data.MetalProduced.x1,
                               (float)side_data.MetalProduced.y1,
                               side_data.metal_color, QString::asprintf("%.2f", metal_t[_id]));

        gfx->small_font->print((float)side_data.MetalConsumed.x1,
                               (float)side_data.MetalConsumed.y1,
                               side_data.metal_color, QString::asprintf("%.2f", metal_u[_id]));
        gfx->small_font->print((float)side_data.Metal0.x1,
                               (float)side_data.Metal0.y1,
                               side_data.metal_color,"0");
        gfx->small_font->print_right((float)side_data.MetalMax.x1,
                                     (float)side_data.MetalMax.y1,
                                     side_data.metal_color, QString::number(metal_s[_id]));
        gfx->small_font->print((float)side_data.EnergyNum.x1,
                               (float)side_data.EnergyNum.y1,
                               side_data.energy_color, QString::number((int)energy[_id]));

        gfx->small_font->print((float)side_data.EnergyProduced.x1,
                               (float)side_data.EnergyProduced.y1,
                               side_data.energy_color,
                               QString::asprintf("%.2f", energy_t[_id]));

        gfx->small_font->print((float)side_data.EnergyConsumed.x1,
                               (float)side_data.EnergyConsumed.y1,
                               side_data.energy_color, QString::asprintf("%.2f", energy_u[_id]));
        gfx->small_font->print((float)side_data.Energy0.x1,
                               (float)side_data.Energy0.y1,
                               side_data.energy_color,"0");
        gfx->small_font->print_right((float)side_data.EnergyMax.x1,
                                     (float)side_data.EnergyMax.y1,
                                     side_data.energy_color, QString::number(energy_s[_id]) );

		glDisable(GL_TEXTURE_2D);

		glDisable(GL_BLEND);
		glBegin(GL_QUADS);			// Dessine les barres de metal et d'énergie
		gfx->set_color( side_data.metal_color );

		if (metal_s[_id])
		{
			const float metal_percent = metal_s[_id] ? metal[_id] / float(metal_s[_id]) : 0.0f;
			glVertex2f( (float)side_data.MetalBar.x1,
						(float)side_data.MetalBar.y1 );
			glVertex2f( (float)side_data.MetalBar.x1 + metal_percent * float(side_data.MetalBar.x2 - side_data.MetalBar.x1),
						(float)side_data.MetalBar.y1 );
			glVertex2f( (float)side_data.MetalBar.x1 + metal_percent * float(side_data.MetalBar.x2 - side_data.MetalBar.x1),
						(float)side_data.MetalBar.y2 );
			glVertex2f( (float)side_data.MetalBar.x1,
						(float)side_data.MetalBar.y2 );
		}

		gfx->set_color( side_data.energy_color );
		if (energy_s[_id])
		{
			const float energy_percent = energy_s[_id] ? energy[_id] / float(energy_s[_id]) : 0.0f;
			glVertex2f((float)side_data.EnergyBar.x1,
					   (float)side_data.EnergyBar.y1 );
			glVertex2f((float)side_data.EnergyBar.x1 + energy_percent * float(side_data.EnergyBar.x2 - side_data.EnergyBar.x1),
					   (float)side_data.EnergyBar.y1 );
			glVertex2f((float)side_data.EnergyBar.x1 + energy_percent * float(side_data.EnergyBar.x2 - side_data.EnergyBar.x1),
					   (float)side_data.EnergyBar.y2 );
			glVertex2f((float)side_data.EnergyBar.x1,
					   (float)side_data.EnergyBar.y2 );
		}

		units->unlock();

		glEnd();
		glColor4ub(0xFF,0xFF,0xFF,0xFF);
	}



	void PLAYERS::player_control()
	{
		for (unsigned int i = 0; i < pPlayerCount; ++i)
		{
			if (control[i] == PLAYER_CONTROL_LOCAL_AI && ai_command)
				ai_command[i].monitor();
		}

        if( (units->current_tick % 3) == 0 && last_ticksynced != units->current_tick && network_manager.isConnected())
		{
            last_ticksynced = units->current_tick;

			units->lock();
			SocketTCP::disableFlush();
			for (size_t e = 0; e < units->nb_unit; ++e)
			{
				const size_t i = units->idx_list[e];
				if (i >= units->max_unit)
					continue;		// Error !!
				units->unlock();

				units->unit[i].lock();
				if (!(units->unit[i].flags & 1))
				{
					units->unit[i].unlock();
					units->lock();
					continue;
				}
				if (units->unit[i].local)
				{
					struct sync sync;
                    sync.timestamp = units->current_tick;
					sync.unit = uint16(i);
					sync.flags = 0;
					if (units->unit[i].flying)
						sync.flags |= SYNC_FLAG_FLYING;
					if( units->unit[i].cloaking)
						sync.flags |= SYNC_FLAG_CLOAKING;
					sync.x = units->unit[i].Pos.x;
					sync.y = units->unit[i].Pos.y;
					sync.z = units->unit[i].Pos.z;
					sync.vx = units->unit[i].V.x;
					sync.vz = units->unit[i].V.z;
					float angle = units->unit[i].Angle.y;
					while (angle < 0.0f)
						angle += 360.0f;
					sync.orientation = (uint16)(angle * 65535.0f / 360.0f);
					sync.hp = (uint16)units->unit[ i ].hp;
					sync.build_percent_left = (uint8)(units->unit[ i ].build_percent_left * 2.55f);

                    uint32 latest_sync = units->current_tick;
					for (int f = 0; f < NB_PLAYERS; ++f)
						if (g_ta3d_network->isRemoteHuman(f))
							latest_sync = Math::Min(latest_sync, units->unit[i].last_synctick[f]);

					const bool sync_needed = NeedSynchronization(sync, units->unit[i].previous_sync);

					if (sync_needed)
					{
						// Don't send what isn't needed
						network_manager.sendSync(&sync);
						units->unit[i].previous_sync = sync;
					}
				}
				units->unit[i].unlock();
				units->lock();
			}
			SocketTCP::enableFlush();
			units->unlock();
		}
	}



	void PLAYERS::proc(void*)
	{
		if (thread_is_running)
			return;

		thread_is_running = true;

		last_ticksynced = 9999;

		while (!thread_ask_to_stop)
		{
			player_control();

			/*---------------------- handle Network events ------------------------------*/

			if (ta3d_network)
				ta3d_network->check();

			/*---------------------- end of Network events ------------------------------*/

			Engine::sync();
		}

		thread_is_running = false;
	}

	void PLAYERS::signalExitThread()
	{
		thread_ask_to_stop = true;
		while (thread_is_running)
			suspend(1);
		thread_ask_to_stop = false;
	}



	void PLAYERS::stop_threads()
	{
		for (unsigned int i = 0; i < pPlayerCount; ++i)
		{
			if (control[i] == PLAYER_CONTROL_LOCAL_AI && ai_command)
				ai_command[i].stop();
		}
	}


	void PLAYERS::clear()		// Remet à 0 la taille des stocks
	{
		for (unsigned int i = 0; i < pPlayerCount; ++i)
		{
			r_energy[i]         = requested_energy[i];     // Used to modulate the use of resources and distribute them amoung builders
			r_metal[i]          = requested_metal[i];
			requested_energy[i] = 0;     // Used to modulate the use of resources and distribute them amoung builders
			requested_metal[i]  = 0;
			c_energy[i]         = energy[i];
			c_metal[i]          = metal[i];
			c_energy_s[i]       = c_metal_s[i]=0;         // Stocks
			c_metal_t[i]        = c_energy_t[i] = 0.0f;   // Production
			c_metal_u[i]        = c_energy_u[i] = 0.0f;   // Consommation
			c_commander[i]      = false;
			c_annihilated[i]    = true;
			c_nb_unit[i]        = 0;

			if (r_energy[i] <= energy[i] || Math::Zero(r_energy[i]))
				energy_factor[i] = 1.0f;
			else
				energy_factor[i] = 0.9f * energy[i] / r_energy[i];

			if (r_metal[i] <= metal[i] || Math::Zero(r_metal[i]))
				metal_factor[i] = 1.0f;
			else
				metal_factor[i] = 0.9f * metal[i] / r_metal[i];
		}
	}


	void PLAYERS::refresh()		// Copy the newly computed values over old ones
	{
		for (unsigned int i = 0; i < pPlayerCount; ++i)
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
		pPlayerCount = 0;
		NB_PLAYERS = 0;
		local_human_id = -1;
		clear();
		refresh();
        name.clear();
        side.clear();
        for (int i = 0; i < TA3D_PLAYERS_HARD_LIMIT; ++i)
		{
            side.push_back(QString());
            name.push_back(QString());
			com_metal[i] = M;
			com_energy[i] = E;
			control[i] = PLAYER_CONTROL_NONE;
			if (ai_command)
			{
				ai_command[i].destroy();
				ai_command[i].setPlayerID( i );
			}
			energy[i] = float(E);
			metal[i] = float(M);
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
        name.clear();
		side.clear();
		init();
	}


	PLAYERS::~PLAYERS()
	{
		destroy();
        name.clear();
		side.clear();
		DELETE_ARRAY(ai_command);
		destroyThread();
	}



}
