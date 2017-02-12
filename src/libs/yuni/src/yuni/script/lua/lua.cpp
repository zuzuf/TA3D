
#include "lua.h"
#include "../../private/script/lua.proxy.h"
#include <iostream>


// Defines for call() and bind()
#include "../script.defines.h"
#include "../../private/script/script.defines.h"


namespace Yuni
{
namespace Script
{



	Lua::Lua()
		:pEvalPending(0)
	{
		this->pProxy = new Private::ScriptImpl::LuaProxy();
		this->pProxy->pState = luaL_newstate();
		luaL_openlibs(this->pProxy->pState);
	}


	Lua::~Lua()
	{
		lua_close(this->pProxy->pState);
		delete this->pProxy;
	}


	void Lua::reset()
	{
		lua_close(this->pProxy->pState);
		this->pProxy->pState = luaL_newstate();
		luaL_openlibs(this->pProxy->pState);
		pEvalPending = 0;
	}


	/*
	* TODO: Use Events to emit proper signals.
	*/
	bool Lua::appendFromFile(const String& file)
	{
		const int result = luaL_loadfile(pProxy->pState, file.c_str());
		switch (result)
		{
			case LUA_ERRSYNTAX:
				// Emit errsyntax, return false
				std::cout << "Lua: Syntax error" << std::endl;
				return false;
				break;
			case LUA_ERRMEM:
				// Emit insufficient memory, return false
				std::cout << "Lua: Insufficient memory" << std::endl;
				return false;
				break;
			case LUA_ERRFILE:
				// Emit file/permission errors, return false
				std::cout << "Lua: File access error" << std::endl;
				return false;
				break;
			default:
				++pEvalPending;
				// We must move the script that is now on top of the stack behind
				// other scripts already loaded. (hence the -(int)pEvalPending)
				if (pEvalPending > 1)
					lua_insert(pProxy->pState, -(int)pEvalPending);
				// Ok
				break;
		}
		return true;
	}


	bool Lua::appendFromString(const String& script)
	{
		const int result = luaL_loadstring(pProxy->pState, script.c_str());
		switch (result)
		{
			case LUA_ERRSYNTAX:
				// Emit errsyntax, return false
				std::cout << "Lua: Syntax error" << std::endl;
				return false;
				break;
			case LUA_ERRMEM:
				// Emit insufficient memory, return false
				std::cout << "Lua: Insufficient memory" << std::endl;
				return false;
				break;
			default:
				++pEvalPending;
				// We must move the script that is now on top of the stack behind
				// other scripts already loaded. (hence the -(int)pEvalPending)
				if (pEvalPending > 1)
					lua_insert(pProxy->pState, -(int)pEvalPending);
				// Ok
				break;
		}
		return true;
	}


	bool Lua::appendFromBuffer(const char * scriptBuf, const unsigned int scriptSize)
	{
		const int result = luaL_loadbuffer(pProxy->pState, scriptBuf, scriptSize, "<unnamed_buffer>");
		switch (result)
		{
			case LUA_ERRSYNTAX:
				// Emit errsyntax, return false
				std::cout << "Lua: Syntax error" << std::endl;
				return false;
				break;
			case LUA_ERRMEM:
				// Emit insufficient memory, return false
				std::cout << "Lua: Insufficient memory" << std::endl;
				return false;
				break;
			default:
				++pEvalPending;
				// We must move the script that is now on top of the stack behind
				// other scripts already loaded. (hence the -(int)pEvalPending)
				if (pEvalPending > 1)
					lua_insert(pProxy->pState, -(int)pEvalPending);
				// Ok
				break;
		}
		return true;
	}


	bool Lua::prepare()
	{
		while (pEvalPending > 0)
		{
			--pEvalPending;
			if (lua_pcall(pProxy->pState, 0, 0, 0) != 0)
			{
				size_t len;
				std::cout << lua_tolstring(pProxy->pState, lua_gettop(pProxy->pState), &len) << std::endl;
				lua_pop(pProxy->pState, 1);
				return false;
			}
		}
		return true;
	}


	int Lua::popReturnValues(int args, Any* poppedValues)
	{
		// For now, return 0.
		return 0;

		if (0 == args)
			return 0;

		if (!poppedValues) /* We don't care, pop all values at once */
		{
			lua_pop(pProxy->pState, args);
			return args;
		}

		// We have only one value, output it directly.
		if (1 == args)
		{
			Any& result = (*poppedValues);
			switch (lua_type(pProxy->pState, lua_gettop(pProxy->pState)))
			{
				case LUA_TNIL:
					result = Any(); /* Empty variant */
					break;
				case LUA_TNUMBER:
					result = lua_tonumber(pProxy->pState, lua_gettop(pProxy->pState));
					break;
				case LUA_TBOOLEAN:
					result = lua_toboolean(pProxy->pState, lua_gettop(pProxy->pState));
					break;
				case LUA_TSTRING:
					result = lua_tostring(pProxy->pState, lua_gettop(pProxy->pState));
					break;
				case LUA_TLIGHTUSERDATA:
					result = lua_tonumber(pProxy->pState, lua_gettop(pProxy->pState));
					break;
				default:
					/* So we did not convert the result. Output an empty Any. */
					result = Any();
					std::cerr << "Lua: Return type not implemented." << std::endl;
			}
			lua_pop(pProxy->pState, 1);
		}
		// FIXME !!!!
		return 1;
	}


	bool Lua::push(const Any& var)
	{
		lua_checkstack(pProxy->pState, 1);

		/* 32-bit integers */
#define YUNI_IMPL_LPI(type) \
		if (var.is<type>()) \
		{ \
			lua_pushinteger(pProxy->pState, static_cast<lua_Integer>(var.to<type>())); \
			return true; \
		}

		// Unsigned integers
		YUNI_IMPL_LPI(uint8)
		YUNI_IMPL_LPI(uint16)
		YUNI_IMPL_LPI(uint32)
		YUNI_IMPL_LPI(uint64)

		// Signed integers
		YUNI_IMPL_LPI(sint8)
		YUNI_IMPL_LPI(sint16)
		YUNI_IMPL_LPI(sint32)
		YUNI_IMPL_LPI(sint64)

		// Floating point numbers 
		if (var.is<double>())
		{
			lua_pushnumber(pProxy->pState, static_cast<lua_Number>(var.to<double>()));
			return true;
		}
		if (var.is<float>())
		{
			lua_pushnumber(pProxy->pState, static_cast<lua_Number>(var.to<float>()));
			return true;
		}

		// Strings
		if (var.is<String>()) /* String type encompasses also C Strings. See Any impl. */
		{
			const String &str = var.to<String>();
			lua_pushlstring(pProxy->pState, str.c_str(), str.size());
			return true;
		}
		if (var.is<std::string>())
		{
			const std::string &str = var.to<std::string>();
			lua_pushlstring(pProxy->pState, str.c_str(), str.size());
			return true;
		}

		// Light User data
		if (var.is<void*>())
		{
			lua_pushlightuserdata(pProxy->pState, var.to<void*>());
			return true;
		}

		// Nil values
		if (var.empty())
		{
			lua_pushnil(pProxy->pState);
			return true;
		}

		return false;
	}


	/*
	* Implementation of the call() method variants.
	* This is done via macros, because it's very repetitive and
	* will be unmaintainable otherwise. The drawback is that it's a little
	* harder to understand.
	*
	* In the first macro below, _PART1, we get the method on the stack.
	* If the stack did not move, the method does not exist, so we bail out.
	*
	* Then, we push, via the _PUSH_ARG macro, every argument given to our
	* function (they are named argX by _X_ANYS macros).
	*
	* Finally, we make the call and catch any errors with _PART2.
	*
	* TODO: implement the return value handling.
	*/

# define YUNI_SCRIPT_LUA_DEFINE_CALL_PART1 \
	\
		if (pEvalPending && !this->prepare()) \
			return false; /* A script runtime error occured while evaluating loaded chunks. */ \
		int argc = 0; \
		int stackTop = lua_gettop(pProxy->pState); \
		lua_getfield(pProxy->pState, LUA_GLOBALSINDEX, method.c_str()); \
		int stackPos = lua_gettop(pProxy->pState); \
		if (stackTop == stackPos) /* Nothing has been pushed on the stack ? Strange. */ \
		{ \
			return false; /* The call can't succeed, we have nothing to call. */ \
		}



# define YUNI_SCRIPT_LUA_DEFINE_CALL_PART2 \
		if (lua_pcall(pProxy->pState, argc, LUA_MULTRET, 0) != 0) \
		{ \
			size_t len; \
			std::cout << lua_tolstring(pProxy->pState, lua_gettop(pProxy->pState), &len) << std::endl; \
			lua_pop(pProxy->pState, 1); \
			return false; \
		} \
		/* The call succeeded. Pop any values still hanging on the stack. */ \
		stackPos = lua_gettop(pProxy->pState); \
		this->popReturnValues(stackPos - stackTop, retValues); \
		return true;



# define YUNI_SCRIPT_LUA_PUSH_ARG(arg) \
		argc += (this->push(arg) ? 1 : 0);

	YUNI_SCRIPT_SCRIPT_DEFINE_CALL_WITH(Lua)
	{
		YUNI_SCRIPT_LUA_DEFINE_CALL_PART1
		YUNI_SCRIPT_LUA_DEFINE_CALL_PART2
	}

	YUNI_SCRIPT_SCRIPT_DEFINE_CALL_WITH(Lua, YUNI_SCRIPT_SCRIPT_1_ANY)
	{
		YUNI_SCRIPT_LUA_DEFINE_CALL_PART1
		YUNI_SCRIPT_LUA_PUSH_ARG(arg1)
		YUNI_SCRIPT_LUA_DEFINE_CALL_PART2
	}

	YUNI_SCRIPT_SCRIPT_DEFINE_CALL_WITH(Lua, YUNI_SCRIPT_SCRIPT_2_ANYS)
	{
		YUNI_SCRIPT_LUA_DEFINE_CALL_PART1
		YUNI_SCRIPT_LUA_PUSH_ARG(arg1)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg2)
		YUNI_SCRIPT_LUA_DEFINE_CALL_PART2
	}

	YUNI_SCRIPT_SCRIPT_DEFINE_CALL_WITH(Lua, YUNI_SCRIPT_SCRIPT_3_ANYS)
	{
		YUNI_SCRIPT_LUA_DEFINE_CALL_PART1
		YUNI_SCRIPT_LUA_PUSH_ARG(arg1)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg2)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg3)
		YUNI_SCRIPT_LUA_DEFINE_CALL_PART2
	}

	YUNI_SCRIPT_SCRIPT_DEFINE_CALL_WITH(Lua, YUNI_SCRIPT_SCRIPT_4_ANYS)
	{
		YUNI_SCRIPT_LUA_DEFINE_CALL_PART1
		YUNI_SCRIPT_LUA_PUSH_ARG(arg1)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg2)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg3)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg4)
		YUNI_SCRIPT_LUA_DEFINE_CALL_PART2
	}

	YUNI_SCRIPT_SCRIPT_DEFINE_CALL_WITH(Lua, YUNI_SCRIPT_SCRIPT_5_ANYS)
	{
		YUNI_SCRIPT_LUA_DEFINE_CALL_PART1
		YUNI_SCRIPT_LUA_PUSH_ARG(arg1)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg2)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg3)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg4)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg5)
		YUNI_SCRIPT_LUA_DEFINE_CALL_PART2
	}


	YUNI_SCRIPT_SCRIPT_DEFINE_CALL_WITH(Lua, YUNI_SCRIPT_SCRIPT_6_ANYS)
	{
		YUNI_SCRIPT_LUA_DEFINE_CALL_PART1
		YUNI_SCRIPT_LUA_PUSH_ARG(arg1)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg2)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg3)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg4)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg5)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg6)
		YUNI_SCRIPT_LUA_DEFINE_CALL_PART2
	}

	YUNI_SCRIPT_SCRIPT_DEFINE_CALL_WITH(Lua, YUNI_SCRIPT_SCRIPT_7_ANYS)
	{
		YUNI_SCRIPT_LUA_DEFINE_CALL_PART1
		YUNI_SCRIPT_LUA_PUSH_ARG(arg1)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg2)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg3)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg4)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg5)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg6)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg7)
		YUNI_SCRIPT_LUA_DEFINE_CALL_PART2
	}

	YUNI_SCRIPT_SCRIPT_DEFINE_CALL_WITH(Lua, YUNI_SCRIPT_SCRIPT_8_ANYS)
	{
		YUNI_SCRIPT_LUA_DEFINE_CALL_PART1
		YUNI_SCRIPT_LUA_PUSH_ARG(arg1)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg2)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg3)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg4)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg5)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg6)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg7)
		YUNI_SCRIPT_LUA_PUSH_ARG(arg8)
		YUNI_SCRIPT_LUA_DEFINE_CALL_PART2
	}


#undef YUNI_SCRIPT_LUA_PUSH_ARG
#undef YUNI_SCRIPT_LUA_DEFINE_CALL_PART1
#undef YUNI_SCRIPT_LUA_DEFINE_CALL_PART2



	int Lua::callbackProxy(void* luaContext)
	{
		// We got the raw lua context as a void*, because we do not want to
		// include it when Yuni is used in a program.
		lua_State *state = static_cast<lua_State*>(luaContext);

		// Start by popping back from the lua stack our pointers.
		Lua *This = static_cast<Lua *>(lua_touserdata(state, lua_upvalueindex(1)));
		Private::ScriptImpl::Bind::IBinding *shell =
			static_cast<Private::ScriptImpl::Bind::IBinding*>(lua_touserdata(state, lua_upvalueindex(2)));

		// If we're in luck, we now have good pointers.

		// Let the shell we got Call the method and return the result count.
		// Lua must indeed know how many arguments should be popped.
		return shell->performFunctionCall(This);
	}


	/*
	 * @todo Temp
	 */
	bool Lua::internalBindWL(const char* name, Private::ScriptImpl::Bind::IBinding* func)
	{
		// Push on the stack pointers to us and to the bound function wrapper.
		// The bound function wrapper will know what and how to call,
		// but it will need the context data.
		lua_pushlightuserdata(pProxy->pState, static_cast<void *>(this));
		lua_pushlightuserdata(pProxy->pState, static_cast<void *>(func));

		// Push the address of our "callback proxy", which will know how
		// to decode the call, and perform the C++ call.
		lua_pushcclosure(pProxy->pState, reinterpret_cast<lua_CFunction>(&(Lua::callbackProxy)),
						 2 /* 2 arguments in the closure */);

		// Finally assign the closure to its name in the script context
		lua_setfield(pProxy->pState, LUA_GLOBALSINDEX, name);

		// TODO What if we do not succeed ?
		return true;
	}


	void Lua::internalUnbindWL(const char* name)
	{
		// TODO: Implement this method.
		std::cout << "Lua internal unbind WL was called for function [" << name << std::endl;
	}




} // namespace Script
} // namespace Yuni

#include "../../private/script/script.undefs.h"
#include "../script.undefs.h"

