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

#ifndef __AiScript_H__
#define __AiScript_H__

# include <yuni/yuni.h>
# include <misc/string.h>
# include "lua.thread.h"
# include <zuzuf/smartptr.h>


namespace TA3D
{
	/*!
	** \brief 
	**
	** This class represents AI scripts, it's used to script AI behavior
	** This is a mean to implement new AIs without rebuilding the code
	*/
	class AiScript : public LuaThread, public Thread
	{
	public:
		//! The most suitable smartptr for this class
        typedef zuzuf::smartptr<AiScript> Ptr;

	public:

		AiScript();
		virtual ~AiScript();
		void setPlayerID(int id);
		int getPlayerID();
		void setType(int type);
		int getType();

		void changeName(const String& newName);		// Change AI name
		void save();
		void loadAI(const String& filename, const int id);    // Load a saved AI (NB: this is not the same symbol used to load Lua scripts!! so this works :) )

		void monitor();

	protected:
		virtual void proc(void *);

	public:
		/*virtual*/ void register_functions();
		/*virtual*/ void register_info();

	private:
		int         playerID;
		String      name;

	}; // class AiScript





} // namespace TA3D

#endif
