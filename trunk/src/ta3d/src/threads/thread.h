/*  TA3D, a remake of Total Annihilation
	Copyright (C) 2006  Roland BROCHARD

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


#ifndef __TA3D_THREAD_H__
#define __TA3D_THREAD_H__

#include <SDL_thread.h>
#include "mutex.h"

namespace TA3D
{


	/*
	**  Guide to thread classes
	**
	**  To make new thread classes you need to
	**  make a subclass of Thread and define the
	**  proc method. In this method you need to
	**  dereference the params struct as whatever
	**  type you are using (like net_thread_params)
	**
	*/

	class BaseThread
	{
	private:
		virtual void proc(void* param) = 0;

	protected:
		int pDead;

	public:
		BaseThread();
		virtual ~BaseThread();
		virtual void spawn(void* param) = 0;
		virtual void join() = 0;
		virtual bool isDead() { return (pDead != 0); }
	};




	class Thread : public BaseThread
	{
	private:
		struct thread_params
		{
			void* more;
			Thread* thisthread;
		};
		struct thread_params secondary;

	protected:
		SDL_Thread  *thread;

	private:
		static int run(void* param)
		{
			((struct thread_params*)param)->thisthread->proc(((struct thread_params*)param)->more);
			return 0;
		}

	protected:
		// Returns true if current calling thread == our thread
		bool inWorkerThread();

		virtual ~Thread() {}
		virtual void proc(void* param) = 0;
		virtual void signalExitThread() {};

		//! Helpful for debugging, this function returns the name of the class (needs to be implemented for each class)
		virtual const char *className() = 0;

	public:
		// Call this to end the Thread, it will signal the thread to tell it to end
		//   and will block until the thread ends.
		void destroyThread() { join(); }
		void start() { spawn(NULL); }
		bool isRunning() const    { return pDead == 0;    }

		virtual void spawn(void* param);
		virtual void join();
		virtual bool isDead() const { return !isRunning(); }

	}; // class Thread





	class ObjectSync
	{
	public:
		//! \name Constructor & Destructor
		//@{
		ObjectSync() {}
		virtual ~ObjectSync() {}
		//@}

		//! Lock the object
		void lock() { pMutex.lock(); }
		//! Unlock the object
		void unlock() { pMutex.unlock(); }

	protected:
		//! Mutex
		Mutex pMutex;
	};





} // namespace TA3D

#endif      // __TA3D_THREAD_H__
