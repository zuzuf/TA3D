#ifndef __TA3D_NET_NETWORK_UTILS_H__
# define __TA3D_NET_NETWORK_UTILS_H__

# include "../stdafx.h"
# include "../threads/thread.h"


namespace TA3D
{

    /*!
    ** /brief Thread which accepts new connections
    */
    class ListenThread : public Thread
    {
        virtual void proc(void* param);
    };


    /*!
    ** \brief Thread: Receives events, syncs, orders, etc
    */
    class SocketThread : public Thread
    {
        virtual void proc(void* param);
    };

    /*!
    ** \brief Thread: Sends a large file in the background
    */
    class SendFileThread : public Thread
    {
        virtual void proc(void* param);
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
    };



    /*!
    ** \brief
    */
    class BroadCastThread : public Thread{
        virtual void proc(void* param);
    };



    /*!
    ** \brief
    */
    class UDPThread : public Thread
    {
        virtual void proc(void* param);
    };



    /*!
    ** \brief
    */
    struct SERVER_DATA
    {
        //! Name of the server
        String name;
        //! Timeout for this server
        int timer;
        //! Count of opened player slots
        int nb_open;
        //! Host name of this server
        String host;
        //! Is an Internet (non-local) server ?
        bool internet;
        //! Which version of TA3D is it running ?
        String version;
    };


    /*!
    ** \brief
    */
    struct FileTransferProgress
    {
        String	id;				// Pointer to transfer thread
        int		size;
        int		pos;
    };


} // namespaceTA3D

#endif // __TA3D_NET_NETWORK_UTILS_H__
