#ifndef __YUNI_CORE_STRING_STRING_TRAITS_VSPRINTF_HXX__
# define __YUNI_CORE_STRING_STRING_TRAITS_VSPRINTF_HXX__


namespace Yuni
{
namespace Private
{
namespace StringImpl
{



	/*!
	** \brief Generic implementation of vsnprintf
	*/
	template<class C>
	int vnsprintf(C* buffer, size_t bufferSize, const C* format, va_list argptr);


	template<>
	inline int
	vnsprintf<char>(char* buffer, size_t bufferSize, const char* format, va_list argptr)
	{
		# if defined YUNI_OS_MSVC
		#	ifdef YUNI_MSVC_SECURE_VSPRINTF
		return ::_vsnprintf_s(buffer, bufferSize-1, _TRUNCATE, format, argptr);
		#	else
		return ::_vsnprintf(buffer, bufferSize-1, format, argptr);
		#	endif
		# else // YUNI_OS_MSVC
		return ::vsnprintf(buffer, bufferSize-1, format, argptr);
		# endif
	}

	template<>
	inline int
	vnsprintf<wchar_t>(wchar_t* buffer, size_t bufferSize, const wchar_t* format, va_list argptr)
	{
		# ifdef YUNI_OS_WINDOWS
		# 	if defined YUNI_OS_MSVC
		#		ifdef YUNI_MSVC_SECURE_VSPRINTF
		return ::_vsnwprintf_s(buffer, bufferSize-1, _TRUNCATE, format, argptr);
		#		else
		return ::_vsnwprintf(buffer, bufferSize-1, format, argptr);
		#		endif
		# 	else // YUNI_OS_MSVC
		return ::vsnwprintf(buffer, bufferSize-1, format, argptr);
		# 	endif
		# else // YUNI_OS_WINDOWS
		return ::vswprintf(buffer, bufferSize-1, format, argptr);
		# endif
	}


	template<class C, int Chunk>
	static int
	vsprintf(StringBase<C,Chunk>& out, const C* format, va_list args)
	{
		if (!format || '\0' == *format)
			return 0;

		static const size_t ChunkSize = 1024;
		size_t curBufSize(0);

		int i;
		// keep trying to write the string to an ever-increasing buffer until
		// either we get the string written or we run out of memory
		while (1)
		{
			// allocate a local buffer
			curBufSize += ChunkSize;
			C* buffer = new C[curBufSize];

			// format output to local buffer
			i = Private::StringImpl::vnsprintf<C>(buffer, curBufSize * sizeof(C), format, args);
			if (-1 == i)
			{
				delete[] buffer;
				continue;
			}
			else
			{
				if (i < 0)
				{
					delete[] buffer;
					return i;
				}
			}

			out.append(buffer, static_cast<typename StringBase<C,Chunk>::Size>(i));
			delete[] buffer;
			return i;
		}
		return -1;
	}




} // namespace StringImpl
} // namespace Private
} // namespace Yuni

#endif // __YUNI_CORE_STRING_STRING_TRAITS_VSPRINTF_HXX__
