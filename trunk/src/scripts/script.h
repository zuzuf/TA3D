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

# include "../lua/lua.hpp"
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

        void draw(GfxFont &fnt);
    };

    class LUA_PROGRAM : public ObjectSync
    {
        byte        *buffer;
        lua_State   *L;             // Pointer to the lua data

        //  Pour l'éxecution du code
        int         last;           // Dernière lecture du timer
        int         amx,amy,amz;    // Coordonnées du curseur
        int         amb;            // Boutons de la souris
        float       asm_timer;      // Timer to match the game speed
        bool        running;

    public:
        float       sleep_time;     // Temps d'attente
        bool        sleeping;       // Indique si le programme marque une pause
        bool        waiting;        // Indique si le programme attend une action utilisateur
        DRAW_LIST   draw_list;      // Liste de commandes d'affichage

        inline void stop()
        {
            destroy();
        }

        void init()
        {
            running = false;

            buffer = NULL;
            L = NULL;

            asm_timer = 0.0f;

            draw_list.init();

            amx=amy=amz=0;
            amb=0;

            sleep_time=0.0f;
            sleeping=false;
            waiting=false;
        }

        void destroy()
        {
            if( L )
                lua_close( L );
            if( buffer )
                delete[] buffer;
            running = false;

            draw_list.destroy();
            init();
        }

        LUA_PROGRAM();

        ~LUA_PROGRAM()
        {
            destroy();
        }

        void load(const String &filename, MAP *map);                    // Load a lua script

        int run(MAP *map,float dt,int viewer_id);                   // Execute le script
    };

    extern LUA_PROGRAM  *lua_program;

    void generate_script_from_mission( String Filename, TDFParser& ota_parser, int schema = 0 );


} // namespace TA3D

#endif
