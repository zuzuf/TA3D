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
#include "../logs/logs.h"
#include "script.interface.h"

namespace TA3D
{
    ScriptInterface::ScriptInterface() : caller(NULL), running(false), waiting(false), sleeping(false), sleep_time(0.0f)
    {
    }

    void ScriptInterface::kill()
    {
        running = false;
    }

    void ScriptInterface::stop()
    {
        waiting = true;
    }

    void ScriptInterface::resume()
    {
        waiting = false;
    }

    void ScriptInterface::pause(float time)
    {
        sleeping = true;
        sleep_time = time;
    }

    void ScriptInterface::addThread(ScriptInterface *pChild)
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

    void ScriptInterface::removeThread(ScriptInterface *pChild)
    {
        if (caller == pChild) return;
        MutexLocker mLock(pMutex);
        if (caller)
            caller->removeThread(pChild);
        else
            for(std::vector<ScriptInterface*>::iterator i = childs.begin() ; i != childs.end() ; ++i)
                if (*i == pChild)
                {
                    delete *i;
                    childs.erase(i);
                    return;
                }
    }

    void ScriptInterface::processSignal(uint32 signal)
    {
        MutexLocker mLock(pMutex);
        if (caller)
            caller->processSignal(signal);
        else
        {
            if (signal == signal_mask)
                kill();
            for(std::vector<ScriptInterface*>::iterator i = childs.begin() ; i != childs.end() ; ++i)
                if ((*i)->signal_mask == signal)
                    (*i)->kill();
        }
    }

    void ScriptInterface::setSignalMask(uint32 signal)
    {
        lock();
        signal_mask = signal;
        unlock();
    }

    uint32 ScriptInterface::getSignalMask()
    {
        return signal_mask;
    }

    void ScriptInterface::clean()
    {
        MutexLocker mLock(pMutex);
        if (caller)         // Don't go up to caller this would make complexity O(N²)!!
            return;         // and it would not be safe at all!
        else
            for(std::vector<ScriptInterface*>::iterator i = childs.begin() ; i != childs.end() ; )
                if (!(*i)->is_running())
                {
                    delete *i;
                    i = childs.erase(i);
                }
                else
                    ++i;
    }

    void ScriptInterface::dumpDebugInfo()
    {
        LOG_DEBUG(LOG_PREFIX_SCRIPT << "sorry dumpDebugInfo not implemented for this type of script");
    }

    void ScriptInterface::save_state(gzFile file)
    {
        pMutex.lock();

        gzwrite(file, &last, sizeof(last));
        gzwrite(file, &running, sizeof(running));
        gzwrite(file, &sleep_time, sizeof(sleep_time));
        gzwrite(file, &sleeping, sizeof(sleeping));
        gzwrite(file, &waiting, sizeof(waiting));
        gzwrite(file, &signal_mask, sizeof(signal_mask));
        save_thread_state(file);

        int nb_childs = childs.size();
        gzwrite(file, &nb_childs, sizeof(nb_childs));
        for(int i = 0 ; i < nb_childs ; i++)
            childs[i]->save_state(file);

        pMutex.unlock();
    }

    void ScriptInterface::restore_state(gzFile file)
    {
        pMutex.lock();

        gzread(file, &last, sizeof(last));
        gzread(file, &running, sizeof(running));
        gzread(file, &sleep_time, sizeof(sleep_time));
        gzread(file, &sleeping, sizeof(sleeping));
        gzread(file, &waiting, sizeof(waiting));
        gzread(file, &signal_mask, sizeof(signal_mask));
        restore_thread_state(file);

        int nb_childs = childs.size();
        gzread(file, &nb_childs, sizeof(nb_childs));
        for(int i = 0 ; i < nb_childs ; i++)
        {
            ScriptInterface *newThread = fork();
            newThread->restore_state(file);
        }

        pMutex.unlock();
    }
}
