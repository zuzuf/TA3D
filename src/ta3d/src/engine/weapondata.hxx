#ifndef __TA3D_ENGINE_WEAPON_DATA_HXX__
# define __TA3D_ENGINE_WEAPON_DATA_HXX__


namespace TA3D
{


	inline WeaponData::WeaponData()
		: aim_dir(), target_pos(), target(NULL), delay(0.), time(0.), aim_piece(-1),
		burst(0), stock(0), data(-1), state(WEAPON_FLAG_IDLE), flags(0)
	{}

	inline WeaponData::WeaponData(const WeaponData& copy)
		: aim_dir(copy.aim_dir), target_pos(copy.target_pos), target(copy.target),
		delay(copy.delay), time(copy.time), aim_piece(copy.aim_piece),
		burst(copy.burst), stock(copy.stock), data(copy.data),
		state(copy.state), flags(copy.flags)
	{}


	inline WeaponData::~WeaponData()
	{}


	inline void WeaponData::init()
	{
		delay = 0.;
		time = 0.;
		burst = 0;
		stock = 0;
		state = WEAPON_FLAG_IDLE;
		flags = 0;
		target_pos.reset();
		target = NULL;
		data = -1;
		aim_dir.reset();
        aim_piece = -1;
	}


	inline WeaponData& WeaponData::operator = (const WeaponData& rhs)
	{
		aim_dir = rhs.aim_dir;
		target_pos = rhs.target_pos;
		target = rhs.target;
		delay = rhs.delay;
		time = rhs.time;
		aim_piece = rhs.aim_piece;
		burst = rhs.burst;
		stock = rhs.stock;
		data = rhs.data;
		state = rhs.state;
		flags = rhs.flags;
		return *this;
	}



} // namespace TA3D


#endif // __TA3D_ENGINE_WEAPON_DATA_HXX__
