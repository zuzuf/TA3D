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
#include "../UnitEngine.h"
#include "cob.vm.h"

/*!
 * \brief Display the executed code if enabled
 */
#define DEBUG_USE_PRINT_CODE 0

#if DEBUG_USE_PRINT_CODE == 1
#   define DEBUG_PRINT_CODE(X)  if (print_code) LOG_DEBUG(X)
#else
#   define DEBUG_PRINT_CODE(X)
#endif


#define SQUARE(X)  ((X)*(X))

namespace TA3D
{
    void SCRIPT_ENV_STACK::init()
    {
        for(uint8 i = 0 ; i < 15 ; i++)
            var[i] = 0;
        cur = 0;
        signal_mask = 0;
        next = NULL;
    }

    SCRIPT_ENV_STACK::SCRIPT_ENV_STACK()
    {
        init();
    }


    void SCRIPT_ENV::init()
    {
        while(!sStack.empty())
            sStack.pop();
        env = NULL;
        wait = 0.0f;
        running = false;
    }

    SCRIPT_ENV::SCRIPT_ENV()
    {
        init();
    }

    void SCRIPT_ENV::destroy()
    {
        while(!sStack.empty())
            sStack.pop();
        while(env)
        {
            SCRIPT_ENV_STACK *next = env->next;
            delete env;
            env = next;
        }
        init();
    }

    SCRIPT_ENV::~SCRIPT_ENV()
    {
        //		destroy();
    }

    void SCRIPT_ENV::push(int v)
    {
        sStack.push(v);
    }

    int SCRIPT_ENV::pop()
    {
        if (sStack.empty())// Si la pile est vide, renvoie 0 et un message pour le débuggage
        {
            # ifdef DEBUG_MODE
            LOG_WARNING("COB VM: stack is empty !");
            # endif
            return 0;
        }
        int v = sStack.top();
        sStack.pop();
        return v;
    }


    COB_VM::COB_VM(COB_SCRIPT *p_script)
    {
        init();
        script = p_script;
    }

    COB_VM::COB_VM()
    {
        init();
    }

    COB_VM::~COB_VM()
    {
        destroy();
    }

    void COB_VM::init()
    {
        script = NULL;
        s_var.clear();
        nb_running = 0;
        script_env.clear();
        script_val.clear();
    }

    void COB_VM::destroy()
    {
        script = NULL;
        s_var.clear();
        nb_running = 0;
        script_env.clear();
        script_val.clear();
    }

    int COB_VM::run_thread(const float &dt, const int &id, int max_code)
    {
        if (id >= (int)script_env.size() && !script_env[id].running)
            return 2;
        if (script_env[id].wait > 0.0f)
        {
            script_env[id].wait -= dt;
            return 1;
        }
        if (script == NULL || script_env[id].env == NULL)
        {
            script_env[id].running = false;
            return 2;	// S'il n'y a pas de script associé on quitte la fonction
        }
        sint16 script_id = (script_env[id].env->cur & 0xFF);			// Récupère l'identifiant du script en cours d'éxecution et la position d'éxecution
        sint16 pos = (script_env[id].env->cur >> 8);

        if (script_id < 0 || script_id >= script->nb_script)
        {
            script_env[id].running = false;
            return 2;		// Erreur, ce n'est pas un script repertorié
        }

        UNIT *pUnit = &(units.unit[ uid ]);

        float divisor(I2PWR16);
        float div = 0.5f * divisor;
        bool done = false;
        int nb_code = 0;

#if DEBUG_USE_PRINT_CODE == 1
        bool print_code = false;
        //bool	print_code = String::ToLower( unit_manager.unit_type[type_id]->Unitname ) == "armtship" && (String::ToLower( script->name[script_id] ) == "transportpickup" || String::ToLower( script->name[script_id] ) == "boomcalc" );
#endif

        do
        {
            //			uint32 code = script->script_code[script_id][pos];			// Lit un code
            //			pos++;
            nb_code++;
            if (nb_code >= max_code) done = true;			// Pour éviter que le programme ne fige à cause d'un script
            //			switch(code)			// Code de l'interpréteur
            switch(script->script_code[script_id][pos++])
            {
                case SCRIPT_MOVE_OBJECT:
                    {
                        DEBUG_PRINT_CODE("MOVE_OBJECT");
                        int obj = script->script_code[script_id][pos++];
                        int axis = script->script_code[script_id][pos++];
                        int v1 = script_env[id].pop();
                        int v2 = script_env[id].pop();
                        pUnit->script_move_object(obj, axis, v1 * div, v2 * div);
                        break;
                    }
                case SCRIPT_WAIT_FOR_TURN:
                    {
                        DEBUG_PRINT_CODE("WAIT_FOR_TURN");
                        int obj = script->script_code[script_id][pos++];
                        int axis = script->script_code[script_id][pos++];
                        if (pUnit->script_is_turning(obj, axis))
                            pos -= 3;
                        done = true;
                        break;
                    }
                case SCRIPT_RANDOM_NUMBER:
                    {
                        DEBUG_PRINT_CODE("RANDOM_NUMBER");
                        int high = script_env[id].pop();
                        int low = script_env[id].pop();
                        script_env[id].push(((sint32)(Math::RandFromTable() % (high - low + 1))) + low);
                        break;
                    }
                case SCRIPT_GREATER_EQUAL:
                    {
                        DEBUG_PRINT_CODE("GREATER_EQUAL");
                        int v2 = script_env[id].pop();
                        int v1 = script_env[id].pop();
                        script_env[id].push(v1 >= v2 ? 1 : 0);
                        break;
                    }
                case SCRIPT_GREATER:
                    {
                        DEBUG_PRINT_CODE("GREATER");
                        int v2 = script_env[id].pop();
                        int v1 = script_env[id].pop();
                        script_env[id].push(v1 > v2 ? 1 : 0);
                        break;
                    }
                case SCRIPT_LESS:
                    {
                        DEBUG_PRINT_CODE("LESS");
                        int v2 = script_env[id].pop();
                        int v1 = script_env[id].pop();
                        script_env[id].push(v1 < v2 ? 1 : 0);
                        break;
                    }
                case SCRIPT_EXPLODE:
                    {
                        DEBUG_PRINT_CODE("EXPLODE");
                        int obj = script->script_code[script_id][pos++];
                        int explosion_type = script_env[id].pop();
                        pUnit->script_explode(obj, explosion_type);
                        break;
                    }
                case SCRIPT_TURN_OBJECT:
                    {
                        DEBUG_PRINT_CODE("TURN_OBJECT");
                        int obj = script->script_code[script_id][pos++];
                        int axis = script->script_code[script_id][pos++];
                        int v1 = script_env[id].pop();
                        int v2 = script_env[id].pop();
                        pUnit->script_turn_object(obj, axis, v1 * TA2DEG, v2 * TA2DEG);
                        break;
                    }
                case SCRIPT_WAIT_FOR_MOVE:
                    {
                        DEBUG_PRINT_CODE("WAIT_FOR_MOVE");
                        int obj = script->script_code[script_id][pos++];
                        int axis = script->script_code[script_id][pos++];
                        if (pUnit->script_is_moving(obj, axis))
                            pos -= 3;
                        done = true;
                        break;
                    }
                case SCRIPT_CREATE_LOCAL_VARIABLE:
                    {
                        DEBUG_PRINT_CODE("CREATE_LOCAL_VARIABLE");
                        break;
                    }
                case SCRIPT_SUBTRACT:
                    {
                        DEBUG_PRINT_CODE("SUBSTRACT");
                        int v1 = script_env[id].pop();
                        int v2 = script_env[id].pop();
                        script_env[id].push(v2 - v1);
                        break;
                    }
                case SCRIPT_GET_VALUE_FROM_PORT:
                    {
                        DEBUG_PRINT_CODE("GET_VALUE_FROM_PORT:");
                        int value = script_env[id].pop();
                        DEBUG_PRINT_CODE(value);
                        // TODO : clean this thing
                        int param[2];
                        switch(value)
                        {
                        case ATAN:
                            param[1] = script_env[id].pop();
                            param[0] = script_env[id].pop();
                            break;
                        case HYPOT:
                            param[1] = script_env[id].pop();
                            param[0] = script_env[id].pop();
                            break;
                        };
                        script_env[id].push( pUnit->script_get_value_from_port(value, param) );
                    }
                    break;
                case SCRIPT_LESS_EQUAL:
                    {
                        DEBUG_PRINT_CODE("LESS_EQUAL");
                        int v2 = script_env[id].pop();
                        int v1 = script_env[id].pop();
                        script_env[id].push(v1 <= v2 ? 1 : 0);
                        break;
                    }
                case SCRIPT_SPIN_OBJECT:
                    {
                        DEBUG_PRINT_CODE("SPIN_OBJECT");
                        int obj = script->script_code[script_id][pos++];
                        int axis = script->script_code[script_id][pos++];
                        int v1 = script_env[id].pop();
                        int v2 = script_env[id].pop();
                        pUnit->script_spin_object(obj, axis, v1 * TA2DEG, v2 * TA2DEG);
                    }
                    break;
                case SCRIPT_SLEEP:
                    {
                        DEBUG_PRINT_CODE("SLEEP");
                        script_env[id].wait = script_env[id].pop() * 0.001f;
                        done = true;
                        break;
                    }
                case SCRIPT_MULTIPLY:
                    {
                        DEBUG_PRINT_CODE("MULTIPLY");
                        int v1 = script_env[id].pop();
                        int v2 = script_env[id].pop();
                        script_env[id].push(v1 * v2);
                        break;
                    }
                case SCRIPT_CALL_SCRIPT:
                    {
                        DEBUG_PRINT_CODE("CALL_SCRIPT");
                        int function_id = script->script_code[script_id][pos];			// Lit un code
                        ++pos;
                        int num_param = script->script_code[script_id][pos];			// Lit un code
                        ++pos;
                        script_env[id].env->cur = script_id + (pos<<8);
                        SCRIPT_ENV_STACK *old = script_env[id].env;
                        script_env[id].env = new SCRIPT_ENV_STACK();
                        script_env[id].env->init();
                        script_env[id].env->next = old;
                        script_env[id].env->cur = function_id;
                        for(int i = num_param - 1 ; i >= 0 ; i--)		// Lit les paramètres
                            script_env[id].env->var[i] = script_env[id].pop();
                        done = true;
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
                        int v1 = script_env[id].pop();
                        int v2 = script_env[id].pop();
                        script_env[id].push(v1 == v2 ? 1 : 0);
                        break;
                    }
                case SCRIPT_NOT_EQUAL:
                    {
                        DEBUG_PRINT_CODE("NOT_EQUAL");
                        int v1 = script_env[id].pop();
                        int v2 = script_env[id].pop();
                        script_env[id].push(v1 != v2 ? 1 : 0);
                        break;
                    }
                case SCRIPT_IF:
                    {
                        DEBUG_PRINT_CODE("IF");
                        if (script_env[id].pop() != 0)
                            pos++;
                        else
                        {
                            int target_offset = script->script_code[script_id][pos];        // Lit un code
                            pos = target_offset - script->dec_offset[script_id];            // Déplace l'éxecution
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
                        script_env[id].env->cur = script_id + (pos<<8);	    // Sauvegarde la position
                        raise_signal(script_env[id].pop());                 // Tue tout les processus utilisant ce signal
                        return 0;
                    }
                case SCRIPT_DONT_CACHE:
                    {
                        DEBUG_PRINT_CODE("DONT_CACHE");
                        ++pos;
                        break;
                    }
                case SCRIPT_SET_SIGNAL_MASK:
                    {
                        DEBUG_PRINT_CODE("SET_SIGNAL_MASK");
                        script_env[id].env->signal_mask = script_env[id].pop();
                        break;
                    }
                case SCRIPT_NOT:
                    {
                        DEBUG_PRINT_CODE("NOT");
                        script_env[id].push(!script_env[id].pop());
                        break;
                    }
                case SCRIPT_DONT_SHADE:
                    {
                        DEBUG_PRINT_CODE("DONT_SHADE");
                        ++pos;
                        break;
                    }
                case SCRIPT_EMIT_SFX:
                    {
                        DEBUG_PRINT_CODE("EMIT_SFX:");
                        int smoke_type = script_env[id].pop();
                        int from_piece = script->script_code[script_id][pos++];
                        DEBUG_PRINT_CODE("smoke_type " << smoke_type << " from " << from_piece);
                        pUnit->script_emit_sfx( smoke_type, from_piece );
                    }
                    break;
                case SCRIPT_PUSH_CONST:
                    {
                        DEBUG_PRINT_CODE("PUSH_CONST (" << script->script_code[script_id][pos] << ")");
                        script_env[id].push(script->script_code[script_id][pos]);
                        ++pos;
                        break;
                    }
                case SCRIPT_PUSH_VAR:
                    {
                        DEBUG_PRINT_CODE("PUSH_VAR (" << script->script_code[script_id][pos] << ") = "
                                         << script_env[id].env->var[script->script_code[script_id][pos]]);
                        script_env[id].push(script_env[id].env->var[script->script_code[script_id][pos]]);
                        ++pos;
                        break;
                    }
                case SCRIPT_SET_VAR:
                    {
                        DEBUG_PRINT_CODE("SET_VAR (" << script->script_code[script_id][pos] << ")");
                        script_env[id].env->var[script->script_code[script_id][pos]] = script_env[id].pop();
                        ++pos;
                        break;
                    }
                case SCRIPT_PUSH_STATIC_VAR:
                    {
                        DEBUG_PRINT_CODE("PUSH_STATIC_VAR");
                        if (script->script_code[script_id][pos] >= s_var.size() )
                            s_var.resize( script->script_code[script_id][pos] + 1 );
                        script_env[id].push(s_var[script->script_code[script_id][pos]]);
                        ++pos;
                        break;
                    }
                case SCRIPT_SET_STATIC_VAR:
                    {
                        DEBUG_PRINT_CODE("SET_STATIC_VAR");
                        if (script->script_code[script_id][pos] >= s_var.size() )
                            s_var.resize( script->script_code[script_id][pos] + 1 );
                        s_var[script->script_code[script_id][pos]] = script_env[id].pop();
                        ++pos;
                        break;
                    }
                case SCRIPT_OR:
                    {
                        DEBUG_PRINT_CODE("OR");
                        int v1 = script_env[id].pop(), v2 = script_env[id].pop();
                        script_env[id].push(v1 | v2);
                        break;
                    }
                case SCRIPT_START_SCRIPT:				// Transfère l'éxecution à un autre script
                    {
                        DEBUG_PRINT_CODE("START_SCRIPT");
                        int function_id = script->script_code[script_id][pos++];			// Lit un code
                        int num_param = script->script_code[script_id][pos++];			// Lit un code
                        int s_id = call(function_id, 0, NULL);
                        if (s_id >= 0)
                        {
                            for(int i = num_param - 1 ; i >= 0 ; i--)		// Lit les paramètres
                                script_env[s_id].env->var[i] = script_env[id].pop();
                            script_env[s_id].env->signal_mask = script_env[id].env->signal_mask;
                        }
                        else
                        {
                            for (int i = 0 ; i < num_param ; ++i)		// Enlève les paramètres de la pile
                                script_env[id].pop();
                        }
                        done = true;
                        break;
                    }
                case SCRIPT_RETURN:		// Retourne au script précédent
                    {
                        DEBUG_PRINT_CODE("RETURN");
                        if (script_val.size() <= script_id )
                            script_val.resize( script_id + 1 );
                        script_val[script_id] = script_env[id].env->var[0];
                        SCRIPT_ENV_STACK *old = script_env[id].env;
                        script_env[id].env = script_env[id].env->next;
                        delete old;
                        script_env[id].pop();		// Enlève la valeur retournée
                        if (script_env[id].env)
                        {
                            script_id = (script_env[id].env->cur & 0xFF);			// Récupère l'identifiant du script en cours d'éxecution et la position d'éxecution
                            pos = (script_env[id].env->cur >> 8);
                        }
                        done = true;
                        break;
                    }
                case SCRIPT_JUMP:						// Commande de saut
                    {
                        DEBUG_PRINT_CODE("JUMP");
                        int target_offset = script->script_code[script_id][pos];        // Lit un code
                        pos = target_offset - script->dec_offset[script_id];            // Déplace l'éxecution
                        break;
                    }
                case SCRIPT_ADD:
                    {
                        DEBUG_PRINT_CODE("ADD");
                        int v1 = script_env[id].pop();
                        int v2 = script_env[id].pop();
                        script_env[id].push(v1 + v2);
                        break;
                    }
                case SCRIPT_STOP_SPIN:
                    {
                        DEBUG_PRINT_CODE("STOP_SPIN");
                        int obj = script->script_code[script_id][pos++];
                        int axis = script->script_code[script_id][pos++];
                        int v = script_env[id].pop();
                        pUnit->script_stop_spin(obj, axis, v);
                        break;
                    }
                case SCRIPT_DIVIDE:
                    {
                        DEBUG_PRINT_CODE("DIVIDE");
                        int v1 = script_env[id].pop();
                        int v2 = script_env[id].pop();
                        script_env[id].push(v2 / v1);
                        break;
                    }
                case SCRIPT_MOVE_PIECE_NOW:
                    {
                        DEBUG_PRINT_CODE("MOVE_PIECE_NOW");
                        int obj = script->script_code[script_id][pos++];
                        int axis = script->script_code[script_id][pos++];
                        int pos = script_env[id].pop();
                        pUnit->script_move_piece_now(obj, axis, pos * div);
                        break;
                    }
                case SCRIPT_TURN_PIECE_NOW:
                    {
                        DEBUG_PRINT_CODE("TURN_PIECE_NOW");
                        int obj = script->script_code[script_id][pos++];
                        int axis = script->script_code[script_id][pos++];
                        int v = script_env[id].pop();
                        pUnit->script_turn_piece_now(obj, axis, v * TA2DEG);
                        break;
                    }
                case SCRIPT_CACHE:
                    DEBUG_PRINT_CODE("CACHE");
                    ++pos;
                    break;	//added
                case SCRIPT_COMPARE_AND:
                    {
                        DEBUG_PRINT_CODE("COMPARE_AND");
                        int v1 = script_env[id].pop();
                        int v2 = script_env[id].pop();
                        script_env[id].push(v1 && v2);
                        break;
                    }
                case SCRIPT_COMPARE_OR:
                    {
                        DEBUG_PRINT_CODE("COMPARE_OR");
                        int v1 = script_env[id].pop();
                        int v2 = script_env[id].pop();
                        script_env[id].push(v1 || v2);
                        break;
                    }
                case SCRIPT_CALL_FUNCTION:
                    {
                        DEBUG_PRINT_CODE("CALL_FUNCTION");
                        int function_id = script->script_code[script_id][pos++];			// Lit un code
                        int num_param = script->script_code[script_id][pos++];			// Lit un code
                        script_env[id].env->cur = script_id + (pos << 8);
                        SCRIPT_ENV_STACK *old = script_env[id].env;
                        script_env[id].env = new SCRIPT_ENV_STACK();
                        script_env[id].env->init();
                        script_env[id].env->next = old;
                        script_env[id].env->cur = function_id;
                        for(int i = num_param - 1 ; i >= 0 ; i--)		// Lit les paramètres
                            script_env[id].env->var[i] = script_env[id].pop();
                        done = true;
                        pos = 0;
                        script_id = function_id;
                        break;
                    }
                case SCRIPT_GET:
                    {
                        DEBUG_PRINT_CODE("GET *");
                        script_env[id].pop();
                        script_env[id].pop();
                        int v2 = script_env[id].pop();
                        int v1 = script_env[id].pop();
                        int val = script_env[id].pop();
                        script_env[id].push( pUnit->script_get(val, v1, v2) );
                        break;	//added
                    }
                case SCRIPT_SET_VALUE:
                    {
                        DEBUG_PRINT_CODE("SET_VALUE *:");
                        int v1 = script_env[id].pop();
                        int v2 = script_env[id].pop();
                        DEBUG_PRINT_CODE(v1 << " " << v2);
                        pUnit->script_set_value( v2, v1 );
                    }
                    break;	//added
                case SCRIPT_ATTACH_UNIT:
                    {
                        DEBUG_PRINT_CODE("ATTACH_UNIT");
                        /*int v3 =*/ script_env[id].pop();
                        int v2 = script_env[id].pop();
                        int v1 = script_env[id].pop();
                        pUnit->script_attach_unit(v1, v2);
                        break;	//added
                    }
                case SCRIPT_DROP_UNIT:
                    {
                        DEBUG_PRINT_CODE("DROP_UNIT *");
                        int v1 = script_env[id].pop();
                        DEBUG_PRINT_CODE("Dropping " << v1);
                        pUnit->script_drop_unit(v1);
                        break;	//added
                    }
                default:
                    LOG_ERROR("UNKNOWN " << script->script_code[script_id][--pos] << ", Stopping script");
                    {
                        if (script_val.size() <= script_id )
                            script_val.resize( script_id + 1 );
                        script_val[script_id] = script_env[id].env->var[0];
                        SCRIPT_ENV_STACK *old = script_env[id].env;
                        script_env[id].env = script_env[id].env->next;
                        delete old;
                    }
                    if (script_env[id].env)
                    {
                        script_id = (script_env[id].env->cur & 0xFF);			// Récupère l'identifiant du script en cours d'éxecution et la position d'éxecution
                        pos = (script_env[id].env->cur >> 8);
                    }
                    else
                        script_env[id].running = false;
                    done = true;
            };
        } while(!done);

        if (script_env[id].env)
            script_env[id].env->cur = script_id + (pos << 8);
        return 0;
    }

    int COB_VM::call(int id, int nb_param, int *param)			// Start a script as a separate "thread" of the unit
    {
        if (!script || id < 0 || id >= script->nb_script)
            return -2;

        if (nb_running >= 25)	// Too much scripts running
        {
            LOG_WARNING("Too much script running");
            return -3;
        }

        if (script_env.size() <= nb_running )
            script_env.resize( nb_running + 1);
        script_env[nb_running].init();
        script_env[nb_running].env = new SCRIPT_ENV_STACK();
        script_env[nb_running].env->init();
        script_env[nb_running].env->cur = id;
        script_env[nb_running].running = true;
        if (nb_param > 0 && param != NULL)
        {
            for(int i = 0 ; i < nb_param ; i++)
                script_env[nb_running].env->var[i] = param[i];
        }
        return nb_running++;
    }

    void COB_VM::raise_signal(uint32 signal)		// Tue les processus associés
    {
        SCRIPT_ENV_STACK *tmp;
        for (int i = 0; i  < nb_running; ++i)
        {
            tmp = script_env[i].env;
            while (tmp)
            {
                if (tmp->signal_mask == signal)
                {
                    tmp = script_env[i].env;
                    while (tmp != NULL)
                    {
                        script_env[i].env = tmp->next;
                        delete tmp;
                        tmp = script_env[i].env;
                    }
                }
                if (tmp)
                    tmp = tmp->next;
            }
            if (script_env[i].env == NULL)
                script_env[i].running = false;
        }
    }
}
