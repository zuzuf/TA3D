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

#ifndef __SCRIPT_INTERFACE_H__
#define __SCRIPT_INTERFACE_H__

#include "../threads/thread.h"
#include <zlib.h>

#define UNPACKX(xz) ((sint16)((xz)>>16))
#define UNPACKZ(xz) ((sint16)((xz)&0xFFFF))
#define PACKXZ(x,z) ((((int)(x))<<16) | (((int)(z))&0xFFFF))

namespace TA3D
{
    /*!
    ** This class is an interface for all scripts types
    */
    class SCRIPT_INTERFACE : public ObjectSync
    {
    protected:
        //! Variables to control thread execution
        int                             last;           // Last timer check
        bool                            running;

        float                           sleep_time;     // Time to wait
        bool                            sleeping;       // Is the thread paused ?
        bool                            waiting;        // Is the thread waiting for some user action ?

        uint32                          signal_mask;    // This thread will be killed as soon as it catchs this signal
        SCRIPT_INTERFACE                *caller;        // NULL if main thread
        std::vector<SCRIPT_INTERFACE*>  childs;         // Child processes, empty for childs this is to keep track of running threads
    public:
        //! stops definitely the thread
        void kill();
        void stop();
        void pause(float time);
        void resume();

        virtual int run(float dt) = 0;              // Run the script

        inline bool is_running() { return running || !childs.empty(); }
        inline bool is_waiting() { return waiting; }
        inline bool is_sleeping() { return sleeping; }

        //! functions used to call/run functions
        virtual void call(const String &functionName, int *parameters = NULL, int nb_params = 0) = 0;
        virtual int execute(const String &functionName, int *parameters = NULL, int nb_params = 0) = 0;

        //! functions used to save/restore scripts state
        virtual void save_state(gzFile file) = 0;
        virtual void restore_state(gzFile file) = 0;

        //! functions used to create new threads sharing the same environment
        virtual SCRIPT_INTERFACE *fork() = 0;
        virtual SCRIPT_INTERFACE *fork(const String &functionName, int *parameters = NULL, int nb_params = 0) = 0;

    protected:
        void addThread(SCRIPT_INTERFACE *pChild);
        void removeThread(SCRIPT_INTERFACE *pChild);
    public:
        void clean();
        void processSignal(uint32 signal);
        void setSignalMask(uint32 signal);
        uint32 getSignalMask();
    };

}

#endif
