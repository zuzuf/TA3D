
#include "unit.h"
#include <UnitEngine.h>
#include <ingame/players.h>
#include <gfx/fx.manager.h>
#include <sounds/manager.h>
#include <input/mouse.h>



template<class T>
    inline T SQUARE(T X)   { return ((X)*(X)); }



namespace TA3D
{



	Unit::Unit()
		:script(NULL), render(), model(NULL), owner_id(0), type_id(0),
		hp(0.), Pos(), V(), Angle(), V_Angle(), sel(false),
		data(), drawing(false), port(NULL), mission(), def_mission(),
		flags(0), kills(0), selfmove(false), lastEnergy(0.0f), c_time(0), compute_coord(false), idx(0), ID(0),
		h(0.), visible(false), on_radar(false), on_mini_radar(false), groupe(0),
		built(0), attacked(false), planned_weapons(0.), memory(NULL), mem_size(0),
		attached(false), attached_list(NULL), link_list(NULL), nb_attached(0), just_created(false),
		first_move(false), severity(0), cur_px(0), cur_py(0),
		metal_prod(0.), metal_cons(0.), energy_prod(0.), energy_cons(0.),
		last_time_sound(0),
		cur_metal_prod(0.), cur_metal_cons(0.), cur_energy_prod(0.), cur_energy_cons(0.),
		ripple_timer(0), weapon(),
		death_delay(0.), was_moving(false), last_path_refresh(0.), shadow_scale_dir(0.),
		hidden(false), flying(false), cloaked(false), cloaking(false), paralyzed(0.),
		//
		drawn_open(false), drawn_flying(false), drawn_x(0), drawn_y(0), drawn(false),
		//
		sight(0), radar_range(0), sonar_range(0), radar_jam_range(0), sonar_jam_range(0),
		old_px(0), old_py(0),
		move_target_computed(), was_locked(0.), self_destruct(0.), build_percent_left(0.),
		metal_extracted(0.), requesting_pathfinder(false), pad1(0), pad2(0), pad_timer(0.),
		command_locked(false), yardmap_timer(0), death_timer(0),
		//
		sync_hash(0), last_synctick(NULL), local(false), exploding(false), previous_sync(),
		nanolathe_target(0), nanolathe_reverse(false), nanolathe_feature(false)
	{
	}


	Unit::~Unit()
	{
		DELETE_ARRAY(port);
		DELETE_ARRAY(memory);
		DELETE_ARRAY(attached_list);
		DELETE_ARRAY(link_list);
		DELETE_ARRAY(last_synctick);
	}



	void Unit::destroy(bool full)
	{
		while (drawing)
			rest(0);
		pMutex.lock();
		ID = 0;
		script = NULL;
		while (!mission.empty())
			clear_mission();
		clear_def_mission();
        data.destroy();
		init();
		flags = 0;
		groupe = 0;
		pMutex.unlock();
		if (full)
		{
			DELETE_ARRAY(port);
			DELETE_ARRAY(memory);
			DELETE_ARRAY(attached_list);
			DELETE_ARRAY(link_list);
			DELETE_ARRAY(last_synctick);
		}
	}


	void Unit::start_building(const Vector3D &dir)
	{
		activate();
		// Work in model coordinates
		Vector3D Dir(dir * RotateXZY(-Angle.x * DEG2RAD, -Angle.z * DEG2RAD, -Angle.y * DEG2RAD));
		Vector3D P(Dir);
		P.y = 0.0f;
		float angle = acosf(P.z / P.norm()) * RAD2DEG;
		if (P.x < 0.0f)
			angle = -angle;
		if (angle > 180)
			angle -= 360;
		else
		{
			if (angle < -180)
				angle += 360;
		}

		float angleX = asinf(Dir.y / Dir.norm()) * RAD2DEG;
		if (angleX > 180)	angleX -= 360;
		else if (angleX < -180)	angleX += 360;
		int param[] = { (int)(angle * DEG2TA), (int)(angleX * DEG2TA) };
		launchScript(SCRIPT_startbuilding, 2, param );
		playSound( "build" );
	}


	void Unit::start_mission_script(int mission_type)
	{
		if (script)
		{
			switch (mission_type)
			{
				case MISSION_STOP:
				case MISSION_STANDBY:
					if (port[INBUILDSTANCE])
					{
						launchScript(SCRIPT_stopbuilding);
						deactivate();
					}
					break;
				case MISSION_ATTACK:
					stopMoving();
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
				flags &= 191;
		}
	}


	void Unit::clear_mission()
	{
		if (!mission)
			return;

		// Don't forget to detach the planes from air repair pads!
		if (mission->mission() == MISSION_GET_REPAIRED && mission->getTarget().isUnit())
		{
			Unit *target_unit = mission->getUnit();
			target_unit->lock();
			if (target_unit->flags & 1)
			{
				int piece_id = mission->getData() >= 0 ? mission->getData() : (-mission->getData() - 1);
				if (target_unit->pad1 == piece_id) // tell others we've left
					target_unit->pad1 = 0xFFFF;
				else
					target_unit->pad2 = 0xFFFF;
			}
			target_unit->unlock();
		}
		pMutex.lock();
		mission.next();
		pMutex.unlock();
	}



	void Unit::compute_model_coord()
	{
		if (!compute_coord || !model)
			return;
		pMutex.lock();
		compute_coord = false;
		if (type_id < 0 || !(flags & 1))		// The unit is dead
		{
			pMutex.unlock();
			return;
		}
		const UnitType *pType = unit_manager.unit_type[type_id];
        const float scale = pType->Scale;

		// Matrice pour le calcul des positions des éléments du modèle de l'unité
		//    M = RotateZ(Angle.z*DEG2RAD) * RotateY(Angle.y * DEG2RAD) * RotateX(Angle.x * DEG2RAD) * Scale(scale);
		Matrix M = RotateZYX( Angle.z * DEG2RAD, Angle.y * DEG2RAD, Angle.x * DEG2RAD) * Scale(scale);
		model->compute_coord(&data, &M);
		pMutex.unlock();
	}


	void Unit::init_alloc_data()
	{
		port = new sint16[21];				// Ports
		memory = new uint32[TA3D_PLAYERS_HARD_LIMIT];				// Pour se rappeler sur quelles armes on a déjà tiré
		attached_list = new short[20];
		link_list = new short[20];
		last_synctick = new uint32[TA3D_PLAYERS_HARD_LIMIT];
	}


	void Unit::toggle_self_destruct()
	{
		const UnitType *pType = unit_manager.unit_type[type_id];
        if (self_destruct < 0.0f)
            self_destruct = pType->selfdestructcountdown;
		else
			self_destruct = -1.0f;
	}

	bool Unit::isEnemy(const int t) const
	{
		return t >= 0 && t < (int)units.max_unit && !(players.team[units.unit[t].owner_id] & players.team[owner_id]);
	}

	int Unit::runScriptFunction(const int id, int nb_param, int *param)	// Launch and run the script, returning it's values to param if not NULL
	{
		const int type = type_id;
        if (!script || type == -1 || !unit_manager.unit_type[type]->script || !unit_manager.unit_type[type]->script->isCached(id))
            return -1;
		const String f_name( UnitScriptInterface::get_script_name(id) );
		MutexLocker mLocker( pMutex );
		if (script)
        {
			const int result = script->execute(f_name, param, nb_param);
            if (result == -2)
            {
                unit_manager.unit_type[type]->script->Uncache(id);
                return -1;
            }
            return result;
        }
		return -1;
	}

	void Unit::resetScript()
	{
		pMutex.lock();
		pMutex.unlock();
	}


	void Unit::stopMoving()
	{
		if (mission->getFlags() & MISSION_FLAG_MOVE)
		{
			unsetFlag(mission->Flags(), MISSION_FLAG_MOVE);
			if (!mission->Path().empty())
			{
				mission->Path().clear();
				V.reset();
			}
			if (!(unit_manager.unit_type[type_id]->canfly && nb_attached > 0)) // Once charged with units the Atlas cannot land
				stopMovingAnimation();
			was_moving = false;
			if (!(mission->Flags() & MISSION_FLAG_DONT_STOP_MOVE))
				V.reset();		// Stop unit's movement
		}
		else if (selfmove && !(unit_manager.unit_type[type_id]->canfly && nb_attached > 0))
			stopMovingAnimation();
		selfmove = false;
	}

	void Unit::stopMovingAnimation()
	{
		requestedMovingAnimationState = false;
	}

	void Unit::startMovingAnimation()
	{
		requestedMovingAnimationState = true;
	}

	void Unit::lock_command()
	{
		pMutex.lock();
		command_locked = true;
		pMutex.unlock();
	}

	void Unit::unlock_command()
	{
		pMutex.lock();
		command_locked = false;
		pMutex.unlock();
	}


	void Unit::activate()
	{
		pMutex.lock();
		if (port[ACTIVATION] == 0)
		{
			playSound("activate");
			launchScript(SCRIPT_Activate);
			port[ACTIVATION] = 1;
		}
		pMutex.unlock();
	}

	void Unit::deactivate()
	{
		pMutex.lock();
		if (port[ACTIVATION] != 0)
		{
			playSound("deactivate");
			launchScript(SCRIPT_Deactivate);
			port[ACTIVATION] = 0;
		}
		pMutex.unlock();
	}



	void Unit::init(int unit_type, int owner, bool full, bool basic)
	{
		pMutex.lock();

		birthTime = 0.0f;

		movingAnimation = false;

		kills = 0;
		selfmove = false;
		lastEnergy = 99999999.9f;

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
		drawn = false;
		drawn_obstacle = false;

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
		cur_px = 0;
		cur_py = 0;
		sight = 0;
		radar_range = 0;
		sonar_range = 0;
		radar_jam_range = 0;
		sonar_jam_range = 0;
		severity = 0;
		if (full)
			init_alloc_data();
		just_created = true;
		first_move = true;
		attached = false;
		nb_attached = 0;
		mem_size = 0;
		planned_weapons = 0.0f;
		attacked = false;
		groupe = 0;
		weapon.clear();
		h = 0.0f;
		compute_coord = true;
		c_time = 0.0f;
		flags = 1;
		sel = false;
		script = NULL;
		model = NULL;
		owner_id = (byte)owner;
		type_id = -1;
		hp = 0.0f;
		V.reset();
		Pos.reset();
		data.init();
		Angle.reset();
		V_Angle = Angle;
		int i;
		for(i = 0 ; i < 21 ; ++i)
			port[i]=0;
		if (unit_type < 0 || unit_type >= unit_manager.nb_unit)
			unit_type = -1;
		port[ACTIVATION] = 0;
		mission.clear();
		def_mission.clear();
		build_percent_left = 0.0f;
		memset( last_synctick, 0, 40 );
		if (unit_type != -1)
		{
			type_id = unit_type;
			if (!basic)
			{
				pMutex.unlock();
				set_mission(MISSION_STANDBY);
				pMutex.lock();
			}
			const UnitType* const pType = unit_manager.unit_type[type_id];
            model = pType->model;
            weapon.resize(pType->weapon.size());
			hp = (float)pType->MaxDamage;
            script = UnitScriptInterface::instanciate( pType->script );
            port[STANDINGMOVEORDERS] = pType->StandingMoveOrder;
            port[STANDINGFIREORDERS] = pType->StandingFireOrder;
			if (!basic)
			{
				pMutex.unlock();
                set_mission(pType->DefaultMissionType);
				pMutex.lock();
			}
			if (script)
			{
				script->setUnitID( idx );
				data.load( script->getNbPieces() );
				launchScript(SCRIPT_create);
			}
		}
		pMutex.unlock();
	}


	void Unit::clear_def_mission()
	{
		pMutex.lock();
		def_mission.clear();
		pMutex.unlock();
	}




	bool Unit::is_on_radar(byte p_mask) const
	{
		if (type_id == -1)
			return false;
		const UnitType* const pType = unit_manager.unit_type[type_id];
		const int px = cur_px >> 1;
		const int py = cur_py >> 1;
		gfx->lock();
		if (px >= 0 && py >= 0 && px < the_map->radar_map.getWidth() && py < the_map->radar_map.getHeight())
		{
			const bool r = ( (the_map->radar_map(px,py) & p_mask) && !pType->Stealth && (pType->fastCategory & CATEGORY_NOTSUB) )
						   || ( (the_map->sonar_map(px,py) & p_mask) && !(pType->fastCategory & CATEGORY_NOTSUB) );
			gfx->unlock();
			return r;
		}
		gfx->unlock();
		return false;
	}

	void Unit::add_mission(int mission_type, const Vector3D* target, bool step, int dat, void* pointer,
						   byte m_flags, int move_data, int patrol_node)
	{
		MutexLocker locker(pMutex);

		if (command_locked && !(mission_type & MISSION_FLAG_AUTO))
			return;

		const UnitType* const pType = unit_manager.unit_type[type_id];
        mission_type &= ~MISSION_FLAG_AUTO;

		uint32 target_ID = 0;
		int targetIdx = -1;
		Mission::Target::Type targetType = Mission::Target::TargetNone;

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
					target_ID = ((Unit*)pointer)->ID;
					targetType = Mission::Target::TargetUnit;
					targetIdx = ((Unit*)pointer)->idx;
					break;
				case MISSION_ATTACK:
					if (!(m_flags & MISSION_FLAG_TARGET_WEAPON))
					{
						target_ID = ((Unit*)pointer)->ID;
						targetIdx = ((Unit*)pointer)->idx;
						targetType = Mission::Target::TargetUnit;
					}
					else
					{
						targetIdx = ((Weapon*)pointer)->idx;
						targetType = Mission::Target::TargetWeapon;
					}
					break;
			}
		}
		else if (target)
			targetType = Mission::Target::TargetStatic;

		bool def_mode = false;
        if (type_id != -1 && !pType->BMcode)
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

		if (mission_type == MISSION_MOVE || mission_type == MISSION_PATROL)
			m_flags |= MISSION_FLAG_MOVE;

        if (type_id != -1 && mission_type == MISSION_BUILD && pType->BMcode && pType->Builder && target != NULL)
		{
			bool removed = false;
			MissionStack::iterator cur = mission.begin();
			if (!mission.empty())
				++cur;		// Don't read the first one ( which is being executed )

			while (cur != mission.end()) 	// Reads the mission list
			{
				const float x_space = fabsf(cur->lastStep().getTarget().getPos().x - target->x);
				const float z_space = fabsf(cur->lastStep().getTarget().getPos().z - target->z);
				const int cur_data = cur->lastStep().getData();
				if (cur->lastMission() == MISSION_BUILD && cur_data >= 0 && cur_data < unit_manager.nb_unit
					&& x_space < ((unit_manager.unit_type[ dat ]->FootprintX + unit_manager.unit_type[ cur_data ]->FootprintX) << 2)
					&& z_space < ((unit_manager.unit_type[ dat ]->FootprintZ + unit_manager.unit_type[ cur_data ]->FootprintZ) << 2) ) // Remove it
				{
					cur = mission.erase(cur);
					removed = true;
				}
				else
					++cur;
			}
			if (removed)
				return;
		}

		Mission tmp;
		Mission &new_mission = step ? (def_mode ? def_mission.front() : mission.front()) : tmp;

		new_mission.addStep();
		new_mission.setMissionType((uint8)mission_type);
		new_mission.getTarget().set(targetType, targetIdx, target_ID);
		if (target)
			new_mission.getTarget().setPos(*target);
		new_mission.setTime(0.0f);
		new_mission.setData(dat);
		new_mission.Path().clear();
		new_mission.setLastD(9999999.0f);
		new_mission.setFlags(m_flags);
		new_mission.setMoveData(move_data);
		new_mission.setNode((uint16)patrol_node);

		if (!step && patrol_node == -1 && mission_type == MISSION_PATROL)
		{
			MissionStack &mission_base = def_mode ? def_mission : mission;
			if (!mission_base.empty()) // Ajoute l'ordre aux autres
			{
				MissionStack::iterator cur = mission_base.begin();
				MissionStack::iterator last = mission_base.end();
				patrol_node = 0;
				while (cur != mission_base.end())
				{
					if (cur->mission() == MISSION_PATROL && patrol_node <= cur->getNode())
					{
						patrol_node = cur->getNode();
						last = cur;
					}
					++cur;
				}
				new_mission.setNode(uint16(patrol_node + 1));

				if (last != mission_base.end())
					++last;

				mission_base.insert(last, new_mission);
			}
			else
			{
				new_mission.setNode(1);
				mission_base.add(new_mission);
			}
		}
		else
		{
			bool stop = true;
			if (step)
			{
				switch(new_mission.lastMission())
				{
				case MISSION_MOVE:
				case MISSION_PATROL:
				case MISSION_STANDBY:
				case MISSION_VTOL_STANDBY:
				case MISSION_STOP:		// Don't stop if it's not necessary
					stop = !(mission_type == MISSION_MOVE || mission_type == MISSION_PATROL);
					break;
				case MISSION_BUILD:
				case MISSION_BUILD_2:
					// Prevent factories from closing when already building a unit
					stop = mission_type == MISSION_BUILD && type_id != -1 && !pType->BMcode;
				};
			}
			else
				stop = pType->BMcode;
			if (stop)
			{
				new_mission.addStep();
				new_mission.setMissionType(MISSION_STOP);
				new_mission.setFlags(byte(m_flags & ~MISSION_FLAG_MOVE));
			}
			if (!step)
			{
				if (def_mode)
					def_mission.add(new_mission);
				else
					mission.add(new_mission);
			}
			else if (!def_mode)
				start_mission_script(mission->mission());
		}
	}



	void Unit::set_mission(int mission_type, const Vector3D* target, bool /*step*/, int dat, bool stopit,
						   void* pointer, byte m_flags, int move_data)
	{
		MutexLocker locker(pMutex);

		if (command_locked && !( mission_type & MISSION_FLAG_AUTO))
			return;
		const UnitType *pType = unit_manager.unit_type[type_id];
        mission_type &= ~MISSION_FLAG_AUTO;

		uint32 target_ID = 0;
		int targetIdx = -1;
		Mission::Target::Type targetType = Mission::Target::TargetNone;

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
					target_ID = ((Unit*)pointer)->ID;
					targetIdx = ((Unit*)pointer)->idx;
					targetType = Mission::Target::TargetUnit;
					break;
				case MISSION_ATTACK:
					if (!(m_flags&MISSION_FLAG_TARGET_WEAPON) )
					{
						target_ID = ((Unit*)pointer)->ID;
						targetIdx = ((Unit*)pointer)->idx;
						targetType = Mission::Target::TargetUnit;
					}
					else
					{
						targetIdx = ((Weapon*)pointer)->idx;
						targetType = Mission::Target::TargetWeapon;
					}
					break;
			}
		}
		else if (target)
			targetType = Mission::Target::TargetStatic;

		if (nanolathe_target >= 0 && network_manager.isConnected())
		{
			nanolathe_target = -1;
			g_ta3d_network->sendUnitNanolatheEvent( idx, -1, false, false );
		}

		bool def_mode = false;
        if (type_id != -1 && !pType->BMcode)
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

		int old_mission = -1;
		if (!def_mode)
		{
			if (!mission.empty())
				old_mission = mission->mission();
		}
		else
			clear_def_mission();

		bool already_running = false;

		if (mission_type == MISSION_MOVE || mission_type == MISSION_PATROL )
			m_flags |= MISSION_FLAG_MOVE;

		switch(old_mission)		// Commandes de fin de mission
		{
		case MISSION_REPAIR:
		case MISSION_REVIVE:
		case MISSION_RECLAIM:
		case MISSION_CAPTURE:
		case MISSION_BUILD_2:
			deactivate();
			launchScript(SCRIPT_stopbuilding);
			if (type_id != -1 && !pType->BMcode) // Delete the unit we were building
			{
				sint32 prev = -1;
				for(int i = units.nb_unit-1; i >= 0 ; --i)
				{
					if (units.idx_list[i] == mission->getUnit()->idx)
					{
						prev = i;
						break;
					}
				}
				if (prev >= 0)
					units.kill(mission->getUnit()->idx, prev);
			}
			break;
		case MISSION_ATTACK:
			if (mission_type != MISSION_ATTACK && type_id != -1 &&
				(!pType->canfly
				 || (pType->canfly && mission_type != MISSION_MOVE && mission_type != MISSION_PATROL)))
				deactivate();
			else
			{
				stopit = false;
				already_running = true;
			}
			break;
		case MISSION_MOVE:
		case MISSION_PATROL:
			if (mission_type == MISSION_MOVE || mission_type == MISSION_PATROL
				|| (type_id != -1 && pType->canfly && mission_type == MISSION_ATTACK ) )
			{
				stopit = false;
				already_running = true;
			}
			break;
		};

		if (!def_mode)
		{
			while (!mission.empty())
				clear_mission();			// Efface les ordres précédents
			last_path_refresh = 10.0f;
			requesting_pathfinder = false;
			if (nanolathe_target >= 0 && network_manager.isConnected())		// Stop nanolathing
			{
				nanolathe_target = -1;
				g_ta3d_network->sendUnitNanolatheEvent( idx, -1, false, false );
			}
		}

		if (def_mode)
		{
			def_mission.clear();
			def_mission.add();

			def_mission->setMissionType((uint8)mission_type);
			def_mission->getTarget().set(targetType, targetIdx, target_ID);
			def_mission->setData(dat);
			def_mission->Path().clear();
			def_mission->setLastD(9999999.0f);
			def_mission->setFlags(m_flags);
			def_mission->setMoveData(move_data);
			def_mission->setNode(1);
			if (target)
				def_mission->getTarget().setPos(*target);

			if (stopit)
			{
				def_mission->addStep();
				def_mission->setMissionType(MISSION_STOP);
				def_mission->setData(0);
				def_mission->Path().clear();
				def_mission->setFlags(uint8(m_flags & ~MISSION_FLAG_MOVE));
			}
		}
		else
		{
			mission.clear();
			mission.add();

			mission->setMissionType(uint8(mission_type));
			mission->getTarget().set(targetType, targetIdx, target_ID);
			mission->setData(dat);
			mission->Path().clear();
			mission->setLastD(9999999.0f);
			mission->setFlags(m_flags);
			mission->setMoveData(move_data);
			mission->setNode(1);
			if (target)
				mission->getTarget().setPos(*target);

			if (stopit)
			{
				mission->addStep();
				mission->setMissionType(MISSION_STOP);
				mission->setData(0);
				mission->Path().clear();
				mission->setLastD(9999999.0f);
				mission->setFlags(m_flags | MISSION_FLAG_MOVE);
				mission->setMoveData(move_data);
			}
			else
			{
				if (!already_running)
					start_mission_script(mission->mission());
			}
			c_time = 0.0f;
		}
	}



	void Unit::next_mission()
	{
		UnitType *pType = type_id != -1 ? unit_manager.unit_type[type_id] : NULL;
        last_path_refresh = 10.0f;		// By default allow to compute a new path
		requesting_pathfinder = false;
		if (nanolathe_target >= 0 && network_manager.isConnected())
		{
			nanolathe_target = -1;
			g_ta3d_network->sendUnitNanolatheEvent( idx, -1, false, false );
		}

		if (!mission)
		{
			command_locked = false;
			if (type_id != -1)
                set_mission( pType->DefaultMissionType, NULL, false, 0, false);
			return;
		}
		switch (mission->mission()) // Commandes de fin de mission
		{
			case MISSION_REPAIR:
			case MISSION_RECLAIM:
			case MISSION_BUILD_2:
			case MISSION_REVIVE:
			case MISSION_CAPTURE:
				if (!mission.hasNext()
					|| (pType && pType->BMcode)
					|| mission[1].lastMission() != MISSION_BUILD)
				{
					launchScript(SCRIPT_stopbuilding);
					deactivate();
				}
				break;
			case MISSION_ATTACK:
				deactivate();
				break;
		}
		if (mission->mission() == MISSION_STOP && !mission.hasNext())
		{
			command_locked = false;
			mission->setData(0);
			return;
		}
		bool old_step = mission->isStep();
		mission.next();
		if (!mission)
		{
			command_locked = false;
			if (type_id != -1)
                set_mission(pType->DefaultMissionType);
		}

		// Skip a stop order before a normal order if the unit can fly (prevent planes from looking for a place to land when they don't need to land !!)
		if (pType != NULL
			&& pType->canfly
			&& mission->mission() == MISSION_STOP
			&& mission.hasNext() && mission(1) != MISSION_STOP)
		{
			mission.next();
		}

		if (old_step
			&& !mission.empty()
			&& mission.hasNext()
			&& (mission->mission() == MISSION_STOP
				|| mission->mission() == MISSION_VTOL_STANDBY
				|| mission->mission() == MISSION_STANDBY))
			next_mission();

		start_mission_script(mission->mission());
		c_time = 0.0f;
	}


	void Unit::draw(float t, bool height_line)
	{
		MutexLocker locker(pMutex);

		if (!(flags & 1) || type_id == -1 || ID != render.UID || !visible)
			return;

		UnitType* const pType = unit_manager.unit_type[type_id];
        visible = false;
		on_radar = false;
		on_mini_radar = false;

		if (!model || hidden)
			return;		// S'il n'y a pas de modèle associé, on quitte la fonction

		const int px = render.px >> 1;
		const int py = render.py >> 1;
		if (px < 0 || py < 0 || px >= the_map->bloc_w || py >= the_map->bloc_h)
			return;	// Unité hors de la carte
		const byte player_mask = byte(1 << players.local_human_id);

		on_radar = on_mini_radar = is_on_radar( player_mask );
		if (the_map->view(px, py) == 0 || ( the_map->view(px, py) > 1 && !on_radar ) )
			return;	// Unit is not visible

		if (!on_radar)
		{
			gfx->lock();
			if (!(the_map->sight_map(px, py) & player_mask))
			{
				gfx->unlock();
				return;
			}
			gfx->unlock();
		}

		const bool radar_detected = on_radar;

		on_radar &= the_map->view(px, py) > 1;

		Vector3D D (render.Pos - Camera::inGame->pos); // Vecteur "viseur unité" partant de la caméra vers l'unité

		const float dist = D.sq();
		const float p = D % Camera::inGame->dir;
		if (dist >= 16384.0f && p <= 0.0f)
			return;
		if (p > Camera::inGame->zfar2)
			return;		// Si l'objet est hors champ on ne le dessine pas

		if (!cloaked || owner_id == players.local_human_id) // Don't show cloaked units
		{
			visible = true;
			on_radar |= Camera::inGame->rpos.y > gfx->low_def_limit;
		}
		else
		{
			on_radar |= radar_detected;
			visible = on_radar;
		}

		Matrix M;
		glPushMatrix();
		if (on_radar) // for mega zoom, draw only an icon
		{
			glDisable(GL_DEPTH_TEST);
			glTranslatef( render.Pos.x, Math::Max(render.Pos.y, the_map->sealvl + 5.0f), render.Pos.z);
			glEnable(GL_TEXTURE_2D);
			int unit_nature = ICON_UNKNOWN;
			// In orthographic mode we need another formula
			const float size = lp_CONFIG->ortho_camera
						 ? Camera::inGame->zoomFactor * 9.0f
						 : (D % Camera::inGame->dir) * 12.0f / float(gfx->height);

            if (pType->fastCategory & CATEGORY_KAMIKAZE)
				unit_nature = ICON_KAMIKAZE;
            else if (( pType->TEDclass & CLASS_COMMANDER ) == CLASS_COMMANDER)
				unit_nature = ICON_COMMANDER;
            else if (( pType->TEDclass & CLASS_ENERGY ) == CLASS_ENERGY)
				unit_nature = ICON_ENERGY;
            else if (( pType->TEDclass & CLASS_METAL ) == CLASS_METAL)
				unit_nature = ICON_METAL;
			else if (pType->Builder)
			{
				if (!pType->BMcode)
					unit_nature = ICON_FACTORY;
				else
					unit_nature = ICON_BUILDER;
			}
			else if (( pType->TEDclass & CLASS_TANK ) == CLASS_TANK)
				unit_nature = ICON_GROUND_ASSAULT;
			else if (( pType->TEDclass & CLASS_KBOT ) == CLASS_KBOT)
				unit_nature = ICON_GROUND_ASSAULT;
            else if (( pType->TEDclass & CLASS_SHIP ) == CLASS_SHIP )
				unit_nature = ICON_WATERUNIT;
            else if (( pType->TEDclass & CLASS_FORT ) == CLASS_FORT )
				unit_nature = ICON_DEFENSE;
			else if (pType->checkCategory("KBOT") || pType->checkCategory("TANK") || pType->checkCategory("HOVER"))
				unit_nature = ICON_GROUND_ASSAULT;
			else if (( pType->fastCategory & CATEGORY_NOTAIR ) && ( pType->fastCategory & CATEGORY_NOTSUB ) )
				unit_nature = ICON_LANDUNIT;
            else if (!( pType->fastCategory & CATEGORY_NOTAIR ) )
				unit_nature = ICON_AIRUNIT;
            else if (!( pType->fastCategory & CATEGORY_NOTSUB ) )
				unit_nature = ICON_SUBUNIT;

			const float sizew = size * (float)units.icons[ unit_nature ].getWidth() / 24.0f;
			const float sizeh = size * (float)units.icons[ unit_nature ].getHeight() / 24.0f;
			glBindTexture( GL_TEXTURE_2D, units.icons[ unit_nature ].get() );
			glDisable( GL_CULL_FACE );
			glDisable(GL_LIGHTING);
			glTranslatef( model->center.x, model->center.y, model->center.z );
			const Vector3D side = Camera::inGame->side;
			const Vector3D up = Camera::inGame->up;
			if (!Yuni::Math::Zero(player_color[player_color_map[owner_id] * 3])
				|| !Yuni::Math::Zero(player_color[player_color_map[owner_id] * 3 + 1])
				|| !Yuni::Math::Zero(player_color[player_color_map[owner_id] * 3 + 2]))
			{
				glColor4f(player_color[player_color_map[owner_id] * 3],
						  player_color[player_color_map[owner_id] * 3 + 1],
						  player_color[player_color_map[owner_id] * 3 + 2],
						  1.0f);
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);		gfx->loadVertex(-sizew * side + sizeh * up);
				glTexCoord2f(1.0f, 0.0f);		gfx->loadVertex(sizew * side + sizeh * up);
				glTexCoord2f(1.0f, 1.0f);		gfx->loadVertex(sizew * side - sizeh * up);
				glTexCoord2f(0.0f, 1.0f);		gfx->loadVertex(-sizew * side - sizeh * up);
				glEnd();
				glDisable(GL_BLEND);
			}
			else
			{								// If it's black, then invert colors
				glColor4ub(0xFF,0xFF,0xFF,0xFF);
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_BLEND);
				glBegin(GL_QUADS);
				gfx->loadVertex(-sizew * side + sizeh * up);
				gfx->loadVertex(sizew * side + sizeh * up);
				gfx->loadVertex(sizew * side - sizeh * up);
				gfx->loadVertex(-sizew * side - sizeh * up);
				glEnd();
				glEnable(GL_TEXTURE_2D);
				glBlendFunc( GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
				glEnable(GL_BLEND);
				glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 0.0f);		gfx->loadVertex(-sizew * side + sizeh * up);
				glTexCoord2f(1.0f, 0.0f);		gfx->loadVertex(sizew * side + sizeh * up);
				glTexCoord2f(1.0f, 1.0f);		gfx->loadVertex(sizew * side - sizeh * up);
				glTexCoord2f(0.0f, 1.0f);		gfx->loadVertex(-sizew * side - sizeh * up);
				glEnd();
				glDisable(GL_BLEND);
			}
			glEnable( GL_CULL_FACE );
			if (owner_id == players.local_human_id && sel)
			{
				glDisable( GL_TEXTURE_2D );
				glColor3ub(0xFF,0xFF,0);
				glBegin(GL_LINE_LOOP);
				gfx->loadVertex(-sizew * side + sizeh * up);
				gfx->loadVertex(sizew * side + sizeh * up);
				gfx->loadVertex(sizew * side - sizeh * up);
				gfx->loadVertex(-sizew * side - sizeh * up);
				glEnd();
			}
			glEnable(GL_DEPTH_TEST);
		}
		else
			if (visible && model)
			{
				// Set time origin to our birth date
				t -= birthTime;
				glTranslatef( render.Pos.x, render.Pos.y, render.Pos.z );

				if (lp_CONFIG->underwater_bright && the_map->water && render.Pos.y < the_map->sealvl)
				{
					double eqn[4]= { 0.0f, -1.0f, 0.0f, the_map->sealvl - render.Pos.y };
					glClipPlane(GL_CLIP_PLANE2, eqn);
				}

				glRotatef(render.Angle.x,1.0f,0.0f,0.0f);
				glRotatef(render.Angle.z,0.0f,0.0f,1.0f);
				glRotatef(render.Angle.y,0.0f,1.0f,0.0f);
				const float scale = pType->Scale;
				glScalef(scale,scale,scale);

				//            M=RotateY(Angle.y*DEG2RAD)*RotateZ(Angle.z*DEG2RAD)*RotateX(Angle.x*DEG2RAD)*Scale(scale);			// Matrice pour le calcul des positions des éléments du modèle de l'unité
				M = RotateYZX(render.Angle.y * DEG2RAD,
							  render.Angle.z * DEG2RAD,
							  render.Angle.x * DEG2RAD) * Scale(scale);			// Matrice pour le calcul des positions des éléments du modèle de l'unité

				const Vector3D *target = NULL, *center = NULL;
				Vector3D upos;
				bool c_part = false;
				bool reverse = false;
				float size = 0.0f;
				Mesh *src = NULL;
				AnimationData *src_data = NULL;
				Vector3D v_target;				// Needed in network mode
				Unit *unit_target = NULL;
				Model* const the_model = model;
				drawing = true;

				if (!pType->emitting_points_computed) // Compute model emitting points if not already done, do it here in Unit::Locked code ...
				{
                    pType->emitting_points_computed = true;
					int first = runScriptFunction( SCRIPT_QueryNanoPiece );
					int current;
					int i = 0;
					do
					{
						current = runScriptFunction( SCRIPT_QueryNanoPiece );
						if (model)
							model->mesh->compute_emitter_point( current );
						++i;
					} while( first != current && i < 1000 );
				}

				if (Yuni::Math::Zero(build_percent_left) && !mission.empty() && port[ INBUILDSTANCE ] != 0 && local )
				{
					if (c_time >= 0.125f)
					{
						reverse = (mission->mission() == MISSION_RECLAIM);
						c_time = 0.0f;
						c_part = true;
						upos.x = upos.y = upos.z = 0.0f;
						upos = upos + render.Pos;
						if (mission->getTarget().isUnit()
							&& (mission->mission() == MISSION_REPAIR
								|| mission->mission() == MISSION_BUILD
								|| mission->mission() == MISSION_BUILD_2
								|| mission->mission() == MISSION_CAPTURE
								|| mission->mission() == MISSION_RECLAIM))
						{
							unit_target = mission->getUnit();
							if (unit_target)
							{
								pMutex.unlock();
								unit_target->lock();
								if ((unit_target->flags & 1) && unit_target->model != NULL)
								{
									size = unit_target->model->size2;
									center = &(unit_target->model->center);
									src = unit_target->model->mesh;
									src_data = &(unit_target->data);
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
								unit_target = NULL;
								c_part = false;
							}
						}
						else
						{
							if (mission->mission() == MISSION_RECLAIM
								|| mission->mission() == MISSION_REVIVE ) // Reclaiming features
							{
								const int feature_type = features.feature[ mission->getData() ].type;
								const Feature* const feature = feature_manager.getFeaturePointer(feature_type);
								if (mission->getData() >= 0 && feature && feature->model )
								{
									size = feature->model->size2;
                                    center = &(feature->model->center);
									src = feature->model->mesh;
									src_data = NULL;
								}
								else
								{
									D.reset();
									center = &D;
									size = 32.0f;
								}
							}
							else
								c_part = false;
						}
						target = &(mission->getTarget().getPos());
					}
				}
				else
				{
					if (!local && nanolathe_target >= 0 && port[ INBUILDSTANCE ] != 0 )
					{
						if (c_time >= 0.125f)
						{
							reverse = nanolathe_reverse;
							c_time = 0.0f;
							c_part = true;
							upos.reset();
							upos = upos + render.Pos;
							if (!nanolathe_feature)
							{
								unit_target = &(units.unit[ nanolathe_target ]);
								pMutex.unlock();
								unit_target->lock();
								if ((unit_target->flags & 1) && unit_target->model )
								{
									size = unit_target->model->size2;
                                    center = &(unit_target->model->center);
									src = unit_target->model->mesh;
                                    src_data = &(unit_target->data);
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
								const int feature_type = features.feature[ nanolathe_target ].type;
								v_target = features.feature[ nanolathe_target ].Pos;
								const Feature* const feature = feature_manager.getFeaturePointer(feature_type);
								if (feature && feature->model)
								{
									size = feature->model->size2;
                                    center = &(feature->model->center);
									src = feature->model->mesh;
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

				const bool old_mode = gfx->getShadowMapMode();
                glColor4ub( 0xFF, 0xFF, 0xFF, 0xFF );

				if (Yuni::Math::Zero(build_percent_left))
				{
					if (pType->onoffable && !port[ACTIVATION])
						t = 0.0f;
					if (cloaked || ( cloaking && owner_id != players.local_human_id ))
						glColor4ub( 0xFF, 0xFF, 0xFF, 0x7F );
					glDisable(GL_CULL_FACE);
					the_model->draw(t, &render.Anim, owner_id == players.local_human_id && sel, false, c_part, build_part, target, &upos, &M, size, center, reverse, owner_id, cloaked, src, src_data);
					glEnable(GL_CULL_FACE);
					if (cloaked || ( cloaking && owner_id != players.local_human_id ))
						gfx->set_color( 0xFFFFFFFF );
                    if (height_line && h>1.0f && pType->canfly) // For flying units, draw a line that shows how high is the unit
					{
						glPopMatrix();
						glPushMatrix();
						glDisable(GL_TEXTURE_2D);
						glDisable(GL_LIGHTING);
						glColor3ub(0xFF,0xFF,0);
						glBegin(GL_LINES);
						for (float y = render.Pos.y ; y > render.Pos.y - h ; y -= 10.0f)
						{
							glVertex3f(render.Pos.x, y, render.Pos.z);
							glVertex3f(render.Pos.x, y - 5.0f, render.Pos.z);
						}
						glEnd();
					}
				}
				else
				{
					gfx->setShadowMapMode(true);
					if (build_percent_left <= 33.0f)
					{
						float h = model->top - model->bottom;
						double eqn[4]= { 0.0f, 1.0f, 0.0f, -model->bottom - h * (33.0f - build_percent_left) * 0.033333f};

						glClipPlane(GL_CLIP_PLANE0, eqn);
						glEnable(GL_CLIP_PLANE0);
						the_model->draw(t, &render.Anim, owner_id == players.local_human_id && sel, true, c_part, build_part, target, &upos, &M, size, center, reverse, owner_id, true, src, src_data);

						eqn[1] = -eqn[1];	eqn[3] = -eqn[3];
						glClipPlane(GL_CLIP_PLANE0, eqn);
						the_model->draw(t, &render.Anim, owner_id == players.local_human_id && sel, false, false, build_part, target, &upos, &M, size, center, reverse, owner_id);
						glDisable(GL_CLIP_PLANE0);

						glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
						the_model->draw(t, &render.Anim, owner_id == players.local_human_id && sel, true, false, build_part, target, &upos, &M, size, center, reverse, owner_id);
						glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
					}
					else
					{
						if (build_percent_left <= 66.0f)
						{
							float h = model->top - model->bottom;
							double eqn[4]= { 0.0f, 1.0f, 0.0f, -model->bottom - h * (66.0f - build_percent_left) * 0.033333f};

							glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
							glClipPlane(GL_CLIP_PLANE0, eqn);
							glEnable(GL_CLIP_PLANE0);
							glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
							the_model->draw(t, &render.Anim, owner_id == players.local_human_id && sel, true, c_part, build_part, target, &upos, &M, size, center, reverse, owner_id, true, src, src_data);
							glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

							eqn[1] = -eqn[1];	eqn[3] = -eqn[3];
							glClipPlane(GL_CLIP_PLANE0, eqn);
							the_model->draw(t, &render.Anim, owner_id == players.local_human_id && sel, true, false, build_part, target, &upos, &M, size, center, reverse, owner_id);
							glDisable(GL_CLIP_PLANE0);
						}
						else
						{
							glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
							the_model->draw(t, &render.Anim, owner_id == players.local_human_id && sel, true, false, build_part, target, &upos, &M, size, center, reverse, owner_id);
							glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
						}
					}
				}

				if (lp_CONFIG->underwater_bright && the_map->water && render.Pos.y < the_map->sealvl)
				{
					gfx->setShadowMapMode(true);
					glEnable(GL_CLIP_PLANE2);

					glEnable( GL_BLEND );
					glBlendFunc( GL_ONE, GL_ONE );
					glDepthFunc( GL_EQUAL );
					glColor4ub( 0x7F, 0x7F, 0x7F, 0x7F );
					the_model->draw(t, &render.Anim, false, true, false, 0, NULL, NULL, NULL, 0.0f,NULL, false, owner_id, false);
					glColor4ub( 0xFF, 0xFF, 0xFF, 0xFF );
					glDepthFunc( GL_LESS );
					glDisable( GL_BLEND );

					glDisable(GL_CLIP_PLANE2);
				}
				gfx->setShadowMapMode(old_mode);

				if (unit_target)
				{
					unit_target->unlock();
					pMutex.lock();
				}
			}
		drawing = false;
		glPopMatrix();
	}



	void Unit::draw_shadow(const Vector3D& Dir)
	{
		pMutex.lock();
		if (!(flags & 1) || ID != render.UID)
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
		{
			LOG_WARNING("Model is NULL ! (" << __FILE__ << ":" << __LINE__ << ")");
			pMutex.unlock();
			return;
		}

		if (cloaked && owner_id != players.local_human_id) // Unit is cloaked
		{
			pMutex.unlock();
			return;
		}

		if (!visible)
		{
			const Vector3D S_Pos = render.Pos - (h / Dir.y) * Dir;//the_map->hit(Pos,Dir);
			const int px = ((int)(S_Pos.x) + the_map->map_w_d) >> 4;
			const int py = ((int)(S_Pos.z) + the_map->map_h_d) >> 4;
			if (px < 0 || py < 0 || px >= the_map->bloc_w || py >= the_map->bloc_h)
			{
				pMutex.unlock();
				return;	// Shadow out of the map
			}
			if (the_map->view(px, py) != 1)
			{
				pMutex.unlock();
				return;	// Unvisible shadow
			}
		}

		const UnitType* const pType = unit_manager.unit_type[type_id];
        drawing = true;			// Prevent the model to be set to NULL and the data structure from being reset
		pMutex.unlock();

		glPushMatrix();
		glTranslatef(render.Pos.x, render.Pos.y, render.Pos.z);
		glRotatef(render.Angle.x,1.0f,0.0f,0.0f);
		glRotatef(render.Angle.z,0.0f,0.0f,1.0f);
		glRotatef(render.Angle.y,0.0f,1.0f,0.0f);
		const float scale = pType->Scale;
		glScalef(scale,scale,scale);
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

        if ((type_id != -1 && pType->canmove) || shadow_scale_dir < 0.0f)
		{
			Vector3D H = render.Pos;
			H.y += 2.0f * model->size2 + 1.0f;
			const Vector3D D = the_map->hit( H, Dir, true, 2000.0f);
			shadow_scale_dir = (D - H).norm();
		}
		model->draw_shadow(shadow_scale_dir * Dir * RotateXZY(-render.Angle.x * DEG2RAD, -render.Angle.z * DEG2RAD, -render.Angle.y * DEG2RAD), 0.0f, &render.Anim);

		glPopMatrix();

		drawing = false;
	}


	void Unit::drawShadowBasic(const Vector3D& Dir)
	{
		pMutex.lock();
		if (!(flags & 1) || ID != render.UID)
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
		{
			LOG_WARNING("Model is NULL ! (" << __FILE__ << ":" << __LINE__ << ")");
			pMutex.unlock();
			return;
		}

		if (cloaked && owner_id != players.local_human_id) // Unit is cloaked
		{
			pMutex.unlock();
			return;
		}

		if (!visible)
		{
			const Vector3D S_Pos (render.Pos - (h / Dir.y) * Dir);//the_map->hit(Pos,Dir);
			const int px = ((int)(S_Pos.x + (float)the_map->map_w_d)) >> 4;
			const int py = ((int)(S_Pos.z + (float)the_map->map_h_d)) >> 4;
			if (px < 0 || py < 0 || px >= the_map->bloc_w || py >= the_map->bloc_h)
			{
				pMutex.unlock();
				return;	// Shadow out of the map
			}
			if (the_map->view(px, py) != 1)
			{
				pMutex.unlock();
				return;	// Unvisible shadow
			}
		}
		const UnitType* const pType = unit_manager.unit_type[type_id];
        drawing = true;			// Prevent the model to be set to NULL and the data structure from being reset
		pMutex.unlock();

		glPushMatrix();
		glTranslatef(render.Pos.x, render.Pos.y, render.Pos.z);
		glRotatef(render.Angle.x,1.0f,0.0f,0.0f);
		glRotatef(render.Angle.z,0.0f,0.0f,1.0f);
		glRotatef(render.Angle.y,0.0f,1.0f,0.0f);
		const float scale = pType->Scale;
		glScalef(scale,scale,scale);
		glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
		if (pType->canmove || shadow_scale_dir < 0.0f)
		{
			Vector3D H = render.Pos;
			H.y += 2.0f * model->size2 + 1.0f;
			const Vector3D D = the_map->hit(H, Dir, true, 2000.0f);
			shadow_scale_dir = (D - H).norm();
		}
		model->draw_shadow_basic(shadow_scale_dir * Dir * RotateXZY(-render.Angle.x * DEG2RAD, -render.Angle.z * DEG2RAD, -render.Angle.y * DEG2RAD), 0.0f, &render.Anim);

		glPopMatrix();

		drawing = false;
	}


	void Unit::explode()
	{
		exploding = true;
		const UnitType* const pType = unit_manager.unit_type[type_id];
        if (local && network_manager.isConnected() ) // Sync unit destruction (and corpse creation ;) )
		{
			struct event explode_event;
			explode_event.type = EVENT_UNIT_EXPLODE;
			explode_event.opt1 = idx;
			explode_event.opt2 = (uint16)severity;
			explode_event.x = Pos.x;
			explode_event.y = Pos.y;
			explode_event.z = Pos.z;
			network_manager.sendEvent( &explode_event );
		}

		const int power = Math::Max(pType->FootprintX, pType->FootprintZ);
		fx_manager.addFlash( Pos, float(power * 32) );
		fx_manager.addExplosion( Pos, V, power * 3, float(power * 10) );

        int param[] = { severity * 100 / pType->MaxDamage, 0 };
		int corpse_type = runScriptFunction(SCRIPT_killed, 2, param);
		if (attached)
			corpse_type = 3;			// When we were flying we just disappear
		const bool sinking = the_map->get_unit_h( Pos.x, Pos.z ) <= the_map->sealvl;

		switch( corpse_type )
		{
			case 1:			// Some good looking corpse
				{
					pMutex.unlock();
					flags = 1;				// Set it to 1 otherwise it won't remove it from map
					clear_from_map();
					flags = 4;
					pMutex.lock();
					if (cur_px > 0 && cur_py > 0 && cur_px < (the_map->bloc_w<<1) && cur_py < (the_map->bloc_h<<1))
						if (the_map->map_data(cur_px, cur_py).stuff == -1)
						{
							int type = feature_manager.get_feature_index(pType->Corpse);
							if (type >= 0)
							{
								the_map->map_data(cur_px, cur_py).stuff = features.add_feature(Pos,type);
								if (the_map->map_data(cur_px, cur_py).stuff >= 0) 	// Keep unit orientation
								{
									features.feature[ the_map->map_data(cur_px, cur_py).stuff ].angle = Angle.y;
									if (sinking)
										features.sink_feature( the_map->map_data(cur_px, cur_py).stuff );
									features.drawFeatureOnMap( the_map->map_data(cur_px, cur_py).stuff );
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
						if (the_map->map_data(cur_px, cur_py).stuff == -1)
						{
							int type = feature_manager.get_feature_index( String(pType->name) << "_heap" );
							if (type >= 0)
							{
								the_map->map_data(cur_px, cur_py).stuff = features.add_feature(Pos,type);
								if (the_map->map_data(cur_px, cur_py).stuff >= 0)			// Keep unit orientation
								{
									features.feature[ the_map->map_data(cur_px, cur_py).stuff ].angle = Angle.y;
									if (sinking )
										features.sink_feature( the_map->map_data(cur_px, cur_py).stuff );
									features.drawFeatureOnMap( the_map->map_data(cur_px, cur_py).stuff );
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
		const int w_id = weapons.add_weapon(weapon_manager.get_weapon_index( Yuni::Math::Zero(self_destruct) ? pType->SelfDestructAs : pType->ExplodeAs ),idx);
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
			if (!(data.data[i].flag & FLAG_EXPLODE))// || (data.flag[i]&FLAG_EXPLODE && (data.explosion_flag[i]&EXPLODE_BITMAPONLY)))
				data.data[i].flag |= FLAG_HIDE;
		}
	}

	//! Ballistic calculations take place here
	float ballistic_angle(float v, float g, float d, float y_s, float y_e) // Calculs de ballistique pour l'angle de tir
	{
		const float v2 = v * v;
		const float gd = g * d;
		const float v2gd = v2 / gd;
		const float a = v2gd * (4.0f * v2gd - 8.0f * (y_e - y_s) / d) - 4.0f;
		if (a < 0.0f)				// Pas de solution
			return 360.0f;
		return RAD2DEG * atanf(v2gd - 0.5f * sqrtf(a));
	}

	//! Compute the local map energy WITHOUT current unit contribution (it assumes (x,y) is on pType->gRepulsion)
	float Unit::getLocalMapEnergy(int x, int y)
	{
		if (x < 0 || y < 0 || x >= the_map->bloc_w_db || y >= the_map->bloc_h_db)
			return 999999999.0f;
		float e = the_map->energy(x, y);
		const UnitType *pType = unit_manager.unit_type[type_id];
		e -= pType->gRepulsion( x - cur_px + (pType->gRepulsion.getWidth() >> 1),
								y - cur_py + (pType->gRepulsion.getHeight() >> 1) );
		return e;
	}

	//! Compute the dir vector based on MAP::energy and targeting
	void Unit::computeHeadingBasedOnEnergy(Vector3D &dir, const bool moving)
	{
		static const int order_dx[] = { -1, 0, 1, 1, 1, 0, -1, -1 };
		static const int order_dz[] = { -1, -1, -1, 0, 1, 1, 1, 0 };
		int b = -1;
		const int x = ((int)dir.x + the_map->map_w_d + 4) >> 3;
		const int z = ((int)dir.z + the_map->map_h_d + 4) >> 3;
		float E = getLocalMapEnergy(cur_px, cur_py);
		const UnitType *pType = unit_manager.unit_type[type_id];
		if (moving)
		{
			const float dist = sqrtf(float(SQUARE(cur_px - x) + SQUARE(cur_py - z)));
			if (dist < 64.0f)
				E *= dist * 0.015625f;
			E += 80.0f * float(pType->MaxSlope) * dist;
		}
		for(int i = 0 ; i < 8 ; ++i)
		{
			float e = getLocalMapEnergy(cur_px + order_dx[i], cur_py + order_dz[i]);
			if (moving)
			{
				const float dist = sqrtf(float(SQUARE(cur_px + order_dx[i] - x) + SQUARE(cur_py + order_dz[i] - z)));
				if (dist < 64.0f)
					e *= dist * 0.015625f;
				e += 80.0f * float(pType->MaxSlope) * dist;
			}
			if (e < E)
			{
				E = e;
				b = i;
			}
		}
		if (b == -1)
			dir.reset();
		else
		{
			dir = Vector3D((float)order_dx[b], 0.0f, (float)order_dz[b]);
			dir.unit();
		}
	}

	//! Send a request to the pathfinder when we need a complex path, then follow computed paths
	void Unit::followPath(const float dt, bool &b_TargetAngle, float &f_TargetAngle, Vector3D &NPos, int &n_px, int &n_py, bool &precomputed_position)
	{
		// Don't control remote units
		if (!local)
			return;

		const UnitType *pType = unit_manager.unit_type[type_id];
		//----------------------------------- Beginning of moving code ------------------------------------

		if (pType->canmove && pType->BMcode && (!pType->canfly || (mission->getFlags() & MISSION_FLAG_MOVE)))
		{
			Vector3D J,I,K(0.0f, 1.0f, 0.0f);
			Vector3D Target(mission->getTarget().getPos());
			if (mission->getFlags() & MISSION_FLAG_MOVE)
			{
				if (selfmove)
				{
					selfmove = false;
					if (was_moving)
					{
						V.reset();
						if (!(pType->canfly && nb_attached > 0)) // Once charged with units the Atlas cannot land
							stopMovingAnimation();
					}
					was_moving = false;
					requesting_pathfinder = false;
				}
				if (mission.mission() == MISSION_STOP)		// Special mission type : MISSION_STOP
				{
					stopMoving();
					return;
				}
				else
				{
					if (!mission->Path().empty()
						&& ( !(mission->getFlags() & MISSION_FLAG_REFRESH_PATH)
							 || (last_path_refresh < 5.0f
								 && !pType->canfly
								 && (mission->getFlags() & MISSION_FLAG_REFRESH_PATH)) ) )
						Target = mission->Path().Pos();
					else
					{// Look for a path to the target
						if (!mission->Path().empty())	// If we want to refresh the path
						{
							Target = mission->getTarget().getPos();
							mission->Path().clear();
						}
						const float dist = (Target - Pos).sq();
						if ( (mission->getMoveData() <= 0 && dist > 100.0f)
							|| ((SQUARE(mission->getMoveData()) << 6) < dist))
						{
							if ((last_path_refresh >= 5.0f && !requesting_pathfinder)
								|| pType->canfly)
							{
								unsetFlag(mission->Flags(), ~MISSION_FLAG_REFRESH_PATH);

								move_target_computed = mission->getTarget().getPos();
								last_path_refresh = 0.0f;
								if (pType->canfly)
								{
									requesting_pathfinder = false;
									if (mission->getMoveData() <= 0)
										mission->Path() = Pathfinder::directPath(mission->getTarget().getPos());
									else
									{
										Vector3D Dir = mission->getTarget().getPos() - Pos;
										Dir.unit();
										mission->Path() = Pathfinder::directPath(mission->getTarget().getPos() - float(mission->getMoveData() << 3) * Dir);
									}
								}
								else
								{
									requesting_pathfinder = true;
									Pathfinder::instance()->addTask(idx, mission->getMoveData(), Pos, mission->getTarget().getPos());

									if (!(unit_manager.unit_type[type_id]->canfly && nb_attached > 0)) // Once loaded with units the Atlas cannot land
										stopMovingAnimation();
									was_moving = false;
									V.reset();
									V_Angle.reset();
								}
								if (!mission->Path().empty())// Update required data
									Target = mission->Path().Pos();
							}
						}
						else
						{
							stopMoving();
							return;
						}
					}
				}
				if (!mission->Path().empty()) // If we have a path, follow it
				{
					if ((mission->getTarget().getPos() - move_target_computed).sq() >= 10000.0f)			// Follow the target above all...
						mission->Flags() |= MISSION_FLAG_REFRESH_PATH;
					J = Target - Pos;
					J.y = 0.0f;
					const float dist = J.sq();
					if (dist > mission->getLastD() && (dist < 256.0f || (dist < 225.0f && mission->Path().length() <= 1)))
					{
						mission->Path().next();
						mission->setLastD(99999999.0f);
						if (mission->Path().empty()) // End of path reached
						{
							requesting_pathfinder = false;
							J = move_target_computed - Pos;
							J.y = 0.0f;
							if (J.sq() <= 256.0f || flying)
							{
								if (!(mission->getFlags() & MISSION_FLAG_DONT_STOP_MOVE)
									&& (!mission.empty() || mission->mission() != MISSION_PATROL))
									playSound( "arrived1" );
								unsetFlag(mission->Flags(), MISSION_FLAG_MOVE);
							}
							else										// We are not where we are supposed to be !!
								setFlag(mission->Flags(), MISSION_FLAG_REFRESH_PATH);
							if (!( pType->canfly && nb_attached > 0 ))		// Once charged with units the Atlas cannot land
							{
								stopMovingAnimation();
								was_moving = false;
							}
							if (!(mission->getFlags() & MISSION_FLAG_DONT_STOP_MOVE))
								V.reset();		// Stop unit's movement
							if (mission->isStep())      // It's meaningless to try to finish this mission like an ordinary order
							{
								next_mission();
								return;
							}
							if (!(mission->getFlags() & MISSION_FLAG_DONT_STOP_MOVE))
								return;
						}
						else				// If we managed to get there you can forget path refresh order
							unsetFlag(mission->Flags(), MISSION_FLAG_REFRESH_PATH);
					}
					else
						mission->setLastD(dist);
				}
			}

			if (pType->canfly)
			{
				if (mission->getFlags() & MISSION_FLAG_MOVE)
				{
					J = Target - Pos;
				}
				else
					J.reset();
			}
			else
			{
				const float energy = getLocalMapEnergy(cur_px, cur_py);
				if (selfmove || ((mission->getFlags() & MISSION_FLAG_MOVE) && !mission->Path().empty()))
				{
					J = Target;
					computeHeadingBasedOnEnergy(J, mission->getFlags() & MISSION_FLAG_MOVE);
				}
				else if (!(mission->getFlags() & MISSION_FLAG_MOVE) && lastEnergy < energy && !requesting_pathfinder && local)
				{
					switch(mission->mission())
					{
						case MISSION_ATTACK:
						case MISSION_GUARD:
						case MISSION_STANDBY:
						case MISSION_VTOL_STANDBY:
						case MISSION_STOP:
							J = Target;
							computeHeadingBasedOnEnergy(J, false);
							selfmove = J.sq() > 0.1f;
							break;
						default:
							J.reset();
					};
				}
				else
					J.reset();
				lastEnergy = energy;
			}

			if (((mission->getFlags() & MISSION_FLAG_MOVE) && !mission->Path().empty())
				|| (!(mission->getFlags() & MISSION_FLAG_MOVE) && J.sq() > 0.1f))	// Are we still moving ??
			{
				if (!was_moving)
				{
					startMovingAnimation();
					was_moving = true;
				}
				const float dist = (mission->getFlags() & MISSION_FLAG_MOVE) ? (Target - Pos).norm() : 999999.9f;
				if (dist > 0.0f && pType->canfly)
					J = 1.0f / dist * J;

				b_TargetAngle = true;
				f_TargetAngle = acosf( J.z ) * RAD2DEG;
				if (J.x < 0.0f) f_TargetAngle = -f_TargetAngle;

				if (Angle.y - f_TargetAngle >= 360.0f )	f_TargetAngle += 360.0f;
				else if (Angle.y - f_TargetAngle <= -360.0f )	f_TargetAngle -= 360.0f;

				J.z = cosf(Angle.y*DEG2RAD);
				J.x = sinf(Angle.y*DEG2RAD);
				J.y = 0.0f;
				I.z = -J.x;
				I.x = J.z;
				I.y = 0.0f;
				V = (V%K)*K + (V%J)*J;

				Vector3D D(Target.z - Pos.z, 0.0f, Pos.x - Target.x);
				D.unit();
				float speed = sqrtf(V.x * V.x + V.z * V.z);
				const float vsin = fabsf(D % V);
				const float deltaX = 8.0f * vsin / (pType->TurnRate * DEG2RAD);
				const float time_to_stop = speed / pType->BrakeRate;
				const float min_dist = time_to_stop * (speed - pType->BrakeRate * 0.5f * time_to_stop);
				if ((deltaX > dist && vsin * dist > speed * 16.0f)
					|| (min_dist >= dist
//					&& mission->Path().length() == 1
					&& !(mission->getFlags() & MISSION_FLAG_DONT_STOP_MOVE)
					&& ( !mission.hasNext()
						 || (mission(1) != MISSION_MOVE
							 && mission(1) != MISSION_PATROL))))	// Brake if needed
				{
					V = V - pType->BrakeRate * dt * J;
					// Don't go backward
					if (J % V <= 0.0f)
						V.reset();
				}
				else if (speed < pType->MaxVelocity)
				{
					V = V + pType->Acceleration * dt * J;
					speed = V.norm();
					if (speed > pType->MaxVelocity)
						V = pType->MaxVelocity / speed * V;
				}
//				if (!(dist < 15.0f && fabsf( Angle.y - f_TargetAngle ) >= 1.0f))
//				{
//					if (fabsf( Angle.y - f_TargetAngle ) >= 45.0f)
//					{
//						if (J % V > 0.0f && V.norm() > pType->BrakeRate * dt)
//							V = V - ((( fabsf( Angle.y - f_TargetAngle ) - 35.0f ) / 135.0f + 1.0f) * 0.5f * pType->BrakeRate * dt) * J;
//					}
//					else
//					{
//						float speed = V.norm();
//						float time_to_stop = speed / pType->BrakeRate;
//						float min_dist = time_to_stop * (speed-pType->BrakeRate*0.5f*time_to_stop);
//						if (min_dist >= dist
//							&& mission->Path().length() == 1
//							&& !(mission->getFlags() & MISSION_FLAG_DONT_STOP_MOVE)
//							&& ( !mission.hasNext()
//								 || (mission(1) != MISSION_MOVE
//									 && mission(1) != MISSION_PATROL)))	// Brake if needed
//							V = V - pType->BrakeRate * dt * J;
//						else if (speed < pType->MaxVelocity)
//							V = V + pType->Acceleration * dt * J;
//						else
//							V = pType->MaxVelocity / speed * V;
//					}
//				}
//				else
//				{
//					float speed = V.norm();
//					if (speed > pType->MaxVelocity)
//						V = pType->MaxVelocity / speed * V;
//				}
			}
			else if (selfmove)
			{
				selfmove = false;
				if (was_moving)
				{
					V.reset();
					if (!(pType->canfly && nb_attached > 0)) // Once charged with units the Atlas cannot land
						stopMovingAnimation();
				}
				was_moving = false;
				if (!(mission->getFlags() & MISSION_FLAG_MOVE))
					requesting_pathfinder = false;
			}

			NPos = Pos + dt * V;			// Check if the unit can go where V brings it
			if (!Yuni::Math::Zero(was_locked)) // Random move to solve the unit lock problem
			{
				if (V.x > 0.0f)
					NPos.x += float(Math::RandomTable() % 101) * 0.01f;
				else
					NPos.x -= float(Math::RandomTable() % 101) * 0.01f;
				if (V.z > 0.0f)
					NPos.z += float(Math::RandomTable() % 101) * 0.01f;
				else
					NPos.z -= float(Math::RandomTable() % 101) * 0.01f;

				if (was_locked >= 5.0f)
				{
					was_locked = 5.0f;
					setFlag(mission->Flags(), MISSION_FLAG_REFRESH_PATH);			// Refresh path because this shouldn't happen unless
																			// obstacles have moved
				}
			}
			n_px = ((int)(NPos.x) + the_map->map_w_d + 4) >> 3;
			n_py = ((int)(NPos.z) + the_map->map_h_d + 4) >> 3;
			precomputed_position = true;
			bool locked = false;
			if (!flying)
			{
				if (n_px != cur_px || n_py != cur_py) // has something changed ??
				{
					bool place_is_empty = can_be_there( n_px, n_py, type_id, owner_id, idx );
					if (!(flags & 64) && !place_is_empty)
					{
						if (!pType->canfly)
						{
							locked = true;
							// Check some basic solutions first
							if (cur_px != n_px
								&& can_be_there( cur_px, n_py, type_id, owner_id, idx ))
							{
								V.z = !Yuni::Math::Zero(V.z)
									  ? (V.z < 0.0f
										 ? -sqrtf( SQUARE(V.z) + SQUARE(V.x) )
										 : sqrtf( SQUARE(V.z) + SQUARE(V.x) ) )
									  : 0.0f;
								V.x = 0.0f;
								NPos.x = Pos.x;
								n_px = cur_px;
							}
							else if (cur_py != n_py && can_be_there( n_px, cur_py, type_id, owner_id, idx ))
							{
								V.x = !Yuni::Math::Zero(V.x)
									  ? ((V.x < 0.0f)
										 ? -sqrtf(SQUARE(V.z) + SQUARE(V.x))
										 : sqrtf(SQUARE(V.z) + SQUARE(V.x)))
									  : 0.0f;
								V.z = 0.0f;
								NPos.z = Pos.z;
								n_py = cur_py;
							}
							else if (can_be_there( cur_px, cur_py, type_id, owner_id, idx )) {
								V.x = V.y = V.z = 0.0f;		// Don't move since we can't
								NPos = Pos;
								n_px = cur_px;
								n_py = cur_py;
								mission->Flags() |= MISSION_FLAG_MOVE;
								if (fabsf( Angle.y - f_TargetAngle ) <= 0.1f || !b_TargetAngle) // Don't prevent unit from rotating!!
									mission->Path().clear();
							}
							else
								LOG_WARNING("A Unit is blocked !" << __FILE__ << ":" << __LINE__);
						}
						else if (!flying && local )
						{
							if (Pos.x < -the_map->map_w_d
								|| Pos.x > the_map->map_w_d
								|| Pos.z < -the_map->map_h_d
								|| Pos.z > the_map->map_h_d)
							{
								Vector3D target = Pos;
								if (target.x < -the_map->map_w_d + 256)
									target.x = float(-the_map->map_w_d + 256);
								else if (target.x > the_map->map_w_d - 256)
									target.x = float(the_map->map_w_d - 256);
								if (target.z < -the_map->map_h_d + 256)
									target.z = float(-the_map->map_h_d + 256);
								else if (target.z > the_map->map_h_d - 256)
									target.z = float(the_map->map_h_d - 256);
								next_mission();
								add_mission(MISSION_MOVE | MISSION_FLAG_AUTO,&target,true,0,NULL,0,1);		// Stay on map
							}
							else
							{
								if (!can_be_there( cur_px, cur_py, type_id, owner_id, idx ) && !flying)
								{
									NPos = Pos;
									n_px = cur_px;
									n_py = cur_py;
									Vector3D target = Pos;
									target.x += float(((sint32)(Math::RandomTable() & 0x1F)) - 16);		// Look for a place to land
									target.z += float(((sint32)(Math::RandomTable() & 0x1F)) - 16);
									setFlag(mission->Flags(), MISSION_FLAG_MOVE);
									mission->Path() = Pathfinder::directPath( target );
								}
							}
						}
					}
					else if (!(flags & 64)
						&& pType->canfly
								&& (!mission
									|| (mission->mission() != MISSION_MOVE
										&& mission->mission() != MISSION_GUARD
										&& mission->mission() != MISSION_ATTACK)))
						flags |= 64;
				}
				else
				{
					const bool place_is_empty = the_map->check_rect(n_px-(pType->FootprintX>>1),n_py-(pType->FootprintZ>>1),pType->FootprintX,pType->FootprintZ,idx);
					if (!place_is_empty)
					{
						pMutex.unlock();
						clear_from_map();
						pMutex.lock();
						LOG_WARNING("A Unit is blocked ! (probably spawned on something)" << __FILE__ << ":" << __LINE__);
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
			if (was_moving)
				stopMoving();
			was_moving = false;
			requesting_pathfinder = false;
		}

		if (flying && local) // Force planes to stay on map
		{
			if (Pos.x<-the_map->map_w_d || Pos.x>the_map->map_w_d || Pos.z<-the_map->map_h_d || Pos.z>the_map->map_h_d)
			{
				if (Pos.x < -the_map->map_w_d)
					V.x += dt * ( -(float)the_map->map_w_d - Pos.x ) * 0.1f;
				else if (Pos.x > the_map->map_w_d)
					V.x -= dt * ( Pos.x - (float)the_map->map_w_d ) * 0.1f;
				if (Pos.z < -the_map->map_h_d)
					V.z += dt * ( -(float)the_map->map_h_d - Pos.z ) * 0.1f;
				else if (Pos.z > the_map->map_h_d)
					V.z -= dt * ( Pos.z - (float)the_map->map_h_d ) * 0.1f;
				float speed = V.norm();
				if (speed > pType->MaxVelocity && speed > 0.0f)
				{
					V = pType->MaxVelocity / speed * V;
					speed = pType->MaxVelocity;
				}
				if (speed > 0.0f)
				{
					Angle.y = acosf( V.z / speed ) * RAD2DEG;
					if (V.x < 0.0f)
						Angle.y = -Angle.y;
				}
			}
		}

		//----------------------------------- End of moving code ------------------------------------
	}

	int Unit::move(const float dt, const int key_frame)
	{
		pMutex.lock();

		requestedMovingAnimationState = movingAnimation;

		const bool was_open = port[YARD_OPEN] != 0;
		const bool was_flying = flying;
		const sint32	o_px = cur_px;
		const sint32	o_py = cur_py;
		compute_coord = true;
		const Vector3D	old_V = V;			// Store the speed, so we can do some calculations
		bool	b_TargetAngle = false;		// Do we aim, move, ... ?? Need to change unit angle
		float	f_TargetAngle = 0.0f;

		Vector3D NPos = Pos;
		int n_px = cur_px;
		int n_py = cur_py;
		bool precomputed_position = false;

		if (type_id < 0 || type_id >= unit_manager.nb_unit || flags == 0 ) // A unit which cannot exist
		{
			pMutex.unlock();
			LOG_ERROR("Unit::move : A unit which doesn't exist was found");
			return	-1;		// Should NEVER happen
		}

		const UnitType* const pType = unit_manager.unit_type[type_id];

		const float resource_min_factor = TA3D::Math::Min(TA3D::players.energy_factor[owner_id], TA3D::players.metal_factor[owner_id]);

		if (Yuni::Math::Zero(build_percent_left) && pType->isfeature) // Turn this unit into a feature
		{
			if (cur_px > 0 && cur_py > 0 && cur_px < (the_map->bloc_w << 1) && cur_py < (the_map->bloc_h << 1) )
			{
				if (the_map->map_data(cur_px, cur_py).stuff == -1)
				{
					const int type = feature_manager.get_feature_index(pType->Corpse);
					if (type >= 0)
					{
						features.lock();
						the_map->map_data(cur_px, cur_py).stuff=features.add_feature(Pos,type);
						if (the_map->map_data(cur_px, cur_py).stuff == -1)
                            LOG_ERROR("Could not turn `" << pType->Unitname << "` into a feature ! Cannot create the feature");
						else
							features.feature[the_map->map_data(cur_px, cur_py).stuff].angle = Angle.y;
						pMutex.unlock();
						clear_from_map();
						pMutex.lock();
						features.drawFeatureOnMap( the_map->map_data(cur_px, cur_py).stuff );
						features.unlock();
						flags = 4;
					}
					else
                        LOG_ERROR("Could not turn `" << pType->Unitname << "` into a feature ! Feature not found");
				}
			}
			pMutex.unlock();
			return -1;
		}

		if (the_map->ota_data.waterdoesdamage && Pos.y < the_map->sealvl)		// The unit is damaged by the "acid" water
			hp -= dt * float(the_map->ota_data.waterdamage);

		const bool jump_commands = (((idx + key_frame) & 0xF) == 0);		// Saute certaines commandes / Jump some commands so it runs faster with lots of units

		if (Yuni::Math::Zero(build_percent_left) && self_destruct >= 0.0f) // Self-destruction code
		{
			const int old = (int)self_destruct;
			self_destruct -= dt;
			if (old != (int)self_destruct) // Play a sound :-)
				playSound( String("count") << old );
			if (self_destruct <= 0.0f)
			{
				self_destruct = 0.0f;
				hp = 0.0f;
                severity = pType->MaxDamage;
			}
		}

		if (hp <= 0.0f && (local || exploding)) // L'unité est détruite
		{
			if (!mission.empty()
                && !pType->BMcode
				&& (mission->mission() == MISSION_BUILD_2
					|| mission->mission() == MISSION_BUILD))		// It was building something that we must destroy too
			{
				Unit *p = mission->getUnit();
				if (p)
				{
					p->lock();
					p->hp = 0.0f;
					p->built = false;
					p->unlock();
				}
			}
			++death_timer;
			if (death_timer == 255) // Ok we've been dead for a long time now ...
			{
				pMutex.unlock();
				return -1;
			}
			switch(flags & 0x17)
			{
				case 1:				// Début de la mort de l'unité	(Lance le script)
					flags = 4;		// Don't remove the data on the position map because they will be replaced
					if (Yuni::Math::Zero(build_percent_left) && local)
						explode();
					else
						flags = 1;
					death_delay = 1.0f;
					if (flags == 1)
					{
						pMutex.unlock();
						return -1;
					}
					break;
				case 4:				// Vérifie si le script est terminé
					if (death_delay <= 0.0f || !data.explode)
					{
						flags = 1;
						pMutex.unlock();
						clear_from_map();
						return -1;
					}
					death_delay -= dt;
					for(AnimationData::DataVector::iterator i = data.data.begin() ; i != data.data.end() ; ++i)
						if (!(i->flag & FLAG_EXPLODE))// || (data.flag[i]&FLAG_EXPLODE && (data.explosion_flag[i]&EXPLODE_BITMAPONLY)))
							i->flag |= FLAG_HIDE;
					break;
				case 0x14:				// Unit has been captured, this is a FAKE unit, just here to be removed
					flags = 4;
					pMutex.unlock();
					return -1;
				default:		// It doesn't explode (it has been reclaimed for example)
					flags = 1;
					pMutex.unlock();
					clear_from_map();
					return -1;
			}
			if (data.nb_piece > 0 && Yuni::Math::Zero(build_percent_left))
			{
				data.move(dt,the_map->ota_data.gravity);
				if (c_time >= 0.1f)
				{
					c_time = 0.0f;
					for(AnimationData::DataVector::iterator i = data.data.begin() ; i != data.data.end() ; ++i)
						if ((i->flag & FLAG_EXPLODE)
							&& (i->explosion_flag & EXPLODE_BITMAPONLY) != EXPLODE_BITMAPONLY)
						{
							if (i->explosion_flag & EXPLODE_FIRE)
							{
								compute_model_coord();
								particle_engine.make_smoke(Pos + i->pos, fire, 1, 0.0f, 0.0f);
							}
							if (i->explosion_flag & EXPLODE_SMOKE)
							{
								compute_model_coord();
								particle_engine.make_smoke(Pos + i->pos, 0, 1, 0.0f, 0.0f);
							}
						}
				}
			}
			goto script_exec;
		}
		else if (!jump_commands && do_nothing() && local)
			if (Pos.x<-the_map->map_w_d || Pos.x>the_map->map_w_d || Pos.z<-the_map->map_h_d || Pos.z>the_map->map_h_d)
			{
				Vector3D target = Pos;
				if (target.x < -the_map->map_w_d + 256)
					target.x = float(-the_map->map_w_d + 256);
				else if (target.x > the_map->map_w_d - 256)
					target.x = float(the_map->map_w_d - 256);
				if (target.z < -the_map->map_h_d + 256)
					target.z = float(-the_map->map_h_d + 256);
				else if (target.z > the_map->map_h_d - 256)
					target.z = float(the_map->map_h_d - 256);
				add_mission(MISSION_MOVE | MISSION_FLAG_AUTO, &target, true, 0, NULL, 0, 1);		// Stay on map
			}

		flags &= 0xEF;		// To fix a bug

		if (build_percent_left > 0.0f) // Unit isn't finished
		{
			if (!built && local)
			{
				float frac = 1000.0f / float(6 * pType->BuildTime);
				metal_prod = frac * (float)pType->BuildCostMetal;
				frac *= dt;
				hp -= frac * (float)pType->MaxDamage;
				build_percent_left += frac * 100.0f;
			}
			else
				metal_prod = 0.0f;
			goto script_exec;
		}
		else
		{
            if (hp < pType->MaxDamage && pType->HealTime > 0)
			{
				hp += (float)pType->MaxDamage * dt / (float)pType->HealTime;
				if (hp > (float)pType->MaxDamage )
					hp = (float)pType->MaxDamage;
			}
		}

		if (data.nb_piece > 0)
			data.move(dt,units.g_dt);

		if (cloaking && paralyzed <= 0.0f)
		{
			const int conso_energy = (!mission || !(mission->getFlags() & MISSION_FLAG_MOVE) ) ? pType->CloakCost : pType->CloakCostMoving;
			TA3D::players.requested_energy[owner_id] += (float)conso_energy;
			if (players.energy[ owner_id ] >= (energy_cons + (float)conso_energy) * dt)
			{
				energy_cons += (float)conso_energy;
				const int dx = pType->mincloakdistance >> 3;
				const int distance = SQUARE(pType->mincloakdistance);
				// byte mask = 1 << owner_id;
				bool found = false;
				for(int y = cur_py - dx ; y <= cur_py + dx && !found ; y++)
					if (y >= 0 && y < the_map->bloc_h_db - 1)
						for(int x = cur_px - dx ; x <= cur_px + dx ; x++)
							if (x >= 0 && x < the_map->bloc_w_db - 1)
							{
								const int cur_idx = the_map->map_data(x, y).unit_idx;

								if (cur_idx >= 0 && cur_idx < (int)units.max_unit && (units.unit[cur_idx].flags & 1) && units.unit[cur_idx].owner_id != owner_id
									&& distance >= (Pos - units.unit[ cur_idx ].Pos).sq())
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
            if (pType->model)
			{
				Vector3D randVec;
				bool random_vector = false;
				int n = 0;
				for (int base_n = Math::RandomTable() ; !random_vector && n < (int)pType->model->nb_obj ; ++n)
					random_vector = pType->model->mesh->random_pos( &data, (base_n + n) % (int)pType->model->nb_obj, &randVec );
				if (random_vector)
					fx_manager.addElectric( Pos + randVec );
			}
			if (build_percent_left <= 0.0f)
				metal_prod = 0.0f;
		}

		if (attached || paralyzed > 0.0f)
			goto script_exec;

        if (pType->canload && nb_attached > 0)
		{
			int e = 0;
			compute_model_coord();
			for (int i = 0; i + e < nb_attached; ++i)
			{
				if (units.unit[attached_list[i]].flags)
				{
					units.unit[attached_list[i]].Pos = Pos + data.data[link_list[i]].pos;
					units.unit[attached_list[i]].cur_px = ((int)(units.unit[attached_list[i]].Pos.x) + the_map->map_w_d) >> 3;
					units.unit[attached_list[i]].cur_py = ((int)(units.unit[attached_list[i]].Pos.z) + the_map->map_h_d) >> 3;
					units.unit[attached_list[i]].Angle = Angle;
				}
				else
				{
					++e;
					--i;
					continue;
				}
				attached_list[i] = attached_list[i + e];
			}
			nb_attached -= e;
		}

		if (planned_weapons > 0.0f)	// Construit des armes / build weapons
		{
			const float old = planned_weapons - float(int(planned_weapons));
			int idx = -1;
            for (unsigned int i = 0; i < pType->weapon.size(); ++i)
			{
                if (pType->weapon[i] && pType->weapon[i]->stockpile)
				{
					idx = i;
					break;
				}
			}
			if (idx != -1 && !Yuni::Math::Zero(pType->weapon[idx]->reloadtime))
			{
				const float dn = dt / pType->weapon[idx]->reloadtime;
				const float conso_metal = ((float)pType->weapon[idx]->metalpershot) / pType->weapon[idx]->reloadtime;
				const float conso_energy = ((float)pType->weapon[idx]->energypershot) / pType->weapon[idx]->reloadtime;

				TA3D::players.requested_energy[owner_id] += conso_energy;
				TA3D::players.requested_metal[owner_id] += conso_metal;

				if (players.metal[owner_id] >= (metal_cons + conso_metal * resource_min_factor) * dt
					&& players.energy[owner_id] >= (energy_cons + conso_energy * resource_min_factor) * dt)
				{
					metal_cons += conso_metal * resource_min_factor;
					energy_cons += conso_energy * resource_min_factor;
					planned_weapons -= dn * resource_min_factor;
					const float last = planned_weapons - float(int(planned_weapons));
					if ((Yuni::Math::Zero(last) && last != old) || (last > old && old > 0.0f) || planned_weapons <= 0.0f)		// On en a fini une / one is finished
						weapon[idx].stock++;
					if (planned_weapons < 0.0f)
						planned_weapons = 0.0f;
				}
			}
		}

		V_Angle.reset();
		c_time += dt;

		//------------------------------ Beginning of weapon related code ---------------------------------------
		for (unsigned int i = 0; i < weapon.size(); ++i)
		{
            if (pType->weapon[i] == NULL)
				continue;		// Skip that weapon if not present on the unit
			weapon[i].delay += dt;
			weapon[i].time += dt;

			int Query_script;
			int Aim_script;
			int AimFrom_script;
			int Fire_script;
			switch(i)
			{
				case 0:
					Query_script = SCRIPT_QueryPrimary;
					Aim_script = SCRIPT_AimPrimary;
					AimFrom_script = SCRIPT_AimFromPrimary;
					Fire_script = SCRIPT_FirePrimary;
					break;
				case 1:
					Query_script = SCRIPT_QuerySecondary;
					Aim_script = SCRIPT_AimSecondary;
					AimFrom_script = SCRIPT_AimFromSecondary;
					Fire_script = SCRIPT_FireSecondary;
					break;
				case 2:
					Query_script = SCRIPT_QueryTertiary;
					Aim_script = SCRIPT_AimTertiary;
					AimFrom_script = SCRIPT_AimFromTertiary;
					Fire_script = SCRIPT_FireTertiary;
					break;
				default:
					Query_script = SCRIPT_QueryWeapon + (i - 3) * 4;
					Aim_script = SCRIPT_AimWeapon + (i - 3) * 4;
					AimFrom_script = SCRIPT_AimFromWeapon + (i - 3) * 4;
					Fire_script = SCRIPT_FireWeapon + (i - 3) * 4;
			}

			switch (weapon[i].state & 3)
			{
				case WEAPON_FLAG_IDLE:										// Doing nothing, waiting for orders
					if (pType->weapon[ i ]->turret)
						script->setReturnValue( UnitScriptInterface::get_script_name(Aim_script), 0);
					weapon[i].data = -1;
					break;
				case WEAPON_FLAG_AIM:											// Vise une unité / aiming code
                    if (jump_commands)	break;
					if (!(mission->getFlags() & MISSION_FLAG_CAN_ATTACK) || (weapon[i].target == NULL && pType->weapon[i]->toairweapon))
					{
						weapon[i].data = -1;
						weapon[i].state = WEAPON_FLAG_IDLE;
						break;
					}

					if (weapon[i].target == NULL
						|| ((weapon[i].state & WEAPON_FLAG_WEAPON) == WEAPON_FLAG_WEAPON && ((Weapon*)(weapon[i].target))->weapon_id != -1)
						|| ((weapon[i].state & WEAPON_FLAG_WEAPON) != WEAPON_FLAG_WEAPON && (((Unit*)(weapon[i].target))->flags & 1)))
					{
						if ((weapon[i].state & WEAPON_FLAG_WEAPON) != WEAPON_FLAG_WEAPON && weapon[i].target != NULL && ((Unit*)(weapon[i].target))->cloaked
							&& ((const Unit*)(weapon[i].target))->owner_id != owner_id && !((const Unit*)(weapon[i].target))->is_on_radar(byte(1 << owner_id)))
						{
							weapon[i].data = -1;
							weapon[i].state = WEAPON_FLAG_IDLE;
							break;
						}

                        if (!(weapon[i].state & WEAPON_FLAG_COMMAND_FIRE) && pType->weapon[i]->commandfire) // Not allowed to fire
						{
							weapon[i].data = -1;
							weapon[i].state = WEAPON_FLAG_IDLE;
							break;
						}

                        if (weapon[i].delay >= pType->weapon[ i ]->reloadtime || pType->weapon[ i ]->stockpile)
						{
							bool readyToFire = false;

							Unit* const target_unit = (weapon[i].state & WEAPON_FLAG_WEAPON ) == WEAPON_FLAG_WEAPON ? NULL : (Unit*) weapon[i].target;
							const Weapon* const target_weapon = (weapon[i].state & WEAPON_FLAG_WEAPON ) == WEAPON_FLAG_WEAPON ? (Weapon*) weapon[i].target : NULL;

							Vector3D target = target_unit==NULL ? (target_weapon==NULL ? weapon[i].target_pos-Pos : target_weapon->Pos-Pos) : target_unit->Pos-Pos;
							float dist = target.sq();
							int maxdist = 0;
							int mindist = 0;

							if (pType->attackrunlength > 0)
							{
								if (target % V < 0.0f)
								{
									weapon[i].state = WEAPON_FLAG_IDLE;
									weapon[i].data = -1;
									break;	// We're not shooting at the target
								}
								const float t = 2.0f / the_map->ota_data.gravity * fabsf(target.y);
								mindist = (int)sqrtf(t * V.sq())-((pType->attackrunlength + 1) >> 1);
								maxdist = mindist + (pType->attackrunlength);
							}
							else
							{
								if (pType->weapon[ i ]->waterweapon && Pos.y > the_map->sealvl)
								{
									if (target % V < 0.0f)
									{
										weapon[i].state = WEAPON_FLAG_IDLE;
										weapon[i].data = -1;
										break;	// We're not shooting at the target
									}
									const float t = 2.0f / the_map->ota_data.gravity * fabsf(target.y);
									mindist = (int)sqrtf(t * V.sq());
									maxdist = mindist + (pType->weapon[ i ]->range >> 1);
								}
								else
									maxdist = pType->weapon[ i ]->range >> 1;
							}

							if (dist > maxdist * maxdist || dist < mindist * mindist)
							{
								weapon[i].state = WEAPON_FLAG_IDLE;
								weapon[i].data = -1;
								break;	// We're too far from the target
							}

							Vector3D target_translation;
							if (target_unit != NULL)
								for(int k = 0 ; k < 3 ; ++k)		// Iterate to get a better approximation
									target_translation = ((target + target_translation).norm() / pType->weapon[ i ]->weaponvelocity) * (target_unit->V - V);

                            if (pType->weapon[ i ]->turret) 	// Si l'unité doit viser, on la fait viser / if it must aim, we make it aim
							{
                                readyToFire = script->getReturnValue( UnitScriptInterface::get_script_name(Aim_script) );

								int start_piece = weapon[i].aim_piece;
                                if (weapon[i].aim_piece < 0)
                                    weapon[i].aim_piece = start_piece = runScriptFunction(AimFrom_script);
								if (start_piece < 0 || start_piece >= data.nb_piece)
									start_piece = 0;
								compute_model_coord();

								Vector3D target_pos_on_unit;						// Read the target piece on the target unit so we better know where to aim
								target_pos_on_unit.reset();
								const Model* pModel = NULL;
								Vector3D pos_of_target_unit;
								if (target_unit != NULL)
								{
									target_unit->lock();
									if (target_unit->flags & 1)
									{
										pos_of_target_unit = target_unit->Pos;
										pModel = target_unit->model;
										if (weapon[i].data == -1 && pModel && pModel->nb_obj > 0)
											weapon[i].data = sint16(Math::RandomTable() % pModel->nb_obj);
										if (weapon[i].data >= 0)
										{
											target_unit->compute_model_coord();
											if (target_unit->flags & 1)
											{
												if (pModel && (int)target_unit->data.data.size() < weapon[i].data && pModel->mesh->random_pos( &(target_unit->data), weapon[i].data, &target_pos_on_unit ))
													target_pos_on_unit = target_unit->data.data[weapon[i].data].tpos;
											}
										}
										else if (pModel)
											target_pos_on_unit = pModel->center;
									}
									target_unit->unlock();
								}

								target += target_translation - data.data[start_piece].tpos;
                                if (target_unit != NULL)
									target += target_pos_on_unit;

                                if (pType->aim_data[i].check)     // Check angle limitations (not in OTA)
								{
									// Go back in model coordinates so we can compare to the weapon main direction
									Vector3D dir = target * RotateXZY(-Angle.x * DEG2RAD, -Angle.z * DEG2RAD, -Angle.y * DEG2RAD);
									// Check weapon
                                    if (VAngle(dir, pType->aim_data[i].dir) > pType->aim_data[i].Maxangledif)
									{
										weapon[i].state = WEAPON_FLAG_IDLE;
										weapon[i].data = -1;
										break;
									}
								}

								dist = target.norm();
								target = (1.0f / dist) * target;
								const Vector3D	I(0.0f, 0.0f, 1.0f),
												J(1.0f, 0.0f, 0.0f),
												IJ(0.0f, 1.0f, 0.0f);
								Vector3D RT = target;
								RT.y = 0.0f;
								RT.unit();
								float angle = acosf(RT.z) * RAD2DEG;
								if (RT.x < 0.0f) angle =- angle;
								angle -= Angle.y;
								if (angle < -180.0f)        angle += 360.0f;
								else if (angle > 180.0f)    angle -= 360.0f;

								int aiming[] = { (int)(angle * DEG2TA), -4096 };
                                if (pType->weapon[ i ]->ballistic) // Calculs de ballistique / ballistic calculations
								{
									Vector3D D = target_unit == NULL
												 ? ( target_weapon == NULL
													 ? Pos + data.data[start_piece].tpos - weapon[i].target_pos
													 : (Pos + data.data[start_piece].tpos - target_weapon->Pos) )
												 : (Pos + data.data[start_piece].tpos - pos_of_target_unit - target_pos_on_unit);
									D.y = 0.0f;
									float v;
									if (Yuni::Math::Zero(pType->weapon[ i ]->startvelocity))
                                        v = pType->weapon[ i ]->weaponvelocity;
									else
                                        v = pType->weapon[ i ]->startvelocity;
									if (target_unit == NULL)
									{
										if (target_weapon == NULL)
											aiming[1] = (int)(ballistic_angle(v,the_map->ota_data.gravity,D.norm(),(Pos + data.data[start_piece].tpos).y,weapon[i].target_pos.y)*DEG2TA);
										else
											aiming[1] = (int)(ballistic_angle(v,the_map->ota_data.gravity,D.norm(),(Pos + data.data[start_piece].tpos).y,target_weapon->Pos.y)*DEG2TA);
									}
									else if (pModel)
										aiming[1] = (int)(ballistic_angle(v, the_map->ota_data.gravity, D.norm(),
																		  (Pos + data.data[start_piece].tpos).y,
																		  pos_of_target_unit.y + pModel->center.y * 0.5f) * DEG2TA);
								}
								else
								{
									angle = acosf(RT % target) * RAD2DEG;
									if (target.y < 0.0f)
										angle = -angle;
									angle -= Angle.x;
									if (angle > 180.0f)     angle -= 360.0f;
									if (angle < -180.0f)    angle += 360.0f;
									if (fabsf(angle) > 180.0f)
									{
										weapon[i].state = WEAPON_FLAG_IDLE;
										weapon[i].data = -1;
										break;
									}
									aiming[1] = (int)(angle * DEG2TA);
								}
                                if (readyToFire)
                                {
                                    if (pType->weapon[i]->lineofsight)
                                    {
                                        if (!target_unit)
                                        {
                                            if (target_weapon == NULL )
												weapon[i].aim_dir = weapon[i].target_pos - (Pos + data.data[start_piece].tpos);
                                            else
												weapon[i].aim_dir = ((Weapon*)(weapon[i].target))->Pos - (Pos + data.data[start_piece].tpos);
                                        }
                                        else
											weapon[i].aim_dir = ((Unit*)(weapon[i].target))->Pos + target_pos_on_unit - (Pos + data.data[start_piece].tpos);
										weapon[i].aim_dir += target_translation;
                                        weapon[i].aim_dir.unit();
                                    }
                                    else
										weapon[i].aim_dir = cosf((float)aiming[1] * TA2RAD) * (cosf((float)aiming[0] * TA2RAD + Angle.y * DEG2RAD) * I
																							   + sinf((float)aiming[0] * TA2RAD + Angle.y * DEG2RAD) * J)
															+ sinf((float)aiming[1] * TA2RAD) * IJ;
                                }
								else
                                    launchScript(Aim_script, 2, aiming);
                            }
							else
							{
								readyToFire = launchScript(Aim_script, 0, NULL) != 0;
								if (!readyToFire)
									readyToFire = script->getReturnValue( UnitScriptInterface::get_script_name(Aim_script) );
								if (pType->weapon[i]->lineofsight)
								{
									int start_piece = weapon[i].aim_piece;
									if (weapon[i].aim_piece < 0)
										weapon[i].aim_piece = start_piece = runScriptFunction(AimFrom_script);
									if (start_piece < 0 || start_piece >= data.nb_piece)
										start_piece = 0;
									compute_model_coord();

									if (!target_unit)
									{
										if (target_weapon == NULL)
											weapon[i].aim_dir = weapon[i].target_pos - (Pos + data.data[start_piece].tpos);
										else
											weapon[i].aim_dir = ((Weapon*)(weapon[i].target))->Pos - (Pos + data.data[start_piece].tpos);
									}
									else
									{
										Vector3D target_pos_on_unit;						// Read the target piece on the target unit so we better know where to aim
										if (weapon[i].data == -1)
											weapon[i].data = (sint16)target_unit->get_sweet_spot();
										if (weapon[i].data >= 0)
										{
											if (target_unit->model && target_unit->model->mesh->random_pos( &(target_unit->data), weapon[i].data, &target_pos_on_unit ))
												target_pos_on_unit = target_unit->data.data[weapon[i].data].tpos;
										}
										else if (target_unit->model)
											target_pos_on_unit = target_unit->model->center;
										weapon[i].aim_dir = ((Unit*)(weapon[i].target))->Pos + target_pos_on_unit - (Pos + data.data[start_piece].tpos);
									}
									weapon[i].aim_dir += target_translation;
									weapon[i].aim_dir.unit();
								}
								weapon[i].data = -1;
							}
							if (readyToFire)
							{
								weapon[i].time = 0.0f;
								weapon[i].state = WEAPON_FLAG_SHOOT;									// (puis) on lui demande de tirer / tell it to fire
								weapon[i].burst = 0;
							}
						}
					}
					else
					{
						launchScript(SCRIPT_TargetCleared);
						weapon[i].state = WEAPON_FLAG_IDLE;
						weapon[i].data = -1;
					}
					break;
				case WEAPON_FLAG_SHOOT:											// Tire sur une unité / fire!
					if (weapon[i].target == NULL
						|| (( weapon[i].state & WEAPON_FLAG_WEAPON ) == WEAPON_FLAG_WEAPON && ((Weapon*)(weapon[i].target))->weapon_id != -1)
						|| (( weapon[i].state & WEAPON_FLAG_WEAPON ) != WEAPON_FLAG_WEAPON && (((Unit*)(weapon[i].target))->flags&1)))
					{
                        if (weapon[i].burst > 0 && weapon[i].delay < pType->weapon[ i ]->burstrate)
							break;
                        if ((players.metal[owner_id]<pType->weapon[ i ]->metalpershot
                             || players.energy[owner_id]<pType->weapon[ i ]->energypershot)
                            && !pType->weapon[ i ]->stockpile)
						{
							weapon[i].state = WEAPON_FLAG_AIM;		// Pas assez d'énergie pour tirer / not enough energy to fire
							weapon[i].data = -1;
							script->setReturnValue(UnitScriptInterface::get_script_name(Aim_script), 0);
							break;
						}
						if (pType->weapon[ i ]->stockpile && weapon[i].stock <= 0)
						{
							weapon[i].state = WEAPON_FLAG_AIM;		// Plus rien pour tirer / nothing to fire
							weapon[i].data = -1;
							script->setReturnValue(UnitScriptInterface::get_script_name(Aim_script), 0);
							break;
						}
						int start_piece = runScriptFunction(Query_script);
						if (start_piece >= 0 && start_piece < data.nb_piece)
						{
							compute_model_coord();
							if (!pType->weapon[ i ]->waterweapon && Pos.y + data.data[start_piece].tpos.y <= the_map->sealvl)     // Can't shoot from water !!
								break;
							Vector3D Dir = data.data[start_piece].dir;
                            if (pType->weapon[ i ]->vlaunch)
							{
								Dir.x = 0.0f;
								Dir.y = 1.0f;
								Dir.z = 0.0f;
							}
							else if (!pType->weapon[ i ]->turret || Dir.isNull())
								Dir = weapon[i].aim_dir;
							if (i == 3)
							{
								LOG_DEBUG("firing from " << (Pos + data.data[start_piece].tpos).y << " (" << the_map->get_unit_h((Pos + data.data[start_piece].tpos).x, (Pos + data.data[start_piece].tpos).z) << ")");
								LOG_DEBUG("from piece " << start_piece << " (" << Query_script << "," << Aim_script << "," << Fire_script << ")" );
							}

							// SHOOT NOW !!
                            if (pType->weapon[ i ]->stockpile )
								weapon[i].stock--;
							else
							{													// We use energy and metal only for weapons with no prebuilt ammo
								players.c_metal[owner_id] -= (float)pType->weapon[ i ]->metalpershot;
								players.c_energy[owner_id] -= (float)pType->weapon[ i ]->energypershot;
							}
							launchScript( Fire_script );			// Run the fire animation script
                            if (!pType->weapon[ i ]->soundstart.empty())	sound_manager->playSound(pType->weapon[i]->soundstart, &Pos);

							if (weapon[i].target == NULL)
								shoot(-1,Pos + data.data[start_piece].tpos,Dir,i, weapon[i].target_pos );
							else
							{
								if (weapon[i].state & WEAPON_FLAG_WEAPON)
									shoot(((Weapon*)(weapon[i].target))->idx,Pos + data.data[start_piece].tpos,Dir,i, weapon[i].target_pos);
								else
									shoot(((Unit*)(weapon[i].target))->idx,Pos + data.data[start_piece].tpos,Dir,i, weapon[i].target_pos);
							}
							weapon[i].burst++;
                            if (weapon[i].burst >= pType->weapon[i]->burst)
                                weapon[i].burst = 0;
                            weapon[i].delay = 0.0f;
                            weapon[i].aim_piece = -1;
						}
                        if (weapon[i].burst == 0 && pType->weapon[ i ]->commandfire && !pType->weapon[ i ]->dropped)    // Shoot only once
						{
							weapon[i].state = WEAPON_FLAG_IDLE;
							weapon[i].data = -1;
							script->setReturnValue(UnitScriptInterface::get_script_name(Aim_script), 0);
							if (!mission.empty())
								mission->Flags() |= MISSION_FLAG_COMMAND_FIRED;
							break;
						}
						if (weapon[i].target != NULL
							&& (weapon[i].state & WEAPON_FLAG_WEAPON) != WEAPON_FLAG_WEAPON
							&& ((Unit*)(weapon[i].target))->hp > 0)  // La cible est-elle détruite ?? / is target destroyed ??
						{
							if (weapon[i].burst == 0)
							{
								weapon[i].state = WEAPON_FLAG_AIM;
								weapon[i].data = -1;
								weapon[i].time = 0.0f;
								script->setReturnValue(UnitScriptInterface::get_script_name(Aim_script), 0);
							}
						}
						else if (weapon[i].target != NULL || weapon[i].burst == 0)
						{
							launchScript(SCRIPT_TargetCleared);
							weapon[i].state = WEAPON_FLAG_IDLE;
							weapon[i].data = -1;
							script->setReturnValue(UnitScriptInterface::get_script_name(Aim_script), 0);
						}
					}
					else
					{
						launchScript(SCRIPT_TargetCleared);
						weapon[i].state = WEAPON_FLAG_IDLE;
						weapon[i].data = -1;
					}
					break;
			}
		}

		//---------------------------- Beginning of mission execution code --------------------------------------

		if (!mission)
			was_moving = false;

		if (!mission.empty())
		{
			mission->setTime( mission->getTime() + dt );
			last_path_refresh += dt;

			followPath(dt, b_TargetAngle, f_TargetAngle, NPos, n_px, n_py, precomputed_position);

			switch(mission->mission())						// Commandes générales / General orders
			{
				case MISSION_WAIT:					// Wait for a specified time (campaign)
					mission->setFlags(0);			// Don't move, do not shoot !! just wait
					if (mission->getTime() >= (float)mission->getData() * 0.001f )	// Done :)
						next_mission();
					break;
				case MISSION_WAIT_ATTACKED:			// Wait until a specified unit is attacked (campaign)
					if (mission->getData() < 0
						|| mission->getData() >= (int)units.max_unit
						|| !(units.unit[ mission->getData() ].flags & 1))
						next_mission();
					else if (units.unit[ mission->getData() ].attacked)
						next_mission();
					break;
				case MISSION_GET_REPAIRED:
					if (!mission->getTarget().isValid())
					{
						next_mission();
						break;
					}
					if (mission->getTarget().isUnit()
						&& mission->getUnit()
						&& (mission->getUnit()->flags & 1))
					{
						Unit *target_unit = mission->getUnit();

						if (!(mission->getFlags() & MISSION_FLAG_PAD_CHECKED))
						{
							mission->Flags() |= MISSION_FLAG_PAD_CHECKED;
							int param[] = { 0, 1 };
							target_unit->runScriptFunction( SCRIPT_QueryLandingPad, 2, param );
							mission->setData(param[ 0 ]);
						}

						target_unit->compute_model_coord();
						const int piece_id = mission->getData() >= 0 ? mission->getData() : (-mission->getData() - 1);
						const Vector3D target = target_unit->Pos + target_unit->data.data[piece_id].pos;

						Vector3D Dir = target - Pos;
						Dir.y = 0.0f;
						const float dist = Dir.sq();
						const int maxdist = 6;
						if (dist > maxdist * maxdist && pType->BMcode)	// Si l'unité est trop loin du chantier
						{
							unsetFlag(mission->Flags(), MISSION_FLAG_BEING_REPAIRED);
							c_time = 0.0f;
							setFlag(mission->Flags(), MISSION_FLAG_MOVE);
							mission->setMoveData( maxdist * 8 / 80 );
							if (!mission->Path().empty())
								mission->Path().setPos( target );			// Update path in real time!
						}
						else if (!(mission->getFlags() & MISSION_FLAG_MOVE))
						{
							b_TargetAngle = true;
							f_TargetAngle = target_unit->Angle.y;
							if (mission->getData() >= 0)
							{
								setFlag(mission->Flags(), MISSION_FLAG_BEING_REPAIRED);
								Dir = target - Pos;
								Pos = Pos + 3.0f * dt * Dir;
								Pos.x = target.x;
								Pos.z = target.z;
								if (Dir.sq() < 3.0f)
								{
									target_unit->lock();
									if (target_unit->pad1 != 0xFFFF && target_unit->pad2 != 0xFFFF)		// We can't land here
									{
										target_unit->unlock();
										next_mission();
										if (!mission.empty() && mission->mission() == MISSION_STOP)		// Don't stop we were patroling
											next_mission();
										break;
									}
									if (target_unit->pad1 == 0xFFFF)			// tell others we're here
										target_unit->pad1 = (uint16)piece_id;
									else
										target_unit->pad2 = (uint16)piece_id;
									target_unit->unlock();
									mission->setData( -mission->getData() - 1 );
								}
							}
							else
							{						// being repaired
								Pos = target;
								V.reset();

								if (target_unit->port[ ACTIVATION ])
								{
									const float conso_energy = float(unit_manager.unit_type[target_unit->type_id]->WorkerTime * pType->BuildCostEnergy) / float(pType->BuildTime);
									TA3D::players.requested_energy[owner_id] += conso_energy;
									if (players.energy[owner_id] >= (energy_cons + conso_energy * TA3D::players.energy_factor[owner_id]) * dt)
									{
										target_unit->lock();
										target_unit->energy_cons += conso_energy * TA3D::players.energy_factor[owner_id];
										target_unit->unlock();
										hp += dt * TA3D::players.energy_factor[owner_id] * float(unit_manager.unit_type[target_unit->type_id]->WorkerTime * pType->MaxDamage) / (float)pType->BuildTime;
									}
                                    if (hp >= pType->MaxDamage) // Unit has been repaired
									{
										hp = (float)pType->MaxDamage;
										target_unit->lock();
										if (target_unit->pad1 == piece_id )			// tell others we've left
											target_unit->pad1 = 0xFFFF;
										else target_unit->pad2 = 0xFFFF;
										target_unit->unlock();
										next_mission();
										if (!mission.empty() && mission->mission() == MISSION_STOP)		// Don't stop we were patroling
											next_mission();
										break;
									}
									built = true;
								}
							}
						}
						else
							stopMoving();
					}
					else
						next_mission();
					break;
				case MISSION_STANDBY_MINE:		// Don't even try to do something else, the unit must die !!
					if (self_destruct < 0.0f)
					{
                        int dx = ((pType->SightDistance+(int)(h+0.5f))>>3) + 1;
						int enemy_idx=-1;
						int sx=Math::RandomTable()&1;
						int sy=Math::RandomTable()&1;
						// byte mask=1<<owner_id;
						for(int y = cur_py - dx + sy ; y <= cur_py + dx ; y += 2)
						{
							if (y >= 0 && y < the_map->bloc_h_db - 1)
								for(int x = cur_px - dx + sx ; x <= cur_px + dx ; x += 2)
									if (x >= 0 && x < the_map->bloc_w_db - 1)
									{
										const int cur_idx = the_map->map_data(x, y).unit_idx;
										if (cur_idx >= 0 && cur_idx < (int)units.max_unit && (units.unit[cur_idx].flags & 1) && units.unit[cur_idx].owner_id != owner_id
											&& unit_manager.unit_type[units.unit[cur_idx].type_id]->ShootMe )		// This unit is on the sight_map since dx = sightdistance !!
										{
											enemy_idx = cur_idx;
											break;
										}
									}
							if (enemy_idx >= 0)	break;
							sx ^= 1;
						}
						if (enemy_idx >= 0)					// Annihilate it !!!
							toggle_self_destruct();
					}
					break;
				case MISSION_UNLOAD:
					if (nb_attached > 0 )
					{
						Vector3D Dir = mission->getTarget().getPos() - Pos;
						Dir.y = 0.0f;
						float dist = Dir.sq();
						int maxdist = 0;
						if (pType->TransportMaxUnits == 1)		// Code for units like the arm atlas
							maxdist = 3;
						else
							maxdist = pType->SightDistance;
						if (dist > maxdist * maxdist && pType->BMcode)	// Si l'unité est trop loin du chantier
						{
							c_time = 0.0f;
							mission->Flags() |= MISSION_FLAG_MOVE;
							mission->setMoveData( maxdist * 8 / 80 );
						}
						else if (!(mission->getFlags() & MISSION_FLAG_MOVE) )
						{
							if (mission->getLastD() >= 0.0f)
							{
								if (pType->TransportMaxUnits == 1)// Code for units like the arm atlas
								{
									if (attached_list[0] >= 0 && attached_list[0] < (int)units.max_unit				// Check we can do that
										&& units.unit[ attached_list[0] ].flags && can_be_built( Pos, units.unit[ attached_list[0] ].type_id, owner_id ) ) {
										launchScript(SCRIPT_EndTransport);

										Unit *target_unit = &(units.unit[ attached_list[0] ]);
										target_unit->attached = false;
										target_unit->hidden = false;
										nb_attached = 0;
										pMutex.unlock();
										target_unit->draw_on_map();
										pMutex.lock();
									}
									else if (attached_list[0] < 0 || attached_list[0] >= (int)units.max_unit
											 || units.unit[ attached_list[0] ].flags == 0 )
										nb_attached = 0;

									next_mission();
								}
								else
								{
									if (attached_list[ nb_attached - 1 ] >= 0 && attached_list[ nb_attached - 1 ] < (int)units.max_unit				// Check we can do that
										&& units.unit[ attached_list[ nb_attached - 1 ] ].flags && can_be_built( mission->getTarget().getPos(), units.unit[ attached_list[ nb_attached - 1 ] ].type_id, owner_id ) ) {
										const int idx = attached_list[ nb_attached - 1 ];
										int param[] = { idx, PACKXZ( mission->getTarget().getPos().x * 2.0f + (float)the_map->map_w, mission->getTarget().getPos().z * 2.0f + (float)the_map->map_h ) };
										launchScript(SCRIPT_TransportDrop, 2, param);
									}
									else if (attached_list[ nb_attached - 1 ] < 0 || attached_list[ nb_attached - 1 ] >= (int)units.max_unit
											 || units.unit[ attached_list[ nb_attached - 1 ] ].flags == 0 )
										nb_attached--;
								}
								mission->setLastD(-1.0f);
							}
							else
							{
								//                                if (!is_running(get_script_index(SCRIPT_TransportDrop)) && port[ BUSY ] == 0.0f )
								if (port[ BUSY ] == 0)
									next_mission();
							}
						}
						else
							stopMoving();
					}
					else
						next_mission();
					break;
				case MISSION_LOAD:
					if (!mission->getTarget().isValid())
					{
						next_mission();
						break;
					}
					if (mission->getUnit())
					{
						Unit *target_unit = mission->getUnit();
						if (!(target_unit->flags & 1))
						{
							next_mission();
							break;
						}
						Vector3D Dir = target_unit->Pos - Pos;
						Dir.y = 0.0f;
						float dist = Dir.sq();
						int maxdist = 0;
						if (pType->TransportMaxUnits == 1)		// Code for units like the arm atlas
							maxdist = 3;
						else
							maxdist = pType->SightDistance;
						if (dist > maxdist * maxdist && pType->BMcode)	// Si l'unité est trop loin du chantier
						{
							c_time = 0.0f;
							mission->Flags() |= MISSION_FLAG_MOVE;
							mission->setMoveData( maxdist * 8 / 80 );
						}
						else if (!(mission->getFlags() & MISSION_FLAG_MOVE))
						{
							if (mission->getLastD() >= 0.0f)
							{
								if (pType->TransportMaxUnits == 1)  		// Code for units like the arm atlas
								{
									if (nb_attached == 0)
									{
										//										int param[] = { (int)((Pos.y - target_unit->Pos.y - target_unit->model->top)*2.0f) << 16 };
										int param[] = { (int)((Pos.y - target_unit->Pos.y)*2.0f) << 16 };
										launchScript(SCRIPT_BeginTransport, 1, param);
										runScriptFunction( SCRIPT_QueryTransport, 1, param);
										target_unit->attached = true;
										link_list[nb_attached] = (short)param[0];
										target_unit->hidden = param[0] < 0;
										attached_list[nb_attached++] = target_unit->idx;
										target_unit->clear_from_map();
									}
									next_mission();
								}
								else
								{
									if (nb_attached >= pType->TransportMaxUnits)
									{
										next_mission();
										break;
									}
									int param[]= { target_unit->idx };
									launchScript(SCRIPT_TransportPickup, 1, param);
								}
								mission->setLastD(-1.0f);
							}
							else
							{
								if (port[ BUSY ] == 0)
									next_mission();
							}
						}
						else
							stopMoving();
					}
					else
						next_mission();
					break;
				case MISSION_CAPTURE:
				case MISSION_REVIVE:
				case MISSION_RECLAIM:
					selfmove = false;
					if (!mission->getTarget().isValid())
					{
						next_mission();
						break;
					}
					if (mission->getUnit())		// Récupère une unité / It's a unit
					{
						Unit *target_unit = mission->getUnit();
						if (target_unit->flags & 1)
						{
							if (mission->mission() == MISSION_CAPTURE)
							{
								if (unit_manager.unit_type[target_unit->type_id]->commander || target_unit->owner_id == owner_id)
								{
									playSound( "cant1" );
									next_mission();
									break;
								}
								if (!(mission->getFlags() & MISSION_FLAG_TARGET_CHECKED))
								{
									mission->Flags() |= MISSION_FLAG_TARGET_CHECKED;
									mission->setData( Math::Min(unit_manager.unit_type[target_unit->type_id]->BuildCostMetal * 100, 10000) );
								}
							}
							Vector3D Dir = target_unit->Pos-Pos;
							Dir.y = 0.0f;
							float dist = Dir.sq();
							UnitType *tType = target_unit->type_id == - 1 ? NULL : unit_manager.unit_type[target_unit->type_id];
							int tsize = (tType == NULL) ? 0 : ((tType->FootprintX + tType->FootprintZ) << 2);
							int maxdist = (mission->mission() == MISSION_CAPTURE ? (int)(pType->SightDistance) : (int)(pType->BuildDistance))
										  + tsize;
                            if (dist > maxdist * maxdist && pType->BMcode)	// Si l'unité est trop loin du chantier
							{
								c_time = 0.0f;
								if (!(mission->Flags() & MISSION_FLAG_MOVE))
									mission->Flags() |= MISSION_FLAG_REFRESH_PATH | MISSION_FLAG_MOVE;
								mission->setMoveData(Math::Max(maxdist * 7 / 80, (tsize + 7) >> 3));
								mission->setLastD(0.0f);
							}
							else if (!(mission->getFlags() & MISSION_FLAG_MOVE))
							{
								if (mission->getLastD() >= 0.0f)
								{
									start_building(target_unit->Pos - Pos);
									mission->setLastD(-1.0f);
								}

								if (pType->BMcode && port[ INBUILDSTANCE ] != 0)
								{
									if (local && network_manager.isConnected() && nanolathe_target < 0 ) {		// Synchronize nanolathe emission
										nanolathe_target = target_unit->idx;
										g_ta3d_network->sendUnitNanolatheEvent( idx, target_unit->idx, false, mission->mission() == MISSION_RECLAIM );
									}

									playSound( "working" );
									if (mission->mission() == MISSION_CAPTURE)
									{
										mission->setData( mission->getData() - (int)(dt * 1000.0f + 0.5f) );
										if (mission->getData() <= 0 )			// Unit has been captured
										{
											pMutex.unlock();

											target_unit->clear_from_map();
											target_unit->lock();

											Unit *new_unit = (Unit*) create_unit( target_unit->type_id, owner_id, target_unit->Pos);
											if (new_unit)
											{
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
											next_mission();
										}
									}
									else
									{
										// Récupère l'unité
										const UnitType* const pTargetType = unit_manager.unit_type[target_unit->type_id];
										const float recup = std::min(dt * float(pType->WorkerTime * pTargetType->MaxDamage) * target_unit->damage_modifier() / ((isVeteran() ? 5.5f : 11.0f) * (float)pTargetType->BuildCostMetal),
																	 target_unit->hp);

										target_unit->hp -= recup;
										if (dt > 0.0f)
											metal_prod += recup * (float)pTargetType->BuildCostMetal / (dt * (float)pTargetType->MaxDamage);
										if (target_unit->hp <= 0.0f)		// Work done
										{
											target_unit->flags |= 0x10;			// This unit is being reclaimed it doesn't explode!
											next_mission();
										}
									}
								}
							}
							else
								stopMoving();
						}
						else
							next_mission();
					}
					else if (mission->getData() >= 0 && mission->getData() < features.max_features)	// Reclaim a feature/wreckage
					{
						features.lock();
						if (features.feature[mission->getData()].type <= 0)
						{
							features.unlock();
							next_mission();
							break;
						}
						bool feature_locked = true;

						Vector3D Dir = features.feature[mission->getData()].Pos - Pos;
						Dir.y = 0.0f;
						mission->getTarget().setPos(features.feature[mission->getData()].Pos);
						float dist = Dir.sq();
						Feature *pFeature = feature_manager.getFeaturePointer(features.feature[mission->getData()].type);
						int tsize = pFeature == NULL ? 0 : ((pFeature->footprintx + pFeature->footprintz) << 2);
						int maxdist = (mission->mission() == MISSION_REVIVE ? (int)(pType->SightDistance) : (int)(pType->BuildDistance))
										  + tsize;
						if (dist > maxdist * maxdist && pType->BMcode)	// If the unit is too far from its target
						{
							c_time = 0.0f;
							if (!(mission->Flags() & MISSION_FLAG_MOVE))
								mission->Flags() |= MISSION_FLAG_REFRESH_PATH | MISSION_FLAG_MOVE;
							mission->setMoveData( Math::Max(maxdist * 7 / 80, (tsize + 7) >> 3) );
							mission->setLastD(0.0f);
						}
						else if (!(mission->getFlags() & MISSION_FLAG_MOVE))
						{
							if (mission->getLastD() >= 0.0f)
							{
								start_building(features.feature[mission->getData()].Pos - Pos);
								mission->setLastD(-1.0f);
							}
                            if (pType->BMcode && port[ INBUILDSTANCE ] != 0)
							{
								if (local && network_manager.isConnected() && nanolathe_target < 0)		// Synchronize nanolathe emission
								{
									nanolathe_target = mission->getData();
									g_ta3d_network->sendUnitNanolatheEvent( idx, mission->getData(), true, true );
								}

								playSound( "working" );
								// Reclaim the object
								const Feature *feature = feature_manager.getFeaturePointer(features.feature[mission->getData()].type);
								const float recup = std::min(dt * float(pType->WorkerTime * feature->damage) / (5.5f * (float)feature->metal), features.feature[mission->getData()].hp);
								features.feature[mission->getData()].hp -= recup;
								if (dt > 0.0f && mission->mission() == MISSION_RECLAIM)
								{
									metal_prod += recup * (float)feature->metal / (dt * (float)feature->damage);
									energy_prod += recup * (float)feature->energy / (dt * (float)feature->damage);
								}
								if (features.feature[mission->getData()].hp <= 0.0f)		// Job done
								{
									features.removeFeatureFromMap( mission->getData() );		// Remove the object from map

									if (mission->mission() == MISSION_REVIVE
										&& !feature->name.empty())			// Creates the corresponding unit
									{
										bool success = false;
										String wreckage_name = feature->name;
										wreckage_name = Substr(wreckage_name, 0, wreckage_name.length() - 5 );		// Remove the _dead/_heap suffix

										int wreckage_type_id = unit_manager.get_unit_index( wreckage_name );
										Vector3D obj_pos = features.feature[mission->getData()].Pos;
										float obj_angle = features.feature[mission->getData()].angle;
										features.unlock();
										feature_locked = false;
										if (network_manager.isConnected() )
											g_ta3d_network->sendFeatureDeathEvent( mission->getData() );
										features.delete_feature(mission->getData());			// Delete the object

										if (wreckage_type_id >= 0)
										{
											pMutex.unlock();
											Unit *unit_p = (Unit*) create_unit( wreckage_type_id, owner_id, obj_pos );

											if (unit_p)
											{
												unit_p->lock();

												unit_p->Angle.y = obj_angle;
												unit_p->hp = 0.01f;					// Need to be repaired :P
												unit_p->build_percent_left = 0.0f;	// It's finished ...
												unit_p->unlock();
												unit_p->draw_on_map();
											}
											pMutex.lock();

											if (unit_p)
											{
												mission->setMissionType(MISSION_REPAIR);		// Now let's repair what we've resurrected
												mission->getTarget().set(Mission::Target::TargetUnit, unit_p->idx, unit_p->ID);
												mission->setData(1);
												success = true;
											}
										}
										if (!success)
										{
											playSound("cant1");
											next_mission();
										}
									}
									else
									{
										features.unlock();
										feature_locked = false;
										if (network_manager.isConnected())
											g_ta3d_network->sendFeatureDeathEvent( mission->getData() );
										features.delete_feature(mission->getData());			// Delete the object
										next_mission();
									}
								}
							}
						}
						else
							stopMoving();
						if (feature_locked)
							features.unlock();
					}
					else
						next_mission();
					break;
				case MISSION_GUARD:
					if (jump_commands)	break;
					if (!mission->getTarget().isValid())
					{
						next_mission();
						break;
					}
					if (mission->getUnit()
						&& (mission->getUnit()->flags & 1)
						&& mission->getUnit()->owner_id == owner_id)
					{		// On ne défend pas n'importe quoi
                        if (pType->Builder)
						{
							if (mission->getUnit()->build_percent_left > 0.0f
								|| mission->getUnit()->hp < unit_manager.unit_type[mission->getUnit()->type_id]->MaxDamage) // Répare l'unité
							{
								add_mission(MISSION_REPAIR | MISSION_FLAG_AUTO, &mission->getUnit()->Pos,true,0,mission->getUnit());
								break;
							}
							else
								if (!mission->getUnit()->mission.empty()
									&& (mission->getUnit()->mission->mission() == MISSION_BUILD_2
										|| mission->getUnit()->mission->mission() == MISSION_REPAIR)) // L'aide à construire
								{
									add_mission(MISSION_REPAIR | MISSION_FLAG_AUTO, &mission->getUnit()->mission->getTarget().getPos(),true,0,mission->getUnit()->mission->getUnit());
									break;
								}
						}
                        if (pType->canattack)
						{
							if (!mission->getUnit()->mission.empty()
								&& mission->getUnit()->mission->mission() == MISSION_ATTACK) // L'aide à attaquer
							{
								add_mission(MISSION_ATTACK | MISSION_FLAG_AUTO, &mission->getUnit()->mission->getTarget().getPos(),true,0,mission->getUnit()->mission->getUnit());
								break;
							}
						}
						if (!pType->canfly)
						{
							if (((Vector3D)(Pos - mission->getUnit()->Pos)).sq() >= 25600.0f) // On reste assez près
							{
								mission->Flags() |= MISSION_FLAG_MOVE;// | MISSION_FLAG_REFRESH_PATH;
								mission->setMoveData(10);
								c_time = 0.0f;
								break;
							}
							else if (mission->getFlags() & MISSION_FLAG_MOVE)
								stopMoving();
						}
					}
					else
						next_mission();
					break;
				case MISSION_PATROL:					// Mode patrouille
					{
						pad_timer += dt;

						if (!mission.hasNext())
							add_mission(MISSION_PATROL | MISSION_FLAG_AUTO,&Pos,false,0,NULL,MISSION_FLAG_CAN_ATTACK,0,0);	// Retour à la case départ après l'éxécution de tous les ordres / back to beginning

                        if (pType->CanReclamate                                                 // Auto reclaim things on the battle field when needed
                            && (players.r_energy[owner_id] >= players.energy_t[owner_id]
                                || players.r_metal[owner_id] >= players.metal_t[owner_id]))
                        {
							const bool energyLack = players.r_energy[owner_id] >= players.energy_t[owner_id];
							const bool metalLack = players.r_metal[owner_id] >= players.metal_t[owner_id];
							const int dx = pType->SightDistance >> 3;
							const int dx2 = SQUARE(dx);
							int feature_idx = -1;
							const int sx = Math::RandomTable() & 0xF;
							const int sy = Math::RandomTable() & 0xF;
                            for(int y = cur_py - dx + sy ; y <= cur_py + dx && feature_idx == -1 ; y += 0x8)
                            {
								if (y >= 0 && y < the_map->bloc_h_db - 1)
                                {
                                    for(int x = cur_px - dx + sx ; x <= cur_px + dx && feature_idx == -1 ; x += 0x8)
                                    {
                                        if (SQUARE(cur_px - x) + SQUARE(cur_py - y) > dx2)  continue;
										if (x >= 0 && x < the_map->bloc_w_db - 1)
                                        {
											const int cur_idx = the_map->map_data(x, y).stuff;
                                            if (cur_idx >= 0)      // There is a feature
                                            {
                                                Feature *pFeature = feature_manager.getFeaturePointer(features.feature[cur_idx].type);
                                                if (pFeature && pFeature->autoreclaimable
                                                    && ((pFeature->metal > 0 && metalLack)
                                                        || (pFeature->energy > 0 && energyLack)))
                                                    feature_idx = cur_idx;
                                            }
                                        }
                                    }
                                }
                            }
                            if (feature_idx >= 0)           // We've something to recycle :P
                            {
                                add_mission(MISSION_RECLAIM, &(features.feature[feature_idx].Pos), true, feature_idx, NULL);
                                break;
                            }
                        }
						if (pType->Builder)									// Repair units if we can
						{
							const int dx = pType->SightDistance;
							std::deque<UnitTKit::T> friends;
							units.kdTreeFriends[owner_id]->maxDistanceQuery(friends, Pos, (float)dx);
							bool done = false;

							for(std::deque<UnitTKit::T>::const_iterator i = friends.begin() ; i != friends.end() ; ++i)
							{
								const Unit* const pUnit = i->first;
								if (pUnit == this)		// No self-healing
									continue;
								const int friend_type_id = pUnit->type_id;
								if (friend_type_id == -1)
									continue;
								const UnitType* const pFriendType = unit_manager.unit_type[friend_type_id];
								if (pFriendType->BMcode && pUnit->build_percent_left > 0.0f)		// Don't help factories
									continue;
								if ((pUnit->flags & 1) && pUnit->hp < pFriendType->MaxDamage)
								{
									add_mission(MISSION_REPAIR, &(pUnit->Pos), true, 0, (void*)pUnit);
									done = true;
									break;
								}
							}
							if (done)
								break;
						}

						mission->Flags() |= MISSION_FLAG_CAN_ATTACK;
						if (pType->canfly) // Don't stop moving and check if it can be repaired
						{
							mission->Flags() |= MISSION_FLAG_DONT_STOP_MOVE;

							if (hp < (float)pType->MaxDamage * 0.75f && !attacked && pad_timer >= 5.0f ) // Check if a repair pad is free
							{
								bool attacking = false;
								for (uint32 i = 0 ; i < weapon.size() ; ++i)
								{
									if (weapon[i].state != WEAPON_FLAG_IDLE)
									{
										attacking = true;
										break;
									}
								}
								if (!attacking)
								{
									pad_timer = 0.0f;
									bool going_to_repair_pad = false;
									std::deque<UnitTKit::T> repair_pads;
									units.kdTreeRepairPads[owner_id]->maxDistanceQuery(repair_pads, Pos, pType->ManeuverLeashLength);
									for (std::deque<UnitTKit::T>::const_iterator i = repair_pads.begin(); i != repair_pads.end() && !going_to_repair_pad ; ++i)
									{
										const Unit* const pUnit = i->first;
										if ((pUnit->pad1 == 0xFFFF || pUnit->pad2 == 0xFFFF) && Yuni::Math::Zero(pUnit->build_percent_left)) // He can repair us :)
											{
												add_mission( MISSION_GET_REPAIRED | MISSION_FLAG_AUTO, &(pUnit->Pos), true, 0, (void*)pUnit);
												going_to_repair_pad = true;
											}
									}
									if (going_to_repair_pad)
										break;
								}
							}
						}

						if ((mission->getFlags() & MISSION_FLAG_MOVE) == 0) // Monitor the moving process
						{
							if (!pType->canfly
								|| (!mission.hasNext() || mission->mission() != MISSION_PATROL ))
							{
								V.reset();			// Stop the unit
								if (precomputed_position)
								{
									NPos = Pos;
									n_px = cur_px;
									n_py = cur_py;
								}
							}

							mission.add(mission.front());
							mission.back().Flags() |= MISSION_FLAG_MOVE;
							mission.back().Path().clear();

							MissionStack::iterator cur = mission.begin();
							for ( ++cur ; cur != mission.end() && cur->mission() != MISSION_PATROL ; ++cur )
							{
								mission.add(*cur);
								mission.back().Path().clear();
							}

							next_mission();
						}
					}
					break;
				case MISSION_STANDBY:
				case MISSION_VTOL_STANDBY:
					if (jump_commands)	break;
					if (mission->getData() > 5)
					{
						if (mission.hasNext())		// If there is a mission after this one
						{
							next_mission();
							if (!mission.empty()
								&& (mission->mission() == MISSION_STANDBY
									|| mission->mission() == MISSION_VTOL_STANDBY))
								mission->setData(0);
						}
					}
					else
						mission->setData(mission->getData() + 1);
					break;
				case MISSION_ATTACK:										// Attaque une unité / attack a unit
					if (!mission->getTarget().isValid())
					{
						next_mission();
						break;
					}
					{
						Unit *target_unit = mission->getUnit();
						Weapon *target_weapon = mission->getWeapon();
						if ((target_unit != NULL && (target_unit->flags & 1))
							|| (target_weapon != NULL && target_weapon->weapon_id != -1)
							|| mission->getTarget().isStatic())
						{
							if (target_unit)				// Check if we can target the unit
							{
								const byte mask = byte(1 << owner_id);
								if (target_unit->cloaked && !target_unit->is_on_radar( mask ))
								{
									for (uint32 i = 0 ; i < weapon.size() ; ++i)
										if (weapon[ i ].target == target_unit)		// Stop shooting
											weapon[ i ].state = WEAPON_FLAG_IDLE;
									next_mission();
									break;
								}
							}

							if (jump_commands && mission->getData() != 0
								&& pType->attackrunlength == 0)	break;					// Just do basic checks every tick, and advanced ones when needed

							if (weapon.size() == 0
                                && !pType->kamikaze)		// Check if this units has weapons
							{
								next_mission();
								break;
							}

							Vector3D Dir = mission->getTarget().getPos() - Pos;
							Dir.y = 0.0f;
							float dist = Dir.sq();
							int maxdist = 0;
							int mindist = 0xFFFFF;

                            //                            if (target_unit != NULL && unit_manager.unit_type[target_unit->type_id]->checkCategory( pType->BadTargetCategory ))
                            if (target_unit != NULL && unit_manager.unit_type[target_unit->type_id]->checkCategory( pType->NoChaseCategory ))
							{
								next_mission();
								break;
							}

							for (uint32 i = 0 ; i < weapon.size() ; ++i)
							{
								if (pType->weapon[ i ] == NULL || pType->weapon[ i ]->interceptor)
									continue;
								int cur_mindist;
								int cur_maxdist;
								bool allowed_to_fire = true;
								if (pType->attackrunlength > 0)
								{
									if (Dir % V < 0.0f)
										allowed_to_fire = false;
									const float t = 2.0f / the_map->ota_data.gravity * fabsf(Pos.y - mission->getTarget().getPos().y);
									cur_mindist = (int)sqrtf(t * V.sq()) - ((pType->attackrunlength + 1)>>1);
									cur_maxdist = cur_mindist + pType->attackrunlength;
								}
								else if (pType->weapon[ i ]->waterweapon && Pos.y > the_map->sealvl)
								{
									if (Dir % V < 0.0f)
										allowed_to_fire = false;
									const float t = 2.0f / the_map->ota_data.gravity * fabsf(Pos.y - mission->getTarget().getPos().y);
									cur_maxdist = (int)sqrtf(t*V.sq()) + (pType->weapon[ i ]->range >> 1);
									cur_mindist = 0;
								}
								else
								{
									cur_maxdist = pType->weapon[ i ]->range >> 1;
									cur_mindist = 0;
								}
								if (maxdist < cur_maxdist)	maxdist = cur_maxdist;
								if (mindist > cur_mindist)	mindist = cur_mindist;
                                if (allowed_to_fire && dist >= cur_mindist * cur_mindist && dist <= cur_maxdist * cur_maxdist && !pType->weapon[ i ]->interceptor)
								{
//									if (( (weapon[i].state & 3) == WEAPON_FLAG_IDLE || ( (weapon[i].state & 3) != WEAPON_FLAG_IDLE && weapon[i].target != mission->p ) )
//										&& ( target_unit == NULL || ( (!pType->weapon[ i ]->toairweapon
//																	   || ( pType->weapon[ i ]->toairweapon && target_unit->flying ) )
//																	  && !unit_manager.unit_type[target_unit->type_id]->checkCategory( pType->w_badTargetCategory[i] ) ) )
//										&& ( ((mission->getFlags() & MISSION_FLAG_COMMAND_FIRE) && (pType->weapon[ i ]->commandfire || !pType->candgun) )
//											 || (!(mission->getFlags() & MISSION_FLAG_COMMAND_FIRE) && !pType->weapon[ i ]->commandfire)
//											 || pType->weapon[ i ]->dropped ) )
									if (( (weapon[i].state & 3) == WEAPON_FLAG_IDLE || ( (weapon[i].state & 3) != WEAPON_FLAG_IDLE && weapon[i].target != target_unit ) )
                                        && ( target_unit == NULL || ( (!pType->weapon[ i ]->toairweapon
                                                                       || ( pType->weapon[ i ]->toairweapon && target_unit->flying ) )
                                                                      && !unit_manager.unit_type[target_unit->type_id]->checkCategory( pType->NoChaseCategory ) ) )
										&& ( ((mission->getFlags() & MISSION_FLAG_COMMAND_FIRE) && (pType->weapon[ i ]->commandfire || !pType->candgun) )
											 || (!(mission->getFlags() & MISSION_FLAG_COMMAND_FIRE) && !pType->weapon[ i ]->commandfire)
                                             || pType->weapon[ i ]->dropped ) )
									{
										weapon[i].state = WEAPON_FLAG_AIM;
										weapon[i].target = target_unit;
										weapon[i].target_pos = mission->getTarget().getPos();
										weapon[i].data = -1;
										if (mission->getFlags() & MISSION_FLAG_TARGET_WEAPON)
											weapon[i].state |= WEAPON_FLAG_WEAPON;
                                        if (pType->weapon[ i ]->commandfire)
											weapon[i].state |= WEAPON_FLAG_COMMAND_FIRE;
									}
								}
							}

                            if (pType->kamikaze && pType->kamikazedistance > maxdist)
                                maxdist = pType->kamikazedistance;

							if (mindist > maxdist)	mindist = maxdist;

							mission->Flags() |= MISSION_FLAG_CAN_ATTACK;

                            if (pType->kamikaze				// Kamikaze attack !!
                                && dist <= pType->kamikazedistance * pType->kamikazedistance
								&& self_destruct < 0.0f)
								self_destruct = 0.01f;

							if (dist > maxdist * maxdist || dist < mindist * mindist)	// Si l'unité est trop loin de sa cible / if unit isn't where it should be
							{
                                if (!pType->canmove)		// Bah là si on peut pas bouger faut changer de cible!! / need to change target
								{
									next_mission();
									break;
								}
                                else if (!pType->canfly || pType->hoverattack)
								{
									c_time = 0.0f;
									mission->Flags() |= MISSION_FLAG_MOVE;
									mission->setMoveData( maxdist * 7 / 80 );
								}
							}
							else if (mission->getData() == 0)
							{
								mission->setData(2);
								int param[] = { 0 };
								for (uint32 i = 0 ; i < weapon.size() ; ++i)
                                    if (pType->weapon[ i ])
                                        param[ 0 ] = Math::Max(param[0], (int)( pType->weapon[i]->reloadtime * 1000.0f) * Math::Max(1, (int)pType->weapon[i]->burst));
								launchScript(SCRIPT_SetMaxReloadTime, 1, param);
							}

							if (mission->getFlags() & MISSION_FLAG_COMMAND_FIRED)
								next_mission();
						}
						else
							next_mission();
					}
					break;
				case MISSION_GUARD_NOMOVE:
					setFlag(mission->Flags(), MISSION_FLAG_CAN_ATTACK);
					unsetFlag(mission->Flags(), MISSION_FLAG_MOVE);
					if (mission.hasNext())
						next_mission();
					break;
				case MISSION_STOP:											// Arrête tout ce qui était en cours / stop everything running
					while (mission.hasNext()
						   && (mission->mission() == MISSION_STOP
							   || mission->mission() == MISSION_STANDBY
							   || mission->mission() == MISSION_VTOL_STANDBY)
						   && (mission(1) == MISSION_STOP
							   || mission(1) == MISSION_STANDBY
							   || mission(1) == MISSION_VTOL_STANDBY))     // Don't make a big stop stack :P
						next_mission();
					if (mission->mission() != MISSION_STOP
						&& mission->mission() != MISSION_STANDBY
						&& mission->mission() != MISSION_VTOL_STANDBY)
						break;
					mission->setMissionType(MISSION_STOP);
					if (jump_commands && mission->getData() != 0)	break;
					if (mission->getData() > 5)
					{
						if (mission.hasNext())
						{
							next_mission();
							if (!mission.empty() && mission->mission() == MISSION_STOP)		// Mode attente / wait mode
								mission->setData(1);
						}
					}
					else
					{
						if (mission->getData() == 0)
						{
							stopMovingAnimation();
							was_moving = false;
							selfmove = false;
							if (port[INBUILDSTANCE])
							{
								launchScript(SCRIPT_stopbuilding);
								deactivate();
							}
							for (uint32 i = 0 ; i < weapon.size() ; ++i)
								if (weapon[i].state)
								{
									launchScript(SCRIPT_TargetCleared);
									break;
								}
							for (uint32 i = 0 ; i < weapon.size() ; ++i)			// Stop weapons
							{
								weapon[i].state = WEAPON_FLAG_IDLE;
								weapon[i].data = -1;
							}
						}
						mission->setData(mission->getData() + 1);
					}
					break;
				case MISSION_REPAIR:
					if (!mission->getTarget().isValid())
					{
						next_mission();
						break;
					}
					{
						Unit *target_unit = mission->getUnit();
						if (target_unit != NULL
							&& (target_unit->flags & 1)
							&& Yuni::Math::Zero(target_unit->build_percent_left))
						{
							if (target_unit->hp >= unit_manager.unit_type[target_unit->type_id]->MaxDamage
								|| !pType->BMcode)
							{
                                if (pType->BMcode)
									target_unit->hp = (float)unit_manager.unit_type[target_unit->type_id]->MaxDamage;
								next_mission();
							}
							else
							{
								Vector3D Dir = target_unit->Pos - Pos;
								Dir.y = 0.0f;
								const float dist = Dir.sq();
								const int maxdist = (int)pType->BuildDistance
													+ ((unit_manager.unit_type[target_unit->type_id]->FootprintX + unit_manager.unit_type[target_unit->type_id]->FootprintZ) << 1);
								if (dist > maxdist * maxdist && pType->BMcode)	// Si l'unité est trop loin du chantier
								{
									mission->Flags() |= MISSION_FLAG_MOVE;
									mission->setMoveData( maxdist * 7 / 80 );
									mission->setData(0);
									c_time = 0.0f;
								}
								else
								{
									if (mission->getFlags() & MISSION_FLAG_MOVE) // Stop moving if needed
									{
										stopMoving();
										break;
									}
									if (mission->getData() == 0)
									{
										mission->setData(1);
										start_building(target_unit->Pos - Pos);
									}

									if (port[ INBUILDSTANCE ] != 0)
									{
										if (local && network_manager.isConnected() && nanolathe_target < 0 )		// Synchronize nanolathe emission
										{
											nanolathe_target = target_unit->idx;
											g_ta3d_network->sendUnitNanolatheEvent( idx, target_unit->idx, false, false );
										}

										const float conso_energy = ((float)(pType->WorkerTime * unit_manager.unit_type[target_unit->type_id]->BuildCostEnergy)) / (float)unit_manager.unit_type[target_unit->type_id]->BuildTime;
										TA3D::players.requested_energy[owner_id] += conso_energy;
										if (players.energy[owner_id] >= (energy_cons + conso_energy * TA3D::players.energy_factor[owner_id]) * dt)
										{
											energy_cons += conso_energy * TA3D::players.energy_factor[owner_id];
											const UnitType *pTargetType = unit_manager.unit_type[target_unit->type_id];
											const float maxdmg = float(pTargetType->MaxDamage);
											target_unit->hp = std::min(maxdmg,
																	   target_unit->hp + dt * TA3D::players.energy_factor[owner_id] * (float)pType->WorkerTime * maxdmg / (float)pTargetType->BuildTime);
										}
										target_unit->built = true;
									}
								}
							}
						}
						else if (target_unit != NULL && target_unit->flags)
						{
							Vector3D Dir = target_unit->Pos - Pos;
							Dir.y = 0.0f;
							const float dist = Dir.sq();
							const int maxdist = (int)pType->BuildDistance
												+ ((unit_manager.unit_type[target_unit->type_id]->FootprintX + unit_manager.unit_type[target_unit->type_id]->FootprintZ) << 1);
							if (dist > maxdist * maxdist && pType->BMcode)	// Si l'unité est trop loin du chantier
							{
								c_time = 0.0f;
								mission->Flags() |= MISSION_FLAG_MOVE;
								mission->setMoveData( maxdist * 7 / 80 );
							}
							else
							{
								if (mission->getFlags() & MISSION_FLAG_MOVE) // Stop moving if needed
									stopMoving();
								if (pType->BMcode)
								{
									start_building(target_unit->Pos - Pos);
									mission->setMissionType(MISSION_BUILD_2);		// Change de type de mission
								}
							}
						}
					}
					break;
				case MISSION_BUILD_2:
					if (!mission->getTarget().isValid())
					{
						next_mission();
						break;
					}
					{
						Unit *target_unit = mission->getUnit();
						if (target_unit && target_unit->flags)
						{
							target_unit->lock();
							if (target_unit->build_percent_left <= 0.0f)
							{
								target_unit->build_percent_left = 0.0f;
								if (unit_manager.unit_type[target_unit->type_id]->ActivateWhenBuilt)		// Start activated
								{
									target_unit->port[ ACTIVATION ] = 0;
									target_unit->activate();
								}
								if (unit_manager.unit_type[target_unit->type_id]->init_cloaked )				// Start cloaked
									target_unit->cloaking = true;
                                if (!pType->BMcode) // Ordre de se déplacer
								{
									Vector3D target = Pos;
									target.z += 128.0f;
									if (!def_mission)
										target_unit->set_mission(MISSION_MOVE | MISSION_FLAG_AUTO, &target, false, 5, true, NULL, 0, 5);		// Fait sortir l'unité du bâtiment
									else
										target_unit->mission = def_mission;
								}
								mission->getTarget().set(Mission::Target::TargetNone, -1, 0);
								next_mission();
							}
							else if (port[ INBUILDSTANCE ] != 0)
							{
								if (local && network_manager.isConnected() && nanolathe_target < 0) // Synchronize nanolathe emission
								{
									nanolathe_target = target_unit->idx;
									g_ta3d_network->sendUnitNanolatheEvent( idx, target_unit->idx, false, false );
								}

								unsetFlag(mission->Flags(), MISSION_FLAG_CAN_ATTACK);			// Don't attack when building

								const float conso_metal = ((float)(pType->WorkerTime * unit_manager.unit_type[target_unit->type_id]->BuildCostMetal)) / (float)unit_manager.unit_type[target_unit->type_id]->BuildTime;
								const float conso_energy = ((float)(pType->WorkerTime * unit_manager.unit_type[target_unit->type_id]->BuildCostEnergy)) / (float)unit_manager.unit_type[target_unit->type_id]->BuildTime;

								TA3D::players.requested_energy[owner_id] += conso_energy;
								TA3D::players.requested_metal[owner_id] += conso_metal;

								if (players.metal[owner_id]>= (metal_cons + conso_metal * resource_min_factor) * dt
									&& players.energy[owner_id]>= (energy_cons + conso_energy * resource_min_factor) * dt)
								{
									metal_cons += conso_metal * resource_min_factor;
									energy_cons += conso_energy * resource_min_factor;
									const UnitType *pTargetType = unit_manager.unit_type[target_unit->type_id];
									const float base = dt * resource_min_factor * (float)pType->WorkerTime;
									const float maxdmg = float(pTargetType->MaxDamage);
									target_unit->build_percent_left = std::max(0.0f, target_unit->build_percent_left - base * 100.0f / (float)pTargetType->BuildTime);
									target_unit->hp = std::min(maxdmg, target_unit->hp + base * maxdmg / (float)pTargetType->BuildTime);
								}
                                if (!pType->BMcode)
								{
									const int buildinfo = runScriptFunction(SCRIPT_QueryBuildInfo);
									if (buildinfo >= 0)
									{
										compute_model_coord();
										Vector3D old_pos = target_unit->Pos;
										target_unit->Pos = Pos + data.data[buildinfo].pos;
										if (unit_manager.unit_type[target_unit->type_id]->Floater || ( unit_manager.unit_type[target_unit->type_id]->canhover && old_pos.y <= the_map->sealvl ) )
											target_unit->Pos.y = old_pos.y;
										if (((Vector3D)(old_pos-target_unit->Pos)).sq() > 1000000.0f) // It must be continuous
										{
											target_unit->Pos.x = old_pos.x;
											target_unit->Pos.z = old_pos.z;
										}
										else
										{
											target_unit->cur_px = ((int)(target_unit->Pos.x) + the_map->map_w_d + 4) >> 3;
											target_unit->cur_py = ((int)(target_unit->Pos.z) + the_map->map_h_d + 4) >> 3;
										}
										target_unit->Angle = Angle;
										target_unit->Angle.y += data.data[buildinfo].axe[1].angle;
										pMutex.unlock();
										target_unit->draw_on_map();
										pMutex.lock();
									}
								}
								target_unit->built = true;
							}
							else
							{
								activate();
								target_unit->built = true;
							}
							target_unit->unlock();
						}
						else
							next_mission();
					}
					break;
				case MISSION_BUILD:
					if (mission->getUnit())
					{
						mission->setMissionType(MISSION_BUILD_2);		// Change mission type
						mission->getUnit()->built = true;
					}
					else
					{
						Vector3D Dir = mission->getTarget().getPos() - Pos;
						Dir.y = 0.0f;
						const float dist = Dir.sq();
						const int maxdist = (int)pType->BuildDistance
											+ ((unit_manager.unit_type[mission->getData()]->FootprintX + unit_manager.unit_type[mission->getData()]->FootprintZ) << 1);
						if (dist > maxdist * maxdist && pType->BMcode)	// Si l'unité est trop loin du chantier
						{
							setFlag(mission->Flags(), MISSION_FLAG_MOVE);
							mission->setMoveData( maxdist * 7 / 80 );
						}
						else
						{
							if (mission->getFlags() & MISSION_FLAG_MOVE) // Stop moving if needed
							{
								stopMoving();
								break;
							}
							if (!pType->BMcode)
							{
								const int buildinfo = runScriptFunction(SCRIPT_QueryBuildInfo);
								if (buildinfo >= 0)
								{
									compute_model_coord();
									mission->getTarget().setPos(Pos + data.data[buildinfo].pos);
								}
							}
							if (port[ INBUILDSTANCE ])// && (pType->BMcode || (!pType->BMcode && port[YARD_OPEN] && !port[BUGGER_OFF])))
							{
								V.x = 0.0f;
								V.y = 0.0f;
								V.z = 0.0f;
								const Vector3D target = mission->getTarget().getPos();
								if (the_map->check_rect((((int)(target.x) + the_map->map_w_d+4)>>3)-(unit_manager.unit_type[mission->getData()]->FootprintX>>1),
													(((int)(target.z) + the_map->map_h_d+4)>>3)-(unit_manager.unit_type[mission->getData()]->FootprintZ>>1),
													unit_manager.unit_type[mission->getData()]->FootprintX,
													unit_manager.unit_type[mission->getData()]->FootprintZ,
													-1)) // Check if we have an empty place to build our unit
								{
									pMutex.unlock();
									Unit *p = (Unit*)create_unit(mission->getData(), owner_id, mission->getTarget().getPos());
									if (p)
										mission->getTarget().set(Mission::Target::TargetUnit, p->idx, p->ID);
									pMutex.lock();
									if (p)
									{
										p->hp = 0.000001f;
										p->built = true;
									}
//                                    else
//                                        LOG_WARNING(idx << " can't create unit! (`" << __FILE__ << "`:" << __LINE__ << ")");
								}
                                else if (pType->BMcode)
									next_mission();
							}
							else
								start_building( mission->getTarget().getPos() - Pos );
						}
					}
					break;
			};

            switch(pType->TEDclass)			// Commandes particulières
			{
				case CLASS_PLANT:
					switch(mission->mission())
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
						if (!(mission->getFlags() & MISSION_FLAG_MOVE)
							&& !(mission->getFlags() & MISSION_FLAG_DONT_STOP_MOVE)
							&& ((mission->mission() != MISSION_ATTACK && pType->canfly) || !pType->canfly)
							&& !selfmove)
						{
							if (!flying)
								V.x = V.z = 0.0f;
							if (precomputed_position)
							{
								NPos = Pos;
								n_px = cur_px;
								n_py = cur_py;
							}
						}
						switch(mission->mission())
						{
							case MISSION_ATTACK:
							case MISSION_PATROL:
							case MISSION_REPAIR:
							case MISSION_BUILD:
							case MISSION_BUILD_2:
							case MISSION_GET_REPAIRED:
                                if (pType->canfly)
									activate();
								break;
							case MISSION_STANDBY:
								if (mission.hasNext())
									next_mission();
								if (!selfmove)
									V.reset();			// Frottements
								break;
							case MISSION_MOVE:
								mission->Flags() |= MISSION_FLAG_CAN_ATTACK;
								if (!(mission->getFlags() & MISSION_FLAG_MOVE) )			// Monitor the moving process
								{
									if (mission.hasNext()
										&& (mission(1) == MISSION_MOVE
											|| (mission(1) == MISSION_STOP
												&& mission(2) == MISSION_MOVE) ) )
										mission->Flags() |= MISSION_FLAG_DONT_STOP_MOVE;

									if (!(mission->getFlags() & MISSION_FLAG_DONT_STOP_MOVE))			// If needed
										V.reset();			// Stop the unit
									if (precomputed_position)
									{
										NPos = Pos;
										n_px = cur_px;
										n_py = cur_py;
									}
									if ((mission->getFlags() & MISSION_FLAG_DONT_STOP_MOVE)
										&& mission.hasNext() && mission(1) == MISSION_STOP)			// If needed
										next_mission();
									next_mission();
								}
								break;
							default:
                                if (pType->canfly)
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
                    LOG_WARNING("Unknown type :" << pType->TEDclass);
			};

			switch(mission->mission())		// Quelques animations spéciales / Some special animation code
			{
            case MISSION_ATTACK:
                if (pType->canfly && !pType->hoverattack)			// Un avion?? / A plane ?
                {
                    activate();
					unsetFlag(mission->Flags(), MISSION_FLAG_MOVE);			// We're doing it here, so no need to do it twice
					Vector3D J, I, K(0.0f, 1.0f, 0.0f);
					Vector3D Target = mission->getTarget().getPos();
					J = Target - Pos;
                    J.y = 0.0f;
					const float dist = J.norm();
					mission->setLastD(dist);
                    if (dist > 0.0f)
                        J = 1.0f / dist * J;
					if (dist > (float)pType->ManeuverLeashLength * 0.5f)
                    {
                        b_TargetAngle = true;
                        f_TargetAngle = acosf(J.z) * RAD2DEG;
						if (J.x < 0.0f)
							f_TargetAngle = -f_TargetAngle;
                    }

                    J.z = cosf(Angle.y * DEG2RAD);
                    J.x = sinf(Angle.y * DEG2RAD);
                    J.y = 0.0f;
                    I.z = -J.x;
                    I.x = J.z;
                    I.y = 0.0f;
					V = (V % K) * K + (V % J) * J + units.exp_dt_4 * (V % I) * I;
					const float speed = V.sq();
                    if (speed < pType->MaxVelocity * pType->MaxVelocity)
						V = V + pType->Acceleration * dt * J;
                }
                if (!pType->hoverattack)
                    break;
            case MISSION_CAPTURE:
            case MISSION_RECLAIM:
            case MISSION_REPAIR:
            case MISSION_BUILD_2:
            case MISSION_REVIVE:
                if (flying)             // Brawler and construction aircrafts animation
                {
                    activate();
					unsetFlag(mission->Flags(), MISSION_FLAG_MOVE);			// We're doing it here, so no need to do it twice
                    Vector3D K(0.0f, 1.0f, 0.0f);
					Vector3D J = mission->getTarget().getPos() - Pos;
                    J.y = 0.0f;
					const float dist = J.norm();
                    if (dist > 0.0f)
                        J = 1.0f / dist * J;
                    b_TargetAngle = true;
                    f_TargetAngle = acosf(J.z) * RAD2DEG;
                    if (J.x < 0.0f) f_TargetAngle = -f_TargetAngle;

					float ideal_dist = (float)pType->SightDistance * 0.25f;
					switch (mission->mission())
                    {
                    case MISSION_BUILD_2:
                    case MISSION_CAPTURE:
                    case MISSION_REPAIR:
                    case MISSION_RECLAIM:
                    case MISSION_REVIVE:
                        ideal_dist = pType->BuildDistance * 0.5f;
                    };

                    V += (Math::Clamp(10.0f * (dist - ideal_dist), -pType->Acceleration, pType->Acceleration) * dt) * J;

                    if (dist < 2.0f * ideal_dist)
                    {
                        J.z = sinf(Angle.y * DEG2RAD);
                        J.x = -cosf(Angle.y * DEG2RAD);
                        J.y = 0.0f;
						V = units.exp_dt_4 * V + (dt * dist * Math::Max(8.0f * (cosf(PI * ((float)units.current_tick) / (float)TICKS_PER_SEC) - 0.5f), 0.0f)) * J;
                    }
                    else
                    {
                        J.z = sinf(Angle.y * DEG2RAD);
                        J.x = -cosf(Angle.y * DEG2RAD);
                        J.y = 0.0f;
                        J = (J % V) * J;
                        V = (V - J) + units.exp_dt_4 * J;
                    }
					const float speed = V.sq();
                    if (speed > pType->MaxVelocity * pType->MaxVelocity)
                        V = pType->MaxVelocity / V.norm() * V;
                }
                break;
			case MISSION_GUARD:
				if (pType->canfly)             // Aircrafts fly around guarded units
				{
					activate();
					unsetFlag(mission->Flags(), MISSION_FLAG_MOVE);			// We're doing it here, so no need to do it twice
					Vector3D J = mission->getTarget().getPos() - Pos;
					J.y = 0.0f;
					const float dist = J.norm();
					if (dist > 0.0f)
						J = 1.0f / dist * J;
					b_TargetAngle = true;
					f_TargetAngle = acosf(J.z) * RAD2DEG;
					if (J.x < 0.0f) f_TargetAngle = -f_TargetAngle;
					const float ideal_dist = (float)pType->SightDistance;

					Vector3D acc;
					if (dist > 2.0f * ideal_dist)
						acc = pType->Acceleration * J;
					else
					{
						f_TargetAngle += 90.0f;
						acc = pType->Acceleration * (10.0f * (dist - ideal_dist) * J + Vector3D(J.z, 0.0f, -J.x));
						if (acc.sq() >= pType->Acceleration * pType->Acceleration)
						{
							acc.unit();
							acc *= pType->Acceleration;
						}
					}
					V += dt * acc;

					J.z = sinf(Angle.y * DEG2RAD);
					J.x = -cosf(Angle.y * DEG2RAD);
					J.y = 0.0f;
					J = (J % V) * J;
					V = (V - J) + units.exp_dt_4 * J;

					const float speed = V.sq();
					if (speed > pType->MaxVelocity * pType->MaxVelocity)
						V = pType->MaxVelocity / sqrtf(speed) * V;
				}
				break;
			}

			if (( (mission->getFlags() & MISSION_FLAG_MOVE) || !local ) && !jump_commands )// Set unit orientation if it's on the ground
			{
                if (!pType->canfly && !pType->Upright
                    && !pType->floatting()
					&& !( pType->canhover && Pos.y <= the_map->sealvl ))
				{
					Vector3D I,J,K,A,B,C;
					Matrix M = RotateY((Angle.y + 90.0f) * DEG2RAD);
					I.x = 4.0f;
					J.z = 4.0f;
					K.y = 1.0f;
					A = Pos - pType->FootprintZ * I * M;
					B = Pos + (pType->FootprintX * I - pType->FootprintZ * J) * M;
					C = Pos + (pType->FootprintX * I + pType->FootprintZ * J) * M;
					A.y = the_map->get_unit_h(A.x,A.z);	// Projete le triangle
					B.y = the_map->get_unit_h(B.x,B.z);
					C.y = the_map->get_unit_h(C.x,C.z);
					Vector3D D = (B - A) * (B - C);
					if (D.y >= 0.0f) // On ne met pas une unité à l'envers!!
					{
						D.unit();
						const float dist_sq = sqrtf( D.y * D.y + D.z * D.z );
						float angle_1 = !Yuni::Math::Zero(dist_sq) ? acosf( D.y / dist_sq ) * RAD2DEG : 0.0f;
						if (D.z < 0.0f)	angle_1 = -angle_1;
						D = D * RotateX(-angle_1 * DEG2RAD);
						float angle_2 = VAngle(D, K) * RAD2DEG;
						if (D.x > 0.0f)	angle_2 = -angle_2;
						if (fabsf(angle_1 - Angle.x) <= 180.0f && fabsf(angle_2 - Angle.z) <= 180.0f)
						{
							Angle.x = angle_1;
							Angle.z = angle_2;
						}
					}
				}
                else if (!pType->canfly)
					Angle.x = Angle.z = 0.0f;
			}

			bool returning_fire = ( port[ STANDINGFIREORDERS ] == SFORDER_RETURN_FIRE && attacked );
			if (( ((mission->getFlags() & MISSION_FLAG_CAN_ATTACK) == MISSION_FLAG_CAN_ATTACK) || do_nothing() )
				&& ( port[ STANDINGFIREORDERS ] == SFORDER_FIRE_AT_WILL || returning_fire )
				&& !jump_commands && local)
			{
				// Si l'unité peut attaquer d'elle même les unités enemies proches, elle le fait / Attack nearby enemies

                bool can_fire = pType->AutoFire && pType->canattack;
				bool canTargetGround = false;

				if (!can_fire)
				{
					for (uint32 i = 0 ; i < weapon.size() && !can_fire ; ++i)
                        can_fire =  pType->weapon[i] != NULL && !pType->weapon[i]->commandfire
                            && !pType->weapon[i]->interceptor && weapon[i].state == WEAPON_FLAG_IDLE;
				}
				else
				{
					can_fire = false;
					for (uint32 i = 0 ; i < weapon.size() && !can_fire ; ++i)
                        can_fire =  pType->weapon[i] != NULL && weapon[i].state == WEAPON_FLAG_IDLE;
				}
				for (uint32 i = 0 ; i < weapon.size() && !canTargetGround ; ++i)
					if (pType->weapon[i] && weapon[i].state == WEAPON_FLAG_IDLE)
						canTargetGround |= !pType->weapon[i]->toairweapon;

				if (can_fire)
				{
					int dx = pType->SightDistance + (int)(h+0.5f);
					int enemy_idx = -1;
					for (uint32 i = 0 ; i < weapon.size() ; ++i)
						if (pType->weapon[i] != NULL
							&& (pType->weapon[i]->range >> 1) > dx
							&& !pType->weapon[i]->interceptor
							&& !pType->weapon[i]->commandfire)
							dx = pType->weapon[i]->range >> 1;
					if (pType->kamikaze && pType->kamikazedistance > dx)
						dx = pType->kamikazedistance;
					const byte mask = byte(1 << owner_id);

					std::deque<UnitTKit::T> possibleTargets;
					for(int i = 0 ; i < NB_PLAYERS ; ++i)
						if (i != owner_id && !(players.team[owner_id] & players.team[i]))
							units.kdTree[i]->maxDistanceQuery(possibleTargets, Pos, float(dx));

					for(std::deque<UnitTKit::T>::iterator i = possibleTargets.begin() ; enemy_idx == -1 && i != possibleTargets.end() ; ++i)
					{
						const int cur_idx = i->first->idx;
						const int x = i->first->cur_px;
						const int y = i->first->cur_py;
						const int cur_type_id = units.unit[cur_idx].type_id;
						if (x < 0
							|| x >= the_map->bloc_w_db - 1
							|| y < 0
							|| y >= the_map->bloc_h_db - 1
							|| cur_type_id == -1)
							continue;
						if (units.unit[cur_idx].flags
							&& ( units.unit[cur_idx].is_on_radar( mask ) ||
								 ( (the_map->sight_map(x >> 1, y >> 1) & mask)
								   && !units.unit[cur_idx].cloaked ) )
							&& (canTargetGround || units.unit[cur_idx].flying)
							&& !unit_manager.unit_type[ cur_type_id ]->checkCategory( pType->NoChaseCategory ) )
							//                                             && !unit_manager.unit_type[ units.unit[cur_idx].type_id ]->checkCategory( pType->BadTargetCategory ) )
						{
							if (returning_fire)
							{
								for(uint32 i = 0 ; i < units.unit[cur_idx].weapon.size() ; ++i)
								{
									if (units.unit[cur_idx].weapon[i].state != WEAPON_FLAG_IDLE
										&& units.unit[cur_idx].weapon[i].target == this)
									{
										enemy_idx = cur_idx;
										break;
									}
								}
							}
							else
								enemy_idx = cur_idx;
						}
					}
					if (enemy_idx >= 0)			// Si on a trouvé une unité, on l'attaque
					{
						if (do_nothing())
                            set_mission(MISSION_ATTACK | MISSION_FLAG_AUTO,&(units.unit[enemy_idx].Pos),false,0,true,&(units.unit[enemy_idx]));
						else if (!mission.empty() && mission->mission() == MISSION_PATROL)
						{
							add_mission(MISSION_MOVE | MISSION_FLAG_AUTO, &(Pos), true, 0);
							add_mission(MISSION_ATTACK | MISSION_FLAG_AUTO, &(units.unit[enemy_idx].Pos), true, 0, &(units.unit[enemy_idx]));
						}
						else
							for (uint32 i = 0 ; i < weapon.size() ; ++i)
                                if (weapon[i].state == WEAPON_FLAG_IDLE && pType->weapon[ i ] != NULL
                                    && !pType->weapon[ i ]->commandfire
                                    && !pType->weapon[ i ]->interceptor
                                    && (!pType->weapon[ i ]->toairweapon
                                        || ( pType->weapon[ i ]->toairweapon && units.unit[enemy_idx].flying ) )
                                    && !unit_manager.unit_type[ units.unit[enemy_idx].type_id ]->checkCategory( pType->NoChaseCategory ) )
                                    //                                        && !unit_manager.unit_type[ units.unit[enemy_idx].type_id ]->checkCategory( pType->w_badTargetCategory[i] ) ) )
								{
									weapon[i].state = WEAPON_FLAG_AIM;
									weapon[i].target = &(units.unit[enemy_idx]);
									weapon[i].data = -1;
								}
					}
				}
                if (weapon.size() > 0 && pType->antiweapons && pType->weapon[0])
				{
					const float coverage = pType->weapon[0]->coverage * pType->weapon[0]->coverage;
					const float range = float(pType->weapon[0]->range * pType->weapon[0]->range >> 2);
					int enemy_idx = -1;
					int e = 0;
					for(int i = 0 ; i + e < mem_size ; ++i)
					{
						if (memory[i + e] >= weapons.nb_weapon || weapons.weapon[memory[i + e]].weapon_id == -1)
						{
							++e;
							--i;
							continue;
						}
						memory[i] = memory[i + e];
					}
					mem_size -= e;
					unlock();
					weapons.lock();
					for(std::vector<uint32>::iterator f = weapons.idx_list.begin() ; f != weapons.idx_list.end() ; ++f)
					{
						const uint32 i = *f;
						// Yes we don't defend against allies :D, can lead to funny situations :P
						if (weapons.weapon[i].weapon_id != -1 && !(players.team[ units.unit[weapons.weapon[i].shooter_idx].owner_id ] & players.team[ owner_id ])
							&& weapon_manager.weapon[weapons.weapon[i].weapon_id].targetable)
						{
							if (((Vector3D)(weapons.weapon[i].target_pos-Pos)).sq() <= coverage
								&& ((Vector3D)(weapons.weapon[i].Pos-Pos)).sq() <= range)
							{
								int idx = -1;
								for (e = 0 ; e < mem_size ; ++e)
								{
									if (memory[e] == i)
									{
										idx = i;
										break;
									}
								}
								if (idx == -1)
								{
									enemy_idx = i;
									if (mem_size < TA3D_PLAYERS_HARD_LIMIT)
									{
										memory[mem_size] = i;
										mem_size++;
									}
									break;
								}
							}
						}
					}
					weapons.unlock();
					lock();
					if (enemy_idx >= 0)			// If we found a target, then attack it, here  we use attack because we need the mission list to act properly
						add_mission(MISSION_ATTACK | MISSION_FLAG_AUTO,
									&(weapons.weapon[enemy_idx].Pos),
									false, 0, &(weapons.weapon[enemy_idx]), 12);	// 12 = 4 | 8, targets a weapon and automatic fire
				}
			}
		}

        if (pType->canfly) // Set plane orientation
		{
			Vector3D J,K(0.0f, 1.0f, 0.0f);
			J = V * K;

			Vector3D virtual_G;						// Compute the apparent gravity force ( seen from the plane )
			virtual_G.x = virtual_G.z = 0.0f;		// Standard gravity vector
			virtual_G.y = -4.0f * units.g_dt;
			float d = J.sq();
			if (!Yuni::Math::Zero(d))
				virtual_G = virtual_G + (((old_V - V) % J) / d) * J;		// Add the opposite of the speed derivative projected on the side of the unit

			d = virtual_G.norm();
			if (!Yuni::Math::Zero(d))
			{
				virtual_G = -1.0f / d * virtual_G;

				d = sqrtf(virtual_G.y*virtual_G.y+virtual_G.z*virtual_G.z);
				float angle_1 = !Yuni::Math::Zero(d) ? acosf(virtual_G.y / d) * RAD2DEG : 0.0f;
				if (virtual_G.z < 0.0f)	angle_1 = -angle_1;
				virtual_G = virtual_G * RotateX(-angle_1*DEG2RAD);
				float angle_2 = acosf( virtual_G % K )*RAD2DEG;
				if (virtual_G.x > 0.0f)	angle_2 = -angle_2;

				if (fabsf( angle_1 - Angle.x ) < 360.0f)
					Angle.x += 5.0f * dt * (angle_1 - Angle.x);				// We need something continuous
				if (fabsf( angle_2 - Angle.z ) < 360.0f)
					Angle.z += 5.0f * dt * (angle_2 - Angle.z);

				if (Angle.x < -360.0f || Angle.x > 360.0f)		Angle.x = 0.0f;
				if (Angle.z < -360.0f || Angle.z > 360.0f)		Angle.z = 0.0f;
			}
		}

		if (Yuni::Math::Zero(build_percent_left))
		{

			// Change the unit's angle the way we need it to be changed

            if (b_TargetAngle && !isNaN(f_TargetAngle) && pType->BMcode)	// Don't remove the class check otherwise factories can spin
			{
				while (!isNaN(f_TargetAngle) && fabsf( f_TargetAngle - Angle.y ) > 180.0f)
				{
					if (f_TargetAngle < Angle.y)
						Angle.y -= 360.0f;
					else
						Angle.y += 360.0f;
				}
				if (!isNaN(f_TargetAngle) && fabsf( f_TargetAngle - Angle.y ) >= 1.0f)
				{
                    float aspeed = pType->TurnRate;
					if (f_TargetAngle < Angle.y )
						aspeed =- aspeed;
					float a = f_TargetAngle - Angle.y;
					V_Angle.y = aspeed;
					float b = f_TargetAngle - (Angle.y + dt*V_Angle.y);
					if (((a < 0.0f && b > 0.0f) || (a > 0.0f && b < 0.0f)) && !isNaN(f_TargetAngle))
					{
						V_Angle.y = 0.0f;
						Angle.y = f_TargetAngle;
					}
				}
			}

			Angle = Angle + dt * V_Angle;
			Vector3D OPos = Pos;
			if (precomputed_position)
			{
                if (pType->canmove && pType->BMcode && !flying )
					V.y-=units.g_dt;			// L'unité subit la force de gravitation
				Pos = NPos;
				Pos.y = OPos.y + V.y * dt;
				cur_px = n_px;
				cur_py = n_py;
			}
			else
			{
                if (pType->canmove && pType->BMcode )
					V.y -= units.g_dt;			// L'unité subit la force de gravitation
				Pos += dt * V;			// Déplace l'unité
				cur_px = ((int)(Pos.x)+the_map->map_w_d+4)>>3;
				cur_py = ((int)(Pos.z)+the_map->map_h_d+4)>>3;
			}
			if (units.current_tick - ripple_timer >= (lp_CONFIG->water_quality >= 5 ? 1 : 7) && Pos.y <= the_map->sealvl && Pos.y + model->top >= the_map->sealvl && (pType->fastCategory & CATEGORY_NOTSUB)
				&& cur_px >= 0 && cur_py >= 0 && cur_px < the_map->bloc_w_db && cur_py < the_map->bloc_h_db && !the_map->map_data(cur_px, cur_py).isLava() && the_map->water )
			{
				Vector3D Diff = OPos - Pos;
				Diff.y = 0.0f;
				if (Diff.sq() > 0.1f && lp_CONFIG->waves)
				{
					ripple_timer = units.current_tick;
					Vector3D ripple_pos = Pos;
					ripple_pos.y = the_map->sealvl + 1.0f;
					fx_manager.addRipple( ripple_pos, float(((sint32)(Math::RandomTable() % 201)) - 100) * 0.0001f );
				}
			}
		}
script_exec:
		if (!attached && ( (!jump_commands && pType->canmove) || first_move ))
		{
			bool hover_on_water = false;
			float min_h = the_map->get_unit_h(Pos.x,Pos.z);
			h = Pos.y - min_h;
			if (!pType->Floater && !pType->canfly && !pType->canhover && h > 0.0f && Yuni::Math::Zero(pType->WaterLine))
				Pos.y = min_h;
			else if (pType->canhover && Pos.y <= the_map->sealvl)
			{
				hover_on_water = true;
				Pos.y = the_map->sealvl;
				if (V.y < 0.0f)
					V.y = 0.0f;
			}
            else if (pType->Floater)
			{
				Pos.y = the_map->sealvl + (float)pType->AltFromSeaLevel * H_DIV;
				V.y = 0.0f;
			}
			else if (!Yuni::Math::Zero(pType->WaterLine))
			{
				Pos.y = the_map->sealvl - pType->WaterLine * H_DIV;
				V.y = 0.0f;
			}
			else if (!pType->canfly && Pos.y > Math::Max( min_h, the_map->sealvl ) && pType->BMcode)	// Prevent non flying units from "jumping"
			{
				Pos.y = Math::Max(min_h, the_map->sealvl);
				if (V.y < 0.0f)
					V.y = 0.0f;
			}
            if (pType->canhover)
			{
				int param[1] = { hover_on_water ? ( the_map->sealvl - min_h >= 8.0f ? 2 : 1) : 4 };
				runScriptFunction(SCRIPT_setSFXoccupy, 1, param);
			}
			if (min_h > Pos.y)
			{
				Pos.y = min_h;
				if (V.y < 0.0f)
					V.y = 0.0f;
			}
			if (pType->canfly && Yuni::Math::Zero(build_percent_left) && local)
			{
				if (!mission.empty()
					&& ( (mission->getFlags() & MISSION_FLAG_MOVE)
						 || mission->mission() == MISSION_BUILD
						 || mission->mission() == MISSION_BUILD_2
						 || mission->mission() == MISSION_REPAIR
						 || mission->mission() == MISSION_ATTACK
						 || mission->mission() == MISSION_MOVE
						 || mission->mission() == MISSION_GUARD
						 || mission->mission() == MISSION_GET_REPAIRED
						 || mission->mission() == MISSION_PATROL
						 || mission->mission() == MISSION_RECLAIM
						 || nb_attached > 0
						 || Pos.x < -the_map->map_w_d
						 || Pos.x > the_map->map_w_d
						 || Pos.z < -the_map->map_h_d
						 || Pos.z > the_map->map_h_d ))
				{
					if (!(mission->mission() == MISSION_GET_REPAIRED
						  && (mission->getFlags() & MISSION_FLAG_BEING_REPAIRED) ) )
					{
						const float ideal_h = Math::Max(min_h, the_map->sealvl) + (float)pType->CruiseAlt * H_DIV;
						V.y = (ideal_h - Pos.y) * 2.0f;
					}
					flying = true;
				}
				else
				{
					if (can_be_there( cur_px, cur_py, type_id, owner_id, idx ))		// Check it can be there
					{
						float ideal_h = min_h;
						V.y = (ideal_h - Pos.y) * 1.5f;
						flying = false;
					}
					else				// There is someone there, find an other place to land
					{
						flying = true;
						if (do_nothing())   // Wait for MISSION_STOP to check if we have some work to do
						{                                                                               // This prevents planes from keeping looking for a place to land
							Vector3D next_target = Pos;                                                 // instead of going back to work :/
							const float find_angle = float(Math::RandomTable() % 360) * DEG2RAD;
							next_target.x += cosf( find_angle ) * float(32 + pType->FootprintX * 8);
							next_target.z += sinf( find_angle ) * float(32 + pType->FootprintZ * 8);
							add_mission( MISSION_MOVE | MISSION_FLAG_AUTO, &next_target, true );
						}
					}
				}
			}
			port[GROUND_HEIGHT] = (sint16)(Pos.y - min_h + 0.5f);
		}
		port[HEALTH] = (sint16)((int)hp * 100 / pType->MaxDamage);

		// Update moving animation state
		if (requestedMovingAnimationState ^ movingAnimation)
		{
			if (!requestedMovingAnimationState)
				launchScript(SCRIPT_StopMoving);
			else
			{
				if (pType->canfly)
					activate();
				launchScript(SCRIPT_startmoving);
				if (nb_attached == 0)
					launchScript(SCRIPT_MoveRate1);		// For the armatlas
				else
					launchScript(SCRIPT_MoveRate2);
			}
			movingAnimation = requestedMovingAnimationState;
		}

		if (script)
			script->run(dt);
		yardmap_timer--;
		if (hp > 0.0f &&
			(((o_px != cur_px
			   || o_py != cur_py
			   || first_move
			   || (was_flying ^ flying)
			   || (!Yuni::Math::Zero(port[YARD_OPEN]) ^ was_open)
			   || yardmap_timer == 0) && build_percent_left <= 0.0f) || !drawn || (drawn && drawn_obstacle != is_obstacle())))
		{
			first_move = build_percent_left > 0.0f;
			pMutex.unlock();
			draw_on_map();
			pMutex.lock();
			yardmap_timer = TICKS_PER_SEC + (Math::RandomTable() & 15);
		}

		built = false;
		attacked = false;
		pMutex.unlock();
		return 0;
	}



	bool Unit::hit(const Vector3D &P, const Vector3D &Dir, Vector3D* hit_vec, const float length)
	{
		pMutex.lock();
		if (!(flags&1))
		{
			pMutex.unlock();
			return false;
		}
		if (model)
		{
			const Vector3D c_dir = model->center + Pos - P;
			if (c_dir.norm() - length <= model->size2)
			{
				const UnitType *pType = unit_manager.unit_type[type_id];
				const float scale = pType->Scale;
				const Matrix M = RotateXZY(-Angle.x * DEG2RAD, -Angle.z * DEG2RAD, -Angle.y * DEG2RAD) * Scale(1.0f / scale);
				const Vector3D RP = (P-Pos) * M;
				const bool is_hit = model->hit(RP,Dir,&data,hit_vec,M) >= -1;
				if (is_hit)
				{
					*hit_vec = ((*hit_vec) * RotateYZX(Angle.y * DEG2RAD, Angle.z * DEG2RAD, Angle.x * DEG2RAD)) * Scale(scale) + Pos;
					*hit_vec = ((*hit_vec - P) % Dir) * Dir + P;
				}

				pMutex.unlock();
				return is_hit;
			}
		}
		pMutex.unlock();
		return false;
	}

	bool Unit::hit_fast(const Vector3D &P, const Vector3D &Dir, Vector3D* hit_vec, const float length)
	{
		pMutex.lock();
		if (!(flags&1))
		{
			pMutex.unlock();
			return false;
		}
		if (model)
		{
			const Vector3D c_dir = model->center + Pos - P;
			if (c_dir.sq() <= ( model->size2 + length ) * ( model->size2 + length ))
			{
				const UnitType *pType = unit_manager.unit_type[type_id];
				const float scale = pType->Scale;
				const Matrix M = RotateXZY(-Angle.x * DEG2RAD, -Angle.z * DEG2RAD, -Angle.y * DEG2RAD) * Scale(1.0f / scale);
				const Vector3D RP = (P - Pos) * M;
				const bool is_hit = model->hit_fast(RP,Dir,&data,hit_vec,M);
				if (is_hit)
				{
					*hit_vec = ((*hit_vec) * RotateYZX(Angle.y * DEG2RAD, Angle.z * DEG2RAD, Angle.x * DEG2RAD)) * Scale(scale) + Pos;
					*hit_vec = ((*hit_vec - P) % Dir) * Dir + P;
				}

				pMutex.unlock();
				return is_hit;
			}
		}
		pMutex.unlock();
		return false;
	}

	void Unit::show_orders(bool only_build_commands, bool def_orders)				// Dessine les ordres reçus
	{
		if (!def_orders)
			show_orders( only_build_commands, true );

		pMutex.lock();

		if (!(flags&1))
		{
			pMutex.unlock();
			return;
		}

		bool low_def = (Camera::inGame->rpos.y > gfx->low_def_limit);

		MissionStack::iterator cur = def_orders ? def_mission.begin() : mission.begin();
		MissionStack::iterator end = def_orders ? def_mission.end() : mission.end();
		if (low_def)
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
		Vector3D p_target = Pos;
		Vector3D n_target = Pos;
		const float rab = float(msec_timer % 1000) * 0.001f;
		const UnitType *pType = unit_manager.unit_type[type_id];
        uint32	remaining_build_commands = !(pType->BMcode) ? 0 : 0xFFFFFFF;

		std::vector<Vector3D>	points;

		while(cur != end)
		{
			Unit *p = cur->lastStep().getTarget().getUnit();
			if (!only_build_commands)
			{
				const int curseur = anim_cursor(CURSOR_CROSS_LINK);
				const float dx = 0.5f * (float)cursor[CURSOR_CROSS_LINK].ofs_x[curseur];
				const float dz = 0.5f * (float)cursor[CURSOR_CROSS_LINK].ofs_y[curseur];
				float x,y,z;
				const float dist = ((Vector3D)(cur->lastStep().getTarget().getPos() - p_target)).norm();
				const int rec = (int)(dist / 30.0f);
				switch (cur->lastMission())
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
						if (cur->lastStep().getFlags() & MISSION_FLAG_TARGET_WEAPON)
						{
							++cur;
							continue;	// Don't show this, it'll be removed
						}
						n_target = cur->lastStep().getTarget().getPos();
						n_target.y = Math::Max(the_map->get_unit_h( n_target.x, n_target.z ), the_map->sealvl);
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
									x = p_target.x + (n_target.x - p_target.x) * ((float)i + rab) / (float)rec;
									z = p_target.z + (n_target.z - p_target.z) * ((float)i + rab) / (float)rec;
									y = Math::Max(the_map->get_unit_h( x, z ), the_map->sealvl);
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
			Vector3D target = cur->lastStep().getTarget().getPos();
			switch(cur->lastMission())
			{
				case MISSION_BUILD:
					if (p != NULL)
						target = p->Pos;
					if (cur->lastStep().getData() >= 0
						&& cur->lastStep().getData() < unit_manager.nb_unit
						&& remaining_build_commands > 0)
					{
						--remaining_build_commands;
						const float DX = float(unit_manager.unit_type[cur->lastStep().getData()]->FootprintX << 2);
						const float DZ = float(unit_manager.unit_type[cur->lastStep().getData()]->FootprintZ << 2);
						const byte blue = only_build_commands ? 0xFF : 0x00, green = only_build_commands ? 0x00 : 0xFF;
						glPushMatrix();
						glTranslatef(target.x,Math::Max( target.y, the_map->sealvl ), target.z);
						glDisable(GL_CULL_FACE);
						glDisable(GL_TEXTURE_2D);
						glEnable(GL_BLEND);
						glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
						glBegin(GL_QUADS);
						glColor4ub(0x00,green,blue,0xFF);
						glVertex3f(-DX,0.0f,-DZ);			// First quad
						glVertex3f(DX,0.0f,-DZ);
						glColor4ub(0x00,green,blue,0x00);
						glVertex3f(DX+2.0f,5.0f,-DZ-2.0f);
						glVertex3f(-DX-2.0f,5.0f,-DZ-2.0f);

						glColor4ub(0x00,green,blue,0xFF);
						glVertex3f(-DX,0.0f,-DZ);			// Second quad
						glVertex3f(-DX,0.0f,DZ);
						glColor4ub(0x00,green,blue,0x00);
						glVertex3f(-DX-2.0f,5.0f,DZ+2.0f);
						glVertex3f(-DX-2.0f,5.0f,-DZ-2.0f);

						glColor4ub(0x00,green,blue,0xFF);
						glVertex3f(DX,0.0f,-DZ);			// Third quad
						glVertex3f(DX,0.0f,DZ);
						glColor4ub(0x00,green,blue,0x00);
						glVertex3f(DX+2.0f,5.0f,DZ+2.0f);
						glVertex3f(DX+2.0f,5.0f,-DZ-2.0f);

						glEnd();
						glDisable(GL_BLEND);
						glEnable(GL_TEXTURE_2D);
						glEnable(GL_CULL_FACE);
						glPopMatrix();
						if (unit_manager.unit_type[cur->lastStep().getData()]->model != NULL)
						{
							glEnable(GL_LIGHTING);
							glEnable(GL_CULL_FACE);
							glEnable(GL_DEPTH_TEST);
							glPushMatrix();
							glTranslatef(target.x, target.y, target.z);
							glColor4ub(0x00,green,blue,0x7F);
							glDepthFunc( GL_GREATER );
							unit_manager.unit_type[cur->lastStep().getData()]->model->mesh->draw(0.0f,NULL,false,false,false);
							glDepthFunc( GL_LESS );
							unit_manager.unit_type[cur->lastStep().getData()]->model->mesh->draw(0.0f,NULL,false,false,false);
							glPopMatrix();
							glEnable(GL_BLEND);
							glEnable(GL_TEXTURE_2D);
							glDisable(GL_LIGHTING);
							glDisable(GL_CULL_FACE);
							glDisable(GL_DEPTH_TEST);
							glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
						}
						glPushMatrix();
						glTranslatef(target.x,Math::Max( target.y, the_map->sealvl ), target.z);
						glDisable(GL_CULL_FACE);
						glDisable(GL_TEXTURE_2D);
						glEnable(GL_BLEND);
						glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
						glBegin(GL_QUADS);
						glColor4ub(0x00,green,blue,0xFF);
						glVertex3f(-DX,0.0f,DZ);			// Fourth quad
						glVertex3f(DX,0.0f,DZ);
						glColor4ub(0x00,green,blue,0x00);
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
						glColor4ub(0xFF,0xFF,0xFF,0xFF);
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
						if (p != NULL)
							target = p->Pos;
						int cursor_type = CURSOR_ATTACK;
						switch(cur->lastMission())
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
						const int curseur = anim_cursor( cursor_type );
						const float x = target.x - 0.5f * (float)cursor[cursor_type].ofs_x[curseur];
						const float y = target.y + 1.0f;
						const float z = target.z - 0.5f * (float)cursor[cursor_type].ofs_y[curseur];
						const float sx = 0.5f * float(cursor[cursor_type].bmp[curseur]->w - 1);
						const float sy = 0.5f * float(cursor[cursor_type].bmp[curseur]->h - 1);
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
			++cur;
		}

		if (!points.empty())
		{
			const int curseur = anim_cursor(CURSOR_CROSS_LINK);
			const float sx = 0.5f * float(cursor[CURSOR_CROSS_LINK].bmp[curseur]->w - 1);
			const float sy = 0.5f * float(cursor[CURSOR_CROSS_LINK].bmp[curseur]->h - 1);

			Vector3D* P = new Vector3D[points.size() << 2];
			float* T = new float[points.size() << 3];

			int n = 0;
			for (std::vector<Vector3D>::const_iterator i = points.begin(); i != points.end(); ++i)
			{
				P[n] = *i;
				T[n<<1] = 0.0f;		T[(n<<1)|1] = 0.0f;
				++n;

				P[n] = *i;	P[n].x += sx;
				T[n<<1] = 1.0f;		T[(n<<1)|1] = 0.0f;
				++n;

				P[n] = *i;	P[n].x += sx;	P[n].z += sy;
				T[n<<1] = 1.0f;		T[(n<<1)|1] = 1.0f;
				++n;

				P[n] = *i;	P[n].z += sy;
				T[n<<1] = 0.0f;		T[(n<<1)|1] = 1.0f;
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

			DELETE_ARRAY(P);
			DELETE_ARRAY(T);
		}
		glDisable(GL_BLEND);
		pMutex.unlock();
	}


	int Unit::shoot(const int target, const Vector3D &startpos, const Vector3D &Dir, const int w_id, const Vector3D &target_pos)
	{
		const UnitType *pType = unit_manager.unit_type[type_id];
		const WeaponDef *pW = pType->weapon[ w_id ];        // Critical information, we can't lose it so we save it before unlocking this unit
		const int owner = owner_id;
		const Vector3D D = Dir * RotateY( -Angle.y * DEG2RAD );
		int param[] = { (int)(-10.0f*DEG2TA*D.z), (int)(-10.0f*DEG2TA*D.x) };
        launchScript( SCRIPT_RockUnit, 2, param );

        if (pW->startsmoke && visible)
			particle_engine.make_smoke(startpos, 0, 1, 0.0f, -1.0f, 0.0f, 0.3f);

        pMutex.unlock();

		weapons.lock();

		const int w_idx = weapons.add_weapon(pW->nb_id,idx);

		if (network_manager.isConnected() && local) // Send synchronization packet
		{
			struct event event;
			event.type = EVENT_WEAPON_CREATION;
			event.opt1 = idx;
			event.opt2 = (uint16)target;
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

		weapons.weapon[w_idx].damage = (float)pW->damage;
		weapons.weapon[w_idx].Pos = startpos;
		weapons.weapon[w_idx].local = local;
		if (Yuni::Math::Zero(pW->startvelocity) && !pW->selfprop)
			weapons.weapon[w_idx].V = pW->weaponvelocity * Dir;
		else
			weapons.weapon[w_idx].V = pW->startvelocity * Dir;
		//        if (pW->dropped || !pW->lineofsight)
		weapons.weapon[w_idx].V = weapons.weapon[w_idx].V + V;
		weapons.weapon[w_idx].owner = (byte)owner;
		weapons.weapon[w_idx].target = target;
		if (target >= 0)
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


	void Unit::draw_on_map()
	{
		const int type = type_id;
		if (type == -1 || !(flags & 1) )
			return;

		if (drawn)	clear_from_map();
		if (attached)	return;

		pMutex.lock();
		drawn_obstacle = is_obstacle();
		pMutex.unlock();
		drawn_flying = flying;
		const UnitType* const pType = unit_manager.unit_type[type];
		if (!flying)
		{
			// First check we're on a "legal" place if it can move
			pMutex.lock();
			if (pType->canmove && pType->BMcode
				&& !can_be_there( cur_px, cur_py, type_id, owner_id ) )
			{
				// Try to find a suitable place

				bool found = false;
				for( int r = 1 ; r < 20 && !found ; r++ ) // Circular check
				{
					const int r2 = r * r;
					for (int y = 0 ; y <= r ; ++y)
					{
						const int x = (int)(sqrtf(float(r2 - y * y)) + 0.5f);
						if (can_be_there( cur_px + x, cur_py + y, type_id, owner_id ) )
						{
							cur_px += x;
							cur_py += y;
							found = true;
							break;
						}
						if (can_be_there( cur_px - x, cur_py + y, type_id, owner_id ) )
						{
							cur_px -= x;
							cur_py += y;
							found = true;
							break;
						}
						if (can_be_there( cur_px + x, cur_py - y, type_id, owner_id ) )
						{
							cur_px += x;
							cur_py -= y;
							found = true;
							break;
						}
						if (can_be_there( cur_px - x, cur_py - y, type_id, owner_id ) )
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
					Pos.x = float((cur_px << 3) + 4 - the_map->map_w_d);
					Pos.z = float((cur_py << 3) + 4 - the_map->map_h_d);
					if (!mission.empty() && (mission->getFlags() & MISSION_FLAG_MOVE))
						mission->Flags() |= MISSION_FLAG_REFRESH_PATH;
				}
				else
				{
					LOG_ERROR(LOG_PREFIX_BATTLE << "units overlaps on yardmap !!");
				}
			}
			pMutex.unlock();

			if (!(pType->canmove && pType->BMcode) || drawn_obstacle)
				the_map->obstaclesRect( cur_px - (pType->FootprintX >> 1),
										cur_py - (pType->FootprintZ >> 1),
										pType->FootprintX, pType->FootprintZ, true,
										pType->yardmap, !Yuni::Math::Zero(port[YARD_OPEN]));
			the_map->rect( cur_px - (pType->FootprintX >> 1),
							 cur_py - (pType->FootprintZ >> 1),
							 pType->FootprintX, pType->FootprintZ,
							 idx, pType->yardmap, !Yuni::Math::Zero(port[YARD_OPEN]) );
			the_map->energy.add(pType->gRepulsion,
								  cur_px - (pType->gRepulsion.getWidth() >> 1),
								  cur_py - (pType->gRepulsion.getHeight() >> 1));
			drawn_open = !Yuni::Math::Zero(port[YARD_OPEN]);
		}
		drawn_x = cur_px;
		drawn_y = cur_py;
		drawn = true;
	}

	void Unit::clear_from_map()
	{
		if (!drawn)
			return;

		const int type = type_id;

		if (type == -1 || !(flags & 1) )
			return;

		const UnitType* const pType = unit_manager.unit_type[type];
		drawn = false;
		if (!drawn_flying)
		{
			if (!(pType->canmove && pType->BMcode) || drawn_obstacle)
				the_map->obstaclesRect( cur_px - (pType->FootprintX >> 1),
										cur_py - (pType->FootprintZ >> 1),
										pType->FootprintX, pType->FootprintZ, false,
										pType->yardmap, drawn_open);
			the_map->rect( drawn_x - (pType->FootprintX >> 1),
							 drawn_y - (pType->FootprintZ >> 1),
							 pType->FootprintX, pType->FootprintZ,
							 -1, pType->yardmap, drawn_open );
			the_map->energy.sub(pType->gRepulsion,
								  drawn_x - (pType->gRepulsion.getWidth() >> 1),
								  drawn_y - (pType->gRepulsion.getHeight() >> 1));
		}
	}

	void Unit::draw_on_FOW( bool jamming )
	{
		if (hidden || !Yuni::Math::Zero(build_percent_left))
			return;

		const int unit_type = type_id;

		if (flags == 0 || unit_type == -1)  return;

		const bool system_activated = (port[ACTIVATION] && unit_manager.unit_type[unit_type]->onoffable) || !unit_manager.unit_type[unit_type]->onoffable;

		if (jamming )
		{
			radar_jam_range = system_activated ? (unit_manager.unit_type[unit_type]->RadarDistanceJam >> 4) : 0;
			sonar_jam_range = system_activated ? (unit_manager.unit_type[unit_type]->SonarDistanceJam >> 4) : 0;

			the_map->update_player_visibility( owner_id, cur_px, cur_py, 0, 0, 0, radar_jam_range, sonar_jam_range, true );
		}
		else
		{
			uint32 cur_sight = ((int)h + unit_manager.unit_type[unit_type]->SightDistance) >> 4;
			radar_range = system_activated ? (unit_manager.unit_type[unit_type]->RadarDistance >> 4) : 0;
			sonar_range = system_activated ? (unit_manager.unit_type[unit_type]->SonarDistance >> 4) : 0;

			the_map->update_player_visibility(owner_id, cur_px, cur_py, cur_sight, radar_range, sonar_range, 0, 0, false, old_px != cur_px || old_py != cur_py || cur_sight != sight);

			sight = cur_sight;
			old_px = cur_px;
			old_py = cur_py;
		}
	}



	void Unit::playSound(const String &key)
	{
		pMutex.lock();
		if (owner_id == players.local_human_id && int(msec_timer - last_time_sound) >= units.sound_min_ticks )
		{
			last_time_sound = msec_timer;
			const UnitType *pType = unit_manager.unit_type[type_id];
            sound_manager->playTDFSound(pType->soundcategory, key , &Pos);
		}
		pMutex.unlock();
	}



	int Unit::launchScript(const int id, int nb_param, int *param)			// Start a script as a separate "thread" of the unit
	{
		const int type = type_id;
        if (!script || type == -1 || !unit_manager.unit_type[type]->script || !unit_manager.unit_type[type]->script->isCached(id))
            return -2;
        const String& f_name = UnitScriptInterface::get_script_name(id);
		if (f_name.empty())
			return -2;

		MutexLocker locker(pMutex);

		if (local && network_manager.isConnected()) // Send synchronization event
		{
			struct event event;
			event.type = EVENT_UNIT_SCRIPT;
			event.opt1 = idx;
			event.opt2 = (uint16)id;
			event.opt3 = nb_param;
			memcpy(event.str, param, sizeof(int) * nb_param);
			network_manager.sendEvent(&event);
		}

		ScriptInterface *newThread = script->fork( f_name, param, nb_param );

        if (newThread == NULL || !newThread->is_self_running())
        {
            unit_manager.unit_type[type]->script->Uncache(id);
			return -2;
        }

		return 0;
	}

    int Unit::get_sweet_spot()
    {
        if (type_id < 0)
            return -1;
        UnitType *pType = unit_manager.unit_type[type_id];
        if (pType->sweetspot_cached == -1)
        {
            lock();
            pType->sweetspot_cached = runScriptFunction(SCRIPT_SweetSpot);
            unlock();
        }
        return pType->sweetspot_cached;
    }

	void Unit::drawHealthBar() const
	{
		if (render.type_id < 0 || owner_id != players.local_human_id || render.UID != ID)
			return;

		const int maxdmg = unit_manager.unit_type[render.type_id]->MaxDamage;
		Vector3D vPos = render.Pos;
		const float size = unit_manager.unit_type[render.type_id]->model->size2 * 0.5f;

		const float scale = 200.0f;
		float w = 0.04f * scale;
		float h = 0.006f * scale;
		vPos -= size * Camera::inGame->up;
		units.hbars_bkg.push_back(vPos - w * Camera::inGame->side + h * Camera::inGame->up);
		units.hbars_bkg.push_back(vPos + w * Camera::inGame->side + h * Camera::inGame->up);
		units.hbars_bkg.push_back(vPos + w * Camera::inGame->side - h * Camera::inGame->up);
		units.hbars_bkg.push_back(vPos - w * Camera::inGame->side - h * Camera::inGame->up);

		w -= scale * gfx->SCREEN_W_INV;
		h -= scale * gfx->SCREEN_H_INV;

		if (hp <= 0.0f)
			return;

		uint32 color = makeacol(0x50, 0xD0, 0x50, 0xFF);
		if (hp <= (maxdmg >> 2))
			color = makeacol(0xFF, 0x46, 0x00, 0xFF);
		else if (hp <= (maxdmg >> 1))
			color = makeacol(0xFF, 0xD0, 0x00, 0xFF);
		units.hbars_color.push_back(color);
		units.hbars_color.push_back(color);
		units.hbars_color.push_back(color);
		units.hbars_color.push_back(color);
		const float pw = w * (2.0f * (hp / (float)maxdmg) - 1.0f);
		units.hbars.push_back(vPos - w * Camera::inGame->side + h * Camera::inGame->up);
		units.hbars.push_back(vPos + pw * Camera::inGame->side + h * Camera::inGame->up);
		units.hbars.push_back(vPos + pw * Camera::inGame->side - h * Camera::inGame->up);
		units.hbars.push_back(vPos - w * Camera::inGame->side - h * Camera::inGame->up);
	}

	void Unit::renderTick()
	{
		render.UID = ID;
		render.Pos = Pos;
		render.Angle = Angle;
		lock();
		render.Anim = data;
		unlock();
		render.px = cur_px;
		render.py = cur_py;
		render.type_id = type_id;
	}

} // namespace TA3D
