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

#include <stdafx.h>
#include "noscript.h"

namespace TA3D
{
	NoScript::NoScript()
	{
	}

	NoScript::~NoScript()
	{
	}

	void NoScript::setUnitID(uint32)
	{
	}

	int NoScript::getNbPieces()
	{
		return 0;
	}

	void NoScript::load(ScriptData *)
	{
	}

	int NoScript::run(float, bool)                  // Run the script
	{
		return -1;
	}

	//! functions used to call/run functions
	void NoScript::call(const String &, int *, int)
	{
	}

	int NoScript::execute(const String &, int *, int)
	{
		return 0;
	}

	//! functions used to create new threads sharing the same environment
	NoScript *NoScript::fork()
	{
		return NULL;
	}

	NoScript *NoScript::fork(const String &, int *, int)
	{
		return NULL;
	}

	//! functions used to save/restore scripts state
	void NoScript::save_thread_state(gzFile)
	{
	}

	void NoScript::restore_thread_state(gzFile)
	{
	}
}
