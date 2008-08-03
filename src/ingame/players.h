#ifndef __TA3D_INGAME_PLAYERS_H__
# define __TA3D_INGAME_PLAYERS_H__

# include "../stdafx.h"
# include "../threads/cThread.h"
# include "../ai/ai.h"
# include "../network/TA3D_Network.h"



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

    class PLAYERS : public cThread
    {
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
        void set_network(TA3DNetwork *net) { ta3d_network = net;}

        /*!
        ** \brief
        */
        void set_map(MAP *p_map) {map = p_map;}

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
        int add(const String& name, char *SIDE, byte _control, int E = 10000, int M = 10000, byte AI_level = AI_TYPE_EASY);

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
        //!
        sint8		    nb_player;		// Nombre de joueurs (maximum TA3D_PLAYERS_HARD_LIMIT joueurs)
        //!
        int			    local_human_id;	// Quel est le joueur qui commande depuis cette machine??
        //!
        byte		    control[TA3D_PLAYERS_HARD_LIMIT];	// Qui controle ce joueur??
        //!
        String::Vector	nom;    		// Noms des joueurs
        //!
        String::Vector	side;   		// Camp des joueurs
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

        //		Variables used to compute the data we need ( because of threading )

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
        virtual int Run();
        //!
        virtual void SignalExitThread();

    protected:
        //!
        uint32 last_ticksynced;
        //!
        TA3DNetwork* ta3d_network;
        //!
        MAP* map;
        //!
        bool thread_is_running;
        //!
        bool thread_ask_to_stop;

    }; // class PLAYERS


    // TODO SHould be removed
    extern int NB_PLAYERS;
    extern PLAYERS	players;		// Objet contenant les données sur les joueurs

} // namespace TA3D


#endif // __TA3D_INGAME_PLAYERS_H__
