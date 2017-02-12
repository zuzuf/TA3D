#ifndef __YUNI_CORE_LOGS_LOGS_HXX__
# define __YUNI_CORE_LOGS_LOGS_HXX__


namespace Yuni
{
namespace Logs
{


	template<class Handlers, class Decorators, template<class> class TP>
	inline Logger<Handlers,Decorators,TP>::Logger()
		:verbosityLevel(Logger<Handlers,Decorators,TP>::defaultVerbosityLevel)
	{}


	template<class Handlers, class Decorators, template<class> class TP>
	inline Logger<Handlers,Decorators,TP>::Logger(const Logger&)
	{
		YUNI_STATIC_ASSERT(false, ThisClassCannotBeCopied);
	}



	template<class Handlers, class Decorators, template<class> class TP>
	inline Logger<Handlers,Decorators,TP>::~Logger()
	{}



	template<class Handlers, class Decorators, template<class> class TP>
	template<class VerbosityType, class StringT>
	void
	Logger<Handlers,Decorators,TP>::dispatchMessageToHandlers(const StringT& s)
	{
		// Locking the operation, according to the threading policy
		typename ThreadingPolicy::MutexLocker locker(*this);

		// Filtering the verbosity level
		// 'verbosityLevel' is a public variable
		if (VerbosityType::level <= verbosityLevel)
		{
			// Ask to all handlers to internalDecoratorWriteWL the message
			Handlers::template internalDecoratorWriteWL<LoggerType,VerbosityType, StringT>(*this, s);
		}
	}


	template<class Handlers, class Decorators, template<class> class TP>
	inline typename Logger<Handlers,Decorators,TP>::NoticeBuffer
	Logger<Handlers,Decorators,TP>::notice()
	{
		return NoticeBuffer(*this);
	}

	template<class Handlers, class Decorators, template<class> class TP>
	inline typename Logger<Handlers,Decorators,TP>::InfoBuffer
	Logger<Handlers,Decorators,TP>::info()
	{
		return InfoBuffer(*this);
	}

	template<class Handlers, class Decorators, template<class> class TP>
	inline typename Logger<Handlers,Decorators,TP>::CompatibilityBuffer
	Logger<Handlers,Decorators,TP>::compatibility()
	{
		return CompatibilityBuffer(*this);
	}



	template<class Handlers, class Decorators, template<class> class TP>
	inline typename Logger<Handlers,Decorators,TP>::CheckpointBuffer
	Logger<Handlers,Decorators,TP>::checkpoint()
	{
		return typename Logger<Handlers,Decorators,TP>::CheckpointBuffer(*this);
	}


	template<class Handlers, class Decorators, template<class> class TP>
	inline typename Logger<Handlers,Decorators,TP>::WarningBuffer
	Logger<Handlers,Decorators,TP>::warning()
	{
		return WarningBuffer(*this);
	}


	template<class Handlers, class Decorators, template<class> class TP>
	inline typename Logger<Handlers,Decorators,TP>::ErrorBuffer
	Logger<Handlers,Decorators,TP>::error()
	{
		return ErrorBuffer(*this);
	}


	template<class Handlers, class Decorators, template<class> class TP>
	inline typename Logger<Handlers,Decorators,TP>::ProgressBuffer
	Logger<Handlers,Decorators,TP>::progress()
	{
		return ProgressBuffer(*this);
	}


	template<class Handlers, class Decorators, template<class> class TP>
	inline typename Logger<Handlers,Decorators,TP>::FatalBuffer
	Logger<Handlers,Decorators,TP>::fatal()
	{
		return FatalBuffer(*this);
	}


	template<class Handlers, class Decorators, template<class> class TP>
	inline typename Logger<Handlers,Decorators,TP>::DebugBuffer
	Logger<Handlers,Decorators,TP>::debug()
	{
		return DebugBuffer(*this);
	}


	template<class Handlers, class Decorators, template<class> class TP>
	template<typename U>
	inline typename Logger<Handlers,Decorators,TP>::NoticeBuffer
	Logger<Handlers,Decorators,TP>::notice(const U& u)
	{
		return NoticeBuffer(*this, u);
	}

	template<class Handlers, class Decorators, template<class> class TP>
	template<typename U>
	inline typename Logger<Handlers,Decorators,TP>::InfoBuffer
	Logger<Handlers,Decorators,TP>::info(const U& u)
	{
		return InfoBuffer(*this, u);
	}



	template<class Handlers, class Decorators, template<class> class TP>
	template<typename U>
	inline typename Logger<Handlers,Decorators,TP>::CheckpointBuffer
	Logger<Handlers,Decorators,TP>::checkpoint(const U& u)
	{
		return CheckpointBuffer(*this, u);
	}


	template<class Handlers, class Decorators, template<class> class TP>
	template<typename U>
	inline typename Logger<Handlers,Decorators,TP>::WarningBuffer
	Logger<Handlers,Decorators,TP>::warning(const U& u)
	{
		return WarningBuffer(*this, u);
	}


	template<class Handlers, class Decorators, template<class> class TP>
	template<typename U>
	inline typename Logger<Handlers,Decorators,TP>::ErrorBuffer
	Logger<Handlers,Decorators,TP>::error(const U& u)
	{
		return ErrorBuffer(*this, u);
	}


	template<class Handlers, class Decorators, template<class> class TP>
	template<typename U>
	inline typename Logger<Handlers,Decorators,TP>::ProgressBuffer
	Logger<Handlers,Decorators,TP>::progress(const U& u)
	{
		return ProgressBuffer(*this, u);
	}


	template<class Handlers, class Decorators, template<class> class TP>
	template<typename U>
	inline typename Logger<Handlers,Decorators,TP>::CompatibilityBuffer
	Logger<Handlers,Decorators,TP>::compatibility(const U& u)
	{
		return CompatibilityBuffer(*this, u);
	}


	template<class Handlers, class Decorators, template<class> class TP>
	template<typename U>
	inline typename Logger<Handlers,Decorators,TP>::FatalBuffer
	Logger<Handlers,Decorators,TP>::fatal(const U& u)
	{
		return FatalBuffer(*this, u);
	}


	template<class Handlers, class Decorators, template<class> class TP>
	template<typename U>
	inline typename Logger<Handlers,Decorators,TP>::DebugBuffer
	Logger<Handlers,Decorators,TP>::debug(const U& u)
	{
		return DebugBuffer(*this, u);
	}


	template<class Handlers, class Decorators, template<class> class TP>
	template<class C>
	inline Private::LogImpl::Buffer<Logger<Handlers,Decorators,TP>, C, C::enabled>
	Logger<Handlers,Decorators,TP>::custom()
	{
		return Private::LogImpl::Buffer<LoggerType, C, C::enabled>(*this);
	}


	template<class Handlers, class Decorators, template<class> class TP>
	template<typename U>
	inline typename Logger<Handlers,Decorators,TP>::UnknownBuffer
	Logger<Handlers,Decorators,TP>::operator << (const U& u)
	{
		return UnknownBuffer(*this, u);
	}





} // namespace Logs
} // namespace Yuni


#endif // __YUNI_CORE_LOGS_LOGS_HXX__
