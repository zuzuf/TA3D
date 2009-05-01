#ifndef __TA3D_ENGINE_MISSION_H__
# define __TA3D_ENGINE_MISSION_H__

# include "../stdafx.h"


# define MISSION_FLAG_CAN_ATTACK		0x01
# define MISSION_FLAG_SEARCH_PATH		0x02
# define MISSION_FLAG_TARGET_WEAPON		0x04
# define MISSION_FLAG_COMMAND_FIRE		0x08
# define MISSION_FLAG_MOVE				0x10
# define MISSION_FLAG_REFRESH_PATH		0x20
# define MISSION_FLAG_DONT_STOP_MOVE	0x40
# define MISSION_FLAG_COMMAND_FIRED		0x80
# define MISSION_FLAG_TARGET_CHECKED	0x08 // For MISSION_CAPTURE to tell when data has been set to the time left before capture is finished
# define MISSION_FLAG_PAD_CHECKED		0x08 // For MISSION_GET_REPAIRED to tell when data has been set to the landing pad
# define MISSION_FLAG_BEING_REPAIRED	0x04 // For MISSION_GET_REPAIRED to tell the unit is being repaired


# define MISSION_STANDBY        0x00        // Aucune mission
# define MISSION_VTOL_STANDBY   0x01
# define MISSION_GUARD_NOMOVE   0x02        // Patrouille immobile
# define MISSION_MOVE           0x03        // Déplacement de l'unité
# define MISSION_BUILD          0x04        // Création d'une unité
# define MISSION_BUILD_2        0x05        // Construction d'une unité
# define MISSION_STOP           0x06        // Arrêt des opérations en cours
# define MISSION_REPAIR         0x07        // Réparation d'une unité
# define MISSION_ATTACK         0x08        // Attaque une unité
# define MISSION_PATROL         0x09        // Patrouille
# define MISSION_GUARD          0x0A        // Surveille une unité
# define MISSION_RECLAIM        0x0B        // Récupère une unité/un cadavre
# define MISSION_LOAD           0x0C        // Load other units
# define MISSION_UNLOAD         0x0D        // Unload other units
# define MISSION_STANDBY_MINE   0x0E        // Mine mission, must explode when an enemy gets too close
# define MISSION_REVIVE         0x0F        // Resurrect a wreckage
# define MISSION_CAPTURE        0x10        // Capture an enemy unit
# define MISSION_GET_REPAIRED   0x20        // For aircrafts getting repaired by air repair pads

// Specific campaign missions
# define MISSION_WAIT           0x21        // Wait for a specified time
# define MISSION_WAIT_ATTACKED  0x22        // Wait until a specified unit is attacked

# define MISSION_FLAG_AUTO      0x10000     // Mission is sent from UNIT::move so don't ignore it



namespace TA3D
{


	/*!
	** \brief Mission for any Unit
	*/
	class Mission
	{
	public:
		//! \name Constructors & Destructor
		//@{
		//! Default constructor
		Mission();
		//! Copy constructor
		Mission(const Mission& rhs);
		//! Destructor
		~Mission();
		//@}

		//! Operator =
		Mission& operator = (const Mission& rhs);

	public:
		float		time;		// Temps écoulé depuis la création de l'ordre
		float		last_d;		// Dernière distance enregistrée
		int			data;		// Données de l'ordre
		int			move_data;	// Required data for the moving part of the order
		Mission 	*next;		// Mission suivante
		uint8		mission;
		PATH_NODE	*path;		// Chemin emprunté par l'unité si besoin pour la mission
		Vector3D	target;
		bool		step;		// Etape d'une mission
		byte		flags;		// Données supplémentaires
		void		*p;			// Pointer to whatever we need
		uint32		target_ID;	// Identify a target unit
		uint16		node;		// Tell which patrol node is this mission

	}; // class Mission




} // namespace TA3D

# include "mission.hxx"

#endif // __TA3D_ENGINE_MISSION_H__
