/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2005  Roland BROCHARD

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/
#ifndef __TA3D_InGameWeapons_ING_H__
# define __TA3D_InGameWeapons_ING_H__

# include <stdafx.h>
# include <threads/thread.h>
# include "weapons.h"
# include "weapons.single.h"
# include <misc/camera.h>

namespace TA3D
{
	template<typename T, typename TKit>	class BVH;
	class Unit;

	// Define the TKit structure required by the KDTree structure
	class BVH_UnitTKit
	{
	public:
		typedef Vector3D		Vec;
		typedef std::pair<const Unit*, std::pair<Vec, float> >		T;

		struct Predicate
		{
			const unsigned int D;
			const float f;

			inline Predicate(const Vec &v, const unsigned int N) : D(N), f(v[N])	{}

			inline bool operator() (const T &i) const
			{
				return i.second.first[D] < f;
			}
		};

	public:
		static inline const Vec &pos(const T &elt)	{	return elt.second.first;	}
		static inline float radius(const T &elt)	{	return elt.second.second;	}
		static inline void getTopBottom(const std::vector<T>::const_iterator &begin, const std::vector<T>::const_iterator &end, Vec &top, Vec &bottom);
		static inline unsigned int getPrincipalDirection(const Vec &v)
		{
			const Vector3D m = TA3D::Math::Abs(v);
			if (m.x > m.y)
			{
				if (m.x > m.z)
					return 0U;
				return 2U;
			}
			if (m.y > m.z)
				return 1U;
			return 2U;
		}
	};

	inline void BVH_UnitTKit::getTopBottom(const std::vector<T>::const_iterator &begin, const std::vector<T>::const_iterator &end, Vec &top, Vec &bottom)
	{
		if (begin == end)
		{
			top = bottom = Vector3D();
			return;
		}
		top = bottom = begin->second.first;
		for(std::vector<BVH_UnitTKit::T>::const_iterator i = begin ; i != end ; ++i)
		{
			const Vector3D L(i->second.second, i->second.second, i->second.second);
			top = Math::Max(top, i->second.first + L);
			bottom = Math::Min(bottom, i->second.first - L);
		}
	}

	/*! \class InGameWeapons
    **
    ** \brief
    */
	class InGameWeapons : public ObjectSync, public Thread
    {
    public:
        //! \name Constructor & Destructor
        //@{
        //! Default constructor
		InGameWeapons();
        //! Destructor
		~InGameWeapons();
        //@}


        /*!
        ** \brief
        ** \param real
        */
        void init(bool real = true);

        /*!
        ** \brief
        */
        void destroy();


        /*!
        ** \brief
        ** \param weapon_id
        ** \param shooter
        */
        int add_weapon(int weapon_id,int shooter);

        /*!
        ** \brief
        */
		void move(const float dt);

        /*!
        ** \brief
        */
		void draw(bool underwater = false);

        /*!
        ** \brief
        */
        void draw_mini(float map_w, float map_h, int mini_w, int mini_h); // Repère les unités sur la mini-carte


    public:
        //! Weapons count
        uint32 nb_weapon;			// Nombre d'armes
        //!
		std::vector< Weapon > weapon;			// Tableau regroupant les armes
        //!
        Gaf::Animation nuclogo;			// Logos des armes atomiques sur la minicarte / Logo of nuclear weapons on minimap

        //!
        std::vector< uint32 > idx_list;
        //!
        std::vector< uint32 > free_idx;
		//! A BVH structure to store units (for fast collision detection)
		BVH< BVH_UnitTKit::T, BVH_UnitTKit > *bvhUnits;

    protected:
        //!
		volatile bool thread_running;
        //!
		volatile bool thread_ask_to_stop;
        //!
        void proc(void*);
        //!
        void signalExitThread();

	}; // class InGameWeapons



	extern InGameWeapons weapons;


}

#endif // __TA3D_InGameWeapons_ING_H__
