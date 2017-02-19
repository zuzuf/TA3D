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

#include "mutex.h"
#include <zuzuf/smartptr.h>
#include <QThread>

namespace TA3D
{

    class Thread
	{
        class ThreadType : public QThread
        {
        public:
            ThreadType(Thread *ptr) : ptr(ptr)  {}

            virtual void run();

        public:
            void *param;
            Thread *ptr;
        };

	protected:
		volatile int pDead;
        ThreadType _thread;

	protected:
		Thread();
		virtual ~Thread();
		virtual void proc(void* param) = 0;
		virtual void signalExitThread() {}

	public:
		// Call this to end the Thread, it will signal the thread to tell it to end
		//   and will block until the thread ends.
		void destroyThread() { join(); }
        bool isDead() const { return pDead; }
        bool isRunning() const  {   return pDead == 0;  }

        bool suspend(int ms)	{ QThread::msleep(ms);   return false; }

		void start()	{	spawn(NULL);	}

		virtual void spawn(void* param);
		virtual void join();
	}; // class Thread





    class ObjectSync : public virtual zuzuf::ref_count
	{
	public:
        typedef zuzuf::smartptr<ObjectSync>	Ptr;
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

        //! \brief Don't copy the mutex
        ObjectSync &operator=(const ObjectSync&)
        {
            return *this;
        }

	protected:
		//! Mutex
		Mutex pMutex;
	};





} // namespace TA3D

#endif      // __TA3D_THREAD_H__
