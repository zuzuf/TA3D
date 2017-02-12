#ifndef __YUNI_CORE_STRING_STD_CONVERTERS_FROM_HXX__
# define __YUNI_CORE_STRING_STD_CONVERTERS_FROM_HXX__

# include <cassert>


namespace Yuni
{
namespace Private
{
namespace StringImpl
{



	template<>
	struct From<char*>
	{

		template<typename C, int Chnk>
		static inline void
		Append(StringBase<C,Chnk>& s, const char* str)
		{
			if (str && '\0' != *str)
			{
				AppendRaw(s, str,
					Private::StringImpl::Length<StringBase<C,Chnk>, char*>::Value(str));
			}
		}

		template<typename C, int Chnk>
		static inline void
		Append(StringBase<C,Chnk>& s, const char* str, const typename StringBase<C,Chnk>::Size len)
		{
			if (len && str)
			{
				AppendRaw(s, str, Private::StringImpl::Min(
					Private::StringImpl::Length<StringBase<C,Chnk>, char*>::Value(str), len));
			}
		}

		template<typename C, int Chnk>
		static inline void Append(StringBase<C,Chnk>& s, const char* str, const typename StringBase<C,Chnk>::Size offset,
			const typename StringBase<C,Chnk>::Size len)
		{
			if (str && len)
			{
				const typename StringBase<C,Chnk>::Size realLen =
					Private::StringImpl::Length<StringBase<C,Chnk>, char*>::Value(str);
				if (offset < realLen)
					From<C*>::AppendRaw(s, str + offset, Private::StringImpl::Min(realLen - offset, len));
			}
		}


		template<typename C, int Chnk>
		static inline void
		Insert(StringBase<C,Chnk>& s, const char* str, const typename StringBase<C,Chnk>::Size offset)
		{
			if (str && '\0' != *str)
			{
				InsertRaw(s, str,
					Private::StringImpl::Length<StringBase<C,Chnk>, char*>::Value(str),
					offset);
			}
		}

		template<typename C, int Chnk>
		static inline void
		Insert(StringBase<C,Chnk>& s, const char* str, const typename StringBase<C,Chnk>::Size len,
			const typename StringBase<C,Chnk>::Size offset)
		{
			if (len && str && '\0' != *str)
			{
				InsertRaw(s, str,
					Private::StringImpl::Min(Private::StringImpl::Length<StringBase<C,Chnk>, char*>::Value(str), len),
					offset);
			}
		}

		template<typename C, int Chnk>
		static inline void
		AppendRaw(StringBase<C,Chnk>& s, const char* str, const typename StringBase<C,Chnk>::Size len)
		{
			assert(s.pPtr != str && "undefined behavior");
			s.reserve(s.pSize + len + 1);
			(void)::memcpy(s.pPtr + sizeof(C) * s.pSize, (const char*)str, sizeof(char) * len);
			s.pSize += len;
			s.pPtr[s.pSize] = '\0';
		}

		template<typename C, int Chnk>
		static inline void
		InsertRaw(StringBase<C,Chnk>& s, const char* str, const typename StringBase<C,Chnk>::Size len,
			const typename StringBase<C,Chnk>::Size offset)
		{
			assert(s.pPtr != str && "undefined behavior");
			s.reserve(s.pSize + len + 1);
			(void)::memmove(s.pPtr + sizeof(C) * (offset + len), s.pPtr + sizeof(C) * (offset), sizeof(C) * (s.pSize-offset));
			(void)::memcpy(s.pPtr + sizeof(C) * (offset), str, sizeof(C) * len);
			s.pSize += len;
			s.pPtr[s.pSize] = '\0';
		}

	}; // char*




	template<>
	struct From<wchar_t*>
	{

		template<typename C, int Chnk>
		static void Append(StringBase<C,Chnk>& s, const wchar_t* str)
		{
			if (str)
			{
				const size_t l = wcslen(str);
				if (!l)
					return;
				char* b = new char[l + 1];
				# if !defined(YUNI_OS_WINDOWS) || defined(YUNI_OS_MINGW)
				::wcstombs(&b[0], str, l);
				# else
				size_t i = 0;
				::wcstombs_s(&i, &b[0], l + 1, str, _TRUNCATE);
				# endif
				if (0 != *b)
					From<char*>::AppendRaw(s, b, strlen(b));
				delete[] b;
			}
		}

		template<typename C, int Chnk>
		static void Append(StringBase<C,Chnk>& s, const wchar_t* str, const typename StringBase<C,Chnk>::Size len)
		{
			if (len && str)
			{
				size_t l = wcslen(str);
				if (!l)
					return;
				if (l > len)
					l = len;
				char* b = new char[l + 1];
				# if !defined(YUNI_OS_WINDOWS) || defined(YUNI_OS_MINGW)
				::wcstombs(&b[0], str, l);
				# else
				size_t i;
				::wcstombs_s(&i, &b[0], l + 1, str, _TRUNCATE);
				# endif
				if (0 != *b)
					From<char*>::AppendRaw(s, b, strlen(b));
				delete[] b;
			}
		}

		template<typename C, int Chnk>
		static void Insert(StringBase<C,Chnk>& s, const wchar_t* str,
			const typename StringBase<C,Chnk>::Size offset)
		{
			if (str)
			{
				const size_t l = wcslen(str);
				if (!l)
					return;
				char* b = new char[l + 1];
				# if !defined(YUNI_OS_WINDOWS) || defined(YUNI_OS_MINGW)
				::wcstombs(&b[0], str, l);
				# else
				size_t i;
				::wcstombs_s(&i, &b[0], l, str, l);
				# endif
				if (0 != *b)
					From<char*>::InsertRaw(s, b, strlen(b), offset);
				delete[] b;
			}
		}

		template<typename C, int Chnk>
		static void Append(StringBase<C,Chnk>& s, const wchar_t* str, const typename StringBase<C,Chnk>::Size len,
			const typename StringBase<C,Chnk>::Size offset)
		{
			if (len && str)
			{
				size_t l = wcslen(str);
				if (!l)
					return;
				if (l > len)
					l = len;
				char* b = new char[l + 1];
				# if !defined(YUNI_OS_WINDOWS) || defined(YUNI_OS_MINGW)
				::wcstombs(&b[0], str, l);
				# else
				size_t i;
				::wcstombs_s(&i, &b[0], l, str, l);
				# endif
				if (0 != *b)
					From<char*>::AppendRaw(s, b, strlen(b), offset);
				delete[] b;
			}
		}


	}; // wchar_t




	template<>
	struct From<char>
	{
		template<typename C, int Chnk>
		static inline void
		Append(StringBase<C,Chnk>& s, const char c)
		{
			s.reserve(s.pSize + 2);
			*(s.pPtr + s.pSize) = c;
			++(s.pSize);
			*(s.pPtr + s.pSize) = '\0';
		}

		template<typename C, int Chnk>
		static inline void
		Append(StringBase<C,Chnk>& s, const char c, const typename StringBase<C,Chnk>::Size len)
		{
			if (len)
			{
				s.reserve(s.pSize + 2);
				*(s.pPtr + s.pSize) = c;
				++(s.pSize);
				*(s.pPtr + s.pSize) = '\0';
			}
		}

		template<typename C, int Chnk>
		static inline void Insert(StringBase<C,Chnk>& s, const char str, const typename StringBase<C,Chnk>::Size offset)
		{
			From<char*>::InsertRaw<C,Chnk>(s, &str, 1, offset);
		}

		template<typename C, int Chnk>
		static inline void Insert(StringBase<C,Chnk>& s, const char str, const typename StringBase<C,Chnk>::Size len, const typename StringBase<C,Chnk>::Size offset)
		{
			if (len)
				From<char*>::InsertRaw<C,Chnk>(s, &str, 1, offset);
		}

	}; // char




	template<int N>
	struct From<char[N]>
	{
		template<typename C, int Chnk>
		static inline void Append(StringBase<C,Chnk>& s, const char* str)
		{
			if (N > 1)
				From<char*>::AppendRaw<C,Chnk>(s, str, N - 1);
		}

		template<typename C, int Chnk>
		static inline void Append(StringBase<C,Chnk>& s, const char* str, const typename StringBase<C,Chnk>::Size len)
		{
			if (len)
				From<char*>::AppendRaw<C,Chnk>(s, str, Private::StringImpl::Min<typename StringBase<C,Chnk>::Size>(N - 1, len));
		}

		template<typename C, int Chnk>
		static inline void Append(StringBase<C,Chnk>& s, const char* str, const typename StringBase<C,Chnk>::Size offset,
			const typename StringBase<C,Chnk>::Size len)
		{
			if (str && len && offset < N - 1)
				From<C*>::AppendRaw(s, str + offset, Private::StringImpl::Min<typename StringBase<C,Chnk>::Size>(N - 1 - offset, len));
		}


		template<typename C, int Chnk>
		static inline void Insert(StringBase<C,Chnk>& s, const char* str, const typename StringBase<C,Chnk>::Size offset)
		{
			if (N > 1)
				From<char*>::InsertRaw<C,Chnk>(s, str, N - 1, offset);
		}

		template<typename C, int Chnk>
		static inline void Insert(StringBase<C,Chnk>& s, const char* str, const typename StringBase<C,Chnk>::Size len, const typename StringBase<C,Chnk>::Size offset)
		{
			if (len)
				From<char*>::InsertRaw<C,Chnk>(s, str, Private::StringImpl::Min<typename StringBase<C,Chnk>::Size>(N - 1, len), offset);
		}

	}; // char[N]



	template<int N>
	struct From<wchar_t[N]>
	{
		template<typename C, int Chnk>
		static inline void Append(StringBase<C,Chnk>& s, const wchar_t* str)
		{
			if (N > 1)
				From<wchar_t*>::Append<C,Chnk>(s, str, N - 1);
		}

		template<typename C, int Chnk>
		static inline void Append(StringBase<C,Chnk>& s, const wchar_t* str, const typename StringBase<C,Chnk>::Size len)
		{
			if (N > 1)
				From<wchar_t*>::Append<C,Chnk>(s, str, (len < N - 1) ? len : N - 1);
		}

		template<typename C, int Chnk>
		static inline void Insert(StringBase<C,Chnk>& s, const wchar_t* str, const typename StringBase<C,Chnk>::Size offset)
		{
			if (N > 1)
				From<wchar_t*>::Insert<C,Chnk>(s, str, N - 1,offset);
		}

		template<typename C, int Chnk>
		static inline void Insert(StringBase<C,Chnk>& s, const wchar_t* str, const typename StringBase<C,Chnk>::Size len, const typename StringBase<C,Chnk>::Size offset)
		{
			if (N > 1)
				From<wchar_t*>::Insert<C,Chnk>(s, str, (len < N - 1) ? len : N - 1, offset);
		}

	}; // char[N]



	template<typename U>
	struct From< std::basic_string<U> >
	{
		template<typename C, int Chnk>
		static inline void Append(StringBase<C,Chnk>& s, const std::basic_string<U>& str)
		{
			if (!str.empty())
				From<U*>::AppendRaw(s, str.c_str(), str.size());
		}

		template<typename C, int Chnk>
		static inline void Append(StringBase<C,Chnk>& s, const std::basic_string<U>& str, const typename StringBase<C,Chnk>::Size len)
		{
			if (!str.empty() && len)
				From<U*>::AppendRaw(s, str.c_str(), Private::StringImpl::Min(str.size(), len));
		}

		template<typename C, int Chnk>
		static inline void Insert(StringBase<C,Chnk>& s, const std::basic_string<U>& str,
			const typename StringBase<C,Chnk>::Size offset)
		{
			if (!str.empty())
				From<U*>::InsertRaw(s, str.c_str(), str.size(), offset);
		}

		template<typename C, int Chnk>
		static inline void Insert(StringBase<C,Chnk>& s, const std::basic_string<U>& str,
			const typename StringBase<C,Chnk>::Size len, const typename StringBase<C,Chnk>::Size offset)
		{
			if (!str.empty() && len)
				From<U*>::InsertRaw(s, str.c_str(), Private::StringImpl::Min(str.size(), len), offset);
		}

	}; // std::string



	template<typename C, int Chnk1>
	struct From< StringBase<C, Chnk1> >
	{
		template<int Chnk>
		static inline void Append(StringBase<C,Chnk>& s, const StringBase<C,Chnk1>& str)
		{
			assert(reinterpret_cast<const void*>(&s) != reinterpret_cast<const void*>(&str) && "undefined behavior");
			if (str.pSize)
				From<C*>::AppendRaw(s, str.pPtr, str.pSize);
		}

		template<int Chnk>
		static inline void Append(StringBase<C,Chnk>& s, const StringBase<C,Chnk1>& str, const typename StringBase<C,Chnk>::Size len)
		{
			assert(reinterpret_cast<const void*>(&s) != reinterpret_cast<const void*>(&str) && "undefined behavior");
			if (str.pSize && len)
				From<C*>::AppendRaw(s, str.pPtr, Private::StringImpl::Min(str.pSize, len));
		}

		template<int Chnk>
		static inline void Append(StringBase<C,Chnk>& s, const StringBase<C,Chnk1>& str, const typename StringBase<C,Chnk>::Size offset,
			const typename StringBase<C,Chnk>::Size len)
		{
			assert(reinterpret_cast<const void*>(&s) != reinterpret_cast<const void*>(&str) && "undefined behavior");
			if (offset < str.pSize && len)
				From<C*>::AppendRaw(s, str.pPtr + offset, Private::StringImpl::Min(str.pSize - offset, len));
		}


		template<int Chnk>
		static inline void Insert(StringBase<C,Chnk>& s, const StringBase<C,Chnk1>& str,
			const typename StringBase<C,Chnk>::Size offset)
		{
			assert(reinterpret_cast<const void*>(&s) != reinterpret_cast<const void*>(&str) && "undefined behavior");
			if (str.pSize)
				From<C*>::InsertRaw(s, str.pPtr, str.pSize, offset);
		}

		template<int Chnk>
		static inline void Insert(StringBase<C,Chnk>& s, const StringBase<C,Chnk1>& str,
			const typename StringBase<C,Chnk>::Size len, const typename StringBase<C,Chnk>::Size offset)
		{
			assert(reinterpret_cast<const void*>(&s) != reinterpret_cast<const void*>(&str) && "undefined behavior");
			if (str.pSize && len)
				From<C*>::InsertRaw(s, str.pPtr, (str.pSize < len) ? str.pSize : len, offset);
		}

	}; // yuni::string


	template<unsigned int Chnk1, bool ExpT, bool ZeroT>
	struct From< CustomString<Chnk1, ExpT, ZeroT> >
	{
		typedef CustomString<Chnk1, ExpT, ZeroT> SourceType;
		typedef typename CustomString<Chnk1, ExpT, ZeroT>::Char C;

		template<int Chnk>
		static inline void Append(StringBase<C,Chnk>& s, const SourceType& str)
		{
			if (str.size())
				From<C*>::AppendRaw(s, str.c_str(), str.size());
		}

		template<int Chnk>
		static inline void Append(StringBase<C,Chnk>& s, const SourceType& str, const typename StringBase<C,Chnk>::Size len)
		{
			assert(reinterpret_cast<const void*>(&s) != reinterpret_cast<const void*>(&str) && "undefined behavior");
			if (str.size() && len)
				From<C*>::AppendRaw(s, str.c_str(), Private::StringImpl::Min<size_t>(str.size(), len));
		}

		template<int Chnk>
		static inline void Append(StringBase<C,Chnk>& s, const SourceType& str, const typename StringBase<C,Chnk>::Size offset,
			const typename StringBase<C,Chnk>::Size len)
		{
			assert(reinterpret_cast<const void*>(&s) != reinterpret_cast<const void*>(&str) && "undefined behavior");
			if (offset < str.size() && len)
				From<C*>::AppendRaw(s, str.c_str() + offset, Private::StringImpl::Min<size_t>(str.size() - offset, len));
		}


		template<int Chnk>
		static inline void Insert(StringBase<C,Chnk>& s, const SourceType& str,
			const typename StringBase<C,Chnk>::Size offset)
		{
			assert(reinterpret_cast<const void*>(&s) != reinterpret_cast<const void*>(&str) && "undefined behavior");
			if (str.size())
				From<C*>::InsertRaw(s, str.c_str(), str.size(), offset);
		}

		template<int Chnk>
		static inline void Insert(StringBase<C,Chnk>& s, const SourceType& str,
			const typename StringBase<C,Chnk>::Size len, const typename StringBase<C,Chnk>::Size offset)
		{
			assert(reinterpret_cast<void*>(&s) != reinterpret_cast<const void*>(&str) && "undefined behavior");
			if (str.size() && len)
				From<C*>::InsertRaw(s, str.c_str(), (str.size() < len) ? str.size() : len, offset);
		}

	}; // std::string




	template<>
	struct From<bool>
	{
		template<typename C, int Chnk>
		static inline void Append(StringBase<C,Chnk>& s, const bool b)
		{
			if (b)
				Private::StringImpl::From<C*>::AppendRaw(s, "true", 4);
			else
				Private::StringImpl::From<C*>::AppendRaw(s, "false", 5);
		}

		template<typename C, int Chnk>
		static inline void Append(StringBase<C,Chnk>& s, const bool b, const typename StringBase<C,Chnk>::Size len)
		{
			if (b)
				Private::StringImpl::From<C*>::AppendRaw(s, "true", Private::StringImpl::Min(4, len));
			else
				Private::StringImpl::From<C*>::AppendRaw(s, "false", Private::StringImpl::Min(5, len));
		}

		template<typename C, int Chnk>
		static inline void Insert(StringBase<C,Chnk>& s, const bool b, const typename StringBase<C,Chnk>::Size offset)
		{
			if (b)
				From<C*>::InsertRaw(s, "true", 4, offset);
			else
				From<C*>::InsertRaw(s, "false", 5, offset);
		}

		template<typename C, int Chnk>
		static inline void Insert(StringBase<C,Chnk>& s, const bool b, const typename StringBase<C,Chnk>::Size len, const typename StringBase<C,Chnk>::Size offset)
		{
			if (b)
				From<C*>::InsertRaw(s, "true",  Private::StringImpl::Min(4, len), offset);
			else
				From<C*>::InsertRaw(s, "false", Private::StringImpl::Min(5, len), offset);
		}

	}; // bool


    template<>
	struct From<Yuni::NullPtr>
	{
		template<typename C, int Chnk>
		static inline void Append(StringBase<C,Chnk>&, const NullPtr&)
		{
		}

		template<typename C, int Chnk>
		static inline void Append(StringBase<C,Chnk>&, const NullPtr&, const typename StringBase<C,Chnk>::Size)
		{
		}

		template<typename C, int Chnk>
		static inline void Insert(StringBase<C,Chnk>&, const NullPtr&, const typename StringBase<C,Chnk>::Size)
		{
		}

		template<typename C, int Chnk>
		static inline void Insert(StringBase<C,Chnk>&, const NullPtr&, const typename StringBase<C,Chnk>::Size, const typename StringBase<C,Chnk>::Size)
		{
		}

	}; // nullptr




# ifdef YUNI_OS_MSVC
#	define YUNI_PRIVATE_SPTRINF(BUFFER,SIZE, F, V)  ::sprintf_s(BUFFER,SIZE,F,V)
# else
#	define YUNI_PRIVATE_SPTRINF(BUFFER,SIZE, F, V)  ::sprintf(BUFFER,F,V)
# endif


# define YUNI_PRIVATE_STRING_IMPL(BUFSIZE, FORMAT, TYPE) \
	template<> \
	struct From<TYPE> \
	{ \
		template<typename C, int Chnk> \
		static inline void Append(StringBase<C,Chnk>& s, const TYPE v) \
		{ \
			char buffer[BUFSIZE]; \
			(void)YUNI_PRIVATE_SPTRINF(buffer, BUFSIZE, FORMAT, v); \
			if (0 != buffer[0]) \
				From<char*>::AppendRaw(s, buffer, strlen(buffer)); \
		} \
		\
		template<typename C, int Chnk> \
		static inline void Append(StringBase<C,Chnk>& s, const TYPE v, const typename StringBase<C,Chnk>::Size len) \
		{ \
			char buffer[BUFSIZE]; \
			(void)YUNI_PRIVATE_SPTRINF(buffer, BUFSIZE, FORMAT, v); \
			if (0 != buffer[0]) \
				From<char*>::AppendRaw(s, buffer, Private::StringImpl::Min(strlen(buffer), len)); \
		} \
		\
		template<typename C, int Chnk> \
		static inline void Insert(StringBase<C,Chnk>& s, const TYPE v, const typename StringBase<C,Chnk>::Size offset) \
		{ \
			char buffer[BUFSIZE]; \
			(void)YUNI_PRIVATE_SPTRINF(buffer, BUFSIZE, FORMAT, v); \
			if (0 != buffer[0]) \
				From<char*>::InsertRaw(s, buffer, strlen(buffer), offset); \
		} \
		\
		template<typename C, int Chnk> \
		static inline void Insert(StringBase<C,Chnk>& s, const TYPE v, const typename StringBase<C,Chnk>::Size len, \
			const typename StringBase<C,Chnk>::Size offset) \
		{ \
			char buffer[BUFSIZE]; \
			(void)YUNI_PRIVATE_SPTRINF(buffer, BUFSIZE, FORMAT, v); \
			if (0 != buffer[0]) \
				From<char*>::AppendRaw(s, buffer, Private::StringImpl::Min(len, strlen(buffer)), offset); \
		} \
	}


	YUNI_PRIVATE_STRING_IMPL( 8, "%i",   sint16);
	YUNI_PRIVATE_STRING_IMPL(16, "%i",   sint32);
	YUNI_PRIVATE_STRING_IMPL(24, "%lld", sint64);
	YUNI_PRIVATE_STRING_IMPL( 8, "%u",   uint16);
	YUNI_PRIVATE_STRING_IMPL(16, "%u",   uint32);
	# ifdef YUNI_OS_32
	#	ifdef YUNI_OS_MINGW
		YUNI_PRIVATE_STRING_IMPL(27, "%I64u", uint64);
	#	else
		YUNI_PRIVATE_STRING_IMPL(27, "%llu", uint64);
	#	endif
	# else
		YUNI_PRIVATE_STRING_IMPL(27, "%llu", uint64);
	# endif


	YUNI_PRIVATE_STRING_IMPL(24, "%lf", float);
	YUNI_PRIVATE_STRING_IMPL(24, "%lf", double);


	# ifdef YUNI_HAS_LONG
	YUNI_PRIVATE_STRING_IMPL( 8, "%ld",   long);
	YUNI_PRIVATE_STRING_IMPL(16, "%lu",    unsigned long);
	# endif




} // namespace StringImpl
} // namespace Private
} // namespace Yuni

#endif // __YUNI_CORE_STRING_STD_CONVERTERS_FROM_HXX__

