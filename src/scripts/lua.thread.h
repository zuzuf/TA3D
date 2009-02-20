#ifndef __LUA_THREAD_H__
#define __LUA_THREAD_H__

# include "../misc/vector.h"
# include "../lua/lua.hpp"
# include "../threads/thread.h"

# ifndef luaL_dobuffer
#  define luaL_dobuffer(L, s, sz) \
    (luaL_loadbuffer(L, (const char*)s, sz, "main" ) || lua_pcall(L, 0, LUA_MULTRET, 0))
# endif

namespace TA3D
{
    /*!
    ** \brief functions for vector interface with Lua
    */
    void lua_pushvector( lua_State *L, const Vector3D &vec );
    Vector3D lua_tovector( lua_State *L, int idx );

    /*!
    ** \brief functions for color interface with Lua
    */
    void lua_pushcolor( lua_State *L, const uint32 color );
    uint32 lua_tocolor( lua_State *L, int idx );

    /*!
    ** \brief loads a Lua script, preprocessing to allow including other files and use defines
    */
    byte *loadLuaFile(const String &filename, uint32 &filesize);

    /*!
    ** This class represents a basic Lua thread without specialization
    ** To use it, create a new class that inherits LUA_THREAD
    */
    class LUA_THREAD
    {
    protected:
        byte        *buffer;
        lua_State   *L;             // The Lua state

        //! Variables to control thread execution
        int         last;           // Last timer check
        bool        running;

        float       sleep_time;     // Time to wait
        bool        sleeping;       // Is the thread paused ?
        bool        waiting;        // Is the thread waiting for some user action ?

    public:

        LUA_THREAD();
        inline ~LUA_THREAD()    {   destroy();  }

        void init();
        void destroy();

        //! stops definitely the thread
        void kill();
        void stop();
        void sleep(float time);
        void resume();

        void load(const String &filename);                    // Load a lua script

        int run(float dt);                   // Run the script
        int run();                           // Run the script, using default delay

        inline bool is_running() { return running; }
        inline bool is_waiting() { return waiting; }
        inline bool is_sleeping() { return sleeping; }

    private:
        //! functions that register new Lua functions
        void register_basic_functions();
        virtual void register_functions()   {}
    };

    /*!
    ** \brief returns a pointer to the current thread, or NULL on error
    */
    LUA_THREAD *lua_threadID( lua_State *L );
}

#endif
