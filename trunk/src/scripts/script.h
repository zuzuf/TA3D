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

/*----------------------------------------------------------------------\
|                                 script.h                              |
|      contient les classes nécessaires à la gestion des scripts de     |
| controle du déroulement de la partie. Les scripts peuvent influencer  |
| considérablement le déroulement de la partie en manipulant les unités |
| les ressources mais aussi l'écran et déclenche les signaux de défaite |
| et de victoire.                                                       |
\----------------------------------------------------------------------*/

#ifndef CLASSE_SCRIPT
# define CLASSE_SCRIPT

# include "lua.thread.h"
# include "../threads/thread.h"
# include "../misc/tdf.h"
# include "draw.list.h"

namespace TA3D
{

    class LUA_PROGRAM : public LUA_THREAD
    {
    private:
        //  Variables to control execution flow
        int         amx,amy,amz;    // Cursor coordinates
        int         amb;            // Mouse button
        int         signal;         // Current signal (0 is none)

    public:
        DRAW_LIST   draw_list;      // Display commands list

        static bool passive;        // Passive mode, won't do anything like creating units, move units, etc... used to resync a multiplayer game

        void init();
        void destroy();

        LUA_PROGRAM();

        inline ~LUA_PROGRAM()  {   destroy();  }

        int run(float dt);                  // Run the script
        int check();                        // Display DRAW_LIST commands and check if a signal was sent

    private:
        virtual void register_functions();

    protected:
        virtual void proc(void* param);

    public:
        static LUA_PROGRAM	*inGame;
    };

    void generate_script_from_mission( String Filename, TDFParser& ota_parser, int schema = 0 );


} // namespace TA3D

#endif
