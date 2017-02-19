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

/*-----------------------------------------------------------------------------\
  |                                     ai.cpp                                   |
  |       Ce module est responsable de l'intelligence artificielle               |
  |                                                                              |
  \-----------------------------------------------------------------------------*/

#include "ai.h"
#include <misc/matrix.h>
#include <TA3D_NameSpace.h>
#include <ta3dbase.h>
#include <scripts/cob.h>             // To read and execute scripts
#include <EngineClass.h>
#include <UnitEngine.h>
#include <misc/paths.h>
#include <misc/math.h>
#include <logs/logs.h>
#include <ingame/players.h>




namespace TA3D
{
	//#define AI_DEBUG

	AI_PLAYER::AI_PLAYER()
		:ID(0)
	{
	}

	AI_PLAYER::~AI_PLAYER()
	{
	}


	void AI_PLAYER::stop()
	{
		if (ai_controller)
			ai_controller->destroyThread();
		if (ai_script)
			ai_script->destroyThread();
	}

	void AI_PLAYER::destroy()
	{
		ai_controller = NULL;
        ai_script = NULL;
		ID = 0;
	}

	void AI_PLAYER::monitor()
	{
		if (ai_controller)
			ai_controller->monitor();
		if (ai_script)
			ai_script->monitor();
	}

	void AI_PLAYER::setType(int type)
	{
		this->type = type;

		switch(type)
		{
			case AI_TYPE_EASY:
			case AI_TYPE_MEDIUM:
			case AI_TYPE_HARD:
			case AI_TYPE_BLOODY:
                ai_script = NULL;
				if (!ai_controller)
					ai_controller = new AiController();
				ai_controller->setPlayerID( ID );
				break;
			case AI_TYPE_LUA:
				ai_controller = NULL;
				if (!ai_script)
                    ai_script = new AiScript();
				ai_script->setPlayerID( ID );
				break;
		};
	}


	void AI_PLAYER::setPlayerID(int id)
	{
		ID = id;
		if (ai_script)
			ai_script->setPlayerID(id);
		if (ai_controller)
			ai_controller->setPlayerID(id);
	}


	int AI_PLAYER::getPlayerID()
	{
		if (ai_script)
			return ai_script->getPlayerID();
		if (ai_controller)
			return ai_controller->getPlayerID();
		return ID;
	}

	void AI_PLAYER::changeName(const String& newName)		// Change AI's name (-> creates a new file)
	{
		name = newName;
		if (ai_script)
			ai_script->changeName(newName);
		if (ai_controller)
			ai_controller->changeName(newName);
	}


	void AI_PLAYER::save()
	{
		if (ai_script)
			ai_script->save();
		if (ai_controller)
			ai_controller->save();
	}


	void AI_PLAYER::load(const String& filename, const int id)
	{
		if (ai_script)
			ai_script->loadAI(filename, id);
		if (ai_controller)
			ai_controller->loadAI(filename, id);
	}


	String::Vector AI_PLAYER::getAvailableAIs()
	{
		String::Vector l_AI;
		VFS::Instance()->getFilelist("scripts/ai/*.lua", l_AI);

		for (String::Vector::iterator i = l_AI.begin() ; i != l_AI.end() ; ++i)
		{
			LOG_DEBUG(LOG_PREFIX_AI << "AI script found : " << *i);
			*i = String("[LUA] ") << Paths::ExtractFileNameWithoutExtension(*i);
		}

		l_AI.push_back("[C] EASY");
		l_AI.push_back("[C] MEDIUM");
		l_AI.push_back("[C] HARD");
		l_AI.push_back("[C] BLOODY");

		return l_AI;
	}


	void AI_PLAYER::setAI(const String &AI)
	{
		this->AI = AI;

		if (AI == "[C] EASY")
			setType( AI_TYPE_EASY );
		else if (AI == "[C] MEDIUM")
			setType( AI_TYPE_MEDIUM );
		else if (AI == "[C] HARD")
			setType( AI_TYPE_HARD );
		else if (AI == "[C] BLOODY")
			setType( AI_TYPE_BLOODY );
		else
			setType( AI_TYPE_LUA );

		if (ai_script && AI.size() > 6)
		{
			String filename;
			filename << "scripts/ai/" << Substr(AI, 6, AI.size() - 6) << ".lua";
			ai_script->load(filename);
		}
	}




} // namespace TA3D

