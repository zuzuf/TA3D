#ifndef __TA3D_SCRIPTS_LUA_CHUNK_HXX__
# define __TA3D_SCRIPTS_LUA_CHUNK_HXX__
	


namespace TA3D
{

	inline const QString& LuaChunk::getName() const
	{
		return name;
	}


	inline LuaChunk::LuaChunk(lua_State *L, const QString& name)
		:buffer(NULL), size(0)
	{
		dump(L, name);
	}


	inline LuaChunk::LuaChunk()
		:buffer(NULL), size(0)
	{}

	inline LuaChunk::~LuaChunk()
	{
		DELETE_ARRAY(buffer);
	}



} // namespace TA3D

#endif // __TA3D_SCRIPTS_LUA_CHUNK_HXX__
