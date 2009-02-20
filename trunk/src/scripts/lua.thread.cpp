#include "../stdafx.h"
#include "../misc/paths.h"
#include "../gfx/gfx.toolkit.h"
#include "../TA3D_NameSpace.h"
#include "../logs/logs.h"
#include "lua.thread.h"

namespace TA3D
{
    LUA_THREAD *lua_threadID( lua_State *L )
    {
        LUA_THREAD *p = (LUA_THREAD*) lua_touserdata( L, 1 );
        return p;
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
        lua_pushnumber(L, getr(color));
        lua_setfield(L, -2, "r");
        lua_pushnumber(L, getg(color));
        lua_setfield(L, -2, "g");
        lua_pushnumber(L, getb(color));
        lua_setfield(L, -2, "b");
        lua_pushnumber(L, geta(color));
        lua_setfield(L, -2, "a");
    }

    uint32 lua_tocolor( lua_State *L, int idx )
    {
        uint32 r,g,b,a;
        lua_getfield(L, idx, "r");
        r = (uint32) lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, idx, "g");
        g = (uint32) lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, idx, "b");
        b = (uint32) lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, idx, "a");
        if (lua_isnil(L, -1))
            a = 0xFF;
        else
            a = (uint32) lua_tonumber(L, -1);
        lua_pop(L, 1);

        return makeacol(r,g,b,a);
    }

    void LUA_THREAD::kill()
    {
        pMutex.lock();
        destroy();
        pMutex.unlock();
    }

    void LUA_THREAD::stop()
    {
        waiting = true;
    }

    void LUA_THREAD::resume()
    {
        waiting = false;
    }

    void LUA_THREAD::sleep(float time)
    {
        sleeping = true;
        sleep_time = time;
    }

    void LUA_THREAD::init()
    {
        running = false;

        buffer = NULL;
        L = NULL;

        sleep_time = 0.0f;
        sleeping = false;
        waiting = false;

        n_args = 0;
    }

    void LUA_THREAD::destroy()
    {
        if ( L )
            lua_close( L );
        if ( buffer )
            delete[] buffer;
        running = false;

        init();
    }

    LUA_THREAD::LUA_THREAD()
    {
        init();
    }

    byte *loadLuaFile(const String &filename, uint32 &filesize)
    {
        filesize = 0;
        byte *buffer = (byte*)HPIManager->PullFromHPI(filename , &filesize);
        if (buffer)
        {
            String path = Paths::ExtractFilePath(filename);
            int n = 0;
            char *f = NULL;
            while ((f = strstr( (char*)buffer, "#include" ) ) != NULL && n < 20)
            {
                String name;
                int i;
                for( i = 0 ; i < 100 && f[ i + 10 ] != '"' ; i++ )
                    name << f[ i + 10 ];
                if (!HPIManager->Exists(path + name))
                    name = "scripts/" + name;
                else
                    name = path + name;
                uint32 filesize2 = 0;
                byte *buffer2 = (byte*)HPIManager->PullFromHPI(name, &filesize2);
                if (buffer2)
                {
                    byte *buffer3 = new byte[ filesize + filesize2 ];
                    memset( buffer3, 0, filesize + filesize2 );
                    memcpy( buffer3, buffer, f - (char*)buffer );
                    memcpy( buffer3 + (f - (char*)buffer), buffer2, filesize2 );
                    memcpy( buffer3 + (f - (char*)buffer) + filesize2, f + i + 11, filesize - ( f + i + 11 - (char*)buffer ) );
                    filesize += filesize2 - i - 11;
                    delete[] buffer;
                    delete[] buffer2;
                    buffer = buffer3;
                }
                else
                    break;
                n++;
            }
        }
        return buffer;
    }

    void LUA_THREAD::load(const String &filename)					// Load a lua script
    {
        destroy();			// Maybe we're reusing an old object

        uint32 filesize = 0;
        buffer = loadLuaFile(filename , filesize);
        if (buffer)
        {
            L = lua_open();				// Create a lua state object

            if (L == NULL ) {
                running = false;
                delete[] buffer;
                buffer = NULL;
                return;
            }

            lua_gc( L, LUA_GCSTOP, 0 );		// Load libraries
            luaL_openlibs( L );
            lua_gc( L, LUA_GCRESTART, 0 );

            register_basic_functions();
            register_functions();

			lua_pushlightuserdata( L, (void*)this );            // The pointer itself
			lua_insert( L, 0 );				                    // Save this at the first position on the stack :), this identifies the
                                                                // LUA_THREAD associated with this Lua_State object

            uint32 filesize2 = 0;
            byte *header_buffer = loadLuaFile("scripts/ta3d.h" , filesize2);
            if (header_buffer == NULL)
            {
                if (lua_tostring( L, -1 ) != NULL && strlen(lua_tostring( L, -1 )) > 0)
                    LOG_ERROR(LOG_PREFIX_LUA << lua_tostring( L, -1));

                running = false;
                lua_close( L );
                delete[] buffer;
                L = NULL;
                buffer = NULL;
                return;
            }
            byte *tmp = buffer;
            buffer = new byte[filesize + filesize2];
            memcpy(buffer, header_buffer, filesize2);
            memcpy(buffer+filesize2, tmp, filesize);
            buffer[filesize2-1] = '\n';
            filesize += filesize2;
            delete[] header_buffer;

            if (luaL_loadbuffer(L, (const char*)buffer, filesize, "main" ))
            {
                if (lua_tostring( L, -1 ) != NULL && strlen(lua_tostring( L, -1 )) > 0)
                    LOG_ERROR(LOG_PREFIX_LUA << lua_tostring( L, -1));

                running = false;
                lua_close( L );
                delete[] buffer;
                L = NULL;
                buffer = NULL;
            }
            else
                running = true;
        }
        else
        {
            LOG_ERROR(LOG_PREFIX_LUA << "Failed opening `" << filename << "`");
            running = false;
        }
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
        lua_pushnumber( L, msec_timer * 0.001 );
        return 1;
    }

    int thread_wait( lua_State *L )			// __wait()
    {
        LUA_THREAD *lua_thread = lua_threadID(L);
        if (lua_thread)
            lua_thread->stop();
        return 0;
    }

    int thread_sleep( lua_State *L )			// __sleep( time )
    {
        LUA_THREAD *lua_thread = lua_threadID(L);
        if (lua_thread)
            lua_thread->sleep( (float) lua_tonumber( L, -1 ) );
        lua_pop( L, 1 );
        return 0;
    }

    void LUA_THREAD::register_basic_functions()
    {
        lua_register( L, "logmsg", thread_logmsg );
        lua_register( L, "mouse_x", thread_mouse_x );
        lua_register( L, "mouse_y", thread_mouse_y );
        lua_register( L, "mouse_z", thread_mouse_z );
        lua_register( L, "mouse_b", thread_mouse_b );
        lua_register( L, "time", thread_time );
        lua_register( L, "__wait", thread_wait );
        lua_register( L, "__sleep", thread_sleep );
    }

    int LUA_THREAD::run(float dt)                  // Run the script
    {
        MutexLocker mLocker( pMutex );

        if (!running)   return -1;
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
            if (result != LUA_YIELD)
            {
                if (lua_tostring(L, -1) != NULL && strlen(lua_tostring(L, -1)) > 0)
                    LOG_ERROR(LOG_PREFIX_LUA << lua_tostring(L, -1));
                running = false;
                return -1;
            }
            else
                running = true;
        }
        catch(...)
        {
            if (lua_tostring( L, -1 ) != NULL && strlen(lua_tostring( L, -1 )) > 0)
                LOG_ERROR(LOG_PREFIX_LUA << lua_tostring(L, -1));
            running = false;
            return -1;
        }
        return 0;
    }

    int LUA_THREAD::run()                          // Run the script, using default delay
    {
        uint32 timer = msec_timer;
        float dt = timer - last;
        last = timer;
        return run(dt);
    }

    void LUA_THREAD::proc(void* param)
    {
        while (isRunning() && is_running())
        {
            run();
            rest(1);
        }
    }

    LUA_THREAD *LUA_THREAD::fork()
    {
        LUA_THREAD *newThread = new LUA_THREAD();

        newThread->running = false;
        newThread->buffer = NULL;
        newThread->sleeping = false;
        newThread->sleep_time = 0.0f;
        newThread->L = lua_newthread(L);
        lua_pushlightuserdata( newThread->L, (void*)newThread );    // The pointer itself
        lua_insert( newThread->L, 0 );				                // Save this at the first position on the stack :), this identifies the
                                                                    // LUA_THREAD associated with this Lua_State object

        return newThread;
    }

    LUA_THREAD *LUA_THREAD::fork(const String &functionName, int *parameters, int nb_params)
    {
        LUA_THREAD *newThread = fork();
        if (newThread)
            newThread->call(functionName, parameters, nb_params);
        return newThread;
    }

    void LUA_THREAD::call(const String &functionName, int *parameters, int nb_params)
    {
        MutexLocker mLocker( pMutex );

        if (running)    return;     // We cannot run several functions at the same time on the same stack

        lua_getglobal( L, functionName.c_str() );
        if (lua_isnil( L, -1 ))     // Function not found
        {
            lua_pop(L, 1);
            return;
        }

        if (parameters == NULL)
            nb_params = 0;
        for(int i = 0 ; i < nb_params ; i++)
            lua_pushinteger(L, parameters[i]);
        n_args = nb_params;
        running = true;
    }

    int LUA_THREAD::execute(const String &functionName, int *parameters, int nb_params)
    {
        MutexLocker mLocker( pMutex );

        lua_getglobal( L, functionName.c_str() );
        if (lua_isnil( L, -1 ))     // Function not found
        {
            lua_pop(L, 1);
            return 0;
        }

        if (parameters == NULL)
            nb_params = 0;
        for(int i = 0 ; i < nb_params ; i++)
            lua_pushinteger(L, parameters[i]);
        try
        {
            if (lua_pcall( L, nb_params, 1, 0))
            {
                if (lua_tostring(L, -1) != NULL && strlen(lua_tostring(L, -1)) > 0)
                    LOG_ERROR(LOG_PREFIX_LUA << lua_tostring(L, -1));
                running = false;
                return -1;
            }
        }
        catch(...)
        {
            if (lua_tostring( L, -1 ) != NULL && strlen(lua_tostring( L, -1 )) > 0)
                LOG_ERROR(LOG_PREFIX_LUA << lua_tostring(L, -1));
            running = false;
            return -1;
        }

        int result = (int) lua_tointeger( L, -1 );          // Read the result
        lua_pop( L, 1 );
        return result;
    }
}
