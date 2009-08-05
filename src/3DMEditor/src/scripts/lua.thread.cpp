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

#include "../gfx.h"
#include "lua.thread.h"
#include "../../../ta3d/src/lua/lstate.h"      // We need this to access lua_State objects to save and load Lua VMs
#include <QtCore>


namespace TA3D
{

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

    QString extractPath(const QString &filename)
    {
        int len = filename.lastIndexOf("/");
        if (len < 0)
            len = filename.size();
        QString filePath = filename.left(len);
        len = filePath.lastIndexOf("\\");
        if (len < 0)
            len = filePath.size();
        return filePath.left(len);
    }

    byte *readFile(const QString &filename, uint32 *filesize)
    {
        QFile file(filename);
        file.open(QFile::ReadOnly);
        QByteArray data = file.readAll();
        byte *buf = new byte[data.length()+1];
        if (filesize)
            *filesize = data.length();
        memcpy(buf, data.data(), data.length());
        buf[data.length()] = 0;
        return buf;
    }

    byte *loadLuaCode(const QString &code, uint32 &filesize)
	{
        filesize = code.size() + 1;
        byte *buffer = new byte[filesize];
        memcpy(buffer, code.toStdString().c_str(), filesize);
		if (buffer)
		{
            QString path = qApp->applicationDirPath() + "/";
            QString name;
			int n = 0;
			char *f = NULL;
			while ((f = strstr( (char*)buffer, "#include" ) ) != NULL && n < 20)
			{
				int i;
				name.clear();
				for( i = 0 ; i < 100 && f[ i + 10 ] != '"' ; i++ )
                    name += f[ i + 10 ];
                if (!QFile::exists(path + name))
					name = "scripts/" + name;
				else
					name = path + name;
				uint32 filesize2 = 0;
                byte *buffer2 = (byte*)readFile(name, &filesize2);
				if (buffer2)
				{
					byte *buffer3 = new byte[ filesize + filesize2 + 1 ];
					memset( buffer3, 0, filesize + filesize2 + 1 );
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
        lua_pushinteger( L, 0 );
		return 1;
	}

	int thread_mouse_y( lua_State *L )		// mouse_y()
	{
        lua_pushinteger( L, 0 );
		return 1;
	}

	int thread_mouse_z( lua_State *L )		// mouse_z()
	{
        lua_pushinteger( L, 0 );
		return 1;
	}

	int thread_mouse_b( lua_State *L )		// mouse_b()
	{
        lua_pushinteger( L, 0 );
		return 1;
	}

	int thread_time( lua_State *L )		// time()
	{
        lua_pushnumber( L, QTime().msecsTo(QTime::currentTime()) * 0.001 );
		return 1;
	}
} // namespace TA3D
