#include "../script.h"
#include "../../private/script/lua.proxy.h"


namespace Yuni
{
namespace Private
{
namespace ScriptImpl
{
namespace Bind
{



	int Argument<Script::Lua*, int>::Get(Script::Lua* ctx, unsigned int order)
	{
		return (int) lua_tointeger(ctx->pProxy->pState, order + 1);
	}


	bool Argument<Script::Lua*, bool>::Get(Script::Lua* ctx, unsigned int order)
	{
		return (0 != lua_toboolean(ctx->pProxy->pState, order + 1));
	}



	double Argument<Script::Lua*, double>::Get(Script::Lua* ctx, unsigned int order)
	{
		return lua_tonumber(ctx->pProxy->pState, order + 1);
	}


	String Argument<Script::Lua*, String>::Get(Script::Lua* ctx, unsigned int order)
	{
		return lua_tostring(ctx->pProxy->pState, order + 1);
	}


	void* Argument<Script::Lua*, void*>::Get(Script::Lua* ctx, unsigned int order)
	{
		return lua_touserdata(ctx->pProxy->pState, order + 1);
	}


	void Argument<Script::Lua*, int>::Push(Script::Lua* ctx, int value)
	{
		::lua_pushinteger(ctx->pProxy->pState, value);
	}


	void Argument<Script::Lua*, double>::Push(Script::Lua* ctx, double value)
	{
		::lua_pushnumber(ctx->pProxy->pState, value);
	}


	void Argument<Script::Lua*, void*>::Push(Script::Lua* ctx, void* value)
	{
		::lua_pushlightuserdata(ctx->pProxy->pState, value);
	}


	void Argument<Script::Lua*, String>::Push(Script::Lua* ctx, const String& value)
	{
		::lua_pushstring(ctx->pProxy->pState, value.c_str());
	}

	
	void Argument<Script::Lua*, bool>::Push(Script::Lua* ctx, bool value)
	{
		::lua_pushboolean(ctx->pProxy->pState, value);
	}




} // namespace Bind
} // namespace ScriptImpl
} // namespace Private
} // namespace Yuni
