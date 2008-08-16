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
	~SocketListNode() {sock->Close(); thread.join(); delete sock;}
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
