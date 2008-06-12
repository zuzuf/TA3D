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


#ifndef TA3D_NETWORK_THREAD
#define TA3D_NETWORK_THREAD

#include "cCriticalSection.h"


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

class BaseThread{

	virtual void proc(void* param) = 0;
	protected:
		int dead;

	public:
		BaseThread(){dead=1;}
		virtual ~BaseThread(){dead=1;}
		virtual void Spawn(void* param)=0;
		virtual void Join()=0;
};



#ifdef TA3D_PLATFORM_WINDOWS

class Thread : public BaseThread{

	private:

	DWORD threadid;
	HANDLE thread;
	struct thread_params{
		void* more;
		Thread* thisthread;
	};
	struct thread_params secondary;
	
	virtual void proc(void* param) = 0;
	static DWORD WINAPI run(LPVOID param){
		((struct thread_params*)param)->thisthread->proc(((struct thread_params*)param)->more);
		return 0;
	}

	public:
		virtual void Spawn(void* param){
			dead = 0;
			secondary.thisthread = this;
			secondary.more = param;
			thread = ::CreateThread(NULL,0,run,&secondary,0,&threadid);
		}
		virtual void Join(){
			dead = 1;
			::WaitForSingleObject(thread,2000);
		}
		virtual bool isDead(){
			return dead;
		}
};

#endif /*WIN32*/




#if defined TA3D_PLATFORM_LINUX || defined TA3D_PLATFORM_DARWIN
#include <pthread.h>

class Thread : public BaseThread{

	pthread_t thread;
	struct thread_params{
		void* more;
		Thread* thisthread;
	};
	struct thread_params secondary;

	virtual void proc(void* param)=0;
	static void* run(void* param){
		((struct thread_params*)param)->thisthread->proc(((struct thread_params*)param)->more);
		return NULL;
	}


	public:
		virtual void Spawn(void* param){
			dead = 0;
			
			secondary.thisthread = this;
			secondary.more = param;

			pthread_create(&thread,NULL,run,&secondary);
		}
		virtual void Join(){
			if(dead)
				return;
			dead = 1;
			pthread_join(thread,NULL);
		}
		virtual bool isDead(){
			return dead;
		}
};

#endif /*LINUX*/

class Mutex : protected cCriticalSection {
	int locked;

	public:
		Mutex() {locked=0; CreateCS();}
		~Mutex() {DeleteCS();}
		void Lock() {locked=1; EnterCS();}
		void Unlock() {locked=0; LeaveCS();}
		int isLocked() {return locked;}
};

#endif
