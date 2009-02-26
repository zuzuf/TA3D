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
#include "script.interface.h"

namespace TA3D
{
    void SCRIPT_INTERFACE::kill()
    {
        running = false;
    }

    void SCRIPT_INTERFACE::stop()
    {
        waiting = true;
    }

    void SCRIPT_INTERFACE::resume()
    {
        waiting = false;
    }

    void SCRIPT_INTERFACE::pause(float time)
    {
        sleeping = true;
        sleep_time = time;
    }

    void SCRIPT_INTERFACE::addThread(SCRIPT_INTERFACE *pChild)
    {
        if (caller == pChild) return;
        MutexLocker mLock(pMutex);
        if (caller)
            caller->addThread(pChild);
        else
        {
            if (pChild == this) return;
            removeThread(pChild);
            childs.push_back(pChild);
        }
    }

    void SCRIPT_INTERFACE::removeThread(SCRIPT_INTERFACE *pChild)
    {
        if (caller == pChild) return;
        MutexLocker mLock(pMutex);
        if (caller)
            caller->removeThread(pChild);
        else
            for(std::vector<SCRIPT_INTERFACE*>::iterator i = childs.begin() ; i != childs.end() ; ++i)
                if (*i == pChild)
                {
                    delete *i;
                    childs.erase(i);
                    return;
                }
    }

    void SCRIPT_INTERFACE::processSignal(uint32 signal)
    {
        MutexLocker mLock(pMutex);
        if (caller)
            caller->processSignal(signal);
        else
        {
            if (signal == signal_mask)
                kill();
            for(std::vector<SCRIPT_INTERFACE*>::iterator i = childs.begin() ; i != childs.end() ; )
                if ((*i)->signal_mask == signal)
                {
                    delete *i;
                    i = childs.erase(i);
                }
                else
                    ++i;
        }
    }

    void SCRIPT_INTERFACE::setSignalMask(uint32 signal)
    {
        lock();
        signal_mask = signal;
        unlock();
    }

    uint32 SCRIPT_INTERFACE::getSignalMask()
    {
        return signal_mask;
    }

    void SCRIPT_INTERFACE::clean()
    {
        MutexLocker mLock(pMutex);
        if (caller)
            caller->clean();
        else
            for(std::vector<SCRIPT_INTERFACE*>::iterator i = childs.begin() ; i != childs.end() ; )
                if (!(*i)->is_running())
                {
                    delete *i;
                    i = childs.erase(i);
                }
                else
                    ++i;
    }
}
