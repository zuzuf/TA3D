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

#ifndef __TA3D_THREADS_MUTEX_H__
# define __TA3D_THREADS_MUTEX_H__

# include <QMutex>
# include <QMutexLocker>
# include <QWaitCondition>

namespace TA3D
{

    class Mutex : public QMutex
    {
    public:
        Mutex() : QMutex(QMutex::Recursive) {}
    };

    class MutexLocker : public QMutexLocker
    {
    public:
        MutexLocker(Mutex &mtx) : QMutexLocker(&mtx)    {}
    };

	class Synchronizer
	{
	public:
		Synchronizer(int nbThreadsToSync);
		virtual ~Synchronizer();

		void sync();
		void release();
		void lock()	{	pMutex.lock();	}
		void unlock()	{	pMutex.lock();	}

		int getNbWaitingThreads()	{	return nbThreadsWaiting;	}
		void setNbThreadsToSync(int n)	{	nbThreadsToSync = n;	}

	protected:
		int nbThreadsToSync;
		volatile int nbThreadsWaiting;
        QMutex pMutex;

		//! The PThread Condition
        QWaitCondition  pCondition;

		//! Have the condition been really signalled ?
		volatile unsigned int pSignalled;
	};

} // namespace TA3D


#endif // __TA3D_THREADS_MUTEX_H__
