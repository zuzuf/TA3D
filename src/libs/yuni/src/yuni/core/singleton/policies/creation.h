#ifndef __YUNI_CORE_SINGLETON_POLICIES_CREATION_H__
# define __YUNI_CORE_SINGLETON_POLICIES_CREATION_H__

/*!
** \file
** \brief Singleton creation policies
*/


namespace Yuni
{
namespace Policy
{

/*!
** \brief Creation policies
** \ingroup Policies
*/
namespace Creation
{


	/*!
	** \brief Singleton creation using the new operator and an empty constructor
	** \ingroup Policies
	**
	** \tparam T The data type
	*/
	template <class T>
	class EmptyConstructor
	{
	public:
		//! Creation of the data
		static T* Create() { return new T(); }

		//! Destruction of the data
		template<class U> static void Destroy(U* data) { delete data; }
	};



} // namespace Creation
} // namespace Policy
} // namespace Yuni

#endif // __YUNI_CORE_SINGLETON_POLICIES_CREATION_H__
