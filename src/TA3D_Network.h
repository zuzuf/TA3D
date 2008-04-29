/*  TA3D, a remake of Total Annihilation
    Copyright (C) 2006  Roland BROCHARD

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
    
#ifndef __TA3D__TA3D__NETWORK__H
#define __TA3D__TA3D__NETWORK__H

#include "gui.h"
#include "ta3dbase.h"

class TA3DNetwork : protected cCriticalSection
{
private:
	class NetworkMessage
	{
	public:
		String	text;
		uint32	timer;
		
		inline NetworkMessage( const String &m, uint32 t )	{	text = m;	timer = t;	}
		inline ~NetworkMessage()	{	text.clear();	}
	};

	List< NetworkMessage >		messages;
	bool						enter;
	AREA						*area;
	GAME_DATA					*game_data;

public:

	TA3DNetwork( AREA *area, GAME_DATA *game_data );
	~TA3DNetwork();

	void check();
	void draw();

	bool isLocal( int player_id );
	bool isRemoteHuman( int player_id );
};

extern TA3DNetwork *g_ta3d_network;

#endif
