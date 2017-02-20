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
#ifndef __TA3D_INGAME_PLAYERS_H__
# define __TA3D_INGAME_PLAYERS_H__

# include <stdafx.h>
# include <misc/string.h>
# include <threads/thread.h>
# include <ai/ai.h>
# include <network/TA3D_Network.h>



# define PLAYER_CONTROL_LOCAL_HUMAN     0x0
# define PLAYER_CONTROL_REMOTE_HUMAN    0x1
# define PLAYER_CONTROL_LOCAL_AI        0x2
# define PLAYER_CONTROL_REMOTE_AI       0x3
# define PLAYER_CONTROL_NONE            0x4
# define PLAYER_CONTROL_CLOSED          0x8

# define PLAYER_CONTROL_FLAG_REMOTE     0x1
# define PLAYER_CONTROL_FLAG_AI	        0x2



namespace TA3D
{


	class SinglePlayer
	{
	public:
		SinglePlayer() {}
		~SinglePlayer() {}


	};




	class PLAYERS : public Thread
	{
		friend class INGAME_UNITS;
	public:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor
		PLAYERS();
		//! Destructor
		virtual ~PLAYERS();
		//@}

		/*!
		** \brief
		*/
		void set_network(const TA3DNetwork::Ptr &net) { ta3d_network = net;}

		/*!
		** \brief
		*/
		void player_control();

		/*!
		** \brief
		*/
		void stop_threads();

		/*!
		** \brief
		*/
		void clear(); // Remet à 0 la taille des stocks

		/*!
		** \brief
		*/
		void refresh(); // Copy the newly computed values over old ones

		/*!
		** \brief
		*/
		int add(const QString& name, const QString &SIDE, byte _control, unsigned int E = 10000, unsigned int M = 10000,
			const QString &AI_level = "[C] EASY", uint16 teamMask = 0);

		/*!
		** \brief
		*/
		void show_resources();

		/*!
		** \brief
		*/
		void init(int E = 10000, int M = 10000);		// Initialise les données des joueurs


		/*!
		** \brief
		*/
		void destroy();

	public:
		/*!
		** \brief Player count
		*/
		unsigned int count() const;

	public:
		//!
		int			    local_human_id;	// Quel est le joueur qui commande depuis cette machine??
		//!
		byte		    control[TA3D_PLAYERS_HARD_LIMIT];	// Qui controle ce joueur??
		//!
        QStringList	name;    		// Noms des joueurs
		//!
		QStringList	side;   		// Camp des joueurs
		//!
		float		    r_energy[TA3D_PLAYERS_HARD_LIMIT];		// Energy required
		//!
		float		    r_metal[TA3D_PLAYERS_HARD_LIMIT];		// Metal required
		//!
		float		    energy[TA3D_PLAYERS_HARD_LIMIT];		// Energie des joueurs
		//!
		float		    metal[TA3D_PLAYERS_HARD_LIMIT];		// Metal des joueurs
		//!
		float		    metal_u[TA3D_PLAYERS_HARD_LIMIT];	// Metal utilisé
		//!
		float		    energy_u[TA3D_PLAYERS_HARD_LIMIT];	// Energie utilisée
		//!
		float		    metal_t[TA3D_PLAYERS_HARD_LIMIT];	// Metal extrait
		//!
		float		    energy_t[TA3D_PLAYERS_HARD_LIMIT];	// Energie produite
		//!
		uint32		    kills[TA3D_PLAYERS_HARD_LIMIT];		// Victimes
		//!
		uint32		    losses[TA3D_PLAYERS_HARD_LIMIT];		// Pertes
		//!
		uint32		    energy_s[TA3D_PLAYERS_HARD_LIMIT];	// Capacités de stockage d'énergie
		//!
		uint32		    metal_s[TA3D_PLAYERS_HARD_LIMIT];	// Capacités de stockage de metal
		//!
		uint32		    com_metal[TA3D_PLAYERS_HARD_LIMIT];	// Stockage fournit par le commandeur
		//!
		uint32		    com_energy[TA3D_PLAYERS_HARD_LIMIT];
		//!
		bool		    commander[TA3D_PLAYERS_HARD_LIMIT];	// Indique s'il y a un commandeur
		//!
		bool		    annihilated[TA3D_PLAYERS_HARD_LIMIT];// Le joueur a perdu la partie??
		//!
		AI_PLAYER	    *ai_command;	// Controleurs d'intelligence artificielle
		//!
		uint32		    nb_unit[TA3D_PLAYERS_HARD_LIMIT];	// Nombre d'unités de chaque joueur
		//! Side of which we draw the game interface
		uint8		    side_view;
		//! Team array, we use uint16 with masking so a player could belong to several "teams" with some advanced team management (for later)
		uint16          team[TA3D_PLAYERS_HARD_LIMIT];

		//		Variables used to compute the data we need ( because of threading )

		//!
		float		    energy_factor[TA3D_PLAYERS_HARD_LIMIT];      // Energy proportion we can use
		//!
		float		    metal_factor[TA3D_PLAYERS_HARD_LIMIT];		 // Metal proportion we can use
		//!
		float		    requested_energy[TA3D_PLAYERS_HARD_LIMIT];      // Energy required
		//!
		float		    requested_metal[TA3D_PLAYERS_HARD_LIMIT];		// Metal required

		//!
		float		    c_energy[TA3D_PLAYERS_HARD_LIMIT];		// Energie des joueurs
		//!
		float		    c_metal[TA3D_PLAYERS_HARD_LIMIT];		// Metal des joueurs
		//!
		uint32		    c_energy_s[TA3D_PLAYERS_HARD_LIMIT];		// Capacités de stockage d'énergie
		//!
		uint32		    c_metal_s[TA3D_PLAYERS_HARD_LIMIT];		// Capacités de stockage de metal
		//!
		bool		    c_commander[TA3D_PLAYERS_HARD_LIMIT];	// Indique s'il y a un commandeur
		//!
		bool		    c_annihilated[TA3D_PLAYERS_HARD_LIMIT];	// Le joueur a perdu la partie??
		//!
		uint32		    c_nb_unit[TA3D_PLAYERS_HARD_LIMIT];		// Nombre d'unités de chaque joueur
		//!
		float		    c_metal_u[TA3D_PLAYERS_HARD_LIMIT];		// Metal utilisé
		//!
		float		    c_energy_u[TA3D_PLAYERS_HARD_LIMIT];		// Energie utilisée
		//!
		float		    c_metal_t[TA3D_PLAYERS_HARD_LIMIT];		// Metal extrait
		//!
		float		    c_energy_t[TA3D_PLAYERS_HARD_LIMIT];		// Energie produite

		// For statistic purpose only
		//!
		double		    energy_total[TA3D_PLAYERS_HARD_LIMIT];
		//!
		double		    metal_total[TA3D_PLAYERS_HARD_LIMIT];

	protected:
		//!
		void proc(void*);
		//!
		void signalExitThread();

	protected:
		//!
		uint32 last_ticksynced;
		//!
		TA3DNetwork::Ptr ta3d_network;
		//!
		volatile bool thread_is_running;
		//!
		volatile bool thread_ask_to_stop;

	private:
		//! Player count (hard limit: TA3D_PLAYERS_HARD_LIMIT)
		unsigned int pPlayerCount;

	}; // class PLAYERS


	// TODO SHould be removed
	extern int NB_PLAYERS;
	extern PLAYERS	players;		// Objet contenant les données sur les joueurs

} // namespace TA3D

# include "players.hxx"

#endif // __TA3D_INGAME_PLAYERS_H__
