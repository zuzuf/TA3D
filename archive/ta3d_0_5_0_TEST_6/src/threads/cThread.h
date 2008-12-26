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

#ifndef __TA3D_THREADS_H__
# define __TA3D_THREADS_H__


#ifndef TA3D_PLATFORM_WINDOWS
# include <pthread.h>
#else
# define __WIN32_LEAN_AND_MEAN__
# include <windows.h>
#endif



namespace TA3D
{
    class cThread
    {
    protected:
        #ifdef TA3D_PLATFORM_WINDOWS
        DWORD      m_ThreadID;
        HANDLE      m_hThread;
        #else
        pthread_t   m_ThreadID;
        #endif
        bool      m_bStarted;
        bool      m_ErroredOnRun;

    public:
        bool IsRunning();


    private:
        #if defined TA3D_PLATFORM_WINDOWS
        static unsigned long WINAPI ThreadFunction( void *pV );
        #else
        static void *ThreadFunction( void *pV );
        #endif

   protected:
      // InitThread used to initialize the threads variables.
      void InitThread();

      // Returns true if current calling thread == our thread
      bool InWorkerThread();

	  virtual ~cThread() {}

   public:
      // Call this to end the Thread, it will signal the thread to tell it to end
      //   and will block until the thread ends.
      void DestroyThread();

      void Start();

   protected:
      virtual int Run() = 0;
      virtual void SignalExitThread() = 0;

   }; // class cThread


} // namespace TA3D 


#endif // __TA3D_THREADS_H__
