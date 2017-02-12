#ifndef __YUNI_CORE_STL_ALGORITHM_H__
# define __YUNI_CORE_STL_ALGORITHM_H__

# include <algorithm>
# include "../static/remove.h"


namespace Yuni
{
namespace Core
{
namespace STL
{


	/*!
	** \brief Functor to delete a pointer: used in automatic clearing of an STL container
	**
	** \tparam T The original type of items of a STL container
	*/
	template <typename T>
	struct Delete
	{
		inline const Delete& operator () (T* ptr) const
		{
			delete ptr;
			return *this;
		}
	}; // class Delete



	/*!
	** \brief Syntactic sugar to free all pointers in an STL container and clear it
	**
	** \param container The container to clear
	** \tparam T The type of items to remove
	** \tparam U A STL container
	**
	** \code
	** typedef std::vector<int> List;
	** List list;
	** Yuni::Core::STL::DeleteAndClear<int, List>(list);
	** \endcode
	*/
	template <typename T, typename U>
	inline void DeleteAndClear(U& container)
	{
		// The original type
		typedef typename Static::Remove::All<T>::Type Type;

		// Delete each item
		std::for_each(container.begin(), container.end(), Delete<Type>());
		// Clear the container
		container.clear();
	}



} // namespace STL
} // namespace Core
} // namespace Yuni


#endif // __YUNI_CORE_STL_ALGORITHM_H__
