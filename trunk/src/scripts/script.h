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

# ifndef luaL_dobuffer
#  define luaL_dobuffer(L, s, sz) \
    (luaL_loadbuffer(L, (const char*)s, sz, "main" ) || lua_pcall(L, 0, LUA_MULTRET, 0))
# endif

# define DRAW_TYPE_NONE     0x0
# define DRAW_TYPE_POINT    0x1
# define DRAW_TYPE_LINE     0x2
# define DRAW_TYPE_CIRCLE   0x3
# define DRAW_TYPE_TRIANGLE 0x4
# define DRAW_TYPE_BOX      0x5
# define DRAW_TYPE_FILLBOX  0x6
# define DRAW_TYPE_TEXT     0x7
# define DRAW_TYPE_BITMAP   0x8



namespace TA3D
{


    struct DRAW_OBJECT                  // Pour mémoriser le traçage des primitives
    {
        byte    type;
        float   x[4];
        float   y[4];
        float   r[2],g[2],b[2];
        String  text;
        GLuint  tex;
    };

    class DRAW_LIST
    {
    public:
        DRAW_OBJECT     prim;
        DRAW_LIST       *next;

        void init()
        {
            prim.type=DRAW_TYPE_NONE;
            prim.text.clear();
            next=NULL;
        }

        void destroy()
        {
            if(prim.type==DRAW_TYPE_BITMAP)     glDeleteTextures(1,&prim.tex);
            prim.text.clear();
            if(next) {
                next->destroy();
                delete next;
            }
            init();
        }

        DRAW_LIST()
        {
            init();
        }

        void add(DRAW_OBJECT &obj);

        void draw(Font *fnt);
    };

    class LUA_PROGRAM : public LUA_THREAD, public ObjectSync
    {
        //  Pour l'éxecution du code
        int         amx,amy,amz;    // Coordonnées du curseur
        int         amb;            // Boutons de la souris

    public:
        DRAW_LIST   draw_list;      // Liste de commandes d'affichage

        static bool passive;        // Passive mode, won't do anything like creating units, move units, etc... used to resync a multiplayer game

        void init();
        void destroy();

        LUA_PROGRAM();

        inline ~LUA_PROGRAM()  {   destroy();  }

        int run(float dt);                   // Execute le script

    private:
        virtual void register_functions();
    };

    extern LUA_PROGRAM  *lua_program;

    void generate_script_from_mission( String Filename, TDFParser& ota_parser, int schema = 0 );


} // namespace TA3D

#endif
