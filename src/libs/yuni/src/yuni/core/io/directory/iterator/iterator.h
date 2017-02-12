#ifndef __YUNI_CORE_IO_DIRECTORY_ITERATOR_ITERATOR_H__
# define __YUNI_CORE_IO_DIRECTORY_ITERATOR_ITERATOR_H__

# include "../../../../yuni.h"
# include "../../../string.h"
# include "../../directory.h"
# include "detachedthread.h"



namespace Yuni
{
namespace Core
{
namespace IO
{
namespace Directory
{


	/*!
	** \brief Iterate through subfolders and files of a given directory
	**
	** This class is reentrant. Consequently, it will be thread-safe as long as the
	** user custom implementation is thread-safe.
	**
	** \code
	** #include <yuni/yuni.h>
	** #include <yuni/core/io/directory/iterator.h>
	** #include <iostream>
	**
	** using namespace Yuni;
	**
	**
	** class MyIterator : public Core::IO::Directory::IIterator<true>
	** {
	** public:
	**	//! Flow
	**	typedef Core::IO::Flow Flow;
	**
	** public:
	** 	MyIterator() {}
	** 	virtual ~MyIterator()
	** 	{
	** 		// For code robustness and to avoid corrupt vtable
	** 		stop();
	** 	}
	**
	** protected:
	** 	virtual bool onStart(const String& rootFolder)
	** 	{
	** 		std::cout << " [+] " << rootFolder << std::endl;
	** 		pCounter = 1;
	** 		pFileCount = 0;
	** 		pFolderCount = 0;
	** 		pTotalSize = 0;
	** 		return true;
	** 	}
	**
	** 	virtual Flow onBeginFolder(const String&, const String&, const String& name)
	** 	{
	** 		printSpaces();
	** 		std::cout << " [+] " << name << std::endl;
	** 		++pCounter;
	** 		++pFolderCount;
	** 		return Core::IO::flowContinue;
	** 	}
	**
	**	virtual void onEndFolder(const String&, const String&, const String&)
	**	{
	**		--pCounter;
	**	}
	**
	** 	virtual Flow onFile(const String&, const String&, const String& name, uint64 size)
	** 	{
	** 		printSpaces();
	** 		std::cout << "  -  " << name << " (" << size << " bytes)" << std::endl;
	** 		++pFileCount;
	** 		pTotalSize += size;
	** 		return Core::IO::flowContinue;
	** 	}
	**
	** 	virtual void onTerminate()
	** 	{
	** 		std::cout << "\n";
	**		std::cout << pFolderCount << " folder(s), " << pFileCount << " file(s),  "
	**			<< pTotalSize << " bytes" << std::endl;
	** 	}
	**
	** private:
	** 	void printSpaces()
	** 	{
	** 		for (unsigned int i = 0; i != pCounter; ++i)
	** 			std::cout << "    ";
	** 	}
	**
	** private:
	** 	unsigned int pCounter;
	** 	unsigned int pFolderCount;
	** 	unsigned int pFileCount;
	** 	uint64 pTotalSize;
	** };
	**
	**
	** int main()
	** {
	**		MyIterator iterator;
	**		iterator.add("/tmp");
	**		iterator.start();
	**		iterator.wait();
	**		return 0;
	** }
	** \endcode
	**
	** This class is thread-safe even when not in detached mode.
	** \tparam DetachedT True to perform the operation into a separate thread
	**
	** \internal When yuni is compiled without any threading support
	**    the detached mode is automatically disabled.
	*/
	template<bool DetachedT = true>
	class IIterator
		# ifndef YUNI_NO_THREAD_SAFE
		:public Policy::ObjectLevelLockable< IIterator<DetachedT> >
		# else
		:public Policy::ObjectLevelLockable< IIterator<false> >
		# endif
		,public Private::Core::IO::Directory::Iterator::Interface
	{
	public:
		enum
		{
			//! Detached mode
			# ifndef YUNI_NO_THREAD_SAFE
			detached = DetachedT,
			# else
			detached = false,
			# endif
			//! The default timeout for stopping a thread
			defaultTimeout = Thread::IThread::defaultTimeout,
		};
		//! Itself
		typedef IIterator<detached> IteratorType;
		//! The threading policy
		typedef Policy::ObjectLevelLockable<IteratorType> ThreadingPolicy;


	public:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Default Constructor
		*/
		IIterator();
		/*!
		** \brief Copy constructor
		*/
		IIterator(const IIterator& rhs);
		/*!
		** \brief Destructor
		*/
		virtual ~IIterator();
		//@}


		//! \name Search paths
		//@{
		/*!
		** \brief Clear the list of path
		*/
		void clear();

		/*!
		** \brief Add a new entry in the search paths
		*/
		template<class StringT> void add(const StringT& folder);
		//@}


		//! \name Execution flow
		//@{
		/*!
		** \brief Perform the traversing the root folder
		**
		** In not in detached mode, this method will block the calling thread
		** only the traversing is complete.
		** It will have no effect if a traversing is already currently in progress.
		** \return True if the thread has been started (detached mode)
		*/
		bool start();

		/*!
		** \brief Stop the traversing of the root folder
		**
		** \param timeout The timeout in milliseconds before killing the thread (detached mode only)
		** \return An error status (`errNone` if succeeded)
		*/
		bool stop(const uint32 timeout = defaultTimeout);

		/*!
		** \brief Wait for the end of the operation (infinite amount of time)
		**
		** This routine has no effect if not in detached mode.
		** \param timeout The timeout in milliseconds
		** \return An error status (`errNone` if succeeded)
		*/
		void wait();

		/*!
		** \brief Wait for the end of the operation
		**
		** This routine has no effect if not in detached mode.
		** \param timeout The timeout in milliseconds
		** \return An error status (`errNone` if succeeded)
		*/
		void wait(const uint32 timeout);

		/*!
		** \brief Ask to Stop the traversing as soon as possible
		**
		** This routine has no effect if not in detached mode.
		*/
		void gracefulStop();

		/*!
		** \brief Get if the detached thread is currently running
		**
		** \return True if the thread is running. Always false if not in detached mode.
		*/
		bool started() const;
		//@}


		//! \name Operators
		//@{
		//! assignment
		IIterator& operator = (const IIterator& rhs);
		//@}


	protected:
		/*!
		** \brief Event: An iteration has started
		**
		** This method may be called from any thread in detached mode (the calling
		** thread otherwise, but always by the same thread).
		** \param filename The given root path
		** \return False to cancel the operation
		*/
		virtual bool onStart(const String& root);

		/*!
		** \brief Event: The operation is complete
		**
		** This method may be called from any thread in detached mode (the calling
		** thread otherwise, but always by the same thread).
		** This method will not be called if the process has been canceled.
		** \see onAbort()
		*/
		virtual void onTerminate();

		/*!
		** \brief The process has been aborted
		**
		** This method may be called from any thread in detached mode (the calling
		** thread otherwise, but always by the same thread).
		*/
		virtual void onAbort();

		/*!
		** \brief Event: Starting to Traverse a new folder
		**
		** This method may be called from any thread in detached mode (the calling
		** thread otherwise, but always by the same thread).
		**
		** \param filename The full filename (ex: /path/to/my/file.txt)
		** \param parent The parent folder (ex: /path/to/my)
		** \param name The name of the folder found only (ex: file.txt)
		** \return itSkip to not go deeper in this folder
		*/
		virtual Flow onBeginFolder(const String& filename, const String& parent, const String& name);

		/*!
		** \brief Event: A folder has been traversed
		**
		** This method may be called from any thread in detached mode (the calling
		** thread otherwise, but always by the same thread).
		**
		** \param filename The full filename (ex: /path/to/my/file.txt)
		** \param parent The parent folder (ex: /path/to/my)
		** \param name The name of the folder found only (ex: file.txt)
		** \return itSkip to not go deeper in this folder
		*/
		virtual void onEndFolder(const String& filename, const String& parent, const String& name);

		/*!
		** \brief Event: A file has been found
		**
		** This method may be called from any thread in detached mode (the calling
		** thread otherwise, but always by the same thread).
		**
		** \param filename The full filename (ex: /path/to/my/file.txt)
		** \param parent The parent folder
		** \param name The name of the file only
		** \param size Size in bytes
		** \return itAbort to abort the whole process, itSkip to skip the current folder and its sub-folders
		*/
		virtual Flow onFile(const String& filename, const String& parent,
			const String& name, uint64 size);

		/*!
		** \brief Event: It was impossible to open a folder
		**
		** This method may be called from any thread in detached mode (the calling
		** thread otherwise, but always by the same thread).
		**
		** \param filename The full filename (ex: /path/to/my/file.txt)
		** \return itAbort to abort the whole process, itContinue will be used otherwise.
		*/
		virtual Flow onError(const String& filename);

		/*!
		** \brief Event: It was impossible to the status of a file
		**
		** This method may be called from any thread in detached mode (the calling
		** thread otherwise, but always by the same thread).
		**
		** \param filename The full filename (ex: /path/to/my/file.txt)
		** \return itAbort to abort the whole process, itContinue will be used otherwise.
		*/
		virtual Flow onAccessError(const String& filename);


	private:
		# ifndef YUNI_NO_THREAD_SAFE
		typedef Yuni::Private::Core::IO::Directory::Iterator::IDetachedThread  ThreadType;

		class DetachedThread : public ThreadType
		{
		public:
			DetachedThread();
			virtual ~DetachedThread();
		};
		# endif

	private:
		//! The root folder
		String::VectorPtr pRootFolder;
		# ifndef YUNI_NO_THREAD_SAFE
		//! The de tached thread (only valid if detached != 0)
		ThreadType* pThread;
		# endif
		// Friend !
		friend void Yuni::Private::Core::IO::Directory::Iterator::Traverse(Yuni::Private::Core::IO::Directory::Iterator::Options&, Yuni::Private::Core::IO::Directory::Iterator::IDetachedThread*);

	}; // class Iterator






} // namespace Directory
} // namespace IO
} // namespace Core
} // namespace Yuni

# include "iterator.hxx"

#endif // __YUNI_CORE_IO_DIRECTORY_H__
