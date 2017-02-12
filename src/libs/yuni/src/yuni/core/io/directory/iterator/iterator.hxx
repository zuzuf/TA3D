#ifndef __YUNI_CORE_IO_DIRECTORY_ITERATOR_ITERATOR_HXX__
# define __YUNI_CORE_IO_DIRECTORY_ITERATOR_ITERATOR_HXX__

# include "../../../traits/cstring.h"
# include "../../../traits/length.h"
# include "../../../static/remove.h"
# include "../../io.h"




namespace Yuni
{
namespace Core
{
namespace IO
{
namespace Directory
{

	# ifndef YUNI_NO_THREAD_SAFE
	template<bool DetachedT>
	inline IIterator<DetachedT>::DetachedThread::DetachedThread()
	{}

	template<bool DetachedT>
	inline IIterator<DetachedT>::DetachedThread::~DetachedThread()
	{
		stop();
	}
	# endif



	template<bool DetachedT>
	inline IIterator<DetachedT>::IIterator()
		# ifndef YUNI_NO_THREAD_SAFE
		:pThread(NULL)
		# endif
	{
	}

	template<bool DetachedT>
	inline IIterator<DetachedT>::IIterator(const IIterator& rhs)
		# ifndef YUNI_NO_THREAD_SAFE
		:pThread(NULL)
		# endif
	{
		typename ThreadingPolicy::MutexLocker locker(rhs);
		pRootFolder = rhs.pRootFolder;
	}

	template<bool DetachedT>
	IIterator<DetachedT>& IIterator<DetachedT>::operator = (const IIterator<DetachedT>& rhs)
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		typename ThreadingPolicy::MutexLocker locker2(rhs);
		rhs.pRootFolder = rhs.pRootFolder;
	}


	template<bool DetachedT>
	inline IIterator<DetachedT>::~IIterator()
	{
		# ifndef YUNI_NO_THREAD_SAFE
		stop();
		if (pThread)
			delete pThread;
		# endif
	}


	template<bool DetachedT>
	inline void IIterator<DetachedT>::clear()
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		pRootFolder.clear();
	}



	template<bool DetachedT>
	template<class StringT>
	inline void IIterator<DetachedT>::add(const StringT& folder)
	{
		typename ThreadingPolicy::MutexLocker locker(*this);
		// Copy
		String s;
		// We must have absolute paths
		if (Core::IO::IsRelative(folder))
		{
			Core::IO::Directory::Current::Get(s);
			if (!s.empty() && s.last() != '/' && s.last() != '\\')
				s += Core::IO::Separator;
		}
		s += folder;

		# ifdef YUNI_OS_WINDOWS
		// The Win32 API does not really like `/`
		s.replace('/', '\\');
		# endif


		// Pushing it into the list
		String* item = new String();
		item->reserve(s.size());
		Core::IO::Normalize(*item, s);

		// Remove the last slash
		if (item->last() == '/' || item->last() == '\\')
		{
			if (!(*item == "/" || (item->size() == 3 && (*item)[1] == ':')))
				item->removeLast();
		}
		pRootFolder.push_back(item);
	}


	template<bool DetachedT>
	bool IIterator<DetachedT>::start()
	{
		# ifndef YUNI_NO_THREAD_SAFE
		if (detached)
		{
			// Lock
			typename ThreadingPolicy::MutexLocker locker(*this);

			// Early detection of an invalid root folder
			if (pRootFolder.empty())
				return false;
			if (!pThread)
				pThread = new DetachedThread();
			else
			{
				// Do nothing if already started
				if (pThread->started())
					return false;
			}

			// Providing a reference to ourselves for events
			pThread->options.self = this;
			pThread->options.rootFolder = pRootFolder;

			// Starting the thread
			return (Thread::IThread::errNone == pThread->start());
		}
		else
		# endif
		{
			using namespace Yuni::Private::Core::IO::Directory::Iterator;
			Options opts;
			{
				// Lock
				typename ThreadingPolicy::MutexLocker locker(*this);

				// Early detection of an invalid root folder
				if (pRootFolder.empty())
					return false;
				// Providing a reference to ourselves for events
				opts.self = this;
				opts.rootFolder = pRootFolder;
			}

			// The calling thread will block until the traversing is complete
			Traverse(opts, NULL);
		}
		return true;
	}


	template<bool DetachedT>
	bool IIterator<DetachedT>::stop(unsigned int timeout)
	{
		(void) timeout; // to avoid compiler warning
		# ifndef YUNI_NO_THREAD_SAFE
		if (detached)
		{
			typename ThreadingPolicy::MutexLocker locker(*this);
			return (pThread)
				? (Yuni::Thread::IThread::errNone == pThread->stop(timeout))
				: false;
		}
		# endif
		return true;
	}


	template<bool DetachedT>
	inline void IIterator<DetachedT>::wait()
	{
		# ifndef YUNI_NO_THREAD_SAFE
		if (detached)
		{
			// Lock
			typename ThreadingPolicy::MutexLocker locker(*this);
			if (pThread)
				pThread->wait();
		}
		# endif
	}


	template<bool DetachedT>
	inline void IIterator<DetachedT>::wait(uint32 timeout)
	{
		# ifndef YUNI_NO_THREAD_SAFE
		if (detached)
		{
			// Lock
			typename ThreadingPolicy::MutexLocker locker(*this);
			if (pThread)
				pThread->wait(timeout);
		}
		# else
		(void) timeout;
		# endif
	}



	template<bool DetachedT>
	inline Flow
	IIterator<DetachedT>::onBeginFolder(const String&, const String&, const String&)
	{
		// Do nothing
		return flowContinue;
	}

	template<bool DetachedT>
	inline void
	IIterator<DetachedT>::onEndFolder(const String&, const String&, const String&)
	{
		// Do nothing
	}

	template<bool DetachedT>
	inline Flow
	IIterator<DetachedT>::onFile(const String&, const String&, const String&, uint64)
	{
		// Do nothing
		return flowContinue;
	}

	template<bool DetachedT>
	inline Flow
	IIterator<DetachedT>::onError(const String&)
	{
		// Do nothing
		return flowContinue;
	}


	template<bool DetachedT>
	inline Flow
	IIterator<DetachedT>::onAccessError(const String&)
	{
		return flowContinue;
	}


	template<bool DetachedT>
	inline void IIterator<DetachedT>::onAbort()
	{
		// Do nothing
	}


	template<bool DetachedT>
	inline void IIterator<DetachedT>::onTerminate()
	{
		// Do nothing
	}


	template<bool DetachedT>
	inline bool IIterator<DetachedT>::onStart(const String&)
	{
		// Do nothing
		return true;
	}




} // namespace Directory
} // namespace IO
} // namespace Core
} // namespace Yuni

#endif // __YUNI_CORE_IO_DIRECTORY_ITERATOR_ITERATOR_HXX__
