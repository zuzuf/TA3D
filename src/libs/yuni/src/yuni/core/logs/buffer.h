#ifndef __YUNI_CORE_LOGS_STREAM_H__
# define __YUNI_CORE_LOGS_STREAM_H__

# include "../../yuni.h"
# include "../customstring.h"


namespace Yuni
{
namespace Private
{
namespace LogImpl
{

	// Forward declaration
	template<class LogT, class V, int E = V::enabled> class Buffer;




	/*!
	** \brief The buffer for the message
	**
	** \internal This is an intermediate class that handles a temporary buffer where
	** the message can be built. The message will be dispatched to the static list
	** of handlers when this class is destroyed. The method `internalFlush()` is called
	** , which ensures thread-safety (if required) while the message is passing through
	** the handlers.
	**
	** \tparam V The verbosity level of the message
	** \tparam E A non-zero value if the message must be managed
	*/
	template<class LogT, class V, int E>
	class Buffer
	{
	public:
		//! Type of the calling logger
		typedef LogT LoggerType;

	public:
		//! \name Constructos & Destructor
		//@{
		inline Buffer(LoggerType& l)
			:pLogger(l)
		{}

		template<typename U>
		inline Buffer(LoggerType& l, U u)
			:pLogger(l)
		{
			pBuffer.append(u);
		}

		~Buffer()
		{
			// Dispatching the messages to the handlers
			// For example, the buffer will be written to the output
			pLogger.template dispatchMessageToHandlers<V>(pBuffer);
		}
		//@}

		template<typename U> Buffer& operator << (const U& u)
		{
			// Appending the piece of message to the buffer
			pBuffer.append(u);
			return *this;
		}

		void appendFormat(const char f[], ...)
		{
			va_list parg;
			va_start(parg, f);
			pBuffer.vappendFormat(f, parg);
			va_end(parg);
		}

		void vappendFormat(const char f[], va_list parg)
		{
			pBuffer.vappendFormat(f, parg);
		}


	private:
		//! Reference to the original logger
		LoggerType& pLogger;

		/*!
		** \brief Buffer that contains the message
		**
		** The chunk size can not be merely the default one; Log entries often
		** contain path of filename for example.
		*/
		Yuni::CustomString<FILENAME_MAX, false, true> pBuffer;

	}; // class Buffer






	// Specialization when a verbosty level is disabled
	template<class LogT, class V>
	class Buffer<LogT,V,0>
	{
	public:
		//! Type of the calling logger
		typedef LogT LoggerType;

	public:
		Buffer(LoggerType&) {}

		template<typename U> Buffer(LoggerType&, U) {}

		~Buffer()
		{}

		template<typename U> Buffer& operator << (const U&)
		{
			// Do nothing - Disabled
			return *this;
		}

		void appendFormat(const char [], ...)
		{
			// Do nothing
		}

		void vappendFormat(const char [], va_list)
		{
			// Do nothing
		}

	}; // class Buffer





} // namespace LogImpl
} // namespace Private
} // namespace Yuni

#endif // __YUNI_CORE_LOGS_STREAM_H__
