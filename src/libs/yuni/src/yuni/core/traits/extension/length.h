#ifndef __YUNI_CORE_TRAITS_EXTENSION_INTO_LENGTH_H__
# define __YUNI_CORE_TRAITS_EXTENSION_INTO_LENGTH_H__

# include "../../../yuni.h"
# include <string>
# include <cstring>
# include "../../smartptr.h"
# include "../length.h"


namespace Yuni
{
namespace Extension
{


	// C{N}
	template<int N, class SizeT>
	class Length<char[N], SizeT>
	{
	public:
		typedef SizeT SizeType;
		enum { valid = 1, isFixed = 0, fixedLength = 0, };

	public:
		static SizeT Value(const char* const container)
		{
			// This value can not really be known at compile time
			// We may encounter literal strings :
			// "abc" -> N = 4 but the real length is 3
			// or a static buffer  char v[42] where the real length is 42
			return (N == 0) ? 0 : ('\0' == container[N-1] ? N-1 : N);
		}
	};


	// A mere CString (zero-terminated)
	template<class SizeT>
	class Length<char*, SizeT>
	{
	public:
		typedef SizeT SizeType;
		enum { valid = 1, isFixed = 0, fixedLength = 0, };

	public:
		static SizeT Value(const char* const container)
		{
			return container ? (SizeT)::strlen(container) : 0;
		}
	};


	// A mere wide string (zero-terminated)
	template<class SizeT>
	class Length<wchar_t*, SizeT>
	{
	public:
		typedef SizeT SizeType;
		enum { valid = 1, isFixed = 0, fixedLength = 0, };

	public:
		static SizeT Value(const wchar_t* const container)
		{
			return container ? (SizeT)::wcslen(container) : 0;
		}
	};



	// single char

	template<class SizeT>
	class Length<char, SizeT>
	{
	public:
		typedef SizeT SizeType;
		enum { valid = 1, isFixed = 1, fixedLength = 1, };

	public:
		static SizeT Value(const char) {return (SizeT) 1;}
	};


	// A single wide char
	template<class SizeT>
	class Length<wchar_t, SizeT>
	{
	public:
		typedef SizeT SizeType;
		enum { valid = 1, isFixed = 1, fixedLength = 1, };

	public:
		static SizeT Value(const wchar_t) {return (SizeT) 2;}
	};





	// CustomString
	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT, class SizeT>
	class Length<Yuni::CustomString<ChunkSizeT, ExpandableT,ZeroTerminatedT>, SizeT>
	{
	public:
		typedef SizeT SizeType;
		enum { valid = 1, isFixed = 0, fixedLength = 0, };

	private:
		typedef Yuni::CustomString<ChunkSizeT, ExpandableT,ZeroTerminatedT> CustomStringType;

	public:
		static SizeT Value(const CustomStringType& container)
		{
			return (SizeT) container.size();
		}
	};


	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT,
		template <class> class OwspP, template <class> class ChckP, class ConvP,
		template <class> class StorP, template <class> class ConsP, class SizeT>
	class Length<Yuni::SmartPtr<Yuni::CustomString<ChunkSizeT, ExpandableT,ZeroTerminatedT>, OwspP,ChckP,ConvP,StorP,ConsP>, SizeT>
	{
	public:
		typedef SizeT SizeType;
		enum { valid = 1, isFixed = 0, fixedLength = 0, };

	private:
		typedef Yuni::CustomString<ChunkSizeT, ExpandableT,ZeroTerminatedT> CustomStringType;
		typedef Yuni::SmartPtr<Yuni::CustomString<ChunkSizeT, ExpandableT,ZeroTerminatedT>, OwspP,ChckP,ConvP,StorP,ConsP> CustomStringTypePtr;

	public:
		static SizeT Value(const CustomStringTypePtr& container)
		{
			return (!container) ? 0 : (SizeT) container->size();
		}
	};


	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT, class SizeT>
	class Length<Yuni::CustomString<ChunkSizeT, ExpandableT,ZeroTerminatedT>*, SizeT>
	{
	public:
		typedef SizeT SizeType;
		enum { valid = 1, isFixed = 0, fixedLength = 0, };

	public:
		static SizeT Value(const Yuni::CustomString<ChunkSizeT, ExpandableT,ZeroTerminatedT>* const container)
		{
			return (container) ? (SizeT) container->size() : 0;
		}
	};



	// Yuni::String
	template<class C, int ChunkSizeT, class SizeT>
	class Length<Yuni::StringBase<C,ChunkSizeT>, SizeT>
	{
	public:
		typedef SizeT SizeType;
		enum { valid = 1, isFixed = 0, fixedLength = 0, };

	public:
		static SizeT Value(const Yuni::StringBase<C,ChunkSizeT>& container)
		{
			return (SizeT) container.size();
		}
	};


	template<class C, int ChunkSizeT, class SizeT,
		template <class> class OwspP, template <class> class ChckP, class ConvP,
		template <class> class StorP, template <class> class ConsP>
	class Length<Yuni::SmartPtr<Yuni::StringBase<C,ChunkSizeT>, OwspP,ChckP,ConvP,StorP,ConsP>, SizeT>
	{
	public:
		typedef SizeT SizeType;
		enum { valid = 1, isFixed = 0, fixedLength = 0, };

	private:
		typedef StringBase<C,ChunkSizeT> StringType;
		typedef Yuni::SmartPtr<StringBase<C,ChunkSizeT>,OwspP,ChckP,ConvP,StorP,ConsP> StringTypePtr;

	public:
		static SizeT Value(const StringTypePtr& container)
		{
			return (!container) ? 0 : (SizeT) container->size();
		}
	};

	template<class C, int ChunkSizeT, class SizeT>
	class Length<StringBase<C,ChunkSizeT>*, SizeT>
	{
	public:
		typedef SizeT SizeType;
		enum { valid = 1, isFixed = 0, fixedLength = 0, };

	private:
		typedef Yuni::StringBase<C,ChunkSizeT> StringType;

	public:
		static SizeT Value(const StringType* const container)
		{
			return container ? (SizeT)container->size() : 0;
		}
	};



	// std::string
	template<class C, class T, class Alloc, class SizeT>
	class Length<std::basic_string<C,T,Alloc>, SizeT>
	{
	public:
		typedef SizeT SizeType;
		enum { valid = 1, isFixed = 0, fixedLength = 0, };

	private:
		typedef std::basic_string<C,T,Alloc> StringType;

	public:
		static SizeT Value(const StringType& container)
		{
			return container.size();
		}
	};


	template<class C, class T, class Alloc, class SizeT,
		template <class> class OwspP, template <class> class ChckP, class ConvP,
		template <class> class StorP, template <class> class ConsP>
	class Length<Yuni::SmartPtr<std::basic_string<C,T,Alloc>,OwspP,ChckP,ConvP,StorP,ConsP>, SizeT>
	{
	public:
		typedef SizeT SizeType;
		enum { valid = 1, isFixed = 0, fixedLength = 0, };

	private:
		typedef std::basic_string<C,T,Alloc> StringType;
		typedef Yuni::SmartPtr<std::basic_string<C,T,Alloc>,OwspP,ChckP,ConvP,StorP,ConsP> StringTypePtr;

	public:
		static SizeT Value(const StringTypePtr& container)
		{
			return (!container) ? 0 : (SizeT) container->size();
		}
	};


	template<class C, class T, class Alloc, class SizeT>
	class Length<std::basic_string<C,T,Alloc>*, SizeT>
	{
	public:
		typedef SizeT SizeType;
		enum { valid = 1, isFixed = 0, fixedLength = 0, };

	private:
		typedef std::basic_string<C,T,Alloc> StringType;

	public:
		static SizeT Value(const StringType* const container)
		{
			return container ? (SizeT) container->size() : 0;
		}
	};




	// nulptr

	template<class SizeT>
	class Length<NullPtr, SizeT>
	{
	public:
		typedef SizeT SizeType;
		enum { valid = 1, isFixed = 1, fixedLength = 1, };

	public:
		static SizeT Value(const Yuni::NullPtr&)
		{
			return 0;
		}
	};




} // namespace Extension
} // namespace Yuni

#endif // __YUNI_CORE_TRAITS_EXTENSION_INTO_LENGTH_H__
