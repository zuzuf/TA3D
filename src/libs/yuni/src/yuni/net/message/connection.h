#ifndef __YUNI_NET_SERVER_CONNECTION_H__
# define __YUNI_NET_SERVER_CONNECTION_H__

# include "../../yuni.h"
# include "../../core/string.h"


namespace Yuni
{
namespace Net
{
namespace Message
{


	class IConnection
	{
	public:
		//! The most suitable smart pointer for the class
		typedef SmartPtr<IConnection>  Ptr;

	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Constructor
		*/
		IConnection(unsigned int id);
		//! Destructor
		virtual ~IConnection();
		//@}

		/*!
		** \brief Get the local unique ID of the connection
		*/
		unsigned int id() const {return pID;}

		/*!
		** \brief Disconnect the client
		*/
		void disconnect();

		/*!
		** \brief Send a buffer
		*/
		template<class StringT> void send(const StringT& data);

		/*!
		** \brief Send a buffer with a custom size
		**
		** \param data Any string
		** \param size The size of the data
		*/
		template<class StringT> void send(const StringT& data, unsigned int size);

	private:
		//! Local Unique ID
		unsigned int pID;
		//! Amount of data (bytes) sent to the client
		uint64 pDataSent;
		//! Amount of data (bytes) received from the client
		uint64 pDataReceived;
		//! Timestamp when the client connected
		sint64 pTimestampCreation;

	}; // class Connection




} // namespace Message
} // namespace Net
} // namespace Yuni

#endif // __YUNI_NET_SERVER_CONNECTION_H__
