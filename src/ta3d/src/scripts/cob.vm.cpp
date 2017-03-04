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

#include <stdafx.h>
#include <UnitEngine.h>
#include "cob.vm.h"
#include <console/console.h>

/*!
 * \brief Display the executed code if enabled
 */
#define DEBUG_USE_PRINT_CODE 0

#if DEBUG_USE_PRINT_CODE == 1
#   define DEBUG_PRINT_CODE(X)  if (print_code) LOG_DEBUG(X)
#else
#   define DEBUG_PRINT_CODE(X)
#endif


#define MAX_CODE_PER_TICK			100

namespace TA3D
{
    void CobVm::load( ScriptData *data )
    {
        CobScript *p_script = dynamic_cast<CobScript*>(data);

        destroy();

        if (p_script)
        {
            script = p_script;
			// TODO Find a better way than this hack to avoid new[] of 0 bytes...
			// May be a vector
			if (script->nbStaticVar)
	            global_env = new int[script->nbStaticVar];
			else
	            global_env = new int[1];
        }
    }

    CobVm::CobVm()
    {
        init();
    }

    CobVm::~CobVm()
    {
        destroy();
        if (caller == NULL && global_env)
			DELETE_ARRAY(global_env);
    }

    void CobVm::init()
    {
        caller = NULL;
        script = NULL;
        global_env = NULL;
        sStack.clear();
        local_env.clear();
    }

    void CobVm::destroy()
    {
        deleteThreads();
        script = NULL;
        sStack.clear();
        local_env.clear();
    }

    int CobVm::run(float dt, bool alone)  // Run the script
    {
        return run(dt, alone, NULL, 0);
    }

	int CobVm::run(float dt, bool alone, int *pParam, const uint32 nParam)  // Run the script
    {
        if (script == NULL)
        {
            running = false;
            cur.clear();
            local_env.clear();
            return -1;	// No associated script !!
        }

        if (caller == NULL && !alone)
        {
            clean();
			for (int i = (int)childs.size() - 1 ; i >= 0 ; --i)
            {
				const int sig = childs[i]->run(dt);
                if (sig > 0 || sig < -3)
                    return sig;
            }
        }

        if (!is_running())   return -1;
        if (!running)   return 0;
        if (waiting)    return -3;

        if (sleeping)
        {
            sleep_time -= dt;
            if (sleep_time <= 0.0f)
                sleeping = false;
            if (sleeping)
                return -2; 			// Keep sleeping
        }

        if (cur.empty())        // Call stack empty
        {
            running = false;
            cur.clear();
            local_env.clear();
            return -1;
        }

		sint32 script_id = (cur.top() & 0xFF);			// RÃ©cupÃ¨re l'identifiant du script en cours d'Ã©xecution et la position d'Ã©xecution
		sint32 pos = (cur.top() >> 8);
        if (script_id < 0 || script_id >= script->nb_script)
        {
            running = false;
            cur.clear();
            local_env.clear();
            return -1;		// Erreur, ce n'est pas un script repertoriÃ©
        }

        Unit *pUnit = &(units->unit[ unitID ]);

		const float divisor(I2PWR16);
		const float div = 0.5f * divisor;
        bool done = false;
        int nb_code = 0;

#if DEBUG_USE_PRINT_CODE == 1
//        bool print_code = false;
        //bool	print_code = QString::ToLower( unit_manager.unit_type[type_id]->Unitname ) == "armtship" && (QString::ToLower( script->name[script_id] ) == "transportpickup" || QString::ToLower( script->name[script_id] ) == "boomcalc" );
        bool	print_code = script->names[script_id] == "MOTIONCONTROL";
#endif

        do
        {
            //			uint32 code = script->script_code[script_id][pos];			// Lit un code
            //			pos++;
			++nb_code;
            if (nb_code >= MAX_CODE_PER_TICK) done = true;			// Pour Ã©viter que le programme ne fige Ã  cause d'un script
            //			switch(code)			// Code de l'interprÃ©teur
            switch(script->script_code[script_id][pos++])
            {
                case SCRIPT_MOVE_OBJECT:
                    {
                        DEBUG_PRINT_CODE("MOVE_OBJECT");
						const int obj = script->script_code[script_id][pos++];
						const int axis = script->script_code[script_id][pos++];
						const int v1 = sStack.pop();
						const int v2 = sStack.pop();
						pUnit->script_move_object(obj, axis, float(v1) * div, float(v2) * div);
                        break;
                    }
                case SCRIPT_WAIT_FOR_TURN:
                    {
                        DEBUG_PRINT_CODE("WAIT_FOR_TURN");
						const int obj = script->script_code[script_id][pos++];
						const int axis = script->script_code[script_id][pos++];
                        if (pUnit->script_is_turning(obj, axis))
                        {
                            pos -= 3;
                            done = true;
                        }
                        break;
                    }
                case SCRIPT_RANDOM_NUMBER:
                    {
                        DEBUG_PRINT_CODE("RANDOM_NUMBER");
						const int high = sStack.pop();
						const int low = sStack.pop();
                        sStack.push(((sint32)(Math::RandomTable() % (high - low + 1))) + low);
                        break;
                    }
                case SCRIPT_GREATER_EQUAL:
                    {
                        DEBUG_PRINT_CODE("GREATER_EQUAL");
						const int v2 = sStack.pop();
						const int v1 = sStack.pop();
                        sStack.push(v1 >= v2 ? 1 : 0);
                        break;
                    }
                case SCRIPT_GREATER:
                    {
                        DEBUG_PRINT_CODE("GREATER");
						const int v2 = sStack.pop();
						const int v1 = sStack.pop();
                        sStack.push(v1 > v2 ? 1 : 0);
                        break;
                    }
                case SCRIPT_LESS:
                    {
                        DEBUG_PRINT_CODE("LESS");
						const int v2 = sStack.pop();
						const int v1 = sStack.pop();
                        sStack.push(v1 < v2 ? 1 : 0);
                        break;
                    }
                case SCRIPT_EXPLODE:
                    {
                        DEBUG_PRINT_CODE("EXPLODE");
						const int obj = script->script_code[script_id][pos++];
						const int explosion_type = sStack.pop();
                        pUnit->script_explode(obj, explosion_type);
                        break;
                    }
                case SCRIPT_TURN_OBJECT:
                    {
                        DEBUG_PRINT_CODE("TURN_OBJECT");
						const int obj = script->script_code[script_id][pos++];
						const int axis = script->script_code[script_id][pos++];
						const int v1 = sStack.pop();
						const int v2 = sStack.pop();
						pUnit->script_turn_object(obj, axis, float(v1) * TA2DEG, float(v2) * TA2DEG);
                        break;
                    }
                case SCRIPT_WAIT_FOR_MOVE:
                    {
                        DEBUG_PRINT_CODE("WAIT_FOR_MOVE");
						const int obj = script->script_code[script_id][pos++];
						const int axis = script->script_code[script_id][pos++];
                        if (pUnit->script_is_moving(obj, axis))
                        {
                            pos -= 3;
                            done = true;
                        }
                        break;
                    }
                case SCRIPT_CREATE_LOCAL_VARIABLE:
                    {
                        DEBUG_PRINT_CODE("CREATE_LOCAL_VARIABLE");
                        local_env.top().push_back(0);
                        break;
                    }
                case SCRIPT_SUBTRACT:
                    {
                        DEBUG_PRINT_CODE("SUBSTRACT");
						const int v1 = sStack.pop();
						const int v2 = sStack.pop();
                        sStack.push(v2 - v1);
                        break;
                    }
                case SCRIPT_GET_VALUE_FROM_PORT:
                    {
                        DEBUG_PRINT_CODE("GET_VALUE_FROM_PORT:");
						const int value = sStack.pop();
                        DEBUG_PRINT_CODE(value);
                        int param[2];
                        switch(value)
                        {
                        case ATAN:
                            param[1] = sStack.pop();
                            param[0] = sStack.pop();
                            break;
                        case HYPOT:
                            param[1] = sStack.pop();
                            param[0] = sStack.pop();
                            break;
                        };
                        sStack.push( pUnit->script_get_value_from_port(value, param) );
                    }
                    break;
                case SCRIPT_LESS_EQUAL:
                    {
                        DEBUG_PRINT_CODE("LESS_EQUAL");
						const int v2 = sStack.pop();
						const int v1 = sStack.pop();
                        sStack.push(v1 <= v2 ? 1 : 0);
                        break;
                    }
                case SCRIPT_SPIN_OBJECT:
                    {
                        DEBUG_PRINT_CODE("SPIN_OBJECT");
						const int obj = script->script_code[script_id][pos++];
						const int axis = script->script_code[script_id][pos++];
						const int v1 = sStack.pop();
						const int v2 = sStack.pop();
						pUnit->script_spin_object(obj, axis, float(v1) * TA2DEG, float(v2) * TA2DEG);
                    }
                    break;
                case SCRIPT_SLEEP:
                    {
                        DEBUG_PRINT_CODE("SLEEP");
						pause( (float)sStack.pop() * 0.001f );
                        done = true;
                        break;
                    }
                case SCRIPT_MULTIPLY:
                    {
                        DEBUG_PRINT_CODE("MULTIPLY");
						const int v1 = sStack.pop();
						const int v2 = sStack.pop();
                        sStack.push(v1 * v2);
                        break;
                    }
                case SCRIPT_CALL_SCRIPT:
                    {
                        DEBUG_PRINT_CODE("CALL_SCRIPT");
						const int function_id = script->script_code[script_id][pos++];			// Lit un code
						const int num_param = script->script_code[script_id][pos++];			// Lit un code
                        cur.top() = script_id + (pos<<8);
                        cur.push( function_id );
                        local_env.push( SCRIPT_ENV() );
                        local_env.top().resize( num_param );
                        for (int i = num_param - 1 ; i >= 0 ; i--)		// Lit les paramÃ¨tres
                            local_env.top()[i] = sStack.pop();
                        pos = 0;
                        script_id = function_id;
                        break;
                    }
                case SCRIPT_SHOW_OBJECT:
                    {
                        DEBUG_PRINT_CODE("SHOW_OBJECT");
                        pUnit->script_show_object(script->script_code[script_id][pos++]);
                        break;
                    }
                case SCRIPT_EQUAL:
                    {
                        DEBUG_PRINT_CODE("EQUAL");
						const int v1 = sStack.pop();
						const int v2 = sStack.pop();
                        sStack.push(v1 == v2 ? 1 : 0);
                        break;
                    }
                case SCRIPT_NOT_EQUAL:
                    {
                        DEBUG_PRINT_CODE("NOT_EQUAL");
						const int v1 = sStack.pop();
						const int v2 = sStack.pop();
                        sStack.push(v1 != v2 ? 1 : 0);
                        break;
                    }
                case SCRIPT_IF:
                    {
                        DEBUG_PRINT_CODE("IF");
                        if (sStack.pop() != 0)
                            pos++;
                        else
                        {
							const int target_offset = script->script_code[script_id][pos];        // Lit un code
                            pos = target_offset - script->dec_offset[script_id];            // DÃ©place l'Ã©xecution
                        }
                        break;
                    }
                case SCRIPT_HIDE_OBJECT:
                    {
                        DEBUG_PRINT_CODE("HIDE_OBJECT");
                        pUnit->script_hide_object(script->script_code[script_id][pos++]);
                        break;
                    }
                case SCRIPT_SIGNAL:
                    {
                        DEBUG_PRINT_CODE("SIGNAL");
                        cur.top() = script_id + (pos<<8);	    // Sauvegarde la position
                        processSignal(sStack.pop());                 // Tue tout les processus utilisant ce signal
                        if (!running)
                            return 0;
                        break;
                    }
                case SCRIPT_DONT_CACHE:
                    {
                        DEBUG_PRINT_CODE("DONT_CACHE");
                        pUnit->script_dont_cache(script->script_code[script_id][pos++]);
                        break;
                    }
                case SCRIPT_SET_SIGNAL_MASK:
                    {
                        DEBUG_PRINT_CODE("SET_SIGNAL_MASK");
                        setSignalMask( sStack.pop() );
                        break;
                    }
                case SCRIPT_NOT:
                    {
                        DEBUG_PRINT_CODE("NOT");
                        sStack.push(!sStack.pop());
                        break;
                    }
                case SCRIPT_DONT_SHADE:
                    {
                        DEBUG_PRINT_CODE("DONT_SHADE");
                        pUnit->script_dont_shade(script->script_code[script_id][pos++]);
                        break;
                    }
                case SCRIPT_EMIT_SFX:
                    {
                        DEBUG_PRINT_CODE("EMIT_SFX:");
						const int smoke_type = sStack.pop();
						const int from_piece = script->script_code[script_id][pos++];
                        DEBUG_PRINT_CODE("smoke_type " << smoke_type << " from " << from_piece);
                        pUnit->script_emit_sfx( smoke_type, from_piece );
                    }
                    break;
                case SCRIPT_PUSH_CONST:
                    {
                        DEBUG_PRINT_CODE("PUSH_CONST (" << script->script_code[script_id][pos] << ")");
                        sStack.push(script->script_code[script_id][pos]);
                        ++pos;
                        break;
                    }
                case SCRIPT_PUSH_VAR:
                    {
                        DEBUG_PRINT_CODE("PUSH_VAR (" << script->script_code[script_id][pos] << ") = "
                                         << local_env.top()[script->script_code[script_id][pos]]);
                        sStack.push(local_env.top()[script->script_code[script_id][pos]]);
                        ++pos;
                        break;
                    }
                case SCRIPT_SET_VAR:
                    {
                        DEBUG_PRINT_CODE("SET_VAR (" << script->script_code[script_id][pos] << ")");
						const int v_id = script->script_code[script_id][pos];
						if ((int)local_env.top().size() <= v_id)
							local_env.top().resize(v_id + 1);
                        local_env.top()[v_id] = sStack.pop();
                        ++pos;
                        break;
                    }
                case SCRIPT_PUSH_STATIC_VAR:
                    {
                        DEBUG_PRINT_CODE("PUSH_STATIC_VAR");
                        sStack.push(global_env[script->script_code[script_id][pos]]);
                        ++pos;
                        break;
                    }
                case SCRIPT_SET_STATIC_VAR:
                    {
                        DEBUG_PRINT_CODE("SET_STATIC_VAR");
                        global_env[script->script_code[script_id][pos]] = sStack.pop();
                        ++pos;
                        break;
                    }
                case SCRIPT_OR:
                    {
                        DEBUG_PRINT_CODE("OR");
						const int v1 = sStack.pop(), v2 = sStack.pop();
                        sStack.push(v1 | v2);
                        break;
                    }
                case SCRIPT_START_SCRIPT:				// TransfÃ¨re l'Ã©xecution Ã  un autre script
                    {
                        DEBUG_PRINT_CODE("START_SCRIPT");
						const int function_id = script->script_code[script_id][pos++];			// Lit un code
						const int num_param = script->script_code[script_id][pos++];			// Lit un code
                        CobVm *p_cob = fork();
                        if (p_cob)
                        {
                            p_cob->call(function_id, NULL, 0);
                            p_cob->local_env.top().resize( num_param );
                            for (int i = num_param - 1 ; i >= 0 ; i--)		// Lit les paramÃ¨tres
                                p_cob->local_env.top()[i] = sStack.pop();
                            p_cob->setSignalMask( signal_mask );
                        }
                        else
                        {
                            for (int i = 0 ; i < num_param ; ++i)		// EnlÃ¨ve les paramÃ¨tres de la pile
                                sStack.pop();
                        }
                        break;
                    }
                case SCRIPT_RETURN:		// Retourne au script prÃ©cÃ©dent
                    {
                        DEBUG_PRINT_CODE("RETURN");
                        cur.pop();

                        if (cur.empty() && pParam)              // Get back parameter values
							for (uint32 i = 0 ; i < nParam ; ++i)
                                pParam[i] = local_env.top().size() > i ? local_env.top()[i] : 0;

                        local_env.pop();
						const int value = sStack.pop();		// EnlÃ¨ve la valeur retournÃ©e / Pops the return value
                        setReturnValue( script->names[script_id], value );      // Set the return value
                        if (!cur.empty())
                        {
                            script_id = (cur.top() & 0xFF);			// RÃ©cupÃ¨re l'identifiant du script en cours d'Ã©xecution et la position d'Ã©xecution
                            pos = (cur.top() >> 8);
                        }
                        else
                            done = true;
                        break;
                    }
                case SCRIPT_JUMP:						// Commande de saut
                    {
                        DEBUG_PRINT_CODE("JUMP");
						const int target_offset = script->script_code[script_id][pos];        // Lit un code
                        pos = target_offset - script->dec_offset[script_id];            // DÃ©place l'Ã©xecution
                        break;
                    }
                case SCRIPT_ADD:
                    {
                        DEBUG_PRINT_CODE("ADD");
						const int v1 = sStack.pop();
						const int v2 = sStack.pop();
                        sStack.push(v1 + v2);
                        break;
                    }
                case SCRIPT_STOP_SPIN:
                    {
                        DEBUG_PRINT_CODE("STOP_SPIN");
						const int obj = script->script_code[script_id][pos++];
						const int axis = script->script_code[script_id][pos++];
						const int v = sStack.pop();
						pUnit->script_stop_spin(obj, axis, float(v) * TA2DEG);
                        break;
                    }
                case SCRIPT_DIVIDE:
                    {
                        DEBUG_PRINT_CODE("DIVIDE");
						const int v1 = sStack.pop();
						const int v2 = sStack.pop();
                        sStack.push(v2 / v1);
                        break;
                    }
                case SCRIPT_MOVE_PIECE_NOW:
                    {
                        DEBUG_PRINT_CODE("MOVE_PIECE_NOW");
						const int obj = script->script_code[script_id][pos++];
						const int axis = script->script_code[script_id][pos++];
						const int pos = sStack.pop();
						pUnit->script_move_piece_now(obj, axis, float(pos) * div);
                        break;
                    }
                case SCRIPT_TURN_PIECE_NOW:
                    {
                        DEBUG_PRINT_CODE("TURN_PIECE_NOW");
						const int obj = script->script_code[script_id][pos++];
						const int axis = script->script_code[script_id][pos++];
						const int v = sStack.pop();
						pUnit->script_turn_piece_now(obj, axis, float(v) * TA2DEG);
                        break;
                    }
                case SCRIPT_CACHE:
                    DEBUG_PRINT_CODE("CACHE");
                    pUnit->script_cache(script->script_code[script_id][pos++]);
                    break;	//added
                case SCRIPT_COMPARE_AND:
                    {
                        DEBUG_PRINT_CODE("COMPARE_AND");
						const int v1 = sStack.pop();
						const int v2 = sStack.pop();
                        sStack.push(v1 && v2);
                        break;
                    }
                case SCRIPT_COMPARE_OR:
                    {
                        DEBUG_PRINT_CODE("COMPARE_OR");
						const int v1 = sStack.pop();
						const int v2 = sStack.pop();
                        sStack.push(v1 || v2);
                        break;
                    }
                case SCRIPT_CALL_FUNCTION:
                    {
                        DEBUG_PRINT_CODE("CALL_FUNCTION");
						const int function_id = script->script_code[script_id][pos++];			// Lit un code
						const int num_param = script->script_code[script_id][pos++];			// Lit un code
                        cur.top() = script_id + (pos << 8);
                        local_env.push( SCRIPT_ENV() );
                        local_env.top().resize(num_param);
                        for (int i = num_param - 1 ; i >= 0 ; i--)		// Lit les paramÃ¨tres
                            local_env.top()[i] = sStack.pop();
                        done = true;
                        pos = 0;
                        script_id = function_id;
                        cur.push( script_id + (pos << 8) );
                        break;
                    }
                case SCRIPT_GET:
                    {
                        DEBUG_PRINT_CODE("GET *");
                        sStack.pop();
                        sStack.pop();
						const int v2 = sStack.pop();
						const int v1 = sStack.pop();
						const int val = sStack.pop();
                        sStack.push( pUnit->script_get(val, v1, v2) );
                        break;	//added
                    }
                case SCRIPT_SET_VALUE:
                    {
                        DEBUG_PRINT_CODE("SET_VALUE *:");
						const int v1 = sStack.pop();
						const int v2 = sStack.pop();
                        DEBUG_PRINT_CODE(v1 << " " << v2);
                        pUnit->script_set_value( v2, v1 );
                    }
                    break;	//added
                case SCRIPT_ATTACH_UNIT:
                    {
                        DEBUG_PRINT_CODE("ATTACH_UNIT");
						/*int v3 =*/ sStack.pop();
						const int v2 = sStack.pop();
						const int v1 = sStack.pop();
						pUnit->script_attach_unit(v1, v2);
                        break;	//added
                    }
                case SCRIPT_DROP_UNIT:
                    {
                        DEBUG_PRINT_CODE("DROP_UNIT *");
						const int v1 = sStack.pop();
                        DEBUG_PRINT_CODE("Dropping " << v1);
                        pUnit->script_drop_unit(v1);
                        break;	//added
                    }
                default:
                    LOG_ERROR("UNKNOWN " << script->script_code[script_id][--pos] << ", Stopping script");
                    {
                        cur.pop();

                        if (cur.empty() && !local_env.empty() && pParam)              // Get back parameter values
							for (uint32 i = 0 ; i < nParam ; ++i)
                                pParam[i] = local_env.top().size() > i ? local_env.top()[i] : 0;

                        local_env.pop();
                    }
                    if (!cur.empty())
                    {
                        script_id = (cur.top() & 0xFF);			// RÃ©cupÃ¨re l'identifiant du script en cours d'Ã©xecution et la position d'Ã©xecution
                        pos = (cur.top() >> 8);
                    }
                    else
                        running = false;
                    done = true;
            };
        } while(!done);

        if (!cur.empty())
            cur.top() = script_id + (pos << 8);
        return 0;
    }

    void CobVm::call(const int functionID, int *parameters, int nb_params)
    {
        if (!script || functionID < 0 || functionID >= script->nb_script || !cur.empty())
            return;

        cur.push( functionID );
        local_env.push( SCRIPT_ENV() );
        running = true;

        if (nb_params > 0 && parameters != NULL)
        {
            local_env.top().resize(nb_params);
            for (int i = 0 ; i < nb_params ; i++)
                local_env.top()[i] = parameters[i];
        }
    }

    CobVm *CobVm::fork()
    {
        if (!running && caller == NULL)
        {
            cur.clear();
            local_env.clear();
            waiting = false;
            sleeping = false;
            sleep_time = 0.0f;
            signal_mask = 0;
            return this;
        }

        CobVm *newThread = static_cast<CobVm*>(getFreeThread());
        if (newThread)
        {
            newThread->sStack.clear();
            newThread->local_env.clear();
            newThread->cur.clear();
            newThread->signal_mask = 0;
            newThread->running = false;
            newThread->waiting = false;
            newThread->sleeping = false;
            newThread->sleep_time = 0.0f;
            addThread(newThread);

            return newThread;
        }

        newThread = new CobVm();

        newThread->script = script;
        newThread->running = false;
        newThread->waiting = false;
        newThread->sleeping = false;
        newThread->sleep_time = 0.0f;
        newThread->caller = (caller != NULL) ? caller : this;
        newThread->global_env = global_env;
        newThread->setUnitID( unitID );
        addThread(newThread);

        return newThread;
    }

    CobVm *CobVm::fork(const QString &functionName, int *parameters, int nb_params)
    {
        CobVm *newThread = fork();
        if (newThread)
            newThread->call(functionName, parameters, nb_params);
        return newThread;
    }

    void CobVm::call(const QString &functionName, int *parameters, int nb_params)
    {
        call( script->findFromName( functionName ), parameters, nb_params );
    }

    int CobVm::execute(const QString &functionName, int *parameters, int nb_params)
    {
        int params[1] = {-1};
        if (parameters == NULL || nb_params == 0)
        {
            parameters = params;
            nb_params = 1;
        }
        CobVm *cob_thread = fork( functionName, parameters, nb_params);
        if (cob_thread)
        {
            int res = -1;
            while( cob_thread->running )
                res = cob_thread->run(0.0f, true, parameters, nb_params);
            cob_thread->kill();
            return parameters[nb_params-1];
        }
        return 0;
    }

    void CobVm::setUnitID(uint32 ID)
    {
        unitID = ID;
    }

    int CobVm::getNbPieces()
    {
        if (script == NULL)
            return 0;
        return script->nb_piece;
    }

    void CobVm::dumpDebugInfo()
    {
        if (caller)
        {
            caller->dumpDebugInfo();
            return;
        }
        LOG_DEBUG(LOG_PREFIX_SCRIPT << "CobVm::dumpDebufInfo :");
        LOG_DEBUG(LOG_PREFIX_SCRIPT << childs.size() << " child threads");
		Console::Instance()->addEntry("CobVm::dumpDebugInfo :");
        Console::Instance()->addEntry(QString("%1 child threads").arg(int(childs.size())));
		if (running)
		{
            logs.debug() << LOG_PREFIX_SCRIPT << "main thread running : " << (script->names[cur.top() & 0xFF]);
            Console::Instance()->addEntry(QString("main thread running : %1").arg((script->names[cur.top() & 0xFF])));
		}
		for (uint32 i = 0 ; i < childs.size() ; ++i)
            if (childs[i]->is_running())
            {
                QString state;
                if (childs[i]->is_waiting())
                    state += " (waiting)";
                if (childs[i]->is_sleeping())
                    state += QString(" (sleeping = %1)").arg(dynamic_cast<CobVm*>(childs[i])->sleep_time);
                LOG_DEBUG(LOG_PREFIX_SCRIPT << "child thread " << i << " running : " << script->names[(dynamic_cast<CobVm*>(childs[i]))->cur.top() & 0xFF] << state);
                Console::Instance()->addEntry(QString("child thread %1 running : %2").arg(i).arg(script->names[(dynamic_cast<CobVm*>(childs[i]))->cur.top() & 0xFF]) + state);
			}
    }


    void CobVm::save_thread_state(gzFile file)
    {
        if (caller == NULL)
        {
            gzputc(file, 1);
			const int t = script->nbStaticVar;
			gzwrite(file, const_cast<void*>((const void*)&t), (int)sizeof(t));
			gzwrite(file, global_env, t * (int)sizeof(int));
        }
        else
            gzputc(file, 0);

		int t = (int)cur.size();
        gzwrite(file, &t, sizeof(t));
        for (int i = 0 ; i < t ; i++)
            gzwrite(file, &(cur[i]), sizeof(int));

		t = (int)sStack.size();
        gzwrite(file, &t, sizeof(t));
        for (int i = 0 ; i < t ; i++)
            gzwrite(file, &(sStack[i]), sizeof(int));

		t = (int)local_env.size();
        gzwrite(file, &t, sizeof(t));
        for (int i = 0 ; i < t ; i++)
        {
			const int f = (int)local_env[i].size();
			gzwrite(file, const_cast<void*>((const void*)&f), sizeof(f));
            for (int e = 0 ; e < f ; e++)
                gzwrite(file, &(local_env[i][e]), sizeof(int));
        }
    }


    void CobVm::restore_thread_state(gzFile file)
    {
        if (gzgetc(file))
        {
            int t;
            gzread(file, &t, sizeof(t));
			DELETE_ARRAY(global_env);
            global_env = new int[t];
			gzread(file, global_env, t * (int)sizeof(int));
        }

        int t;
        gzread(file, &t, sizeof(t));
        cur.clear();
        for (int i = 0 ; i < t ; i++)
        {
            int v;
            gzread(file, &v, sizeof(int));
            cur.push(v);
        }

        gzread(file, &t, sizeof(t));
        sStack.clear();
        for (int i = 0 ; i < t ; i++)
        {
            int v;
            gzread(file, &v, sizeof(int));
            sStack.push(v);
        }

        gzread(file, &t, sizeof(t));
        local_env.clear();
        for (int i = 0 ; i < t ; i++)
        {
            int f;
            gzread(file, &f, sizeof(t));
            local_env.push( SCRIPT_ENV() );
            local_env.top().resize(f);
            for (int e = 0 ; e < f ; e++)
                gzread(file, &(local_env.top()[e]), sizeof(int));
        }
    }



} // namespace TA3D


