#include "mission.h"
#include <UnitEngine.h>

namespace TA3D
{
	Unit *Mission::Target::getUnit() const
	{
		if (type != TargetUnit || idx < 0 || idx >= units.max_unit)
			return NULL;
		Unit *p = &(units.unit[idx]);
		return (p->ID == UID) ? p : NULL;
	}

	Weapon *Mission::Target::getWeapon() const
	{
		if (type != TargetWeapon || idx < 0 || idx >= weapons.weapon.size())
			return NULL;
		Weapon *p = &(weapons.weapon[idx]);
		return p;
	}


	bool Mission::Target::isUnit() const
	{
		return type == TargetUnit && idx >= 0 && idx < units.max_unit && units.unit[idx].ID == UID;
	}


	bool Mission::Target::isWeapon() const
	{
		return type == TargetWeapon && idx >= 0 && idx < weapons.weapon.size();
	}


	bool Mission::Target::isValid() const
	{
		switch(type)
		{
			case TargetNone:
			case TargetStatic:
				return true;
			case TargetUnit:
				return idx >= 0 && idx < units.max_unit && units.unit[idx].ID == UID;
			case TargetWeapon:
				return idx >= 0 && idx < weapons.weapon.size();
		}
		return false;
	}


	const Vector3D &Mission::Target::getPos() const
	{
		switch(type)
		{
		case TargetUnit:
			{
				Unit *p = getUnit();
				if (p)
					return p->Pos;
			}
			break;
		case TargetWeapon:
			{
				Weapon *p = getWeapon();
				if (p)
					return p->Pos;
			}
			break;
		case TargetNone:
		case TargetStatic:
			return Pos;
		};

		return Pos;
	}

	bool MissionStack::doNothing() const
	{
		return empty() || ((this->mission() == MISSION_STOP
							 || this->mission() == MISSION_STANDBY
							 || this->mission() == MISSION_VTOL_STANDBY)
							&& size() == 1 && front().size() == 1);
	}

	bool MissionStack::doingNothing() const
	{
		return empty() || (this->mission() == MISSION_STOP
							 || this->mission() == MISSION_STANDBY
							 || this->mission() == MISSION_VTOL_STANDBY);
	}

	bool MissionStack::doNothingAI() const
	{
		return empty() || ((this->mission() == MISSION_STOP
							|| this->mission() == MISSION_STANDBY
							|| this->mission() == MISSION_VTOL_STANDBY
							|| this->mission() == MISSION_MOVE)
							&& size() == 1 && front().size() == 1);
	}

#define SAVE( i )	gzwrite( file, (void*)&(i), sizeof( i ) )
#define LOAD( i )	gzread( file, (void*)&(i), sizeof( i ) )

	void Mission::Target::save(gzFile file) const
	{
		uint8 t = type;
		SAVE(t);
		SAVE(idx);
		SAVE(UID);
		SAVE(Pos);
	}

	void Mission::Target::load(gzFile file)
	{
		uint8 t;
		LOAD(t);
		type = (Type)t;
		LOAD(idx);
		LOAD(UID);
		LOAD(Pos);
	}

	void Mission::MissionStep::save(gzFile file) const
	{
		SAVE(type);
		target.save(file);
		SAVE(flags);
		SAVE(data);
	}

	void Mission::MissionStep::load(gzFile file)
	{
		LOAD(type);
		target.load(file);
		LOAD(flags);
		LOAD(data);
	}

	void Mission::save(gzFile file)
	{
		SAVE(time);
		SAVE(last_d);
		SAVE(move_data);

		path.save(file);

		SAVE(node);

		int s = qStep.size();
		SAVE(s);
		for(int i = 0 ; i < s ; ++i)
			qStep[i].save(file);
	}

	void Mission::load(gzFile file)
	{
		LOAD(time);
		LOAD(last_d);
		LOAD(move_data);

		path.load(file);

		LOAD(node);

		int s;
		LOAD(s);
		qStep.clear();
		for(int i = 0 ; i < s ; ++i)
		{
			qStep.push(MissionStep());
			qStep.top().load(file);
		}
	}

	void MissionStack::save(gzFile file)
	{
		for(Container::iterator i = sMission.begin() ; i != sMission.end() ; ++i)
		{
			gzputc(file, 1);
			i->save(file);
		}
		gzputc(file, 0);
	}

	void MissionStack::load(gzFile file)
	{
		sMission.clear();
		while (gzgetc(file))
		{
			add(Mission());
			sMission.back().load(file);
		}
	}
}
