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

#ifndef __ScriptInterface_H__
#define __ScriptInterface_H__

# include <misc/string.h>
# include <threads/thread.h>
# include "script.data.h"
# include <zlib.h>
#include <deque>

# define UNPACKX(xz) ((sint16)((xz)>>16))
# define UNPACKZ(xz) ((sint16)((xz)&0xFFFF))
# define PACKXZ(x,z) ((((int)(x))<<16) | (((int)(z))&0xFFFF))

namespace TA3D
{
    /*!
    ** This class is an interface for all scripts types
    */
    class ScriptInterface : public virtual zuzuf::ref_count
    {
    protected:
        //! Variables to control thread execution
        int                             last;           // Last timer check
        bool                            running;

        float                           sleep_time;     // Time to wait
        bool                            sleeping;       // Is the thread paused ?
        bool                            waiting;        // Is the thread waiting for some user action ?

        uint32                          signal_mask;    // This thread will be killed as soon as it catchs this signal
        ScriptInterface                 *caller;        // NULL if main thread
        std::vector<ScriptInterface*>   childs;         // Child processes, empty for childs this is to keep track of running threads
		std::vector<ScriptInterface*>    freeThreads;    // Old childs processes that are not used, we keep them to prevent allocating/freeing things uselessly
    public:
        ScriptInterface();
        virtual ~ScriptInterface() {    deleteThreads();  }

        virtual void load( ScriptData *data ) = 0;

        //! stops definitely the thread
        void kill();
        void stop();
        void pause(float time);
        void resume();

        virtual int run(float dt, bool alone = false) = 0;      // Run the script

        inline bool is_self_running() { return running; }
        inline bool is_running() { return running || !childs.empty(); }
        inline bool is_waiting() { return waiting; }
        inline bool is_sleeping() { return sleeping; }

		inline int nb_threads() { return (running ? 1 : 0) + int(childs.size()); }

        //! functions used to call/run functions
        virtual void call(const QString &functionName, int *parameters = NULL, int nb_params = 0) = 0;
        virtual int execute(const QString &functionName, int *parameters = NULL, int nb_params = 0) = 0;

        //! functions used to save/restore scripts state
        virtual void save_thread_state(gzFile file) = 0;
        virtual void restore_thread_state(gzFile file) = 0;
        void save_state(gzFile file);
        void restore_state(gzFile file);

        //! functions used to create new threads sharing the same environment
        virtual ScriptInterface *fork() = 0;
        virtual ScriptInterface *fork(const QString &functionName, int *parameters = NULL, int nb_params = 0) = 0;

        ScriptInterface *getFreeThread();

        //! debug functions
        virtual void dumpDebugInfo();
    protected:
        void addThread(ScriptInterface *pChild);
        void removeThread(ScriptInterface *pChild);
        void deleteThreads();
    public:
        void clean();
        void processSignal(uint32 signal);
        void setSignalMask(uint32 signal);
        uint32 getSignalMask();
    };

}

#endif
