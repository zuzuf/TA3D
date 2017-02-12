#ifndef __YUNI_CORE_STATIC_IF_H__
# define __YUNI_CORE_STATIC_IF_H__

# include "remove.h"



namespace Yuni
{
namespace Static
{


	template <bool b, typename IfTrue, typename IfFalse>
	struct If
	{
		typedef IfTrue ResultType;
		typedef IfTrue Type;
		typedef typename Remove::All<IfTrue>::Type   RetTrue;
		typedef typename Remove::All<IfFalse>::Type  RetFalse;

		static RetTrue& choose (RetTrue& tr, RetFalse&)
		{return tr;}

		static const RetTrue& choose (const RetTrue& tr, const RetFalse&)
		{ return tr; }
	};




	template <typename IfTrue, typename IfFalse>
	struct If<false, IfTrue, IfFalse>
	{
		typedef IfFalse ResultType;
		typedef IfFalse Type;
		typedef typename Remove::All<IfTrue>::Type   RetTrue;
		typedef typename Remove::All<IfFalse>::Type  RetFalse;

		static RetFalse& choose (RetTrue&, RetFalse& fa)
		{ return fa; }

		static const RetFalse& choose (const RetTrue&, const RetFalse& fa)
		{ return fa; }
	};



} // namespace Static
} // namespaec Yuni



#endif // __YUNI_CORE_STATIC_IF_H__
