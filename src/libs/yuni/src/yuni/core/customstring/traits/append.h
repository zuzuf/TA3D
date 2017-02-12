#ifndef __YUNI_CORE_MEMORY_BUFFER_TRAITS_APPEND_H__
# define __YUNI_CORE_MEMORY_BUFFER_TRAITS_APPEND_H__

# include "../../traits/length.h"
# include "integer.h"
# include <stdio.h>

# ifdef YUNI_OS_MSVC
#	define YUNI_PRIVATE_MEMBUF_SPTRINF(BUFFER,SIZE, F, V)  ::sprintf_s(BUFFER,SIZE,F,V)
# else
#	define YUNI_PRIVATE_MEMBUF_SPTRINF(BUFFER,SIZE, F, V)  ::snprintf(BUFFER,SIZE,F,V)
# endif


namespace Yuni
{
namespace Extension
{
namespace CustomString
{

	template<class CustomStringT, class C>
	class Append
	{
	public:
		// Unknown type
		YUNI_STATIC_ASSERT(false, CustomString_AppendUnknownType);
	};


	// C*
	template<class CustomStringT, class T>
	class Append<CustomStringT, T*>
	{
	public:
		static void Perform(CustomStringT& s, const T* rhs)
		{
			s += (void*) rhs;
		}
	};


	// char*
	template<class CustomStringT>
	class Append<CustomStringT, char*>
	{
	public:
		typedef typename CustomStringT::Type TypeC;
		typedef typename Static::Remove::Const<TypeC>::Type C;
		static void Perform(CustomStringT& s, const C* rhs)
		{
			if (rhs)
				s.appendWithoutChecking(rhs, Yuni::Traits::Length<C*,typename CustomStringT::Size>::Value(rhs));
		}
	};

	// C[N]
	template<class CustomStringT, int N>
	class Append<CustomStringT, char[N]>
	{
	public:
		typedef typename CustomStringT::Type C;
		static void Perform(CustomStringT& s, const C rhs[N])
		{
			if (N > 0)
			{
				// The calculation with `N` is required to properly handle
				// both a zero-terminated buffer and a simple array
				s.appendWithoutChecking(rhs, N - ((rhs[N-1] == C()) ? 1 : 0));
			}
		}
	};

	// C
	template<class CustomStringT>
	class Append<CustomStringT, char>
	{
	public:
		typedef char C;
		static void Perform(CustomStringT& s, const C rhs)
		{
			s.appendWithoutChecking(rhs);
		}
	};


	// C
	template<class CustomStringT>
	class Append<CustomStringT, unsigned char>
	{
	public:
		typedef unsigned char C;
		static void Perform(CustomStringT& s, const C rhs)
		{
			s.appendWithoutChecking(static_cast<char>(rhs));
		}
	};

	// nullptr
	template<class CustomStringT>
	class Append<CustomStringT, Yuni::NullPtr>
	{
	public:
		static void Perform(CustomStringT&, const Yuni::NullPtr&)
		{ /* Do nothing */ }
	};


	// bool
	template<class CustomStringT>
	class Append<CustomStringT, bool>
	{
	public:
		static void Perform(CustomStringT& s, const bool rhs)
		{
			if (rhs)
				s.appendWithoutChecking("true", 4);
			else
				s.appendWithoutChecking("false", 5);
		}
	};


	// void*
	template<class CustomStringT>
	class Append<CustomStringT, void*>
	{
	public:
		static void Perform(CustomStringT& s, const void* rhs)
		{
			typename CustomStringT::Type buffer[32];
			// On Windows, it may return a negative value
			if (YUNI_PRIVATE_MEMBUF_SPTRINF(buffer, sizeof(buffer), "%p", rhs) >= 0)
			{
				s.appendWithoutChecking(buffer,
					Yuni::Traits::Length<typename CustomStringT::Type*, typename CustomStringT::Size>::Value(buffer));
			}
		}
	};


	// void*
	template<class CustomStringT>
	class Append<CustomStringT, Yuni::UTF8::Char>
	{
	public:
		static void Perform(CustomStringT& s, const Yuni::UTF8::Char& rhs)
		{
			rhs.write(s);
		}
	};




# define YUNI_PRIVATE_MEMORY_BUFFER_APPEND_IMPL(BUFSIZE, FORMAT, TYPE) \
	template<class CustomStringT> \
	class Append<CustomStringT, TYPE> \
	{ \
	public: \
		static void Perform(CustomStringT& s, const TYPE rhs) \
		{ \
			typename CustomStringT::Type buffer[BUFSIZE]; \
			/* On Windows, it may return a negative value */ \
			if (YUNI_PRIVATE_MEMBUF_SPTRINF(buffer, BUFSIZE, FORMAT, rhs) >= 0) \
			{ \
				buffer[BUFSIZE - 1] = '\0'; /* making sure that it is zero-terminated */ \
				s.appendWithoutChecking(buffer, \
					Yuni::Traits::Length<typename CustomStringT::Type*, typename CustomStringT::Size>::Value(buffer)); \
			} \
		} \
	}

# define YUNI_PRIVATE_MEMORY_BUFFER_APPEND_IMPL_INT(TYPE) \
	template<class CustomStringT> \
	class Append<CustomStringT, TYPE> \
	{ \
	public: \
		static void Perform(CustomStringT& s, const TYPE rhs) \
		{ \
			Yuni::Private::CustomStringImpl::From<Math::Base::Decimal, TYPE>::AppendTo(s, rhs); \
		} \
	}


	YUNI_PRIVATE_MEMORY_BUFFER_APPEND_IMPL_INT(sint16);
	YUNI_PRIVATE_MEMORY_BUFFER_APPEND_IMPL_INT(sint32);
	YUNI_PRIVATE_MEMORY_BUFFER_APPEND_IMPL_INT(sint64);
	YUNI_PRIVATE_MEMORY_BUFFER_APPEND_IMPL_INT(uint16);
	YUNI_PRIVATE_MEMORY_BUFFER_APPEND_IMPL_INT(uint32);
	YUNI_PRIVATE_MEMORY_BUFFER_APPEND_IMPL_INT(uint64);
	# ifdef YUNI_HAS_LONG
	YUNI_PRIVATE_MEMORY_BUFFER_APPEND_IMPL_INT(long);
	YUNI_PRIVATE_MEMORY_BUFFER_APPEND_IMPL_INT(unsigned long);
	# endif

	YUNI_PRIVATE_MEMORY_BUFFER_APPEND_IMPL(256, "%f",  float);
	YUNI_PRIVATE_MEMORY_BUFFER_APPEND_IMPL(256, "%f",  double);



	// std::vector<>
	template<class CustomStringT, class T>
	class Append<CustomStringT, std::vector<T> >
	{
	public:
		typedef std::vector<T> ListType;
		static void Perform(CustomStringT& s, const ListType& rhs)
		{
			s += '[';
			if (!rhs.empty())
			{
				const typename ListType::const_iterator end = rhs.end();
				typename ListType::const_iterator i = rhs.begin();
				s += *i;
				++i;
				for (; i != end; ++i)
					s << ", " << *i;
			}
			s += ']';
		}
	};


	// std::vector<>
	template<class CustomStringT, class T>
	class Append<CustomStringT, std::list<T> >
	{
	public:
		typedef std::list<T> ListType;
		static void Perform(CustomStringT& s, const ListType& rhs)
		{
			s += '[';
			if (!rhs.empty())
			{
				const typename ListType::const_iterator end = rhs.end();
				typename ListType::const_iterator i = rhs.begin();
				s += *i;
				++i;
				for (; i != end; ++i)
					s << ", " << *i;
			}
			s += ']';
		}
	};






} // namespace CustomString
} // namespace Extension
} // namespace Yuni

# undef YUNI_PRIVATE_MEMBUF_SPTRINF

#endif // __YUNI_CORE_MEMORY_BUFFER_TRAITS_APPEND_H__
