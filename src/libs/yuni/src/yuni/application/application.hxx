#ifndef __YUNI_APPLICATION_APPLICATION_HXX__
# define __YUNI_APPLICATION_APPLICATION_HXX__


namespace Yuni
{
namespace Application
{


	inline IApplication::~IApplication()
	{}


	inline bool IApplication::terminated() const
	{
		return pTerminated;
	}


	inline void IApplication::terminate(const int ex)
	{
		pExitCode = ex;
		pTerminated = true;
	}


	inline const String& IApplication::exeName() const
	{
		// As this variable is only modified by the constructor, it is safe to
		// return it without any lock
		return pExeName;
	}


	inline const String& IApplication::rootFolder() const
	{
		// As this variable is only modified by the constructor, it is safe to
		// return it without any lock
		return pRootFolder;
	}


	inline int IApplication::exitCode() const
	{
		return pExitCode;
	}


	inline IApplication::IApplication(const IApplication&)
		:ThreadingPolicy()
	{}


	inline IApplication& IApplication::operator = (const IApplication&)
	{
		return *this;
	}


	inline void IApplication::execute()
	{
		if (!pTerminated)
			this->onExecute();
	}



} // namespace Application
} // namespace Yuni

#endif // __YUNI_APPLICATION_APPLICATION_HXX__


