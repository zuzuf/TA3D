#ifndef __YUNI_CORE_SMARTPTR_POLICIES_CHECKING_H__
# define __YUNI_CORE_SMARTPTR_POLICIES_CHECKING_H__



namespace Yuni
{
namespace Policy
{


/*!
** \brief Checking policies
** \ingroup Policies
*/
namespace Checking
{


	/*!
	** \brief Do not make any check
	** \ingroup Policies
	*/
	template<class T>
	class None
	{
	public:
		None() {}
		template<class U> None(const None<U>&) {}

		static void onDefault(const T&) {}

		static void onInit(const T&) {}

		static void onDereference(const T&) {}

		static void swapPointer(None&) {}

	}; // class None



	/*!
	** \brief Ensure the pointer can never be null
	** \ingroup Policies
	**
	** The default constructor (which inits at null) is disabled statically
	** by not defining onDefault.
	*/
	template<class T>
	class NeverNull
	{
	public:
		NeverNull() {}
		template<class U> NeverNull(const NeverNull<U>&) { }

		static void onInit(const T& ptr) { if (!ptr) throw; }

		static void onDereference(const T&) {}

		static void swapPointer(NeverNull&) {}

	}; // class NotNull



} // namespace Checking
} // namespace Policy
} // namespace Yuni

#endif // __YUNI_CORE_SMARTPTR_POLICIES_CHECKING_H__
