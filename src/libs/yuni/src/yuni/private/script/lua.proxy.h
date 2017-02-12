#ifndef __YUNI_PRIVATE_SCRIPT_LUA_PROXY_H__
# define __YUNI_PRIVATE_SCRIPT_LUA_PROXY_H__

# include <lua.hpp>
# include "../../script/lua/lua.h"
# include "../../yuni.h"


namespace Yuni
{
namespace Private
{
/*!
** \brief Contains internal proxy classes for the Script module.
*/
namespace ScriptImpl
{

	/*!
	** \brief A proxy to avoid lua.h inclusions for user programs
	** while using the Yuni::Script::Lua class.
	**
	** This class should contain every lua-related state variable the
	** Yuni::Script::Lua class may use. Any specific methods can
	** also be put there.
	**
	** \see class Yuni::Script::Lua
	*/
	class LuaProxy
	{
		friend class Script::Lua;
		template <class, class> friend struct Bind::Argument;
	private:
		//! \name Constructor & Destructor
		//@{
		//! Default constructor
		LuaProxy() {}
		//! Destructor
		~LuaProxy() {}
		//@}

		//! \name Proxy storages and variables
		//@{
		//! Lua internal state storage
		lua_State*	pState;
		//@}

	}; // class LuaProxy




} // namespace ScriptImpl
} // namespace Private
} // namespace Yuni

#endif // __YUNI_PRIVATE_SCRIPT_LUA_PROXY_H__
