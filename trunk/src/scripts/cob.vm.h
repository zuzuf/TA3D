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

# define UNPACKX(xz) ((sint16)((xz)>>16))
# define UNPACKZ(xz) ((sint16)((xz)&0xFFFF))
# define PACKXZ(x,z) ((((int)(x))<<16) | (((int)(z))&0xFFFF))


namespace TA3D
{
    class SCRIPT_ENV_STACK			// Pile pour la gestion des scripts
    {
    public:
        int					var[15];
        uint32				signal_mask;
        sint32				cur;
        SCRIPT_ENV_STACK	*next;

        SCRIPT_ENV_STACK();
        void init();
    };

    class SCRIPT_ENV			// Classe pour la gestion de l'environnement des scripts
    {
    public:
        std::stack<int>		sStack;			// Script stack
        SCRIPT_ENV_STACK	*env;			// Environment stack
        float				wait;
        bool				running;

        SCRIPT_ENV();
        ~SCRIPT_ENV();

        void init();
        void destroy();

        void push(int v);
        int pop();
    };

    /*!
    ** This class represents a the COB Virtual Machine
    */
    class COB_VM
    {
    protected:
        COB_SCRIPT                  *script;
        std::vector<int>            s_var;          // Tableau de variables pour les scripts
        byte                        nb_running;     // Nombre de scripts lancés en même temps
        std::vector< SCRIPT_ENV >   script_env;     // Environnements des scripts
        std::vector< short >        script_val;     // Tableau de valeurs retournées par les scripts
        uint32                      uid;            // Unit ID

    public:
        COB_VM(COB_SCRIPT *p_script);
        COB_VM();
        ~COB_VM();

        int run_thread(const float &dt, const int &id, int max_code);
        int call(int id, int nb_param, int *param);			// Start a script as a separate "thread" of the unit
        void raise_signal(uint32 signal);		            // Tue les processus associés

    private:
        void init();
        void destroy();
    };
}

#endif
