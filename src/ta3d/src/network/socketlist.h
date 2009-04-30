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

#ifndef __TA3D_NET_SOCKET_LIST_H__
# define __TA3D_NET_SOCKET_LIST_H__

# include "../stdafx.h"
# include "../threads/thread.h"
# include "ta3dsock.h"
# include "networkutils.h"


namespace TA3D
{

/*!
** \brief SockList - a C-style linked list of socket/thread pairs
*/
class SocketListNode
{
public:
	SocketListNode() :next(NULL){}
	~SocketListNode() {sock->close(); thread.join(); delete sock;}
	int id;
	TA3DSock* sock;
	SocketThread thread;
	SocketListNode* next;
};




class SockList
{
private:
    SocketListNode* list;
	int maxid;

public:
	SockList();
	~SockList();

	int getMaxId() {return maxid;}
	int Add(TA3DSock* sock);
	int Remove(const int id);

    /*!
    ** \brief Close all opened sockets
    */
	void Shutdown();

	TA3DSock* getSock(const int id) const;
    SocketThread* getThread(int id) const;

}; // class SockList



} // namespace TA3D

#endif // __TA3D_NET_SOCKET_LIST_H__
