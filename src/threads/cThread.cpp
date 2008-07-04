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

#include "../stdafx.h"
#include "cThread.h"

namespace TA3D {
   void cThread::InitThread()
   {
      m_bStarted   = false;
      m_ErroredOnRun = false;

#ifdef TA3D_PLATFORM_WINDOWS
      m_hThread   = NULL;
#endif
   }

   bool cThread::InWorkerThread()
   {
      if( m_bStarted == false )
         return false;

#ifdef TA3D_PLATFORM_WINDOWS
      return ( ::GetCurrentThreadId() == m_ThreadID );
#else
      return ::pthread_equal( pthread_self(), m_ThreadID );
#endif
   }

   void cThread::Start()
   {
      if( InWorkerThread() )
         throw "cThread:Start() Thread shouln't create threads.";

      if( m_bStarted )
         throw "cThread:Start() Thread is already running.";

      m_bStarted   = true;

#ifdef TA3D_PLATFORM_WINDOWS
      m_hThread = ::CreateThread( NULL,
         0,
         (LPTHREAD_START_ROUTINE)ThreadFunction,
         this,
         0,
         &m_ThreadID );
         
      if( !m_hThread )
         throw "cThread::Start() Failed to CreateThread";
#else
      if( ::pthread_create( &m_ThreadID, NULL, ThreadFunction, this ) < 0 )
         throw "cThread::Start() Failed to pthread_create";
#endif
   }

#ifdef TA3D_PLATFORM_WINDOWS
   unsigned long WINAPI cThread::ThreadFunction( void *pV )
#else
   void * cThread::ThreadFunction( void *pV )
#endif
   {
      int result = 0;

      cThread* pThis = (cThread*)pV;

      if (pThis) {
         try {
            result = pThis->Run();
            }
         catch(...) {
            pThis->m_ErroredOnRun = true;
            //DestoryThread();
            // TODO: handle errors.
            }
         }


#ifndef TA3D_PLATFORM_WINDOWS
      return (void*) result;
#else
      return result;
#endif
   }

   void cThread::DestroyThread()
   {
      if( !m_bStarted )
         return;

      this->SignalExitThread();

#ifdef TA3D_PLATFORM_WINDOWS
   ::WaitForSingleObject( m_hThread, INFINITE );
   ::CloseHandle( m_hThread );
#else
   ::pthread_join( m_ThreadID, NULL );
#endif

      m_bStarted = false;
   }

   bool cThread::IsRunning()
   {
      if( !m_bStarted || m_ErroredOnRun )
         return false;

      return true;
   }
} 
