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

#include "cCriticalSection.h"

#pragma once

namespace TA3D
{
#define I_Msg( x, xx,xxx,xxxx ) InterfaceManager->DispatchMsg( x,xx,xxx,xxxx )
#define I_sMsg( x ) InterfaceManager->DispatchMsg( x )

	class cInterfaceManager;
	class cInterface;

	enum TA3D_INTERFACE_MESSAGES
	{
		TA3D_IM_DEBUG_MSG,
		TA3D_IM_GFX_MSG,
		TA3D_IM_GUI_MSG
	};

	enum INTERFACE_RESULT
	{
		INTERFACE_RESULT_HANDLED =0,
		INTERFACE_RESULT_CONTINUE =1
	//	INTERFACE_RESULT_IGNORE_THIS_MSG =2
	};

	typedef struct cInterfaceMessage
	{
		uint32 MsgID;

		void *lpParm1;
		void *lpParm2;
		void *lpParm3;
		cInterfaceMessage( uint32 mid, void *a, void *b, void *c )
		{
			MsgID = mid;
			lpParm1 = a;
			lpParm2 = b;
			lpParm3 = c;
		}
	} cIMsg, *lpcImsg;

	class cInterface
	{
		friend class cInterfaceManager;

	public:
		virtual ~cInterface() {}

	private:
		uint32 m_InterfaceID;

		virtual uint32 InterfaceMsg( const lpcImsg msg ) = 0;

	protected:
		void InitInterface();
		void DeleteInterface();
	};


	class cInterfaceManager : protected cCriticalSection
	{
		friend class cInterface;

	private:
		std::vector< cInterface * >      m_Interfaces;
		uint32                     m_NextInterfaceID;

	private:
		void AddInterface( cInterface *obj )
		{
			EnterCS();
				m_Interfaces.push_back( obj );

				obj->m_InterfaceID = m_NextInterfaceID;
				m_NextInterfaceID++;
			LeaveCS();
		}

		void RemoveInterface( cInterface *obj )
		{
			EnterCS();

				std::vector< cInterface * >::iterator cur;

				for( cur = m_Interfaces.begin(); cur < m_Interfaces.end(); cur++ )
				{
					if( (*cur)->m_InterfaceID == obj->m_InterfaceID )
					{
						m_Interfaces.erase( cur );
						break;
					}
				}
			LeaveCS();
		}

		void DispatchMsg( const lpcImsg msg )
		{
			EnterCS();
				std::vector< cInterface * >::iterator cur;

				for( cur = m_Interfaces.begin(); cur < m_Interfaces.end(); cur++ )
				{
					if( (*cur)->InterfaceMsg( msg ) == INTERFACE_RESULT_HANDLED )
						break;
				}
			LeaveCS();
		}

	public:
		void DispatchMsg( uint32 mID, void *a, void *b, void *c )
		{
			EnterCS();
				cInterfaceMessage *cimsg = new cInterfaceMessage( mID, a, b, c );

				DispatchMsg( cimsg );

				delete cimsg;

			LeaveCS();
		}

		cInterfaceManager()
		{
			CreateCS();
			m_NextInterfaceID = 1;
		}

		~cInterfaceManager()
		{
			EnterCS();
				m_Interfaces.clear();
			LeaveCS();

			DeleteCS();
		}
	};
}
