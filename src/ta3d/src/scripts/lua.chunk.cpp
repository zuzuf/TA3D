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
#include <TA3D_NameSpace.h>
#include <yuni/core/io/file/stream.h>
#include "lua.chunk.h"
#include "lua.thread.h"

using namespace std;

namespace TA3D
{


    int LuaChunk::WriterFunc(lua_State*, const void* p, size_t size, void* u)
    {
        LuaChunk *chunk = (LuaChunk*) u;
        if (u == NULL)
			return 1;

        byte *nBuf = new byte[chunk->size + size];
        if (chunk->buffer)
        {
            memcpy(nBuf, chunk->buffer, chunk->size);
			DELETE_ARRAY(chunk->buffer);
        }
        memcpy(nBuf + chunk->size, p, size);
		chunk->size += (uint32)size;
        chunk->buffer = nBuf;
        return 0;
    }


    int LuaChunk::load(lua_State *L)
    {
        return luaL_loadbuffer(L, (char*)buffer, size, name.toStdString().c_str());
    }

    void LuaChunk::dump(lua_State *L, const QString &name)
    {
        destroy();
        lua_dump(L, WriterFunc, (void*)this);
        this->name = name;
    }


    void LuaChunk::load(const QString &filename)                    // Load a lua chunk
    {
        destroy();

		LuaThread::Ptr thread = new LuaThread;
        thread->load(filename);

		LuaChunk::Ptr chunk = thread->dump();

        buffer = chunk->buffer;
        size = chunk->size;
        name = chunk->name;
        chunk->buffer = NULL;
    }

    void LuaChunk::save(const QString &filename)                    // Save the lua chunk
    {
        if (buffer == NULL || size == 0)    return;

        QFile file(filename);
        file.open(QIODevice::WriteOnly);
        if (file.isOpen())
        {
            file.write((char*)buffer, size);
            file.close();
        }
    }


    void LuaChunk::init()
    {
        buffer = NULL;
        size = 0;
        piece_name.clear();
    }


    void LuaChunk::destroy()
    {
		DELETE_ARRAY(buffer);
        size = 0;
    }


    int LuaChunk::identify(const QString &name)
    {
        if (piece_name.empty())
        {
			LuaThread::Ptr thread = new LuaThread();
            thread->load(this);
            thread->run();      // Initialize the thread (read functions, pieces, ...)
            if (thread->L)
            {
                lua_getglobal(thread->L, "__piece_list");
                if (lua_istable(thread->L, -1))
                {
                    const size_t nb_pieces = lua_objlen(thread->L, -1);
                    piece_name.clear();
                    piece_name.reserve(nb_pieces);
                    for(size_t i = 1 ; i <= nb_pieces ; ++i)
                    {
                        lua_rawgeti(thread->L, -1, i);
                        piece_name.push_back(lua_tostring(thread->L, -1));
                        lua_pop(thread->L, 1);
                    }
                }
            }
        }

        const QString query(name.toLower());
		for (uint32 i = 0; i < piece_name.size(); ++i)
		{
            if (piece_name[i] == query)
                return i;
		}
        return -1;
    }



} // namespace TA3D
