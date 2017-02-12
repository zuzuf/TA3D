#ifndef __YUNI_CORE_CUSTOM_STRING_TRAITS_ASSIGN_H__
# define __YUNI_CORE_CUSTOM_STRING_TRAITS_ASSIGN_H__

# include "../../traits/length.h"

namespace Yuni
{
namespace Extension
{
namespace CustomString
{


	template<class CustomStringT, class C>
	class Assign
	{
	public:
		static void Perform(CustomStringT& s, const C& rhs)
		{
			// By Default, we will clear the buffer and then append the new content
			// Which is the standard behavior but may not the most efficient way
			s.clear();
			Yuni::Extension::CustomString::Append<CustomStringT, C>::Perform(s, rhs);
		}
	};


	// T*
	template<class CustomStringT, class T>
	class Assign<CustomStringT, T*>
	{
	public:
		static void Perform(CustomStringT& s, const T* rhs)
		{
			s = (void*) rhs;
		}
	};


	// cha*
	template<class CustomStringT>
	class Assign<CustomStringT, char*>
	{
	public:
		static void Perform(CustomStringT& s, const char* rhs)
		{
			if (rhs)
				s.assignWithoutChecking(rhs, Yuni::Traits::Length<char*, typename CustomStringT::Size>::Value(rhs));
			else
				s.clear();
		}
	};


	// C[N]
	template<class CustomStringT, int N>
	class Assign<CustomStringT, char[N]>
	{
	public:
		typedef char C;
		static void Perform(CustomStringT& s, const C rhs[N])
		{
			if (N > 0)
			{
				// The calculation with `N` is required to properly handle
				// both a zero-terminated buffer and a simple array
				s.assignWithoutChecking(rhs, N - ((rhs[N-1] == C()) ? 1 : 0));
			}
			else
				s.clear();
		}
	};


	// C
	template<class CustomStringT>
	class Assign<CustomStringT, char>
	{
	public:
		static void Perform(CustomStringT& s, const char rhs)
		{
			s.assignWithoutChecking(rhs);
		}
	};


	// nullptr
	template<class CustomStringT>
	class Assign<CustomStringT, Yuni::NullPtr>
	{
	public:
		static void Perform(CustomStringT& s, const Yuni::NullPtr&)
		{
			s.clear();
		}
	};



	// bool
	template<class CustomStringT>
	class Assign<CustomStringT, bool>
	{
	public:
		static void Perform(CustomStringT& s, const bool rhs)
		{
			if (rhs)
				s.assignWithoutChecking("true", 4);
			else
				s.assignWithoutChecking("false", 5);
		}
	};






} // namespace CustomString
} // namespace Extension
} // namespace Yuni

#endif // __YUNI_CORE_CUSTOM_STRING_TRAITS_ASSIGN_H__
