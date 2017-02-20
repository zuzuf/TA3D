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

#ifndef __TA3D_NET_NETWORK_UTILS_H__
# define __TA3D_NET_NETWORK_UTILS_H__

# include <stdafx.h>
# include <misc/string.h>
# include <threads/thread.h>


namespace TA3D
{

    /*!
    ** /brief Thread which accepts new connections
    */
    class ListenThread : public Thread
    {
        virtual void proc(void* param);
    public:
        virtual ~ListenThread() {   destroyThread();    }
    };


    /*!
    ** \brief Thread: Receives events, syncs, orders, etc
    */
    class SocketThread : public Thread
    {
        virtual void proc(void* param);
    public:
        virtual ~SocketThread() {   destroyThread();    }
    };

    /*!
    ** \brief Thread: Sends a large file in the background
    */
    class SendFileThread : public Thread
    {
        virtual void proc(void* param);
    public:
        virtual ~SendFileThread()   {   destroyThread();    }
    public :
        int		port;
        int		player_id;
        int		progress;
    };


    /*!
    ** \brief Thread: Gets a large file in the background and writes to disk
    */
    class GetFileThread : public Thread
    {
        virtual void proc(void* param);
    public :
        GetFileThread();
        virtual ~GetFileThread()    {   destroyThread();    }

        int		port;
        byte	*buffer;
        int		buffer_size;
        bool	ready;
    };



    /*!
    ** \brief
    */
    class AdminThread : public Thread
    {
        virtual void proc(void* param);
    public:
        virtual ~AdminThread()  {   destroyThread();    }
    };



    /*!
    ** \brief
    */
    class BroadCastThread : public Thread
    {
        virtual void proc(void* param);
    public:
        virtual ~BroadCastThread()  {   destroyThread();    }
    };



    /*!
    ** \brief
    */
    struct SERVER_DATA
    {
		SERVER_DATA()
			:timer(0), nb_open(0), internet(false)
		{}
        //! Name of the server
        QString name;
        //! Timeout for this server
        int timer;
        //! Count of opened player slots
        int nb_open;
        //! Host name of this server
        QString host;
        //! Is an Internet (non-local) server ?
        bool internet;
    };


    /*!
    ** \brief
    */
    struct FileTransferProgress
    {
        QString	id;				// Pointer to transfer thread
        int		size;
        int		pos;
    };


} // namespaceTA3D

#endif // __TA3D_NET_NETWORK_UTILS_H__
