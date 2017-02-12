#ifndef __YUNI_CORE_IO_DIRECTORY_ITERATOR_DETACHED_THREAD_H__
# define __YUNI_CORE_IO_DIRECTORY_ITERATOR_DETACHED_THREAD_H__

# include "../../../../thread/thread.h"


namespace Yuni
{
namespace Private
{
namespace Core
{
namespace IO
{
namespace Directory
{
namespace Iterator
{

	// Forward declarations
	class Interface;
	class Options;
	class IDetachedThread;

	typedef Yuni::Core::IO::Flow Flow;

	/*!
	** \brief Platform specific iplementation for traversing a folder
	*/
	void Traverse(Options& options, IDetachedThread* thread);

	Flow TraverseUnixFolder(const String&, Options& options, IDetachedThread* thread, bool files);
	Flow TraverseWindowsFolder(const String&, Options& options, IDetachedThread* thread, bool files);




	class Interface
	{
	public:
		Interface() {}
		virtual ~Interface() {}

	protected:
		virtual bool onStart(const String& root) = 0;

		virtual void onTerminate() = 0;

		virtual void onAbort() = 0;

		virtual Flow onBeginFolder(const String& filename, const String& parent, const String& name) = 0;
		virtual void onEndFolder(const String& filename, const String& parent, const String& name) = 0;

		virtual Flow onFile(const String& filename, const String& parent,
			const String& name, uint64 size) = 0;

		virtual Flow onError(const String& filename) = 0;

		virtual Flow onAccessError(const String& filename) = 0;

	public:
		friend void Traverse(Options&, IDetachedThread*);
		friend Flow TraverseUnixFolder(const String&, Options&, IDetachedThread*, bool);
		friend Flow TraverseWindowsFolder(const String&, Options&, IDetachedThread*, bool);
	}; // class Interface



	class Options
	{
	public:
		/*!
		** \brief The root folder
		** \internal The shared access to this variable is already guaranted
		**   by the class IIterator
		*/
		String::VectorPtr rootFolder;
		//! Pointer to the parent class
		Interface* self;

		# ifdef YUNI_OS_WINDOWS
		wchar_t* wbuffer;
		# endif

		/*!
		** \brief Arbitrary counter to reduce the number of calls to suspend()
		*/
		int counter;
	};


	# ifndef YUNI_NO_THREAD_SAFE
	class IDetachedThread : public Yuni::Thread::IThread
	{
	public:
		IDetachedThread() {}
		virtual ~IDetachedThread() {}

		bool suspend()
		{
			return Yuni::Thread::IThread::suspend();
		}

	public:
		Options options;

	protected:
		virtual bool onExecute()
		{
			Traverse(options, this);
			return false;
		}

	}; // class IDetachedThread

	# else
	class IDetachedThread {};
	# endif




} // namespace Iterator
} // namespace Directory
} // namespace IO
} // namespace Core
} // namespace Private
} // namespace Yuni

#endif // __YUNI_CORE_IO_DIRECTORY_ITERATOR_DETACHED_THREAD_H__
