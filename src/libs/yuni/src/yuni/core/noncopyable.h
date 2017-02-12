#ifndef __YUNI_CORE_NON_COPYABLE_H__
# define __YUNI_CORE_NON_COPYABLE_H__


namespace Yuni
{

	/*!
	** \brief Prevent objects of a class from being copy-constructed or assigned to each other
	**
	** \code
	** class ClassThatCanNotBeCopied : private NonCopyable<ClassThatCanNotBeCopied>
	** {
	** // ...
	** };
	** \endcode
	*/
	template <class T>
	class NonCopyable
	{
	protected:
		//! \name Constructor & Destructor
		//@{
		/*!
		** \brief Default constructor
		*/
		NonCopyable () {}
		//! Protected non-virtual destructor
		~NonCopyable () {}
		//@}

	private:
		//! Private copy constructor
		NonCopyable (const NonCopyable &) {}
		//! Private copy operator
		// The implementation is provided to avoid compilation issues with Visual Studio
		T & operator = (const T &) {return static_cast<T*>(this);}

	}; // class NonCopyable




} // namespace Yuni

#endif // __YUNI_CORE_NON_COPYABLE_H__
