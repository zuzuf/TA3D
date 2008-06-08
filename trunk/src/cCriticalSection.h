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
#pragma once

#if defined TA3D_PLATFORM_LINUX
	#include <pthread.h>
#endif

namespace TA3D
{
	class cCriticalSection
	{
	// Member functions
	protected:
		void CreateCS();
		void DeleteCS();

		void EnterCS();
		void LeaveCS();
		
	// Member Variables
	private:
#if defined TA3D_PLATFORM_WINDOWS
		CRITICAL_SECTION m_hCritSection;
//#elif defined TA3D_PLATFORM_LINUX
//		
#else
		pthread_mutex_t m_hCritSection;
#endif
	}; // class cCriticalSection

	class ThreadSync : protected cCriticalSection
	{
	public:
		ThreadSync()	{	CreateCS();	}
		~ThreadSync()	{	DeleteCS();	}

		void EnterSync()	{	EnterCS();	}
		void LeaveSync()	{	LeaveCS();	}
	};
} // namespace TA3D;
