#ifndef __TA3D_ENGINE_WEAPON_DATA_HXX__
# define __TA3D_ENGINE_WEAPON_DATA_HXX__


namespace TA3D
{


	inline WeaponData::WeaponData()
		:delay(0.), time(0.), burst(0), stock(0), state(WEAPON_FLAG_IDLE),
		flags(0), target_pos(), target(NULL), data(-1), aim_dir()
	{}

	inline WeaponData::WeaponData(const WeaponData& copy)
		:delay(copy.delay), time(copy.time), burst(copy.burst), stock(copy.stock),
		state(copy.state), flags(copy.flags), target_pos(copy.target_pos), target(copy.target),
		data(copy.data), aim_dir(copy.aim_dir)
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
	}


	inline WeaponData& WeaponData::operator = (const WeaponData& rhs)
	{
		delay = rhs.delay;
		time = rhs.time;
		burst = rhs.burst;
		stock = rhs.stock;
		state = rhs.state;
		flags = rhs.flags;
		target_pos = rhs.target_pos;
		target = rhs.target;
		data = rhs.data;
		aim_dir = rhs.aim_dir;
		return *this;
	}



} // namespace TA3D


#endif // __TA3D_ENGINE_WEAPON_DATA_HXX__
