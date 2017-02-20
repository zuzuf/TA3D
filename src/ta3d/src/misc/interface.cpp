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

#include <stdafx.h>
#include <TA3D_NameSpace.h>
#include "interface.h"
#include <logs/logs.h>



namespace TA3D
{

	IInterfaceManager::Ptr TA3D::VARS::InterfaceManager;



    void IInterface::InitInterface()
    {
        m_InterfaceID = 0;
        InterfaceManager->AddInterface(this);
    }

    void IInterface::DeleteInterface()
    {
        InterfaceManager->RemoveInterface(this);
    } 




    IInterfaceManager::IInterfaceManager()
        :pNextInterfaceID(1)
    {}

    IInterfaceManager::~IInterfaceManager()
    {}



    void IInterfaceManager::AddInterface(IInterface* i)
    {
        pMutex.lock();
        pInterfaces.push_back(i);
        i->m_InterfaceID = pNextInterfaceID;
        ++pNextInterfaceID;
        pMutex.unlock();
    }


    void IInterfaceManager::RemoveInterface(IInterface* i)
    {
        LOG_ASSERT(i);
        pMutex.lock();
        for (InterfacesList::iterator cur = pInterfaces.begin(); cur != pInterfaces.end(); ++cur)
        {
            if( (*cur)->m_InterfaceID == i->m_InterfaceID)
            {
                pInterfaces.erase(cur);
                pMutex.unlock();
                return;
            }
        }
        pMutex.unlock();
    }


	void IInterfaceManager::DispatchMsg(const uint32 mID, const QString &msg)
    {
		pMutex.lock();
		for (InterfacesList::iterator cur = pInterfaces.begin(); cur != pInterfaces.end(); ++cur)
		{
			if ((*cur)->InterfaceMsg( mID, msg ) == INTERFACE_RESULT_HANDLED)
			{
				pMutex.unlock();
				return;
			}
		}
		pMutex.unlock();
	}



}
