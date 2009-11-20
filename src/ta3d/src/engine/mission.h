#ifndef __TA3D_ENGINE_MISSION_H__
# define __TA3D_ENGINE_MISSION_H__

# include <stdafx.h>
# include <ai/pathfinding.h>
# include <misc/stack.hxx>
# include <zlib.h>

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
	class Unit;
	class Weapon;
	class Vector3D;


	/*!
	** \brief Mission for any Unit
	*/
	class Mission
	{
	public:
		/*!
		** \brief Defines a target (none, unit, weapon)
		*/
		class Target
		{
		public:
			//! Target types
			enum Type { TargetNone, TargetUnit, TargetWeapon, TargetStatic };
		public:
			//! Constructors
			Target() : type(TargetNone), idx(-1), UID(0), Pos()	{}
			Target(Type type, int idx = -1, uint32 UID = 0) : type(type), idx(idx), UID(UID), Pos()
			{}
			Target(const Vector3D &Pos) : type(TargetNone), idx(-1), UID(0), Pos(Pos)
			{}

			//! Functions to access class members
			Type getType() const	{	return type;	}
			int getIdx() const	{	return idx;	}
			uint32 getUID() const	{	return UID;	}
			bool isUnit() const;
			bool isWeapon() const;
			bool isStatic() const	{	return type == TargetStatic;	}
			bool isNone() const	{	return !isUnit() && !isWeapon() && !isStatic();	}

			//! Interface to set the target
			void set(Type type, int idx = -1, uint32 UID = 0)
			{
				this->type = type;
				this->idx = idx;
				this->UID = UID;
			}

			void setPos(const Vector3D &Pos)
			{
				if (type == TargetStatic)
					this->Pos = Pos;
			}

			//! Functions to get a pointer to the target (checks target type)
			Unit *getUnit() const;
			Weapon *getWeapon() const;
			const Vector3D &getPos() const;

			void save(gzFile file) const;
			void load(gzFile file);

		private:
			Type		type;
			int			idx;
			uint32		UID;
			Vector3D	Pos;
		};

		/*!
		** \brief Mission step
		*/
		class MissionStep
		{
		public:
			MissionStep() : type(MISSION_STANDBY), target(), flags(0), data(0)	{}

			uint8	getType() const	{	return type;	}
			Target	&getTarget()	{	return target;	}
			byte	getFlags() const {	return flags;	}
			byte	&Flags()		{	return flags;	}
			int		getData() const {	return data;	}

			void	setType(uint8 type)	{	this->type = type;	}
			void	setFlags(byte flags)	{	this->flags = flags;	}
			void	setData(int data)	{	this->data = data;	}

			void	save(gzFile file) const;
			void	load(gzFile file);
		private:
			uint8	type;
			Target	target;
			byte	flags;
			int		data;
		};

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

		bool empty()	{	return qStep.empty();	}
		int size()		{	return qStep.size();	}
		void addStep()	{	qStep.push(MissionStep());	}

		void next()
		{
			if (!empty())
				qStep.pop();
		}

		uint8 operator[](int n)
		{
			if (n < 0 || n >= size())	return 0;
			return qStep[n].getType();
		}

		bool hasNext()		{	return qStep.size() > 1;	}

		MissionStep &lastStep()	{	return qStep.bottom();	}
		uint8 lastMission() const	{	return qStep.bottom().getType();	}
		uint8 mission() const	{	return qStep.top().getType();	}
		uint8 getMissionType() const	{	return qStep.top().getType();	}
		float getTime() const	{	return time;	}
		float getLastD() const	{	return last_d;	}
		int getData() const		{	return qStep.top().getData();	}
		int getMoveData() const	{	return move_data;	}
		const AI::Path &getPath()	const	{	return path;	}
		AI::Path &Path()		{	return path;	}
		Target &getTarget()		{	return qStep.top().getTarget();	}
		bool isStep() const		{	return qStep.size() != 1;	}
		byte getFlags() const	{	return qStep.top().getFlags();	}
		byte& Flags()			{	return qStep.top().Flags();	}
		uint16 getNode() const	{	return node;	}
		Unit *getUnit()			{	return getTarget().getUnit();	}
		Weapon *getWeapon()		{	return getTarget().getWeapon();	}

		void setMissionType(uint8 type)	{	qStep.top().setType(type);	}
		void setTime(float time)	{	this->time = time;	}
		void setLastD(float last_d)	{	this->last_d = last_d;	}
		void setData(int data)		{	qStep.top().setData(data);	}
		void setMoveData(int move_data)	{	this->move_data = move_data;	}
		void setFlags(byte flags)		{	qStep.top().setFlags(flags);	}
		void setNode(uint16 node)	{	this->node = node;	}

		void save(gzFile file);
		void load(gzFile file);

	private:
		Stack<MissionStep> qStep;	// Stack of mission steps
		float		time;		// Temps écoulé depuis la création de l'ordre
		float		last_d;		// Dernière distance enregistrée
//		int			data;		// Données de l'ordre
		int			move_data;	// Required data for the moving part of the order
//		Mission 	*next;		// Mission suivante
//		uint8		mission;
		AI::Path    path;		// Chemin emprunté par l'unité si besoin pour la mission
//		byte		flags;		// Données supplémentaires
//		void		*p;			// Pointer to whatever we need
//		uint32		target_ID;	// Identify a target unit
		uint16		node;		// Tell which patrol node is this mission

	}; // class Mission


	class MissionStack
	{
	public:
		typedef std::list<Mission> Container;
		typedef Container::iterator iterator;
	public:
		bool empty() const	{	return sMission.empty();	}
		bool operator!() const	{	return sMission.empty();	}
		int size() const	{	return int(sMission.size());	}

		iterator begin()	{	return sMission.begin();	}
		iterator end()	{	return sMission.end();	}
		iterator erase(iterator &it)	{	return sMission.erase(it);	}
		void clear()	{	sMission.clear();	}
		void add()		{	sMission.push_back(Mission()); front().addStep();	}
		void add(const Mission &mission)		{	sMission.push_back(mission);	}
		void insert(const iterator &it, const Mission &mission)
		{	sMission.insert(it, mission);	}

		uint8 mission() const	{	return empty() ? 0 : sMission.front().getMissionType();	}

		Mission& front()
		{
			return sMission.front();
		}

		Mission& back()
		{
			return sMission.back();
		}

		Mission* operator->()
		{
			return &(sMission.front());
		}

		Mission& operator[](int n)
		{
			iterator i = begin();
			for(;n > 0 && i != end() ; --n)
				++i;
			return *i;
		}

		uint8 operator()(int n)
		{
			iterator i = begin();
			for(;n > 0 && i != end() ; --n, ++i)
				if (i->size() > n)
					return (*i)[n];
			return 0;
		}

		bool hasNext()
		{
			return !sMission.empty() && (sMission.front().size() > 1 || sMission.size() > 1);
		}

		void next()
		{
			if (sMission.empty())
				return;
			sMission.front().next();
			if (sMission.front().empty())
			{
				sMission.pop_front();
				return;
			}
		}

		bool doNothing();
		bool doNothingAI();

		void save(gzFile file);
		void load(gzFile file);
	private:
		Container	sMission;
	};


} // namespace TA3D

# include "mission.hxx"

#endif // __TA3D_ENGINE_MISSION_H__
