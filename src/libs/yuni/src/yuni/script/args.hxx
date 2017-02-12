#ifndef __YUNI_SCRIPT_ARGS_HXX__
# define __YUNI_SCRIPT_ARGS_HXX__

# include "../core/static/assert.h"


namespace Yuni
{
namespace Private
{
namespace ScriptImpl
{
namespace Bind
{


	/*!
	** \brief Base class for Argument getters
	**
	** This class is empty, see script.args.language.hxx
	** for specializations.
	** The goal of this class is to get the Nth argument, forced to the
	** C++ type ArgT, for the language ScriptT. If the argument is not
	** compatible with ArgT, it will be set to the value of ArgT()
	**
	** \tparam ScriptT The script engine type (Lua, ...)
	** \tparam ArgT The desired type of the argument.
	*/
	template <class ScriptT, class ArgT>
	struct Argument
	{
		static ArgT Get(ScriptT, unsigned int)
		{
			YUNI_STATIC_ASSERT(false, Script_ArgumentTypeNotImplemented_Get);
		}

		static void Push(ScriptT, ArgT, unsigned int)
		{
			YUNI_STATIC_ASSERT(false, Script_ArgumentTypeNotImplemented_Push);
		}
	};




	template <class ScriptT, class ArgT>
	struct Argument<ScriptT, ArgT&>
	{
		static ArgT Get(ScriptT script, unsigned int arg)
		{
			return Argument<ScriptT,ArgT>::Get(script, arg);
		}

		static void Push(ScriptT script, ArgT arg)
		{
			Argument<ScriptT, ArgT>::Push(script, arg);
		}
	};


	template <class ScriptT, class ArgT>
	struct Argument<ScriptT, const ArgT&>
	{
		static ArgT Get(ScriptT script, unsigned int arg)
		{
			return Argument<ScriptT,ArgT>::Get(script, arg);
		}
		static void Push(ScriptT script, ArgT arg)
		{
			Argument<ScriptT, ArgT>::Push(script, arg);
		}
	};





} // namespace Bind
} // namespace ScriptImpl
} // namespace Private
} // namespace Yuni


// All available specialisations
// Lua
# include "lua/args.hxx"


#endif // __YUNI_SCRIPT_ARGS_HXX__
