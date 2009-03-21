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

#include "lua.persist.h"
#include "../lua/lstate.h"      // We need this to access lua_State objects to save and load Lua VMs

namespace TA3D
{
#warning TODO: implement save/restore mechanism for Lua script
#define WRITE(x) (gzwrite(file, &x, sizeof(x)))
#define WRITE_STR(str, len) (gzwrite(file, str, len))

    void write_TValue( TValue *v, gzFile file)
    {
        WRITE(v->tt);
        switch( ttype(v) )
        {
        case LUA_TNIL:
            break;
        case LUA_TNUMBER:
            WRITE(v->value.n);
            break;
        case LUA_TSTRING:
            WRITE(v->value.gc->ts.tsv.len);
            WRITE_STR( svalue(v), v->value.gc->ts.tsv.len );
            break;
        case LUA_TTABLE:
            break;
        case LUA_TFUNCTION:
            break;
        case LUA_TBOOLEAN:
            WRITE(v->value.b);
            break;
        case LUA_TUSERDATA:
            break;
        case LUA_TTHREAD:
            save_lua_state( thvalue(v), file );
            break;
        case LUA_TLIGHTUSERDATA:
            break;
        };
    }

    void save_lua_state(lua_State *L, gzFile file)
    {
        global_State *G = L->l_G;
        if (G->mainthread == L)
        {
            WRITE(G->currentwhite);
            WRITE(G->gcstate);
            WRITE(G->sweepstrgc);
            WRITE(G->GCthreshold);
            WRITE(G->totalbytes);
            WRITE(G->estimate);
            WRITE(G->gcdept);
            WRITE(G->gcpause);
        }
    }
}
