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

	bool MissionStack::doNothing()
	{
		return empty() || ((this->mission() == MISSION_STOP
							 || this->mission() == MISSION_STANDBY
							 || this->mission() == MISSION_VTOL_STANDBY)
							&& size() == 1 && this->size() == 1);
	}

	bool MissionStack::doNothingAI()
	{
		return empty() || ((this->mission() == MISSION_STOP
							|| this->mission() == MISSION_STANDBY
							|| this->mission() == MISSION_VTOL_STANDBY
							|| this->mission() == MISSION_MOVE)
							&& size() == 1 && this->size() == 1);
	}

#define SAVE( i )	gzwrite( file, (void*)&(i), sizeof( i ) )
#define LOAD( i )	gzread( file, (void*)&(i), sizeof( i ) )

	void Mission::Target::save(gzFile file) const
	{
		SAVE(type);
		SAVE(idx);
		SAVE(UID);
		SAVE(Pos);
	}

	void Mission::Target::load(gzFile file)
	{
		LOAD(type);
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

	void Mission::save(gzFile file) const
	{
		SAVE(time);
		SAVE(last_d);
		SAVE(move_data);

		for (PATH::const_iterator p = path.begin() ; p != path.end() ; ++p)
		{
			gzputc(file, 1);
			SAVE( p->x );
			SAVE( p->y );
			SAVE( p->Pos );
			SAVE( p->made_direct );
		}
		gzputc(file, 0);

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

		path.clear();
		while (gzgetc( file ))
		{
			PATH_NODE node;
			LOAD( node.x );
			LOAD( node.y );
			LOAD( node.Pos );
			LOAD( node.made_direct );
			path.push_back(node);
		}

		LOAD(node);

		int s;
		LOAD(s);
		qStep.clear();
		for(int i = 0 ; i < s ; ++i)
		{
			qStep.push(MissionStep());
			qStep.bottom().load(file);
		}
	}

	void MissionStack::save(gzFile file) const
	{
		for(Container::const_iterator i = sMission.begin() ; i != sMission.end() ; ++i)
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
			Mission m;
			m.load(file);
			add(m);
		}
	}
}
