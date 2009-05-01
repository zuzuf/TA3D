#ifndef __TA3D_ENGIN_WEAPON_DATA_H__
# define __TA3D_ENGIN_WEAPON_DATA_H__

# include <vector>
# include "../stdafx.h"
# include "../misc/vector.h"


//! When IDLE the weapon can target a unit
# define WEAPON_FLAG_IDLE			0x0
//! The weapon is aiming
# define WEAPON_FLAG_AIM			0x1
//! Fire
# define WEAPON_FLAG_SHOOT			0x2
//! The weapon is targeting a weapon
# define WEAPON_FLAG_WEAPON			0x4
//! The unit didn't auto select this target, it's on user command
# define WEAPON_FLAG_COMMAND_FIRE	0x8



namespace TA3D
{


	/*!
	** \brief Weapon Data
	*/
	class WeaponData
	{
	public:
		//! Vector of Weapon data
		typedef std::vector<WeaponData>  Vector;

	public:
		//! \name Constructor & Destructor
		//@{
		//! Default Constructor
		WeaponData();
		//! Copy constructor
		WeaponData(const WeaponData& copy);
		//! Destructor
		~WeaponData();
		//@}

		/*!
		** \brief Reset all data
		*/
		void init();

		//! \name Operators
		//@{
		//! Operator =
		WeaponData& operator = (const WeaponData& rhs);
		//@}

	public:
		//! Delay
		float delay;
		//! Time
		float time;
		//! Burst
		uint16 burst;
		//! Stock
		uint16 stock;
		//! State
		byte state;
		//! Flags
		byte flags;
		//! Position of the target
		Vector3D target_pos;
		//! Target
		void *target;
		//! Data
		sint16	data;
		//! Aim direction
		Vector3D aim_dir;


	}; // class WeaponData


} // namespace TA3D

# include "weapondata.hxx"

#endif // __TA3D_ENGIN_WEAPON_DATA_H__
