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
#include <misc/paths.h>
#include <gfx/gfx.toolkit.h>
#include <TA3D_NameSpace.h>
#include <logs/logs.h>
#include "lua.thread.h"
#include "lua.env.h"
#include <input/mouse.h>
#include <misc/timer.h>



namespace TA3D
{
	int lua_panic( lua_State *L  )
	{
		if (lua_gettop(L) > 0 && lua_isstring(L, -1))
		{
			LOG_ERROR(LOG_PREFIX_LUA << "lua_panic /o\\ : " << lua_tostring(L, -1));
			lua_pop(L, 1);
		}
		else
			LOG_ERROR(LOG_PREFIX_LUA << "lua_panic /o\\ with no error message");
		throw 0;	// Wow we don't want Lua to kill TA3D huh
		return 0;
	}

    LuaThread *lua_threadID( lua_State *L )
	{
		try
		{
			lua_getfield(L, LUA_REGISTRYINDEX, "threadID");
			LuaThread *p = (LuaThread*) lua_touserdata( L, -1 );
			lua_pop(L, 1);
			return p;
		}
		catch(...)
		{
			LOG_ERROR(__FILE__ << " l." << __LINE__ << " : Lua exception caught");
			throw 0;
		}

		return NULL;
	}

	void lua_pushvector( lua_State *L, const Vector3D &vec )
	{
		lua_newtable(L);
		lua_pushnumber(L, vec.x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, vec.y);
		lua_setfield(L, -2, "y");
		lua_pushnumber(L, vec.z);
		lua_setfield(L, -2, "z");

		lua_getfield(L, LUA_GLOBALSINDEX, "__vector_metatable");
		lua_setmetatable(L, -2);
	}

	Vector3D lua_tovector( lua_State *L, int idx )
	{
		Vector3D vec;

		lua_pushstring(L, "x");
		lua_rawget(L, idx - 1);
		vec.x = (float) lua_tonumber(L, -1);

		lua_pushstring(L, "y");
		lua_rawget(L, idx - 2);
		vec.y = (float) lua_tonumber(L, -1);

		lua_pushstring(L, "z");
		lua_rawget(L, idx - 3);
		vec.z = (float) lua_tonumber(L, -1);
		lua_pop(L, 3);

		return vec;
	}

	void lua_pushcolor( lua_State *L, const uint32 color )
	{
		lua_newtable(L);
		lua_pushinteger(L, getr(color));
		lua_setfield(L, -2, "r");
		lua_pushinteger(L, getg(color));
		lua_setfield(L, -2, "g");
		lua_pushinteger(L, getb(color));
		lua_setfield(L, -2, "b");
		lua_pushinteger(L, geta(color));
		lua_setfield(L, -2, "a");
	}

	uint32 lua_tocolor( lua_State *L, int idx )
	{
		uint32 r,g,b,a;
		lua_getfield(L, idx, "r");
		r = (uint32) lua_tointeger(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, idx, "g");
		g = (uint32) lua_tointeger(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, idx, "b");
		b = (uint32) lua_tointeger(L, -1);
		lua_pop(L, 1);

		lua_getfield(L, idx, "a");
		if (lua_isnil(L, -1))
			a = 0xFF;
		else
			a = (uint32) lua_tointeger(L, -1);
		lua_pop(L, 1);

		return makeacol(r,g,b,a);
	}

	void LuaThread::init()
	{
		caller = NULL;

		signal_mask = 0;
		running = false;

		buffer = NULL;
		L = NULL;

		sleep_time = 0.0f;
		sleeping = false;
		waiting = false;

		n_args = 0;

        crashed = false;

        last = msectimer();
	}

	void LuaThread::destroy()
	{
        deleteThreads();

        if (L)
			lua_settop(L, 0);
		if ( L && caller == NULL )
			lua_close( L );
		DELETE_ARRAY(buffer);
		running = false;
        crashed = false;

		init();
	}

	LuaThread::LuaThread() : nextID(0)
	{
		init();
	}

	LuaThread::~LuaThread()
	{
		destroy();
	}

	byte *loadLuaFile(const QString &filename, uint32 &filesize)
	{
		filesize = 0;
        QIODevice *file = VFS::Instance()->readFile(filename);
		byte *buffer = NULL;
		if (file)
		{
			filesize = file->size();
			buffer = new byte[file->size() + 1];
            file->read((char*)buffer, file->size());
			buffer[file->size()] = 0;
			delete file;

            const QString &path = Paths::ExtractFilePath(filename);
			QString name;
			int n = 0;
			char *f = NULL;
			while ((f = strstr( (char*)buffer, "#include" ) ) != NULL && n < 20)
			{
				int i;
				name.clear();
				for (i = 0 ; i < 100 && f[ i + 10 ] != '"' ; ++i)
                    name += f[ i + 10 ];
                if (!VFS::Instance()->fileExists(path + name))
                    name = "scripts/" + name;
				else
                    name = path + name;
				file = VFS::Instance()->readFile(name);
				if (file)
				{
					uint32 filesize2 = file->size();
					byte *buffer2 = new byte[filesize2];
                    file->read((char*)buffer2, file->size());
					delete file;
					if (buffer2)
					{
						byte *buffer3 = new byte[ filesize + filesize2 + 1 ];
						memset( buffer3, 0, filesize + filesize2 + 1 );
						memcpy( buffer3, buffer, f - (char*)buffer );
						memcpy( buffer3 + (f - (char*)buffer), buffer2, filesize2 );
						memcpy( buffer3 + (f - (char*)buffer) + filesize2, f + i + 11, filesize - ( f + i + 11 - (char*)buffer ) );
						filesize += filesize2 - i - 11;
						DELETE_ARRAY(buffer);
						DELETE_ARRAY(buffer2);
						buffer = buffer3;
					}
				}
				else
					break;
				n++;
			}
		}
		return buffer;
	}

	void LuaThread::load(const QString &filename)					// Load a lua script
	{
		destroy();			// Maybe we're reusing an old object

		uint32 filesize = 0;
		buffer = loadLuaFile(filename , filesize);
		if (buffer)
		{
			L = lua_open();				// Create a lua state object
			lua_atpanic(L, lua_panic);	// Just to avoid having Lua exiting TA3D

			if (L == NULL)
			{
				running = false;
				DELETE_ARRAY(buffer);
				return;
			}

			register_basic_functions();
			register_functions();

			uint32 filesize2 = 0;
			byte *header_buffer = loadLuaFile("scripts/ta3d.lh" , filesize2);
			if (header_buffer == NULL)
			{
				if (lua_tostring( L, -1 ) != NULL && strlen(lua_tostring( L, -1 )) > 0)
					LOG_ERROR(LOG_PREFIX_LUA << lua_tostring( L, -1));

				running = false;
				lua_close( L );
				DELETE_ARRAY(buffer);
				L = NULL;
				return;
			}
			byte *tmp = buffer;
            buffer = new byte[filesize + filesize2 + 2];
			memcpy(buffer, header_buffer, filesize2);
            memcpy(buffer + filesize2 + 1, tmp, filesize);
            buffer[filesize2] = '\n';
            filesize += filesize2;
            buffer[filesize] = 0;
			DELETE_ARRAY(header_buffer);
			DELETE_ARRAY(tmp);

			name = filename;
            if (luaL_loadbuffer(L, (const char*)buffer, filesize, name.toStdString().c_str() ))
			{
                if (lua_gettop(L) > 0 && lua_tostring( L, -1 ) != NULL && strlen(lua_tostring( L, -1 )) > 0)
				{
                    LOG_ERROR(LOG_PREFIX_LUA << __FILE__ << " l." << __LINE__);
                    LOG_ERROR(LOG_PREFIX_LUA << lua_tostring( L, -1));
					LOG_ERROR(LOG_PREFIX_LUA << filesize << " -> " << (int)buffer[filesize-1]);
					LOG_ERROR((const char*) buffer);
				}

				running = false;
				lua_close( L );
				DELETE_ARRAY(buffer);
				L = NULL;
			}
			else
			{
				// This may not help debugging
				DELETE_ARRAY(buffer);

				running = true;
				setThreadID();
                last = msectimer();
			}
		}
		else
		{
			LOG_ERROR(LOG_PREFIX_LUA << "Failed opening `" << filename << "`");
			running = false;
		}
	}

	void LuaThread::load(ScriptData *data)
	{
		destroy();
		LuaChunk *chunk = dynamic_cast<LuaChunk*>(data);
		if (chunk)
		{
			L = lua_open();				// Create a lua state object
			lua_atpanic(L, lua_panic);	// Just to avoid having Lua exiting TA3D

			if (L == NULL)
			{
				running = false;
				buffer = NULL;
				return;
			}

			register_basic_functions();
			register_functions();

			name = chunk->getName();
			if (chunk->load(L))
			{
                if (lua_gettop(L) > 0 && lua_tostring( L, -1 ) != NULL && strlen(lua_tostring( L, -1 )) > 0)
                {
                    LOG_ERROR(LOG_PREFIX_LUA << __FILE__ << " l." << __LINE__);
					LOG_ERROR(LOG_PREFIX_LUA << lua_tostring( L, -1));
                }

				running = false;
				lua_close( L );
				L = NULL;
			}
			else
			{
				running = true;
				setThreadID();
			}
		}
	}

	LuaChunk *LuaThread::dump()
	{
		return new LuaChunk(L, name);
	}

	int thread_logmsg( lua_State *L )		// logmsg( str )
	{
		const char *str = lua_tostring( L, -1 );		// Read the result
		if (str)
			LOG_INFO(LOG_PREFIX_LUA << str);
		lua_pop(L, 1);
		return 0;
	}

	int thread_mouse_x( lua_State *L )		// mouse_x()
	{
		lua_pushinteger( L, mouse_x );
		return 1;
	}

	int thread_mouse_y( lua_State *L )		// mouse_y()
	{
		lua_pushinteger( L, mouse_y );
		return 1;
	}

	int thread_mouse_z( lua_State *L )		// mouse_z()
	{
		lua_pushinteger( L, mouse_z );
		return 1;
	}

	int thread_mouse_b( lua_State *L )		// mouse_b()
	{
		lua_pushinteger( L, mouse_b );
		return 1;
	}

	int thread_time( lua_State *L )		// time()
	{
        lua_pushnumber( L, msectimer() * 0.001 );
		return 1;
	}

	int thread_signal( lua_State *L )       // signal( sig )
	{
		LuaThread *lua_thread = lua_threadID(L);
		if (lua_thread)
			lua_thread->processSignal( (uint32)lua_tointeger(L, 1) );
		lua_settop(L, 0);
		return 0;
	}

	int thread_start_script( lua_State *L )         // start_script( function, params )
	{
		int n = lua_gettop(L);

		if (lua_isfunction(L, 1))
		{
			LuaThread *lua_thread = lua_threadID(L);
			if (lua_thread)
			{
				LuaThread *newThread = lua_thread->fork(L, n);
				newThread->setSignalMask( lua_thread->getSignalMask() );
			}
		}

		return 0;
	}

	void LuaThread::register_basic_functions()
	{
		lua_register( L, "logmsg", thread_logmsg );
		lua_register( L, "mouse_x", thread_mouse_x );
		lua_register( L, "mouse_y", thread_mouse_y );
		lua_register( L, "mouse_z", thread_mouse_z );
		lua_register( L, "mouse_b", thread_mouse_b );
		lua_register( L, "time", thread_time );
		lua_register( L, "signal", thread_signal );
		lua_register( L, "start_script", thread_start_script );
		LuaEnv::register_global_functions( L );
	}

	int LuaThread::run(float dt, bool alone)               // Run the script
	{
		MutexLocker mLocker( pMutex );
        if (!L)
            return -1;

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

		try
		{
			int result = lua_resume(L, n_args);
			n_args = 0;
            if (result != LUA_YIELD && result != 0)
			{
                if (lua_gettop(L) > 0 && !lua_isnoneornil(L, -1) && lua_tostring(L, -1) != NULL && strlen(lua_tostring(L, -1)) > 0)
                {
                    LOG_ERROR(LOG_PREFIX_LUA << __FILE__ << " l." << __LINE__);
                    LOG_ERROR(LOG_PREFIX_LUA << lua_tostring(L, -1));
                }
				running = false;
                crashed = true;
                return -0xFFFF;         // Crashed
			}
            else if (lua_gettop(L) > 0)
			{
                running = result == LUA_YIELD;
				result = 0;
                while(lua_gettop(L) > 0 && !lua_isnone(L, -1) && !lua_isfunction(L, -1))
				{
					switch(result)
					{
						case 0:
							result = (int)lua_tointeger(L, -1);
							break;
						case 1:             // sleep
							pause( (float)lua_tonumber(L, -1) );
							result = 0;
							break;
						case 2:             // wait
							stop();
							result = 0;
							break;
						case 3:             // set_signal_mask
							setSignalMask( (uint32)lua_tointeger(L, -1) );
							result = 0;
							break;
						case 4:             // end_thread
							kill();
							result = 0;
							break;
					};
					lua_pop(L, 1);
				}
				return result;
			}
            running = result == LUA_YIELD;
            return 0;
        }
		catch(...)
		{
			LOG_ERROR(__FILE__ << " l." << __LINE__ << " : Lua exception caught");
			if (lua_gettop(L) > 0 && !lua_isnoneornil(L, -1) && lua_tostring( L, -1 ) != NULL && strlen(lua_tostring( L, -1 )) > 0)
            {
                LOG_ERROR(LOG_PREFIX_LUA << __FILE__ << " l." << __LINE__);
                LOG_ERROR(LOG_PREFIX_LUA << lua_tostring(L, -1));
            }
			running = false;
            crashed = true;
            return -0xFFFF;         // Crashed
		}
		return 0;
	}

	int LuaThread::run()                          // Run the script, using default delay
	{
        const uint32 timer = msectimer();
		const float dt = (float)(timer - last);
		last = timer;
		return run(dt);
	}

	LuaThread *LuaThread::fork()
	{
		pMutex.lock();

		if (running == false && caller == NULL)
		{
			sleeping = false;
			sleep_time = 0.0f;
			waiting = false;
			signal_mask = 0;
			lua_settop(L, 0);
			pMutex.unlock();
			return this;
		}

        LuaThread *newThread = static_cast<LuaThread*>(getFreeThread());
        if (newThread)
        {
            newThread->running = false;
            newThread->waiting = false;
            newThread->sleeping = false;
            newThread->sleep_time = 0.0f;
            newThread->signal_mask = 0;
            lua_settop(newThread->L, 0);
            addThread(newThread);

            pMutex.unlock();
            return newThread;
        }

        newThread = new LuaThread();

		newThread->running = false;
		newThread->buffer = NULL;
		newThread->waiting = false;
		newThread->sleeping = false;
		newThread->sleep_time = 0.0f;
		newThread->caller = (caller != NULL) ? caller : this;

		newThread->L = lua_newthread(L);
        QString globalName( QString("__thread%1").arg(getNextID()) );
        lua_setglobal(L, globalName.toStdString().c_str());  // We don't want to keep this thread value on top of the stack
		addThread(newThread);

		pMutex.unlock();
		return newThread;
	}

	LuaThread *LuaThread::fork(const QString &functionName, int *parameters, int nb_params)
	{
		pMutex.lock();

		LuaThread *newThread = fork();
		if (newThread)
			newThread->call(functionName, parameters, nb_params);

		pMutex.unlock();
		return newThread;
	}

	void LuaThread::call(const QString &functionName, int *parameters, int nb_params)
	{
		MutexLocker mLocker( pMutex );

		if (running)    return;     // We cannot run several functions at the same time on the same stack

        crashed = false;

		try
		{
			lua_settop(L, 0);
            lua_getglobal( L, functionName.toStdString().c_str() );
			if (lua_isnil( L, -1 ))     // Function not found
			{
				lua_pop(L, 1);
				LOG_DEBUG(LOG_PREFIX_LUA << "call: function not found `" << functionName << "`");
				return;
			}

			if (parameters == NULL)
				nb_params = 0;
			for(int i = 0 ; i < nb_params ; i++)
				lua_pushinteger(L, parameters[i]);
			n_args = nb_params;
			running = true;
		}
		catch(...)
		{
			LOG_ERROR(__FILE__ << " l." << __LINE__ << " : Lua exception caught");
			running = false;
		}
	}

	int LuaThread::execute(const QString &functionName, int *parameters, int nb_params)
	{
		MutexLocker mLocker( pMutex );

        crashed = false;

		try
		{
			lua_settop(L, 0);
            lua_getglobal( L, functionName.toStdString().c_str() );
			if (lua_isnil( L, -1 ))     // Function not found
			{
				lua_pop(L, 1);
				LOG_DEBUG(LOG_PREFIX_LUA << "execute: function not found `" << functionName << "`");
				return -2;
			}

			if (parameters == NULL)
				nb_params = 0;
			for(int i = 0 ; i < nb_params ; i++)
				lua_pushinteger(L, parameters[i]);
			if (lua_pcall( L, nb_params, 1, 0))
			{
                if (lua_gettop(L) > 0 && lua_tostring(L, -1) != NULL && strlen(lua_tostring(L, -1)) > 0)
                {
                    LOG_ERROR(LOG_PREFIX_LUA << __FILE__ << " l." << __LINE__);
                    LOG_ERROR(LOG_PREFIX_LUA << lua_tostring(L, -1));
                }
				running = false;
				return -1;
			}
		}
		catch(...)
		{
			LOG_ERROR(__FILE__ << " l." << __LINE__ << " : Lua exception caught");
			if (lua_gettop(L) > 0 && lua_tostring( L, -1 ) != NULL && strlen(lua_tostring( L, -1 )) > 0)
            {
                LOG_ERROR(LOG_PREFIX_LUA << __FILE__ << " l." << __LINE__);
                LOG_ERROR(LOG_PREFIX_LUA << lua_tostring(L, -1));
            }
			running = false;
			return -1;
		}

        if (lua_gettop(L) > 0)
        {
			const int result = lua_isboolean(L,-1) ? lua_toboolean(L,-1) : (int)lua_tointeger( L, -1 );    // Read the result
            lua_pop( L, 1 );
            return result;
        }
        return 0;
	}

	LuaThread *LuaThread::fork(lua_State *cL, int n)
	{
		pMutex.lock();

		LuaThread *newThread = fork();

        if (lua_isfunction(cL, -n))
        {
            lua_xmove(cL, newThread->L, n);
            newThread->n_args = n - 1;
            newThread->running = true;
        }
        else
            newThread->running = false;

		pMutex.unlock();
		return newThread;
	}

	void LuaThread::setThreadID()
	{
		try
		{
			lua_pushlightuserdata( L, (void*)this );            // The pointer itself
			lua_setfield(L, LUA_REGISTRYINDEX, "threadID");     // Save this at the first position on the stack :), this identifies the
			// LuaThread associated with this Lua_State object
			if (lua_threadID(L) == NULL)
				LOG_ERROR(LOG_PREFIX_LUA << "impossible to write LuaThread pointer into Lua_State stack !!");
		}
		catch(...)
		{
			LOG_ERROR(__FILE__ << " l." << __LINE__ << " : Lua exception caught");
			throw 0;
		}
	}

	void LuaThread::save_thread_state(gzFile /*file*/)
	{
	}

	void LuaThread::restore_thread_state(gzFile /*file*/)
	{
	}

    bool LuaThread::runCommand(const QString &cmd)
    {
        MutexLocker mLocker( pMutex );
        if (L == NULL)
            return false;

        if (luaL_loadbuffer(L, (const char*)cmd.toStdString().c_str(), cmd.size(), "user command" ))
        {
            if (lua_gettop(L) > 0 && lua_tostring( L, -1 ) != NULL && strlen(lua_tostring( L, -1 )) > 0)
            {
                LOG_ERROR(LOG_PREFIX_LUA << lua_tostring( L, -1));
                LOG_ERROR(cmd);
            }
            return false;
        }
        else
        {
            try
            {
                if (lua_pcall(L, 0, 0, 0))
                {
                    if (lua_gettop(L) > 0 && lua_tostring(L, -1) != NULL && strlen(lua_tostring(L, -1)) > 0)
                    {
                        LOG_ERROR(LOG_PREFIX_LUA << __FILE__ << " l." << __LINE__);
                        LOG_ERROR(LOG_PREFIX_LUA << lua_tostring(L, -1));
                    }
                    return false;
                }
            }
            catch(...)
            {
                if (lua_gettop(L) > 0 && lua_tostring( L, -1 ) != NULL && strlen(lua_tostring( L, -1 )) > 0)
                {
                    LOG_ERROR(LOG_PREFIX_LUA << __FILE__ << " l." << __LINE__);
                    LOG_ERROR(LOG_PREFIX_LUA << lua_tostring(L, -1));
                }
                return false;
            }
        }
        return true;
    }

    int LuaThread::getMem()
    {
        lock();
        int mem = L != NULL ? lua_gc(L, LUA_GCCOUNT, 0) * 1024 + lua_gc(L, LUA_GCCOUNTB, 0) : 0;
        unlock();
        return mem;
    }

} // namespace TA3D
