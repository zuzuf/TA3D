#ifndef __YUNI_CORE_TRAITS_EXTENSION_INTO_CSTRING_H__
# define __YUNI_CORE_TRAITS_EXTENSION_INTO_CSTRING_H__

# include "../../../yuni.h"
# include <string>
# include "../../smartptr.h"
# include "../cstring.h"


namespace Yuni
{
namespace Extension
{


	template<>
	class IntoCString<char*>
	{
	public:
		enum { valid = 1, converted = 0, zeroTerminated = 1, };

	public:
		static const char* Perform(const char* const container)
		{
			return container; /* identity */
		}
	};


	template<int N>
	class IntoCString<char[N]>
	{
	public:
		enum { valid = 1, converted = 0, zeroTerminated = 1, };

	public:
		static const char* Perform(const char* const container)
		{
			return container; /* identity */
		}
	};


	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT>
	class IntoCString<Yuni::CustomString<ChunkSizeT, ExpandableT,ZeroTerminatedT> >
	{
	public:
		typedef Yuni::CustomString<ChunkSizeT, ExpandableT,ZeroTerminatedT> CustomStringType;
		enum { valid = 1, converted = 0, zeroTerminated = CustomStringType::zeroTerminated, };

	public:
		static const char* Perform(const CustomStringType& container)
		{
			return container.c_str();
		}
	};

	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT,
		template <class> class OwspP, template <class> class ChckP, class ConvP,
		template <class> class StorP, template <class> class ConsP>
	class IntoCString<Yuni::SmartPtr<Yuni::CustomString<ChunkSizeT, ExpandableT,ZeroTerminatedT>, OwspP,ChckP,ConvP,StorP,ConsP> >
	{
	public:
		typedef Yuni::CustomString<ChunkSizeT, ExpandableT,ZeroTerminatedT> CustomStringType;
		typedef Yuni::SmartPtr<Yuni::CustomString<ChunkSizeT, ExpandableT,ZeroTerminatedT>, OwspP,ChckP,ConvP,StorP,ConsP> CustomStringTypePtr;
		enum { valid = 1, converted = 0, zeroTerminated = CustomStringType::zeroTerminated, };

	public:
		static const char* Perform(const CustomStringTypePtr& container)
		{
			return (!container) ? NULL : container->c_str();
		}
	};



	template<unsigned int ChunkSizeT, bool ExpandableT, bool ZeroTerminatedT>
	class IntoCString<Yuni::CustomString<ChunkSizeT, ExpandableT,ZeroTerminatedT>* >
	{
	public:
		typedef typename Yuni::CustomString<ChunkSizeT, ExpandableT,ZeroTerminatedT> CustomStringType;
		enum { valid = 1, converted = 0, zeroTerminated = CustomStringType::zeroTerminated, };

	public:
		static const char* Perform(const CustomStringType* const container)
		{
			return container ? container->data() : NULL;
		}
	};



	template<int ChunkSizeT>
	class IntoCString<StringBase<char,ChunkSizeT> >
	{
	public:
		enum { valid = 1, converted = 0, zeroTerminated = 1, };

	public:
		static const char* Perform(const Yuni::StringBase<char,ChunkSizeT>& container)
		{
			return container.c_str();
		}
	};

	template<int ChunkSizeT,
		template <class> class OwspP, template <class> class ChckP, class ConvP,
		template <class> class StorP, template <class> class ConsP>
	class IntoCString<Yuni::SmartPtr<StringBase<char, ChunkSizeT>, OwspP,ChckP,ConvP,StorP,ConsP> >
	{
	public:
		typedef Yuni::StringBase<char, ChunkSizeT> StringBaseType;
		typedef Yuni::SmartPtr<StringBase<char,ChunkSizeT>, OwspP,ChckP,ConvP,StorP,ConsP> StringBaseTypePtr;
		enum { valid = 1, converted = 0, zeroTerminated = 1, };

	public:
		static const char* Perform(const StringBaseTypePtr& container)
		{
			return (!container) ? NULL : container->c_str();
		}
	};





	template<int ChunkSizeT>
	class IntoCString<StringBase<char,ChunkSizeT>* >
	{
	public:
		enum { valid = 1, converted = 0, zeroTerminated = 1, };

	public:
		static const char* Perform(const Yuni::StringBase<char,ChunkSizeT>* const container)
		{
			return container ? container->c_str() : NULL;
		}
	};


	template<class T, class Alloc>
	class IntoCString<std::basic_string<char,T,Alloc> >
	{
	public:
		enum { valid = 1, converted = 0, zeroTerminated = 1, };

	public:
		static const char* Perform(const std::basic_string<char,T,Alloc>& container)
		{
			return container.c_str();
		}
	};


	template<class T, class Alloc,
		template <class> class OwspP, template <class> class ChckP, class ConvP,
		template <class> class StorP, template <class> class ConsP>
	class IntoCString<Yuni::SmartPtr<std::basic_string<char, T,Alloc>, OwspP,ChckP,ConvP,StorP,ConsP> >
	{
	public:
		typedef std::basic_string<char, T,Alloc> StringBaseType;
		typedef Yuni::SmartPtr<std::basic_string<char,T,Alloc>, OwspP,ChckP,ConvP,StorP,ConsP> StringBaseTypePtr;
		enum { valid = 1, converted = 0, zeroTerminated = 1, };

	public:
		static const char* Perform(const StringBaseTypePtr& container)
		{
			return (!container) ? NULL : container->c_str();
		}
	};



	template<class T, class Alloc>
	class IntoCString<std::basic_string<char,T,Alloc>* >
	{
	public:
		enum { valid = 1, converted = 0, zeroTerminated = 1, };

	public:
		static const char* Perform(const std::basic_string<char,T,Alloc>* const container)
		{
			return container ? container->c_str() : NULL;
		}
	};


	template<>
	class IntoCString<NullPtr>
	{
	public:
		enum { valid = 1, converted = 0, zeroTerminated = 1, };

	public:
		static const char* Perform(const Yuni::NullPtr&)
		{
			return NULL;
		}
	};






} // namespace Extension
} // namespace Yuni

#endif // __YUNI_CORE_TRAITS_EXTENSION_INTO_CSTRING_H__
