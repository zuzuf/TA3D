/*  TA3D, a remake of Total Annihilation
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/

#include "socketlist.h"


namespace TA3D
{

	SockList::SocketNode::~SocketNode()
	{
		thread.join();
		sock->close();
		delete sock;
	}

	SockList::SockList()
	{
		maxid = 0;
	}

	SockList::~SockList()
	{
		Shutdown();
	}

	void SockList::Shutdown()
	{
		lock();
		for(SockType::iterator i = sockets.begin() ; i != sockets.end() ; ++i)
			delete *i;
		maxid = 0;
		sockets.clear();
		unlock();
	}

	int SockList::Add(TA3DSock* sock)
	{
		MutexLocker mLock(*this);
		if (maxid > 10000) //arbitrary limit
			return -1;
		SocketNode *node = new SocketNode;
		++maxid;
		sockets[maxid] = node;
		node->sock = sock;

		return maxid;
	}

	int SockList::Remove(const int id)
	{
		MutexLocker mLock(*this);
		if (!sockets.contains(id))
			return -1;
		delete sockets[id];
		sockets.remove(id);
		return 0;
	}

	SocketThread *SockList::getThread(const int id)
	{
		MutexLocker mLock(*this);
		SockType::iterator i = sockets.find(id);
		if (i == sockets.end())
			return NULL;
		return &((*i)->thread);
	}

	TA3DSock *SockList::getSock(const int id)
	{
		MutexLocker mLock(*this);
		SockType::iterator i = sockets.find(id);
		if (i == sockets.end())
			return NULL;
		return (*i)->sock;
	}


} // namespace TA3D
