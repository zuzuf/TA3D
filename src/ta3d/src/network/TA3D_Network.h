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

# include <ta3dbase.h>
# include <threads/thread.h>
# include <ingame/gamedata.h>
# include <deque>
# include <gfx/gui/area.h>
# include <misc/string.h>


namespace TA3D
{


	class TA3DNetwork : public ObjectSync
	{
	public:
        typedef zuzuf::smartptr<TA3DNetwork>	Ptr;
	private:
		class NetworkMessage
		{
		public:
			QString	text;
			uint32	timer;

			NetworkMessage(const QString& m, uint32 t): text(m), timer(t) {}
			~NetworkMessage() {text.clear();}
		};

		std::deque<NetworkMessage>	messages;
		bool						enter;
		Gui::AREA					*area;
	public:
		GameData					*game_data;
	private:

		int							signal;

	public:
		TA3DNetwork(Gui::AREA *area, GameData *game_data );
		virtual ~TA3DNetwork();

		void set_signal( int s )	{	signal = s;	}
		int get_signal() const { return signal; }

		void check();
		void draw();

		bool isLocal(const unsigned int id) const;
		bool isRemoteHuman(const unsigned int id) const;
		void sendDamageEvent( int idx, float damage );
		void sendParalyzeEvent( int idx, float damage );
		void sendFeatureCreationEvent( int idx );
		void sendFeatureDeathEvent( int idx );
		void sendFeatureFireEvent( int idx );
		void sendUnitNanolatheEvent( int idx, int target, bool feature, bool reverse );
		int getNetworkID( int unit_id );
	};




	extern TA3DNetwork::Ptr g_ta3d_network;


} // namespace TA3D

#endif
