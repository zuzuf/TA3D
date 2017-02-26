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

#ifndef __TA3D_ENGINE_H__
# define __TA3D_ENGINE_H__

# include "threads/thread.h"



namespace TA3D
{


    class Engine : public QThread
	{
		// These classes need access to the Synchronizer object
		friend class INGAME_UNITS;
		friend class InGameWeapons;
		friend class PARTICLE_ENGINE;
		friend class PLAYERS;
		friend class Battle;
	public:
		//! \name Constructor & Destructor
		//@{
		//! Default Constructor
		Engine();
		//! Destructor
		virtual ~Engine();
		//@}

		bool GFXModeActive() const { return pGFXModeActive; }

		/*!
		** \brief Make initialization (from the main thread only) after the engine was initialized from another thread
		*/
		void initializationFromTheMainThread();

        inline bool started() const {   return bStarted;    }
	protected:
        virtual void run();

	private:
		/*!
		** \brief Didplay informations about the current configuration of OpenGL
		*/
		void displayInfosAboutOpenGL() const;

	private:
		bool pGFXModeActive;
        volatile bool bStarted;

	private:
		static void sync()
		{
			synchronizer.sync();
		}
	private:
		static Synchronizer synchronizer;
	}; // class Engine




} // namespace TA3D

#endif // __TA3D_ENGINE_H__
