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

#include "thread.h"
#include <logs/logs.h>
#include <typeinfo>


namespace TA3D
{
    Thread::Thread() : pDead(1), _thread(this)
	{
	}

	Thread::~Thread()
	{
        join();
	}

	void Thread::spawn(void* param)
	{
        _thread.param = param;
        pDead = 0;
        _thread.start();
	}


    void Thread::ThreadType::run()
    {
        ptr->proc(param);
        ptr->pDead = 1;
    }

    void Thread::join()
	{
		if (pDead == 0)
		{
			signalExitThread();
			pDead = 1;
            _thread.wait();
		}
	}
} // namespace TA3D
