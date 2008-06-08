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

#include "stdafx.h"
#include "cCriticalSection.h"

namespace TA3D
{
	void cCriticalSection::CreateCS()
	{
        #ifdef TA3D_PLATFORM_WINDOWS
		::InitializeCriticalSection(&m_hCritSection);
        #else
		pthread_mutexattr_t mutexattr;
		pthread_mutexattr_init(&mutexattr);
        # ifdef TA3D_PLATFORM_DARWIN
		pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE);
        # else
		pthread_mutexattr_settype(&mutexattr, PTHREAD_MUTEX_RECURSIVE_NP);
        # endif
		::pthread_mutex_init( &m_hCritSection, &mutexattr );
		pthread_mutexattr_destroy(&mutexattr);
        #endif
	}

	void cCriticalSection::DeleteCS()
	{
        #ifdef TA3D_PLATFORM_WINDOWS
		::DeleteCriticalSection( &m_hCritSection );
        #else
		::pthread_mutex_unlock( &m_hCritSection );
        #endif
	}
} // namespace TA3D
