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

#ifndef __CobVm_H__
#define __CobVm_H__


# include <stdafx.h>
# include <misc/string.h>
# include "cob.h"
# include "unit.script.interface.h"
# include <misc/stack.hxx>

# define  SCRIPT_MOVE_OBJECT              0x10001000
# define  SCRIPT_WAIT_FOR_TURN            0x10011000
# define  SCRIPT_RANDOM_NUMBER            0x10041000
# define  SCRIPT_LESS                     0x10051000
# define  SCRIPT_GREATER_EQUAL            0x10054000
# define  SCRIPT_GREATER                  0x10053000
# define  SCRIPT_START_SCRIPT             0x10061000
# define  SCRIPT_EXPLODE                  0x10071000
# define  SCRIPT_TURN_OBJECT              0x10002000
# define  SCRIPT_WAIT_FOR_MOVE            0x10012000
# define  SCRIPT_CREATE_LOCAL_VARIABLE    0x10022000
# define  SCRIPT_SUBTRACT                 0x10032000
# define  SCRIPT_GET_VALUE_FROM_PORT      0x10042000
# define  SCRIPT_LESS_EQUAL               0x10052000
# define  SCRIPT_SPIN_OBJECT              0x10003000
# define  SCRIPT_SLEEP                    0x10013000
# define  SCRIPT_MULTIPLY                 0x10033000
# define  SCRIPT_CALL_SCRIPT              0x10063000
# define  SCRIPT_JUMP                     0x10064000
# define  SCRIPT_SHOW_OBJECT              0x10005000
# define  SCRIPT_EQUAL                    0x10055000
# define  SCRIPT_RETURN                   0x10065000
# define  SCRIPT_NOT_EQUAL                0x10056000
# define  SCRIPT_IF                       0x10066000
# define  SCRIPT_HIDE_OBJECT              0x10006000
# define  SCRIPT_SIGNAL                   0x10067000
# define  SCRIPT_DONT_CACHE               0x10008000
# define  SCRIPT_SET_SIGNAL_MASK          0x10068000
# define  SCRIPT_NOT                      0x1005A000
# define  SCRIPT_DONT_SHADE               0x1000E000
# define  SCRIPT_EMIT_SFX                 0x1000F000
# define  SCRIPT_PUSH_CONST               0x10021001
# define  SCRIPT_PUSH_VAR                 0x10021002
# define  SCRIPT_SET_VAR                  0x10023002
# define  SCRIPT_PUSH_STATIC_VAR          0x10021004
# define  SCRIPT_SET_STATIC_VAR           0x10023004
# define  SCRIPT_OR                       0x10036000
# define  SCRIPT_ADD                      0x10031000  //added
# define  SCRIPT_STOP_SPIN                0x10004000  //added
# define  SCRIPT_DIVIDE                   0x10034000  //added
# define  SCRIPT_MOVE_PIECE_NOW           0x1000B000  //added
# define  SCRIPT_TURN_PIECE_NOW           0x1000C000  //added
# define  SCRIPT_CACHE                    0x10007000  //added
# define  SCRIPT_COMPARE_AND              0x10057000  //added
# define  SCRIPT_COMPARE_OR               0x10058000  //added
# define  SCRIPT_CALL_FUNCTION            0x10062000  //added
# define  SCRIPT_GET                      0x10043000  //added
# define  SCRIPT_SET_VALUE                0x10082000  //added
# define  SCRIPT_ATTACH_UNIT              0x10083000  //added
# define  SCRIPT_DROP_UNIT                0x10084000  //added

namespace TA3D
{
    typedef std::vector<int>    SCRIPT_ENV;

    /*!
    ** This class represents a the COB Virtual Machine
    */
    class CobVm : public UnitScriptInterface
    {
    protected:
        CobScript                   *script;
        int                         *global_env;    // Global COB environment
        Stack<int>				    cur;
        Stack<int>                  sStack;         // Script stack
        Stack<SCRIPT_ENV>           local_env;      // Local COB environment

    public:
        CobVm();
        virtual ~CobVm();

        /*virtual*/ void load( ScriptData *data );

        /*virtual*/ int run(float dt, bool alone = false);  // Run the script
		int run(float dt, bool alone, int *pParam, const uint32 nParam);  // Run the script

        //! functions used to call/run functions
        void call(const int functionID, int *parameters = NULL, int nb_params = 0);
        /*virtual*/ void call(const QString &functionName, int *parameters = NULL, int nb_params = 0);
        /*virtual*/ int execute(const QString &functionName, int *parameters = NULL, int nb_params = 0);

        //! functions used to create new threads sharing the same environment
        /*virtual*/ CobVm *fork();
        /*virtual*/ CobVm *fork(const QString &functionName, int *parameters = NULL, int nb_params = 0);

        //! functions used to save/restore scripts state
        /*virtual*/ void save_thread_state(gzFile file);
        /*virtual*/ void restore_thread_state(gzFile file);

        /*virtual*/ void setUnitID(uint32 ID);
        /*virtual*/ int getNbPieces();

        //! debug functions
        /*virtual*/ void dumpDebugInfo();
    private:
        void init();
        void destroy();
    };
}

#endif
