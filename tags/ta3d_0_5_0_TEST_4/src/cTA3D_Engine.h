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

#include "threads/cThread.h"
#include "logs/cLogger.h"

namespace TA3D
{
	class cTA3D_Engine : public ObjectSync, public cThread
	{
	private:
		TA3D::Interfaces::cLogger* m_lpcLogger;
		bool m_AllegroRunning;
		bool m_GFXModeActive;
		bool m_SignaledToStop;

	private:
		void Init();

	protected:
		int Run();
		void SignalExitThread();

	public:
		cTA3D_Engine( void );
		virtual ~cTA3D_Engine( void );

		bool GFXModeActive() const { return m_GFXModeActive; }
		bool AllegroRunning() const { return m_AllegroRunning; }
	};


} // namespace TA3D


#endif // __TA3D_ENGINE_H__
