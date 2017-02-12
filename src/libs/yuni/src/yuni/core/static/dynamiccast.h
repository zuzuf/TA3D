#ifndef __YUNI_CORE_STATIC_DYNAMIC_CAST_H__
# define __YUNI_CORE_STATIC_DYNAMIC_CAST_H__

# include "inherit.h"


namespace Yuni
{
namespace Static
{


	// Forward declaration
	template<class C, class BaseT, int CInheritsFromBaseT = InheritsFrom<C,BaseT>::Yes>
	struct DynamicCastWhenInherits;



	/*!
	** \brief Perform dynamic_cast operations only when a class `C` inherits from another one `BaseT`
	**
	** \tparam C Any class (may be virtual)
	** \tparam BaseT The base class to compare with (may be virtual)
	*/
	template<class C, class BaseT, int CInheritsFromBaseT>
	struct DynamicCastWhenInherits
	{
		enum { Yes = 1, No = 0, };

		/*!
		** \brief Test if two pointer are equivalent (means "represents the same real object")
		**
		** \param ref A pointer to a base class
		** \param objPointer A pointer to any descendant of `ref`
		** \return True if C inherits from BaseT and that the two pointer are dynamically equal
		**   (represent the same object)
		*/
		template<class U, class V>
		static bool Equals(const U* ref, const V* objPointer)
		{
			return ref == dynamic_cast<const U*>(objPointer);
		}

		template<class U>
		static BaseT* Perform(U* object)
		{
			return dynamic_cast<BaseT*>(object);
		}

		template<class U>
		static const BaseT* PerformConst(const U* object)
		{
			return dynamic_cast<const BaseT*>(object);
		}

	};


	template<class C, class BaseT>
	struct DynamicCastWhenInherits<C,BaseT,0>
	{
		enum { Yes = 0, No = 1, };

		template<class U, class V>
		static bool Equals(const U*, const V*)
		{
			return false;
		}

		template<class U>
		static BaseT* Perform(U*)
		{
			return NULL;
		}

		template<class U>
		static const BaseT* PerformConst(const U*)
		{
			return NULL;
		}

	};





} // namespace Static
} // namespace Yuni

#endif // __YUNI_CORE_STATIC_DYNAMIC_CAST_H__
