/*  TA3D, a remake of Total Annihilation
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA*/

#ifndef __TA3D_NET_SOCKET_LIST_H__
# define __TA3D_NET_SOCKET_LIST_H__

# include <stdafx.h>
# include <threads/thread.h>
# include "ta3dsock.h"
# include "networkutils.h"
# include <misc/hash_table.h>
# include <threads/mutex.h>

namespace TA3D
{

	/*!
** \brief SockList - a structure to manage of socket/thread pairs
*/
	class SockList : public Mutex
	{
	private:
		class SocketNode
		{
		public:
			~SocketNode();

			TA3DSock* sock;
			SocketThread thread;
		};
	private:
		typedef UTILS::HashMap<SocketNode*, int>::Dense SockType;
		SockType sockets;
		int maxid;

	public:
		SockList();
		virtual ~SockList();

		int getMaxId() const {	return maxid;	}
		int Add(TA3DSock* sock);
		int Remove(const int id);

		/*!
    ** \brief Close all opened sockets
    */
		void Shutdown();

		TA3DSock* getSock(const int id);
		SocketThread* getThread(const int id);

	}; // class SockList



} // namespace TA3D

#endif // __TA3D_NET_SOCKET_LIST_H__
