#ifndef __YUNI_CORE_ATOMIC_CUSTOMSTRING_HXX__
# define __YUNI_CORE_ATOMIC_CUSTOMSTRING_HXX__


namespace Yuni
{
namespace Extension
{
namespace CustomString
{

	template<class CustomStringT, class C> class Append;
	template<class T> class Into;


	// Atomic<>
	template<class CustomStringT, int SizeT, template<class> class TP>
	class Append<CustomStringT, Yuni::Atomic::Int<SizeT,TP> >
	{
	public:
		typedef typename CustomStringT::Type TypeC;
		typedef typename Static::Remove::Const<TypeC>::Type C;
		static void Perform(CustomStringT& s, const Yuni::Atomic::Int<SizeT, TP>& rhs)
		{
			s.append((typename Yuni::Atomic::Int<SizeT, TP>::ScalarType) rhs);
		}
	};


	template<int SizeT, template<class> class TP>
	class Into<Yuni::Atomic::Int<SizeT,TP> >
	{
	public:
		enum { valid = 1 };

		template<class StringT> static bool Perform(const StringT& s, Yuni::Atomic::Int<SizeT, TP>& out)
		{
			typedef typename Yuni::Atomic::Int<SizeT, TP>::ScalarType Scalar;
			Scalar tmp;
			if (s.template to<Scalar>(tmp))
			{
				out = tmp;
				return true;
			}
			return false;
		}

		template<class StringT>
		static typename Yuni::Atomic::Int<SizeT, TP>::ScalarType Perform(const StringT& s)
		{
			return s.template to<typename Yuni::Atomic::Int<SizeT, TP>::ScalarType>();
		}
	};



} // namespace CustomString
} // namespace Extension
} // namespace Yuni

#endif // __YUNI_CORE_ATOMIC_CUSTOMSTRING_HXX__
