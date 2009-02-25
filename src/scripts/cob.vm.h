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

#ifndef __COB_VM_H__
#define __COB_VM_H__

# include "cob.h"
# include "script.interface.h"
# include "../misc/stack.h"

namespace TA3D
{
    typedef std::vector<int>    SCRIPT_ENV;

    /*!
    ** This class represents a the COB Virtual Machine
    */
    class COB_VM : public SCRIPT_INTERFACE
    {
    protected:
        COB_SCRIPT                  *script;
        SCRIPT_ENV                  *global_env;    // Global COB environment
        Stack<int>				    cur;
        Stack<int>                  sStack;         // Script stack
        Stack<SCRIPT_ENV>           local_env;      // Local COB environment

        std::vector< short >        script_val;     // Tableau de valeurs retournées par les scripts
        uint32                      uid;            // Unit ID

    public:
        COB_VM(COB_SCRIPT *p_script);
        COB_VM();
        ~COB_VM();

        int run(float dt);              // Run the script

        //! functions used to call/run functions
        void call(const int functionID, int *parameters = NULL, int nb_params = 0);
        void call(const String &functionName, int *parameters = NULL, int nb_params = 0);
        int execute(const String &functionName, int *parameters = NULL, int nb_params = 0);

        //! functions used to create new threads sharing the same environment
        COB_VM *fork();
        COB_VM *fork(const String &functionName, int *parameters = NULL, int nb_params = 0);

        //! functions used to save/restore scripts state
        void save_state(gzFile file) {};
        void restore_state(gzFile file) {};
    private:
        void init();
        void destroy();
    };
}

#endif
