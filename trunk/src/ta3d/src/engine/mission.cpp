#include "mission.h"
#include "../UnitEngine.h"

namespace TA3D
{
	Unit *Mission::Target::getUnit()
	{
		if (type != TargetUnit || idx < 0 || idx >= units.nb_unit)
			return NULL;
		Unit *p = &(units.unit[idx]);
		return p->ID == UID ? p : NULL;
	}

	Weapon *Mission::Target::getWeapon()
	{
		if (type != TargetWeapon || idx < 0 || idx >= weapons.nb_weapon)
			return NULL;
		Weapon *p = &(weapons.weapon[idx]);
		return p;
	}

	const Vector3D &Mission::Target::getPos()
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
			return Pos;
		};

		return Pos;
	}
}
