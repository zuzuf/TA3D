#ifndef __YUNI_SCRIPT_LUA_ARGS_HXX__
# define __YUNI_SCRIPT_LUA_ARGS_HXX__


namespace Yuni
{
namespace Private
{
namespace ScriptImpl
{
namespace Bind
{


	// Lua - int
	template<>
	struct Argument<Script::Lua*, int>
	{
		static int Get(Script::Lua* ctx, unsigned int order);
		static void Push(Script::Lua* ctx, int value);
	};

	// Lua - Bool
	template<>
	struct Argument<Script::Lua*, bool>
	{
		static bool Get(Script::Lua* ctx, unsigned int order);
		static void Push(Script::Lua* ctx, bool value);
	};


	// Lua - double
	template<>
	struct Argument<Script::Lua*, double>
	{
		static double Get(Script::Lua* ctx, unsigned int order);
		static void Push(Script::Lua* ctx, double value);
	};

	// Lua - Yuni::String
	template<>
	struct Argument<Script::Lua*, String>
	{
		static String Get(Script::Lua* ctx, unsigned int order);
		static void Push(Script::Lua* ctx, const String& value);
	};

	// Lua - void*
	template<>
	struct Argument<Script::Lua*, void*>
	{
		static void* Get(Script::Lua* ctx, unsigned int order);
		static void Push(Script::Lua* ctx, void* value);
	};




} // namespace Bind
} // namespace ScriptImpl
} // namespace Private
} // namespace Yuni


#endif // __YUNI_SCRIPT_LUA_ARGS_HXX__
