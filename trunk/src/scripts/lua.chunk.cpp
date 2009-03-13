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
#include <fstream>
#include "lua.chunk.h"
#include "lua.thread.h"

using namespace std;

namespace TA3D
{
    int LUA_CHUNK::WriterFunc(lua_State* L, const void* p, size_t size, void* u)
    {
        LUA_CHUNK *chunk = (LUA_CHUNK*) u;
        if (u == NULL)  return 1;

        chunk->destroy();
        chunk->size = size;
        chunk->buffer = new char[size];
        memcpy(chunk->buffer, p, size);
        return 0;
    }

    LUA_CHUNK::LUA_CHUNK(lua_State *L, const String &name)
    {
        init();
        dump(L, name);
    }

    LUA_CHUNK::LUA_CHUNK()
    {
        init();
    }

    LUA_CHUNK::~LUA_CHUNK()
    {
        destroy();
    }

    int LUA_CHUNK::load(lua_State *L)
    {
        return luaL_loadbuffer(L, buffer, size, name.c_str());
    }

    void LUA_CHUNK::dump(lua_State *L, const String &name)
    {
        lua_dump(L, WriterFunc, (void*)this);
        this->name = name;
    }

    String LUA_CHUNK::getName()
    {
        return name;
    }

    void LUA_CHUNK::load(const String &filename)                    // Load a lua chunk
    {
        fstream file(filename.c_str(), fstream::in | fstream::binary);
        if (file.is_open())
        {
            destroy();

            int s = name.size();
            file.read((char*)&s, sizeof(s));
            char *tmp = new char[s+1];
            tmp[s] = 0;
            file.read(tmp, s);
            name = tmp;
            delete[] tmp;

            file.read((char*)&size, sizeof(size));
            buffer = new char[size];
            file.read(buffer, size);
            file.close();
        }
    }

    void LUA_CHUNK::save(const String &filename)                    // Save the lua chunk
    {
        if (buffer == NULL || size == 0)    return;

        fstream file(filename.c_str(), fstream::out | fstream::binary);
        if (file.is_open())
        {
            int s = name.size();
            file.write((char*)&s, sizeof(s));
            file.write(name.c_str(), s);

            file.write((char*)&size, sizeof(size));
            file.write(buffer, size);
            file.close();
        }
    }

    void LUA_CHUNK::init()
    {
        buffer = NULL;
        size = 0;
        piece_name.clear();
    }

    void LUA_CHUNK::destroy()
    {
        if (buffer)
            delete[] buffer;
        buffer = NULL;
        size = 0;
    }

    int LUA_CHUNK::identify(const String &name)
    {
#warning TODO: fix piece identifier
        if (piece_name.empty())
        {
            LUA_THREAD *thread = new LUA_THREAD();
            thread->load(this);
            thread->run();      // Initialize the thread
            lua_getglobal(thread->L, "__piece_list");
            if (!lua_isnil(thread->L, -1))
            {
                piece_name.resize(lua_objlen(thread->L, -1));
                for(int i = 1 ; i <= piece_name.size() ; i++)
                {
                    lua_rawgeti(thread->L, -1, i);
                    piece_name[i-1] = lua_tostring(thread->L, -1);
                    lua_pop(thread->L, 1);
                }
            }
            delete thread;
        }
        String query = String::ToLower(name);
        for(int i = 0 ; i < piece_name.size() ; i++)
            if (piece_name[i] == query)
                return i;
        return -1;
    }
}
